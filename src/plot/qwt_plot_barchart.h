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

#ifndef QWT_PLOT_BAR_CHART_H
#define QWT_PLOT_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_abstract_barchart.h"
#include "qwt_column_symbol.h"

class QwtColumnRect;
template< typename T >
class QwtSeriesData;

/**
 * \if ENGLISH
 * @brief QwtPlotBarChart displays a series of values as bars
 * @details Each bar might be customized individually by implementing
 *          a specialSymbol(). Otherwise it is rendered using a default symbol.
 * 
 *          Depending on its orientation() the bars are displayed horizontally
 *          or vertically. The bars cover the interval between the baseline()
 *          and the value.
 * 
 *          By activating the LegendBarTitles mode each sample will have
 *          its own entry on the legend.
 * 
 *          The most common use case of a bar chart is to display a
 *          list of y coordinates, where the x coordinate is simply the index
 *          in the list. But for other situations ( f.e. when values are related
 *          to dates ) it is also possible to set x coordinates explicitly.
 * 
 * @sa QwtPlotMultiBarChart, QwtPlotHistogram, QwtPlotCurve::Sticks,
 *     QwtPlotSeriesItem::orientation(), QwtPlotAbstractBarChart::baseline()
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotBarChart 将一系列值显示为条形
 * @details 每个条形可以通过实现 specialSymbol() 进行单独自定义。
 *          否则，它将使用默认符号进行渲染。
 * 
 *          根据其 orientation()，条形可以水平或垂直显示。
 *          条形覆盖从 baseline() 到值之间的区间。
 * 
 *          通过激活 LegendBarTitles 模式，每个样本将在图例上有自己的条目。
 * 
 *          条形图最常见的用例是显示 y 坐标列表，其中 x 坐标只是列表中的索引。
 *          但对于其他情况（例如，当值与日期相关时），也可以显式设置 x 坐标。
 * 
 * @sa QwtPlotMultiBarChart, QwtPlotHistogram, QwtPlotCurve::Sticks,
 *     QwtPlotSeriesItem::orientation(), QwtPlotAbstractBarChart::baseline()
 * \endif
 */
class QWT_EXPORT QwtPlotBarChart : public QwtPlotAbstractBarChart, public QwtSeriesStore< QPointF >
{
public:
    /**
     * \if ENGLISH
     * @brief Legend modes
     * @details The default setting is QwtPlotBarChart::LegendChartTitle.
     * @sa setLegendMode(), legendMode()
     * \endif
     * 
     * \if CHINESE
     * @brief 图例模式
     * @details 默认设置是 QwtPlotBarChart::LegendChartTitle。
     * @sa setLegendMode(), legendMode()
     * \endif
     */
    enum LegendMode
    {
        /**
         * \if ENGLISH
         * One entry on the legend showing the default symbol
         * and the title() of the chart
         * @sa QwtPlotItem::title()
         * \endif
         * 
         * \if CHINESE
         * 图例上的一个条目，显示默认符号和图表的 title()
         * @sa QwtPlotItem::title()
         * \endif
         */
        LegendChartTitle,

        /**
         * \if ENGLISH
         * One entry for each value showing the individual symbol
         * of the corresponding bar and the bar title.
         * @sa specialSymbol(), barTitle()
         * \endif
         * 
         * \if CHINESE
         * 每个值的一个条目，显示对应条形的单独符号和条形标题。
         * @sa specialSymbol(), barTitle()
         * \endif
         */
        LegendBarTitles
    };

    // Constructor
    explicit QwtPlotBarChart(const QString& title = QString());
    // Constructor with QwtText title
    explicit QwtPlotBarChart(const QwtText& title);

    // Destructor
    virtual ~QwtPlotBarChart();

    // Get the runtime type information
    virtual int rtti() const override;

    // Set samples from QVector<QPointF>
    void setSamples(const QVector< QPointF >&);
    // Set samples from QVector<double>
    void setSamples(const QVector< double >&);
    // Set samples from QwtSeriesData
    void setSamples(QwtSeriesData< QPointF >*);

    // Set symbol
    void setSymbol(QwtColumnSymbol*);
    // Get symbol
    const QwtColumnSymbol* symbol() const;

    // Set pen
    void setPen(const QPen& p);
    // Get pen
    QPen pen() const;

    // Set brush
    void setBrush(const QBrush& b);
    // Get brush
    QBrush brush() const;

    // Set frame style
    void setFrameStyle(QwtColumnSymbol::FrameStyle f);
    // Get frame style
    QwtColumnSymbol::FrameStyle frameStyle() const;

    // Set legend mode
    void setLegendMode(LegendMode);
    // Get legend mode
    LegendMode legendMode() const;

    // Draw the series
    virtual void drawSeries(QPainter*,
                            const QwtScaleMap& xMap,
                            const QwtScaleMap& yMap,
                            const QRectF& canvasRect,
                            int from,
                            int to) const override;

    // Get the bounding rectangle
    virtual QRectF boundingRect() const override;

    // Get special symbol for a sample
    virtual QwtColumnSymbol* specialSymbol(int sampleIndex, const QPointF&) const;

    // Get bar title for a sample
    virtual QwtText barTitle(int sampleIndex) const;

protected:
    /// Draw a sample
    virtual void drawSample(QPainter* painter,
                            const QwtScaleMap& xMap,
                            const QwtScaleMap& yMap,
                            const QRectF& canvasRect,
                            const QwtInterval& boundingInterval,
                            int index,
                            const QPointF& sample) const;

    /// Draw a bar
    virtual void drawBar(QPainter*, int sampleIndex, const QPointF& sample, const QwtColumnRect&) const;

    /// Get column rectangle
    QwtColumnRect columnRect(const QwtScaleMap& xMap,
                             const QwtScaleMap& yMap,
                             const QRectF& canvasRect,
                             const QwtInterval& boundingInterval,
                             const QPointF& sample) const;

    /// Get legend data
    QList< QwtLegendData > legendData() const override;
    /// Get legend icon
    QwtGraphic legendIcon(int index, const QSizeF&) const override;

private:
    /// Initialize the bar chart
    void init();

    class PrivateData;
    PrivateData* m_data;
};

#endif
