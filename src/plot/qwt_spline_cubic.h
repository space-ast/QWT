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

#ifndef QWT_SPLINE_CUBIC_H
#define QWT_SPLINE_CUBIC_H

#include "qwt_global.h"
#include "qwt_spline.h"

/**
 * \if ENGLISH
 * @brief A cubic spline
 *
 * A cubic spline is a spline with C2 continuity at all control points.
 * It is a non local spline, where a modification of one control point
 * affects the whole spline.
 *
 * The implementation is based on the equation system of the
 * second derivatives M(x) of the control points P(x).
 *
 * QwtSplineCubic offers several algorithms for finding M(x), that
 * are selected by setBoundaryType().
 *
 * The default setting is a "natural spline" having M(x0) = M(xn) = 0.
 *
 * @sa QwtSpline::BoundaryType
 * \endif
 *
 * \if CHINESE
 * @brief 三次样条
 *
 * 三次样条是在所有控制点处具有 C2 连续性的样条。
 * 它是一种非局部样条，其中一个控制点的修改会影响整个样条。
 *
 * 该实现基于控制点 P(x) 的二阶导数 M(x) 的方程组。
 *
 * QwtSplineCubic 提供了几种查找 M(x) 的算法，
 * 这些算法通过 setBoundaryType() 选择。
 *
 * 默认设置是"自然样条"，具有 M(x0) = M(xn) = 0。
 *
 * @sa QwtSpline::BoundaryType
 * \endif
 */
class QWT_EXPORT QwtSplineCubic : public QwtSplineC2
{
  public:
    //! Constructor
    QwtSplineCubic();
    //! Destructor
    virtual ~QwtSplineCubic();

    //! Get locality (always 0 - non-local)
    virtual uint locality() const override;

    //! Get painter path from polygon
    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    //! Get Bezier control lines
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& points ) const override;

    //! Get polynomials from polygon
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const override;
    //! Get slopes at control points
    virtual QVector< double > slopes( const QPolygonF& ) const override;
    //! Get curvatures at control points
    virtual QVector< double > curvatures( const QPolygonF& ) const override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
