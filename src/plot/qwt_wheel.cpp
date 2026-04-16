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

#include "qwt_wheel.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include "qwt_utils.h"

#include <qevent.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qelapsedtimer.h>
#include <qmath.h>

class QwtWheel::PrivateData
{
public:
    PrivateData()
        : orientation(Qt::Horizontal)
        , viewAngle(175.0)
        , totalAngle(360.0)
        , tickCount(10)
        , wheelBorderWidth(2)
        , borderWidth(2)
        , wheelWidth(20)
        , mouseOffset(0.0)
        , updateInterval(50)
        , mass(0.0)
        , timerId(0)
        , speed(0.0)
        , mouseValue(0.0)
        , flyingValue(0.0)
        , minimum(0.0)
        , maximum(100.0)
        , singleStep(1.0)
        , pageStepCount(1)
        , value(0.0)
        , isScrolling(false)
        , tracking(true)
        , stepAlignment(true)
        , pendingValueChanged(false)
        , inverted(false)
        , wrapping(false)
    {
    }

    Qt::Orientation orientation;
    double viewAngle;
    double totalAngle;
    int tickCount;
    int wheelBorderWidth;
    int borderWidth;
    int wheelWidth;

    double mouseOffset;

    int updateInterval;
    double mass;

    // for the flying wheel effect
    int timerId;
    QElapsedTimer timer;
    double speed;
    double mouseValue;
    double flyingValue;

    double minimum;
    double maximum;

    double singleStep;
    int pageStepCount;

    double value;

    bool isScrolling;
    bool tracking;
    bool stepAlignment;
    bool pendingValueChanged;  // when not tracking
    bool inverted;
    bool wrapping;
};

//! Constructor
QwtWheel::QwtWheel(QWidget* parent) : QWidget(parent)
{
    m_data = new PrivateData;

    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

//! Destructor
QwtWheel::~QwtWheel()
{
    delete m_data;
}

/*!
   \if ENGLISH
   \brief En/Disable tracking

   If tracking is enabled (the default), the wheel emits the valueChanged()
   signal while the wheel is moving. If tracking is disabled, the wheel
   emits the valueChanged() signal only when the wheel movement is terminated.

   The wheelMoved() signal is emitted regardless id tracking is enabled or not.

   \param enable On/Off
   \sa isTracking()
   \endif
   *
   \if CHINESE
   \brief 启用/禁用跟踪

   如果启用了跟踪（默认），轮在移动时会发出 valueChanged() 信号。
   如果禁用了跟踪，轮只在移动终止时才发出 valueChanged() 信号。

   wheelMoved() 信号无论是否启用跟踪都会发出。

   \param enable 开/关
   \sa isTracking()
   \endif
 */
void QwtWheel::setTracking(bool enable)
{
    m_data->tracking = enable;
}

/*!
   \if ENGLISH
   \return True, when tracking is enabled
   \sa setTracking(), valueChanged(), wheelMoved()
   \endif
   *
   \if CHINESE
   \return 当启用跟踪时返回 true
   \sa setTracking(), valueChanged(), wheelMoved()
   \endif
 */
bool QwtWheel::isTracking() const
{
    return m_data->tracking;
}

/*!
   \if ENGLISH
   \brief Specify the update interval when the wheel is flying

   Default and minimum value is 50 ms.

   \param interval Interval in milliseconds
   \sa updateInterval(), setMass(), setTracking()
   \endif
   *
   \if CHINESE
   \brief 指定轮飞行时的更新间隔

   默认值和最小值是 50 毫秒。

   \param interval 间隔（毫秒）
   \sa updateInterval(), setMass(), setTracking()
   \endif
 */
void QwtWheel::setUpdateInterval(int interval)
{
    m_data->updateInterval = qMax(interval, 50);
}

/*!
   \if ENGLISH
   \return Update interval when the wheel is flying
   \sa setUpdateInterval(), mass(), isTracking()
   \endif
   *
   \if CHINESE
   \return 轮飞行时的更新间隔
   \sa setUpdateInterval(), mass(), isTracking()
   \endif
 */
int QwtWheel::updateInterval() const
{
    return m_data->updateInterval;
}

/*!
   \if ENGLISH
   \brief Mouse press event handler

   Start movement of the wheel.

   \param event Mouse event
   \endif
   *
   \if CHINESE
   \brief 鼠标按下事件处理程序

   开始轮的移动。

   \param event 鼠标事件
   \endif
 */
void QwtWheel::mousePressEvent(QMouseEvent* event)
{
    stopFlying();

    m_data->isScrolling = wheelRect().contains(event->pos());

    if (m_data->isScrolling) {
        m_data->timer.start();
        m_data->speed               = 0.0;
        m_data->mouseValue          = valueAt(event->pos());
        m_data->mouseOffset         = m_data->mouseValue - m_data->value;
        m_data->pendingValueChanged = false;

        Q_EMIT wheelPressed();
    }
}

/*!
   \if ENGLISH
   \brief Mouse Move Event handler

   Turn the wheel according to the mouse position

   \param event Mouse event
   \endif
   *
   \if CHINESE
   \brief 鼠标移动事件处理程序

   根据鼠标位置转动轮

   \param event 鼠标事件
   \endif
 */
void QwtWheel::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_data->isScrolling)
        return;

    double mouseValue = valueAt(event->pos());

    if (m_data->mass > 0.0) {
        double ms = m_data->timer.restart();

        // the interval when mouse move events are posted are somehow
        // random. To avoid unrealistic speed values we limit ms

        ms = qMax(ms, 5.0);

        m_data->speed = (mouseValue - m_data->mouseValue) / ms;
    }

    m_data->mouseValue = mouseValue;

    double value = boundedValue(mouseValue - m_data->mouseOffset);
    if (m_data->stepAlignment)
        value = alignedValue(value);

    if (value != m_data->value) {
        m_data->value = value;

        update();

        Q_EMIT wheelMoved(m_data->value);

        if (m_data->tracking)
            Q_EMIT valueChanged(m_data->value);
        else
            m_data->pendingValueChanged = true;
    }
}

/*!
   \if ENGLISH
   \brief Mouse Release Event handler

   When the wheel has no mass the movement of the wheel stops, otherwise
   it starts flying.

   \param event Mouse event
   \endif
   *
   \if CHINESE
   \brief 鼠标释放事件处理程序

   当轮没有质量时，轮的移动停止，否则开始飞行。

   \param event 鼠标事件
   \endif
 */

void QwtWheel::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    if (!m_data->isScrolling)
        return;

    m_data->isScrolling = false;

    bool startFlying = false;

    if (m_data->mass > 0.0) {
        const qint64 ms = m_data->timer.elapsed();
        if ((std::fabs(m_data->speed) > 0.0) && (ms < 50))
            startFlying = true;
    }

    if (startFlying) {
        m_data->flyingValue = boundedValue(m_data->mouseValue - m_data->mouseOffset);

        m_data->timerId = startTimer(m_data->updateInterval);
    } else {
        if (m_data->pendingValueChanged)
            Q_EMIT valueChanged(m_data->value);
    }

    m_data->pendingValueChanged = false;
    m_data->mouseOffset         = 0.0;

    Q_EMIT wheelReleased();
}

/*!
   \if ENGLISH
   \brief Qt timer event

   The flying wheel effect is implemented using a timer

   \param event Timer event

   \sa updateInterval()
   \endif
   *
   \if CHINESE
   \brief Qt 定时器事件

   飞轮效果使用定时器实现

   \param event 定时器事件

   \sa updateInterval()
   \endif
 */
void QwtWheel::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != m_data->timerId) {
        QWidget::timerEvent(event);
        return;
    }

    m_data->speed *= std::exp(-m_data->updateInterval * 0.001 / m_data->mass);

    m_data->flyingValue += m_data->speed * m_data->updateInterval;
    m_data->flyingValue = boundedValue(m_data->flyingValue);

    double value = m_data->flyingValue;
    if (m_data->stepAlignment)
        value = alignedValue(value);

    if (std::fabs(m_data->speed) < 0.001 * m_data->singleStep) {
        // stop if m_data->speed < one step per second
        stopFlying();
    }

    if (value != m_data->value) {
        m_data->value = value;
        update();

        if (m_data->tracking || m_data->timerId == 0)
            Q_EMIT valueChanged(m_data->value);
    }
}

/*!
   \if ENGLISH
   \brief Handle wheel events

   In/Decrement the value

   \param event Wheel event
   \endif
   *
   \if CHINESE
   \brief 处理轮事件

   增加/减少值

   \param event 轮事件
   \endif
 */
void QwtWheel::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION < 0x050e00
    const QPoint wheelPos = event->pos();
    const int wheelDelta  = event->delta();
#else
    const QPoint wheelPos = event->position().toPoint();

    const QPoint delta   = event->angleDelta();
    const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
#endif

    if (!wheelRect().contains(wheelPos)) {
        event->ignore();
        return;
    }

    if (m_data->isScrolling)
        return;

    stopFlying();

    double increment = 0.0;

    if ((event->modifiers() & Qt::ControlModifier) || (event->modifiers() & Qt::ShiftModifier)) {
        // one page regardless of delta
        increment = m_data->singleStep * m_data->pageStepCount;
        if (wheelDelta < 0)
            increment = -increment;
    } else {
        const int numSteps = wheelDelta / 120;
        increment          = m_data->singleStep * numSteps;
    }

    if (m_data->orientation == Qt::Vertical && m_data->inverted)
        increment = -increment;

    double value = boundedValue(m_data->value + increment);

    if (m_data->stepAlignment)
        value = alignedValue(value);

    if (value != m_data->value) {
        m_data->value = value;
        update();

        Q_EMIT valueChanged(m_data->value);
        Q_EMIT wheelMoved(m_data->value);
    }
}

/*!
   \if ENGLISH
   Handle key events

   - Qt::Key_Home\n
    Step to minimum()

   - Qt::Key_End\n
    Step to maximum()

   - Qt::Key_Up\n
    In case of a horizontal or not inverted vertical wheel the value
    will be incremented by the step size. For an inverted vertical wheel
    the value will be decremented by the step size.

   - Qt::Key_Down\n
    In case of a horizontal or not inverted vertical wheel the value
    will be decremented by the step size. For an inverted vertical wheel
    the value will be incremented by the step size.

   - Qt::Key_PageUp\n
    The value will be incremented by pageStepSize() * singleStepSize().

   - Qt::Key_PageDown\n
    The value will be decremented by pageStepSize() * singleStepSize().

   \param event Key event
   \endif
   *
   \if CHINESE
   处理键盘事件

   - Qt::Key_Home\n
    步进到 minimum()

   - Qt::Key_End\n
    步进到 maximum()

   - Qt::Key_Up\n
    对于水平或非反向的垂直轮，值将增加步长。
    对于反向的垂直轮，值将减少步长。

   - Qt::Key_Down\n
    对于水平或非反向的垂直轮，值将减少步长。
    对于反向的垂直轮，值将增加步长。

   - Qt::Key_PageUp\n
    值将增加 pageStepSize() * singleStepSize()。

   - Qt::Key_PageDown\n
    值将减少 pageStepSize() * singleStepSize()。

   \param event 键盘事件
   \endif
 */
void QwtWheel::keyPressEvent(QKeyEvent* event)
{
    if (m_data->isScrolling) {
        // don't interfere mouse scrolling
        return;
    }

    double value     = m_data->value;
    double increment = 0.0;

    switch (event->key()) {
    case Qt::Key_Down: {
        if (m_data->orientation == Qt::Vertical && m_data->inverted)
            increment = m_data->singleStep;
        else
            increment = -m_data->singleStep;

        break;
    }
    case Qt::Key_Up: {
        if (m_data->orientation == Qt::Vertical && m_data->inverted)
            increment = -m_data->singleStep;
        else
            increment = m_data->singleStep;

        break;
    }
    case Qt::Key_Left: {
        if (m_data->orientation == Qt::Horizontal) {
            if (m_data->inverted)
                increment = m_data->singleStep;
            else
                increment = -m_data->singleStep;
        }
        break;
    }
    case Qt::Key_Right: {
        if (m_data->orientation == Qt::Horizontal) {
            if (m_data->inverted)
                increment = -m_data->singleStep;
            else
                increment = m_data->singleStep;
        }
        break;
    }
    case Qt::Key_PageUp: {
        increment = m_data->pageStepCount * m_data->singleStep;
        break;
    }
    case Qt::Key_PageDown: {
        increment = -m_data->pageStepCount * m_data->singleStep;
        break;
    }
    case Qt::Key_Home: {
        value = m_data->minimum;
        break;
    }
    case Qt::Key_End: {
        value = m_data->maximum;
        break;
    }
    default:;
        {
            event->ignore();
        }
    }

    if (event->isAccepted())
        stopFlying();

    if (increment != 0.0) {
        value = boundedValue(m_data->value + increment);

        if (m_data->stepAlignment)
            value = alignedValue(value);
    }

    if (value != m_data->value) {
        m_data->value = value;
        update();

        Q_EMIT valueChanged(m_data->value);
        Q_EMIT wheelMoved(m_data->value);
    }
}

/*!
   \if ENGLISH
   \brief Adjust the number of grooves in the wheel's surface.

   The number of grooves is limited to 6 <= count <= 50.
   Values outside this range will be clipped.
   The default value is 10.

   \param count Number of grooves per 360 degrees
   \sa tickCount()
   \endif
   *
   \if CHINESE
   \brief 调整轮表面凹槽的数量。

   凹槽数量限制为 6 <= count <= 50。
   超出此范围的值将被裁剪。
   默认值为 10。

   \param count 每 360 度的凹槽数
   \sa tickCount()
   \endif
 */
void QwtWheel::setTickCount(int count)
{
    count = qBound(6, count, 50);

    if (count != m_data->tickCount) {
        m_data->tickCount = qBound(6, count, 50);
        update();
    }
}

/*!
   \if ENGLISH
   \return Number of grooves in the wheel's surface.
   \sa setTickCnt()
   \endif
   *
   \if CHINESE
   \return 轮表面凹槽的数量。
   \sa setTickCnt()
   \endif
 */
int QwtWheel::tickCount() const
{
    return m_data->tickCount;
}

/*!
   \if ENGLISH
   \brief Set the wheel border width of the wheel.

   The wheel border must not be smaller than 1
   and is limited in dependence on the wheel's size.
   Values outside the allowed range will be clipped.

   The wheel border defaults to 2.

   \param borderWidth Border width
   \sa internalBorder()
   \endif
   *
   \if CHINESE
   \brief 设置轮的边框宽度。

   轮边框不能小于 1，
   并且根据轮的大小有限制。
   超出允许范围的值将被裁剪。

   轮边框默认为 2。

   \param borderWidth 边框宽度
   \sa internalBorder()
   \endif
 */
void QwtWheel::setWheelBorderWidth(int borderWidth)
{
    const int d              = qMin(width(), height()) / 3;
    borderWidth              = qMin(borderWidth, d);
    m_data->wheelBorderWidth = qMax(borderWidth, 1);
    update();
}

/*!
   \if ENGLISH
   \return Wheel border width
   \sa setWheelBorderWidth()
   \endif
   *
   \if CHINESE
   \return 轮边框宽度
   \sa setWheelBorderWidth()
   \endif
 */
int QwtWheel::wheelBorderWidth() const
{
    return m_data->wheelBorderWidth;
}

/*!
   \if ENGLISH
   \brief Set the border width

   The border defaults to 2.

   \param width Border width
   \sa borderWidth()
   \endif
   *
   \if CHINESE
   \brief 设置边框宽度

   边框默认为 2。

   \param width 边框宽度
   \sa borderWidth()
   \endif
 */
void QwtWheel::setBorderWidth(int width)
{
    m_data->borderWidth = qMax(width, 0);
    update();
}

/*!
   \if ENGLISH
   \return Border width
   \sa setBorderWidth()
   \endif
   *
   \if CHINESE
   \return 边框宽度
   \sa setBorderWidth()
   \endif
 */
int QwtWheel::borderWidth() const
{
    return m_data->borderWidth;
}

/*!
   \return Rectangle of the wheel without the outer border
 */
QRect QwtWheel::wheelRect() const
{
    const int bw = m_data->borderWidth;
    return contentsRect().adjusted(bw, bw, -bw, -bw);
}

/*!
   \if ENGLISH
   \brief Set the total angle which the wheel can be turned.

   One full turn of the wheel corresponds to an angle of
   360 degrees. A total angle of n*360 degrees means
   that the wheel has to be turned n times around its axis
   to get from the minimum value to the maximum value.

   The default setting of the total angle is 360 degrees.

   \param angle total angle in degrees
   \sa totalAngle()
   \endif
   *
   \if CHINESE
   \brief 设置轮可以转动的总角度。

   轮的一整圈对应 360 度。
   n*360 度的总角度意味着轮必须绕其轴转动 n 圈
   才能从最小值到最大值。

   总角度的默认设置是 360 度。

   \param angle 总角度（度）
   \sa totalAngle()
   \endif
 */
void QwtWheel::setTotalAngle(double angle)
{
    if (angle < 0.0)
        angle = 0.0;

    m_data->totalAngle = angle;
    update();
}

/*!
   \if ENGLISH
   \return Total angle which the wheel can be turned.
   \sa setTotalAngle()
   \endif
   *
   \if CHINESE
   \return 轮可以转动的总角度。
   \sa setTotalAngle()
   \endif
 */
double QwtWheel::totalAngle() const
{
    return m_data->totalAngle;
}

/**
 * \if ENGLISH
 * @brief Set the wheel's orientation
 * @details The default orientation is Qt::Horizontal.
 * @param[in] orientation Qt::Horizontal or Qt::Vertical
 * \sa orientation()
 * \endif
 * \if CHINESE
 * @brief 设置轮的方向
 * @details 默认方向是 Qt::Horizontal。
 * @param[in] orientation Qt::Horizontal 或 Qt::Vertical
 * \sa orientation()
 * \endif
 */
void QwtWheel::setOrientation(Qt::Orientation orientation)
{
    if (m_data->orientation == orientation)
        return;

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    m_data->orientation = orientation;
    update();
}

/**
 * \if ENGLISH
 * @brief Return the orientation
 * @return Orientation
 * \sa setOrientation()
 * \endif
 * \if CHINESE
 * @brief 返回方向
 * @return 方向
 * \sa setOrientation()
 * \endif
 */
Qt::Orientation QwtWheel::orientation() const
{
    return m_data->orientation;
}

/**
 * \if ENGLISH
 * @brief Specify the visible portion of the wheel
 * @details You may use this function for fine-tuning the appearance of the wheel.
 *          The default value is 175 degrees. The value is limited from 10 to 175 degrees.
 * @param[in] angle Visible angle in degrees
 * \sa viewAngle(), setTotalAngle()
 * \endif
 * \if CHINESE
 * @brief 指定轮的可见部分
 * @details 您可以使用此函数微调轮的外观。
 *          默认值为 175 度。值限制在 10 到 175 度之间。
 * @param[in] angle 可见角度（度）
 * \sa viewAngle(), setTotalAngle()
 * \endif
 */
void QwtWheel::setViewAngle(double angle)
{
    m_data->viewAngle = qBound(10.0, angle, 175.0);
    update();
}

/**
 * \if ENGLISH
 * @brief Return the visible portion of the wheel
 * @return Visible angle in degrees
 * \sa setViewAngle(), totalAngle()
 * \endif
 * \if CHINESE
 * @brief 返回轮的可见部分
 * @return 可见角度（度）
 * \sa setViewAngle(), totalAngle()
 * \endif
 */
double QwtWheel::viewAngle() const
{
    return m_data->viewAngle;
}

/*!
   Determine the value corresponding to a specified point

   \param pos Position
   \return Value corresponding to pos
 */
double QwtWheel::valueAt(const QPoint& pos) const
{
    const QRectF rect = wheelRect();

    double w, dx;
    if (m_data->orientation == Qt::Vertical) {
        w  = rect.height();
        dx = rect.top() - pos.y();
    } else {
        w  = rect.width();
        dx = pos.x() - rect.left();
    }

    if (w == 0.0)
        return 0.0;

    if (m_data->inverted) {
        dx = w - dx;
    }

    // w pixels is an arc of viewAngle degrees,
    // so we convert change in pixels to change in angle
    const double ang = dx * m_data->viewAngle / w;

    // value range maps to totalAngle degrees,
    // so convert the change in angle to a change in value
    const double val = ang * (maximum() - minimum()) / m_data->totalAngle;

    return val;
}

/*!
   \if ENGLISH
   \brief Qt Paint Event
   \param event Paint event
   \endif
   *
   \if CHINESE
   \brief Qt 绘制事件
   \param event 绘制事件
   \endif
 */
void QwtWheel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    qDrawShadePanel(&painter, contentsRect(), palette(), true, m_data->borderWidth);

    drawWheelBackground(&painter, wheelRect());
    drawTicks(&painter, wheelRect());

    if (hasFocus())
        QwtPainter::drawFocusRect(&painter, this);
}

/*!
   Draw the Wheel's background gradient

   \param painter Painter
   \param rect Geometry for the wheel
 */
void QwtWheel::drawWheelBackground(QPainter* painter, const QRectF& rect)
{
    painter->save();

    QPalette pal = palette();

    //  draw shaded background
    QLinearGradient gradient(rect.topLeft(), (m_data->orientation == Qt::Horizontal) ? rect.topRight() : rect.bottomLeft());
    gradient.setColorAt(0.0, pal.color(QPalette::Button));
    gradient.setColorAt(0.2, pal.color(QPalette::Midlight));
    gradient.setColorAt(0.7, pal.color(QPalette::Mid));
    gradient.setColorAt(1.0, pal.color(QPalette::Dark));

    painter->fillRect(rect, gradient);

    // draw internal border

    const QPen lightPen(palette().color(QPalette::Light), m_data->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap);
    const QPen darkPen(pal.color(QPalette::Dark), m_data->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap);

    const double bw2 = 0.5 * m_data->wheelBorderWidth;

    if (m_data->orientation == Qt::Horizontal) {
        painter->setPen(lightPen);
        painter->drawLine(QPointF(rect.left(), rect.top() + bw2), QPointF(rect.right(), rect.top() + bw2));

        painter->setPen(darkPen);
        painter->drawLine(QPointF(rect.left(), rect.bottom() - bw2), QPointF(rect.right(), rect.bottom() - bw2));
    } else  // Qt::Vertical
    {
        painter->setPen(lightPen);
        painter->drawLine(QPointF(rect.left() + bw2, rect.top()), QPointF(rect.left() + bw2, rect.bottom()));

        painter->setPen(darkPen);
        painter->drawLine(QPointF(rect.right() - bw2, rect.top()), QPointF(rect.right() - bw2, rect.bottom()));
    }

    painter->restore();
}

/*!
   Draw the Wheel's ticks

   \param painter Painter
   \param rect Geometry for the wheel
 */
void QwtWheel::drawTicks(QPainter* painter, const QRectF& rect)
{
    const double range = m_data->maximum - m_data->minimum;

    if (range == 0.0 || m_data->totalAngle == 0.0) {
        return;
    }

    const QPen lightPen(palette().color(QPalette::Light), 0, Qt::SolidLine, Qt::FlatCap);
    const QPen darkPen(palette().color(QPalette::Dark), 0, Qt::SolidLine, Qt::FlatCap);

    const double cnvFactor = qAbs(m_data->totalAngle / range);
    const double halfIntv  = 0.5 * m_data->viewAngle / cnvFactor;
    const double loValue   = value() - halfIntv;
    const double hiValue   = value() + halfIntv;
    const double tickWidth = 360.0 / double(m_data->tickCount) / cnvFactor;
    const double sinArc    = qFastSin(m_data->viewAngle * M_PI / 360.0);

    if (m_data->orientation == Qt::Horizontal) {
        const double radius = rect.width() * 0.5;

        double l1 = rect.top() + m_data->wheelBorderWidth;
        double l2 = rect.bottom() - m_data->wheelBorderWidth - 1;

        // draw one point over the border if border > 1
        if (m_data->wheelBorderWidth > 1) {
            l1--;
            l2++;
        }

        const double maxpos = rect.right() - 2;
        const double minpos = rect.left() + 2;

        // draw tick marks
        for (double tickValue = std::ceil(loValue / tickWidth) * tickWidth; tickValue < hiValue; tickValue += tickWidth) {
            const double angle = qwtRadians(tickValue - value());
            const double s     = qFastSin(angle * cnvFactor);

            const double off = radius * (sinArc + s) / sinArc;

            double tickPos;
            if (m_data->inverted)
                tickPos = rect.left() + off;
            else
                tickPos = rect.right() - off;

            if ((tickPos <= maxpos) && (tickPos > minpos)) {
                painter->setPen(darkPen);
                painter->drawLine(QPointF(tickPos - 1, l1), QPointF(tickPos - 1, l2));
                painter->setPen(lightPen);
                painter->drawLine(QPointF(tickPos, l1), QPointF(tickPos, l2));
            }
        }
    } else  // Qt::Vertical
    {
        const double radius = rect.height() * 0.5;

        double l1 = rect.left() + m_data->wheelBorderWidth;
        double l2 = rect.right() - m_data->wheelBorderWidth - 1;

        if (m_data->wheelBorderWidth > 1) {
            l1--;
            l2++;
        }

        const double maxpos = rect.bottom() - 2;
        const double minpos = rect.top() + 2;

        for (double tickValue = std::ceil(loValue / tickWidth) * tickWidth; tickValue < hiValue; tickValue += tickWidth) {
            const double angle = qwtRadians(tickValue - value());
            const double s     = qFastSin(angle * cnvFactor);

            const double off = radius * (sinArc + s) / sinArc;

            double tickPos;

            if (m_data->inverted)
                tickPos = rect.bottom() - off;
            else
                tickPos = rect.top() + off;

            if ((tickPos <= maxpos) && (tickPos > minpos)) {
                painter->setPen(darkPen);
                painter->drawLine(QPointF(l1, tickPos - 1), QPointF(l2, tickPos - 1));
                painter->setPen(lightPen);
                painter->drawLine(QPointF(l1, tickPos), QPointF(l2, tickPos));
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Set the width of the wheel
 * @details Corresponds to the wheel height for horizontal orientation,
 *          and the wheel width for vertical orientation.
 * @param[in] width The wheel's width
 * \sa wheelWidth()
 * \endif
 * \if CHINESE
 * @brief 设置轮的宽度
 * @details 对应水平方向的轮高度和垂直方向的轮宽度。
 * @param[in] width 轮的宽度
 * \sa wheelWidth()
 * \endif
 */
void QwtWheel::setWheelWidth(int width)
{
    m_data->wheelWidth = width;
    update();
}

/**
 * \if ENGLISH
 * @brief Return the width of the wheel
 * @return Wheel width
 * \sa setWheelWidth()
 * \endif
 * \if CHINESE
 * @brief 返回轮的宽度
 * @return 轮宽度
 * \sa setWheelWidth()
 * \endif
 */
int QwtWheel::wheelWidth() const
{
    return m_data->wheelWidth;
}

/**
 * \if ENGLISH
 * @brief Return the size hint
 * @return Size hint
 * \endif
 * \if CHINESE
 * @brief 返回大小提示
 * @return 大小提示
 * \endif
 */
QSize QwtWheel::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return qwtExpandedToGlobalStrut(hint);
}

/**
 * \if ENGLISH
 * @brief Return the minimum size hint
 * @return Minimum size hint
 * \warning The return value is based on the wheel width.
 * \endif
 * \if CHINESE
 * @brief 返回最小大小提示
 * @return 最小大小提示
 * \warning 返回值基于轮宽度。
 * \endif
 */
QSize QwtWheel::minimumSizeHint() const
{
    QSize sz(3 * m_data->wheelWidth + 2 * m_data->borderWidth, m_data->wheelWidth + 2 * m_data->borderWidth);
    if (m_data->orientation != Qt::Horizontal)
        sz.transpose();

    return sz;
}

/**
 * \if ENGLISH
 * @brief Set the step size of the counter
 * @details A value <= 0.0 disables stepping.
 * @param[in] stepSize Single step size
 * \sa singleStep(), setPageStepCount()
 * \endif
 * \if CHINESE
 * @brief 设置计数器的步长
 * @details 值 <= 0.0 禁用步进。
 * @param[in] stepSize 单步大小
 * \sa singleStep(), setPageStepCount()
 * \endif
 */
void QwtWheel::setSingleStep(double stepSize)
{
    m_data->singleStep = qwtMaxF(stepSize, 0.0);
}

/**
 * \if ENGLISH
 * @brief Return the single step size
 * @return Single step size
 * \sa setSingleStep()
 * \endif
 * \if CHINESE
 * @brief 返回单步大小
 * @return 单步大小
 * \sa setSingleStep()
 * \endif
 */
double QwtWheel::singleStep() const
{
    return m_data->singleStep;
}

/**
 * \if ENGLISH
 * @brief En/Disable step alignment
 * @details When step alignment is enabled, value changes initiated by
 *          user input (mouse, keyboard, wheel) are aligned to multiples of the single step.
 * @param[in] on On/Off
 * \sa stepAlignment(), setSingleStep()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用步对齐
 * @details 当启用步对齐时，用户输入（鼠标、键盘、滚轮）引起的值变化将对齐到单步的倍数。
 * @param[in] on 开/关
 * \sa stepAlignment(), setSingleStep()
 * \endif
 */
void QwtWheel::setStepAlignment(bool on)
{
    if (on != m_data->stepAlignment) {
        m_data->stepAlignment = on;
    }
}

/**
 * \if ENGLISH
 * @brief Return whether step alignment is enabled
 * @return True when step alignment is enabled
 * \sa setStepAlignment(), singleStep()
 * \endif
 * \if CHINESE
 * @brief 返回是否启用了步对齐
 * @return 如果启用了步对齐返回 true
 * \sa setStepAlignment(), singleStep()
 * \endif
 */
bool QwtWheel::stepAlignment() const
{
    return m_data->stepAlignment;
}

/**
 * \if ENGLISH
 * @brief Set the page step count
 * @details pageStepCount is a multiplicator for the single step size
 *          that typically corresponds to the user pressing PageUp or PageDown.
 *          A value of 0 disables page stepping. The default value is 1.
 * @param[in] count Multiplicator for the single step size
 * \sa pageStepCount(), setSingleStep()
 * \endif
 * \if CHINESE
 * @brief 设置页步数
 * @details pageStepCount 是单步大小的乘数，通常对应用户按下 PageUp 或 PageDown。
 *          值为 0 禁用页步进。默认值为 1。
 * @param[in] count 单步大小的乘数
 * \sa pageStepCount(), setSingleStep()
 * \endif
 */
void QwtWheel::setPageStepCount(int count)
{
    m_data->pageStepCount = qMax(0, count);
}

/**
 * \if ENGLISH
 * @brief Return the page step count
 * @return Page step count
 * \sa setPageStepCount(), singleStep()
 * \endif
 * \if CHINESE
 * @brief 返回页步数
 * @return 页步数
 * \sa setPageStepCount(), singleStep()
 * \endif
 */
int QwtWheel::pageStepCount() const
{
    return m_data->pageStepCount;
}

/**
 * \if ENGLISH
 * @brief Set the minimum and maximum values
 * @details The maximum is adjusted if necessary to ensure that the range remains valid.
 *          The value might be modified to be inside of the range.
 * @param[in] min Minimum value
 * @param[in] max Maximum value
 * \sa minimum(), maximum()
 * \endif
 * \if CHINESE
 * @brief 设置最小值和最大值
 * @details 如有必要会调整最大值以确保范围有效。
 *          值可能会被修改以保持在范围内。
 * @param[in] min 最小值
 * @param[in] max 最大值
 * \sa minimum(), maximum()
 * \endif
 */
void QwtWheel::setRange(double min, double max)
{
    max = qwtMaxF(min, max);

    if (m_data->minimum == min && m_data->maximum == max)
        return;

    m_data->minimum = min;
    m_data->maximum = max;

    if (m_data->value < min || m_data->value > max) {
        m_data->value = qBound(min, m_data->value, max);

        update();
        Q_EMIT valueChanged(m_data->value);
    }
}
/**
 * \if ENGLISH
 * @brief Set the minimum value of the range
 * @param[in] value Minimum value
 * \sa setRange(), setMaximum(), minimum()
 * \note The maximum is adjusted if necessary to ensure that the range remains valid.
 * \endif
 * \if CHINESE
 * @brief 设置范围的最小值
 * @param[in] value 最小值
 * \sa setRange(), setMaximum(), minimum()
 * \note 如有必要会调整最大值以确保范围有效。
 * \endif
 */
void QwtWheel::setMinimum(double value)
{
    setRange(value, maximum());
}

/**
 * \if ENGLISH
 * @brief Return the minimum of the range
 * @return Minimum value
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 * \if CHINESE
 * @brief 返回范围的最小值
 * @return 最小值
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 */
double QwtWheel::minimum() const
{
    return m_data->minimum;
}

/**
 * \if ENGLISH
 * @brief Set the maximum value of the range
 * @param[in] value Maximum value
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 * \if CHINESE
 * @brief 设置范围的最大值
 * @param[in] value 最大值
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 */
void QwtWheel::setMaximum(double value)
{
    setRange(minimum(), value);
}

/**
 * \if ENGLISH
 * @brief Return the maximum of the range
 * @return Maximum value
 * \sa setRange(), setMaximum(), minimum()
 * \endif
 * \if CHINESE
 * @brief 返回范围的最大值
 * @return 最大值
 * \sa setRange(), setMaximum(), minimum()
 * \endif
 */
double QwtWheel::maximum() const
{
    return m_data->maximum;
}

/**
 * \if ENGLISH
 * @brief Set a new value without adjusting to the step raster
 * @param[in] value New value
 * \sa value(), valueChanged()
 * \warning The value is clipped when it lies outside the range.
 * \endif
 * \if CHINESE
 * @brief 设置新值而不调整到步栅格
 * @param[in] value 新值
 * \sa value(), valueChanged()
 * \warning 当值超出范围时会被裁剪。
 * \endif
 */
void QwtWheel::setValue(double value)
{
    stopFlying();
    m_data->isScrolling = false;

    value = qBound(m_data->minimum, value, m_data->maximum);

    if (m_data->value != value) {
        m_data->value = value;

        update();
        Q_EMIT valueChanged(m_data->value);
    }
}

/**
 * \if ENGLISH
 * @brief Return the current value of the wheel
 * @return Current value
 * \sa setValue(), valueChanged()
 * \endif
 * \if CHINESE
 * @brief 返回轮的当前值
 * @return 当前值
 * \sa setValue(), valueChanged()
 * \endif
 */
double QwtWheel::value() const
{
    return m_data->value;
}

/**
 * \if ENGLISH
 * @brief En/Disable inverted appearance
 * @details An inverted wheel increases its values in the opposite direction.
 *          The direction of an inverted horizontal wheel will be from right to left,
 *          an inverted vertical wheel will increase from bottom to top.
 * @param[in] on En/Disable inverted appearance
 * \sa isInverted()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用反转外观
 * @details 反转的轮以相反方向增加其值。
 *          反转的水平轮方向将从右到左，反转的垂直轮将从底到顶增加。
 * @param[in] on 启用/禁用反转外观
 * \sa isInverted()
 * \endif
 */
void QwtWheel::setInverted(bool on)
{
    if (m_data->inverted != on) {
        m_data->inverted = on;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return whether the wheel is inverted
 * @return True when the wheel is inverted
 * \sa setInverted()
 * \endif
 * \if CHINESE
 * @brief 返回轮是否反转
 * @return 如果轮反转返回 true
 * \sa setInverted()
 * \endif
 */
bool QwtWheel::isInverted() const
{
    return m_data->inverted;
}

/**
 * \if ENGLISH
 * @brief En/Disable wrapping
 * @details If wrapping is true, stepping up from maximum() value will take
 *          you to the minimum() value and vice versa.
 * @param[in] on En/Disable wrapping
 * \sa wrapping()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用环绕
 * @details 如果启用环绕，从 maximum() 值向上步进会到达 minimum() 值，反之亦然。
 * @param[in] on 启用/禁用环绕
 * \sa wrapping()
 * \endif
 */
void QwtWheel::setWrapping(bool on)
{
    m_data->wrapping = on;
}

/**
 * \if ENGLISH
 * @brief Return whether wrapping is enabled
 * @return True when wrapping is set
 * \sa setWrapping()
 * \endif
 * \if CHINESE
 * @brief 返回是否启用了环绕
 * @return 如果启用了环绕返回 true
 * \sa setWrapping()
 * \endif
 */
bool QwtWheel::wrapping() const
{
    return m_data->wrapping;
}

/**
 * \if ENGLISH
 * @brief Set the slider's mass for flywheel effect
 * @details If the slider's mass is greater than 0, it will continue to move
 *          after the mouse button has been released. Its speed decreases
 *          with time at a rate depending on the slider's mass.
 *          A large mass means that it will continue to move for a long time.
 *          Derived widgets may overload this function to make it public.
 * @param[in] mass New mass in kg
 * \warning If the mass is smaller than 1g, it is set to zero.
 *          The maximal mass is limited to 100kg.
 * \sa mass()
 * \endif
 * \if CHINESE
 * @brief 设置滑块的质量以实现飞轮效果
 * @details 如果滑块的质量大于 0，它将在鼠标按钮释放后继续移动。
 *          其速度随时间递减，速率取决于滑块的质量。
 *          大质量意味着它将继续移动很长时间。
 *          派生控件可以重载此函数使其公开。
 * @param[in] mass 新质量（千克）
 * \warning 如果质量小于 1g，则设置为 0。
 *          最大质量限制为 100kg。
 * \sa mass()
 * \endif
 */
void QwtWheel::setMass(double mass)
{
    if (mass < 0.001) {
        m_data->mass = 0.0;
    } else {
        m_data->mass = qwtMinF(100.0, mass);
    }

    if (m_data->mass <= 0.0)
        stopFlying();
}

/**
 * \if ENGLISH
 * @brief Return the mass for flywheel effect
 * @return Mass in kg
 * \sa setMass()
 * \endif
 * \if CHINESE
 * @brief 返回飞轮效果的质量
 * @return 质量（千克）
 * \sa setMass()
 * \endif
 */
double QwtWheel::mass() const
{
    return m_data->mass;
}

/*!
   \if ENGLISH
   \brief Stop the flying movement of the wheel
   \endif
   *
   \if CHINESE
   \brief 停止轮的飞行移动
   \endif
 */
void QwtWheel::stopFlying()
{
    if (m_data->timerId != 0) {
        killTimer(m_data->timerId);
        m_data->timerId = 0;
        m_data->speed   = 0.0;
    }
}

double QwtWheel::boundedValue(double value) const
{
    const double range = m_data->maximum - m_data->minimum;

    if (m_data->wrapping && range >= 0.0) {
        if (value < m_data->minimum) {
            value += std::ceil((m_data->minimum - value) / range) * range;
        } else if (value > m_data->maximum) {
            value -= std::ceil((value - m_data->maximum) / range) * range;
        }
    } else {
        value = qBound(m_data->minimum, value, m_data->maximum);
    }

    return value;
}

double QwtWheel::alignedValue(double value) const
{
    const double stepSize = m_data->singleStep;

    if (stepSize > 0.0) {
        value = m_data->minimum + qRound((value - m_data->minimum) / stepSize) * stepSize;

        if (stepSize > 1e-12) {
            if (qFuzzyCompare(value + 1.0, 1.0)) {
                // correct rounding error if value = 0
                value = 0.0;
            } else if (qFuzzyCompare(value, m_data->maximum)) {
                // correct rounding error at the border
                value = m_data->maximum;
            }
        }
    }

    return value;
}
