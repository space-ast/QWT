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

#ifndef QWT_PLOT_INTERVAL_CURVE_H
#define QWT_PLOT_INTERVAL_CURVE_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"

class QwtIntervalSymbol;
template< typename T > class QwtSeriesData;

/**
 * \if ENGLISH
 * @brief QwtPlotIntervalCurve represents a series of samples, where each value
 *        is associated with an interval ( \f$[y1,y2] = f(x)\f$ )
 * @details The representation depends on the style() and an optional symbol()
 *          that is displayed for each interval. QwtPlotIntervalCurve might be used
 *          to display error bars or the area between 2 curves.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotIntervalCurve 表示一系列样本，其中每个值都与一个区间相关联 ( \f$[y1,y2] = f(x)\f$ )
 * @details 表示方式取决于 style() 和为每个区间显示的可选 symbol()。
 *          QwtPlotIntervalCurve 可用于显示误差条或两条曲线之间的区域。
 * \endif
 */
class QWT_EXPORT QwtPlotIntervalCurve
    : public QwtPlotSeriesItem
    , public QwtSeriesStore< QwtIntervalSample >
{
  public:
    /**
     * \if ENGLISH
     * @brief Curve styles
     * @details The default setting is QwtPlotIntervalCurve::Tube.
     * @sa setStyle(), style()
     * \endif
     * 
     * \if CHINESE
     * @brief 曲线样式
     * @details 默认设置是 QwtPlotIntervalCurve::Tube。
     * @sa setStyle(), style()
     * \endif
     */
    enum CurveStyle
    {
        /**
         * \if ENGLISH
         * Don't draw a curve. Note: This doesn't affect the symbols.
         * \endif
         * 
         * \if CHINESE
         * 不绘制曲线。注意：这不会影响符号。
         * \endif
         */
        NoCurve,

        /**
         * \if ENGLISH
         * Build 2 curves from the upper and lower limits of the intervals
         * and draw them with the pen(). The area between the curves is
         * filled with the brush().
         * \endif
         * 
         * \if CHINESE
         * 从区间的上限和下限构建 2 条曲线，并用 pen() 绘制它们。
         * 曲线之间的区域用 brush() 填充。
         * \endif
         */
        Tube,

        /**
         * \if ENGLISH
         * Styles >= QwtPlotIntervalCurve::UserCurve are reserved for derived
         * classes that overload drawSeries() with
         * additional application specific curve types.
         * \endif
         * 
         * \if CHINESE
         * 样式 >= QwtPlotIntervalCurve::UserCurve 保留给派生类使用，
         * 这些派生类用额外的应用特定曲线类型重载 drawSeries()。
         * \endif
         */
        UserCurve = 100
    };

    /**
     * \if ENGLISH
     * @brief Attributes to modify the drawing algorithm
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     * 
     * \if CHINESE
     * @brief 修改绘制算法的属性
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * Clip polygons before painting them. In situations, where points
         * are far outside the visible area (f.e when zooming deep) this
         * might be a substantial improvement for the painting performance.
         * \endif
         * 
         * \if CHINESE
         * 在绘制多边形之前对其进行裁剪。在点远在可见区域之外的情况下
         * （例如深度缩放时），这可能会显著提高绘制性能。
         * \endif
         */
        ClipPolygons = 0x01,

        /// Check if a symbol is on the plot canvas before painting it
        ClipSymbol   = 0x02
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
explicit QwtPlotIntervalCurve( const QString& title = QString() );

    /**
     * \if ENGLISH
     * @brief Constructor with QwtText title
     * \endif
     */
explicit QwtPlotIntervalCurve( const QwtText& title );

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
virtual ~QwtPlotIntervalCurve();

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
virtual int rtti() const override;

    /**
     * \if ENGLISH
     * @brief Set paint attribute
     * \endif
     */
void setPaintAttribute( PaintAttribute, bool on = true );

    /**
     * \if ENGLISH
     * @brief Test paint attribute
     * \endif
     */
bool testPaintAttribute( PaintAttribute ) const;

    /**
     * \if ENGLISH
     * @brief Set samples from a vector
     * \endif
     */
void setSamples( const QVector< QwtIntervalSample >& );

    /**
     * \if ENGLISH
     * @brief Set samples from a series data
     * \endif
     */
void setSamples( QwtSeriesData< QwtIntervalSample >* );

    /**
     * \if ENGLISH
     * @brief Set pen
     * \endif
     */
void setPen( const QColor&,
    qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    /**
     * \if ENGLISH
     * @brief Set pen
     * \endif
     */
void setPen( const QPen& );

    /**
     * \if ENGLISH
     * @brief Get pen
     * \endif
     */
const QPen& pen() const;

    /**
     * \if ENGLISH
     * @brief Set brush
     * \endif
     */
void setBrush( const QBrush& );

    /**
     * \if ENGLISH
     * @brief Get brush
     * \endif
     */
const QBrush& brush() const;

    /**
     * \if ENGLISH
     * @brief Set curve style
     * \endif
     */
void setStyle( CurveStyle style );

    /**
     * \if ENGLISH
     * @brief Get curve style
     * \endif
     */
CurveStyle style() const;

    /**
     * \if ENGLISH
     * @brief Set symbol
     * \endif
     */
void setSymbol( const QwtIntervalSymbol* );

    /**
     * \if ENGLISH
     * @brief Get symbol
     * \endif
     */
const QwtIntervalSymbol* symbol() const;

    /**
     * \if ENGLISH
     * @brief Draw the series
     * \endif
     */
virtual void drawSeries( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const override;

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
virtual QwtGraphic legendIcon(
    int index, const QSizeF& ) const override;

  protected:

    /**
     * \if ENGLISH
     * @brief Initialize the curve
     * \endif
     */
void init();

    /**
     * \if ENGLISH
     * @brief Draw the tube
     * \endif
     */
virtual void drawTube( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const;

    /**
     * \if ENGLISH
     * @brief Draw the symbols
     * \endif
     */
virtual void drawSymbols( QPainter*, const QwtIntervalSymbol&,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotIntervalCurve::PaintAttributes )

#endif
