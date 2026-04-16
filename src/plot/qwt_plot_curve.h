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

#ifndef QWT_PLOT_CURVE_H
#define QWT_PLOT_CURVE_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"

#include <qstring.h>

class QwtScaleMap;
class QwtSymbol;
class QwtCurveFitter;
template< typename T >
class QwtSeriesData;
class QwtText;
class QPainter;
class QPolygonF;
class QPen;

/**
 * \if ENGLISH
 * @brief A plot item, that represents a series of points
 * @details A curve is the representation of a series of points in the x-y plane.
 *          It supports different display styles, interpolation ( f.e. spline )
 *          and symbols.
 * 
 * @par Usage
 * <dl><dt>a) Assign curve properties</dt>
 * <dd>When a curve is created, it is configured to draw black solid lines
 * with in QwtPlotCurve::Lines style and no symbols.
 * You can change this by calling
 * setPen(), setStyle() and setSymbol().</dd>
 * <dt>b) Connect/Assign data.</dt>
 * <dd>QwtPlotCurve gets its points using a QwtSeriesData object offering
 * a bridge to the real storage of the points ( like QAbstractItemModel ).
 * There are several convenience classes derived from QwtSeriesData, that also store
 * the points inside ( like QStandardItemModel ). QwtPlotCurve also offers
 * a couple of variations of setSamples(), that build QwtSeriesData objects from
 * arrays internally.</dd>
 * <dt>c) Attach the curve to a plot</dt>
 * <dd>See QwtPlotItem::attach()
 * </dd></dl>
 * 
 * @par Example:
 * see examples/bode
 * 
 * @sa QwtPointSeriesData, QwtSymbol, QwtScaleMap
 * \endif
 * 
 * \if CHINESE
 * @brief 表示一系列点的绘图项
 * @details 曲线是 x-y 平面中一系列点的表示。
 *          它支持不同的显示样式、插值（例如样条曲线）和符号。
 * 
 * @par 使用方法
 * <dl><dt>a) 分配曲线属性</dt>
 * <dd>创建曲线时，它被配置为绘制黑色实线，使用 QwtPlotCurve::Lines 样式，没有符号。
 * 您可以通过调用 setPen()、setStyle() 和 setSymbol() 来更改此设置。</dd>
 * <dt>b) 连接/分配数据</dt>
 * <dd>QwtPlotCurve 使用 QwtSeriesData 对象获取其点，该对象提供了到点的实际存储（如 QAbstractItemModel）的桥接。
 * 有几个从 QwtSeriesData 派生的便利类，它们也在内部存储点（如 QStandardItemModel）。
 * QwtPlotCurve 还提供了几种 setSamples() 变体，它们从内部数组构建 QwtSeriesData 对象。</dd>
 * <dt>c) 将曲线附加到绘图</dt>
 * <dd>请参阅 QwtPlotItem::attach()
 * </dd></dl>
 * 
 * @par 示例：
 * 见 examples/bode
 * 
 * @sa QwtPointSeriesData, QwtSymbol, QwtScaleMap
 * \endif
 */
class QWT_EXPORT QwtPlotCurve : public QwtPlotSeriesItem, public QwtSeriesStore< QPointF >
{
public:
    /**
     * \if ENGLISH
     * @brief Curve styles
     * @sa setStyle(), style()
     * \endif
     * 
     * \if CHINESE
     * @brief 曲线样式
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
        NoCurve = -1,

        /**
         * \if ENGLISH
         * Connect the points with straight lines. The lines might
         * be interpolated depending on the 'Fitted' attribute. Curve
         * fitting can be configured using setCurveFitter().
         * \endif
         * 
         * \if CHINESE
         * 用直线连接点。线条可能会根据 'Fitted' 属性进行插值。
         * 可以使用 setCurveFitter() 配置曲线拟合。
         * \endif
         */
        Lines,

        /**
         * \if ENGLISH
         * Draw vertical or horizontal sticks ( depending on the
         * orientation() ) from a baseline which is defined by setBaseline().
         * \endif
         * 
         * \if CHINESE
         * 从由 setBaseline() 定义的基线绘制垂直或水平线条（取决于 orientation()）。
         * \endif
         */
        Sticks,

        /**
         * \if ENGLISH
         * Connect the points with a step function. The step function
         * is drawn from the left to the right or vice versa,
         * depending on the QwtPlotCurve::Inverted attribute.
         * \endif
         * 
         * \if CHINESE
         * 用阶梯函数连接点。阶梯函数从左到右或从右到左绘制，
         * 取决于 QwtPlotCurve::Inverted 属性。
         * \endif
         */
        Steps,

        /**
         * \if ENGLISH
         * Draw dots at the locations of the data points. Note:
         * This is different from a dotted line (see setPen()), and faster
         * as a curve in QwtPlotCurve::NoStyle style and a symbol
         * painting a point.
         * \endif
         * 
         * \if CHINESE
         * 在数据点的位置绘制点。注意：
         * 这与虚线（见 setPen()）不同，并且比 QwtPlotCurve::NoStyle 样式的曲线和
         * 绘制点的符号更快。
         * \endif
         */
        Dots,

        /**
         * \if ENGLISH
         * Styles >= QwtPlotCurve::UserCurve are reserved for derived
         * classes of QwtPlotCurve that overload drawCurve() with
         * additional application specific curve types.
         * \endif
         * 
         * \if CHINESE
         * 样式 >= QwtPlotCurve::UserCurve 保留给 QwtPlotCurve 的派生类，
         * 这些类用额外的应用特定曲线类型重载 drawCurve()。
         * \endif
         */
        UserCurve = 100
    };

    /**
     * \if ENGLISH
     * @brief Attribute for drawing the curve
     * @sa setCurveAttribute(), testCurveAttribute(), curveFitter()
     * \endif
     * 
     * \if CHINESE
     * @brief 绘制曲线的属性
     * @sa setCurveAttribute(), testCurveAttribute(), curveFitter()
     * \endif
     */
    enum CurveAttribute
    {
        /**
         * \if ENGLISH
         * For QwtPlotCurve::Steps only.
         * Draws a step function from the right to the left.
         * \endif
         * 
         * \if CHINESE
         * 仅适用于 QwtPlotCurve::Steps。
         * 从右到左绘制阶梯函数。
         * \endif
         */
        Inverted = 0x01,

        /**
         * \if ENGLISH
         * Only in combination with QwtPlotCurve::Lines
         * A QwtCurveFitter tries to
         * interpolate/smooth the curve, before it is painted.
         * 
         * @note Curve fitting requires temporary memory
         * for calculating coefficients and additional points.
         * If painting in QwtPlotCurve::Fitted mode is slow it might be better
         * to fit the points, before they are passed to QwtPlotCurve.
         * \endif
         * 
         * \if CHINESE
         * 仅与 QwtPlotCurve::Lines 结合使用
         * QwtCurveFitter 尝试在绘制曲线之前对其进行插值/平滑。
         * 
         * @note 曲线拟合需要临时内存来计算系数和额外的点。
         * 如果在 QwtPlotCurve::Fitted 模式下绘制速度慢，
         * 最好在将点传递给 QwtPlotCurve 之前先拟合它们。
         * \endif
         */
        Fitted = 0x02
    };

    Q_DECLARE_FLAGS(CurveAttributes, CurveAttribute)

    /**
     * \if ENGLISH
     * @brief Attributes how to represent the curve on the legend
     * @sa setLegendAttribute(), testLegendAttribute(),
     *     QwtPlotItem::legendData(), legendIcon()
     * \endif
     * 
     * \if CHINESE
     * @brief 如何在图例上表示曲线的属性
     * @sa setLegendAttribute(), testLegendAttribute(),
     *     QwtPlotItem::legendData(), legendIcon()
     * \endif
     */

    enum LegendAttribute
    {
        /**
         * \if ENGLISH
         * QwtPlotCurve tries to find a color representing the curve
         * and paints a rectangle with it.
         * \endif
         * 
         * \if CHINESE
         * QwtPlotCurve 尝试找到代表曲线的颜色并用它绘制一个矩形。
         * \endif
         */
        LegendNoAttribute = 0x00,

        /**
         * \if ENGLISH
         * If the style() is not QwtPlotCurve::NoCurve a line
         * is painted with the curve pen().
         * \endif
         * 
         * \if CHINESE
         * 如果 style() 不是 QwtPlotCurve::NoCurve，则用曲线的 pen() 绘制一条线。
         * \endif
         */
        LegendShowLine = 0x01,

        /**
         * \if ENGLISH
         * If the curve has a valid symbol it is painted.
         * \endif
         * 
         * \if CHINESE
         * 如果曲线有有效的符号，则绘制它。
         * \endif
         */
        LegendShowSymbol = 0x02,

        /**
         * \if ENGLISH
         * If the curve has a brush a rectangle filled with the
         * curve brush() is painted.
         * \endif
         * 
         * \if CHINESE
         * 如果曲线有画笔，则绘制一个用曲线的 brush() 填充的矩形。
         * \endif
         */
        LegendShowBrush = 0x04
    };

    Q_DECLARE_FLAGS(LegendAttributes, LegendAttribute)

    /**
     * \if ENGLISH
     * @brief Attributes to modify the drawing algorithm
     * @details The default setting enables ClipPolygons | FilterPoints
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     * 
     * \if CHINESE
     * @brief 修改绘制算法的属性
     * @details 默认设置启用 ClipPolygons | FilterPoints
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * Clip polygons before painting them. In situations, where points
         * are far outside the visible area (f.e when zooming deep) this
         * might be a substantial improvement for the painting performance
         * \endif
         * 
         * \if CHINESE
         * 在绘制多边形之前裁剪它们。在点远在可见区域之外的情况下（例如深度缩放时），
         * 这可能会显著提高绘制性能
         * \endif
         */
        ClipPolygons = 0x01,

        /**
         * \if ENGLISH
         * Tries to reduce the data that has to be painted, by sorting out
         * duplicates, or paintings outside the visible area. Might have a
         * notable impact on curves with many close points.
         * Only a couple of very basic filtering algorithms are implemented.
         * \endif
         * 
         * \if CHINESE
         * 尝试通过排除重复项或可见区域外的绘制来减少必须绘制的数据。
         * 对于有许多接近点的曲线可能有显著影响。
         * 只实现了几个非常基本的过滤算法。
         * \endif
         */
        FilterPoints = 0x02,

        /**
         * \if ENGLISH
         * Minimize memory usage that is temporarily needed for the
         * translated points, before they get painted.
         * This might slow down the performance of painting
         * \endif
         * 
         * \if CHINESE
         * 最小化绘制前临时需要的转换点内存使用。
         * 这可能会减慢绘制性能
         * \endif
         */
        MinimizeMemory = 0x04,

        /**
         * \if ENGLISH
         * Render the points to a temporary image and paint the image.
         * This is a very special optimization for Dots style, when
         * having a huge amount of points.
         * With a reasonable number of points QPainter::drawPoints()
         * will be faster.
         * \endif
         * 
         * \if CHINESE
         * 将点渲染到临时图像并绘制该图像。
         * 这是对 Dots 样式的非常特殊的优化，当有大量点时。
         * 对于合理数量的点，QPainter::drawPoints() 会更快。
         * \endif
         */
        ImageBuffer = 0x08,

        /**
         * \if ENGLISH
         * More aggressive point filtering trying to filter out
         * intermediate points, accepting minor visual differences.
         * 
         * Has only an effect, when drawing the curve to a paint device
         * in integer coordinates ( f.e. all widgets on screen ) using the fact,
         * that consecutive points are often mapped to the same x or y coordinate.
         * Each chunk of samples mapped to the same coordinate can be reduced to
         * 4 points ( first, min, max last ).
         * 
         * In the worst case the polygon to be rendered will be 4 times the width
         * of the plot canvas.
         * 
         * The algorithm is very fast and effective for huge datasets, and can be used
         * inside a replot cycle.
         * 
         * @note Implemented for QwtPlotCurve::Lines only
         * @note As this algo replaces many small lines by a long one
         *      a nasty bug of the raster paint engine ( Qt 4.8, Qt 5.1 - 5.3 )
         *      becomes more dominant. For these versions the bug can be
         *      worked around by enabling the QwtPainter::polylineSplitting() mode.
         * \endif
         * 
         * \if CHINESE
         * 更积极的点过滤，尝试过滤掉中间点，接受 minor 视觉差异。
         * 
         * 只有在使用以下事实将曲线绘制到整数坐标的绘图设备（例如屏幕上的所有部件）时才有效：
         * 连续点通常映射到相同的 x 或 y 坐标。
         * 映射到相同坐标的每个样本块可以减少到 4 个点（第一个、最小值、最大值、最后一个）。
         * 
         * 在最坏情况下，要渲染的多边形将是绘图画布宽度的 4 倍。
         * 
         * 该算法对大型数据集非常快速有效，可用于重绘周期内。
         * 
         * @note 仅为 QwtPlotCurve::Lines 实现
         * @note 由于此算法用一条长线替换许多短线，
         *      光栅绘制引擎的一个严重错误（Qt 4.8，Qt 5.1 - 5.3）
         *      变得更加明显。对于这些版本，可以通过启用 QwtPainter::polylineSplitting() 模式来解决此错误。
         * \endif
         */
        FilterPointsAggressive = 0x10,
    };

    Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)

// Constructor
explicit QwtPlotCurve(const QString& title = QString());

    // Constructor with QwtText title
explicit QwtPlotCurve(const QwtText& title);

    // Destructor
virtual ~QwtPlotCurve();

    // Get the runtime type information
virtual int rtti() const override;

    // Set paint attribute
void setPaintAttribute(PaintAttribute, bool on = true);

    // Test paint attribute
bool testPaintAttribute(PaintAttribute) const;

    // Set legend attribute
void setLegendAttribute(LegendAttribute, bool on = true);

    // Test legend attribute
bool testLegendAttribute(LegendAttribute) const;

    // Set legend attributes
void setLegendAttributes(LegendAttributes);

    // Get legend attributes
LegendAttributes legendAttributes() const;

// Set raw samples from double arrays
void setRawSamples(const double* xData, const double* yData, int size);

    // Set raw samples from float arrays
void setRawSamples(const float* xData, const float* yData, int size);

    // Set raw samples from double array (y-axis only)
void setRawSamples(const double* yData, int size);

    // Set raw samples from float array (y-axis only)
void setRawSamples(const float* yData, int size);

    // Set samples from double arrays
void setSamples(const double* xData, const double* yData, int size);

    // Set samples from float arrays
void setSamples(const float* xData, const float* yData, int size);

    // Set samples from double array (y-axis only)
void setSamples(const double* yData, int size);

    // Set samples from float array (y-axis only)
void setSamples(const float* yData, int size);

    // Set samples from QVector<double> (y-axis only)
void setSamples(const QVector< double >& yData);

    // Set samples from QVector<float> (y-axis only)
void setSamples(const QVector< float >& yData);

    // Set samples from QVector<double> arrays
void setSamples(const QVector< double >& xData, const QVector< double >& yData);

    // Set samples from QVector<float> arrays
void setSamples(const QVector< float >& xData, const QVector< float >& yData);

    // Set samples from rvalue QVector<double> arrays
void setSamples(QVector< double >&& xData, QVector< double >&& yData);

    // Set samples from rvalue QVector<float> arrays
void setSamples(QVector< float >&& xData, QVector< float >&& yData);

    // Set samples from rvalue QVector<QPointF>
void setSamples(QVector< QPointF >&&);

    // Set samples from QVector<QPointF>
void setSamples(const QVector< QPointF >&);

    // Set samples from QwtSeriesData
void setSamples(QwtSeriesData< QPointF >*);

// Find the closest point to a position
virtual int closestPoint(const QPointF& pos, double* dist = nullptr) const;

    // Get minimum x value
double minXValue() const;

    // Get maximum x value
double maxXValue() const;

    // Get minimum y value
double minYValue() const;

    // Get maximum y value
double maxYValue() const;

    // Set curve attribute
void setCurveAttribute(CurveAttribute, bool on = true);

    // Test curve attribute
bool testCurveAttribute(CurveAttribute) const;

    // Set pen
void setPen(const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine);

    // Set pen
void setPen(const QPen&);

    // Get pen
const QPen& pen() const;

    // Set brush
void setBrush(const QBrush&);

    // Get brush
const QBrush& brush() const;

    // Set baseline
void setBaseline(double);

    // Get baseline
double baseline() const;

    // Set curve style
void setStyle(CurveStyle style);

    // Get curve style
CurveStyle style() const;

    // Set symbol
void setSymbol(QwtSymbol*);

    // Get symbol
const QwtSymbol* symbol() const;

    // Set curve fitter
void setCurveFitter(QwtCurveFitter*);

    // Get curve fitter
QwtCurveFitter* curveFitter() const;

    // Draw the series
virtual void drawSeries(QPainter*,
                            const QwtScaleMap& xMap,
                            const QwtScaleMap& yMap,
                            const QRectF& canvasRect,
                            int from,
                            int to) const override;

    // Get the legend icon
virtual QwtGraphic legendIcon(int index, const QSizeF&) const override;

protected:
    /**
     * \if ENGLISH
     * @brief Initialize the curve
     * \endif
     */
void init();

    /**
     * \if ENGLISH
     * @brief Draw the curve
     * \endif
     */
virtual void drawCurve(QPainter*,
                           int style,
                           const QwtScaleMap& xMap,
                           const QwtScaleMap& yMap,
                           const QRectF& canvasRect,
                           int from,
                           int to) const;

    /**
     * \if ENGLISH
     * @brief Draw symbols
     * \endif
     */
virtual void drawSymbols(QPainter*,
                             const QwtSymbol&,
                             const QwtScaleMap& xMap,
                             const QwtScaleMap& yMap,
                             const QRectF& canvasRect,
                             int from,
                             int to) const;

    /**
     * \if ENGLISH
     * @brief Draw lines
     * \endif
     */
virtual void
    drawLines(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const;

    /**
     * \if ENGLISH
     * @brief Draw sticks
     * \endif
     */
virtual void
    drawSticks(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const;

    /**
     * \if ENGLISH
     * @brief Draw dots
     * \endif
     */
virtual void
    drawDots(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const;

    /**
     * \if ENGLISH
     * @brief Draw steps
     * \endif
     */
virtual void
    drawSteps(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const;

    /**
     * \if ENGLISH
     * @brief Fill the curve
     * \endif
     */
virtual void fillCurve(QPainter*, const QwtScaleMap&, const QwtScaleMap&, const QRectF& canvasRect, QPolygonF&) const;

    /**
     * \if ENGLISH
     * @brief Close the polyline
     * \endif
     */
void closePolyline(QPainter*, const QwtScaleMap&, const QwtScaleMap&, QPolygonF&) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

//! boundingRect().left()
inline double QwtPlotCurve::minXValue() const
{
    return boundingRect().left();
}

//! boundingRect().right()
inline double QwtPlotCurve::maxXValue() const
{
    return boundingRect().right();
}

//! boundingRect().top()
inline double QwtPlotCurve::minYValue() const
{
    return boundingRect().top();
}

//! boundingRect().bottom()
inline double QwtPlotCurve::maxYValue() const
{
    return boundingRect().bottom();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotCurve::PaintAttributes)
Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotCurve::LegendAttributes)
Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotCurve::CurveAttributes)

#endif
