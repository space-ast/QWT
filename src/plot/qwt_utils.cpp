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

/**
 * @brief Expand a size to the global strut (Qt5/Qt6 compatibility function)
 *
 * In Qt 5.0-5.14, QApplication::globalStrut() returns the application-defined
 * minimum widget size. This function expands the given size to at least that
 * minimum size.
 *
 * In Qt 5.15+ and Qt 6, globalStrut() has been removed and this function
 * returns the original size unchanged.
 *
 * @param size Original size
 * @return Size expanded to the global strut, or the original size on Qt 5.15+/Qt 6
 *
 * @internal
 * @warning This function is for internal use by Qwt widget classes only
 *          (such as QwtDial, QwtKnob, QwtSlider, etc.). Application code
 *          should not call this function.
 *
 * @since Qwt 6.0
 */
QSize qwtExpandedToGlobalStrut(const QSize& size)
{
#ifdef QWT_GLOBAL_STRUT
    return size.expandedTo(QApplication::globalStrut());
#else
    return size;
#endif
}
