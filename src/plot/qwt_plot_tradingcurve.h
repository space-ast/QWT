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

#ifndef QWT_PLOT_TRADING_CURVE_H
#define QWT_PLOT_TRADING_CURVE_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"

/**
 * \if ENGLISH
 * @brief QwtPlotTradingCurve illustrates movements in the price of a financial instrument over time
 * @details QwtPlotTradingCurve supports candlestick or bar ( OHLC ) charts
 *          that are used in the domain of technical analysis.
 *
 *          While the length ( height or width depending on orientation() )
 *          of each symbol depends on the corresponding OHLC sample the size
 *          of the other dimension can be controlled using:
 *
 *          - setSymbolExtent()
 *          - setSymbolMinWidth()
 *          - setSymbolMaxWidth()
 *
 *          The extent is a size in scale coordinates, so that the symbol width
 *          is increasing when the plot is zoomed in. Minimum/Maximum width
 *          is in widget coordinates independent from the zoom level.
 *          When setting the minimum and maximum to the same value, the width of
 *          the symbol is fixed.
 * \endif
 *
 * \if CHINESE
 * @brief QwtPlotTradingCurve 展示金融工具价格随时间的变化
 * @details QwtPlotTradingCurve 支持烛台图或柱状图（OHLC），
 *          这些图表用于技术分析领域。
 *
 *          每个符号的长度（高度或宽度，取决于方向）取决于相应的 OHLC 样本，
 *          另一个维度的大小可以通过以下方式控制：
 *
 *          - setSymbolExtent()
 *          - setSymbolMinWidth()
 *          - setSymbolMaxWidth()
 *
 *          范围是缩放坐标中的大小，因此当绘图放大时，符号宽度会增加。
 *          最小/最大宽度是在小部件坐标中，与缩放级别无关。
 *          当将最小值和最大值设置为相同值时，符号的宽度是固定的。
 * \endif
 */
class QWT_EXPORT QwtPlotTradingCurve : public QwtPlotSeriesItem, public QwtSeriesStore< QwtOHLCSample >
{
public:
    /**
     * \if ENGLISH
     * @brief Symbol styles
     * @details The default setting is QwtPlotSeriesItem::CandleStick.
     * @sa setSymbolStyle(), symbolStyle()
     * \endif
     *
     * \if CHINESE
     * @brief 符号样式
     * @details 默认设置是 QwtPlotSeriesItem::CandleStick。
     * @sa setSymbolStyle(), symbolStyle()
     * \endif
     */
    enum SymbolStyle
    {
        /// Nothing is displayed
        NoSymbol = -1,

        /**
         * \if ENGLISH
         * A line on the chart shows the price range (the highest and lowest
         * prices) over one unit of time, e.g. one day or one hour.
         * Tick marks project from each side of the line indicating the
         * opening and closing price.
         * \endif
         *
         * \if CHINESE
         * 图表上的一条线显示一个时间单位（例如一天或一小时）内的价格范围（最高和最低价格）。
         * 从线的两侧伸出的刻度标记表示开盘价和收盘价。
         * \endif
         */
        Bar,

        /**
         * \if ENGLISH
         * The range between opening/closing price are displayed as
         * a filled box. The fill brush depends on the direction of the
         * price movement. The box is connected to the highest/lowest
         * values by lines.
         * \endif
         *
         * \if CHINESE
         * 开盘/收盘价之间的范围显示为填充框。填充画刷取决于价格变动的方向。
         * 该框通过线条连接到最高/最低值。
         * \endif
         */
        CandleStick,

        /**
         * \if ENGLISH
         * SymbolTypes >= UserSymbol are displayed by drawUserSymbol(),
         * that needs to be overloaded and implemented in derived
         * curve classes.
         *
         * @sa drawUserSymbol()
         * \endif
         *
         * \if CHINESE
         * SymbolTypes >= UserSymbol 通过 drawUserSymbol() 显示，
         * 需要在派生的曲线类中重载和实现。
         *
         * @sa drawUserSymbol()
         * \endif
         */
        UserSymbol = 100
    };

    /**
     * \if ENGLISH
     * @brief Direction of a price movement
     * \endif
     *
     * \if CHINESE
     * @brief 价格变动的方向
     * \endif
     */
    enum Direction
    {
        /// The closing price is higher than the opening price
        Increasing,

        /// The closing price is lower than the opening price
        Decreasing
    };

    /**
     * \if ENGLISH
     * @brief Paint attributes
     * @details Attributes to modify the drawing algorithm.
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     *
     * \if CHINESE
     * @brief 绘制属性
     * @details 用于修改绘制算法的属性。
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /// Check if a symbol is on the plot canvas before painting it.
        ClipSymbols = 0x01
    };

    Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)

    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    explicit QwtPlotTradingCurve(const QString& title = QString());
    /**
     * \if ENGLISH
     * @brief Constructor with title
     * \endif
     */
    explicit QwtPlotTradingCurve(const QwtText& title);

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotTradingCurve();

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
    virtual int rtti() const override;

    /**
     * \if ENGLISH
     * @brief Set a paint attribute
     * \endif
     */
    void setPaintAttribute(PaintAttribute, bool on = true);
    /**
     * \if ENGLISH
     * @brief Test a paint attribute
     * \endif
     */
    bool testPaintAttribute(PaintAttribute) const;

    /**
     * \if ENGLISH
     * @brief Set the samples
     * \endif
     */
    void setSamples(const QVector< QwtOHLCSample >&);
    /**
     * \if ENGLISH
     * @brief Set the samples
     * \endif
     */
    void setSamples(QwtSeriesData< QwtOHLCSample >*);

    /**
     * \if ENGLISH
     * @brief Set the symbol style
     * \endif
     */
    void setSymbolStyle(SymbolStyle style);
    /**
     * \if ENGLISH
     * @brief Get the symbol style
     * \endif
     */
    SymbolStyle symbolStyle() const;

    /**
     * \if ENGLISH
     * @brief Set the symbol pen
     * \endif
     */
    void setSymbolPen(const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine);
    /**
     * \if ENGLISH
     * @brief Set the symbol pen
     * \endif
     */
    void setSymbolPen(const QPen&);
    /**
     * \if ENGLISH
     * @brief Get the symbol pen
     * \endif
     */
    QPen symbolPen() const;

    /**
     * \if ENGLISH
     * @brief Set the symbol brush
     * \endif
     */
    void setSymbolBrush(Direction, const QBrush&);
    /**
     * \if ENGLISH
     * @brief Get the symbol brush
     * \endif
     */
    QBrush symbolBrush(Direction) const;

    /**
     * \if ENGLISH
     * @brief Set the symbol extent
     * \endif
     */
    void setSymbolExtent(double);
    /**
     * \if ENGLISH
     * @brief Get the symbol extent
     * \endif
     */
    double symbolExtent() const;

    /**
     * \if ENGLISH
     * @brief Set the minimum symbol width
     * \endif
     */
    void setMinSymbolWidth(double);
    /**
     * \if ENGLISH
     * @brief Get the minimum symbol width
     * \endif
     */
    double minSymbolWidth() const;

    /**
     * \if ENGLISH
     * @brief Set the maximum symbol width
     * \endif
     */
    void setMaxSymbolWidth(double);
    /**
     * \if ENGLISH
     * @brief Get the maximum symbol width
     * \endif
     */
    double maxSymbolWidth() const;

    /**
     * \if ENGLISH
     * @brief Draw the series
     * \endif
     */
    virtual void drawSeries(QPainter*,
                            const QwtScaleMap& xMap,
                            const QwtScaleMap& yMap,
                            const QRectF& canvasRect,
                            int from,
                            int to) const override;

    /**
     * \if ENGLISH
     * @brief Get the bounding rectangle
     * \endif
     */
    virtual QRectF boundingRect() const override;

    /**
     * \if ENGLISH
     * @brief Get the legend icon
     * \endif
     */
    virtual QwtGraphic legendIcon(int index, const QSizeF&) const override;

protected:
    /**
     * \if ENGLISH
     * @brief Initialize the trading curve
     * \endif
     */
    void init();

    /**
     * \if ENGLISH
     * @brief Draw the symbols
     * \endif
     */
    virtual void
    drawSymbols(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const;

    /**
     * \if ENGLISH
     * @brief Draw a user symbol
     * \endif
     */
    virtual void
    drawUserSymbol(QPainter*, SymbolStyle, const QwtOHLCSample&, Qt::Orientation, bool inverted, double symbolWidth) const;

    /**
     * \if ENGLISH
     * @brief Draw a bar
     * \endif
     */
    void drawBar(QPainter*, const QwtOHLCSample&, Qt::Orientation, bool inverted, double width) const;

    /**
     * \if ENGLISH
     * @brief Draw a candlestick
     * \endif
     */
    void drawCandleStick(QPainter*, const QwtOHLCSample&, Qt::Orientation, double width) const;

    /**
     * \if ENGLISH
     * @brief Calculate the scaled symbol width
     * \endif
     */
    virtual double scaledSymbolWidth(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotTradingCurve::PaintAttributes)

#endif
