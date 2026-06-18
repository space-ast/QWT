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

#include "qwt_abstract_scale.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_interval.h"

#include <qcoreevent.h>

class QwtAbstractScale::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtAbstractScale)
public:
    PrivateData(QwtAbstractScale* p) : q_ptr(p), maxMajor(5), maxMinor(3), stepSize(0.0)
    {
        scaleEngine = new QwtLinearScaleEngine();
        scaleDraw   = new QwtScaleDraw();
    }

    ~PrivateData()
    {
        delete scaleEngine;
        delete scaleDraw;
    }

    QwtScaleEngine* scaleEngine;
    QwtAbstractScaleDraw* scaleDraw;

    int maxMajor;
    int maxMinor;
    double stepSize;
};

/**
 * @brief Constructor for QwtAbstractScale
 * @param parent Parent widget
 * @details Creates a default QwtScaleDraw and a QwtLinearScaleEngine.
 *          The initial scale boundaries are set to [ 0.0, 100.0 ]
 *          The scaleStepSize() is initialized to 0.0, scaleMaxMajor() to 5
 *          and scaleMaxMinor to 3.
 */
QwtAbstractScale::QwtAbstractScale(QWidget* parent) : QWidget(parent), QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    rescale(0.0, 100.0, d->stepSize);
}

/**
 * @brief Destructor for QwtAbstractScale
 */
QwtAbstractScale::~QwtAbstractScale()
{
}

/**
 * @brief Set the lower bound of the scale
 * @param value Lower bound value
 * @sa lowerBound(), setScale(), setUpperBound()
 * @note For inverted scales, the lower bound is greater than the upper bound
 */
void QwtAbstractScale::setLowerBound(double value)
{
    setScale(value, upperBound());
}

/**
 * @brief Return the lower bound of the scale
 * @return Lower bound value
 * @sa setLowerBound(), setScale(), upperBound()
 */
double QwtAbstractScale::lowerBound() const
{
    QWT_DC(d);
    return d->scaleDraw->scaleDiv().lowerBound();
}

/**
 * @brief Set the upper bound of the scale
 * @param value Upper bound value
 * @sa upperBound(), setScale(), setLowerBound()
 * @note For inverted scales, the lower bound is greater than the upper bound
 */
void QwtAbstractScale::setUpperBound(double value)
{
    setScale(lowerBound(), value);
}

/**
 * @brief Return the upper bound of the scale
 * @return Upper bound value
 * @sa setUpperBound(), setScale(), lowerBound()
 */
double QwtAbstractScale::upperBound() const
{
    QWT_DC(d);
    return d->scaleDraw->scaleDiv().upperBound();
}

/**
 * @brief Specify a scale by interval bounds
 * @details Define a scale by an interval.
 *          The ticks are calculated using scaleMaxMinor(),
 *          scaleMaxMajor() and scaleStepSize().
 * @param lowerBound Lower limit of the scale interval
 * @param upperBound Upper limit of the scale interval
 * @note For inverted scales, the lower bound is greater than the upper bound
 */
void QwtAbstractScale::setScale(double lowerBound, double upperBound)
{
    QWT_D(d);
    rescale(lowerBound, upperBound, d->stepSize);
}

/**
 * @brief Specify a scale by interval
 * @details Define a scale by an interval.
 *          The ticks are calculated using scaleMaxMinor(),
 *          scaleMaxMajor() and scaleStepSize().
 * @param interval Interval object
 */
void QwtAbstractScale::setScale(const QwtInterval& interval)
{
    setScale(interval.minValue(), interval.maxValue());
}

/**
 * @brief Specify a scale by scale division
 * @details When using this method, scaleMaxMinor(), scaleMaxMajor() and
 *          scaleStepSize() have no effect.
 * @param scaleDiv Scale division object
 * @sa setAutoScale()
 */
void QwtAbstractScale::setScale(const QwtScaleDiv& scaleDiv)
{
    QWT_D(d);
    if (scaleDiv != d->scaleDraw->scaleDiv()) {
#if 1
        if (d->scaleEngine) {
            d->scaleDraw->setTransformation(d->scaleEngine->transformation());
        }
#endif

        d->scaleDraw->setScaleDiv(scaleDiv);

        scaleChange();
    }
}

/**
 * @brief Set the maximum number of major tick intervals
 * @details The scale's major ticks are calculated automatically such that
 *          the number of major intervals does not exceed ticks.
 *          The default value is 5.
 * @param ticks Maximal number of major ticks
 * @sa scaleMaxMajor(), setScaleMaxMinor(), setScaleStepSize(), QwtScaleEngine::divideInterval()
 */
void QwtAbstractScale::setScaleMaxMajor(int ticks)
{
    QWT_D(d);
    if (ticks != d->maxMajor) {
        d->maxMajor = ticks;
        updateScaleDraw();
    }
}

/**
 * @brief Return the maximum number of major tick intervals
 * @return Maximal number of major tick intervals
 * @sa setScaleMaxMajor(), scaleMaxMinor()
 */
int QwtAbstractScale::scaleMaxMajor() const
{
    QWT_DC(d);
    return d->maxMajor;
}

/**
 * @brief Set the maximum number of minor tick intervals
 * @details The scale's minor ticks are calculated automatically such that
 *          the number of minor intervals does not exceed ticks.
 *          The default value is 3.
 * @param ticks Maximal number of minor ticks
 * @sa scaleMaxMajor(), setScaleMaxMinor(), setScaleStepSize(), QwtScaleEngine::divideInterval()
 */
void QwtAbstractScale::setScaleMaxMinor(int ticks)
{
    QWT_D(d);
    if (ticks != d->maxMinor) {
        d->maxMinor = ticks;
        updateScaleDraw();
    }
}

/**
 * @brief Return the maximum number of minor tick intervals
 * @return Maximal number of minor tick intervals
 * @sa setScaleMaxMinor(), scaleMaxMajor()
 */
int QwtAbstractScale::scaleMaxMinor() const
{
    QWT_DC(d);
    return d->maxMinor;
}

/**
 * @brief Set the step size used for calculating scale division
 * @details The step size is a hint for calculating the intervals for
 *          the major ticks of the scale. A value of 0.0 is interpreted
 *          as no hint.
 * @param stepSize Hint for the step size of the scale
 * @sa scaleStepSize(), QwtScaleEngine::divideScale()
 * @note Position and distance between major ticks also depends on scaleMaxMajor()
 */
void QwtAbstractScale::setScaleStepSize(double stepSize)
{
    QWT_D(d);
    if (stepSize != d->stepSize) {
        d->stepSize = stepSize;
        updateScaleDraw();
    }
}

/**
 * @brief Return the step size hint
 * @return Hint for the step size of the scale
 * @sa setScaleStepSize(), QwtScaleEngine::divideScale()
 */
double QwtAbstractScale::scaleStepSize() const
{
    QWT_DC(d);
    return d->stepSize;
}

/**
 * @brief Set the scale draw object
 * @details scaleDraw must be created with new and will be deleted in
 *          the destructor or the next call of setAbstractScaleDraw().
 * @sa abstractScaleDraw()
 */
void QwtAbstractScale::setAbstractScaleDraw(QwtAbstractScaleDraw* scaleDraw)
{
    QWT_D(d);
    if (scaleDraw == nullptr || scaleDraw == d->scaleDraw)
        return;

    if (d->scaleDraw != nullptr)
        scaleDraw->setScaleDiv(d->scaleDraw->scaleDiv());

    delete d->scaleDraw;
    d->scaleDraw = scaleDraw;
}

/**
 * @brief Return the scale draw object (non-const)
 * @return Scale draw object
 * @sa setAbstractScaleDraw()
 */
QwtAbstractScaleDraw* QwtAbstractScale::abstractScaleDraw()
{
    QWT_D(d);
    return d->scaleDraw;
}

/**
 * @brief Return the scale draw object (const)
 * @return Scale draw object
 * @sa setAbstractScaleDraw()
 */
const QwtAbstractScaleDraw* QwtAbstractScale::abstractScaleDraw() const
{
    QWT_DC(d);
    return d->scaleDraw;
}

/**
 * @brief Set the scale engine
 * @details The scale engine is responsible for calculating the scale division
 *          and provides a transformation between scale and widget coordinates.
 *          scaleEngine must be created with new and will be deleted in
 *          the destructor or the next call of setScaleEngine().
 * @param scaleEngine Scale engine object
 */
void QwtAbstractScale::setScaleEngine(QwtScaleEngine* scaleEngine)
{
    QWT_D(d);
    if (scaleEngine != nullptr && scaleEngine != d->scaleEngine) {
        delete d->scaleEngine;
        d->scaleEngine = scaleEngine;
    }
}

/**
 * @brief Return the scale engine (const version)
 * @return Scale engine object
 * @sa setScaleEngine()
 */
const QwtScaleEngine* QwtAbstractScale::scaleEngine() const
{
    QWT_DC(d);
    return d->scaleEngine;
}

/**
 * @brief Return the scale engine (non-const version)
 * @return Scale engine object
 * @sa setScaleEngine()
 */
QwtScaleEngine* QwtAbstractScale::scaleEngine()
{
    QWT_D(d);
    return d->scaleEngine;
}

/**
 * @brief Return the scale division
 * @return Scale boundaries and positions of the ticks
 * @details The scale division might have been assigned explicitly
 *          or calculated implicitly by rescale().
 */
const QwtScaleDiv& QwtAbstractScale::scaleDiv() const
{
    QWT_DC(d);
    return d->scaleDraw->scaleDiv();
}

/**
 * @brief Return the scale map
 * @return Map to translate between scale and widget coordinates
 */
const QwtScaleMap& QwtAbstractScale::scaleMap() const
{
    QWT_DC(d);
    return d->scaleDraw->scaleMap();
}

/**
 * @brief Transform a scale value to widget coordinates
 * @param value Scale value
 * @return Corresponding widget coordinate for value
 * @sa scaleMap(), invTransform()
 */
int QwtAbstractScale::transform(double value) const
{
    QWT_DC(d);
    return qRound(d->scaleDraw->scaleMap().transform(value));
}

/**
 * @brief Transform a widget coordinate to scale value
 * @param value Widget coordinate
 * @return Corresponding scale coordinate for value
 * @sa scaleMap(), transform()
 */
double QwtAbstractScale::invTransform(int value) const
{
    QWT_DC(d);
    return d->scaleDraw->scaleMap().invTransform(value);
}

/**
 * @brief Check if scale is inverted
 * @return True if scale is increasing in opposite direction to widget coordinates
 */
bool QwtAbstractScale::isInverted() const
{
    QWT_DC(d);
    return d->scaleDraw->scaleMap().isInverting();
}

/**
 * @brief Return the minimum boundary
 * @return The boundary with the smaller value
 * @sa maximum(), lowerBound(), upperBound()
 */
double QwtAbstractScale::minimum() const
{
    QWT_DC(d);
    return qMin(d->scaleDraw->scaleDiv().lowerBound(), d->scaleDraw->scaleDiv().upperBound());
}

/**
 * @brief Return the maximum boundary
 * @return The boundary with the larger value
 * @sa minimum(), lowerBound(), upperBound()
 */
double QwtAbstractScale::maximum() const
{
    QWT_DC(d);
    return qMax(d->scaleDraw->scaleDiv().lowerBound(), d->scaleDraw->scaleDiv().upperBound());
}

/**
 * @brief Notify about scale changes
 * @details This virtual function is called when the scale changes.
 *          Override this function in derived classes to handle scale changes.
 */
void QwtAbstractScale::scaleChange()
{
}

/**
 * @brief Recalculate scale division and update scale
 * @param lowerBound Lower limit of the scale interval
 * @param upperBound Upper limit of the scale interval
 * @param stepSize Major step size
 * @sa scaleChange()
 */
void QwtAbstractScale::rescale(double lowerBound, double upperBound, double stepSize)
{
    QWT_D(d);
    const QwtScaleDiv scaleDiv = d->scaleEngine->divideScale(lowerBound, upperBound, d->maxMajor, d->maxMinor, stepSize);

    if (scaleDiv != d->scaleDraw->scaleDiv()) {
#if 1
        d->scaleDraw->setTransformation(d->scaleEngine->transformation());
#endif

        d->scaleDraw->setScaleDiv(scaleDiv);
        scaleChange();
    }
}

/**
 * @brief Handle change events
 * @param event Change event
 * @details Invalidates internal caches if necessary (e.g., on locale change).
 */
void QwtAbstractScale::changeEvent(QEvent* event)
{
    QWT_D(d);
    if (event->type() == QEvent::LocaleChange) {
        d->scaleDraw->invalidateCache();
    }

    QWidget::changeEvent(event);
}

/**
 * @brief Recalculate ticks and scale boundaries
 * @details Updates the scale draw by recalculating ticks and boundaries
 *          based on the current scale division.
 */
void QwtAbstractScale::updateScaleDraw()
{
    QWT_D(d);
    rescale(d->scaleDraw->scaleDiv().lowerBound(), d->scaleDraw->scaleDiv().upperBound(), d->stepSize);
}
