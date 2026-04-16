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

#include "qwt_plot_curve.h"
#include "qwt_point_data.h"
#include "qwt_math.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_spline_curve_fitter.h"
#include "qwt_symbol.h"
#include "qwt_point_mapper.h"
#include "qwt_text.h"
#include "qwt_graphic.h"

#include <qpainter.h>
#include <qpainterpath.h>

static inline QRectF qwtIntersectedClipRect(const QRectF& rect, QPainter* painter)
{
    QRectF clipRect = rect;
    if (painter->hasClipping())
        clipRect &= painter->clipBoundingRect();

    return clipRect;
}

static void qwtUpdateLegendIconSize(QwtPlotCurve* curve)
{
    if (curve->symbol() && curve->testLegendAttribute(QwtPlotCurve::LegendShowSymbol)) {
        QSize sz = curve->symbol()->boundingRect().size();
        sz += QSize(2, 2);  // margin

        if (curve->testLegendAttribute(QwtPlotCurve::LegendShowLine)) {
            // Avoid, that the line is completely covered by the symbol

            int w = qwtCeil(1.5 * sz.width());
            if (w % 2)
                w++;

            sz.setWidth(qMax(8, w));
        }

        curve->setLegendIconSize(sz);
    }
}

class QwtPlotCurve::PrivateData
{
public:
    PrivateData()
        : style(QwtPlotCurve::Lines)
        , baseline(0.0)
        , symbol(nullptr)
        , pen(Qt::black)
        , paintAttributes(QwtPlotCurve::ClipPolygons | QwtPlotCurve::FilterPoints)
    {
        curveFitter = new QwtSplineCurveFitter;
    }

    ~PrivateData()
    {
        delete symbol;
        delete curveFitter;
    }

    QwtPlotCurve::CurveStyle style;
    double baseline;

    const QwtSymbol* symbol;
    QwtCurveFitter* curveFitter;

    QPen pen;
    QBrush brush;

    QwtPlotCurve::CurveAttributes attributes;
    QwtPlotCurve::PaintAttributes paintAttributes;

    QwtPlotCurve::LegendAttributes legendAttributes;
};

/**
 * \if ENGLISH
 * @brief Constructor with QwtText title
 * @param[in] title Title of the curve
 * \endif
 *
 * \if CHINESE
 * @brief 带QwtText标题的构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPlotCurve::QwtPlotCurve(const QwtText& title) : QwtPlotSeriesItem(title)
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor with QString title
 * @param[in] title Title of the curve
 * \endif
 *
 * \if CHINESE
 * @brief QString标题的构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPlotCurve::QwtPlotCurve(const QString& title) : QwtPlotSeriesItem(QwtText(title))
{
    init();
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPlotCurve::~QwtPlotCurve()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Initialize internal members
 * @details Sets default attributes and creates internal data structures.
 * \endif
 *
 * \if CHINESE
 * @brief 初始化内部成员
 * @details 设置默认属性并创建内部数据结构。
 * \endif
 */
void QwtPlotCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend);
    setItemAttribute(QwtPlotItem::AutoScale);

    m_data = new PrivateData;
    setData(new QwtPointSeriesData());

    setZ(20.0);
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotCurve
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotCurve
 * \endif
 */
int QwtPlotCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotCurve;
}

/**
 * \if ENGLISH
 * @brief Set paint attribute
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 设置绘制属性
 * @param[in] attribute 绘制属性
 * @param[in] on 开启/关闭
 * @sa testPaintAttribute()
 * \endif
 */
void QwtPlotCurve::setPaintAttribute(PaintAttribute attribute, bool on)
{
    if (on)
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/**
 * \if ENGLISH
 * @brief Test paint attribute
 * @param[in] attribute Paint attribute
 * @return True when attribute is enabled
 * @sa setPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试绘制属性
 * @param[in] attribute 绘制属性
 * @return 属性启用时返回true
 * @sa setPaintAttribute()
 * \endif
 */
bool QwtPlotCurve::testPaintAttribute(PaintAttribute attribute) const
{
    return (m_data->paintAttributes & attribute);
}

/**
 * \if ENGLISH
 * @brief Set legend attribute
 * @param[in] attribute Legend attribute
 * @param[in] on On/Off
 * @sa testLegendAttribute(), legendIcon()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例属性
 * @param[in] attribute 图例属性
 * @param[in] on 开启/关闭
 * @sa testLegendAttribute(), legendIcon()
 * \endif
 */
void QwtPlotCurve::setLegendAttribute(LegendAttribute attribute, bool on)
{
    if (on != testLegendAttribute(attribute)) {
        if (on)
            m_data->legendAttributes |= attribute;
        else
            m_data->legendAttributes &= ~attribute;

        qwtUpdateLegendIconSize(this);
        legendChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test legend attribute
 * @param[in] attribute Legend attribute
 * @return True when attribute is enabled
 * @sa setLegendAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试图例属性
 * @param[in] attribute 图例属性
 * @return 属性启用时返回true
 * @sa setLegendAttribute()
 * \endif
 */
bool QwtPlotCurve::testLegendAttribute(LegendAttribute attribute) const
{
    return (m_data->legendAttributes & attribute);
}

/**
 * \if ENGLISH
 * @brief Set legend attributes
 * @param[in] attributes Legend attributes
 * @sa setLegendAttribute(), legendIcon()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例属性集合
 * @param[in] attributes 图例属性集合
 * @sa setLegendAttribute(), legendIcon()
 * \endif
 */
void QwtPlotCurve::setLegendAttributes(LegendAttributes attributes)
{
    if (attributes != m_data->legendAttributes) {
        m_data->legendAttributes = attributes;

        qwtUpdateLegendIconSize(this);
        legendChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get legend attributes
 * @return Attributes for drawing the legend icon
 * @sa setLegendAttributes(), testLegendAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例属性
 * @return 用于绘制图例图标属性
 * @sa setLegendAttributes(), testLegendAttribute()
 * \endif
 */
QwtPlotCurve::LegendAttributes QwtPlotCurve::legendAttributes() const
{
    return m_data->legendAttributes;
}

/**
 * \if ENGLISH
 * @brief Set the curve's drawing style
 * @param[in] style Curve style
 * @sa style()
 * \endif
 *
 * \if CHINESE
 * @brief 设置曲线的绘制样式
 * @param[in] style 曲线样式
 * @sa style()
 * \endif
 */
void QwtPlotCurve::setStyle(CurveStyle style)
{
    if (style != m_data->style) {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the curve's drawing style
 * @return Style of the curve
 * @sa setStyle()
 * \endif
 *
 * \if CHINESE
 * @brief 获取曲线的绘制样式
 * @return 曲线样式
 * @sa setStyle()
 * \endif
 */
QwtPlotCurve::CurveStyle QwtPlotCurve::style() const
{
    return m_data->style;
}

/**
 * \if ENGLISH
 * @brief Assign a symbol
 * @details The curve will take the ownership of the symbol, hence the previously
 *          set symbol will be deleted by setting a new one. If symbol is nullptr
 *          no symbol will be drawn.
 * @param[in] symbol Symbol
 * @sa symbol()
 * \endif
 *
 * \if CHINESE
 * @brief 分配符号
 * @details 曲线将拥有符号的所有权，因此设置新符号时之前设置的符号将被删除。
 *          如果 symbol 为 nullptr，则不会绘制符号。
 * @param[in] symbol 符号
 * @sa symbol()
 * \endif
 */
void QwtPlotCurve::setSymbol(QwtSymbol* symbol)
{
    if (symbol != m_data->symbol) {
        delete m_data->symbol;
        m_data->symbol = symbol;

        qwtUpdateLegendIconSize(this);

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the current symbol
 * @return Current symbol or nullptr when no symbol has been assigned
 * @sa setSymbol()
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前符号
 * @return 当前符号或 nullptr（未分配符号时）
 * @sa setSymbol()
 * \endif
 */
const QwtSymbol* QwtPlotCurve::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并分配画笔
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），使其非装饰性
 *          （见 QPen::isCosmetic()）。此方法用于隐藏此不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotCurve::setPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setPen(QPen(color, width, style));
}

/**
 * \if ENGLISH
 * @brief Assign a pen
 * @param[in] pen New pen
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 分配画笔
 * @param[in] pen 新画笔
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotCurve::setPen(const QPen& pen)
{
    if (pen != m_data->pen) {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen used to draw the lines
 * @return Pen used to draw the lines
 * @sa setPen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制线条的画笔
 * @return 用于绘制线条的画笔
 * @sa setPen(), brush()
 * \endif
 */
const QPen& QwtPlotCurve::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Assign a brush
 * @details In case of brush.style() != QBrush::NoBrush and style() != QwtPlotCurve::Sticks,
 *          the area between the curve and the baseline will be filled.
 *          In case !brush.color().isValid() the area will be filled by pen.color().
 *          The fill algorithm simply connects the first and the last curve point to the baseline.
 *          So the curve data has to be sorted (ascending or descending).
 * @param[in] brush New brush
 * @sa brush(), setBaseline(), baseline()
 * \endif
 *
 * \if CHINESE
 * @brief 分配画刷
 * @details 如果 brush.style() != QBrush::NoBrush 且 style() != QwtPlotCurve::Sticks，
 *          曲线和基线之间的区域将被填充。如果 !brush.color().isValid()，
 *          区域将由 pen.color() 填充。填充算法简单地连接第一个和最后一个曲线点到基线。
 *          因此曲线数据需要排序（升序或降序）。
 * @param[in] brush 新画刷
 * @sa brush(), setBaseline(), baseline()
 * \endif
 */
void QwtPlotCurve::setBrush(const QBrush& brush)
{
    if (brush != m_data->brush) {
        m_data->brush = brush;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the brush used to fill the area between lines and the baseline
 * @return Brush used to fill the area between lines and the baseline
 * @sa setBrush(), setBaseline(), baseline()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于填充线条和基线之间区域的画刷
 * @return 用于填充线条和基线之间区域的画刷
 * @sa setBrush(), setBaseline(), baseline()
 * \endif
 */
const QBrush& QwtPlotCurve::brush() const
{
    return m_data->brush;
}

/**
 * \if ENGLISH
 * @brief Draw an interval of the curve
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the curve will be painted to its last point.
 * @sa drawCurve(), drawSymbols()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制曲线的一个区间
 * @param[in] painter 绘图器
 * @param[in] xMap 将x值映射到像素坐标
 * @param[in] yMap 将y值映射到像素坐标
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引。如果to < 0，曲线将绘制到最后一个点。
 * @sa drawCurve(), drawSymbols()
 * \endif
 */
void QwtPlotCurve::drawSeries(QPainter* painter,
                              const QwtScaleMap& xMap,
                              const QwtScaleMap& yMap,
                              const QRectF& canvasRect,
                              int from,
                              int to) const
{
    const size_t numSamples = dataSize();

    if (!painter || numSamples <= 0)
        return;

    if (to < 0)
        to = numSamples - 1;

    if (qwtVerifyRange(numSamples, from, to) > 0) {
        painter->save();
        painter->setPen(m_data->pen);

        /*
           Qt 4.0.0 is slow when drawing lines, but it's even
           slower when the painter has a brush. So we don't
           set the brush before we really need it.
         */

        drawCurve(painter, m_data->style, xMap, yMap, canvasRect, from, to);
        painter->restore();

        if (m_data->symbol && (m_data->symbol->style() != QwtSymbol::NoSymbol)) {
            painter->save();
            drawSymbols(painter, *m_data->symbol, xMap, yMap, canvasRect, from, to);
            painter->restore();
        }
    }
}

/**
 * \if ENGLISH
 * @brief Draw the line part (without symbols) of a curve interval
 * @param[in] painter Painter
 * @param[in] style Curve style, see QwtPlotCurve::CurveStyle
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa draw(), drawDots(), drawLines(), drawSteps(), drawSticks()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制曲线区间的线条部分（不含符号）
 * @param[in] painter 绘图器
 * @param[in] style 曲线样式，见 QwtPlotCurve::CurveStyle
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引
 * @sa draw(), drawDots(), drawLines(), drawSteps(), drawSticks()
 * \endif
 */
void QwtPlotCurve::drawCurve(QPainter* painter,
                             int style,
                             const QwtScaleMap& xMap,
                             const QwtScaleMap& yMap,
                             const QRectF& canvasRect,
                             int from,
                             int to) const
{
    switch (style) {
    case Lines:
        if (testCurveAttribute(Fitted)) {
            // we always need the complete
            // curve for fitting
            from = 0;
            to   = dataSize() - 1;
        }
        drawLines(painter, xMap, yMap, canvasRect, from, to);
        break;
    case Sticks:
        drawSticks(painter, xMap, yMap, canvasRect, from, to);
        break;
    case Steps:
        drawSteps(painter, xMap, yMap, canvasRect, from, to);
        break;
    case Dots:
        drawDots(painter, xMap, yMap, canvasRect, from, to);
        break;
    case NoCurve:
    default:
        break;
    }
}

/**
 * \if ENGLISH
 * @brief Draw lines
 * @details If the CurveAttribute Fitted is enabled a QwtCurveFitter tries
 *          to interpolate/smooth the curve, before it is painted.
 * @param[in] painter Painter
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa setCurveAttribute(), setCurveFitter(), draw(), drawLines(), drawDots(), drawSteps(), drawSticks()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制线条
 * @details 如果 CurveAttribute Fitted 已启用，QwtCurveFitter 将在绘制之前
 *          尝试对曲线进行插值/平滑处理。
 * @param[in] painter 绘图器
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引
 * @sa setCurveAttribute(), setCurveFitter(), draw(), drawLines(), drawDots(), drawSteps(), drawSticks()
 * \endif
 */
void QwtPlotCurve::drawLines(QPainter* painter,
                             const QwtScaleMap& xMap,
                             const QwtScaleMap& yMap,
                             const QRectF& canvasRect,
                             int from,
                             int to) const
{
    if (from > to)
        return;

    const bool doFit   = (m_data->attributes & Fitted) && m_data->curveFitter;
    const bool doAlign = !doFit && QwtPainter::roundingAlignment(painter);
    const bool doFill  = (m_data->brush.style() != Qt::NoBrush) && (m_data->brush.color().alpha() > 0);

    QRectF clipRect;
    if (m_data->paintAttributes & ClipPolygons) {
        clipRect = qwtIntersectedClipRect(canvasRect, painter);

        const qreal pw = QwtPainter::effectivePenWidth(painter->pen());
        clipRect       = clipRect.adjusted(-pw, -pw, pw, pw);
    }

    QwtPointMapper mapper;

    if (doAlign) {
        mapper.setFlag(QwtPointMapper::RoundPoints, true);
        mapper.setFlag(QwtPointMapper::WeedOutIntermediatePoints, testPaintAttribute(FilterPointsAggressive));
    }

    mapper.setFlag(QwtPointMapper::WeedOutPoints,
                   testPaintAttribute(FilterPoints) || testPaintAttribute(FilterPointsAggressive));

    mapper.setBoundingRect(canvasRect);

    QPolygonF polyline = mapper.toPolygonF(xMap, yMap, data(), from, to);

    if (doFill) {
        if (doFit) {
            // it might be better to extend and draw the curvePath, but for
            // the moment we keep an implementation, where we translate the
            // path back to a polyline.

            polyline = m_data->curveFitter->fitCurve(polyline);
        }

        if (painter->pen().style() != Qt::NoPen) {
            // here we are wasting memory for the filled copy,
            // do polygon clipping twice etc .. TODO

            QPolygonF filled = polyline;
            fillCurve(painter, xMap, yMap, canvasRect, filled);
            filled.clear();

            if (m_data->paintAttributes & ClipPolygons)
                QwtClipper::clipPolygonF(clipRect, polyline, false);

            QwtPainter::drawPolyline(painter, polyline);
        } else {
            fillCurve(painter, xMap, yMap, canvasRect, polyline);
        }
    } else {
        if (testPaintAttribute(ClipPolygons)) {
            QwtClipper::clipPolygonF(clipRect, polyline, false);
        }

        if (doFit) {
            if (m_data->curveFitter->mode() == QwtCurveFitter::Path) {
                const QPainterPath curvePath = m_data->curveFitter->fitCurvePath(polyline);

                painter->drawPath(curvePath);
            } else {
                polyline = m_data->curveFitter->fitCurve(polyline);
                QwtPainter::drawPolyline(painter, polyline);
            }
        } else {
            QwtPainter::drawPolyline(painter, polyline);
        }
    }
}

/**
 * \if ENGLISH
 * @brief Draw sticks
 * @param[in] painter Painter
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa draw(), drawCurve(), drawDots(), drawLines(), drawSteps()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制棒状图
 * @param[in] painter 绘图器
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引
 * @sa draw(), drawCurve(), drawDots(), drawLines(), drawSteps()
 * \endif
 */
void QwtPlotCurve::drawSticks(QPainter* painter,
                              const QwtScaleMap& xMap,
                              const QwtScaleMap& yMap,
                              const QRectF& canvasRect,
                              int from,
                              int to) const
{
    Q_UNUSED(canvasRect)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);

    const bool doAlign = QwtPainter::roundingAlignment(painter);

    double x0 = xMap.transform(m_data->baseline);
    double y0 = yMap.transform(m_data->baseline);
    if (doAlign) {
        x0 = qRound(x0);
        y0 = qRound(y0);
    }

    const Qt::Orientation o = orientation();

    const QwtSeriesData< QPointF >* series = data();

    for (int i = from; i <= to; i++) {
        const QPointF sample = series->sample(i);
        double xi            = xMap.transform(sample.x());
        double yi            = yMap.transform(sample.y());
        if (doAlign) {
            xi = qRound(xi);
            yi = qRound(yi);
        }

        if (o == Qt::Horizontal)
            QwtPainter::drawLine(painter, x0, yi, xi, yi);
        else
            QwtPainter::drawLine(painter, xi, y0, xi, yi);
    }

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Draw dots
 * @param[in] painter Painter
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa draw(), drawCurve(), drawSticks(), drawLines(), drawSteps()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制点
 * @param[in] painter 绘图器
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引
 * @sa draw(), drawCurve(), drawSticks(), drawLines(), drawSteps()
 * \endif
 */
void QwtPlotCurve::drawDots(QPainter* painter,
                            const QwtScaleMap& xMap,
                            const QwtScaleMap& yMap,
                            const QRectF& canvasRect,
                            int from,
                            int to) const
{
    const QColor color = painter->pen().color();

    if (painter->pen().style() == Qt::NoPen || color.alpha() == 0) {
        return;
    }

    const bool doFill  = (m_data->brush.style() != Qt::NoBrush) && (m_data->brush.color().alpha() > 0);
    const bool doAlign = QwtPainter::roundingAlignment(painter);

    QwtPointMapper mapper;
    mapper.setBoundingRect(canvasRect);
    mapper.setFlag(QwtPointMapper::RoundPoints, doAlign);

    if (m_data->paintAttributes & FilterPoints) {
        if ((color.alpha() == 255) && !(painter->renderHints() & QPainter::Antialiasing)) {
            mapper.setFlag(QwtPointMapper::WeedOutPoints, true);
        }
    }

    if (doFill) {
        mapper.setFlag(QwtPointMapper::WeedOutPoints, false);

        QPolygonF points = mapper.toPolygonF(xMap, yMap, data(), from, to);

        QwtPainter::drawPoints(painter, points);
        fillCurve(painter, xMap, yMap, canvasRect, points);
    } else if (m_data->paintAttributes & ImageBuffer) {
        const QImage image = mapper.toImage(xMap,
                                            yMap,
                                            data(),
                                            from,
                                            to,
                                            m_data->pen,
                                            painter->testRenderHint(QPainter::Antialiasing),
                                            renderThreadCount());

        painter->drawImage(canvasRect.toAlignedRect(), image);
    } else if (m_data->paintAttributes & MinimizeMemory) {
        const QwtSeriesData< QPointF >* series = data();

        for (int i = from; i <= to; i++) {
            const QPointF sample = series->sample(i);

            double xi = xMap.transform(sample.x());
            double yi = yMap.transform(sample.y());

            if (doAlign) {
                xi = qRound(xi);
                yi = qRound(yi);
            }

            QwtPainter::drawPoint(painter, QPointF(xi, yi));
        }
    } else {
        if (doAlign) {
            const QPolygon points = mapper.toPoints(xMap, yMap, data(), from, to);

            QwtPainter::drawPoints(painter, points);
        } else {
            const QPolygonF points = mapper.toPointsF(xMap, yMap, data(), from, to);

            QwtPainter::drawPoints(painter, points);
        }
    }
}

/**
 * \if ENGLISH
 * @brief Draw step function
 * @details The direction of the steps depends on Inverted attribute.
 * @param[in] painter Painter
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa CurveAttribute, setCurveAttribute(), draw(), drawCurve(), drawDots(), drawLines(), drawSticks()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制阶梯函数
 * @details 阶梯的方向取决于 Inverted 属性。
 * @param[in] painter 绘图器
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引
 * @sa CurveAttribute, setCurveAttribute(), draw(), drawCurve(), drawDots(), drawLines(), drawSticks()
 * \endif
 */
void QwtPlotCurve::drawSteps(QPainter* painter,
                             const QwtScaleMap& xMap,
                             const QwtScaleMap& yMap,
                             const QRectF& canvasRect,
                             int from,
                             int to) const
{
    const bool doAlign = QwtPainter::roundingAlignment(painter);

    QPolygonF polygon(2 * (to - from) + 1);
    QPointF* points = polygon.data();

    bool inverted = orientation() == Qt::Vertical;
    if (m_data->attributes & Inverted)
        inverted = !inverted;

    const QwtSeriesData< QPointF >* series = data();

    int i, ip;
    for (i = from, ip = 0; i <= to; i++, ip += 2) {
        const QPointF sample = series->sample(i);
        double xi            = xMap.transform(sample.x());
        double yi            = yMap.transform(sample.y());
        if (doAlign) {
            xi = qRound(xi);
            yi = qRound(yi);
        }

        if (ip > 0) {
            const QPointF& p0 = points[ ip - 2 ];
            QPointF& p        = points[ ip - 1 ];

            if (inverted) {
                p.rx() = p0.x();
                p.ry() = yi;
            } else {
                p.rx() = xi;
                p.ry() = p0.y();
            }
        }

        points[ ip ].rx() = xi;
        points[ ip ].ry() = yi;
    }

    if (m_data->paintAttributes & ClipPolygons) {
        QRectF clipRect = qwtIntersectedClipRect(canvasRect, painter);

        const qreal pw = QwtPainter::effectivePenWidth(painter->pen());
        clipRect       = clipRect.adjusted(-pw, -pw, pw, pw);

        const QPolygonF clipped = QwtClipper::clippedPolygonF(clipRect, polygon, false);

        QwtPainter::drawPolyline(painter, clipped);
    } else {
        QwtPainter::drawPolyline(painter, polygon);
    }

    if (m_data->brush.style() != Qt::NoBrush)
        fillCurve(painter, xMap, yMap, canvasRect, polygon);
}

/**
 * \if ENGLISH
 * @brief Set curve attribute
 * @param[in] attribute Curve attribute
 * @param[in] on On/Off
 * @sa testCurveAttribute(), setCurveFitter()
 * \endif
 *
 * \if CHINESE
 * @brief 设置曲线属性
 * @param[in] attribute 曲线属性
 * @param[in] on 开启/关闭
 * @sa testCurveAttribute(), setCurveFitter()
 * \endif
 */
void QwtPlotCurve::setCurveAttribute(CurveAttribute attribute, bool on)
{
    if (bool(m_data->attributes & attribute) == on)
        return;

    if (on)
        m_data->attributes |= attribute;
    else
        m_data->attributes &= ~attribute;

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Test curve attribute
 * @param[in] attribute Curve attribute
 * @return True if attribute is enabled
 * @sa setCurveAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试曲线属性
 * @param[in] attribute 曲线属性
 * @return 属性启用时返回true
 * @sa setCurveAttribute()
 * \endif
 */
bool QwtPlotCurve::testCurveAttribute(CurveAttribute attribute) const
{
    return m_data->attributes & attribute;
}

/**
 * \if ENGLISH
 * @brief Assign a curve fitter
 * @details The curve fitter "smooths" the curve points, when the Fitted
 *          CurveAttribute is set. setCurveFitter(nullptr) also disables curve fitting.
 *          The curve fitter operates on the translated points (= widget coordinates)
 *          to be functional for logarithmic scales. Obviously this is less performant
 *          for fitting algorithms, that reduce the number of points.
 *          For situations, where curve fitting is used to improve the performance
 *          of painting huge series of points it might be better to execute the fitter
 *          on the curve points once and to cache the result in the QwtSeriesData object.
 * @param[in] curveFitter Curve fitter
 * @sa Fitted
 * \endif
 *
 * \if CHINESE
 * @brief 分配曲线拟合器
 * @details 当设置 Fitted CurveAttribute 时，曲线拟合器"平滑"曲线点。
 *          setCurveFitter(nullptr) 也可以禁用曲线拟合。
 *          曲线拟合器对转换后的点（= 控件坐标）进行操作，以便在对数刻度上有效工作。
 *          显然，对于减少点数的拟合算法，这会降低性能。
 *          如果使用曲线拟合来提高绘制大量点序列的性能，
 *          最好对曲线点执行一次拟合器，并将结果缓存在 QwtSeriesData 对象中。
 * @param[in] curveFitter 曲线拟合器
 * @sa Fitted
 * \endif
 */
void QwtPlotCurve::setCurveFitter(QwtCurveFitter* curveFitter)
{
    delete m_data->curveFitter;
    m_data->curveFitter = curveFitter;

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the curve fitter
 * @return Curve fitter, or nullptr if curve fitting is disabled
 * @sa setCurveFitter(), Fitted
 * \endif
 *
 * \if CHINESE
 * @brief 获取曲线拟合器
 * @return 曲线拟合器，禁用曲线拟合时返回nullptr
 * @sa setCurveFitter(), Fitted
 * \endif
 */
QwtCurveFitter* QwtPlotCurve::curveFitter() const
{
    return m_data->curveFitter;
}

/**
 * \if ENGLISH
 * @brief Fill the area between the curve and the baseline with the curve brush
 * @param[in] painter Painter
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in,out] polygon Polygon - will be modified
 * @sa setBrush(), setBaseline(), setStyle()
 * \endif
 *
 * \if CHINESE
 * @brief 用曲线画刷填充曲线和基线之间的区域
 * @param[in] painter 绘图器
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in,out] polygon 多边形 - 将被修改
 * @sa setBrush(), setBaseline(), setStyle()
 * \endif
 */
void QwtPlotCurve::fillCurve(QPainter* painter,
                             const QwtScaleMap& xMap,
                             const QwtScaleMap& yMap,
                             const QRectF& canvasRect,
                             QPolygonF& polygon) const
{
    if (m_data->brush.style() == Qt::NoBrush)
        return;

    closePolyline(painter, xMap, yMap, polygon);
    if (polygon.count() <= 2)  // a line can't be filled
        return;

    QBrush brush = m_data->brush;
    if (!brush.color().isValid())
        brush.setColor(m_data->pen.color());

    if (m_data->paintAttributes & ClipPolygons) {
        const QRectF clipRect = qwtIntersectedClipRect(canvasRect, painter);
        QwtClipper::clipPolygonF(clipRect, polygon, true);
    }

    painter->save();

    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);

    QwtPainter::drawPolygon(painter, polygon);

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Complete a polygon to be a closed polygon including the area between the original polygon and the baseline
 * @param[in] painter Painter
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in,out] polygon Polygon to be completed
 * \endif
 *
 * \if CHINESE
 * @brief 完成多边形使其成为闭合多边形，包含原始多边形和基线之间的区域
 * @param[in] painter 绘图器
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in,out] polygon 要完成的多边形
 * \endif
 */
void QwtPlotCurve::closePolyline(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, QPolygonF& polygon) const
{
    if (polygon.size() < 2)
        return;

    const bool doAlign = QwtPainter::roundingAlignment(painter);

    double baseline = m_data->baseline;

    if (orientation() == Qt::Vertical) {
        if (yMap.transformation())
            baseline = yMap.transformation()->bounded(baseline);

        double refY = yMap.transform(baseline);
        if (doAlign)
            refY = qRound(refY);

        polygon += QPointF(polygon.last().x(), refY);
        polygon += QPointF(polygon.first().x(), refY);
    } else {
        if (xMap.transformation())
            baseline = xMap.transformation()->bounded(baseline);

        double refX = xMap.transform(baseline);
        if (doAlign)
            refX = qRound(refX);

        polygon += QPointF(refX, polygon.last().y());
        polygon += QPointF(refX, polygon.first().y());
    }
}

/**
 * \if ENGLISH
 * @brief Draw symbols
 * @param[in] painter Painter
 * @param[in] symbol Curve symbol
 * @param[in] xMap X map
 * @param[in] yMap Y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa setSymbol(), drawSeries(), drawCurve()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制符号
 * @param[in] painter 绘图器
 * @param[in] symbol 曲线符号
 * @param[in] xMap X映射
 * @param[in] yMap Y映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个要绘制的点的索引
 * @param[in] to 最后一个要绘制的点的索引
 * @sa setSymbol(), drawSeries(), drawCurve()
 * \endif
 */
void QwtPlotCurve::drawSymbols(QPainter* painter,
                               const QwtSymbol& symbol,
                               const QwtScaleMap& xMap,
                               const QwtScaleMap& yMap,
                               const QRectF& canvasRect,
                               int from,
                               int to) const
{
    QwtPointMapper mapper;
    mapper.setFlag(QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment(painter));
    mapper.setFlag(QwtPointMapper::WeedOutPoints, testPaintAttribute(QwtPlotCurve::FilterPoints));

    const QRectF clipRect = qwtIntersectedClipRect(canvasRect, painter);
    mapper.setBoundingRect(clipRect);

    const int chunkSize = 500;

    for (int i = from; i <= to; i += chunkSize) {
        const int n = qMin(chunkSize, to - i + 1);

        const QPolygonF points = mapper.toPointsF(xMap, yMap, data(), i, i + n - 1);

        if (points.size() > 0)
            symbol.drawSymbols(painter, points);
    }
}

/**
 * \if ENGLISH
 * @brief Set the value of the baseline
 * @details The baseline is needed for filling the curve with a brush or the Sticks drawing style.
 *          The interpretation of the baseline depends on the orientation().
 *          With Qt::Vertical, the baseline is interpreted as a horizontal line at y = baseline(),
 *          with Qt::Horizontal, it is interpreted as a vertical line at x = baseline().
 *          The default value is 0.0.
 * @param[in] value Value of the baseline
 * @sa baseline(), setBrush(), setStyle(), QwtPlotAbstractSeriesItem::orientation()
 * \endif
 *
 * \if CHINESE
 * @brief 设置基线的值
 * @details 基线用于用画刷填充曲线或 Sticks 绘制样式。
 *          基线的解释取决于 orientation()。
 *          使用 Qt::Vertical 时，基线解释为 y = baseline() 处的水平线，
 *          使用 Qt::Horizontal 时，解释为 x = baseline() 处的垂直线。
 *          默认值为 0.0。
 * @param[in] value 基线的值
 * @sa baseline(), setBrush(), setStyle(), QwtPlotAbstractSeriesItem::orientation()
 * \endif
 */
void QwtPlotCurve::setBaseline(double value)
{
    if (m_data->baseline != value) {
        m_data->baseline = value;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the baseline value
 * @return Value of the baseline
 * @sa setBaseline()
 * \endif
 *
 * \if CHINESE
 * @brief 获取基线值
 * @return 基线的值
 * @sa setBaseline()
 * \endif
 */
double QwtPlotCurve::baseline() const
{
    return m_data->baseline;
}

/**
 * \if ENGLISH
 * @brief Find the closest curve point for a specific position
 * @param[in] pos Position where to look for the closest curve point
 * @param[out] dist If dist != nullptr, returns the distance between the position and the closest curve point
 * @return Index of the closest curve point, or -1 if none can be found (e.g. when the curve has no points)
 * @note closestPoint() implements a dumb algorithm that iterates over all points
 * \endif
 *
 * \if CHINESE
 * @brief 查找特定位置的最近曲线点
 * @param[in] pos 查找最近曲线点的位置
 * @param[out] dist 如果 dist != nullptr，返回位置和最近曲线点之间的距离
 * @return 最近曲线点的索引，找不到时返回 -1（例如曲线没有点时）
 * @note closestPoint() 实现了一个简单算法，遍历所有点
 * \endif
 */
int QwtPlotCurve::closestPoint(const QPointF& pos, double* dist) const
{
    const size_t numSamples = dataSize();

    if (plot() == nullptr || numSamples <= 0)
        return -1;

    const QwtSeriesData< QPointF >* series = data();

    const QwtScaleMap xMap = plot()->canvasMap(xAxis());
    const QwtScaleMap yMap = plot()->canvasMap(yAxis());

    int index   = -1;
    double dmin = 1.0e10;

    for (uint i = 0; i < numSamples; i++) {
        const QPointF sample = series->sample(i);

        const double cx = xMap.transform(sample.x()) - pos.x();
        const double cy = yMap.transform(sample.y()) - pos.y();

        const double f = qwtSqr(cx) + qwtSqr(cy);
        if (f < dmin) {
            index = i;
            dmin  = f;
        }
    }
    if (dist)
        *dist = std::sqrt(dmin);

    return index;
}

/**
 * \if ENGLISH
 * @brief Get the icon representing the curve on the legend
 * @param[in] index Index of the legend entry (ignored as there is only one)
 * @param[in] size Icon size
 * @return Icon representing the curve on the legend
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例上代表曲线的图标
 * @param[in] index 图例条目的索引（忽略，因为只有一个）
 * @param[in] size 图标大小
 * @return 图例上代表曲线的图标
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 * \endif
 */
QwtGraphic QwtPlotCurve::legendIcon(int index, const QSizeF& size) const
{
    Q_UNUSED(index);

    if (size.isEmpty())
        return QwtGraphic();

    QwtGraphic graphic;
    graphic.setDefaultSize(size);
    graphic.setRenderHint(QwtGraphic::RenderPensUnscaled, true);

    QPainter painter(&graphic);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPlotItem::RenderAntialiased));

    if (m_data->legendAttributes == 0 || m_data->legendAttributes & QwtPlotCurve::LegendShowBrush) {
        QBrush brush = m_data->brush;

        if (brush.style() == Qt::NoBrush && m_data->legendAttributes == 0) {
            if (style() != QwtPlotCurve::NoCurve) {
                brush = QBrush(pen().color());
            } else if (m_data->symbol && (m_data->symbol->style() != QwtSymbol::NoSymbol)) {
                brush = QBrush(m_data->symbol->pen().color());
            }
        }

        if (brush.style() != Qt::NoBrush) {
            QRectF r(0, 0, size.width(), size.height());
            painter.fillRect(r, brush);
        }
    }

    if (m_data->legendAttributes & QwtPlotCurve::LegendShowLine) {
        if (pen() != Qt::NoPen) {
            QPen pn = pen();
            pn.setCapStyle(Qt::FlatCap);

            painter.setPen(pn);

            const double y = 0.5 * size.height();
            QwtPainter::drawLine(&painter, 0.0, y, size.width(), y);
        }
    }

    if (m_data->legendAttributes & QwtPlotCurve::LegendShowSymbol) {
        if (m_data->symbol) {
            QRectF r(0, 0, size.width(), size.height());
            m_data->symbol->drawSymbol(&painter, r);
        }
    }

    return graphic;
}

/**
 * \if ENGLISH
 * @brief Assign a series of points
 * @details setSamples() is just a wrapper for setData() without any additional value - 
 *          beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting it when it's not used anymore.
 * \endif
 *
 * \if CHINESE
 * @brief 分配点序列
 * @details setSamples() 只是 setData() 的包装器，没有额外的价值 - 
 *          除了方便开发者查找。
 * @param[in] data 数据
 * @warning 该项拥有数据对象的所有权，不再使用时会删除它。
 * \endif
 */
void QwtPlotCurve::setSamples(QwtSeriesData< QPointF >* data)
{
    setData(data);
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of points
 * @param[in] samples Vector of points
 * @note QVector is implicitly shared
 * @note QPolygonF is derived from QVector<QPointF>
 * \endif
 *
 * \if CHINESE
 * @brief 用点数组初始化数据
 * @param[in] samples 点向量
 * @note QVector 是隐式共享的
 * @note QPolygonF 派生自 QVector<QPointF>
 * \endif
 */
void QwtPlotCurve::setSamples(const QVector< QPointF >& samples)
{
    setData(new QwtPointSeriesData(samples));
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of points (rvalue)
 * @param[in] samples Vector of points
 * @note QVector is implicitly shared
 * @note QPolygonF is derived from QVector<QPointF>
 * \endif
 *
 * \if CHINESE
 * @brief 用点数组初始化数据（右值）
 * @param[in] samples 点向量
 * @note QVector 是隐式共享的
 * @note QPolygonF 派生自 QVector<QPointF>
 * \endif
 */
void QwtPlotCurve::setSamples(QVector< QPointF >&& samples)
{
    setData(new QwtPointSeriesData(samples));
}

/**
 * \if ENGLISH
 * @brief Initialize data by pointing to memory blocks not managed by QwtPlotCurve (double)
 * @details setRawSamples is provided for efficiency. It is important to keep the pointers
 *          during the lifetime of the underlying QwtCPointerData class.
 * @param[in] xData Pointer to x data
 * @param[in] yData Pointer to y data
 * @param[in] size Size of x and y
 * @sa QwtCPointerData
 * \endif
 *
 * \if CHINESE
 * @brief 通过指向不由 QwtPlotCurve 管理的内存块初始化数据（double）
 * @details setRawSamples 用于提高效率。保持指针在底层 QwtCPointerData 类的生命周期内很重要。
 * @param[in] xData x数据指针
 * @param[in] yData y数据指针
 * @param[in] size x和y的大小
 * @sa QwtCPointerData
 * \endif
 */
void QwtPlotCurve::setRawSamples(const double* xData, const double* yData, int size)
{
    setData(new QwtCPointerData< double >(xData, yData, size));
}

/**
 * \if ENGLISH
 * @brief Initialize data by pointing to memory blocks not managed by QwtPlotCurve (float)
 * @details setRawSamples is provided for efficiency. It is important to keep the pointers
 *          during the lifetime of the underlying QwtCPointerData class.
 * @param[in] xData Pointer to x data
 * @param[in] yData Pointer to y data
 * @param[in] size Size of x and y
 * @sa QwtCPointerData
 * \endif
 *
 * \if CHINESE
 * @brief 通过指向不由 QwtPlotCurve 管理的内存块初始化数据（float）
 * @details setRawSamples 用于提高效率。保持指针在底层 QwtCPointerData 类的生命周期内很重要。
 * @param[in] xData x数据指针
 * @param[in] yData y数据指针
 * @param[in] size x和y的大小
 * @sa QwtCPointerData
 * \endif
 */
void QwtPlotCurve::setRawSamples(const float* xData, const float* yData, int size)
{
    setData(new QwtCPointerData< float >(xData, yData, size));
}

/**
 * \if ENGLISH
 * @brief Initialize data by pointing to a memory block not managed by QwtPlotCurve (double, y-axis only)
 * @details The memory contains the y coordinates, while the index is interpreted as x coordinate.
 *          setRawSamples() is provided for efficiency. It is important to keep the pointers
 *          during the lifetime of the underlying QwtCPointerValueData class.
 * @param[in] yData Pointer to y data
 * @param[in] size Size of y data
 * @sa QwtCPointerData
 * \endif
 *
 * \if CHINESE
 * @brief 通过指向不由 QwtPlotCurve 管理的内存块初始化数据（double，仅y轴）
 * @details 内存包含y坐标，而索引被解释为x坐标。
 *          setRawSamples() 用于提高效率。保持指针在底层 QwtCPointerValueData 类的生命周期内很重要。
 * @param[in] yData y数据指针
 * @param[in] size y数据的大小
 * @sa QwtCPointerData
 * \endif
 */
void QwtPlotCurve::setRawSamples(const double* yData, int size)
{
    setData(new QwtCPointerValueData< double >(yData, size));
}

/**
 * \if ENGLISH
 * @brief Initialize data by pointing to a memory block not managed by QwtPlotCurve (float, y-axis only)
 * @details The memory contains the y coordinates, while the index is interpreted as x coordinate.
 *          setRawSamples() is provided for efficiency. It is important to keep the pointers
 *          during the lifetime of the underlying QwtCPointerValueData class.
 * @param[in] yData Pointer to y data
 * @param[in] size Size of y data
 * @sa QwtCPointerData
 * \endif
 *
 * \if CHINESE
 * @brief 通过指向不由 QwtPlotCurve 管理的内存块初始化数据（float，仅y轴）
 * @details 内存包含y坐标，而索引被解释为x坐标。
 *          setRawSamples() 用于提高效率。保持指针在底层 QwtCPointerValueData 类的生命周期内很重要。
 * @param[in] yData y数据指针
 * @param[in] size y数据的大小
 * @sa QwtCPointerData
 * \endif
 */
void QwtPlotCurve::setRawSamples(const float* yData, int size)
{
    setData(new QwtCPointerValueData< float >(yData, size));
}

/**
 * \if ENGLISH
 * @brief Set data by copying x- and y-values from specified memory blocks (double)
 * @details Contrary to setRawSamples(), this function makes a 'deep copy' of the data.
 * @param[in] xData Pointer to x values
 * @param[in] yData Pointer to y values
 * @param[in] size Size of xData and yData
 * @sa QwtPointArrayData
 * \endif
 *
 * \if CHINESE
 * @brief 通过复制指定内存块的x和y值设置数据（double）
 * @details 与 setRawSamples() 不同，此函数对数据进行"深拷贝"。
 * @param[in] xData x值指针
 * @param[in] yData y值指针
 * @param[in] size xData和yData的大小
 * @sa QwtPointArrayData
 * \endif
 */
void QwtPlotCurve::setSamples(const double* xData, const double* yData, int size)
{
    setData(new QwtPointArrayData< double >(xData, yData, size));
}

/**
 * \if ENGLISH
 * @brief Set data by copying x- and y-values from specified memory blocks (float)
 * @details Contrary to setRawSamples(), this function makes a 'deep copy' of the data.
 * @param[in] xData Pointer to x values
 * @param[in] yData Pointer to y values
 * @param[in] size Size of xData and yData
 * @sa QwtPointArrayData
 * \endif
 *
 * \if CHINESE
 * @brief 通过复制指定内存块的x和y值设置数据（float）
 * @details 与 setRawSamples() 不同，此函数对数据进行"深拷贝"。
 * @param[in] xData x值指针
 * @param[in] yData y值指针
 * @param[in] size xData和yData的大小
 * @sa QwtPointArrayData
 * \endif
 */
void QwtPlotCurve::setSamples(const float* xData, const float* yData, int size)
{
    setData(new QwtPointArrayData< float >(xData, yData, size));
}

/**
 * \if ENGLISH
 * @brief Initialize data with x- and y-arrays (explicitly shared, double)
 * @param[in] xData X data
 * @param[in] yData Y data
 * @sa QwtPointArrayData
 * \endif
 *
 * \if CHINESE
 * @brief 用x和y数组初始化数据（显式共享，double）
 * @param[in] xData X数据
 * @param[in] yData Y数据
 * @sa QwtPointArrayData
 * \endif
 */
void QwtPlotCurve::setSamples(const QVector< double >& xData, const QVector< double >& yData)
{
    setData(new QwtPointArrayData< double >(xData, yData));
}

/**
 * \if ENGLISH
 * @brief Initialize data with x- and y-arrays (rvalue, double)
 * @param[in] xData X data
 * @param[in] yData Y data
 * @sa QwtPointArrayData
 * \endif
 *
 * \if CHINESE
 * @brief 用x和y数组初始化数据（右值，double）
 * @param[in] xData X数据
 * @param[in] yData Y数据
 * @sa QwtPointArrayData
 * \endif
 */
void QwtPlotCurve::setSamples(QVector< double >&& xData, QVector< double >&& yData)
{
    setData(new QwtPointArrayData< double >(xData, yData));
}

/**
 * \if ENGLISH
 * @brief Initialize data with x- and y-arrays (explicitly shared, float)
 * @param[in] xData X data
 * @param[in] yData Y data
 * @sa QwtPointArrayData
 * \endif
 *
 * \if CHINESE
 * @brief 用x和y数组初始化数据（显式共享，float）
 * @param[in] xData X数据
 * @param[in] yData Y数据
 * @sa QwtPointArrayData
 * \endif
 */
void QwtPlotCurve::setSamples(const QVector< float >& xData, const QVector< float >& yData)
{
    setData(new QwtPointArrayData< float >(xData, yData));
}

/**
 * \if ENGLISH
 * @brief Initialize data with x- and y-arrays (rvalue, float)
 * @param[in] xData X data
 * @param[in] yData Y data
 * @sa QwtPointArrayData
 * \endif
 *
 * \if CHINESE
 * @brief 用x和y数组初始化数据（右值，float）
 * @param[in] xData X数据
 * @param[in] yData Y数据
 * @sa QwtPointArrayData
 * \endif
 */
void QwtPlotCurve::setSamples(QVector< float >&& xData, QVector< float >&& yData)
{
    setData(new QwtPointArrayData< float >(xData, yData));
}

/**
 * \if ENGLISH
 * @brief Set data by copying y-values from a specified memory block (double)
 * @details The memory contains the y coordinates, while the index is interpreted as x coordinate.
 * @param[in] yData Y data
 * @param[in] size Size of yData
 * @sa QwtValuePointData
 * \endif
 *
 * \if CHINESE
 * @brief 通过复制指定内存块的y值设置数据（double）
 * @details 内存包含y坐标，而索引被解释为x坐标。
 * @param[in] yData Y数据
 * @param[in] size yData的大小
 * @sa QwtValuePointData
 * \endif
 */
void QwtPlotCurve::setSamples(const double* yData, int size)
{
    setData(new QwtValuePointData< double >(yData, size));
}

/**
 * \if ENGLISH
 * @brief Set data by copying y-values from a specified memory block (float)
 * @details The vector contains the y coordinates, while the index is interpreted as x coordinate.
 * @param[in] yData Y data
 * @param[in] size Size of yData
 * @sa QwtValuePointData
 * \endif
 *
 * \if CHINESE
 * @brief 通过复制指定内存块的y值设置数据（float）
 * @details 向量包含y坐标，而索引被解释为x坐标。
 * @param[in] yData Y数据
 * @param[in] size yData的大小
 * @sa QwtValuePointData
 * \endif
 */
void QwtPlotCurve::setSamples(const float* yData, int size)
{
    setData(new QwtValuePointData< float >(yData, size));
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of y values (explicitly shared, double)
 * @details The vector contains the y coordinates, while the index is the x coordinate.
 * @param[in] yData Y data
 * @sa QwtValuePointData
 * \endif
 *
 * \if CHINESE
 * @brief 用y值数组初始化数据（显式共享，double）
 * @details 向量包含y坐标，而索引是x坐标。
 * @param[in] yData Y数据
 * @sa QwtValuePointData
 * \endif
 */
void QwtPlotCurve::setSamples(const QVector< double >& yData)
{
    setData(new QwtValuePointData< double >(yData));
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of y values (explicitly shared, float)
 * @details The vector contains the y coordinates, while the index is the x coordinate.
 * @param[in] yData Y data
 * @sa QwtValuePointData
 * \endif
 *
 * \if CHINESE
 * @brief 用y值数组初始化数据（显式共享，float）
 * @details 向量包含y坐标，而索引是x坐标。
 * @param[in] yData Y数据
 * @sa QwtValuePointData
 * \endif
 */
void QwtPlotCurve::setSamples(const QVector< float >& yData)
{
    setData(new QwtValuePointData< float >(yData));
}
