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

#include "qwt_plot_spectrogram.h"
#include "qwt_painter.h"
#include "qwt_interval.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_colormap_preset.h"
#include "qwt_math.h"

#include <qimage.h>
#include <qpen.h>
#include <qpainter.h>
#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>

#define DEBUG_RENDER 0

#if DEBUG_RENDER
#include <qelapsedtimer.h>
#endif

#include <algorithm>

static inline bool qwtIsNaN(double d)
{
    // qt_is_nan is private header and qIsNaN is not inlined
    // so we need these code here too

    const uchar* ch = (const uchar*)&d;
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return (ch[ 0 ] & 0x7f) == 0x7f && ch[ 1 ] > 0xf0;
    } else {
        return (ch[ 7 ] & 0x7f) == 0x7f && ch[ 6 ] > 0xf0;
    }
}

class QwtPlotSpectrogram::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotSpectrogram)
public:
    PrivateData(QwtPlotSpectrogram* p) : q_ptr(p), data(nullptr), colorTableSize(0)
    {
        colorMap    = QwtColorMapPreset::create(QwtColorMapPreset::Viridis).release();
        displayMode = ImageMode;

        conrecFlags = QwtRasterData::IgnoreAllVerticesOnLevel;
#if 0
        conrecFlags |= QwtRasterData::IgnoreOutOfRange;
#endif
    }

    ~PrivateData()
    {
        delete data;
        delete colorMap;
    }

    void updateColorTable()
    {
        if (colorMap->format() == QwtColorMap::Indexed) {
            colorTable = colorMap->colorTable256();
        } else {
            if (colorTableSize == 0)
                colorTable.clear();
            else
                colorTable = colorMap->colorTable(colorTableSize);
        }
    }

    QwtRasterData* data;
    QwtColorMap* colorMap;
    DisplayModes displayMode;

    QList< double > contourLevels;
    QPen defaultContourPen;
    QwtRasterData::ConrecFlags conrecFlags;

    int colorTableSize;
    QVector< QRgb > colorTable;
};

/**
 * @brief Constructor
 * @details Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 *
 *          The z value is initialized by 8.0.
 * @param[in] title Title
 * @sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 *
 */
QwtPlotSpectrogram::QwtPlotSpectrogram(const QString& title) : QwtPlotRasterItem(title), QWT_PIMPL_CONSTRUCT
{
    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, false);

    setZ(8.0);
}

/**
 * @brief Destructor
 *
 */
QwtPlotSpectrogram::~QwtPlotSpectrogram()
{
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotSpectrogram
 *
 */
int QwtPlotSpectrogram::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectrogram;
}

/**
 * @brief Set a display mode
 * @details The display mode controls how the raster data will be represented.
 *          The default setting enables ImageMode.
 * @param[in] mode Display mode
 * @param[in] on On/Off
 * @sa DisplayMode, testDisplayMode()
 *
 */
void QwtPlotSpectrogram::setDisplayMode(DisplayMode mode, bool on)
{
    QWT_D(d);
    if (on != bool(mode & d->displayMode)) {
        if (on)
            d->displayMode |= mode;
        else
            d->displayMode &= ~mode;
    }

    legendChanged();
    itemChanged();
}

/**
 * @brief Test a display mode
 * @details The display mode controls how the raster data will be represented.
 * @param[in] mode Display mode
 * @return true if mode is enabled
 *
 */
bool QwtPlotSpectrogram::testDisplayMode(DisplayMode mode) const
{
    QWT_DC(d);
    return (d->displayMode & mode);
}

/**
 * @brief Change the color map
 * @details Often it is useful to display the mapping between intensities and
 *          colors as an additional plot axis, showing a color bar.
 * @param[in] colorMap Color map
 * @sa colorMap(), QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 *
 */
void QwtPlotSpectrogram::setColorMap(QwtColorMap* colorMap)
{
    QWT_D(d);
    if (colorMap == nullptr)
        return;

    if (colorMap != d->colorMap) {
        delete d->colorMap;
        d->colorMap = colorMap;
    }

    d->updateColorTable();

    invalidateCache();

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color map used for mapping the intensity values to colors
 * @sa setColorMap()
 *
 */
const QwtColorMap* QwtPlotSpectrogram::colorMap() const
{
    QWT_DC(d);
    return d->colorMap;
}

/**
 * @brief Limit the number of colors being used by the color map
 * @details When using a color table the mapping from the value into a color
 *          is usually faster as it can be done by simple lookups into a
 *          precalculated color table.
 *
 *          Setting a table size > 0 enables using a color table, while setting
 *          the size to 0 disables it. The default size = 0, and no color table is used.
 * @param[in] numColors Number of colors. 0 means not using a color table
 * @note The colorTableSize has no effect when using a color table
 *       of QwtColorMap::Indexed, where the size is always 256.
 * @sa QwtColorMap::colorTable(), colorTableSize()
 *
 */
void QwtPlotSpectrogram::setColorTableSize(int numColors)
{
    QWT_D(d);
    numColors = qMax(numColors, 0);
    if (numColors != d->colorTableSize) {
        d->colorTableSize = numColors;
        d->updateColorTable();
        invalidateCache();
    }
}

/**
 * @brief Get the color table size
 * @return Size of the color table, 0 means not using a color table
 * @sa QwtColorMap::colorTable(), setColorTableSize()
 *
 */
int QwtPlotSpectrogram::colorTableSize() const
{
    QWT_DC(d);
    return d->colorTableSize;
}

/**
 * @brief Build and assign the default pen for the contour lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) what makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa defaultContourPen(), contourPen()
 *
 */
void QwtPlotSpectrogram::setDefaultContourPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setDefaultContourPen(QPen(color, width, style));
}

/**
 * @brief Set the default pen for the contour lines
 * @details If the spectrogram has a valid default contour pen
 *          a contour line is painted using the default contour pen.
 *          Otherwise (pen.style() == Qt::NoPen) the pen is calculated
 *          for each contour level using contourPen().
 * @param[in] pen New default pen
 * @sa defaultContourPen(), contourPen()
 *
 */
void QwtPlotSpectrogram::setDefaultContourPen(const QPen& pen)
{
    QWT_D(d);
    if (pen != d->defaultContourPen) {
        d->defaultContourPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the default contour pen
 * @return Default contour pen
 * @sa setDefaultContourPen()
 *
 */
QPen QwtPlotSpectrogram::defaultContourPen() const
{
    QWT_DC(d);
    return d->defaultContourPen;
}

/**
 * @brief Calculate the pen for a contour line
 * @details The color of the pen is the color for level calculated by the color map.
 * @param[in] level Contour level
 * @return Pen for the contour line
 * @note contourPen is only used if defaultContourPen().style() == Qt::NoPen
 * @sa setDefaultContourPen(), setColorMap(), setContourLevels()
 *
 */
QPen QwtPlotSpectrogram::contourPen(double level) const
{
    QWT_DC(d);
    if (d->data == nullptr || d->colorMap == nullptr)
        return QPen();

    const QwtInterval intensityRange = d->data->interval(Qt::ZAxis);
    const QColor c(d->colorMap->rgb(intensityRange.minValue(), intensityRange.maxValue(), level));

    return QPen(c);
}

/**
 * @brief Modify an attribute of the CONREC algorithm
 * @details Used to calculate the contour lines.
 * @param[in] flag CONREC flag
 * @param[in] on On/Off
 * @sa testConrecFlag(), renderContourLines(), QwtRasterData::contourLines()
 *
 */
void QwtPlotSpectrogram::setConrecFlag(QwtRasterData::ConrecFlag flag, bool on)
{
    QWT_D(d);
    if (bool(d->conrecFlags & flag) == on)
        return;

    if (on)
        d->conrecFlags |= flag;
    else
        d->conrecFlags &= ~flag;

    itemChanged();
}

/**
 * @brief Test an attribute of the CONREC algorithm
 * @details Used to calculate the contour lines.
 *          The default setting enables QwtRasterData::IgnoreAllVerticesOnLevel.
 * @param[in] flag CONREC flag
 * @return true if enabled
 * @sa setConrecFlag(), renderContourLines(), QwtRasterData::contourLines()
 *
 */
bool QwtPlotSpectrogram::testConrecFlag(QwtRasterData::ConrecFlag flag) const
{
    QWT_DC(d);
    return d->conrecFlags & flag;
}

/**
 * @brief Set the levels of the contour lines
 * @param[in] levels Values of the contour levels
 * @sa contourLevels(), renderContourLines(), QwtRasterData::contourLines()
 * @note contourLevels returns the same levels but sorted.
 *
 */
void QwtPlotSpectrogram::setContourLevels(const QList< double >& levels)
{
    QWT_D(d);
    d->contourLevels = levels;
    std::sort(d->contourLevels.begin(), d->contourLevels.end());

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the levels of the contour lines
 * @return Levels of the contour lines. The levels are sorted in increasing order.
 * @sa setContourLevels(), renderContourLines(), QwtRasterData::contourLines()
 *
 */
QList< double > QwtPlotSpectrogram::contourLevels() const
{
    QWT_DC(d);
    return d->contourLevels;
}

/**
 * @brief Set the data to be displayed
 * @details The ownership of the data is managed by QwtPlotSpectrogram.
 * @param[in] data Spectrogram data
 * @sa data()
 *
 */
void QwtPlotSpectrogram::setData(QwtRasterData* data)
{
    QWT_D(d);
    if (data != d->data) {
        delete d->data;
        d->data = data;

        invalidateCache();
        itemChanged();
    }
}

/**
 * @brief Get the spectrogram data (const version)
 * @return Spectrogram data
 * @sa setData()
 *
 */
const QwtRasterData* QwtPlotSpectrogram::data() const
{
    QWT_DC(d);
    return d->data;
}

/**
 * @brief Get the spectrogram data
 * @return Spectrogram data
 * @sa setData()
 *
 */
QwtRasterData* QwtPlotSpectrogram::data()
{
    QWT_D(d);
    return d->data;
}

/**
 * @brief Get the bounding interval for an axis
 * @details The default implementation returns the interval of the
 *          associated raster data object.
 * @param[in] axis X, Y, or Z axis
 * @return Bounding interval for the axis
 * @sa QwtRasterData::interval()
 *
 */
QwtInterval QwtPlotSpectrogram::interval(Qt::Axis axis) const
{
    QWT_DC(d);
    if (d->data == nullptr)
        return QwtInterval();

    return d->data->interval(axis);
}

/**
 * @brief Get the pixel hint
 * @details The geometry of a pixel is used to calculated the resolution and
 *          alignment of the rendered image.
 *          The default implementation returns data()->pixelHint( rect ).
 * @param[in] area In most implementations the resolution of the data doesn't
 *                 depend on the requested area.
 * @return Bounding rectangle of a pixel
 * @sa QwtPlotRasterItem::pixelHint(), QwtRasterData::pixelHint(), render(), renderImage()
 *
 */
QRectF QwtPlotSpectrogram::pixelHint(const QRectF& area) const
{
    QWT_DC(d);
    if (d->data == nullptr)
        return QRectF();

    return d->data->pixelHint(area);
}

/*!
   @brief Render an image from data and color map.

   For each pixel of area the value is mapped into a color.

   @param xMap X-Scale Map
   @param yMap Y-Scale Map
   @param area Requested area for the image in scale coordinates
   @param imageSize Size of the requested image

   @return A QImage::Format_Indexed8 or QImage::Format_ARGB32 depending
           on the color map.

   @sa QwtRasterData::value(), QwtColorMap::rgb(),
       QwtColorMap::colorIndex()
 */
QImage QwtPlotSpectrogram::renderImage(const QwtScaleMap& xMap,
                                       const QwtScaleMap& yMap,
                                       const QRectF& area,
                                       const QSize& imageSize) const
{
    QWT_DC(d);
    if (imageSize.isEmpty() || d->data == nullptr || d->colorMap == nullptr) {
        return QImage();
    }

    const QwtInterval intensityRange = d->data->interval(Qt::ZAxis);
    if (!intensityRange.isValid())
        return QImage();

    const QImage::Format format = (d->colorMap->format() == QwtColorMap::RGB) ? QImage::Format_ARGB32
                                                                              : QImage::Format_Indexed8;

    QImage image(imageSize, format);

    if (d->colorMap->format() == QwtColorMap::Indexed)
        image.setColorTable(d->colorMap->colorTable256());

    d->data->initRaster(area, image.size());

#if DEBUG_RENDER
    QElapsedTimer time;
    time.start();
#endif

#if !defined(QT_NO_QFUTURE)
    uint numThreads = renderThreadCount();

    if (numThreads <= 0)
        numThreads = QThread::idealThreadCount();

    if (numThreads <= 0)
        numThreads = 1;

    const int numRows = imageSize.height() / numThreads;

    QVector< QFuture< void > > futures;
    futures.reserve(numThreads - 1);

    for (uint i = 0; i < numThreads; i++) {
        QRect tile(0, i * numRows, image.width(), numRows);
        if (i == numThreads - 1) {
            tile.setHeight(image.height() - i * numRows);
            renderTile(xMap, yMap, tile, &image);
        } else {
            futures += QtConcurrent::run(
#if QT_VERSION >= 0x060000
                &QwtPlotSpectrogram::renderTile,
                this,
#else
                this,
                &QwtPlotSpectrogram::renderTile,
#endif
                xMap,
                yMap,
                tile,
                &image);
        }
    }

    for (int i = 0; i < futures.size(); i++)
        futures[ i ].waitForFinished();

#else
    const QRect tile(0, 0, image.width(), image.height());
    renderTile(xMap, yMap, tile, &image);
#endif

#if DEBUG_RENDER
    const qint64 elapsed = time.elapsed();
    qDebug() << "renderImage" << imageSize << elapsed;
#endif

    d->data->discardRaster();

    return image;
}

/*!
    @brief Render a tile of an image.

    Rendering in tiles can be used to composite an image in parallel
    threads.

    @param xMap X-Scale Map
    @param yMap Y-Scale Map
    @param tile Geometry of the tile in image coordinates
    @param image Image to be rendered
 */
void QwtPlotSpectrogram::renderTile(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRect& tile, QImage* image) const
{
    QWT_DC(d);
    const QwtInterval range = d->data->interval(Qt::ZAxis);
    if (range.width() <= 0.0)
        return;

    const double rMin = range.minValue();
    const double rMax = range.maxValue();

    const bool hasGaps = !d->data->testAttribute(QwtRasterData::WithoutGaps);

    if (d->colorMap->format() == QwtColorMap::RGB) {
        const int numColors         = d->colorTable.size();
        const QRgb* rgbTable        = d->colorTable.constData();
        const QwtColorMap* colorMap = d->colorMap;

        for (int y = tile.top(); y <= tile.bottom(); y++) {
            const double ty = yMap.invTransform(y);

            QRgb* line = reinterpret_cast< QRgb* >(image->scanLine(y));
            line += tile.left();

            for (int x = tile.left(); x <= tile.right(); x++) {
                const double tx = xMap.invTransform(x);

                const double value = d->data->value(tx, ty);

                if (hasGaps && qwtIsNaN(value)) {
                    *line++ = 0u;
                } else if (numColors == 0) {
                    *line++ = colorMap->rgb(rMin, rMax, value);
                } else {
                    const uint index = colorMap->colorIndex(numColors, rMin, rMax, value);
                    *line++          = rgbTable[ index ];
                }
            }
        }
    } else if (d->colorMap->format() == QwtColorMap::Indexed) {
        for (int y = tile.top(); y <= tile.bottom(); y++) {
            const double ty = yMap.invTransform(y);

            unsigned char* line = image->scanLine(y);
            line += tile.left();

            for (int x = tile.left(); x <= tile.right(); x++) {
                const double tx = xMap.invTransform(x);

                const double value = d->data->value(tx, ty);

                if (hasGaps && qwtIsNaN(value)) {
                    *line++ = 0;
                } else {
                    const uint index = d->colorMap->colorIndex(256, rMin, rMax, value);
                    *line++          = static_cast< unsigned char >(index);
                }
            }
        }
    }
}

/*!
   @brief Return the raster to be used by the CONREC contour algorithm.

   A larger size will improve the precision of the CONREC algorithm,
   but will slow down the time that is needed to calculate the lines.

   The default implementation returns rect.size() / 2 bounded to
   the resolution depending on pixelSize().

   @param area Rectangle, where to calculate the contour lines
   @param rect Rectangle in pixel coordinates, where to paint the contour lines
   @return Raster to be used by the CONREC contour algorithm.

   @note The size will be bounded to rect.size().

   @sa drawContourLines(), QwtRasterData::contourLines()
 */
QSize QwtPlotSpectrogram::contourRasterSize(const QRectF& area, const QRect& rect) const
{
    QSize raster = rect.size() / 2;

    const QRectF pixelRect = pixelHint(area);
    if (!pixelRect.isEmpty()) {
        const QSize res(qwtCeil(rect.width() / pixelRect.width()), qwtCeil(rect.height() / pixelRect.height()));
        raster = raster.boundedTo(res);
    }

    return raster;
}

/*!
   Calculate contour lines

   @param rect Rectangle, where to calculate the contour lines
   @param raster Raster, used by the CONREC algorithm
   @return Calculated contour lines

   @sa contourLevels(), setConrecFlag(),
       QwtRasterData::contourLines()
 */
QwtRasterData::ContourLines QwtPlotSpectrogram::renderContourLines(const QRectF& rect, const QSize& raster) const
{
    QWT_DC(d);
    if (d->data == nullptr)
        return QwtRasterData::ContourLines();

    return d->data->contourLines(rect, raster, d->contourLevels, d->conrecFlags);
}

/*!
   Paint the contour lines

   @param painter Painter
   @param xMap Maps x-values into pixel coordinates.
   @param yMap Maps y-values into pixel coordinates.
   @param contourLines Contour lines

   @sa renderContourLines(), defaultContourPen(), contourPen()
 */
void QwtPlotSpectrogram::drawContourLines(QPainter* painter,
                                          const QwtScaleMap& xMap,
                                          const QwtScaleMap& yMap,
                                          const QwtRasterData::ContourLines& contourLines) const
{
    QWT_DC(d);
    if (d->data == nullptr)
        return;

    const int numLevels = d->contourLevels.size();
    for (int l = 0; l < numLevels; l++) {
        const double level = d->contourLevels[ l ];

        QPen pen = defaultContourPen();
        if (pen.style() == Qt::NoPen)
            pen = contourPen(level);

        if (pen.style() == Qt::NoPen)
            continue;

        painter->setPen(pen);

        const QPolygonF& lines = contourLines[ level ];
        for (int i = 0; i < lines.size(); i += 2) {
            const QPointF p1(xMap.transform(lines[ i ].x()), yMap.transform(lines[ i ].y()));
            const QPointF p2(xMap.transform(lines[ i + 1 ].x()), yMap.transform(lines[ i + 1 ].y()));

            QwtPainter::drawLine(painter, p1, p2);
        }
    }
}

/**
 * @brief Draw the spectrogram
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * @sa setDisplayMode(), renderImage(), QwtPlotRasterItem::draw(), drawContourLines()
 *
 */
void QwtPlotSpectrogram::draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const
{
    QWT_DC(d);
    if (d->displayMode & ImageMode)
        QwtPlotRasterItem::draw(painter, xMap, yMap, canvasRect);

    if (d->displayMode & ContourMode) {
        // Add some pixels at the borders
        const int margin = 2;
        QRectF rasterRect(canvasRect.x() - margin,
                          canvasRect.y() - margin,
                          canvasRect.width() + 2 * margin,
                          canvasRect.height() + 2 * margin);

        QRectF area = QwtScaleMap::invTransform(xMap, yMap, rasterRect);

        const QRectF br = boundingRect();
        if (br.isValid()) {
            area &= br;
            if (area.isEmpty())
                return;

            rasterRect = QwtScaleMap::transform(xMap, yMap, area);
        }

        QSize raster = contourRasterSize(area, rasterRect.toRect());
        raster       = raster.boundedTo(rasterRect.toRect().size());
        if (raster.isValid()) {
            const QwtRasterData::ContourLines lines = renderContourLines(area, raster);

            drawContourLines(painter, xMap, yMap, lines);
        }
    }
}
