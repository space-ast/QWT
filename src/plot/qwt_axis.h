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

#ifndef QWT_AXIS_H
#define QWT_AXIS_H

#include "qwt_global.h"

/**
 * \if ENGLISH
 * @brief Enums and methods for axes
 * \endif
 *
 * \if CHINESE
 * @brief 坐标轴的枚举和方法
 * \endif
 */
namespace QwtAxis
{
/**
 * \if ENGLISH
 * @brief Axis position
 * \endif
 *
 * \if CHINESE
 * @brief 坐标轴位置
 * \endif
 */
enum Position
{
    //! Y axis left of the canvas
    YLeft = 0,

    //! Y axis right of the canvas
    YRight = 1,

    //! X axis below the canvas
    XBottom = 2,

    //! X axis above the canvas
    XTop = 3
};

/**
 * \if ENGLISH
 * @brief Number of axis positions
 * \endif
 *
 * \if CHINESE
 * @brief 坐标轴位置数量
 * \endif
 */
enum
{
    AxisPositions = XTop + 1
};

bool isValid(int axisPos);
bool isYAxis(int axisPos);
bool isXAxis(int axisPos);
}

//! Return true, when axisPos is in the valid range [ YLeft, XTop ]
inline bool QwtAxis::isValid(int axisPos)
{
    return (axisPos >= 0 && axisPos < AxisPositions);
}

//! Return true, when axisPos is XBottom or XTop
inline bool QwtAxis::isXAxis(int axisPos)
{
    return (axisPos == XBottom) || (axisPos == XTop);
}

//! Return true, when axisPos is YLeft or YRight
inline bool QwtAxis::isYAxis(int axisPos)
{
    return (axisPos == YLeft) || (axisPos == YRight);
}

#endif
