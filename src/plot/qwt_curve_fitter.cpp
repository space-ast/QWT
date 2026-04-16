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

#include "qwt_curve_fitter.h"

/**
 * \if ENGLISH
 * @brief Constructor
 * @param mode Preferred fitting mode
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param mode 首选拟合模式
 * \endif
 */
QwtCurveFitter::QwtCurveFitter( Mode mode )
    : m_mode( mode )
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtCurveFitter::~QwtCurveFitter()
{
}

/**
 * \if ENGLISH
 * @brief Get the preferred fitting mode
 * @return Preferred fitting mode
 * \endif
 * \if CHINESE
 * @brief 获取首选拟合模式
 * @return 首选拟合模式
 * \endif
 */
QwtCurveFitter::Mode QwtCurveFitter::mode() const
{
    return m_mode;
}
