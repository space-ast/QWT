/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#include "qwt_figure_widget_overlay.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHash>
#include <QApplication>
#include <QDebug>
#include <QPainter>
// std
#include <algorithm>
// qwt
#include "qwt_algorithm.hpp"
#include "qwt_figure.h"
#include "qwt_plot.h"
#include "qwt_qt5qt6_compat.hpp"

#ifndef QwtFigureWidgetOverlay_DEBUG_PRINT
#define QwtFigureWidgetOverlay_DEBUG_PRINT 0
#endif

class QwtFigureWidgetOverlay::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtFigureWidgetOverlay)
public:
    PrivateData(QwtFigureWidgetOverlay* p);

public:
    QwtFigureWidgetOverlay::BuiltInFunctions mFuntion;  ///< 内置功能
    QPoint mLastMousePressPos { 0, 0 };                 ///< 记录最后一次窗口移动的坐标
    QBrush mContorlPointBrush { Qt::blue };             ///< 绘制chart2d在编辑模式下控制点的画刷
    QPen mBorderPen { Qt::blue };                       ///< 绘制chart2d在编辑模式下的画笔
    bool mIsStartResize { false };                      ///< 标定开始进行缩放
    QWidget* mActiveWidget { nullptr };                 /// 标定当前激活的窗口，如果没有就为nullptr
    QRectF mOldNormRect;                                ///< 保存旧的窗口位置，用于redo/undo
    QRectF mWillSetNormRect;                            ///< 将要设置的正则尺寸
    QSize mControlPointSize { 8, 8 };                   ///< 控制点大小
    QwtFigureWidgetOverlay::ControlType mControlType { QwtFigureWidgetOverlay::OutSide };  ///< 记录当前缩放窗口的位置情况

    bool mShowPrecentText { true };  ///< 显示占比文字
};

QwtFigureWidgetOverlay::PrivateData::PrivateData(QwtFigureWidgetOverlay* p) : q_ptr(p)
{
    mFuntion.setFlag(FunSelectCurrentPlot, true);
    mFuntion.setFlag(FunResizePlot, true);
}

//----------------------------------------------------
// QwtFigureWidgetOverlay
//----------------------------------------------------

/**
 * @if ENGLISH
 * @brief Constructor
 * @param[in] fig The QwtFigure to attach to
 * @note Passing nullptr is not allowed
 * @endif
 *
 * @if CHINESE
 * @brief 构造函数
 * @param[in] fig 要附加的QwtFigure
 * @note 构造函数不允许传入nullptr
 * @endif
 */
QwtFigureWidgetOverlay::QwtFigureWidgetOverlay(QwtFigure* fig) : QwtWidgetOverlay(fig), QWT_PIMPL_CONSTRUCT
{
    Q_ASSERT(fig);

    QwtPlot* gca = fig->currentAxes();
    if (gca) {
        setActiveWidget(gca);
    } else {
        selectNextPlot();
        if (!currentActiveWidget()) {
            selectNextWidget();
        }
    }
    connect(fig, &QwtFigure::axesRemoved, this, &QwtFigureWidgetOverlay::onAxesRemove);
    setMouseTracking(true);
    setTransparentForMouseEvents(false);  // 这里对鼠标不透明，避免被绘图的坐标轴事件截取
}

QwtFigureWidgetOverlay::~QwtFigureWidgetOverlay()
{
}

/**
 * @if ENGLISH
 * @brief Returns the associated QwtFigure
 * @return The parent QwtFigure
 * @endif
 *
 * @if CHINESE
 * @brief 返回关联的QwtFigure
 * @return 父级QwtFigure
 * @endif
 */
QwtFigure* QwtFigureWidgetOverlay::figure() const
{
    return static_cast< QwtFigure* >(parent());
}

/**
 * @if ENGLISH
 * @brief Sets whether the overlay is transparent for mouse events
 * @param[in] on True to make transparent, false otherwise
 * @endif
 *
 * @if CHINESE
 * @brief 设置是否对鼠标事件透明
 * @param[in] on true表示透明，false表示不透明
 * @endif
 */
void QwtFigureWidgetOverlay::setTransparentForMouseEvents(bool on)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, on);
}

/**
 * @if ENGLISH
 * @brief Checks if the overlay is transparent for mouse events
 * @return True if transparent, false otherwise
 * @endif
 *
 * @if CHINESE
 * @brief 判断是否对鼠标事件透明
 * @return true表示透明，false表示不透明
 * @endif
 */
bool QwtFigureWidgetOverlay::isTransparentForMouseEvents() const
{
    return testAttribute(Qt::WA_TransparentForMouseEvents);
}

/**
 * @brief根据范围获取鼠标图标
 * @param rr
 * @return 鼠标图标
 */
Qt::CursorShape QwtFigureWidgetOverlay::controlTypeToCursor(QwtFigureWidgetOverlay::ControlType rr)
{
    switch (rr) {
    case ControlLineTop:
    case ControlLineBottom:
        return (Qt::SizeVerCursor);

    case ControlLineLeft:
    case ControlLineRight:
        return (Qt::SizeHorCursor);

    case ControlPointTopLeft:
    case ControlPointBottomRight:
        return (Qt::SizeFDiagCursor);

    case ControlPointTopRight:
    case ControlPointBottomLeft:
        return (Qt::SizeBDiagCursor);

    case Inner:
        return (Qt::SizeAllCursor);

    default:
        break;
    }
    return (Qt::ArrowCursor);
}

/**
 * @if ENGLISH
 * @brief Determines control type based on point position relative to rectangle
 * @param[in] pos The point position
 * @param[in] region The rectangle region
 * @param[in] err The error tolerance
 * @return The control type at the given position
 * @endif
 *
 * @if CHINESE
 * @brief 根据点和矩形的关系返回控制类型
 * @param[in] pos 点的位置
 * @param[in] region 矩形区域
 * @param[in] err 允许误差
 * @return 给定位置的控制类型
 * @endif
 */
QwtFigureWidgetOverlay::ControlType
QwtFigureWidgetOverlay::getPositionControlType(const QPoint& pos, const QRect& region, int err)
{
    if (!region.adjusted(-err, -err, err, err).contains(pos)) {
        return (OutSide);
    }
    if (pos.x() < (region.left() + err)) {
        if (pos.y() < region.top() + err) {
            return (ControlPointTopLeft);
        } else if (pos.y() > region.bottom() - err) {
            return (ControlPointBottomLeft);
        }
        return (ControlLineLeft);
    } else if (pos.x() > (region.right() - err)) {
        if (pos.y() < region.top() + err) {
            return (ControlPointTopRight);
        } else if (pos.y() > region.bottom() - err) {
            return (ControlPointBottomRight);
        }
        return (ControlLineRight);
    } else if (pos.y() < (region.top() + err)) {
        if (pos.x() < region.left() + err) {
            return (ControlPointTopLeft);
        } else if (pos.x() > region.right() - err) {
            return (ControlPointTopRight);
        }
        return (ControlLineTop);
    } else if (pos.y() > region.bottom() - err) {
        if (pos.x() < region.left() + err) {
            return (ControlPointBottomLeft);
        } else if (pos.x() > region.right() - err) {
            return (ControlPointBottomRight);
        }
        return (ControlLineBottom);
    }
    return (Inner);
}

/**
 * @if ENGLISH
 * @brief Checks if a point is on the edge of a rectangle
 * @param[in] pos The point position
 * @param[in] region The rectangle region
 * @param[in] err The error tolerance
 * @return True if the point is on the edge
 * @endif
 *
 * @if CHINESE
 * @brief 判断点是否在矩形区域的边缘
 * @param[in] pos 点的位置
 * @param[in] region 矩形区域
 * @param[in] err 允许误差
 * @return 如果符合边缘条件，返回true
 * @endif
 */
bool QwtFigureWidgetOverlay::isPointInRectEdget(const QPoint& pos, const QRect& region, int err)
{
    if (!region.adjusted(-err, -err, err, err).contains(pos)) {
        return (false);
    }
    if ((pos.x() < (region.left() - err)) && (pos.x() < (region.left() + err))) {
        return (true);
    } else if ((pos.x() > (region.right() - err)) && (pos.x() < (region.right() + err))) {
        return (true);
    } else if ((pos.y() > (region.top() - err)) && (pos.y() < (region.top() + err))) {
        return (true);
    } else if ((pos.y() > region.bottom() - err) && (pos.y() < region.bottom() + err)) {
        return (true);
    }
    return (false);
}

/**
 * @if ENGLISH
 * @brief Enables or disables built-in functions
 * @param[in] flag The function flag to set
 * @param[in] on True to enable, false to disable
 * @sa QwtFigureWidgetOverlay::BuiltInFunctionsFlag
 * @endif
 *
 * @if CHINESE
 * @brief 设置内置功能的开关
 * @param[in] flag 要设置的功能标志
 * @param[in] on true表示开启，false表示关闭
 * @sa QwtFigureWidgetOverlay::BuiltInFunctionsFlag
 * @endif
 */
void QwtFigureWidgetOverlay::setBuiltInFunctionsEnable(BuiltInFunctionsFlag flag, bool on)
{
    m_data->mFuntion.setFlag(flag, on);
}

/**
 * @if ENGLISH
 * @brief Tests if a built-in function is enabled
 * @param[in] flag The function flag to test
 * @return True if enabled, false otherwise
 * @endif
 *
 * @if CHINESE
 * @brief 判断当前的功能开关状态
 * @param[in] flag 要测试的功能标志
 * @return true表示开启，false表示关闭
 * @endif
 */
bool QwtFigureWidgetOverlay::testBuiltInFunctions(BuiltInFunctionsFlag flag) const
{
    return m_data->mFuntion.testFlag(flag);
}

/**
 * @if ENGLISH
 * @brief Checks if there is an active widget
 * @return True if there is an active widget, false otherwise
 * @endif
 *
 * @if CHINESE
 * @brief 判断当前是否有激活的窗口
 * @return true表示有激活窗口，false表示没有
 * @endif
 */
bool QwtFigureWidgetOverlay::hasActiveWidget() const
{
    return (m_data->mActiveWidget != nullptr);
}

/**
 * @if ENGLISH
 * @brief Checks if currently resizing
 * @return True if resizing, false otherwise
 * @endif
 *
 * @if CHINESE
 * @brief 判断是否正在改变尺寸
 * @return true表示正在改变尺寸，false表示没有
 * @endif
 */
bool QwtFigureWidgetOverlay::isResizing() const
{
    return m_data->mIsStartResize;
}

/**
 * @if ENGLISH
 * @brief Sets the border pen
 * @param[in] p The pen to set
 * @endif
 *
 * @if CHINESE
 * @brief 设置边框的画笔
 * @param[in] p 要设置的画笔
 * @endif
 */
void QwtFigureWidgetOverlay::setBorderPen(const QPen& p)
{
    m_data->mBorderPen = p;
}

/**
 * @if ENGLISH
 * @brief Returns the border pen
 * @return The current border pen
 * @endif
 *
 * @if CHINESE
 * @brief 返回边框的画笔
 * @return 当前的边框画笔
 * @endif
 */
QPen QwtFigureWidgetOverlay::borderPen() const
{
    return m_data->mBorderPen;
}

/**
 * @if ENGLISH
 * @brief Sets the control point brush
 * @param[in] b The brush to set
 * @endif
 *
 * @if CHINESE
 * @brief 设置控制点的填充画刷
 * @param[in] b 要设置的画刷
 * @endif
 */
void QwtFigureWidgetOverlay::setControlPointBrush(const QBrush& b)
{
    m_data->mContorlPointBrush = b;
}

/**
 * @if ENGLISH
 * @brief Returns the control point brush
 * @return The current control point brush
 * @endif
 *
 * @if CHINESE
 * @brief 返回控制点的填充画刷
 * @return 当前的控制点画刷
 * @endif
 */
QBrush QwtFigureWidgetOverlay::controlPointBrush() const
{
    return m_data->mContorlPointBrush;
}

/**
 * @if ENGLISH
 * @brief Sets the control point size
 * @param[in] c The size to set
 * @endif
 *
 * @if CHINESE
 * @brief 设置控制点尺寸
 * @param[in] c 要设置的尺寸
 * @endif
 */
void QwtFigureWidgetOverlay::setControlPointSize(const QSize& c)
{
    m_data->mControlPointSize = c;
}

/**
 * @if ENGLISH
 * @brief Returns the control point size
 * @return The current control point size (default 8x8)
 * @endif
 *
 * @if CHINESE
 * @brief 返回控制点尺寸
 * @return 当前的控制点尺寸（默认8x8）
 * @endif
 */
QSize QwtFigureWidgetOverlay::controlPointSize() const
{
    return m_data->mControlPointSize;
}

/**
 * @if ENGLISH
 * @brief Selects the next widget as the active widget
 * @param[in] forward True for forward selection, false for backward
 * @endif
 *
 * @if CHINESE
 * @brief 选择下一个窗口作为激活窗体
 * @param[in] forward true表示向前选择，false表示向后选择
 * @endif
 */
void QwtFigureWidgetOverlay::selectNextWidget(bool forward)
{
    QList< QWidget* > ws = figure()->findChildren< QWidget* >("", Qt::FindDirectChildrenOnly);
    ws.removeAll(this);
    if (ws.isEmpty()) {
        setActiveWidget(nullptr);
        return;
    }
    // 删除寄生轴
    auto it = std::remove_if(ws.begin(), ws.end(), [](QWidget* w) -> bool {
        if (QwtPlot* plot = qobject_cast< QwtPlot* >(w)) {
            if (plot->isParasitePlot()) {
                return true;
            }
        }
        return false;
    });
    if (it != ws.end()) {
        ws.erase(it, ws.end());  // 删除末尾的“无效”元素,也就是寄生轴都删除
    }
    // 这时ws都是可选中的窗口
    auto nextIt = qwtSelectNextIterator(ws.begin(), ws.end(), currentActiveWidget(), forward);
    setActiveWidget((nextIt != ws.end()) ? *nextIt : nullptr);
}

/**
 * @if ENGLISH
 * @brief Selects the next plot as the active widget
 * @param[in] forward True for forward selection, false for backward
 * @endif
 *
 * @if CHINESE
 * @brief 选择下一个绘图作为激活窗体
 * @param[in] forward true表示向前选择，false表示向后选择
 * @endif
 */
void QwtFigureWidgetOverlay::selectNextPlot(bool forward)
{
    // 此函数不会返回寄生轴
    QList< QwtPlot* > ws = figure()->allAxes();
    if (ws.isEmpty()) {
        setActiveWidget(nullptr);
        return;
    }
    // 转换当前元素类型并获取下一个迭代器
    QwtPlot* current = qobject_cast< QwtPlot* >(currentActiveWidget());
    auto nextIt      = qwtSelectNextIterator(ws.begin(), ws.end(), current, forward);
    setActiveWidget((nextIt != ws.end()) ? *nextIt : nullptr);
}

/**
 * @if ENGLISH
 * @brief Returns the current active widget
 * @return The current active widget, nullptr if none
 * @endif
 *
 * @if CHINESE
 * @brief 获取当前激活的窗体
 * @return 当前激活的窗体，如果没有则为nullptr
 * @endif
 */
QWidget* QwtFigureWidgetOverlay::currentActiveWidget() const
{
    return m_data->mActiveWidget;
}

/**
 * @if ENGLISH
 * @brief Returns the current active plot
 * @return The current active plot, nullptr if none or not a plot
 * @endif
 *
 * @if CHINESE
 * @brief 获取当前激活的绘图
 * @return 当前激活的绘图，如果没有或不是绘图则为nullptr
 * @endif
 */
QwtPlot* QwtFigureWidgetOverlay::currentActivePlot() const
{
    return qobject_cast< QwtPlot* >(m_data->mActiveWidget);
}

/**
 * @if ENGLISH
 * @brief Shows or hides percentage text
 * @param[in] on True to show, false to hide
 * @endif
 *
 * @if CHINESE
 * @brief 显示或隐藏占比数值
 * @param[in] on true表示显示，false表示隐藏
 * @endif
 */
void QwtFigureWidgetOverlay::showPercentText(bool on)
{
    m_data->mShowPrecentText = on;
    updateOverlay();
}

/**
 * @if ENGLISH
 * @brief Cancels the operation
 * @return True on success
 * @note This function emits finished(true) signal. Override should call this explicitly.
 * @code
 * bool MyFigureWidgetOverlay::cancel(){
 *    ...
 *    // Explicit call to trigger finished(true)
 *    QwtFigureWidgetOverlay::cancel();
 *    return true;
 * }
 * @endcode
 * @endif
 *
 * @if CHINESE
 * @brief 取消操作
 * @return 成功返回true
 * @note 此函数会发射finished(true)信号，重写应该显式调用
 * @code
 * bool MyFigureWidgetOverlay::cancel(){
 *    ...
 *    // 显式调用，用以触发finished(true)
 *    QwtFigureWidgetOverlay::cancel();
 *    return true;
 * }
 * @endcode
 * @endif
 */
bool QwtFigureWidgetOverlay::cancel()
{
    Q_EMIT finished(true);
    return true;
}

/**
 * @if ENGLISH
 * @brief Sets the current active widget
 * @param[in] w The widget to set as active
 * @note If w is the same as current active widget, no action is taken
 * @note This function emits activeWidgetChanged signal
 * @sa activeWidgetChanged
 * @endif
 *
 * @if CHINESE
 * @brief 设置当前激活的窗口
 * @param[in] w 要设置为激活的窗口
 * @note 如果w和当前的activePlot一样，不做任何动作
 * @note 此函数会发射activeWidgetChanged信号
 * @sa activeWidgetChanged
 * @endif
 */
void QwtFigureWidgetOverlay::setActiveWidget(QWidget* w)
{
    QWidget* oldact = currentActiveWidget();
    if (w == oldact) {
        // 避免嵌套
        return;
    }
    m_data->mActiveWidget = w;
    updateOverlay();
    Q_EMIT activeWidgetChanged(oldact, w);
}

void QwtFigureWidgetOverlay::drawOverlay(QPainter* p) const
{
    if (!hasActiveWidget()) {
        return;
    }
    // 对于激活的窗口，绘制到四周的距离提示线
    p->save();
    if (m_data->mIsStartResize && m_data->mFuntion.testFlag(FunResizePlot)) {
        // 在resize状态，绘制控制线
        drawResizeingControlLine(p, m_data->mWillSetNormRect);
    } else {
        drawActiveWidget(p, currentActiveWidget());
    }
    p->restore();
}

QRegion QwtFigureWidgetOverlay::maskHint() const
{
    return (figure()->rect());
}

/**
 * @brief 绘制激活的窗口
 *
 * 通过继承此函数可改变绘制的方式，默认绘制会调用@ref drawControlLine 函数
 * @param painter
 * @param activeW
 */
void QwtFigureWidgetOverlay::drawActiveWidget(QPainter* painter, QWidget* activeW) const
{
    const QRect& chartRect      = activeW->frameGeometry();
    const QRectF& normalPercent = figure()->widgetNormRect(activeW);
    drawControlLine(painter, chartRect, normalPercent);
}

/**
 * @brief 绘制resize变换的橡皮筋控制线
 *
 * 通过继承此函数可改变绘制的方式，默认绘制会调用@ref drawControlLine 函数
 * @param painter
 * @param willSetNormRect
 */
void QwtFigureWidgetOverlay::drawResizeingControlLine(QPainter* painter, const QRectF& willSetNormRect) const
{
    QRect actualRect = figure()->calcActualRect(willSetNormRect);
    drawControlLine(painter, actualRect, willSetNormRect);
}

/**
 * @brief 绘制控制线
 * @param painter
 * @param actualRect 真实尺寸
 * @param normRect 归一化尺寸
 */
void QwtFigureWidgetOverlay::drawControlLine(QPainter* painter, const QRect& actualRect, const QRectF& normRect) const
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(m_data->mBorderPen);
    QRect edgetRect = actualRect.adjusted(1, 1, -1, -1);

    // 绘制矩形边框
    painter->drawRect(edgetRect);
    // 绘制边框到figure四周
    QPen linePen(m_data->mBorderPen);

    linePen.setStyle(Qt::DotLine);
    painter->setPen(linePen);
    QPoint center = actualRect.center();

    painter->drawLine(center.x(), 0, center.x(), actualRect.top());            // top
    painter->drawLine(center.x(), actualRect.bottom(), center.x(), height());  // bottom
    painter->drawLine(0, center.y(), actualRect.left(), center.y());           // left
    painter->drawLine(actualRect.right(), center.y(), width(), center.y());    // right
    // 绘制顶部数据
    QFontMetrics fm = painter->fontMetrics();
    // top text
    QString percentText = QString::number(normRect.y() * 100, 'g', 2) + "%";
    QRectF textRect     = fm.boundingRect(percentText);
    textRect.moveTopLeft(QPoint(center.x(), 0));
    painter->drawText(textRect, Qt::AlignCenter, percentText);
    // left
    percentText = QString::number(normRect.x() * 100, 'g', 2) + "%";
    textRect    = fm.boundingRect(percentText);
    textRect.moveBottomLeft(QPoint(0, center.y()));
    painter->drawText(textRect, Qt::AlignCenter, percentText);

    //    painter->drawText(QPointF(0, actualRect.y()), QString::number(percent.x(), 'g', 2));
    // 绘制四个角落
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_data->mContorlPointBrush);
    QRect connerRect(0, 0, m_data->mControlPointSize.width(), m_data->mControlPointSize.height());
    QPoint offset = QPoint(m_data->mControlPointSize.width() / 2, m_data->mControlPointSize.height() / 2);
    connerRect.moveTo(edgetRect.topLeft() - offset);
    painter->drawRect(connerRect);
    connerRect.moveTo(edgetRect.topRight() - offset);
    painter->drawRect(connerRect);
    connerRect.moveTo(edgetRect.bottomLeft() - offset);
    painter->drawRect(connerRect);
    connerRect.moveTo(edgetRect.bottomRight() - offset);
    painter->drawRect(connerRect);
}

/**
 * @brief 开始变换的辅助函数，此函数会记录开始变换的状态
 * @param controlType
 * @param pos
 */
void QwtFigureWidgetOverlay::startResize(QwtFigureWidgetOverlay::ControlType controlType, const QPoint& pos)
{
    QWT_D(d);
    if (!d->mActiveWidget) {
        return;
    }

    QwtFigure* fig = figure();
    Q_ASSERT(fig);

    d->mOldNormRect       = fig->widgetNormRect(d->mActiveWidget);
    d->mLastMousePressPos = pos;
    d->mIsStartResize     = true;
    d->mControlType       = controlType;
    d->mWillSetNormRect   = QRectF();

    //! 捕获鼠标，确保所有鼠标事件都发送到这个窗口
    //! 这个函数是关键，避免鼠标移动的时候被别的窗口捕获掉鼠标，导致无法接收到release事件
    //! grabMouse可以:
    //! - 强制所有鼠标事件（移动、点击、释放等）都发送到调用该函数的窗口部件
    //! - 忽略鼠标的实际位置，即使鼠标移到了其他窗口或屏幕边缘
    //! - 确保鼠标事件链的完整性
    //! 此函数一定要releaseMouse，（releaseMouse在mouseReleaseEvent执行）
    //!
    //! 如果没有这个函数，鼠标移动到了底层绘图窗口或其他子窗口上，鼠标事件可能被这些窗口截获
    //! QwtFigureWidgetOverlay收不到 mouseReleaseEvent，导致状态卡在"调整中"
    // 上面注释改为英文
    grabMouse();
}

void QwtFigureWidgetOverlay::mousePressEvent(QMouseEvent* me)
{
#if QwtFigureWidgetOverlay_DEBUG_PRINT
    qDebug() << "QwtFigureWidgetOverlay::mousePressEvent(" << qwt::compat::eventPos(me) << ")";
#endif
    if (me->button() != Qt::LeftButton) {
        QwtWidgetOverlay::mousePressEvent(me);
        return;
    }

    QWT_D(d);
    const QPoint pos = qwt::compat::eventPos(me);

    // 获取点击位置的窗口（按z序）
    const QList< QwtPlot* > plots = figure()->allAxes(true);
    QWidget* hitPlot              = nullptr;
    for (QWidget* w : plots) {
        if (w->frameGeometry().contains(pos, true)) {
            hitPlot = w;
            break;
        }
    }

    // 重置之前的调整状态
    if (d->mIsStartResize) {
        d->mIsStartResize   = false;
        d->mWillSetNormRect = QRectF();
        releaseMouse();  // 确保释放鼠标捕获
    }

    // ========== 步骤1：检查是否点击了激活窗口的控制点 ==========
    // 这里放在第一步，避免点击控制点改变尺寸会被判断为切换窗口
    if (d->mActiveWidget && testBuiltInFunctions(FunResizePlot)) {
        ControlType ct = getPositionControlType(pos, d->mActiveWidget->frameGeometry(), 4);

        // 只有点击到真正的控制点（边缘和角落）才启动resize
        if (ct != OutSide) {
            // 点击了激活窗口的内部，但hitplot也存在，这种就是点击到了图中图，激活窗口是大图，hitplot是大图里的小图
            if (ct == Inner) {
                if (hitPlot && hitPlot != d->mActiveWidget) {
                    setActiveWidget(hitPlot);
                    me->accept();
                    return;
                }
            }
            startResize(ct, pos);
            me->accept();
            return;
        }
        // 如果是 Inner，继续执行后续逻辑（可能切换窗口）
    }

    // ========== 步骤2：处理窗口切换 ==========
    if (hitPlot) {
        // 点击了某个窗口
        if (hitPlot != d->mActiveWidget) {
            setActiveWidget(hitPlot);
            // 如果点击的就是当前激活窗口内部，保持激活状态（不切换）
            me->accept();
            return;
        } else {
            // 点击就是激活的窗口,这里不处理，下放
            me->ignore();
            return;
        }
    }

    // ========== 步骤3：点击空白处 ==========
    if (d->mActiveWidget) {
        // 有激活窗口时点击空白，取消激活
        setActiveWidget(nullptr);
        updateOverlay();
        me->accept();
    } else {
        // 无激活窗口时点击空白，让事件继续传递
        me->ignore();
    }
}

void QwtFigureWidgetOverlay::mouseMoveEvent(QMouseEvent* me)
{
    QWT_D(d);
#if QwtFigureWidgetOverlay_DEBUG_PRINT
    qDebug() << "QwtFigureWidgetOverlay::mouseMoveEvent(" << qwt::compat::eventPos(me) << "),have active widget"
             << (d->mActiveWidget != nullptr) << ",have FunResizePlot=" << testBuiltInFunctions(FunResizePlot)
             << ",ControlType=" << d->mControlType;
#endif
    QWidget* activeW = d->mActiveWidget;
    if (!testBuiltInFunctions(FunResizePlot)) {
        // 没有resize plot 功能，退出
        return QwtWidgetOverlay::mouseMoveEvent(me);
    }
    if (!activeW) {
        // 没有激活窗口，更新光标并传递事件
        unsetCursor();
        QwtWidgetOverlay::mouseMoveEvent(me);
        return;
    }

    const QPoint pos = qwt::compat::eventPos(me);

    if (d->mIsStartResize) {
        // 开始变换
        QwtFigure* fig = figure();
        Q_ASSERT(fig);
        const QRectF& oldNormRect = d->mOldNormRect;
        QPoint offset             = pos - d->mLastMousePressPos;

        switch (d->mControlType) {
        case ControlLineTop: {
            //  计算offset.y()占高度比例
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            // 要使用figure计算归一化坐标
            QRectF normRect = oldNormRect;
            normRect.setY(oldNormRect.y() + dh);
            normRect.setHeight(oldNormRect.height() - dh);
            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlLineBottom: {
            //  计算offset.y()占高度比例
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            // 要使用figure计算归一化坐标
            QRectF normRect = oldNormRect;
            normRect.setHeight(oldNormRect.height() + dh);
            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlLineLeft: {
            //  计算offset.x()占宽度比例
            qreal dw        = static_cast< qreal >(offset.x()) / fig->width();
            QRectF normRect = oldNormRect;
            normRect.setX(oldNormRect.x() + dw);
            normRect.setWidth(oldNormRect.width() - dw);
            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlLineRight: {
            //  计算offset.x()占宽度比例
            qreal dw        = static_cast< qreal >(offset.x()) / fig->width();
            QRectF normRect = oldNormRect;
            normRect.setWidth(oldNormRect.width() + dw);
            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlPointTopLeft: {
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            qreal dw = static_cast< qreal >(offset.x()) / fig->width();

            QRectF normRect = oldNormRect;
            normRect.setX(oldNormRect.x() + dw);
            normRect.setY(oldNormRect.y() + dh);
            normRect.setWidth(oldNormRect.width() - dw);
            normRect.setHeight(oldNormRect.height() - dh);

            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlPointTopRight: {
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            qreal dw = static_cast< qreal >(offset.x()) / fig->width();

            QRectF normRect = oldNormRect;
            normRect.setY(oldNormRect.y() + dh);
            normRect.setWidth(oldNormRect.width() + dw);
            normRect.setHeight(oldNormRect.height() - dh);

            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlPointBottomLeft: {
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            qreal dw = static_cast< qreal >(offset.x()) / fig->width();

            QRectF normRect = oldNormRect;
            normRect.setX(oldNormRect.x() + dw);
            normRect.setWidth(oldNormRect.width() - dw);
            normRect.setHeight(oldNormRect.height() + dh);

            d->mWillSetNormRect = normRect;
            break;
        }

        case ControlPointBottomRight: {
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            qreal dw = static_cast< qreal >(offset.x()) / fig->width();

            QRectF normRect = oldNormRect;
            normRect.setWidth(oldNormRect.width() + dw);
            normRect.setHeight(oldNormRect.height() + dh);

            d->mWillSetNormRect = normRect;
            break;
        }

        case Inner: {
            qreal dh = static_cast< qreal >(offset.y()) / fig->height();
            qreal dw = static_cast< qreal >(offset.x()) / fig->width();

            QRectF normRect     = oldNormRect.adjusted(dw, dh, dw, dh);
            d->mWillSetNormRect = normRect;
            Q_EMIT widgetNormGeometryChanged(d->mActiveWidget, d->mOldNormRect, d->mWillSetNormRect);
            break;
        }

        default:
            break;
        }
        updateOverlay();
    } else {
        // 没开始变换，则更新光标
        ControlType ct = getPositionControlType(qwt::compat::eventPos(me), activeW->frameGeometry(), 4);

        // 说明控制点变更
        if (ct == OutSide) {
            unsetCursor();
            me->ignore();  // 让事件继续传递
        } else {
            Qt::CursorShape cur = controlTypeToCursor(ct);
            setCursor(cur);
            me->accept();
        }
    }
}

void QwtFigureWidgetOverlay::mouseReleaseEvent(QMouseEvent* me)
{
#if QwtFigureWidgetOverlay_DEBUG_PRINT
    qDebug() << "QwtFigureWidgetOverlay::onMouseReleaseEvent" << me->pos();
#endif
    QWT_D(d);
    if (!testBuiltInFunctions(FunResizePlot)) {
        // 没有resize plot 功能，退出
        return QwtWidgetOverlay::mouseReleaseEvent(me);
    }
    if (me->button() == Qt::LeftButton && d->mIsStartResize) {
        // 结束调整尺寸操作
        d->mIsStartResize = false;

        if (d->mActiveWidget && d->mWillSetNormRect.isValid()) {
            Q_EMIT widgetNormGeometryChanged(d->mActiveWidget, d->mOldNormRect, d->mWillSetNormRect);
        }

        d->mWillSetNormRect = QRectF();
        updateOverlay();

        //! 由于在startResize时捕获了鼠标，因此，这里必须释放鼠标
        releaseMouse();
        me->accept();
        return;
    }

    // 其他情况传递给父类处理
    QwtWidgetOverlay::mouseReleaseEvent(me);
}

void QwtFigureWidgetOverlay::keyPressEvent(QKeyEvent* ke)
{
    switch (ke->key()) {
    case Qt::Key_Return: {
        selectNextWidget(true);
        ke->accept();
    } break;

    case Qt::Key_Up:
    case Qt::Key_Left: {
        selectNextWidget(true);
        ke->accept();
    } break;

    case Qt::Key_Right:
    case Qt::Key_Down: {
        selectNextWidget(false);
        ke->accept();
    case Qt::Key_Escape:
        if (cancel()) {
            hide();
        }
        ke->accept();
    } break;

    default:
        break;
    }
    QwtWidgetOverlay::keyPressEvent(ke);
}

void QwtFigureWidgetOverlay::onAxesRemove(QwtPlot* removedAxes)
{
    if (m_data->mActiveWidget == removedAxes) {
        m_data->mActiveWidget = nullptr;
    }
}
