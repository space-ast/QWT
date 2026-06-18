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

#include "qwt_plot_tradingcurve.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>

static inline bool qwtIsSampleInside(const QwtOHLCSample& sample, double tMin, double tMax, double vMin, double vMax)
{
    const double t             = sample.time;
    const QwtInterval interval = sample.boundingInterval();

    const bool isOffScreen = (t < tMin) || (t > tMax) || (interval.maxValue() < vMin) || (interval.minValue() > vMax);

    return !isOffScreen;
}

class QwtPlotTradingCurve::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotTradingCurve)
public:
    PrivateData(QwtPlotTradingCurve* p)
        : q_ptr(p)
        , symbolStyle(QwtPlotTradingCurve::CandleStick)
        , symbolExtent(0.6)
        , minSymbolWidth(2.0)
        , maxSymbolWidth(-1.0)
        , paintAttributes(QwtPlotTradingCurve::ClipSymbols)
    {
        symbolBrush[ 0 ] = QBrush(Qt::white);
        symbolBrush[ 1 ] = QBrush(Qt::black);
    }

    QwtPlotTradingCurve::SymbolStyle symbolStyle;
    double symbolExtent;
    double minSymbolWidth;
    double maxSymbolWidth;

    QPen symbolPen;
    QBrush symbolBrush[ 2 ];  // Increasing/Decreasing

    QwtPlotTradingCurve::PaintAttributes paintAttributes;
};

/**
 * @brief Constructor
 * @param[in] title Title of the curve
 *
 */
QwtPlotTradingCurve::QwtPlotTradingCurve(const QwtText& title) : QwtPlotSeriesItem(title), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Constructor
 * @param[in] title Title of the curve
 *
 */
QwtPlotTradingCurve::QwtPlotTradingCurve(const QString& title) : QwtPlotSeriesItem(QwtText(title)), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Destructor
 *
 */
QwtPlotTradingCurve::~QwtPlotTradingCurve()
{
}

/**
 * @brief Initialize internal members
 * @details Sets default attributes and creates internal data storage.
 *
 */
void QwtPlotTradingCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend, true);
    setItemAttribute(QwtPlotItem::AutoScale, true);

    setData(new QwtTradingChartData());

    setZ(19.0);
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotTradingCurve
 *
 */
int QwtPlotTradingCurve::rtti() const
{
    return QwtPlotTradingCurve::Rtti_PlotTradingCurve;
}

/**
 * @brief Specify an attribute how to draw the curve
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 *
 */
void QwtPlotTradingCurve::setPaintAttribute(PaintAttribute attribute, bool on)
{
    QWT_D(d);
    if (on)
        d->paintAttributes |= attribute;
    else
        d->paintAttributes &= ~attribute;
}

/**
 * @brief Test a paint attribute
 * @return True when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 *
 */
bool QwtPlotTradingCurve::testPaintAttribute(PaintAttribute attribute) const
{
    QWT_DC(d);
    return (d->paintAttributes & attribute);
}

/**
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of samples
 * @sa QwtPlotSeriesItem::setData()
 *
 */
void QwtPlotTradingCurve::setSamples(const QVector< QwtOHLCSample >& samples)
{
    setData(new QwtTradingChartData(samples));
}

/**
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting it when its not used anymore.
 *
 */
void QwtPlotTradingCurve::setSamples(QwtSeriesData< QwtOHLCSample >* data)
{
    setData(data);
}

/**
 * @brief Set the symbol style
 * @param[in] style Symbol style
 * @sa symbolStyle(), setSymbolExtent(), setSymbolPen(), setSymbolBrush()
 *
 */
void QwtPlotTradingCurve::setSymbolStyle(SymbolStyle style)
{
    QWT_D(d);
    if (style != d->symbolStyle) {
        d->symbolStyle = style;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the symbol style
 * @return Symbol style
 * @sa setSymbolStyle(), symbolExtent(), symbolPen(), symbolBrush()
 *
 */
QwtPlotTradingCurve::SymbolStyle QwtPlotTradingCurve::symbolStyle() const
{
    QWT_DC(d);
    return d->symbolStyle;
}

/**
 * @brief Build and assign the symbol pen
 * @details In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
 *          non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 *
 */
void QwtPlotTradingCurve::setSymbolPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setSymbolPen(QPen(color, width, style));
}

/**
 * @brief Set the symbol pen
 * @details The symbol pen is used for rendering the lines of the bar or candlestick symbols
 * @param[in] pen Pen
 * @sa symbolPen(), setSymbolBrush()
 *
 */
void QwtPlotTradingCurve::setSymbolPen(const QPen& pen)
{
    QWT_D(d);
    if (pen != d->symbolPen) {
        d->symbolPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the symbol pen
 * @return Symbol pen
 * @sa setSymbolPen(), symbolBrush()
 *
 */
QPen QwtPlotTradingCurve::symbolPen() const
{
    QWT_DC(d);
    return d->symbolPen;
}

/**
 * @brief Set the symbol brush
 * @param[in] direction Direction type
 * @param[in] brush Brush used to fill the body of all candlestick symbols with the direction
 * @sa symbolBrush(), setSymbolPen()
 *
 */
void QwtPlotTradingCurve::setSymbolBrush(Direction direction, const QBrush& brush)
{
    QWT_D(d);
    // silencing -Wtautological-constant-out-of-range-compare
    const int index = static_cast< int >(direction);
    if (index < 0 || index >= 2)
        return;

    if (brush != d->symbolBrush[ index ]) {
        d->symbolBrush[ index ] = brush;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the symbol brush
 * @param[in] direction Direction type
 * @return Brush used to fill the body of all candlestick symbols with the direction
 * @sa setSymbolPen(), symbolBrush()
 *
 */
QBrush QwtPlotTradingCurve::symbolBrush(Direction direction) const
{
    QWT_DC(d);
    const int index = static_cast< int >(direction);
    if (index < 0 || index >= 2)
        return QBrush();

    return d->symbolBrush[ index ];
}

/**
 * @brief Set the extent of the symbol
 * @details The width of the symbol is given in scale coordinates. When painting
 *          a symbol the width is scaled into paint device coordinates
 *          by scaledSymbolWidth(). The scaled width is bounded by
 *          minSymbolWidth(), maxSymbolWidth()
 * @param[in] extent Symbol width in scale coordinates
 * @sa symbolExtent(), scaledSymbolWidth(), setMinSymbolWidth(), setMaxSymbolWidth()
 *
 */
void QwtPlotTradingCurve::setSymbolExtent(double extent)
{
    QWT_D(d);
    extent = qwtMaxF(0.0, extent);
    if (extent != d->symbolExtent) {
        d->symbolExtent = extent;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the extent of the symbol
 * @return Extent of a symbol in scale coordinates
 * @sa setSymbolExtent(), scaledSymbolWidth(), minSymbolWidth(), maxSymbolWidth()
 *
 */
double QwtPlotTradingCurve::symbolExtent() const
{
    QWT_DC(d);
    return d->symbolExtent;
}

/**
 * @brief Set a minimum for the symbol width
 * @param[in] width Width in paint device coordinates
 * @sa minSymbolWidth(), setMaxSymbolWidth(), setSymbolExtent()
 *
 */
void QwtPlotTradingCurve::setMinSymbolWidth(double width)
{
    QWT_D(d);
    width = qwtMaxF(width, 0.0);
    if (width != d->minSymbolWidth) {
        d->minSymbolWidth = width;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the minimum for the symbol width
 * @return Minimum for the symbol width
 * @sa setMinSymbolWidth(), maxSymbolWidth(), symbolExtent()
 *
 */
double QwtPlotTradingCurve::minSymbolWidth() const
{
    QWT_DC(d);
    return d->minSymbolWidth;
}

/**
 * @brief Set a maximum for the symbol width
 * @details A value <= 0.0 means an unlimited width
 * @param[in] width Width in paint device coordinates
 * @sa maxSymbolWidth(), setMinSymbolWidth(), setSymbolExtent()
 *
 */
void QwtPlotTradingCurve::setMaxSymbolWidth(double width)
{
    QWT_D(d);
    if (width != d->maxSymbolWidth) {
        d->maxSymbolWidth = width;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the maximum for the symbol width
 * @return Maximum for the symbol width
 * @sa setMaxSymbolWidth(), minSymbolWidth(), symbolExtent()
 *
 */
double QwtPlotTradingCurve::maxSymbolWidth() const
{
    QWT_DC(d);
    return d->maxSymbolWidth;
}

/**
 * @brief Get the bounding rectangle of all samples
 * @return Bounding rectangle of all samples. For an empty series the rectangle is invalid.
 *
 */
QRectF QwtPlotTradingCurve::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if (orientation() == Qt::Vertical)
        rect.setRect(rect.y(), rect.x(), rect.height(), rect.width());

    return rect;
}

/**
 * @brief Draw an interval of the curve
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the curve will be painted to its last point.
 * @sa drawSymbols()
 *
 */
void QwtPlotTradingCurve::drawSeries(QPainter* painter,
                                     const QwtScaleMap& xMap,
                                     const QwtScaleMap& yMap,
                                     const QRectF& canvasRect,
                                     int from,
                                     int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    if (from < 0)
        from = 0;

    if (from > to)
        return;

    QWT_DC(d);
    painter->save();

    if (d->symbolStyle != QwtPlotTradingCurve::NoSymbol)
        drawSymbols(painter, xMap, yMap, canvasRect, from, to);

    painter->restore();
}

/**
 * @brief Draw symbols
 * @param[in] painter Painter
 * @param[in] xMap x map
 * @param[in] yMap y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa drawSeries()
 *
 */
void QwtPlotTradingCurve::drawSymbols(QPainter* painter,
                                      const QwtScaleMap& xMap,
                                      const QwtScaleMap& yMap,
                                      const QRectF& canvasRect,
                                      int from,
                                      int to) const
{
    QWT_DC(d);
    const QRectF tr = QwtScaleMap::invTransform(xMap, yMap, canvasRect);

    const QwtScaleMap *timeMap, *valueMap;
    double tMin, tMax, vMin, vMax;

    const Qt::Orientation orient = orientation();
    if (orient == Qt::Vertical) {
        timeMap  = &xMap;
        valueMap = &yMap;

        tMin = tr.left();
        tMax = tr.right();
        vMin = tr.top();
        vMax = tr.bottom();
    } else {
        timeMap  = &yMap;
        valueMap = &xMap;

        vMin = tr.left();
        vMax = tr.right();
        tMin = tr.top();
        tMax = tr.bottom();
    }

    const bool inverted = timeMap->isInverting();
    const bool doClip   = d->paintAttributes & ClipSymbols;
    const bool doAlign  = QwtPainter::roundingAlignment(painter);

    double symbolWidth = scaledSymbolWidth(xMap, yMap, canvasRect);
    if (doAlign)
        symbolWidth = std::floor(0.5 * symbolWidth) * 2.0;

    QPen pen = d->symbolPen;
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);

    for (int i = from; i <= to; i++) {
        const QwtOHLCSample s = sample(i);

        if (!doClip || qwtIsSampleInside(s, tMin, tMax, vMin, vMax)) {
            QwtOHLCSample translatedSample;

            translatedSample.time  = timeMap->transform(s.time);
            translatedSample.open  = valueMap->transform(s.open);
            translatedSample.high  = valueMap->transform(s.high);
            translatedSample.low   = valueMap->transform(s.low);
            translatedSample.close = valueMap->transform(s.close);

            const int brushIndex = (s.open < s.close) ? QwtPlotTradingCurve::Increasing : QwtPlotTradingCurve::Decreasing;

            if (doAlign) {
                translatedSample.time  = qRound(translatedSample.time);
                translatedSample.open  = qRound(translatedSample.open);
                translatedSample.high  = qRound(translatedSample.high);
                translatedSample.low   = qRound(translatedSample.low);
                translatedSample.close = qRound(translatedSample.close);
            }

            switch (d->symbolStyle) {
            case Bar: {
                drawBar(painter, translatedSample, orient, inverted, symbolWidth);
                break;
            }
            case CandleStick: {
                painter->setBrush(d->symbolBrush[ brushIndex ]);
                drawCandleStick(painter, translatedSample, orient, symbolWidth);
                break;
            }
            default: {
                if (d->symbolStyle >= UserSymbol) {
                    painter->setBrush(d->symbolBrush[ brushIndex ]);
                    drawUserSymbol(painter, d->symbolStyle, translatedSample, orient, inverted, symbolWidth);
                }
            }
            }
        }
    }
}

/**
 * @brief Draw a symbol for a symbol style >= UserSymbol
 * @details The implementation does nothing and is intended to be overloaded
 * @param[in] painter Qt painter, initialized with pen/brush
 * @param[in] symbolStyle Symbol style
 * @param[in] sample Samples already translated into paint device coordinates
 * @param[in] orientation Vertical or horizontal
 * @param[in] inverted True, when the opposite scale ( Qt::Vertical: x, Qt::Horizontal: y ) is increasing
 *                      in the opposite direction as QPainter coordinates.
 * @param[in] symbolWidth Width of the symbol in paint device coordinates
 *
 */
void QwtPlotTradingCurve::drawUserSymbol(QPainter* painter,
                                         SymbolStyle symbolStyle,
                                         const QwtOHLCSample& sample,
                                         Qt::Orientation orientation,
                                         bool inverted,
                                         double symbolWidth) const
{
    Q_UNUSED(painter)
    Q_UNUSED(symbolStyle)
    Q_UNUSED(orientation)
    Q_UNUSED(inverted)
    Q_UNUSED(symbolWidth)
    Q_UNUSED(sample)
}

/**
 * @brief Draw a bar
 * @param[in] painter Qt painter, initialized with pen/brush
 * @param[in] sample Sample, already translated into paint device coordinates
 * @param[in] orientation Vertical or horizontal
 * @param[in] inverted When inverted is false the open tick is painted to the left/top,
 *                      otherwise it is painted right/bottom. The close tick is painted
 *                      in the opposite direction of the open tick.
 * @param[in] width Width or height of the candle, depending on the orientation
 * @sa Bar
 *
 */
void QwtPlotTradingCurve::drawBar(QPainter* painter,
                                  const QwtOHLCSample& sample,
                                  Qt::Orientation orientation,
                                  bool inverted,
                                  double width) const
{
    double w2 = 0.5 * width;
    if (inverted)
        w2 *= -1;

    if (orientation == Qt::Vertical) {
        QwtPainter::drawLine(painter, sample.time, sample.low, sample.time, sample.high);

        QwtPainter::drawLine(painter, sample.time - w2, sample.open, sample.time, sample.open);
        QwtPainter::drawLine(painter, sample.time + w2, sample.close, sample.time, sample.close);
    } else {
        QwtPainter::drawLine(painter, sample.low, sample.time, sample.high, sample.time);
        QwtPainter::drawLine(painter, sample.open, sample.time - w2, sample.open, sample.time);
        QwtPainter::drawLine(painter, sample.close, sample.time + w2, sample.close, sample.time);
    }
}

/**
 * @brief Draw a candle stick
 * @param[in] painter Qt painter, initialized with pen/brush
 * @param[in] sample Samples already translated into paint device coordinates
 * @param[in] orientation Vertical or horizontal
 * @param[in] width Width or height of the candle, depending on the orientation
 * @sa CandleStick
 *
 */
void QwtPlotTradingCurve::drawCandleStick(QPainter* painter,
                                          const QwtOHLCSample& sample,
                                          Qt::Orientation orientation,
                                          double width) const
{
    const double t  = sample.time;
    const double v1 = qwtMinF(sample.low, sample.high);
    const double v2 = qwtMinF(sample.open, sample.close);
    const double v3 = qwtMaxF(sample.low, sample.high);
    const double v4 = qwtMaxF(sample.open, sample.close);

    if (orientation == Qt::Vertical) {
        QwtPainter::drawLine(painter, t, v1, t, v2);
        QwtPainter::drawLine(painter, t, v3, t, v4);

        QRectF rect(t - 0.5 * width, sample.open, width, sample.close - sample.open);

        QwtPainter::drawRect(painter, rect);
    } else {
        QwtPainter::drawLine(painter, v1, t, v2, t);
        QwtPainter::drawLine(painter, v3, t, v4, t);

        const QRectF rect(sample.open, t - 0.5 * width, sample.close - sample.open, width);

        QwtPainter::drawRect(painter, rect);
    }
}

/**
 * @brief Get a legend icon
 * @details Returns a rectangle filled with the color of the symbol pen
 * @param[in] index Index of the legend entry ( usually there is only one )
 * @param[in] size Icon size
 * @return Legend icon
 * @sa setLegendIconSize(), legendData()
 *
 */
QwtGraphic QwtPlotTradingCurve::legendIcon(int index, const QSizeF& size) const
{
    QWT_DC(d);
    Q_UNUSED(index);
    return defaultIcon(d->symbolPen.color(), size);
}

/**
 * @brief Calculate the symbol width in paint coordinates
 * @details The width is calculated by scaling the symbol extent into
 *          paint device coordinates bounded by the minimum/maximum symbol width.
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * @return Symbol width in paint coordinates
 * @sa symbolExtent(), minSymbolWidth(), maxSymbolWidth()
 *
 */
double QwtPlotTradingCurve::scaledSymbolWidth(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const
{
    QWT_DC(d);
    Q_UNUSED(canvasRect);

    if (d->maxSymbolWidth > 0.0 && d->minSymbolWidth >= d->maxSymbolWidth) {
        return d->minSymbolWidth;
    }

    const QwtScaleMap* map = (orientation() == Qt::Vertical) ? &xMap : &yMap;

    const double pos = map->transform(map->s1() + d->symbolExtent);

    double width = qAbs(pos - map->p1());

    width = qwtMaxF(width, d->minSymbolWidth);
    if (d->maxSymbolWidth > 0.0)
        width = qwtMinF(width, d->maxSymbolWidth);

    return width;
}
