/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_curve.h"
#include "qwt_polar.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_symbol.h"
#include "qwt_legend.h"
#include "qwt_curve_fitter.h"
#include "qwt_clipper.h"

#include <qpainter.h>

static inline bool qwtInsidePole(const QwtScaleMap& map, double radius)
{
    return map.isInverting() ? (radius > map.s1()) : (radius < map.s1());
}

class QwtPolarCurve::PrivateData
{
public:
    PrivateData() : style(QwtPolarCurve::Lines), curveFitter(nullptr)
    {
        symbol = new QwtSymbol();
        pen    = QPen(Qt::black);
    }

    ~PrivateData()
    {
        delete symbol;
        delete curveFitter;
    }

    QwtPolarCurve::CurveStyle style;
    const QwtSymbol* symbol;
    QPen pen;
    QwtCurveFitter* curveFitter;

    QwtPolarCurve::LegendAttributes legendAttributes;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtPolarCurve::QwtPolarCurve() : QwtPolarItem(QwtText())
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor with title
 * @param[in] title Title of the curve
 * \endif
 *
 * \if CHINESE
 * @brief 带标题的构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPolarCurve::QwtPolarCurve(const QwtText& title) : QwtPolarItem(title)
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor with title string
 * @param[in] title Title of the curve as string
 * \endif
 *
 * \if CHINESE
 * @brief 带标题字符串的构造函数
 * @param[in] title 曲线标题字符串
 * \endif
 */
QwtPolarCurve::QwtPolarCurve(const QString& title) : QwtPolarItem(QwtText(title))
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
QwtPolarCurve::~QwtPolarCurve()
{
    delete m_series;
    delete m_data;
}

//! Initialize data members
void QwtPolarCurve::init()
{
    m_data   = new PrivateData;
    m_series = nullptr;

    setItemAttribute(QwtPolarItem::AutoScale);
    setItemAttribute(QwtPolarItem::Legend);
    setZ(20.0);

    setRenderHint(RenderAntialiased, true);
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPolarCurve::Rtti_PolarCurve
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPolarCurve::Rtti_PolarCurve
 * \endif
 */
int QwtPolarCurve::rtti() const
{
    return QwtPolarItem::Rtti_PolarCurve;
}

/**
 * \if ENGLISH
 * @brief Specify an attribute for how to draw the legend identifier
 * @param[in] attribute Legend attribute to set
 * @param[in] on True to enable, false to disable
 * @sa LegendAttribute, testLegendAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 指定图例标识符的绘制方式属性
 * @param[in] attribute 要设置的图例属性
 * @param[in] on true 启用，false 禁用
 * @sa LegendAttribute, testLegendAttribute()
 * \endif
 */
void QwtPolarCurve::setLegendAttribute(LegendAttribute attribute, bool on)
{
    if (on)
        m_data->legendAttributes |= attribute;
    else
        m_data->legendAttributes &= ~attribute;
}

/**
 * \if ENGLISH
 * @brief Test if a legend attribute is enabled
 * @param[in] attribute Legend attribute to test
 * @return True if attribute is enabled
 * @sa LegendAttribute, setLegendAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试图例属性是否启用
 * @param[in] attribute 要测试的图例属性
 * @return 如果属性已启用则返回 true
 * @sa LegendAttribute, setLegendAttribute()
 * \endif
 */
bool QwtPolarCurve::testLegendAttribute(LegendAttribute attribute) const
{
    return (m_data->legendAttributes & attribute);
}

/**
 * \if ENGLISH
 * @brief Set the curve's drawing style
 * @param[in] style Curve style
 * @sa CurveStyle, style()
 * \endif
 *
 * \if CHINESE
 * @brief 设置曲线的绘制样式
 * @param[in] style 曲线样式
 * @sa CurveStyle, style()
 * \endif
 */
void QwtPolarCurve::setStyle(CurveStyle style)
{
    if (style != m_data->style) {
        m_data->style = style;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the current style
 * @return Current style
 * @sa CurveStyle, setStyle()
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前样式
 * @return 当前样式
 * @sa CurveStyle, setStyle()
 * \endif
 */
QwtPolarCurve::CurveStyle QwtPolarCurve::style() const
{
    return m_data->style;
}

/**
 * \if ENGLISH
 * @brief Assign a symbol
 * @param[in] symbol New symbol (ownership is transferred)
 * @sa symbol()
 * \endif
 *
 * \if CHINESE
 * @brief 分配符号
 * @param[in] symbol 新符号（所有权转移）
 * @sa symbol()
 * \endif
 */
void QwtPolarCurve::setSymbol(QwtSymbol* symbol)
{
    if (symbol != m_data->symbol) {
        delete m_data->symbol;
        m_data->symbol = symbol;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the current symbol
 * @return The current symbol
 * @sa setSymbol()
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前符号
 * @return 当前符号
 * @sa setSymbol()
 * \endif
 */
const QwtSymbol* QwtPolarCurve::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Assign a pen
 * @param[in] pen New pen
 * @sa pen()
 * \endif
 *
 * \if CHINESE
 * @brief 分配画笔
 * @param[in] pen 新画笔
 * @sa pen()
 * \endif
 */
void QwtPolarCurve::setPen(const QPen& pen)
{
    if (pen != m_data->pen) {
        m_data->pen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen used to draw the lines
 * @return Pen used to draw the lines
 * @sa setPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制线条的画笔
 * @return 用于绘制线条的画笔
 * @sa setPen()
 * \endif
 */
const QPen& QwtPolarCurve::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Initialize data with a pointer to QwtSeriesData<QwtPointPolar>
 * @param[in] data Series data. The x-values represent azimuth, y-values represent radius.
 * @details Ownership of the data is transferred to the curve.
 * @sa data()
 * \endif
 *
 * \if CHINESE
 * @brief 使用 QwtSeriesData<QwtPointPolar> 指针初始化数据
 * @param[in] data 系列数据。x 值表示方位角，y 值表示半径。
 * @details 数据所有权转移给曲线。
 * @sa data()
 * \endif
 */
void QwtPolarCurve::setData(QwtSeriesData< QwtPointPolar >* data)
{
    if (m_series != data) {
        delete m_series;
        m_series = data;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Insert a curve fitter
 * @param[in] curveFitter Curve fitter (ownership is transferred)
 * @details A curve fitter interpolates the curve points. For example, QwtPolarFitter
 *          adds equidistant points so that the connection gets rounded instead
 *          of having straight lines. If curveFitter is nullptr, fitting is disabled.
 * @sa curveFitter()
 * \endif
 *
 * \if CHINESE
 * @brief 插入曲线拟合器
 * @param[in] curveFitter 曲线拟合器（所有权转移）
 * @details 曲线拟合器插值曲线点。例如，QwtPolarFitter 添加等距点，
 *          使连接变得平滑而不是直线。如果 curveFitter 为 nullptr，则禁用拟合。
 * @sa curveFitter()
 * \endif
 */
void QwtPolarCurve::setCurveFitter(QwtCurveFitter* curveFitter)
{
    if (curveFitter != m_data->curveFitter) {
        delete m_data->curveFitter;
        m_data->curveFitter = curveFitter;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the curve fitter
 * @return The curve fitter
 * @sa setCurveFitter()
 * \endif
 *
 * \if CHINESE
 * @brief 获取曲线拟合器
 * @return 曲线拟合器
 * @sa setCurveFitter()
 * \endif
 */
QwtCurveFitter* QwtPolarCurve::curveFitter() const
{
    return m_data->curveFitter;
}

/**
 * \if ENGLISH
 * @brief Draw the curve
 * @param[in] painter Painter
 * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param[in] radialMap Maps radius values into painter coordinates
 * @param[in] pole Position of the pole in painter coordinates
 * @param[in] radius Radius of the complete plot area in painter coordinates
 * @param[in] canvasRect Contents rect of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制曲线
 * @param[in] painter 绘图器
 * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
 * @param[in] radialMap 将半径值映射到绘图器坐标
 * @param[in] pole 绘图器坐标中极点的位置
 * @param[in] radius 绘图器坐标中完整绘图区域的半径
 * @param[in] canvasRect 绘图器坐标中画布的内容矩形
 * \endif
 */
void QwtPolarCurve::draw(QPainter* painter,
                         const QwtScaleMap& azimuthMap,
                         const QwtScaleMap& radialMap,
                         const QPointF& pole,
                         double radius,
                         const QRectF& canvasRect) const
{
    Q_UNUSED(radius);
    Q_UNUSED(canvasRect);

    draw(painter, azimuthMap, radialMap, pole, 0, -1);
}

/**
 * \if ENGLISH
 * @brief Draw an interval of the curve
 * @param[in] painter Painter
 * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param[in] radialMap Maps radius values into painter coordinates
 * @param[in] pole Position of the pole in painter coordinates
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0, the curve will be painted to its last point.
 * @sa drawCurve(), drawSymbols()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制曲线的一个区间
 * @param[in] painter 绘图器
 * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
 * @param[in] radialMap 将半径值映射到绘图器坐标
 * @param[in] pole 绘图器坐标中极点的位置
 * @param[in] from 要绘制的第一个点的索引
 * @param[in] to 要绘制的最后一个点的索引。如果 to < 0，曲线将绘制到最后一个点。
 * @sa drawCurve(), drawSymbols()
 * \endif
 */
void QwtPolarCurve::draw(QPainter* painter,
                         const QwtScaleMap& azimuthMap,
                         const QwtScaleMap& radialMap,
                         const QPointF& pole,
                         int from,
                         int to) const
{
    if (!painter || dataSize() <= 0)
        return;

    if (to < 0)
        to = dataSize() - 1;

    if (qwtVerifyRange(dataSize(), from, to) > 0) {
        painter->save();
        painter->setPen(m_data->pen);

        drawCurve(painter, m_data->style, azimuthMap, radialMap, pole, from, to);

        painter->restore();

        if (m_data->symbol->style() != QwtSymbol::NoSymbol) {
            painter->save();
            drawSymbols(painter, *m_data->symbol, azimuthMap, radialMap, pole, from, to);
            painter->restore();
        }
    }
}

/*!
   Draw the line part (without symbols) of a curve interval.

   \param painter Painter
   \param style Curve style, see QwtPolarCurve::CurveStyle
   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param from index of the first point to be painted
   \param to index of the last point to be painted.
   \sa draw(), drawLines()
 */
void QwtPolarCurve::drawCurve(QPainter* painter,
                              int style,
                              const QwtScaleMap& azimuthMap,
                              const QwtScaleMap& radialMap,
                              const QPointF& pole,
                              int from,
                              int to) const
{
    switch (style) {
    case Lines:
        drawLines(painter, azimuthMap, radialMap, pole, from, to);
        break;
    case NoCurve:
    default:
        break;
    }
}

/*!
   Draw lines

   \param painter Painter
   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param from index of the first point to be painted
   \param to index of the last point to be painted.
   \sa draw(), drawLines(), setCurveFitter()
 */
void QwtPolarCurve::drawLines(QPainter* painter,
                              const QwtScaleMap& azimuthMap,
                              const QwtScaleMap& radialMap,
                              const QPointF& pole,
                              int from,
                              int to) const
{
    int size = to - from + 1;
    if (size <= 0)
        return;

    QPolygonF polyline;

    if (m_data->curveFitter) {
        QPolygonF points(size);
        for (int j = from; j <= to; j++) {
            const QwtPointPolar point = sample(j);
            points[ j - from ]        = QPointF(point.azimuth(), point.radius());
        }

        points = m_data->curveFitter->fitCurve(points);

        polyline.resize(points.size());

        QPointF* polylineData = polyline.data();
        QPointF* pointsData   = points.data();

        for (int i = 0; i < points.size(); i++) {
            const QwtPointPolar point(pointsData[ i ].x(), pointsData[ i ].y());

            double r       = radialMap.transform(point.radius());
            const double a = azimuthMap.transform(point.azimuth());

            polylineData[ i ] = qwtPolar2Pos(pole, r, a);
        }
    } else {
        polyline.resize(size);
        QPointF* polylineData = polyline.data();

        for (int i = from; i <= to; i++) {
            QwtPointPolar point = sample(i);
            if (!qwtInsidePole(radialMap, point.radius())) {
                double r                 = radialMap.transform(point.radius());
                const double a           = azimuthMap.transform(point.azimuth());
                polylineData[ i - from ] = qwtPolar2Pos(pole, r, a);
            } else {
                polylineData[ i - from ] = pole;
            }
        }
    }

    QRectF clipRect;
    if (painter->hasClipping()) {
        clipRect = painter->clipRegion().boundingRect();
    } else {
        clipRect = painter->window();
        if (!clipRect.isEmpty())
            clipRect = painter->transform().inverted().mapRect(clipRect);
    }

    if (!clipRect.isEmpty()) {
        double off = qCeil(qMax(qreal(1.0), painter->pen().widthF()));
        clipRect   = clipRect.toRect().adjusted(-off, -off, off, off);
        QwtClipper::clipPolygonF(clipRect, polyline);
    }

    QwtPainter::drawPolyline(painter, polyline);
}

/*!
   Draw symbols

   \param painter Painter
   \param symbol Curve symbol
   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param from index of the first point to be painted
   \param to index of the last point to be painted.

   \sa setSymbol(), draw(), drawCurve()
 */
void QwtPolarCurve::drawSymbols(QPainter* painter,
                                const QwtSymbol& symbol,
                                const QwtScaleMap& azimuthMap,
                                const QwtScaleMap& radialMap,
                                const QPointF& pole,
                                int from,
                                int to) const
{
    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    const int chunkSize = 500;

    for (int i = from; i <= to; i += chunkSize) {
        const int n = qMin(chunkSize, to - i + 1);

        QPolygonF points;
        for (int j = 0; j < n; j++) {
            const QwtPointPolar point = sample(i + j);

            if (!qwtInsidePole(radialMap, point.radius())) {
                const double r = radialMap.transform(point.radius());
                const double a = azimuthMap.transform(point.azimuth());

                points += qwtPolar2Pos(pole, r, a);
            } else {
                points += pole;
            }
        }

        if (points.size() > 0)
            symbol.drawSymbols(painter, points);
    }
}

/**
 * \if ENGLISH
 * @brief Get the number of data points
 * @return Number of points
 * @sa setData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取数据点数量
 * @return 点的数量
 * @sa setData()
 * \endif
 */
size_t QwtPolarCurve::dataSize() const
{
    return m_series->size();
}

/**
 * \if ENGLISH
 * @brief Get the icon representing the curve on the legend
 * @param[in] index Index of the legend entry (ignored as there is only one)
 * @param[in] size Icon size
 * @return Icon representing the curve on the legend
 * @sa QwtPolarItem::setLegendIconSize(), QwtPolarItem::legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例上代表曲线的图标
 * @param[in] index 图例条目的索引（忽略，因为只有一个）
 * @param[in] size 图标大小
 * @return 图例上代表曲线的图标
 * @sa QwtPolarItem::setLegendIconSize(), QwtPolarItem::legendData()
 * \endif
 */
QwtGraphic QwtPolarCurve::legendIcon(int index, const QSizeF& size) const
{
    Q_UNUSED(index);

    if (size.isEmpty())
        return QwtGraphic();

    QwtGraphic graphic;
    graphic.setDefaultSize(size);
    graphic.setRenderHint(QwtGraphic::RenderPensUnscaled, true);

    QPainter painter(&graphic);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPolarItem::RenderAntialiased));

    if (m_data->legendAttributes == 0) {
        QBrush brush;

        if (style() != QwtPolarCurve::NoCurve) {
            brush = QBrush(pen().color());
        } else if (m_data->symbol && (m_data->symbol->style() != QwtSymbol::NoSymbol)) {
            brush = QBrush(m_data->symbol->pen().color());
        }

        if (brush.style() != Qt::NoBrush) {
            QRectF r(0, 0, size.width(), size.height());
            painter.fillRect(r, brush);
        }
    }

    if (m_data->legendAttributes & QwtPolarCurve::LegendShowLine) {
        if (pen() != Qt::NoPen) {
            QPen pn = pen();
            pn.setCapStyle(Qt::FlatCap);

            painter.setPen(pn);

            const double y = 0.5 * size.height();
            QwtPainter::drawLine(&painter, 0.0, y, size.width(), y);
        }
    }

    if (m_data->legendAttributes & QwtPolarCurve::LegendShowSymbol) {
        if (m_data->symbol) {
            QRectF r(0, 0, size.width(), size.height());
            m_data->symbol->drawSymbol(&painter, r);
        }
    }

    return graphic;
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval necessary to display the item
 * @param[in] scaleId Scale index
 * @return Bounding interval
 * @details This interval can be useful for operations like clipping or autoscaling.
 * @sa QwtData::boundingRect()
 * \endif
 *
 * \if CHINESE
 * @brief 获取显示项所需的边界区间
 * @param[in] scaleId 刻度索引
 * @return 边界区间
 * @details 此区间可用于裁剪或自动缩放等操作。
 * @sa QwtData::boundingRect()
 * \endif
 */
QwtInterval QwtPolarCurve::boundingInterval(int scaleId) const
{
    const QRectF boundingRect = m_series->boundingRect();

    if (scaleId == QwtPolar::ScaleAzimuth)
        return QwtInterval(boundingRect.left(), boundingRect.right());

    if (scaleId == QwtPolar::ScaleRadius)
        return QwtInterval(boundingRect.top(), boundingRect.bottom());

    return QwtInterval();
}
