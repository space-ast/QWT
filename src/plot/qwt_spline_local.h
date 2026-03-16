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

#ifndef QWT_SPLINE_LOCAL_H
#define QWT_SPLINE_LOCAL_H

#include "qwt_global.h"
#include "qwt_spline.h"

/**
 * \if ENGLISH
 * @brief A spline with C1 continuity
 *
 * QwtSplineLocal offers several standard algorithms for interpolating
 * a curve with polynomials having C1 continuity at the control points.
 * All algorithms are local in a sense, that changing one control point
 * only few polynomials.
 * \endif
 *
 * \if CHINESE
 * @brief 具有 C1 连续性的样条
 *
 * QwtSplineLocal 提供了几种标准算法，用于在控制点处用具有 C1 连续性的
 * 多项式插值曲线。所有算法在某种意义上都是局部的，即改变一个控制点
 * 只会影响少数多项式。
 * \endif
 */
class QWT_EXPORT QwtSplineLocal : public QwtSplineC1
{
  public:
    /**
     * \if ENGLISH
     * @brief Spline interpolation type
     *
     * All type of spline interpolations are lightweight algorithms
     * calculating the slopes at a point by looking 1 or 2 points back
     * and ahead.
     * \endif
     *
     * \if CHINESE
     * @brief 样条插值类型
     *
     * 所有类型的样条插值都是轻量级算法，通过查看前后 1 或 2 个点
     * 来计算某点的斜率。
     * \endif
     */
    enum Type
    {
        /**
         * \if ENGLISH
         * A cardinal spline
         *
         * The cardinal spline interpolation is a very cheap calculation with
         * a locality of 1.
         * \endif
         * \if CHINESE
         * 基数样条
         *
         * 基数样条插值是一种非常便宜的计算，局部性为 1。
         * \endif
         */
        Cardinal,

        /**
         * \if ENGLISH
         * Parabolic blending is a cheap calculation with a locality of 1. Sometimes
         * it is also called Cubic Bessel interpolation.
         * \endif
         * \if CHINESE
         * 抛物线混合是一种局部性为 1 的便宜计算。有时也称为三次贝塞尔插值。
         * \endif
         */
        ParabolicBlending,

        /**
         * \if ENGLISH
         * The algorithm of H.Akima is a calculation with a locality of 2.
         * \endif
         * \if CHINESE
         * H.Akima 算法是一种局部性为 2 的计算。
         * \endif
         */
        Akima,

        /**
         * \if ENGLISH
         * Piecewise Cubic Hermite Interpolating Polynomial (PCHIP) is an algorithm
         * that is popular because of being offered by MATLAB.
         *
         * It preserves the shape of the data and respects monotonicity. It has a
         * locality of 1.
         * \endif
         * \if CHINESE
         * 分段三次 Hermite 插值多项式 (PCHIP) 是一种因 MATLAB 提供而流行的算法。
         *
         * 它保持数据的形状并尊重单调性。它的局部性为 1。
         * \endif
         */
        PChip
    };

    /// \if ENGLISH Constructor with type \endif \if CHINESE 带类型的构造函数 \endif
    QwtSplineLocal( Type type );
    /// \if ENGLISH Destructor \endif \if CHINESE 析构函数 \endif
    virtual ~QwtSplineLocal();

    /// \if ENGLISH Get type \endif \if CHINESE 获取类型 \endif
    Type type() const;

    /// \if ENGLISH Get locality (number of points used for calculation) \endif \if CHINESE 获取局部性（用于计算的点数） \endif
    virtual uint locality() const override;

    /// \if ENGLISH Get painter path from polygon \endif \if CHINESE 从多边形获取绘制路径 \endif
    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    /// \if ENGLISH Get Bezier control lines \endif \if CHINESE 获取贝塞尔控制线 \endif
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const override;

    // calculating the parametric equations
    /// \if ENGLISH Get polynomials from polygon \endif \if CHINESE 从多边形获取多项式 \endif
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const override;
    /// \if ENGLISH Get slopes \endif \if CHINESE 获取斜率 \endif
    virtual QVector< double > slopes( const QPolygonF& ) const override;

  private:
    const Type m_type;
};

#endif
