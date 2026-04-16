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

#ifndef QWT_SPLINE_BASIS_H
#define QWT_SPLINE_BASIS_H

#include "qwt_global.h"
#include "qwt_spline.h"

/**
 * \if ENGLISH
 * @brief An approximation using a basis spline
 *
 * QwtSplineBasis approximates a set of points by a polynomials with C2 continuity
 * ( = first and second derivatives are equal ) at the end points.
 *
 * The end points of the spline do not match the original points.
 * \endif
 *
 * \if CHINESE
 * @brief 使用基样条的逼近
 *
 * QwtSplineBasis 通过具有 C2 连续性的多项式（即端点处一阶和二阶导数相等）
 * 来逼近一组点。
 *
 * 样条的端点与原始点不匹配。
 * \endif
 */
class QWT_EXPORT QwtSplineBasis : public QwtSpline
{
  public:
    //! Constructor
    QwtSplineBasis();
    //! Destructor
    virtual ~QwtSplineBasis();

    //! Get painter path from polygon
    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    //! Get locality (always 2)
    virtual uint locality() const override;
};

#endif
