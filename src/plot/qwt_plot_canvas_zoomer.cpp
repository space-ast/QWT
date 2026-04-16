/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/

#include "qwt_plot_canvas_zoomer.h"
#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_picker_machine.h"

#include <qstack.h>
#include <limits>

/**
 * \if ENGLISH
 * @brief Default constructor creating an invalid zoom state
 * \endif
 * 
 * \if CHINESE
 * @brief 默认构造函数，创建一个无效的缩放状态
 * \endif
 */
QwtPlotCanvasZoomState::QwtPlotCanvasZoomState()
{
}

/**
 * \if ENGLISH
 * @brief Constructor creating a zoom state with specified axis intervals
 * @param[in] p Plot widget associated with this zoom state
 * @param[in] yLeft Interval for the left Y axis
 * @param[in] yRight Interval for the right Y axis
 * @param[in] xBottom Interval for the bottom X axis
 * @param[in] xTop Interval for the top X axis
 * \endif
 * 
 * \if CHINESE
 * @brief 使用指定的坐标轴区间创建缩放状态
 * @param[in] p 与此缩放状态关联的绘图控件
 * @param[in] yLeft 左侧Y轴的区间
 * @param[in] yRight 右侧Y轴的区间
 * @param[in] xBottom 底部X轴的区间
 * @param[in] xTop 顶部X轴的区间
 * \endif
 */
QwtPlotCanvasZoomState::QwtPlotCanvasZoomState(QwtPlot* p,
                                               const QwtInterval& yLeft,
                                               const QwtInterval& yRight,
                                               const QwtInterval& xBottom,
                                               const QwtInterval& xTop)
    : plot(p)
{
    axisInterval[ QwtAxis::YLeft ]   = yLeft;
    axisInterval[ QwtAxis::YRight ]  = yRight;
    axisInterval[ QwtAxis::XBottom ] = xBottom;
    axisInterval[ QwtAxis::XTop ]    = xTop;
}

/**
 * \if ENGLISH
 * @brief Create a zoom state from the current axis ranges of a plot
 * @param[in] p Plot widget to capture axis ranges from
 * @return QwtPlotCanvasZoomState containing current axis intervals
 * @details This static method captures the current scale divisions of all four axes
 *          from the given plot and stores them in a new zoom state object.
 * \endif
 * 
 * \if CHINESE
 * @brief 从绘图的当前坐标轴范围创建缩放状态
 * @param[in] p 要捕获坐标轴范围的绘图控件
 * @return 包含当前坐标轴区间的 QwtPlotCanvasZoomState
 * @details 此静态方法从给定绘图中捕获所有四个坐标轴的当前刻度划分，
 *          并将它们存储在新的缩放状态对象中。
 * \endif
 */
QwtPlotCanvasZoomState QwtPlotCanvasZoomState::fromPlot(QwtPlot* p)
{
    QwtPlotCanvasZoomState state;
    if (!p) {
        return state;
    }
    state.plot = p;
    // 获取四个坐标轴的当前范围
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        const QwtScaleDiv& scaleDiv  = p->axisScaleDiv(axisId);
        state.axisInterval[ axisId ] = scaleDiv.interval();
    }

    return state;
}

/**
 * \if ENGLISH
 * @brief Apply this zoom state to the associated plot
 * @details Sets all four axis scales to the intervals stored in this zoom state.
 *          Does nothing if the plot pointer is null.
 * \endif
 * 
 * \if CHINESE
 * @brief 将此缩放状态应用到关联的绘图
 * @details 将所有四个坐标轴的刻度设置为此缩放状态中存储的区间。
 *          如果绘图指针为空，则不执行任何操作。
 * \endif
 */
void QwtPlotCanvasZoomState::apply() const
{
    if (!plot) {
        return;
    }

    // 应用四个坐标轴的范围
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        plot->setAxisScale(axisId, axisInterval[ axisId ].minValue(), axisInterval[ axisId ].maxValue());
    }
}

/**
 * \if ENGLISH
 * @brief Compare two zoom states for equality
 * @param[in] other Another zoom state to compare with
 * @return true if both states have the same plot and axis intervals
 * \endif
 * 
 * \if CHINESE
 * @brief 比较两个缩放状态是否相等
 * @param[in] other 要比较的另一个缩放状态
 * @return 如果两个状态具有相同的绘图和坐标轴区间，则返回 true
 * \endif
 */
bool QwtPlotCanvasZoomState::operator==(const QwtPlotCanvasZoomState& other) const
{
    if (plot != other.plot) {
        return false;
    }
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        if (axisInterval[ axisId ] != other.axisInterval[ axisId ]) {
            return false;
        }
    }
    return true;
}

/**
 * \if ENGLISH
 * @brief Compare two zoom states for inequality
 * @param[in] other Another zoom state to compare with
 * @return true if the states differ in plot or any axis interval
 * \endif
 * 
 * \if CHINESE
 * @brief 比较两个缩放状态是否不相等
 * @param[in] other 要比较的另一个缩放状态
 * @return 如果状态在绘图或任何坐标轴区间上不同，则返回 true
 * \endif
 */
bool QwtPlotCanvasZoomState::operator!=(const QwtPlotCanvasZoomState& other) const
{
    return !(*this == other);
}

/**
 * \if ENGLISH
 * @brief Check if this zoom state is valid
 * @return true if the zoom state has an associated plot pointer
 * \endif
 * 
 * \if CHINESE
 * @brief 检查此缩放状态是否有效
 * @return 如果缩放状态有关联的绘图指针，则返回 true
 * \endif
 */
bool QwtPlotCanvasZoomState::isValid() const
{
    return (!plot.isNull());
}
//----------------------------------------------------
// QwtCanvasZoomer::PrivateData
//----------------------------------------------------
class QwtPlotCanvasZoomer::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotCanvasZoomer)
public:
    PrivateData(QwtPlotCanvasZoomer* p);
    uint zoomStateIndex;
    QStack< QList< QwtPlotCanvasZoomState > > zoomStack;
    int maxStackDepth { -1 };  ///< zoomer记录的最大的缩放次数，如果超过这个次数，不会再让缩放
    bool replot { true };      ///< 是否进行重绘
};

QwtPlotCanvasZoomer::PrivateData::PrivateData(QwtPlotCanvasZoomer* p) : q_ptr(p)
{
}

//----------------------------------------------------
// QwtCanvasZoomer
//----------------------------------------------------

/**
 * \if ENGLISH
 * @brief Constructor creating a zoomer for a plot canvas
 * @param[in] canvas Plot canvas to observe, also the parent object
 * @param[in] doReplot If true, call QwtPlot::replot() before initializing
 * @details The zoomer will zoom all axes of the plot simultaneously.
 *          It uses a drag rectangle picker machine for selecting zoom regions.
 *          The tracker mode is set to ActiveOnly and the rubber band is set to RectRubberBand.
 * \endif
 * 
 * \if CHINESE
 * @brief 为绘图画布创建缩放器的构造函数
 * @param[in] canvas 要观察的绘图画布，同时也是父对象
 * @param[in] doReplot 如果为 true，在初始化前调用 QwtPlot::replot()
 * @details 缩放器将同时缩放绘图的所有坐标轴。
 *          它使用拖拽矩形拾取机器来选择缩放区域。
 *          追踪器模式设置为 ActiveOnly，橡皮筋设置为 RectRubberBand。
 * \endif
 */
QwtPlotCanvasZoomer::QwtPlotCanvasZoomer(QWidget* canvas, bool doReplot) : QwtCanvasPicker(canvas), QWT_PIMPL_CONSTRUCT
{
    if (canvas) {
        init(doReplot);
    }
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
QwtPlotCanvasZoomer::~QwtPlotCanvasZoomer()
{
}

void QwtPlotCanvasZoomer::init(bool doReplot)
{
    m_data->maxStackDepth = -1;
    m_data->replot        = doReplot;
    setTrackerMode(ActiveOnly);
    setRubberBand(RectRubberBand);
    setStateMachine(new QwtPickerDragRectMachine());
    setZoomBase(doReplot);
}

QList< QwtPlotCanvasZoomState > QwtPlotCanvasZoomer::canvasRectToZoomStateList(const QRect& pixelRect) const
{
    const QwtPlot* plt = plot();
    if (!plt) {
        return QList< QwtPlotCanvasZoomState >();
    }
    QList< QwtPlotCanvasZoomState > states;
    // 宿主轴放到最后，一般寄生轴会绑定宿主轴的某个轴，宿主轴最后变更可以把寄生轴重新设置回和宿主轴同步
    const QList< QwtPlot* > plts = plt->plotList(true);
    for (QwtPlot* p : plts) {
        QwtPlotCanvasZoomState s = canvasRectToZoomState(p, pixelRect);
        states.append(s);
    }
    return states;
}

QwtPlotCanvasZoomState QwtPlotCanvasZoomer::canvasRectToZoomState(QwtPlot* plt, const QRect& pixelRect) const
{
    if (!plt) {
        return QwtPlotCanvasZoomState();
    }
    // 将像素矩形转换为四个坐标轴的区间
    QRect normalizedRect = pixelRect.normalized();
    QwtPlotCanvasZoomState state;
    state.plot = plt;
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        const QwtScaleMap& scaleMap = plt->canvasMap(axisId);
        double v1 { 0.0 }, v2 { 0.0 };
        if (QwtAxis::isXAxis(axisId)) {
            v1 = scaleMap.invTransform(normalizedRect.left());
            v2 = scaleMap.invTransform(normalizedRect.right());
        } else {
            v1 = scaleMap.invTransform(normalizedRect.bottom());
            v2 = scaleMap.invTransform(normalizedRect.top());
        }
        if (const QwtTransform* transform = scaleMap.transformation()) {
            // 如果存在变换，要确认变换的范围,bounded实际是qBound( LogMin, value, LogMax );
            v1 = transform->bounded(v1);
            v2 = transform->bounded(v2);
        }
        state.axisInterval[ axisId ].setInterval(v1, v2);
    }
    return state;
}

/**
 * \if ENGLISH
 * @brief Set the maximum depth of the zoom stack
 * @param[in] depth Maximum number of zoom levels, -1 for unlimited
 * @details When the stack exceeds this depth, older zoom states are removed.
 *          If the current zoom level is beyond the new limit, it will be adjusted.
 * \endif
 * 
 * \if CHINESE
 * @brief 设置缩放堆栈的最大深度
 * @param[in] depth 缩放级别的最大数量，-1 表示无限制
 * @details 当堆栈超过此深度时，较旧的缩放状态将被移除。
 *          如果当前缩放级别超出新的限制，则会被调整。
 * \endif
 */
void QwtPlotCanvasZoomer::setMaxStackDepth(int depth)
{
    m_data->maxStackDepth = depth;

    if (depth >= 0) {
        const int zoomOut = m_data->zoomStack.count() - 1 - depth;
        if (zoomOut > 0) {
            zoom(-zoomOut);
            for (int i = m_data->zoomStack.count() - 1; i > int(m_data->zoomStateIndex); i--) {
                (void)m_data->zoomStack.pop();
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum depth of the zoom stack
 * @return Maximum number of zoom levels, -1 means unlimited
 * \endif
 * 
 * \if CHINESE
 * @brief 获取缩放堆栈的最大深度
 * @return 缩放级别的最大数量，-1 表示无限制
 * \endif
 */
int QwtPlotCanvasZoomer::maxStackDepth() const
{
    return m_data->maxStackDepth;
}

/**
 * \if ENGLISH
 * @brief Get the zoom stack containing all zoom states
 * @return Const reference to the internal zoom stack
 * @details The stack contains the complete history of zoom operations,
 *          with index 0 being the base (unzoomed) state.
 * \endif
 * 
 * \if CHINESE
 * @brief 获取包含所有缩放状态的缩放堆栈
 * @return 内部缩放堆栈的常量引用
 * @details 堆栈包含完整的缩放操作历史，
 *          索引 0 是基础（未缩放）状态。
 * \endif
 */
const QStack< QList< QwtPlotCanvasZoomState > >& QwtPlotCanvasZoomer::zoomStack() const
{
    return m_data->zoomStack;
}

/**
 * \if ENGLISH
 * @brief Get the base zoom state (initial unzoomed state)
 * @return List of zoom states for all plots at the base level
 * @details Returns an empty list if the zoom stack is empty.
 * \endif
 * 
 * \if CHINESE
 * @brief 获取基础缩放状态（初始未缩放状态）
 * @return 基础级别所有绘图的缩放状态列表
 * @details 如果缩放堆栈为空，则返回空列表。
 * \endif
 */
QList< QwtPlotCanvasZoomState > QwtPlotCanvasZoomer::zoomBase() const
{
    if (m_data->zoomStack.isEmpty()) {
        return QList< QwtPlotCanvasZoomState >();
    }
    return m_data->zoomStack[ 0 ];
}

/**
 * \if ENGLISH
 * @brief Get the current zoom state
 * @return List of zoom states for all plots at the current zoom level
 * @details Returns an empty list if the zoom stack is empty.
 * \endif
 * 
 * \if CHINESE
 * @brief 获取当前缩放状态
 * @return 当前缩放级别所有绘图的缩放状态列表
 * @details 如果缩放堆栈为空，则返回空列表。
 * \endif
 */
QList< QwtPlotCanvasZoomState > QwtPlotCanvasZoomer::zoomState() const
{
    if (m_data->zoomStack.isEmpty()) {
        return QList< QwtPlotCanvasZoomState >();
    }
    return m_data->zoomStack[ m_data->zoomStateIndex ];
}

/**
 * \if ENGLISH
 * @brief Set the zoom base to the current axis ranges
 * @param[in] doReplot If true, call QwtPlot::replotAll() before capturing state
 * @details This method clears the zoom stack and sets the current view as the base state.
 *          It captures the axis ranges of the main plot and all parasite plots.
 * \endif
 * 
 * \if CHINESE
 * @brief 将缩放基础设置为当前坐标轴范围
 * @param[in] doReplot 如果为 true，在捕获状态前调用 QwtPlot::replotAll()
 * @details 此方法清除缩放堆栈并将当前视图设置为基础状态。
 *          它捕获主绘图和所有寄生绘图的坐标轴范围。
 * \endif
 */
void QwtPlotCanvasZoomer::setZoomBase(bool doReplot)
{
    QwtPlot* plt = plot();
    if (!plt) {
        return;
    }

    if (doReplot) {
        plt->replotAll();
    }

    m_data->zoomStack.clear();
    const QList< QwtPlot* > plts = plt->plotList(true);
    QList< QwtPlotCanvasZoomState > base;
    for (QwtPlot* p : plts) {
        base.append(QwtPlotCanvasZoomState::fromPlot(p));
    }
    m_data->zoomStack.push(base);
    m_data->zoomStateIndex = 0;
}

/**
 * \if ENGLISH
 * @brief Enable or disable automatic replot after zoom operations
 * @param[in] on If true, the plot will be automatically replotted after zoom
 * @details When enabled, the plot will automatically replot after each zoom operation.
 *          This is enabled by default.
 * \endif
 * 
 * \if CHINESE
 * @brief 启用或禁用缩放操作后的自动重绘
 * @param[in] on 如果为 true，缩放后绘图将自动重绘
 * @details 启用后，绘图将在每次缩放操作后自动重绘。
 *          默认启用。
 * \endif
 */
void QwtPlotCanvasZoomer::setAutoReplot(bool on)
{
    m_data->replot = on;
}

/**
 * \if ENGLISH
 * @brief Check if automatic replot is enabled
 * @return true if automatic replot is enabled
 * \endif
 * 
 * \if CHINESE
 * @brief 检查是否启用了自动重绘
 * @return 如果启用了自动重绘，则返回 true
 * \endif
 */
bool QwtPlotCanvasZoomer::isAutoReplot() const
{
    return m_data->replot;
}

/**
 * \if ENGLISH
 * @brief Navigate in the zoom stack by the specified offset
 * @param[in] offset Number of positions to move (negative = zoom out, positive = zoom in)
 * @details Moving by 0 resets to the base zoom level.
 *          The zoomed() signal is emitted if the zoom state changes.
 * \endif
 * 
 * \if CHINESE
 * @brief 按指定偏移量在缩放堆栈中导航
 * @param[in] offset 移动的位置数（负数 = 缩小，正数 = 放大）
 * @details 移动 0 会重置到基础缩放级别。
 *          如果缩放状态改变，将发出 zoomed() 信号。
 * \endif
 */
void QwtPlotCanvasZoomer::zoom(int offset)
{
    int newIndex;
    if (offset == 0) {
        newIndex = 0;
    } else {
        newIndex = m_data->zoomStateIndex + offset;
        newIndex = qBound(0, newIndex, m_data->zoomStack.count() - 1);
    }

    if (newIndex != static_cast< int >(m_data->zoomStateIndex)) {
        m_data->zoomStateIndex = newIndex;
        rescale();
        Q_EMIT zoomed(zoomState());
    }
}

/**
 * \if ENGLISH
 * @brief Append a new zoom state to the zoom stack
 * @param[in] rect List of zoom states for all plots
 * @details This method adds a new zoom state to the stack and makes it the current state.
 *          The zoomed() signal is emitted after the zoom is applied.
 * \endif
 * 
 * \if CHINESE
 * @brief 将新的缩放状态追加到缩放堆栈
 * @param[in] rect 所有绘图的缩放状态列表
 * @details 此方法将新的缩放状态添加到堆栈并使其成为当前状态。
 *          应用缩放后将发出 zoomed() 信号。
 * \endif
 */
void QwtPlotCanvasZoomer::appendZoom(const QList< QwtPlotCanvasZoomState >& rect)
{
    m_data->zoomStack.push(rect);
    m_data->zoomStateIndex = m_data->zoomStack.count() - 1;
    rescale();
    Q_EMIT zoomed(rect);
}

void QwtPlotCanvasZoomer::rescale()
{
    const QList< QwtPlotCanvasZoomState >& states = m_data->zoomStack[ m_data->zoomStateIndex ];
    for (const QwtPlotCanvasZoomState& state : states) {
        state.apply();
    }
    if (m_data->replot) {
        QwtPlot* plt = plot();
        plt->replotAll();
    }
}

void QwtPlotCanvasZoomer::widgetMouseReleaseEvent(QMouseEvent* me)
{
    if (mouseMatch(MouseSelect2, me))
        zoom(0);
    else if (mouseMatch(MouseSelect3, me))
        zoom(-1);
    else if (mouseMatch(MouseSelect6, me))
        zoom(+1);
    else
        QwtPicker::widgetMouseReleaseEvent(me);
}

void QwtPlotCanvasZoomer::widgetKeyPressEvent(QKeyEvent* ke)
{
    if (!isActive()) {
        if (keyMatch(KeyUndo, ke))
            zoom(-1);
        else if (keyMatch(KeyRedo, ke))
            zoom(+1);
        else if (keyMatch(KeyHome, ke))
            zoom(0);
    }

    QwtPicker::widgetKeyPressEvent(ke);
}

bool QwtPlotCanvasZoomer::accept(QPolygon& pa) const
{
    if (pa.count() < 2)
        return false;

    QRect rect = QRect(pa.first(), pa.last());
    rect       = rect.normalized();

    const int minSize = 2;
    if (rect.width() < minSize && rect.height() < minSize)
        return false;

    const int minZoomSize = 11;

    const QPoint center = rect.center();
    rect.setSize(rect.size().expandedTo(QSize(minZoomSize, minZoomSize)));
    rect.moveCenter(center);

    pa.resize(2);
    pa[ 0 ] = rect.topLeft();
    pa[ 1 ] = rect.bottomRight();

    return true;
}

void QwtPlotCanvasZoomer::begin()
{
    if (m_data->maxStackDepth >= 0) {
        if (m_data->zoomStateIndex >= uint(m_data->maxStackDepth))
            return;
    }

    QwtPicker::begin();
}

bool QwtPlotCanvasZoomer::end(bool ok)
{
    ok = QwtPicker::end(ok);
    if (!ok) {
        return false;
    }

    const QPolygon& pa = selection();
    if (pa.count() < 2) {
        return false;
    }

    QRect rect = QRect(pa.first(), pa.last());
    rect       = rect.normalized();

    // 直接将画布矩形转换为四个坐标轴的缩放状态
    QList< QwtPlotCanvasZoomState > newState = canvasRectToZoomStateList(rect);

    appendZoom(newState);
    return true;
}
