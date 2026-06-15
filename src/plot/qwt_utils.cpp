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

#include "qwt_utils.h"

#include <qapplication.h>

// Define QWT_GLOBAL_STRUT for Qt versions that support it
#define QWT_GLOBAL_STRUT

#if QT_VERSION >= 0x050000
#if QT_VERSION >= 0x060000 || !QT_DEPRECATED_SINCE(5, 15)
#undef QWT_GLOBAL_STRUT
#endif
#endif

/*!
 * \brief 将尺寸扩展到全局最小尺寸（Qt5/Qt6 兼容性函数）
 *
 * 在 Qt 5.0-5.14 中，QApplication::globalStrut() 返回应用程序定义的最小控件尺寸。
 * 此函数将给定的尺寸扩展到至少该最小尺寸。
 *
 * 在 Qt 5.15+ 和 Qt 6 中，globalStrut() 已被移除，此函数直接返回原始尺寸不变。
 *
 * \param size 原始尺寸
 * \return 扩展到全局最小尺寸后的尺寸，或在 Qt 5.15+/Qt 6 中返回原始尺寸
 *
 * \internal
 * \warning 此函数仅供 Qwt 内部控件类使用（如 QwtDial、QwtKnob、QwtSlider 等），
 *          应用程序代码不应调用此函数。
 *
 * \since Qwt 6.0
 */
QSize qwtExpandedToGlobalStrut(const QSize& size)
{
#ifdef QWT_GLOBAL_STRUT
    return size.expandedTo(QApplication::globalStrut());
#else
    return size;
#endif
}
