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
    QWT_DECLARE_PUBLIC(QwtWheel)
public:
    PrivateData(QwtWheel* p)
        : q_ptr(p)
        , orientation(Qt::Horizontal)
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
        , flatStyle(true)
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
    bool flatStyle;
};

//! Constructor
QwtWheel::QwtWheel(QWidget* parent) : QWidget(parent), QWT_PIMPL_CONSTRUCT
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

//! Destructor
QwtWheel::~QwtWheel()
{
}

/*!
   @brief En/Disable tracking

   If tracking is enabled (the default), the wheel emits the valueChanged()
   signal while the wheel is moving. If tracking is disabled, the wheel
   emits the valueChanged() signal only when the wheel movement is terminated.

   The wheelMoved() signal is emitted regardless id tracking is enabled or not.

   @param enable On/Off
   @sa isTracking()
   *
 */
void QwtWheel::setTracking(bool enable)
{
    QWT_D(d);
    d->tracking = enable;
}

/*!
   @return True, when tracking is enabled
   @sa setTracking(), valueChanged(), wheelMoved()
   *
 */
bool QwtWheel::isTracking() const
{
    QWT_DC(d);
    return d->tracking;
}

/*!
   @brief Specify the update interval when the wheel is flying

   Default and minimum value is 50 ms.

   @param interval Interval in milliseconds
   @sa updateInterval(), setMass(), setTracking()
   *
 */
void QwtWheel::setUpdateInterval(int interval)
{
    QWT_D(d);
    d->updateInterval = qMax(interval, 50);
}

/*!
   @return Update interval when the wheel is flying
   @sa setUpdateInterval(), mass(), isTracking()
   *
 */
int QwtWheel::updateInterval() const
{
    QWT_DC(d);
    return d->updateInterval;
}

/*!
   @brief Mouse press event handler

   Start movement of the wheel.

   @param event Mouse event
   *
 */
void QwtWheel::mousePressEvent(QMouseEvent* event)
{
    QWT_D(d);
    stopFlying();

    d->isScrolling = wheelRect().contains(event->pos());

    if (d->isScrolling) {
        d->timer.start();
        d->speed               = 0.0;
        d->mouseValue          = valueAt(event->pos());
        d->mouseOffset         = d->mouseValue - d->value;
        d->pendingValueChanged = false;

        Q_EMIT wheelPressed();
    }
}

/*!
   @brief Mouse Move Event handler

   Turn the wheel according to the mouse position

   @param event Mouse event
   *
 */
void QwtWheel::mouseMoveEvent(QMouseEvent* event)
{
    QWT_D(d);
    if (!d->isScrolling)
        return;

    double mouseValue = valueAt(event->pos());

    if (d->mass > 0.0) {
        double ms = d->timer.restart();

        // the interval when mouse move events are posted are somehow
        // random. To avoid unrealistic speed values we limit ms

        ms = qMax(ms, 5.0);

        d->speed = (mouseValue - d->mouseValue) / ms;
    }

    d->mouseValue = mouseValue;

    double value = boundedValue(mouseValue - d->mouseOffset);
    if (d->stepAlignment)
        value = alignedValue(value);

    if (value != d->value) {
        d->value = value;

        update();

        Q_EMIT wheelMoved(d->value);

        if (d->tracking)
            Q_EMIT valueChanged(d->value);
        else
            d->pendingValueChanged = true;
    }
}

/*!
   @brief Mouse Release Event handler

   When the wheel has no mass the movement of the wheel stops, otherwise
   it starts flying.

   @param event Mouse event
   *
 */

void QwtWheel::mouseReleaseEvent(QMouseEvent* event)
{
    QWT_D(d);
    Q_UNUSED(event);

    if (!d->isScrolling)
        return;

    d->isScrolling = false;

    bool startFlying = false;

    if (d->mass > 0.0) {
        const qint64 ms = d->timer.elapsed();
        if ((std::fabs(d->speed) > 0.0) && (ms < 50))
            startFlying = true;
    }

    if (startFlying) {
        d->flyingValue = boundedValue(d->mouseValue - d->mouseOffset);

        d->timerId = startTimer(d->updateInterval);
    } else {
        if (d->pendingValueChanged)
            Q_EMIT valueChanged(d->value);
    }

    d->pendingValueChanged = false;
    d->mouseOffset         = 0.0;

    Q_EMIT wheelReleased();
}

/*!
   @brief Qt timer event

   The flying wheel effect is implemented using a timer

   @param event Timer event

   @sa updateInterval()
   *
 */
void QwtWheel::timerEvent(QTimerEvent* event)
{
    QWT_D(d);
    if (event->timerId() != d->timerId) {
        QWidget::timerEvent(event);
        return;
    }

    d->speed *= std::exp(-d->updateInterval * 0.001 / d->mass);

    d->flyingValue += d->speed * d->updateInterval;
    d->flyingValue = boundedValue(d->flyingValue);

    double value = d->flyingValue;
    if (d->stepAlignment)
        value = alignedValue(value);

    if (std::fabs(d->speed) < 0.001 * d->singleStep) {
        // stop if d->speed < one step per second
        stopFlying();
    }

    if (value != d->value) {
        d->value = value;
        update();

        if (d->tracking || d->timerId == 0)
            Q_EMIT valueChanged(d->value);
    }
}

/*!
   @brief Handle wheel events

   In/Decrement the value

   @param event Wheel event
   *
 */
void QwtWheel::wheelEvent(QWheelEvent* event)
{
    QWT_D(d);
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

    if (d->isScrolling)
        return;

    stopFlying();

    double increment = 0.0;

    if ((event->modifiers() & Qt::ControlModifier) || (event->modifiers() & Qt::ShiftModifier)) {
        // one page regardless of delta
        increment = d->singleStep * d->pageStepCount;
        if (wheelDelta < 0)
            increment = -increment;
    } else {
        const int numSteps = wheelDelta / 120;
        increment          = d->singleStep * numSteps;
    }

    if (d->orientation == Qt::Vertical && d->inverted)
        increment = -increment;

    double value = boundedValue(d->value + increment);

    if (d->stepAlignment)
        value = alignedValue(value);

    if (value != d->value) {
        d->value = value;
        update();

        Q_EMIT valueChanged(d->value);
        Q_EMIT wheelMoved(d->value);
    }
}

/*!
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

   @param event Key event
   *
 */
void QwtWheel::keyPressEvent(QKeyEvent* event)
{
    QWT_D(d);
    if (d->isScrolling) {
        // don't interfere mouse scrolling
        return;
    }

    double value     = d->value;
    double increment = 0.0;

    switch (event->key()) {
    case Qt::Key_Down: {
        if (d->orientation == Qt::Vertical && d->inverted)
            increment = d->singleStep;
        else
            increment = -d->singleStep;

        break;
    }
    case Qt::Key_Up: {
        if (d->orientation == Qt::Vertical && d->inverted)
            increment = -d->singleStep;
        else
            increment = d->singleStep;

        break;
    }
    case Qt::Key_Left: {
        if (d->orientation == Qt::Horizontal) {
            if (d->inverted)
                increment = d->singleStep;
            else
                increment = -d->singleStep;
        }
        break;
    }
    case Qt::Key_Right: {
        if (d->orientation == Qt::Horizontal) {
            if (d->inverted)
                increment = -d->singleStep;
            else
                increment = d->singleStep;
        }
        break;
    }
    case Qt::Key_PageUp: {
        increment = d->pageStepCount * d->singleStep;
        break;
    }
    case Qt::Key_PageDown: {
        increment = -d->pageStepCount * d->singleStep;
        break;
    }
    case Qt::Key_Home: {
        value = d->minimum;
        break;
    }
    case Qt::Key_End: {
        value = d->maximum;
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
        value = boundedValue(d->value + increment);

        if (d->stepAlignment)
            value = alignedValue(value);
    }

    if (value != d->value) {
        d->value = value;
        update();

        Q_EMIT valueChanged(d->value);
        Q_EMIT wheelMoved(d->value);
    }
}

/*!
   @brief Adjust the number of grooves in the wheel's surface.

   The number of grooves is limited to 6 <= count <= 50.
   Values outside this range will be clipped.
   The default value is 10.

   @param count Number of grooves per 360 degrees
   @sa tickCount()
   *
 */
void QwtWheel::setTickCount(int count)
{
    QWT_D(d);
    count = qBound(6, count, 50);

    if (count != d->tickCount) {
        d->tickCount = qBound(6, count, 50);
        update();
    }
}

/*!
   @return Number of grooves in the wheel's surface.
   @sa setTickCnt()
   *
 */
int QwtWheel::tickCount() const
{
    QWT_DC(d);
    return d->tickCount;
}

/*!
   @brief Set the wheel border width of the wheel.

   The wheel border must not be smaller than 1
   and is limited in dependence on the wheel's size.
   Values outside the allowed range will be clipped.

   The wheel border defaults to 2.

   @param borderWidth Border width
   @sa internalBorder()
   *
 */
void QwtWheel::setWheelBorderWidth(int borderWidth)
{
    QWT_D(d);
    const int wd             = qMin(width(), height()) / 3;
    borderWidth              = qMin(borderWidth, wd);
    d->wheelBorderWidth = qMax(borderWidth, 1);
    update();
}

/*!
   @return Wheel border width
   @sa setWheelBorderWidth()
   *
 */
int QwtWheel::wheelBorderWidth() const
{
    QWT_DC(d);
    return d->wheelBorderWidth;
}

/*!
   @brief Set the border width

   The border defaults to 2.

   @param width Border width
   @sa borderWidth()
   *
 */
void QwtWheel::setBorderWidth(int width)
{
    QWT_D(d);
    d->borderWidth = qMax(width, 0);
    update();
}

/*!
   @return Border width
   @sa setBorderWidth()
   *
 */
int QwtWheel::borderWidth() const
{
    QWT_DC(d);
    return d->borderWidth;
}

/**
 * @brief Set flat style
 * @details When enabled (default), the wheel is drawn with flat colors
 *          instead of 3D embossed effects.
 * @param on true for flat style, false for classic 3D style
 * @sa flatStyle()
 */
void QwtWheel::setFlatStyle(bool on)
{
    QWT_D(d);
    if (d->flatStyle != on) {
        d->flatStyle = on;
        update();
    }
}

/**
 * @brief Return whether flat style is enabled
 * @sa setFlatStyle()
 */
bool QwtWheel::flatStyle() const
{
    QWT_DC(d);
    return d->flatStyle;
}

/*!
   @return Rectangle of the wheel without the outer border
 */
QRect QwtWheel::wheelRect() const
{
    QWT_DC(d);
    const int bw = d->borderWidth;
    return contentsRect().adjusted(bw, bw, -bw, -bw);
}

/*!
   @brief Set the total angle which the wheel can be turned.

   One full turn of the wheel corresponds to an angle of
   360 degrees. A total angle of n*360 degrees means
   that the wheel has to be turned n times around its axis
   to get from the minimum value to the maximum value.

   The default setting of the total angle is 360 degrees.

   @param angle total angle in degrees
   @sa totalAngle()
   *
 */
void QwtWheel::setTotalAngle(double angle)
{
    QWT_D(d);
    if (angle < 0.0)
        angle = 0.0;

    d->totalAngle = angle;
    update();
}

/*!
   @return Total angle which the wheel can be turned.
   @sa setTotalAngle()
   *
 */
double QwtWheel::totalAngle() const
{
    QWT_DC(d);
    return d->totalAngle;
}

/**
 * @brief Set the wheel's orientation
 * @details The default orientation is Qt::Horizontal.
 * @param[in] orientation Qt::Horizontal or Qt::Vertical
 * @sa orientation()
 */
void QwtWheel::setOrientation(Qt::Orientation orientation)
{
    QWT_D(d);
    if (d->orientation == orientation)
        return;

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    d->orientation = orientation;
    update();
}

/**
 * @brief Return the orientation
 * @return Orientation
 * @sa setOrientation()
 */
Qt::Orientation QwtWheel::orientation() const
{
    QWT_DC(d);
    return d->orientation;
}

/**
 * @brief Specify the visible portion of the wheel
 * @details You may use this function for fine-tuning the appearance of the wheel.
 *          The default value is 175 degrees. The value is limited from 10 to 175 degrees.
 * @param[in] angle Visible angle in degrees
 * @sa viewAngle(), setTotalAngle()
 */
void QwtWheel::setViewAngle(double angle)
{
    QWT_D(d);
    d->viewAngle = qBound(10.0, angle, 175.0);
    update();
}

/**
 * @brief Return the visible portion of the wheel
 * @return Visible angle in degrees
 * @sa setViewAngle(), totalAngle()
 */
double QwtWheel::viewAngle() const
{
    QWT_DC(d);
    return d->viewAngle;
}

/*!
   Determine the value corresponding to a specified point

   @param pos Position
   @return Value corresponding to pos
 */
double QwtWheel::valueAt(const QPoint& pos) const
{
    QWT_DC(d);
    const QRectF rect = wheelRect();

    double w, dx;
    if (d->orientation == Qt::Vertical) {
        w  = rect.height();
        dx = rect.top() - pos.y();
    } else {
        w  = rect.width();
        dx = pos.x() - rect.left();
    }

    if (w == 0.0)
        return 0.0;

    if (d->inverted) {
        dx = w - dx;
    }

    // w pixels is an arc of viewAngle degrees,
    // so we convert change in pixels to change in angle
    const double ang = dx * d->viewAngle / w;

    // value range maps to totalAngle degrees,
    // so convert the change in angle to a change in value
    const double val = ang * (maximum() - minimum()) / d->totalAngle;

    return val;
}

/*!
   @brief Qt Paint Event
   @param event Paint event
   *
 */
void QwtWheel::paintEvent(QPaintEvent* event)
{
    QWT_D(d);
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (d->flatStyle) {
        const QRect r = contentsRect();
        const int bw = d->borderWidth;
        painter.save();
        painter.setPen(QPen(palette().color(QPalette::Mid), bw));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r.adjusted(bw / 2, bw / 2, -bw / 2 - 1, -bw / 2 - 1));
        painter.restore();
    } else {
        qDrawShadePanel(&painter, contentsRect(), palette(), true, d->borderWidth);
    }

    drawWheelBackground(&painter, wheelRect());
    drawTicks(&painter, wheelRect());

    if (hasFocus())
        QwtPainter::drawFocusRect(&painter, this);
}

/*!
   Draw the Wheel's background gradient

   @param painter Painter
   @param rect Geometry for the wheel
 */
void QwtWheel::drawWheelBackground(QPainter* painter, const QRectF& rect)
{
    QWT_D(d);
    painter->save();

    QPalette pal = palette();

    if (d->flatStyle) {
        painter->fillRect(rect, pal.brush(QPalette::Button));

        const QPen borderPen(pal.color(QPalette::Mid), d->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap);
        const double bw2 = 0.5 * d->wheelBorderWidth;

        if (d->orientation == Qt::Horizontal) {
            painter->setPen(borderPen);
            painter->drawLine(QPointF(rect.left(), rect.top() + bw2), QPointF(rect.right(), rect.top() + bw2));
            painter->drawLine(QPointF(rect.left(), rect.bottom() - bw2), QPointF(rect.right(), rect.bottom() - bw2));
        } else {
            painter->setPen(borderPen);
            painter->drawLine(QPointF(rect.left() + bw2, rect.top()), QPointF(rect.left() + bw2, rect.bottom()));
            painter->drawLine(QPointF(rect.right() - bw2, rect.top()), QPointF(rect.right() - bw2, rect.bottom()));
        }
    } else {
        //  draw shaded background
        QLinearGradient gradient(rect.topLeft(), (d->orientation == Qt::Horizontal) ? rect.topRight() : rect.bottomLeft());
        gradient.setColorAt(0.0, pal.color(QPalette::Button));
        gradient.setColorAt(0.2, pal.color(QPalette::Midlight));
        gradient.setColorAt(0.7, pal.color(QPalette::Mid));
        gradient.setColorAt(1.0, pal.color(QPalette::Dark));

        painter->fillRect(rect, gradient);

        // draw internal border

        const QPen lightPen(palette().color(QPalette::Light), d->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap);
        const QPen darkPen(pal.color(QPalette::Dark), d->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap);

        const double bw2 = 0.5 * d->wheelBorderWidth;

        if (d->orientation == Qt::Horizontal) {
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
    }

    painter->restore();
}

/*!
   Draw the Wheel's ticks

   @param painter Painter
   @param rect Geometry for the wheel
 */
void QwtWheel::drawTicks(QPainter* painter, const QRectF& rect)
{
    QWT_D(d);
    const double range = d->maximum - d->minimum;

    if (range == 0.0 || d->totalAngle == 0.0) {
        return;
    }

    const QPen lightPen(palette().color(QPalette::Light), 0, Qt::SolidLine, Qt::FlatCap);
    const QPen darkPen(palette().color(QPalette::Dark), 0, Qt::SolidLine, Qt::FlatCap);
    const QPen flatPen(palette().color(QPalette::Mid), 0, Qt::SolidLine, Qt::FlatCap);

    const double cnvFactor = qAbs(d->totalAngle / range);
    const double halfIntv  = 0.5 * d->viewAngle / cnvFactor;
    const double loValue   = value() - halfIntv;
    const double hiValue   = value() + halfIntv;
    const double tickWidth = 360.0 / double(d->tickCount) / cnvFactor;
    const double sinArc    = qFastSin(d->viewAngle * M_PI / 360.0);

    if (d->orientation == Qt::Horizontal) {
        const double radius = rect.width() * 0.5;

        double l1 = rect.top() + d->wheelBorderWidth;
        double l2 = rect.bottom() - d->wheelBorderWidth - 1;

        // draw one point over the border if border > 1
        if (d->wheelBorderWidth > 1) {
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
            if (d->inverted)
                tickPos = rect.left() + off;
            else
                tickPos = rect.right() - off;

            if ((tickPos <= maxpos) && (tickPos > minpos)) {
                if (d->flatStyle) {
                    painter->setPen(flatPen);
                    painter->drawLine(QPointF(tickPos, l1), QPointF(tickPos, l2));
                } else {
                    painter->setPen(darkPen);
                    painter->drawLine(QPointF(tickPos - 1, l1), QPointF(tickPos - 1, l2));
                    painter->setPen(lightPen);
                    painter->drawLine(QPointF(tickPos, l1), QPointF(tickPos, l2));
                }
            }
        }
    } else  // Qt::Vertical
    {
        const double radius = rect.height() * 0.5;

        double l1 = rect.left() + d->wheelBorderWidth;
        double l2 = rect.right() - d->wheelBorderWidth - 1;

        if (d->wheelBorderWidth > 1) {
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

            if (d->inverted)
                tickPos = rect.bottom() - off;
            else
                tickPos = rect.top() + off;

            if ((tickPos <= maxpos) && (tickPos > minpos)) {
                if (d->flatStyle) {
                    painter->setPen(flatPen);
                    painter->drawLine(QPointF(l1, tickPos), QPointF(l2, tickPos));
                } else {
                    painter->setPen(darkPen);
                    painter->drawLine(QPointF(l1, tickPos - 1), QPointF(l2, tickPos - 1));
                    painter->setPen(lightPen);
                    painter->drawLine(QPointF(l1, tickPos), QPointF(l2, tickPos));
                }
            }
        }
    }
}

/**
 * @brief Set the width of the wheel
 * @details Corresponds to the wheel height for horizontal orientation,
 *          and the wheel width for vertical orientation.
 * @param[in] width The wheel's width
 * @sa wheelWidth()
 */
void QwtWheel::setWheelWidth(int width)
{
    QWT_D(d);
    d->wheelWidth = width;
    update();
}

/**
 * @brief Return the width of the wheel
 * @return Wheel width
 * @sa setWheelWidth()
 */
int QwtWheel::wheelWidth() const
{
    QWT_DC(d);
    return d->wheelWidth;
}

/**
 * @brief Return the size hint
 * @return Size hint
 */
QSize QwtWheel::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return qwtExpandedToGlobalStrut(hint);
}

/**
 * @brief Return the minimum size hint
 * @return Minimum size hint
 * @warning The return value is based on the wheel width.
 */
QSize QwtWheel::minimumSizeHint() const
{
    QWT_DC(d);
    QSize sz(3 * d->wheelWidth + 2 * d->borderWidth, d->wheelWidth + 2 * d->borderWidth);
    if (d->orientation != Qt::Horizontal)
        sz.transpose();

    return sz;
}

/**
 * @brief Set the step size of the counter
 * @details A value <= 0.0 disables stepping.
 * @param[in] stepSize Single step size
 * @sa singleStep(), setPageStepCount()
 */
void QwtWheel::setSingleStep(double stepSize)
{
    QWT_D(d);
    d->singleStep = qwtMaxF(stepSize, 0.0);
}

/**
 * @brief Return the single step size
 * @return Single step size
 * @sa setSingleStep()
 */
double QwtWheel::singleStep() const
{
    QWT_DC(d);
    return d->singleStep;
}

/**
 * @brief En/Disable step alignment
 * @details When step alignment is enabled, value changes initiated by
 *          user input (mouse, keyboard, wheel) are aligned to multiples of the single step.
 * @param[in] on On/Off
 * @sa stepAlignment(), setSingleStep()
 */
void QwtWheel::setStepAlignment(bool on)
{
    QWT_D(d);
    if (on != d->stepAlignment) {
        d->stepAlignment = on;
    }
}

/**
 * @brief Return whether step alignment is enabled
 * @return True when step alignment is enabled
 * @sa setStepAlignment(), singleStep()
 */
bool QwtWheel::stepAlignment() const
{
    QWT_DC(d);
    return d->stepAlignment;
}

/**
 * @brief Set the page step count
 * @details pageStepCount is a multiplicator for the single step size
 *          that typically corresponds to the user pressing PageUp or PageDown.
 *          A value of 0 disables page stepping. The default value is 1.
 * @param[in] count Multiplicator for the single step size
 * @sa pageStepCount(), setSingleStep()
 */
void QwtWheel::setPageStepCount(int count)
{
    QWT_D(d);
    d->pageStepCount = qMax(0, count);
}

/**
 * @brief Return the page step count
 * @return Page step count
 * @sa setPageStepCount(), singleStep()
 */
int QwtWheel::pageStepCount() const
{
    QWT_DC(d);
    return d->pageStepCount;
}

/**
 * @brief Set the minimum and maximum values
 * @details The maximum is adjusted if necessary to ensure that the range remains valid.
 *          The value might be modified to be inside of the range.
 * @param[in] min Minimum value
 * @param[in] max Maximum value
 * @sa minimum(), maximum()
 */
void QwtWheel::setRange(double min, double max)
{
    QWT_D(d);
    max = qwtMaxF(min, max);

    if (d->minimum == min && d->maximum == max)
        return;

    d->minimum = min;
    d->maximum = max;

    if (d->value < min || d->value > max) {
        d->value = qBound(min, d->value, max);

        update();
        Q_EMIT valueChanged(d->value);
    }
}
/**
 * @brief Set the minimum value of the range
 * @param[in] value Minimum value
 * @sa setRange(), setMaximum(), minimum()
 * @note The maximum is adjusted if necessary to ensure that the range remains valid.
 */
void QwtWheel::setMinimum(double value)
{
    setRange(value, maximum());
}

/**
 * @brief Return the minimum of the range
 * @return Minimum value
 * @sa setRange(), setMinimum(), maximum()
 */
double QwtWheel::minimum() const
{
    QWT_DC(d);
    return d->minimum;
}

/**
 * @brief Set the maximum value of the range
 * @param[in] value Maximum value
 * @sa setRange(), setMinimum(), maximum()
 */
void QwtWheel::setMaximum(double value)
{
    setRange(minimum(), value);
}

/**
 * @brief Return the maximum of the range
 * @return Maximum value
 * @sa setRange(), setMaximum(), minimum()
 */
double QwtWheel::maximum() const
{
    QWT_DC(d);
    return d->maximum;
}

/**
 * @brief Set a new value without adjusting to the step raster
 * @param[in] value New value
 * @sa value(), valueChanged()
 * @warning The value is clipped when it lies outside the range.
 */
void QwtWheel::setValue(double value)
{
    QWT_D(d);
    stopFlying();
    d->isScrolling = false;

    value = qBound(d->minimum, value, d->maximum);

    if (d->value != value) {
        d->value = value;

        update();
        Q_EMIT valueChanged(d->value);
    }
}

/**
 * @brief Return the current value of the wheel
 * @return Current value
 * @sa setValue(), valueChanged()
 */
double QwtWheel::value() const
{
    QWT_DC(d);
    return d->value;
}

/**
 * @brief En/Disable inverted appearance
 * @details An inverted wheel increases its values in the opposite direction.
 *          The direction of an inverted horizontal wheel will be from right to left,
 *          an inverted vertical wheel will increase from bottom to top.
 * @param[in] on En/Disable inverted appearance
 * @sa isInverted()
 */
void QwtWheel::setInverted(bool on)
{
    QWT_D(d);
    if (d->inverted != on) {
        d->inverted = on;
        update();
    }
}

/**
 * @brief Return whether the wheel is inverted
 * @return True when the wheel is inverted
 * @sa setInverted()
 */
bool QwtWheel::isInverted() const
{
    QWT_DC(d);
    return d->inverted;
}

/**
 * @brief En/Disable wrapping
 * @details If wrapping is true, stepping up from maximum() value will take
 *          you to the minimum() value and vice versa.
 * @param[in] on En/Disable wrapping
 * @sa wrapping()
 */
void QwtWheel::setWrapping(bool on)
{
    QWT_D(d);
    d->wrapping = on;
}

/**
 * @brief Return whether wrapping is enabled
 * @return True when wrapping is set
 * @sa setWrapping()
 */
bool QwtWheel::wrapping() const
{
    QWT_DC(d);
    return d->wrapping;
}

/**
 * @brief Set the slider's mass for flywheel effect
 * @details If the slider's mass is greater than 0, it will continue to move
 *          after the mouse button has been released. Its speed decreases
 *          with time at a rate depending on the slider's mass.
 *          A large mass means that it will continue to move for a long time.
 *          Derived widgets may overload this function to make it public.
 * @param[in] mass New mass in kg
 * @warning If the mass is smaller than 1g, it is set to zero.
 *          The maximal mass is limited to 100kg.
 * @sa mass()
 */
void QwtWheel::setMass(double mass)
{
    QWT_D(d);
    if (mass < 0.001) {
        d->mass = 0.0;
    } else {
        d->mass = qwtMinF(100.0, mass);
    }

    if (d->mass <= 0.0)
        stopFlying();
}

/**
 * @brief Return the mass for flywheel effect
 * @return Mass in kg
 * @sa setMass()
 */
double QwtWheel::mass() const
{
    QWT_DC(d);
    return d->mass;
}

/*!
   @brief Stop the flying movement of the wheel
   *
 */
void QwtWheel::stopFlying()
{
    QWT_D(d);
    if (d->timerId != 0) {
        killTimer(d->timerId);
        d->timerId = 0;
        d->speed   = 0.0;
    }
}

double QwtWheel::boundedValue(double value) const
{
    QWT_DC(d);
    const double range = d->maximum - d->minimum;

    if (d->wrapping && range >= 0.0) {
        if (value < d->minimum) {
            value += std::ceil((d->minimum - value) / range) * range;
        } else if (value > d->maximum) {
            value -= std::ceil((value - d->maximum) / range) * range;
        }
    } else {
        value = qBound(d->minimum, value, d->maximum);
    }

    return value;
}

double QwtWheel::alignedValue(double value) const
{
    QWT_DC(d);
    const double stepSize = d->singleStep;

    if (stepSize > 0.0) {
        value = d->minimum + qRound((value - d->minimum) / stepSize) * stepSize;

        if (stepSize > 1e-12) {
            if (qFuzzyCompare(value + 1.0, 1.0)) {
                // correct rounding error if value = 0
                value = 0.0;
            } else if (qFuzzyCompare(value, d->maximum)) {
                // correct rounding error at the border
                value = d->maximum;
            }
        }
    }

    return value;
}
