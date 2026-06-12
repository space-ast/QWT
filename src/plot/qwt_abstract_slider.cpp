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

#include "qwt_abstract_slider.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_math.h"

#include <qevent.h>

static double qwtAlignToScaleDiv(const QwtAbstractSlider* slider, double value)
{
    const QwtScaleDiv& sd = slider->scaleDiv();

    const int tValue = slider->transform(value);

    if (tValue == slider->transform(sd.lowerBound()))
        return sd.lowerBound();

    if (tValue == slider->transform(sd.upperBound()))
        return sd.upperBound();

    for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
        const QList< double > ticks = sd.ticks(i);
        for (int j = 0; j < ticks.size(); j++) {
            if (slider->transform(ticks[ j ]) == tValue)
                return ticks[ j ];
        }
    }

    return value;
}

class QwtAbstractSlider::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtAbstractSlider)
public:
    PrivateData(QwtAbstractSlider* p)
        : q_ptr(p)
        , isScrolling(false)
        , isTracking(true)
        , pendingValueChanged(false)
        , readOnly(false)
        , totalSteps(100)
        , singleSteps(1)
        , pageSteps(10)
        , stepAlignment(true)
        , isValid(false)
        , value(0.0)
        , wrapping(false)
        , invertedControls(false)
    {
    }

    bool isScrolling;
    bool isTracking;
    bool pendingValueChanged;

    bool readOnly;

    uint totalSteps;
    uint singleSteps;
    uint pageSteps;
    bool stepAlignment;

    bool isValid;
    double value;

    bool wrapping;
    bool invertedControls;
};

/**
 * @brief Constructor for QwtAbstractSlider
 * @details The scale is initialized to [0.0, 100.0], the
 *          number of steps is set to 100 with 1 and 10 as single
 *          and page step sizes. Step alignment is enabled.
 *          The initial value is invalid.
 * @param parent Parent widget
 */
QwtAbstractSlider::QwtAbstractSlider(QWidget* parent) : QwtAbstractScale(parent), QWT_PIMPL_CONSTRUCT
{
    setScale(0.0, 100.0);
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * @brief Destructor for QwtAbstractSlider
 */
QwtAbstractSlider::~QwtAbstractSlider()
{
}

/**
 * @brief Set the value to be valid or invalid
 * @param on When true, the value is invalidated
 * @sa setValue()
 */
void QwtAbstractSlider::setValid(bool on)
{
    QWT_D(d);
    if (on != d->isValid) {
        d->isValid = on;
        sliderChange();

        Q_EMIT valueChanged(d->value);
    }
}

/**
 * @brief Check if the value is valid
 * @return True if the value is invalid
 */
bool QwtAbstractSlider::isValid() const
{
    QWT_DC(d);
    return d->isValid;
}

/**
 * @brief Enable or disable read-only mode
 * @details In read-only mode the slider can't be controlled by mouse
 *          or keyboard.
 * @param on Enables read-only mode if true
 * @sa isReadOnly()
 * @warning The focus policy is set to Qt::StrongFocus or Qt::NoFocus
 */
void QwtAbstractSlider::setReadOnly(bool on)
{
    QWT_D(d);
    if (d->readOnly != on) {
        d->readOnly = on;
        setFocusPolicy(on ? Qt::StrongFocus : Qt::NoFocus);

        update();
    }
}

/**
 * @brief Check if read-only mode is enabled
 * @return True if read-only mode is enabled
 * @sa setReadOnly()
 */
bool QwtAbstractSlider::isReadOnly() const
{
    QWT_DC(d);
    return d->readOnly;
}

/**
 * @brief Enable or disable tracking
 * @details If tracking is enabled, the slider emits the valueChanged()
 *          signal while the movable part of the slider is being dragged.
 *          If tracking is disabled, the slider emits the valueChanged() signal
 *          only when the user releases the slider.
 *          Tracking is enabled by default.
 * @param on True to enable tracking, false to disable
 * @sa isTracking(), sliderMoved()
 */
void QwtAbstractSlider::setTracking(bool on)
{
    QWT_D(d);
    d->isTracking = on;
}

/**
 * @brief Check if tracking is enabled
 * @return True if tracking is enabled
 * @sa setTracking()
 */
bool QwtAbstractSlider::isTracking() const
{
    QWT_DC(d);
    return d->isTracking;
}

/**
 * @brief Handle mouse press events
 * @param event Mouse event
 * @details Initiates scrolling if the position is valid.
 * @sa mouseMoveEvent(), mouseReleaseEvent()
 */
void QwtAbstractSlider::mousePressEvent(QMouseEvent* event)
{
    QWT_D(d);
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (!d->isValid || lowerBound() == upperBound())
        return;

    d->isScrolling = isScrollPosition(event->pos());

    if (d->isScrolling) {
        d->pendingValueChanged = false;

        Q_EMIT sliderPressed();
    }
}

/**
 * @brief Handle mouse move events
 * @param event Mouse event
 * @details Updates the slider value while scrolling.
 * @sa mousePressEvent(), mouseReleaseEvent()
 */
void QwtAbstractSlider::mouseMoveEvent(QMouseEvent* event)
{
    QWT_D(d);
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (d->isValid && d->isScrolling) {
        double value = scrolledTo(event->pos());
        if (value != d->value) {
            value = boundedValue(value);

            if (d->stepAlignment) {
                value = alignedValue(value);
            } else {
                value = qwtAlignToScaleDiv(this, value);
            }

            if (value != d->value) {
                d->value = value;

                sliderChange();

                Q_EMIT sliderMoved(d->value);

                if (d->isTracking)
                    Q_EMIT valueChanged(d->value);
                else
                    d->pendingValueChanged = true;
            }
        }
    }
}

/**
 * @brief Handle mouse release events
 * @param event Mouse event
 * @details Ends scrolling and emits valueChanged() if needed.
 * @sa mousePressEvent(), mouseMoveEvent()
 */
void QwtAbstractSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QWT_D(d);
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (d->isScrolling && d->isValid) {
        d->isScrolling = false;

        if (d->pendingValueChanged)
            Q_EMIT valueChanged(d->value);

        Q_EMIT sliderReleased();
    }
}

/**
 * @brief Handle wheel events
 * @details In/decreases the value by a number of steps. The direction
 *          depends on the invertedControls() property.
 *          When the control or shift modifier is pressed the wheel delta
 *          (divided by 120) is mapped to an increment according to
 *          pageSteps(). Otherwise it is mapped to singleSteps().
 * @param event Wheel event
 * @sa keyPressEvent()
 */
void QwtAbstractSlider::wheelEvent(QWheelEvent* event)
{
    QWT_D(d);
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (!d->isValid || d->isScrolling)
        return;

#if QT_VERSION < 0x050000
    const int wheelDelta = event->delta();
#else
    const QPoint delta   = event->angleDelta();
    const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
#endif

    int numSteps = 0;

    if ((event->modifiers() & Qt::ControlModifier) || (event->modifiers() & Qt::ShiftModifier)) {
        // one page regardless of delta
        numSteps = d->pageSteps;
        if (wheelDelta < 0)
            numSteps = -numSteps;
    } else {
        const int numTurns = (wheelDelta / 120);
        numSteps           = numTurns * d->singleSteps;
    }

    if (d->invertedControls)
        numSteps = -numSteps;

    const double value = incrementedValue(d->value, numSteps);
    if (value != d->value) {
        d->value = value;
        sliderChange();

        Q_EMIT sliderMoved(d->value);
        Q_EMIT valueChanged(d->value);
    }
}

/**
 * @brief Handle key press events
 * @details QwtAbstractSlider handles the following keys:
 *          - Qt::Key_Left: Add/Subtract singleSteps() in direction to lowerBound()
 *          - Qt::Key_Right: Add/Subtract singleSteps() in direction to upperBound()
 *          - Qt::Key_Down: Subtract singleSteps(), when invertedControls() is false
 *          - Qt::Key_Up: Add singleSteps(), when invertedControls() is false
 *          - Qt::Key_PageDown: Subtract pageSteps(), when invertedControls() is false
 *          - Qt::Key_PageUp: Add pageSteps(), when invertedControls() is false
 *          - Qt::Key_Home: Set the value to the minimum()
 *          - Qt::Key_End: Set the value to the maximum()
 * @param event Key event
 * @sa isReadOnly(), wheelEvent()
 */
void QwtAbstractSlider::keyPressEvent(QKeyEvent* event)
{
    QWT_D(d);
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (!d->isValid || d->isScrolling)
        return;

    int numSteps = 0;
    double value = d->value;

    switch (event->key()) {
    case Qt::Key_Left: {
        numSteps = -static_cast< int >(d->singleSteps);
        if (isInverted())
            numSteps = -numSteps;

        break;
    }
    case Qt::Key_Right: {
        numSteps = d->singleSteps;
        if (isInverted())
            numSteps = -numSteps;

        break;
    }
    case Qt::Key_Down: {
        numSteps = -static_cast< int >(d->singleSteps);
        if (d->invertedControls)
            numSteps = -numSteps;
        break;
    }
    case Qt::Key_Up: {
        numSteps = d->singleSteps;
        if (d->invertedControls)
            numSteps = -numSteps;

        break;
    }
    case Qt::Key_PageUp: {
        numSteps = d->pageSteps;
        if (d->invertedControls)
            numSteps = -numSteps;
        break;
    }
    case Qt::Key_PageDown: {
        numSteps = -static_cast< int >(d->pageSteps);
        if (d->invertedControls)
            numSteps = -numSteps;
        break;
    }
    case Qt::Key_Home: {
        value = minimum();
        break;
    }
    case Qt::Key_End: {
        value = maximum();
        break;
    }
    default: {
        event->ignore();
    }
    }

    if (numSteps != 0) {
        value = incrementedValue(d->value, numSteps);
    }

    if (value != d->value) {
        d->value = value;
        sliderChange();

        Q_EMIT sliderMoved(d->value);
        Q_EMIT valueChanged(d->value);
    }
}

/**
 * @brief Set the number of steps
 * @details The range of the slider is divided into a number of steps from
 *          which the value increments according to user inputs depend.
 *          The default setting is 100.
 * @param[in] stepCount Number of steps
 * @sa totalSteps(), setSingleSteps(), setPageSteps()
 */
void QwtAbstractSlider::setTotalSteps(uint stepCount)
{
    QWT_D(d);
    d->totalSteps = stepCount;
}

/**
 * @brief Return the number of steps
 * @return Number of steps
 * @sa setTotalSteps(), singleSteps(), pageSteps()
 */
uint QwtAbstractSlider::totalSteps() const
{
    QWT_DC(d);
    return d->totalSteps;
}

/**
 * @brief Set the number of steps for a single increment
 * @details The range of the slider is divided into a number of steps from
 *          which the value increments according to user inputs depend.
 * @param[in] stepCount Number of steps
 * @sa singleSteps(), setTotalSteps(), setPageSteps()
 */
void QwtAbstractSlider::setSingleSteps(uint stepCount)
{
    QWT_D(d);
    d->singleSteps = stepCount;
}

/**
 * @brief Return the number of single steps
 * @return Number of steps
 * @sa setSingleSteps(), totalSteps(), pageSteps()
 */
uint QwtAbstractSlider::singleSteps() const
{
    QWT_DC(d);
    return d->singleSteps;
}

/**
 * @brief Set the number of page steps
 * @details The range of the slider is divided into a number of steps from
 *          which the value increments according to user inputs depend.
 * @param[in] stepCount Number of steps
 * @sa pageSteps(), setTotalSteps(), setSingleSteps()
 */
void QwtAbstractSlider::setPageSteps(uint stepCount)
{
    QWT_D(d);
    d->pageSteps = stepCount;
}

/**
 * @brief Return the number of page steps
 * @return Number of steps
 * @sa setPageSteps(), totalSteps(), singleSteps()
 */
uint QwtAbstractSlider::pageSteps() const
{
    QWT_DC(d);
    return d->pageSteps;
}

/**
 * @brief Enable step alignment
 * @details When step alignment is enabled values resulting from slider
 *          movements are aligned to the step size.
 * @param on Enable step alignment when true
 * @sa stepAlignment()
 */
void QwtAbstractSlider::setStepAlignment(bool on)
{
    QWT_D(d);
    if (on != d->stepAlignment) {
        d->stepAlignment = on;
    }
}

/**
 * @brief Check if step alignment is enabled
 * @return True, when step alignment is enabled
 * @sa setStepAlignment()
 */
bool QwtAbstractSlider::stepAlignment() const
{
    QWT_DC(d);
    return d->stepAlignment;
}

/**
 * @brief Set the slider to the specified value
 * @param value New value
 * @sa setValid(), sliderChange(), valueChanged()
 */
void QwtAbstractSlider::setValue(double value)
{
    QWT_D(d);
    value = qBound(minimum(), value, maximum());

    const bool changed = (d->value != value) || !d->isValid;

    d->value   = value;
    d->isValid = true;

    if (changed) {
        sliderChange();
        Q_EMIT valueChanged(d->value);
    }
}

/**
 * @brief Return the current value
 * @return Current slider value
 * @sa setValue()
 */
double QwtAbstractSlider::value() const
{
    QWT_DC(d);
    return d->value;
}

/**
 * @brief Enable or disable wrapping
 * @details If wrapping is true stepping up from upperBound() value will
 *          take you to the minimum() value and vice versa.
 * @param on Enable wrapping when true
 * @sa wrapping()
 */
void QwtAbstractSlider::setWrapping(bool on)
{
    QWT_D(d);
    d->wrapping = on;
}

/**
 * @brief Check if wrapping is enabled
 * @return True, when wrapping is set
 * @sa setWrapping()
 */
bool QwtAbstractSlider::wrapping() const
{
    QWT_DC(d);
    return d->wrapping;
}

/**
 * @brief Invert wheel and key events
 * @details Usually scrolling the mouse wheel "up" and using keys like page
 *          up will increase the slider's value towards its maximum.
 *          When invertedControls() is enabled the value is scrolled
 *          towards its minimum.
 *          Inverting the controls might be f.e. useful for a vertical slider
 *          with an inverted scale (decreasing from top to bottom).
 * @param on Invert controls when true
 * @sa invertedControls(), keyEvent(), wheelEvent()
 */
void QwtAbstractSlider::setInvertedControls(bool on)
{
    QWT_D(d);
    d->invertedControls = on;
}

/**
 * @brief Check if controls are inverted
 * @return True, when the controls are inverted
 * @sa setInvertedControls()
 */
bool QwtAbstractSlider::invertedControls() const
{
    QWT_DC(d);
    return d->invertedControls;
}

/**
 * @brief Increment the slider
 * @details The step size depends on the number of totalSteps()
 * @param stepCount Number of steps
 * @sa setTotalSteps(), incrementedValue()
 */
void QwtAbstractSlider::incrementValue(int stepCount)
{
    QWT_D(d);
    const double value = incrementedValue(d->value, stepCount);

    if (value != d->value) {
        d->value = value;
        sliderChange();
    }
}

/**
 * @brief Increment a value
 * @param value Value to increment
 * @param stepCount Number of steps
 * @return Incremented value
 * @sa incrementValue(), setTotalSteps()
 */
double QwtAbstractSlider::incrementedValue(double value, int stepCount) const
{
    QWT_DC(d);
    if (d->totalSteps == 0)
        return value;

    const QwtTransform* transformation = scaleMap().transformation();

    if (transformation == nullptr) {
        const double range = maximum() - minimum();
        value += stepCount * range / d->totalSteps;
    } else {
        QwtScaleMap map = scaleMap();
        map.setPaintInterval(0, d->totalSteps);

        // we need equidistant steps according to
        // paint device coordinates
        const double range = transformation->transform(maximum()) - transformation->transform(minimum());

        const double stepSize = range / d->totalSteps;

        double v = transformation->transform(value);

        v = qRound(v / stepSize) * stepSize;
        v += stepCount * range / d->totalSteps;

        value = transformation->invTransform(v);
    }

    value = boundedValue(value);

    if (d->stepAlignment)
        value = alignedValue(value);

    return value;
}

/**
 * @brief Bound a value to the valid range
 * @details Handles wrapping for circular scales
 * @param value Value to bound
 * @return Bounded value
 * @sa wrapping(), minimum(), maximum()
 */
double QwtAbstractSlider::boundedValue(double value) const
{
    QWT_DC(d);
    const double vmin = minimum();
    const double vmax = maximum();

    if (d->wrapping && vmin != vmax) {
        if (qFuzzyCompare(scaleMap().pDist(), 360.0)) {
            // full circle scales: min and max are the same

            if (qFuzzyCompare(value, vmax)) {
                value = vmin;
            } else {
                const double range = vmax - vmin;

                if (value < vmin) {
                    value += std::ceil((vmin - value) / range) * range;
                } else if (value > vmax) {
                    value -= std::ceil((value - vmax) / range) * range;
                }
            }
        } else {
            if (value < vmin)
                value = vmax;
            else if (value > vmax)
                value = vmin;
        }
    } else {
        value = qBound(vmin, value, vmax);
    }

    return value;
}

/**
 * @brief Align a value to the step size
 * @param value Value to align
 * @return Aligned value
 * @sa stepAlignment(), totalSteps()
 */
double QwtAbstractSlider::alignedValue(double value) const
{
    QWT_DC(d);
    if (d->totalSteps == 0)
        return value;

    double stepSize;

    if (scaleMap().transformation() == nullptr) {
        stepSize = (maximum() - minimum()) / d->totalSteps;
        if (stepSize > 0.0) {
            value = lowerBound() + qRound((value - lowerBound()) / stepSize) * stepSize;
        }
    } else {
        stepSize = (scaleMap().p2() - scaleMap().p1()) / d->totalSteps;

        if (stepSize > 0.0) {
            double v = scaleMap().transform(value);

            v = scaleMap().p1() + qRound((v - scaleMap().p1()) / stepSize) * stepSize;

            value = scaleMap().invTransform(v);
        }
    }

    if (qAbs(stepSize) > 1e-12) {
        if (qFuzzyCompare(value + 1.0, 1.0)) {
            // correct rounding error if value = 0
            value = 0.0;
        } else {
            // correct rounding error at the border
            if (qFuzzyCompare(value, upperBound()))
                value = upperBound();
            else if (qFuzzyCompare(value, lowerBound()))
                value = lowerBound();
        }
    }

    return value;
}

/**
 * @brief Update the slider according to modifications of the scale
 * @details Updates the current value to stay within the new scale range
 *          and emits valueChanged() if the value was adjusted.
 * @sa sliderChange(), valueChanged()
 */
void QwtAbstractSlider::scaleChange()
{
    QWT_D(d);
    const double value = qBound(minimum(), d->value, maximum());

    const bool changed = (value != d->value);
    if (changed) {
        d->value = value;
    }

    if (d->isValid || changed)
        Q_EMIT valueChanged(d->value);

    updateGeometry();
    update();
}

/**
 * @brief Handle slider changes
 * @details Called when the slider needs to update its appearance.
 *          The default implementation calls update().
 * @sa setValue(), incrementValue()
 */
void QwtAbstractSlider::sliderChange()
{
    update();
}
