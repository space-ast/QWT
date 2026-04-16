/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/

#include "qwt_plot_picker.h"
#include "qwt_plot.h"
#include "qwt_text.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_picker_machine.h"

class QwtPlotPicker::PrivateData
{
public:
    PrivateData() : xAxisId(-1), yAxisId(-1)
    {
    }

    QwtAxisId xAxisId;
    QwtAxisId yAxisId;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Creates a plot picker attached to the specified canvas.
 *          The picker is set to those x- and y-axis of the plot that are enabled.
 *          If both or no x-axis are enabled, the picker is set to QwtAxis::XBottom.
 *          If both or no y-axis are enabled, it is set to QwtAxis::YLeft.
 * @param[in] canvas Plot canvas to observe, also the parent object
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @details 创建一个附加到指定画布的绘图画布选择器。
 *          选择器会绑定到绘图中已启用的 X 轴和 Y 轴。
 *          如果两个或没有 X 轴启用，选择器绑定到 QwtAxis::XBottom。
 *          如果两个或没有 Y 轴启用，选择器绑定到 QwtAxis::YLeft。
 * @param[in] canvas 要观察的绘图画布，同时也是父对象
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 * \endif
 */
QwtPlotPicker::QwtPlotPicker(QWidget* canvas) : QwtPicker(canvas)
{
    m_data = new PrivateData;

    if (!canvas)
        return;

    const QwtPlot* plot = QwtPlotPicker::plot();
    // attach axes

    using namespace QwtAxis;

    int xAxis = XBottom;
    if (!plot->isAxisVisible(XBottom) && plot->isAxisVisible(XTop))
        xAxis = XTop;

    int yAxis = YLeft;
    if (!plot->isAxisVisible(YLeft) && plot->isAxisVisible(YRight))
        yAxis = YRight;

    setAxes(xAxis, yAxis);
}

/**
 * \if ENGLISH
 * @brief Constructor with axes
 * @details Creates a plot picker attached to the specified canvas with given axes.
 * @param[in] xAxisId X axis of the picker
 * @param[in] yAxisId Y axis of the picker
 * @param[in] canvas Plot canvas to observe, also the parent object
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 * \endif
 * 
 * \if CHINESE
 * @brief 带坐标轴参数的构造函数
 * @details 创建一个附加到指定画布并绑定指定坐标轴的绘图画布选择器。
 * @param[in] xAxisId 选择器的 X 轴ID
 * @param[in] yAxisId 选择器的 Y 轴ID
 * @param[in] canvas 要观察的绘图画布，同时也是父对象
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 * \endif
 */
QwtPlotPicker::QwtPlotPicker(QwtAxisId xAxisId, QwtAxisId yAxisId, QWidget* canvas) : QwtPicker(canvas)
{
    m_data          = new PrivateData;
    m_data->xAxisId = xAxisId;
    m_data->yAxisId = yAxisId;
}

/**
 * \if ENGLISH
 * @brief Constructor with axes, rubber band and tracker mode
 * @details Creates a plot picker with specified axes, rubber band style and tracker mode.
 * @param[in] xAxisId X axis of the picker
 * @param[in] yAxisId Y axis of the picker
 * @param[in] rubberBand Rubber band style
 * @param[in] trackerMode Tracker mode
 * @param[in] canvas Plot canvas to observe, also the parent object
 * @sa QwtPicker, QwtPicker::setSelectionFlags(), QwtPicker::setRubberBand(), QwtPicker::setTrackerMode
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 * \endif
 * 
 * \if CHINESE
 * @brief 带坐标轴、橡皮筋和跟踪模式的构造函数
 * @details 创建一个具有指定坐标轴、橡皮筋样式和跟踪模式的绘图画布选择器。
 * @param[in] xAxisId 选择器的 X 轴ID
 * @param[in] yAxisId 选择器的 Y 轴ID
 * @param[in] rubberBand 橡皮筋样式
 * @param[in] trackerMode 跟踪模式
 * @param[in] canvas 要观察的绘图画布，同时也是父对象
 * @sa QwtPicker, QwtPicker::setSelectionFlags(), QwtPicker::setRubberBand(), QwtPicker::setTrackerMode
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 * \endif
 */
QwtPlotPicker::QwtPlotPicker(QwtAxisId xAxisId, QwtAxisId yAxisId, RubberBand rubberBand, DisplayMode trackerMode, QWidget* canvas)
    : QwtPicker(rubberBand, trackerMode, canvas)
{
    m_data          = new PrivateData;
    m_data->xAxisId = xAxisId;
    m_data->yAxisId = yAxisId;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * @details Destroys the plot picker and releases all allocated resources.
 * \endif
 * 
 * \if CHINESE
 * @brief 析构函数
 * @details 销毁绘图画布选择器并释放所有分配的资源。
 * \endif
 */
QwtPlotPicker::~QwtPlotPicker()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the canvas widget
 * @return Observed plot canvas widget
 * \endif
 * 
 * \if CHINESE
 * @brief 获取画布控件
 * @return 被观察的绘图画布控件
 * \endif
 */
QWidget* QwtPlotPicker::canvas()
{
    return parentWidget();
}

/**
 * \if ENGLISH
 * @brief Get the canvas widget (const version)
 * @return Observed plot canvas widget
 * \endif
 * 
 * \if CHINESE
 * @brief 获取画布控件（常量版本）
 * @return 被观察的绘图画布控件
 * \endif
 */
const QWidget* QwtPlotPicker::canvas() const
{
    return parentWidget();
}

/**
 * \if ENGLISH
 * @brief Get the plot widget
 * @return Plot widget containing the observed plot canvas
 * \endif
 * 
 * \if CHINESE
 * @brief 获取绘图控件
 * @return 包含被观察画布的绘图控件
 * \endif
 */
QwtPlot* QwtPlotPicker::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/**
 * \if ENGLISH
 * @brief Get the plot widget (const version)
 * @return Plot widget containing the observed plot canvas
 * \endif
 * 
 * \if CHINESE
 * @brief 获取绘图控件（常量版本）
 * @return 包含被观察画布的绘图控件
 * \endif
 */
const QwtPlot* QwtPlotPicker::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
   \return Normalized bounding rectangle of the axes
   \sa QwtPlot::autoReplot(), QwtPlot::replot().
 */
QRectF QwtPlotPicker::scaleRect() const
{
    QRectF rect;

    if (plot()) {
        const QwtScaleDiv& xs = plot()->axisScaleDiv(xAxis());
        const QwtScaleDiv& ys = plot()->axisScaleDiv(yAxis());

        rect = QRectF(xs.lowerBound(), ys.lowerBound(), xs.range(), ys.range());
        rect = rect.normalized();
    }

    return rect;
}

/**
 * \if ENGLISH
 * @brief Set the x and y axes
 * @details Binds the picker to the specified x and y axes.
 * @param[in] xAxisId X axis ID
 * @param[in] yAxisId Y axis ID
 * \endif
 * 
 * \if CHINESE
 * @brief 设置 X 轴和 Y 轴
 * @details 将选择器绑定到指定的 X 轴和 Y 轴。
 * @param[in] xAxisId X 轴ID
 * @param[in] yAxisId Y 轴ID
 * \endif
 */
void QwtPlotPicker::setAxes(QwtAxisId xAxisId, QwtAxisId yAxisId)
{
    const QwtPlot* plt = plot();
    if (!plt)
        return;

    if (xAxisId != m_data->xAxisId || yAxisId != m_data->yAxisId) {
        m_data->xAxisId = xAxisId;
        m_data->yAxisId = yAxisId;
    }
}

/**
 * \if ENGLISH
 * @brief Get the x axis
 * @return X axis ID
 * \endif
 * 
 * \if CHINESE
 * @brief 获取 X 轴
 * @return X 轴ID
 * \endif
 */
QwtAxisId QwtPlotPicker::xAxis() const
{
    return m_data->xAxisId;
}

/**
 * \if ENGLISH
 * @brief Get the y axis
 * @return Y axis ID
 * \endif
 * 
 * \if CHINESE
 * @brief 获取 Y 轴
 * @return Y 轴ID
 * \endif
 */
QwtAxisId QwtPlotPicker::yAxis() const
{
    return m_data->yAxisId;
}

/*!
   Translate a pixel position into a position string

   \param pos Position in pixel coordinates
   \return Position string
 */
QwtText QwtPlotPicker::trackerText(const QPoint& pos) const
{
    if (plot() == nullptr)
        return QwtText();

    return trackerTextF(invTransform(pos));
}

/*!
   \brief Translate a position into a position string

   In case of HLineRubberBand the label is the value of the
   y position, in case of VLineRubberBand the value of the x position.
   Otherwise the label contains x and y position separated by a ',' .

   The format for the double to string conversion is "%.4f".

   \param pos Position
   \return Position string
 */
QwtText QwtPlotPicker::trackerTextF(const QPointF& pos) const
{
    QString text;

    switch (rubberBand()) {
    case HLineRubberBand:
        text = QString::number(pos.y(), 'f', 4);
        break;
    case VLineRubberBand:
        text = QString::number(pos.x(), 'f', 4);
        break;
    default:
        text = QString::number(pos.x(), 'f', 4) + ", " + QString::number(pos.y(), 'f', 4);
    }
    return QwtText(text);
}

/*!
   Append a point to the selection and update rubber band and tracker.

   \param pos Additional point
   \sa isActive, begin(), end(), move(), appended()

   \note The appended(const QPoint &), appended(const QDoublePoint &)
        signals are emitted.
 */
void QwtPlotPicker::append(const QPoint& pos)
{
    QwtPicker::append(pos);
    Q_EMIT appended(invTransform(pos));
}

/*!
   Move the last point of the selection

   \param pos New position
   \sa isActive, begin(), end(), append()

   \note The moved(const QPoint &), moved(const QDoublePoint &)
        signals are emitted.
 */
void QwtPlotPicker::move(const QPoint& pos)
{
    QwtPicker::move(pos);
    Q_EMIT moved(invTransform(pos));
}

/*!
   Close a selection setting the state to inactive.

   \param ok If true, complete the selection and emit selected signals
            otherwise discard the selection.
   \return True if the selection has been accepted, false otherwise
 */

bool QwtPlotPicker::end(bool ok)
{
    ok = QwtPicker::end(ok);
    if (!ok)
        return false;

    QwtPlot* plot = QwtPlotPicker::plot();
    if (!plot)
        return false;

    const QPolygon points = selection();
    if (points.count() == 0)
        return false;

    QwtPickerMachine::SelectionType selectionType = QwtPickerMachine::NoSelection;

    if (stateMachine())
        selectionType = stateMachine()->selectionType();

    switch (selectionType) {
    case QwtPickerMachine::PointSelection: {
        const QPointF pos = invTransform(points.first());
        Q_EMIT selected(pos);
        break;
    }
    case QwtPickerMachine::RectSelection: {
        if (points.count() >= 2) {
            const QPoint p1 = points.first();
            const QPoint p2 = points.last();

            const QRect rect = QRect(p1, p2).normalized();
            Q_EMIT selected(invTransform(rect));
        }
        break;
    }
    case QwtPickerMachine::PolygonSelection: {
        QVector< QPointF > dpa(points.count());
        for (int i = 0; i < points.count(); i++)
            dpa[ i ] = invTransform(points[ i ]);

        Q_EMIT selected(dpa);
    }
    default:
        break;
    }

    return true;
}

/*!
    Translate a rectangle from pixel into plot coordinates

    \return Rectangle in plot coordinates
    \sa transform()
 */
QRectF QwtPlotPicker::invTransform(const QRect& rect) const
{
    const QwtScaleMap xMap = plot()->canvasMap(xAxis());
    const QwtScaleMap yMap = plot()->canvasMap(yAxis());

    return QwtScaleMap::invTransform(xMap, yMap, rect);
}

/*!
    Translate a rectangle from plot into pixel coordinates
    \return Rectangle in pixel coordinates
    \sa invTransform()
 */
QRect QwtPlotPicker::transform(const QRectF& rect) const
{
    const QwtScaleMap xMap = plot()->canvasMap(xAxis());
    const QwtScaleMap yMap = plot()->canvasMap(yAxis());

    return QwtScaleMap::transform(xMap, yMap, rect).toRect();
}

/*!
    Translate a point from pixel into plot coordinates
    \return Point in plot coordinates
    \sa transform()
 */
QPointF QwtPlotPicker::invTransform(const QPoint& pos) const
{
    const QwtScaleMap xMap = plot()->canvasMap(xAxis());
    const QwtScaleMap yMap = plot()->canvasMap(yAxis());

    return QPointF(xMap.invTransform(pos.x()), yMap.invTransform(pos.y()));
}

/*!
    Translate a point from plot into pixel coordinates
    \return Point in pixel coordinates
    \sa invTransform()
 */
QPoint QwtPlotPicker::transform(const QPointF& pos) const
{
    const QwtScaleMap xMap = plot()->canvasMap(xAxis());
    const QwtScaleMap yMap = plot()->canvasMap(yAxis());

    const QPointF p(xMap.transform(pos.x()), yMap.transform(pos.y()));

    return p.toPoint();
}
