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

#include "qwt_sampling_thread.h"
#include <qelapsedtimer.h>

class QwtSamplingThread::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtSamplingThread)
public:
    PrivateData(QwtSamplingThread* p) : q_ptr(p), msecsInterval(1e3)
    {
    }

    QElapsedTimer timer;
    double msecsInterval;
};

/**
 * @brief Constructor
 * @param parent Parent object
 *
 */
QwtSamplingThread::QwtSamplingThread(QObject* parent) : QThread(parent), QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Destructor
 *
 */
QwtSamplingThread::~QwtSamplingThread()
{
}

/**
 * @brief Change the interval (in ms), when sample() is called.
 *
 * The default interval is 1000.0 ( = 1s )
 *
 * @param msecs Interval
 * @sa interval()
 *
 */
void QwtSamplingThread::setInterval(double msecs)
{
    QWT_D(d);
    if (msecs < 0.0)
        msecs = 0.0;

    d->msecsInterval = msecs;
}

/**
 * @brief Get the interval (in ms), between 2 calls of sample()
 * @return Interval in milliseconds
 * @sa setInterval()
 *
 */
double QwtSamplingThread::interval() const
{
    QWT_DC(d);
    return d->msecsInterval;
}

/**
 * @brief Get the time (in ms) since the thread was started
 * @return Elapsed time in milliseconds
 * @sa QThread::start(), run()
 *
 */
double QwtSamplingThread::elapsed() const
{
    QWT_DC(d);
    if (d->timer.isValid())
        return d->timer.nsecsElapsed() / 1e6;

    return 0.0;
}

/**
 * @brief Terminate the collecting thread
 * @sa QThread::start(), run()
 *
 */
void QwtSamplingThread::stop()
{
    QWT_D(d);
    d->timer.invalidate();
}

/**
 * @brief Loop collecting samples started from QThread::start()
 * @sa stop()
 *
 */
void QwtSamplingThread::run()
{
    QWT_D(d);
    d->timer.start();

    /*
        We should have all values in nsecs/qint64, but
        this would break existing code. TODO ...
        Anyway - for QThread::usleep we even need microseconds( usecs )
     */
    while (d->timer.isValid()) {
        const qint64 timestamp = d->timer.nsecsElapsed();
        sample(timestamp / 1e9);  // seconds

        if (d->msecsInterval > 0.0) {
            const double interval = d->msecsInterval * 1e3;
            const double elapsed  = (d->timer.nsecsElapsed() - timestamp) / 1e3;

            const double usecs = interval - elapsed;

            if (usecs > 0.0)
                QThread::usleep(qRound(usecs));
        }
    }
}
