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

#ifndef QWT_BEZIER_H
#define QWT_BEZIER_H

#include "qwt_global.h"

class QPointF;
class QPolygonF;

/**
 * \if ENGLISH
 * @brief An implementation of the de Casteljau's Algorithm for interpolating Bézier curves
 * @details The flatness criterion for terminating the subdivision is based on
 *          "Piecewise Linear Approximation of Bézier Curves" by Roger Willcocks.
 *          See: https://jeremykun.com/2013/05/11/bezier-curves-and-picasso
 * \endif
 * \if CHINESE
 * @brief de Casteljau 算法的 Bézier 曲线插值实现
 * @details 细分终止的平坦度判据基于 Roger Willcocks 的
 *          "Piecewise Linear Approximation of Bézier Curves"。
 *          参见：https://jeremykun.com/2013/05/11/bezier-curves-and-picasso
 * \endif
 */
class QWT_EXPORT QwtBezier
{
public:
    QwtBezier(double tolerance = 0.5);
    ~QwtBezier();

    void setTolerance(double tolerance);
    double tolerance() const;

    QPolygonF toPolygon(const QPointF& p1, const QPointF& cp1, const QPointF& cp2, const QPointF& p2) const;

    void appendToPolygon(const QPointF& p1, const QPointF& cp1, const QPointF& cp2, const QPointF& p2, QPolygonF& polygon) const;

    static QPointF pointAt(const QPointF& p1, const QPointF& cp1, const QPointF& cp2, const QPointF& p2, double t);

private:
    double m_tolerance;
    double m_flatness;
};

/**
 * \if ENGLISH
 * @return Tolerance used as criterion for the subdivision
 * \sa setTolerance()
 * \endif
 * \if CHINESE
 * @return 用作细分判据的容差值
 * \sa setTolerance()
 * \endif
 */
inline double QwtBezier::tolerance() const
{
    return m_tolerance;
}

#endif
