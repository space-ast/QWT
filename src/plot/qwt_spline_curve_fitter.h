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

#ifndef QWT_SPLINE_CURVE_FITTER_H
#define QWT_SPLINE_CURVE_FITTER_H

#include "qwt_curve_fitter.h"

class QwtSpline;

/**
 * \if ENGLISH
 * @brief A curve fitter using a spline interpolation
 *
 * The default setting for the spline is a cardinal spline with
 * uniform parametrization.
 *
 * @sa QwtSpline, QwtSplineLocal
 * \endif
 *
 * \if CHINESE
 * @brief 使用样条插值的曲线拟合器
 *
 * 样条的默认设置是具有均匀参数化的基数样条。
 *
 * @sa QwtSpline, QwtSplineLocal
 * \endif
 */
class QWT_EXPORT QwtSplineCurveFitter : public QwtCurveFitter
{
  public:
    /// \if ENGLISH Constructor \endif \if CHINESE 构造函数 \endif
    QwtSplineCurveFitter();
    /// \if ENGLISH Destructor \endif \if CHINESE 析构函数 \endif
    virtual ~QwtSplineCurveFitter();

    /// \if ENGLISH Set spline \endif \if CHINESE 设置样条 \endif
    void setSpline( QwtSpline* );

    /// \if ENGLISH Get spline (const version) \endif \if CHINESE 获取样条（const 版本） \endif
    const QwtSpline* spline() const;
    /// \if ENGLISH Get spline (non-const version) \endif \if CHINESE 获取样条（非 const 版本） \endif
    QwtSpline* spline();

    /// \if ENGLISH Fit curve to polygon \endif \if CHINESE 将曲线拟合到多边形 \endif
    virtual QPolygonF fitCurve( const QPolygonF& ) const override;
    /// \if ENGLISH Fit curve path to polygon \endif \if CHINESE 将曲线路径拟合到多边形 \endif
    virtual QPainterPath fitCurvePath( const QPolygonF& ) const override;

  private:
    QwtSpline* m_spline;
};

#endif
