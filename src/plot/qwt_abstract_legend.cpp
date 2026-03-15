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

#include "qwt_abstract_legend.h"
#include "qwt_legend_data.h"

/**
 * \if ENGLISH
 * @brief Constructor for QwtAbstractLegend
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief QwtAbstractLegend 构造函数
 * @param parent 父控件
 * \endif
 */
QwtAbstractLegend::QwtAbstractLegend(QWidget* parent) : QFrame(parent)
{
}

/**
 * \if ENGLISH
 * @brief Destructor for QwtAbstractLegend
 * \endif
 * \if CHINESE
 * @brief QwtAbstractLegend 析构函数
 * \endif
 */
QwtAbstractLegend::~QwtAbstractLegend()
{
}

/**
 * \if ENGLISH
 * @brief Return the extent needed for scroll elements
 * @details Returns the space needed for elements to scroll the legend (usually scrollbars).
 * @param orientation Orientation (Horizontal or Vertical)
 * @return Extent of the corresponding scroll element (0 by default)
 * \endif
 * \if CHINESE
 * @brief 返回滚动元素所需的空间范围
 * @details 返回滚动图例所需元素（通常是滚动条）的空间。
 * @param orientation 方向（水平或垂直）
 * @return 对应滚动元素的范围（默认为 0）
 * \endif
 */
int QwtAbstractLegend::scrollExtent(Qt::Orientation orientation) const
{
    Q_UNUSED(orientation);
    return 0;
}
