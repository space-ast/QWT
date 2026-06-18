/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *   Summary of major modifications (see ChangeLog.md for full history):
 *   1. CMake build system & C++11 throughout.
 *   2. Core panner/ zoomer refactored:
 *        - QwtPanner → QwtCachePanner (pixmap-cache version)
 *        - New real-time QwtPlotPanner derived from QwtPicker.
 *   3. Zoomer supports multi-axis.
 *   4. Parasite-plot framework:
 *        - QwtFigure, QwtPlotParasiteLayout, QwtPlotTransparentCanvas,
 *        - QwtPlotScaleEventDispatcher, built-in pan/zoom on axis.
 *   5. New picker: QwtPlotSeriesDataPicker (works with date axis).
 *   6. Raster & color-map extensions:
 *        - QwtGridRasterData (2-D table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 *****************************************************************************/

#include "qwt_plot_vectorfield.h"
#include "qwt_vectorfield_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qdebug.h>
#include <cstdlib>
#include <limits>

#define DEBUG_RENDER 0

#if DEBUG_RENDER
#include <qelapsedtimer.h>
#endif

static inline double qwtVector2Radians(double vx, double vy)
{
    if (vx == 0.0)
        return (vy >= 0) ? M_PI_2 : 3 * M_PI_2;

    return std::atan2(vy, vx);
}

static inline double qwtVector2Magnitude(double vx, double vy)
{
    return sqrt(vx * vx + vy * vy);
}

static QwtInterval qwtMagnitudeRange(const QwtSeriesData< QwtVectorFieldSample >* series)
{
    if (series->size() == 0)
        return QwtInterval(0, 1);

    const QwtVectorFieldSample s0 = series->sample(0);

    double min = s0.vx * s0.vx + s0.vy * s0.vy;
    double max = min;

    for (uint i = 1; i < series->size(); i++) {
        const QwtVectorFieldSample s = series->sample(i);
        const double l               = s.vx * s.vx + s.vy * s.vy;

        if (l < min)
            min = l;

        if (l > max)
            max = l;
    }

    min = std::sqrt(min);
    max = std::sqrt(max);

    if (max == min)
        max += 1.0;

    return QwtInterval(min, max);
}

static inline QTransform
qwtSymbolTransformation(const QTransform& oldTransform, double x, double y, double vx, double vy, double magnitude)
{
    QTransform transform = oldTransform;

    if (!transform.isIdentity()) {
        transform.translate(x, y);

        const double radians = qwtVector2Radians(vx, vy);
        transform.rotateRadians(radians);
    } else {
        /*
            When starting with no transformation ( f.e on screen )
            the matrix can be found without having to use
            trigonometric functions
         */

        qreal sin, cos;
        if (magnitude == 0.0) {
            // something
            sin = 1.0;
            cos = 0.0;
        } else {
            sin = vy / magnitude;
            cos = vx / magnitude;
        }

        transform.setMatrix(cos, sin, 0.0, -sin, cos, 0.0, x, y, 1.0);
    }

    return transform;
}

namespace
{
class FilterMatrix
{
public:
    class Entry
    {
    public:
        inline void addSample(double sx, double sy, double svx, double svy)
        {
            x += sx;
            y += sy;

            vx += svx;
            vy += svy;

            count++;
        }

        quint32 count;

        // screen positions -> float is good enough
        float x;
        float y;
        float vx;
        float vy;
    };

    FilterMatrix(const QRectF& dataRect, const QRectF& canvasRect, const QSizeF& cellSize)
    {
        m_dx = cellSize.width();
        m_dy = cellSize.height();

        m_x0 = dataRect.x();
        if (m_x0 < canvasRect.x())
            m_x0 += int((canvasRect.x() - m_x0) / m_dx) * m_dx;

        m_y0 = dataRect.y();
        if (m_y0 < canvasRect.y())
            m_y0 += int((canvasRect.y() - m_y0) / m_dy) * m_dy;

        m_numColumns = canvasRect.width() / m_dx + 1;
        m_numRows    = canvasRect.height() / m_dy + 1;

#if 1
        /*
            limit column and row count to a maximum of 1000000,
            so that memory usage is not an issue
         */
        if (m_numColumns > 1000) {
            m_dx         = canvasRect.width() / 1000;
            m_numColumns = canvasRect.width() / m_dx + 1;
        }

        if (m_numRows > 1000) {
            m_dy      = canvasRect.height() / 1000;
            m_numRows = canvasRect.height() / m_dx + 1;
        }
#endif

        m_x1 = m_x0 + m_numColumns * m_dx;
        m_y1 = m_y0 + m_numRows * m_dy;

        m_entries = (Entry*)::calloc(m_numRows * m_numColumns, sizeof(Entry));
        if (m_entries == nullptr) {
            qWarning() << "QwtPlotVectorField: raster for filtering too fine - running out of memory";
        }
    }

    ~FilterMatrix()
    {
        if (m_entries)
            std::free(m_entries);
    }

    inline int numColumns() const
    {
        return m_numColumns;
    }

    inline int numRows() const
    {
        return m_numRows;
    }

    inline void addSample(double x, double y, double u, double v)
    {
        if (x >= m_x0 && x < m_x1 && y >= m_y0 && y < m_y1) {
            Entry& entry = m_entries[ indexOf(x, y) ];
            entry.addSample(x, y, u, v);
        }
    }

    const FilterMatrix::Entry* entries() const
    {
        return m_entries;
    }

private:
    inline int indexOf(qreal x, qreal y) const
    {
        const int col = (x - m_x0) / m_dx;
        const int row = (y - m_y0) / m_dy;

        return row * m_numColumns + col;
    }

    qreal m_x0, m_x1, m_y0, m_y1, m_dx, m_dy;
    int m_numColumns;
    int m_numRows;

    Entry* m_entries;
};
}

class QwtPlotVectorField::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotVectorField)
public:
    PrivateData(QwtPlotVectorField* p)
        : q_ptr(p)
        , pen(Qt::black)
        , brush(Qt::black)
        , indicatorOrigin(QwtPlotVectorField::OriginHead)
        , magnitudeScaleFactor(1.0)
        , rasterSize(20, 20)
        , minArrowLength(0.0)
        , maxArrowLength(std::numeric_limits< short >::max())
        , magnitudeModes(MagnitudeAsLength)
    {
        colorMap = nullptr;
        symbol   = new QwtVectorFieldThinArrow();
    }

    ~PrivateData()
    {
        delete colorMap;
        delete symbol;
    }

    QPen pen;
    QBrush brush;

    IndicatorOrigin indicatorOrigin;
    QwtVectorFieldSymbol* symbol;
    QwtColorMap* colorMap;

    /*
        Stores the range of magnitudes to be used for the color map.
        If invalid (min=max or negative values), the range is determined
        from the data samples themselves.
     */
    QwtInterval magnitudeRange;
    mutable QwtInterval boundingMagnitudeRange;

    qreal magnitudeScaleFactor;
    QSizeF rasterSize;

    double minArrowLength;
    double maxArrowLength;

    PaintAttributes paintAttributes;
    MagnitudeModes magnitudeModes;
};

/**
 * @brief Constructor
 * @param[in] title Title of the curve
 *
 */
QwtPlotVectorField::QwtPlotVectorField(const QwtText& title) : QwtPlotSeriesItem(title), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Constructor
 * @param[in] title Title of the curve
 *
 */
QwtPlotVectorField::QwtPlotVectorField(const QString& title) : QwtPlotSeriesItem(QwtText(title)), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Destructor
 *
 */
QwtPlotVectorField::~QwtPlotVectorField()
{
}

/**
 * @brief Initialize data members
 * @details Initializes the internal data members and sets default attributes.
 *
 */
void QwtPlotVectorField::init()
{
    setItemAttribute(QwtPlotItem::Legend);
    setItemAttribute(QwtPlotItem::AutoScale);

    setData(new QwtVectorFieldData());

    setZ(20.0);
}

/**
 * @brief Assign a pen
 * @param[in] pen New pen
 * @note The pen is ignored in MagnitudeAsColor mode
 * @sa pen(), brush()
 *
 */
void QwtPlotVectorField::setPen(const QPen& pen)
{
    QWT_D(d);
    if (d->pen != pen) {
        d->pen = pen;

        itemChanged();
        legendChanged();
    }
}

/**
 * @brief Get the pen used to draw the lines
 * @return Pen used for drawing
 * @sa setPen(), brush()
 *
 */
QPen QwtPlotVectorField::pen() const
{
    QWT_DC(d);
    return d->pen;
}

/**
 * @brief Assign a brush
 * @param[in] brush New brush
 * @note The brush is ignored in MagnitudeAsColor mode
 * @sa brush(), pen()
 *
 */
void QwtPlotVectorField::setBrush(const QBrush& brush)
{
    QWT_D(d);
    if (d->brush != brush) {
        d->brush = brush;

        itemChanged();
        legendChanged();
    }
}

/**
 * @brief Get the brush used to fill the symbol
 * @return Brush used for filling
 * @sa setBrush(), pen()
 *
 */
QBrush QwtPlotVectorField::brush() const
{
    QWT_DC(d);
    return d->brush;
}

/**
 * @brief Set the origin for the symbols/arrows
 * @param[in] origin Origin position
 * @sa indicatorOrigin()
 *
 */
void QwtPlotVectorField::setIndicatorOrigin(IndicatorOrigin origin)
{
    QWT_D(d);
    d->indicatorOrigin = origin;
    if (d->indicatorOrigin != origin) {
        d->indicatorOrigin = origin;
        itemChanged();
    }
}

/**
 * @brief Get the origin for the symbols/arrows
 * @return Origin position
 * @sa setIndicatorOrigin()
 *
 */
QwtPlotVectorField::IndicatorOrigin QwtPlotVectorField::indicatorOrigin() const
{
    QWT_DC(d);
    return d->indicatorOrigin;
}

/**
 * @brief Set the magnitude scale factor
 * @details The length of the arrow in screen coordinate units is calculated by
 *          scaling the magnitude by the magnitudeScaleFactor.
 * @param[in] factor Scale factor
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa magnitudeScaleFactor(), arrowLength()
 *
 */
void QwtPlotVectorField::setMagnitudeScaleFactor(double factor)
{
    QWT_D(d);
    if (factor != d->magnitudeScaleFactor) {
        d->magnitudeScaleFactor = factor;
        itemChanged();
    }
}

/**
 * @brief Get the scale factor used to calculate the arrow length from the magnitude
 * @details The length of the arrow in screen coordinate units is calculated by
 *          scaling the magnitude by the magnitudeScaleFactor.
 *          Default implementation simply scales the vector using the magnitudeScaleFactor property.
 *          Re-implement this function to provide special handling for zero/non-zero magnitude arrows,
 *          or impose minimum/maximum arrow length limits.
 * @return Scale factor
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa setMagnitudeScaleFactor()
 *
 */
double QwtPlotVectorField::magnitudeScaleFactor() const
{
    QWT_DC(d);
    return d->magnitudeScaleFactor;
}

/**
 * @brief Set the raster size used for filtering samples
 * @param[in] size Raster size
 * @sa rasterSize(), QwtPlotVectorField::FilterVectors
 *
 */
void QwtPlotVectorField::setRasterSize(const QSizeF& size)
{
    QWT_D(d);
    if (size != d->rasterSize) {
        d->rasterSize = size;
        itemChanged();
    }
}

/**
 * @brief Get the raster size used for filtering samples
 * @return Raster size
 * @sa setRasterSize(), QwtPlotVectorField::FilterVectors
 *
 */
QSizeF QwtPlotVectorField::rasterSize() const
{
    QWT_DC(d);
    return d->rasterSize;
}

/**
 * @brief Specify an attribute how to draw the curve
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 *
 */
void QwtPlotVectorField::setPaintAttribute(PaintAttribute attribute, bool on)
{
    QWT_D(d);
    PaintAttributes attributes = d->paintAttributes;

    if (on)
        attributes |= attribute;
    else
        attributes &= ~attribute;

    if (d->paintAttributes != attributes) {
        d->paintAttributes = attributes;
        itemChanged();
    }
}

/**
 * @brief Test a paint attribute
 * @return True when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 *
 */
bool QwtPlotVectorField::testPaintAttribute(PaintAttribute attribute) const
{
    QWT_DC(d);
    return (d->paintAttributes & attribute);
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotVectorField
 *
 */
int QwtPlotVectorField::rtti() const
{
    return QwtPlotItem::Rtti_PlotVectorField;
}

/**
 * @brief Set a new arrow symbol
 * @details Sets a new arrow symbol (implementation of arrow drawing code).
 * @param[in] symbol Arrow symbol
 * @note Ownership is transferred to QwtPlotVectorField.
 * @sa symbol(), drawSymbol()
 *
 */
void QwtPlotVectorField::setSymbol(QwtVectorFieldSymbol* symbol)
{
    QWT_D(d);
    if (d->symbol == symbol)
        return;

    delete d->symbol;
    d->symbol = symbol;

    itemChanged();
    legendChanged();
}

/**
 * @brief Get the arrow symbol
 * @return Arrow symbol
 * @sa setSymbol(), drawSymbol()
 *
 */
const QwtVectorFieldSymbol* QwtPlotVectorField::symbol() const
{
    QWT_DC(d);
    return d->symbol;
}

/**
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of points
 *
 */
void QwtPlotVectorField::setSamples(const QVector< QwtVectorFieldSample >& samples)
{
    setData(new QwtVectorFieldData(samples));
}

/**
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting it when its not used anymore.
 *
 */
void QwtPlotVectorField::setSamples(QwtVectorFieldData* data)
{
    setData(data);
}

/**
 * @brief Change the color map
 * @details The color map is used to map the magnitude of a sample into
 *          a color using a known range for the magnitudes.
 * @param[in] colorMap Color Map
 * @sa colorMap(), magnitudeRange()
 *
 */
void QwtPlotVectorField::setColorMap(QwtColorMap* colorMap)
{
    QWT_D(d);
    if (colorMap == nullptr)
        return;

    if (colorMap != d->colorMap) {
        delete d->colorMap;
        d->colorMap = colorMap;
    }

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color Map
 * @sa setColorMap()
 *
 */
const QwtColorMap* QwtPlotVectorField::colorMap() const
{
    QWT_DC(d);
    return d->colorMap;
}

/**
 * @brief Specify a mode how to represent the magnitude of an arrow/symbol
 * @param[in] mode Mode
 * @param[in] on On/Off
 * @sa testMagnitudeMode()
 *
 */
void QwtPlotVectorField::setMagnitudeMode(MagnitudeMode mode, bool on)
{
    QWT_D(d);
    if (on == testMagnitudeMode(mode))
        return;

    if (on)
        d->magnitudeModes |= mode;
    else
        d->magnitudeModes &= ~mode;

    itemChanged();
}

/**
 * @brief Test a magnitude mode
 * @return True when mode is enabled
 * @sa MagnitudeMode, setMagnitudeMode()
 *
 */
bool QwtPlotVectorField::testMagnitudeMode(MagnitudeMode mode) const
{
    QWT_DC(d);
    return d->magnitudeModes & mode;
}

/**
 * @brief Set the magnitude range for color map lookups
 * @details Sets the min/max magnitudes to be used for color map lookups.
 *          If invalid (min=max=0 or negative values), the range is determined from
 *          the current range of magnitudes in the vector samples.
 * @param[in] magnitudeRange Magnitude range
 * @sa magnitudeRange(), colorMap()
 *
 */
void QwtPlotVectorField::setMagnitudeRange(const QwtInterval& magnitudeRange)
{
    QWT_D(d);
    if (d->magnitudeRange != magnitudeRange) {
        d->magnitudeRange = magnitudeRange;
        itemChanged();
    }
}

/**
 * @brief Get the magnitude range for color map lookups
 * @return Magnitude range
 * @sa setMagnitudeRange(), colorMap()
 *
 */
QwtInterval QwtPlotVectorField::magnitudeRange() const
{
    QWT_DC(d);
    return d->magnitudeRange;
}

/**
 * @brief Set a minimum for the arrow length of non zero vectors
 * @param[in] length Minimum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa minArrowLength(), setMaxArrowLength(), arrowLength()
 *
 */
void QwtPlotVectorField::setMinArrowLength(double length)
{
    QWT_D(d);
    length = qMax(length, 0.0);

    if (d->minArrowLength != length) {
        d->minArrowLength = length;
        itemChanged();
    }
}

/**
 * @brief Get the minimum for the arrow length of non zero vectors
 * @return Minimum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa setMinArrowLength(), maxArrowLength(), arrowLength()
 *
 */
double QwtPlotVectorField::minArrowLength() const
{
    QWT_DC(d);
    return d->minArrowLength;
}

/**
 * @brief Set a maximum for the arrow length
 * @param[in] length Maximum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa maxArrowLength(), setMinArrowLength(), arrowLength()
 *
 */
void QwtPlotVectorField::setMaxArrowLength(double length)
{
    QWT_D(d);
    length = qMax(length, 0.0);

    if (d->maxArrowLength != length) {
        d->maxArrowLength = length;
        itemChanged();
    }
}

/**
 * @brief Get the maximum for the arrow length
 * @return Maximum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa setMinArrowLength(), maxArrowLength(), arrowLength()
 *
 */
double QwtPlotVectorField::maxArrowLength() const
{
    QWT_DC(d);
    return d->maxArrowLength;
}

/**
 * @brief Calculate the arrow length for a given magnitude
 * @details Computes length of the arrow in screen coordinate units based on its magnitude.
 *          Default implementation simply scales the vector using the magnitudeScaleFactor().
 *          If the result is not null, the length is then bounded into the interval
 *          [ minArrowLength(), maxArrowLength() ].
 *          Re-implement this function to provide special handling for
 *          zero/non-zero magnitude arrows, or impose minimum/maximum arrow length limits.
 * @param[in] magnitude Magnitude
 * @return Length of arrow to be drawn in dependence of vector magnitude.
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa magnitudeScaleFactor, minArrowLength(), maxArrowLength()
 *
 */
double QwtPlotVectorField::arrowLength(double magnitude) const
{
    QWT_DC(d);
#if 0
    /*
       Normalize magnitude with respect to value range.  Then, magnitudeScaleFactor
       is the number of pixels to draw for a vector of length equal to
       magnitudeRange.maxValue(). The relative scaling ensures that change of data
       samples of very different magnitudes will always lead to a reasonable
       display on screen.
     */
    const QwtVectorFieldData* vectorData = dynamic_cast< const QwtVectorFieldData* >( data() );
    if ( d->magnitudeRange.maxValue() > 0 )
        magnitude /= d->magnitudeRange.maxValue();
#endif

    double length = magnitude * d->magnitudeScaleFactor;

    if (length > 0.0)
        length = qBound(d->minArrowLength, length, d->maxArrowLength);

    return length;
}

/**
 * @brief Get the bounding rectangle
 * @return Bounding rectangle of all samples
 *
 */
QRectF QwtPlotVectorField::boundingRect() const
{
#if 0
    /*
        The bounding rectangle of the samples comes from the origins
        only, but as we know the scaling factor for the magnitude
        ( qwtVector2Magnitude ) here, we could try to include it ?
     */
#endif

    return QwtPlotSeriesItem::boundingRect();
}

/**
 * @brief Get the icon representing the vector fields on the legend
 * @param[in] index Index of the legend entry ( ignored as there is only one )
 * @param[in] size Icon size
 * @return Legend icon
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 *
 */
QwtGraphic QwtPlotVectorField::legendIcon(int index, const QSizeF& size) const
{
    QWT_DC(d);
    Q_UNUSED(index);

    QwtGraphic icon;
    icon.setDefaultSize(size);

    if (size.isEmpty())
        return icon;

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPlotItem::RenderAntialiased));

    painter.translate(-size.width(), -0.5 * size.height());

    painter.setPen(d->pen);
    painter.setBrush(d->brush);

    d->symbol->setLength(size.width() - 2);
    d->symbol->paint(&painter);

    return icon;
}

/**
 * @brief Draw a subset of the points
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first sample to be painted
 * @param[in] to Index of the last sample to be painted. If to < 0 the
 *               series will be painted to its last sample.
 *
 */
void QwtPlotVectorField::drawSeries(QPainter* painter,
                                    const QwtScaleMap& xMap,
                                    const QwtScaleMap& yMap,
                                    const QRectF& canvasRect,
                                    int from,
                                    int to) const
{
    if (!painter || dataSize() <= 0)
        return;

    if (to < 0)
        to = dataSize() - 1;

    if (from < 0)
        from = 0;

    if (from > to)
        return;

#if DEBUG_RENDER
    QElapsedTimer timer;
    timer.start();
#endif

    drawSymbols(painter, xMap, yMap, canvasRect, from, to);

#if DEBUG_RENDER
    qDebug() << timer.elapsed();
#endif
}

/*!
   Draw symbols

   @param painter Painter
   @param xMap x map
   @param yMap y map
   @param canvasRect Contents rectangle of the canvas
   @param from Index of the first sample to be painted
   @param to Index of the last sample to be painted

   @sa setSymbol(), drawSymbol(), drawSeries()
 */
void QwtPlotVectorField::drawSymbols(QPainter* painter,
                                     const QwtScaleMap& xMap,
                                     const QwtScaleMap& yMap,
                                     const QRectF& canvasRect,
                                     int from,
                                     int to) const
{
    QWT_DC(d);
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    const bool doClip  = false;

    const bool isInvertingX = xMap.isInverting();
    const bool isInvertingY = yMap.isInverting();

    const QwtSeriesData< QwtVectorFieldSample >* series = data();

    if (d->magnitudeModes & MagnitudeAsColor) {
        // user input error, can't draw without color map
        // TODO: Discuss! Without colormap, silently fall back to uniform colors?
        if (d->colorMap == nullptr)
            return;
    } else {
        painter->setPen(d->pen);
        painter->setBrush(d->brush);
    }

    if ((d->paintAttributes & FilterVectors) && !d->rasterSize.isEmpty()) {
        const QRectF dataRect = QwtScaleMap::transform(xMap, yMap, boundingRect());

        // TODO: Discuss. How to handle raster size when switching from screen to print size!
        //       DPI-aware adjustment of rastersize? Or make "rastersize in screen coordinate"
        //       or "rastersize in plotcoordinetes" a user option?
#if 1
        // define filter matrix based on screen/print coordinates
        FilterMatrix matrix(dataRect, canvasRect, d->rasterSize);
#else
        // define filter matrix based on real coordinates

        // get scale factor from real coordinates to screen coordinates
        double xScale = 1;
        if (xMap.sDist() != 0)
            xScale = xMap.pDist() / xMap.sDist();

        double yScale = 1;
        if (yMap.sDist() != 0)
            yScale = yMap.pDist() / yMap.sDist();

        QSizeF canvasRasterSize(xScale * d->rasterSize.width(), yScale * d->rasterSize.height());
        FilterMatrix matrix(dataRect, canvasRect, canvasRasterSize);
#endif

        for (int i = from; i <= to; i++) {
            const QwtVectorFieldSample sample = series->sample(i);
            if (!sample.isNull()) {
                matrix.addSample(xMap.transform(sample.x), yMap.transform(sample.y), sample.vx, sample.vy);
            }
        }

        const int numEntries               = matrix.numRows() * matrix.numColumns();
        const FilterMatrix::Entry* entries = matrix.entries();

        for (int i = 0; i < numEntries; i++) {
            const FilterMatrix::Entry& entry = entries[ i ];

            if (entry.count == 0)
                continue;

            double xi = entry.x / entry.count;
            double yi = entry.y / entry.count;

            if (doAlign) {
                xi = qRound(xi);
                yi = qRound(yi);
            }

            const double vx = entry.vx / entry.count;
            const double vy = entry.vy / entry.count;

            drawSymbol(painter, xi, yi, isInvertingX ? -vx : vx, isInvertingY ? -vy : vy);
        }
    } else {
        for (int i = from; i <= to; i++) {
            const QwtVectorFieldSample sample = series->sample(i);

            // arrows with zero length are never drawn
            if (sample.isNull())
                continue;

            double xi = xMap.transform(sample.x);
            double yi = yMap.transform(sample.y);

            if (doAlign) {
                xi = qRound(xi);
                yi = qRound(yi);
            }

            if (doClip) {
                if (!canvasRect.contains(xi, yi))
                    continue;
            }

            drawSymbol(painter, xi, yi, isInvertingX ? -sample.vx : sample.vx, isInvertingY ? -sample.vy : sample.vy);
        }
    }
}

/*!
   Draw a arrow/symbols at a specific position

   x, y, are paint device coordinates, while vx, vy are from
   the corresponding sample.

   @sa setSymbol(), drawSeries()
 */
void QwtPlotVectorField::drawSymbol(QPainter* painter, double x, double y, double vx, double vy) const
{
    QWT_DC(d);
    const double magnitude = qwtVector2Magnitude(vx, vy);

    const QTransform oldTransform = painter->transform();

    QTransform transform = qwtSymbolTransformation(oldTransform, x, y, vx, vy, magnitude);

    QwtVectorFieldSymbol* symbol = d->symbol;

    double length = 0.0;

    if (d->magnitudeModes & MagnitudeAsLength) {
        length = arrowLength(magnitude);
    }

    symbol->setLength(length);

    if (d->indicatorOrigin == OriginTail) {
        const qreal dx = symbol->length();
        transform.translate(dx, 0.0);
    } else if (d->indicatorOrigin == OriginCenter) {
        const qreal dx = symbol->length();
        transform.translate(0.5 * dx, 0.0);
    }

    if (d->magnitudeModes & MagnitudeAsColor) {
        // Determine color for arrow if colored by magnitude.

        QwtInterval range = d->magnitudeRange;

        if (!range.isValid()) {
            if (!d->boundingMagnitudeRange.isValid())
                const_cast< QwtInterval& >(d->boundingMagnitudeRange) = qwtMagnitudeRange(data());

            range = d->boundingMagnitudeRange;
        }

        const QColor c = QColor::fromRgba(d->colorMap->rgb(range.minValue(), range.maxValue(), magnitude));

#if 1
        painter->setBrush(c);
        painter->setPen(c);
#endif
    }

    painter->setWorldTransform(transform, false);
    symbol->paint(painter);
    painter->setWorldTransform(oldTransform, false);
}

void QwtPlotVectorField::dataChanged()
{
    QWT_D(d);
    d->boundingMagnitudeRange.invalidate();
    QwtPlotSeriesItem::dataChanged();
}
