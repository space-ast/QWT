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

#include "qwt_plot_barchart.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_column_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_legend_data.h"

#include <qpainter.h>

class QwtPlotBarChart::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotBarChart)

public:
    PrivateData(QwtPlotBarChart* p)
        : q_ptr(p)
        , symbol(new QwtColumnSymbol(QwtColumnSymbol::Box))
        , legendMode(QwtPlotBarChart::LegendChartTitle)
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtColumnSymbol* symbol;
    QwtPlotBarChart::LegendMode legendMode;
    bool m_userSetPen = false;
    bool m_userSetBrush = false;
};

/**
 * @brief Constructor
 * @param[in] title Title of the chart
 */
QwtPlotBarChart::QwtPlotBarChart(const QwtText& title)
    : QwtPlotAbstractBarChart(title)
    , QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Constructor
 * @param[in] title Title of the chart
 */
QwtPlotBarChart::QwtPlotBarChart(const QString& title)
    : QwtPlotAbstractBarChart(QwtText(title))
    , QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Destructor
 */
QwtPlotBarChart::~QwtPlotBarChart()
{
}

void QwtPlotBarChart::init()
{
    setData(new QwtPointSeriesData());
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotBarChart
 */
int QwtPlotBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotBarChart;
}

/**
 * @brief Attach the bar chart to a plot
 * @details If the brush has not been explicitly set by the user, the bar chart
 *          automatically receives a color from the plot's color cycle.
 *          The pen is set to a darker shade of the fill color unless user-set.
 * @param plot Plot to attach to (nullptr to detach)
 */
void QwtPlotBarChart::attach(QwtPlot* plot)
{
    QWT_D(d);
    if (plot && (!d->m_userSetBrush || !d->m_userSetPen)) {
        if (!d->symbol)
            d->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);

        const QColor c = plot->nextColorForItem(rtti());
        if (!d->m_userSetBrush)
            d->symbol->setBrush(c);
        if (!d->m_userSetPen)
            d->symbol->setPen(QPen(c.darker(150), 1));
    }
    QwtPlotItem::attach(plot);
}

/**
 * @brief Initialize data with an array of points
 * @param[in] samples Vector of points
 * @note QVector is implicitly shared.
 * @note QPolygonF is derived from QVector<QPointF>.
 */
void QwtPlotBarChart::setSamples(const QVector< QPointF >& samples)
{
    setData(new QwtPointSeriesData(samples));
}

/**
 * @brief Initialize data with an array of doubles
 * @details The indices in the array are taken as x coordinate,
 *          while the doubles are interpreted as y values.
 * @param[in] samples Vector of y coordinates
 * @note QVector is implicitly shared.
 */
void QwtPlotBarChart::setSamples(const QVector< double >& samples)
{
    QVector< QPointF > points;
    points.reserve(samples.size());

    for (int i = 0; i < samples.size(); i++)
        points += QPointF(i, samples[ i ]);

    setData(new QwtPointSeriesData(points));
}

/**
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting
 *          it when its not used anymore.
 */
void QwtPlotBarChart::setSamples(QwtSeriesData< QPointF >* data)
{
    setData(data);
}

/**
 * @brief Assign a symbol
 * @details The bar chart will take the ownership of the symbol, hence the previously
 *          set symbol will be deleted by setting a new one. If \p symbol is
 *          \c nullptr no symbol will be drawn.
 * @param[in] symbol Symbol
 * @sa symbol()
 */
void QwtPlotBarChart::setSymbol(QwtColumnSymbol* symbol)
{
    QWT_D(d);
    if (symbol != d->symbol) {
        delete d->symbol;
        d->symbol = symbol;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the current symbol
 * @return Current symbol or nullptr, when no symbol has been assigned.
 * @sa setSymbol()
 */
const QwtColumnSymbol* QwtPlotBarChart::symbol() const
{
    QWT_DC(d);
    return d->symbol;
}

/**
 * @brief Set the bar symbol pen
 * @param[in] p Pen for drawing the bar outline
 */
void QwtPlotBarChart::setPen(const QPen& p)
{
    QWT_D(d);
    d->m_userSetPen = true;
    if (!d->symbol) {
        d->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    }

    d->symbol->setPen(p);

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the bar symbol pen
 * @return Pen for drawing the bar outline
 */
QPen QwtPlotBarChart::pen() const
{
    QWT_DC(d);
    if (d->symbol) {
        return d->symbol->pen();
    }
    return QPen();
}

/**
 * @brief Set the bar symbol brush
 * @param[in] b Brush for filling the bar
 */
void QwtPlotBarChart::setBrush(const QBrush& b)
{
    QWT_D(d);
    d->m_userSetBrush = true;
    if (!d->symbol) {
        d->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    }

    d->symbol->setBrush(b);

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the bar symbol brush
 * @return Brush for filling the bar
 */
QBrush QwtPlotBarChart::brush() const
{
    QWT_DC(d);
    if (d->symbol) {
        return d->symbol->brush();
    }
    return QBrush();
}

/**
 * @brief Set the bar symbol frame style
 * @param[in] f Frame style for the bar
 */
void QwtPlotBarChart::setFrameStyle(QwtColumnSymbol::FrameStyle f)
{
    QWT_D(d);
    if (!d->symbol) {
        d->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    }

    d->symbol->setFrameStyle(f);

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the bar symbol frame style
 * @return Frame style for the bar
 */
QwtColumnSymbol::FrameStyle QwtPlotBarChart::frameStyle() const
{
    QWT_DC(d);
    if (d->symbol) {
        return d->symbol->frameStyle();
    }
    return QwtColumnSymbol::NoFrame;
}

/**
 * @brief Set the mode that decides what to display on the legend
 * @details In case of LegendBarTitles barTitle() needs to be overloaded
 *          to return individual titles for each bar.
 * @param[in] mode New mode
 * @sa legendMode(), legendData(), barTitle(), QwtPlotItem::ItemAttribute
 */
void QwtPlotBarChart::setLegendMode(LegendMode mode)
{
    QWT_D(d);
    if (mode != d->legendMode) {
        d->legendMode = mode;
        legendChanged();
    }
}

/**
 * @brief Get the legend mode
 * @return Legend mode
 * @sa setLegendMode()
 */
QwtPlotBarChart::LegendMode QwtPlotBarChart::legendMode() const
{
    QWT_DC(d);
    return d->legendMode;
}

/**
 * @brief Get the bounding rectangle of all samples
 * @return Bounding rectangle of all samples. For an empty series the rectangle is invalid.
 */
QRectF QwtPlotBarChart::boundingRect() const
{
    const size_t numSamples = dataSize();
    if (numSamples == 0)
        return QwtPlotSeriesItem::boundingRect();

    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if (rect.height() >= 0) {
        const double baseLine = baseline();

        if (rect.bottom() < baseLine)
            rect.setBottom(baseLine);

        if (rect.top() > baseLine)
            rect.setTop(baseLine);
    }

    if (orientation() == Qt::Horizontal)
        rect.setRect(rect.y(), rect.x(), rect.height(), rect.width());

    return rect;
}

/**
 * @brief Draw an interval of the bar chart
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rect of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the
 *              curve will be painted to its last point.
 * @sa drawSymbols()
 */
void QwtPlotBarChart::drawSeries(QPainter* painter,
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

    const QRectF br = data()->boundingRect();
    const QwtInterval interval(br.left(), br.right());

    painter->save();

    for (int i = from; i <= to; i++) {
        drawSample(painter, xMap, yMap, canvasRect, interval, i, sample(i));
    }

    painter->restore();
}

/*!
   Calculate the geometry of a bar in widget coordinates

   @param xMap x map
   @param yMap y map
   @param canvasRect Contents rect of the canvas
   @param boundingInterval Bounding interval of sample values
   @param sample Value of the sample

   @return Geometry of the column
 */
QwtColumnRect QwtPlotBarChart::columnRect(const QwtScaleMap& xMap,
                                          const QwtScaleMap& yMap,
                                          const QRectF& canvasRect,
                                          const QwtInterval& boundingInterval,
                                          const QPointF& sample) const
{
    QwtColumnRect barRect;

    if (orientation() == Qt::Horizontal) {
        const double barHeight = sampleWidth(yMap, canvasRect.height(), boundingInterval.width(), sample.y());

        const double x1 = xMap.transform(baseline());
        const double x2 = xMap.transform(sample.y());

        const double y  = yMap.transform(sample.x());
        const double y1 = y - 0.5 * barHeight;
        const double y2 = y + 0.5 * barHeight;

        barRect.direction = (x1 < x2) ? QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

        barRect.hInterval = QwtInterval(x1, x2).normalized();
        barRect.vInterval = QwtInterval(y1, y2);
    } else {
        const double barWidth = sampleWidth(xMap, canvasRect.width(), boundingInterval.width(), sample.y());

        const double x  = xMap.transform(sample.x());
        const double x1 = x - 0.5 * barWidth;
        const double x2 = x + 0.5 * barWidth;

        const double y1 = yMap.transform(baseline());
        const double y2 = yMap.transform(sample.y());

        barRect.direction = (y1 < y2) ? QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

        barRect.hInterval = QwtInterval(x1, x2);
        barRect.vInterval = QwtInterval(y1, y2).normalized();
    }

    return barRect;
}

/*!
   Draw a sample

   @param painter Painter
   @param xMap x map
   @param yMap y map
   @param canvasRect Contents rect of the canvas
   @param boundingInterval Bounding interval of sample values
   @param index Index of the sample
   @param sample Value of the sample

   @sa drawSeries()
 */
void QwtPlotBarChart::drawSample(QPainter* painter,
                                 const QwtScaleMap& xMap,
                                 const QwtScaleMap& yMap,
                                 const QRectF& canvasRect,
                                 const QwtInterval& boundingInterval,
                                 int index,
                                 const QPointF& sample) const
{
    const QwtColumnRect barRect = columnRect(xMap, yMap, canvasRect, boundingInterval, sample);

    drawBar(painter, index, sample, barRect);
}

/*!
   Draw a bar

   @param painter Painter
   @param sampleIndex Index of the sample represented by the bar
   @param sample Value of the sample
   @param rect Bounding rectangle of the bar
 */
void QwtPlotBarChart::drawBar(QPainter* painter, int sampleIndex, const QPointF& sample, const QwtColumnRect& rect) const
{
    QWT_DC(d);
    const QwtColumnSymbol* specialSym = specialSymbol(sampleIndex, sample);

    const QwtColumnSymbol* sym = specialSym;
    if (sym == nullptr)
        sym = d->symbol;

    if (sym) {
        sym->draw(painter, rect);
    } else {
        // we build a temporary default symbol
        QwtColumnSymbol columnSymbol(QwtColumnSymbol::Box);
        columnSymbol.setLineWidth(1);
        columnSymbol.setFrameStyle(QwtColumnSymbol::Plain);
        columnSymbol.draw(painter, rect);
    }

    delete specialSym;
}

/**
 * @brief Get a special symbol for a specific sample
 * @details Needs to be overloaded to return a non default symbol for a specific sample.
 * @param[in] sampleIndex Index of the sample represented by the bar
 * @param[in] sample Value of the sample
 * @return nullptr, indicating to use the default symbol
 */
QwtColumnSymbol* QwtPlotBarChart::specialSymbol(int sampleIndex, const QPointF& sample) const
{
    Q_UNUSED(sampleIndex);
    Q_UNUSED(sample);

    return nullptr;
}

/**
 * @brief Return the title of a bar
 * @details In LegendBarTitles mode the title is displayed on
 *          the legend entry corresponding to a bar.
 *          The default implementation is a dummy, that is intended
 *          to be overloaded.
 * @param[in] sampleIndex Index of the bar
 * @return An empty text
 * @sa LegendBarTitles
 */
QwtText QwtPlotBarChart::barTitle(int sampleIndex) const
{
    Q_UNUSED(sampleIndex);
    return QwtText();
}

/*!
   @brief Return all information, that is needed to represent
          the item on the legend

   In case of LegendBarTitles an entry for each bar is returned,
   otherwise the chart is represented like any other plot item
   from its title() and the legendIcon().

   @return Information, that is needed to represent the item on the legend
   @sa title(), setLegendMode(), barTitle(), QwtLegend, QwtPlotLegendItem
 */
QList< QwtLegendData > QwtPlotBarChart::legendData() const
{
    QWT_DC(d);
    QList< QwtLegendData > list;

    if (d->legendMode == LegendBarTitles) {
        const size_t numSamples = dataSize();
        list.reserve(numSamples);

        for (size_t i = 0; i < numSamples; i++) {
            QwtLegendData data;

            data.setValue(QwtLegendData::TitleRole, QVariant::fromValue(barTitle(i)));

            if (!legendIconSize().isEmpty()) {
                data.setValue(QwtLegendData::IconRole, QVariant::fromValue(legendIcon(i, legendIconSize())));
            }

            list += data;
        }
    } else {
        return QwtPlotAbstractBarChart::legendData();
    }

    return list;
}

/*!
   @return Icon representing a bar or the chart on the legend

   When the legendMode() is LegendBarTitles the icon shows
   the bar corresponding to index - otherwise the bar
   displays the default symbol.

   @param index Index of the legend entry
   @param size Icon size

   @sa setLegendMode(), drawBar(),
       QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 */
QwtGraphic QwtPlotBarChart::legendIcon(int index, const QSizeF& size) const
{
    QWT_DC(d);
    QwtColumnRect column;
    column.hInterval = QwtInterval(0.0, size.width() - 1.0);
    column.vInterval = QwtInterval(0.0, size.height() - 1.0);

    QwtGraphic icon;
    icon.setDefaultSize(size);
    icon.setRenderHint(QwtGraphic::RenderPensUnscaled, true);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPlotItem::RenderAntialiased));

    int barIndex = -1;
    if (d->legendMode == QwtPlotBarChart::LegendBarTitles)
        barIndex = index;

    drawBar(&painter, barIndex, QPointF(), column);

    return icon;
}
