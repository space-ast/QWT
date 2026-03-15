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

#ifndef QWT_AXIS_ID_H
#define QWT_AXIS_ID_H

#include "qwt_global.h"
#include "qwt_axis.h"

/**
 * \if ENGLISH
 * @brief Axis identifier
 *
 * An axis id is one of values of QwtAxis::Position.
 *
 * QwtAxisId is a placeholder for future releases ( -> multiaxes branch ),
 * where it is possible to have more than one axis at each side of a plot.
 *
 * @sa QwtAxis
 * \endif
 *
 * \if CHINESE
 * @brief 坐标轴标识符
 *
 * 坐标轴ID是QwtAxis::Position的值之一。
 *
 * QwtAxisId是未来版本（-> multiaxes分支）的占位符，
 * 其中可以在绘图的每一侧有多个坐标轴。
 *
 * @sa QwtAxis
 * \endif
 */
using QwtAxisId = int;

#endif
