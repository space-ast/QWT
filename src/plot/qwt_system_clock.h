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

#ifndef QWT_SYSTEM_CLOCK_H
#define QWT_SYSTEM_CLOCK_H

#include "qwt_global.h"
#include <qelapsedtimer.h>

/**
 * \if ENGLISH
 * @brief QwtSystemClock provides high resolution clock time functions.
 *
 * Precision and time intervals are multiples of milliseconds (ms).
 *
 * ( QwtSystemClock is deprecated as QElapsedTimer offers the same precision )
 * \endif
 *
 * \if CHINESE
 * @brief QwtSystemClock 提供高分辨率时钟时间函数。
 *
 * 精度和时间间隔以毫秒 (ms) 为单位。
 *
 * ( QwtSystemClock 已弃用，因为 QElapsedTimer 提供相同的精度 )
 * \endif
 */

class QWT_EXPORT QwtSystemClock
{
  public:
    //! Check if the clock has been started
    bool isNull() const;

    //! Start or restart the clock
    void start();
    //! Restart the clock and return the elapsed time
    double restart();
    //! Return the elapsed time since the clock was started
    double elapsed() const;

  private:
    QElapsedTimer m_timer;
};

#endif
