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

#include "qwt_plot_rescaler.h"
#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include "qwt_interval.h"
#include "qwt_plot_canvas.h"

#include <qevent.h>

class QwtPlotRescaler::AxisData
{
public:
    AxisData() : aspectRatio(1.0), expandingDirection(QwtPlotRescaler::ExpandUp)
    {
    }

    double aspectRatio;
    QwtInterval intervalHint;
    QwtPlotRescaler::ExpandingDirection expandingDirection;
    mutable QwtScaleDiv scaleDiv;
};

class QwtPlotRescaler::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotRescaler)
public:
    PrivateData(QwtPlotRescaler* p)
        : q_ptr(p)
        , referenceAxis(QwtAxis::XBottom), rescalePolicy(QwtPlotRescaler::Expanding), isEnabled(false), inReplot(0)
    {
    }

    QwtPlotRescaler::AxisData* axisData(QwtAxisId axisId)
    {
        if (!QwtAxis::isValid(axisId))
            return nullptr;

        return &m_axisData[ axisId ];
    }

    const QwtPlotRescaler::AxisData* axisData(QwtAxisId axisId) const
    {
        if (!QwtAxis::isValid(axisId))
            return nullptr;

        return &m_axisData[ axisId ];
    }

    QwtAxisId referenceAxis;
    RescalePolicy rescalePolicy;
    bool isEnabled;

    mutable int inReplot;

private:
    QwtPlotRescaler::AxisData m_axisData[ QwtAxis::AxisPositions ];
};

/*!
   @brief Constructs a rescaler for the given canvas
   @param[in] canvas Canvas widget
   @param[in] referenceAxis Reference axis, see RescalePolicy
   @param[in] policy Rescale policy
   @sa setRescalePolicy(), setReferenceAxis()

 */
QwtPlotRescaler::QwtPlotRescaler(QWidget* canvas, QwtAxisId referenceAxis, RescalePolicy policy) : QObject(canvas), QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    d->referenceAxis = referenceAxis;
    d->rescalePolicy = policy;

    setEnabled(true);
}

/*!
   @brief Destructs the rescaler

 */
QwtPlotRescaler::~QwtPlotRescaler()
{
}

/*!
   @brief Enables or disables the rescaler
   @details When enabled is true, an event filter is installed for the canvas,
            otherwise the event filter is removed.
   @param[in] on true to enable, false to disable
   @sa isEnabled(), eventFilter()

 */
void QwtPlotRescaler::setEnabled(bool on)
{
    QWT_D(d);
    if (d->isEnabled != on) {
        d->isEnabled = on;

        QWidget* w = canvas();
        if (w) {
            if (d->isEnabled)
                w->installEventFilter(this);
            else
                w->removeEventFilter(this);
        }
    }
}

/*!
   @brief Checks if the rescaler is enabled
   @return true if enabled, false otherwise
   @sa setEnabled(), eventFilter()

 */
bool QwtPlotRescaler::isEnabled() const
{
    QWT_DC(d);
    return d->isEnabled;
}

/*!
   @brief Sets the rescale policy
   @param[in] policy Rescale policy
   @sa rescalePolicy()

 */
void QwtPlotRescaler::setRescalePolicy(RescalePolicy policy)
{
    QWT_D(d);
    d->rescalePolicy = policy;
}

/*!
   @brief Gets the rescale policy
   @return Rescale policy
   @sa setRescalePolicy()

 */
QwtPlotRescaler::RescalePolicy QwtPlotRescaler::rescalePolicy() const
{
    QWT_DC(d);
    return d->rescalePolicy;
}

/*!
   @brief Sets the reference axis
   @details The reference axis is used as the basis for rescale calculations.
   @param[in] axisId Axis identifier
   @sa referenceAxis()

 */
void QwtPlotRescaler::setReferenceAxis(QwtAxisId axisId)
{
    QWT_D(d);
    d->referenceAxis = axisId;
}

/*!
   @brief Gets the reference axis
   @return Reference axis identifier
   @sa setReferenceAxis()

 */
QwtAxisId QwtPlotRescaler::referenceAxis() const
{
    QWT_DC(d);
    return d->referenceAxis;
}

/*!
   @brief Sets the expanding direction for all axes
   @param[in] direction Expanding direction
   @sa expandingDirection()

 */
void QwtPlotRescaler::setExpandingDirection(ExpandingDirection direction)
{
    for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
        setExpandingDirection(axis, direction);
}

/*!
   @brief Sets the expanding direction for a specific axis
   @param[in] axisId Axis identifier
   @param[in] direction Expanding direction
   @sa expandingDirection()

 */
void QwtPlotRescaler::setExpandingDirection(QwtAxisId axisId, ExpandingDirection direction)
{
    QWT_D(d);
    if (AxisData* axisData = d->axisData(axisId))
        axisData->expandingDirection = direction;
}

/*!
   @brief Gets the expanding direction for a specific axis
   @param[in] axisId Axis identifier
   @return Expanding direction for the specified axis
   @sa setExpandingDirection()

 */
QwtPlotRescaler::ExpandingDirection QwtPlotRescaler::expandingDirection(QwtAxisId axisId) const
{
    QWT_DC(d);
    if (const AxisData* axisData = d->axisData(axisId))
        return axisData->expandingDirection;

    return ExpandBoth;
}

/*!
   @brief Sets the aspect ratio for all axes
   @details Sets the aspect ratio between the scale of the reference axis
            and other scales. The default ratio is 1.0.
   @param[in] ratio Aspect ratio
   @sa aspectRatio()

 */
void QwtPlotRescaler::setAspectRatio(double ratio)
{
    for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
        setAspectRatio(axis, ratio);
}

/*!
   @brief Sets the aspect ratio for a specific axis
   @details Sets the aspect ratio between the scale of the reference axis
            and another scale. The default ratio is 1.0.
   @param[in] axisId Axis identifier
   @param[in] ratio Aspect ratio
   @sa aspectRatio()

 */
void QwtPlotRescaler::setAspectRatio(QwtAxisId axisId, double ratio)
{
    QWT_D(d);
    if (AxisData* axisData = d->axisData(axisId)) {
        if (ratio < 0.0)
            ratio = 0.0;

        axisData->aspectRatio = ratio;
    }
}

/*!
   @brief Gets the aspect ratio for a specific axis
   @param[in] axisId Axis identifier
   @return Aspect ratio between the specified axis and the reference axis
   @sa setAspectRatio()

 */
double QwtPlotRescaler::aspectRatio(QwtAxisId axisId) const
{
    QWT_DC(d);
    if (const AxisData* axisData = d->axisData(axisId))
        return axisData->aspectRatio;

    return 0.0;
}

/*!
   @brief Sets an interval hint for an axis
   @details In Fitting mode, the hint is used as the minimal interval
            that always needs to be displayed.
   @param[in] axisId Axis identifier
   @param[in] interval Interval hint
   @sa intervalHint(), RescalePolicy

 */
void QwtPlotRescaler::setIntervalHint(QwtAxisId axisId, const QwtInterval& interval)
{
    QWT_D(d);
    if (AxisData* axisData = d->axisData(axisId))
        axisData->intervalHint = interval;
}

/*!
   @brief Gets the interval hint for an axis
   @param[in] axisId Axis identifier
   @return Interval hint
   @sa setIntervalHint(), RescalePolicy

 */
QwtInterval QwtPlotRescaler::intervalHint(QwtAxisId axisId) const
{
    QWT_DC(d);
    if (const AxisData* axisData = d->axisData(axisId))
        return axisData->intervalHint;

    return QwtInterval();
}

/*!
   @brief Gets the canvas widget
   @return Canvas widget

 */
QWidget* QwtPlotRescaler::canvas()
{
    return qobject_cast< QWidget* >(parent());
}

/*!
   @brief Gets the canvas widget (const version)
   @return Canvas widget (const)

 */
const QWidget* QwtPlotRescaler::canvas() const
{
    return qobject_cast< const QWidget* >(parent());
}

/*!
   @brief Gets the plot widget
   @return Plot widget

 */
QwtPlot* QwtPlotRescaler::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/*!
   @brief Gets the plot widget (const version)
   @return Plot widget (const)

 */
const QwtPlot* QwtPlotRescaler::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
   @brief Event filter for the plot canvas
   @details Handles resize and polish request events for the canvas.
   @param[in] object Object that received the event
   @param[in] event Event that was received
   @return false to allow further processing of the event

 */
bool QwtPlotRescaler::eventFilter(QObject* object, QEvent* event)
{
    if (object && object == canvas()) {
        switch (event->type()) {
        case QEvent::Resize: {
            canvasResizeEvent(static_cast< QResizeEvent* >(event));
            break;
        }
        case QEvent::PolishRequest: {
            rescale();
            break;
        }
        default:;
        }
    }

    return false;
}

/*!
   Event handler for resize events of the plot canvas

   @param event Resize event
   @sa rescale()
 */
void QwtPlotRescaler::canvasResizeEvent(QResizeEvent* event)
{
    const QMargins m = canvas()->contentsMargins();
    const QSize marginSize(m.left() + m.right(), m.top() + m.bottom());

    const QSize newSize = event->size() - marginSize;
    const QSize oldSize = event->oldSize() - marginSize;

    rescale(oldSize, newSize);
}

/*!
   @brief Rescales the plot axes
   @details Adjusts the plot axes scales based on the current canvas size.

 */
void QwtPlotRescaler::rescale() const
{
    const QSize size = canvas()->contentsRect().size();
    rescale(size, size);
}

/*!
   Adjust the plot axes scales

   @param oldSize Previous size of the canvas
   @param newSize New size of the canvas
 */
void QwtPlotRescaler::rescale(const QSize& oldSize, const QSize& newSize) const
{
    if (newSize.isEmpty())
        return;

    QwtInterval intervals[ QwtAxis::AxisPositions ];
    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        const QwtAxisId axisId(axisPos);
        intervals[ axisPos ] = interval(axisId);
    }

    const QwtAxisId refAxis = referenceAxis();
    intervals[ refAxis ]    = expandScale(refAxis, oldSize, newSize);

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        const QwtAxisId axisId(axisPos);
        if (aspectRatio(axisId) > 0.0 && axisId != refAxis) {
            intervals[ axisPos ] = syncScale(axisId, intervals[ refAxis ], newSize);
        }
    }

    updateScales(intervals);
}

/*!
   Calculate the new scale interval of a plot axis

   @param axisId Axis
   @param oldSize Previous size of the canvas
   @param newSize New size of the canvas

   @return Calculated new interval for the axis
 */
QwtInterval QwtPlotRescaler::expandScale(QwtAxisId axisId, const QSize& oldSize, const QSize& newSize) const
{
    const QwtInterval oldInterval = interval(axisId);

    QwtInterval expanded = oldInterval;
    switch (rescalePolicy()) {
    case Fixed: {
        break;  // do nothing
    }
    case Expanding: {
        if (!oldSize.isEmpty()) {
            double width = oldInterval.width();
            if (orientation(axisId) == Qt::Horizontal)
                width *= double(newSize.width()) / oldSize.width();
            else
                width *= double(newSize.height()) / oldSize.height();

            expanded = expandInterval(oldInterval, width, expandingDirection(axisId));
        }
        break;
    }
    case Fitting: {
        double dist = 0.0;
        for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
            const QwtAxisId axisId(axisPos);
            const double d = pixelDist(axisId, newSize);
            if (d > dist)
                dist = d;
        }
        if (dist > 0.0) {
            double width;
            if (orientation(axisId) == Qt::Horizontal)
                width = newSize.width() * dist;
            else
                width = newSize.height() * dist;

            expanded = expandInterval(intervalHint(axisId), width, expandingDirection(axisId));
        }
        break;
    }
    }

    return expanded;
}

/*!
   Synchronize an axis scale according to the scale of the reference axis

   @param axisId Axis
   @param reference Interval of the reference axis
   @param size Size of the canvas

   @return New interval for axis
 */
QwtInterval QwtPlotRescaler::syncScale(QwtAxisId axisId, const QwtInterval& reference, const QSize& size) const
{
    double dist;
    if (orientation(referenceAxis()) == Qt::Horizontal)
        dist = reference.width() / size.width();
    else
        dist = reference.width() / size.height();

    if (orientation(axisId) == Qt::Horizontal)
        dist *= size.width();
    else
        dist *= size.height();

    dist /= aspectRatio(axisId);

    QwtInterval intv;
    if (rescalePolicy() == Fitting)
        intv = intervalHint(axisId);
    else
        intv = interval(axisId);

    intv = expandInterval(intv, dist, expandingDirection(axisId));

    return intv;
}

/*!
   @return Orientation of an axis
   @param axisId Axis
 */
Qt::Orientation QwtPlotRescaler::orientation(QwtAxisId axisId) const
{
    return QwtAxis::isYAxis(axisId) ? Qt::Vertical : Qt::Horizontal;
}

/*!
   @param axisId Axis
   @return Normalized interval of an axis
 */
QwtInterval QwtPlotRescaler::interval(QwtAxisId axisId) const
{
    if (!plot()->isAxisValid(axisId))
        return QwtInterval();

    return plot()->axisScaleDiv(axisId).interval().normalized();
}

/*!
   Expand the interval

   @param interval Interval to be expanded
   @param width Distance to be added to the interval
   @param direction Direction of the expand operation

   @return Expanded interval
 */
QwtInterval QwtPlotRescaler::expandInterval(const QwtInterval& interval, double width, ExpandingDirection direction) const
{
    QwtInterval expanded = interval;

    switch (direction) {
    case ExpandUp:
        expanded.setMinValue(interval.minValue());
        expanded.setMaxValue(interval.minValue() + width);
        break;

    case ExpandDown:
        expanded.setMaxValue(interval.maxValue());
        expanded.setMinValue(interval.maxValue() - width);
        break;

    case ExpandBoth:
    default:
        expanded.setMinValue(interval.minValue() + interval.width() / 2.0 - width / 2.0);
        expanded.setMaxValue(expanded.minValue() + width);
    }
    return expanded;
}

double QwtPlotRescaler::pixelDist(QwtAxisId axisId, const QSize& size) const
{
    const QwtInterval intv = intervalHint(axisId);

    double dist = 0.0;
    if (!intv.isNull()) {
        if (axisId == referenceAxis()) {
            dist = intv.width();
        } else {
            const double r = aspectRatio(axisId);
            if (r > 0.0)
                dist = intv.width() * r;
        }
    }

    if (dist > 0.0) {
        if (orientation(axisId) == Qt::Horizontal)
            dist /= size.width();
        else
            dist /= size.height();
    }

    return dist;
}

/*!
   Update the axes scales

   @param intervals Scale intervals
 */
void QwtPlotRescaler::updateScales(QwtInterval intervals[ QwtAxis::AxisPositions ]) const
{
    QWT_DC(d);
    if (d->inReplot >= 5) {
        return;
    }

    QwtPlot* plt = const_cast< QwtPlot* >(plot());
    plt->saveAutoReplotState();
    plt->setAutoReplot(false);

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        {
            const QwtAxisId axisId(axisPos);

            if (axisId == referenceAxis() || aspectRatio(axisId) > 0.0) {
                double v1 = intervals[ axisPos ].minValue();
                double v2 = intervals[ axisPos ].maxValue();

                if (!plt->axisScaleDiv(axisId).isIncreasing())
                    qSwap(v1, v2);

                if (d->inReplot >= 1)
                    d->axisData(axisId)->scaleDiv = plt->axisScaleDiv(axisId);

                if (d->inReplot >= 2) {
                    QList< double > ticks[ QwtScaleDiv::NTickTypes ];
                    for (int t = 0; t < QwtScaleDiv::NTickTypes; t++)
                        ticks[ t ] = d->axisData(axisId)->scaleDiv.ticks(t);

                    plt->setAxisScaleDiv(axisId, QwtScaleDiv(v1, v2, ticks));
                } else {
                    plt->setAxisScale(axisId, v1, v2);
                }
            }
        }
    }

    QwtPlotCanvas* canvas = qobject_cast< QwtPlotCanvas* >(plt->canvas());

    bool immediatePaint = false;
    if (canvas) {
        immediatePaint = canvas->testPaintAttribute(QwtPlotCanvas::ImmediatePaint);
        canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);
    }

    plt->restoreAutoReplotState();

    d->inReplot++;
    plt->replot();
    d->inReplot--;

    if (canvas && immediatePaint) {
        canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
    }
}