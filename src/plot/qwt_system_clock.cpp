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

#include "qwt_system_clock.h"
#include <qelapsedtimer.h>

/**
 * \if ENGLISH
 * @brief Check if the elapsed timer is valid
 * @return true if the timer is valid, false otherwise
 * \endif
 *
 * \if CHINESE
 * @brief 检查计时器是否有效
 * @return 如果计时器有效返回 true，否则返回 false
 * \endif
 */
bool QwtSystemClock::isNull() const
{
    return m_timer.isValid();
}

/**
 * \if ENGLISH
 * @brief Start the elapsed timer
 * \endif
 *
 * \if CHINESE
 * @brief 启动计时器
 * \endif
 */
void QwtSystemClock::start()
{
    m_timer.start();
}

/**
 * \if ENGLISH
 * @brief Restart the elapsed timer
 * @return Elapsed time in multiples of milliseconds
 * \endif
 *
 * \if CHINESE
 * @brief 重启计时器
 * @return 经过的时间，以毫秒为单位
 * \endif
 */
double QwtSystemClock::restart()
{
    const qint64 nsecs = m_timer.restart();
    return nsecs / 1e6;
}

/**
 * \if ENGLISH
 * @brief Get elapsed time in multiples of milliseconds
 * @return Elapsed time since the timer was started
 * \endif
 *
 * \if CHINESE
 * @brief 获取经过的时间，以毫秒为单位
 * @return 自计时器启动以来经过的时间
 * \endif
 */
double QwtSystemClock::elapsed() const
{
    const qint64 nsecs = m_timer.nsecsElapsed();
    return nsecs / 1e6;
}
