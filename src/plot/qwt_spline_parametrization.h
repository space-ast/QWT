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

#ifndef QWT_SPLINE_PARAMETRIZATION_H
#define QWT_SPLINE_PARAMETRIZATION_H

#include "qwt_global.h"
#include "qwt_math.h"

#include <qpoint.h>

/**
 * \if ENGLISH
 * @brief Curve parametrization used for a spline interpolation
 *
 * Parametrization is the process of finding a parameter value for
 * each curve point - usually related to some physical quantity
 * ( distance, time ... ).
 *
 * Often accumulating the curve length is the intended way of parametrization,
 * but as the interpolated curve is not known in advance an approximation
 * needs to be used.
 *
 * The values are calculated by cumulating increments, that are provided
 * by QwtSplineParametrization. As the curve parameters need to be
 * montonically increasing, each increment need to be positive.
 *
 * - t[0] = 0;
 * - t[i] = t[i-1] + valueIncrement( point[i-1], p[i] );
 *
 * QwtSplineParametrization provides the most common used type of
 * parametrizations and offers an interface to inject custom implementations.
 *
 * @note The most relevant types of parametrization are trying to provide an
 *       approximation of the curve length.
 *
 * @sa QwtSpline::setParametrization()
 * \endif
 *
 * \if CHINESE
 * @brief 用于样条插值的曲线参数化
 *
 * 参数化是为每个曲线点找到参数值的过程——通常与某些物理量相关
 * （距离、时间等）。
 *
 * 通常累积曲线长度是参数化的预期方式，但由于插值曲线预先未知，
 * 需要使用近似方法。
 *
 * 值通过累积增量来计算，增量由 QwtSplineParametrization 提供。
 * 由于曲线参数需要单调递增，每个增量都需要为正值。
 *
 * - t[0] = 0;
 * - t[i] = t[i-1] + valueIncrement( point[i-1], p[i] );
 *
 * QwtSplineParametrization 提供了最常用的参数化类型，
 * 并提供了注入自定义实现的接口。
 *
 * @note 最相关的参数化类型是尝试提供曲线长度的近似。
 *
 * @sa QwtSpline::setParametrization()
 * \endif
 */
class QWT_EXPORT QwtSplineParametrization
{
  public:
    //! Parametrization type
    enum Type
    {
        /*!
           No parametrization: t[i] = x[i]
           \sa valueIncrementX()
         */
        ParameterX,

        /*!
           No parametrization: t[i] = y[i]
           \sa valueIncrementY()
         */
        ParameterY,

        /*!
           Uniform parametrization: t[i] = i;

           A very fast parametrization, with good results, when the geometry
           of the control points is somehow "equidistant". F.e. when
           recording the position of a body, that is moving with constant
           speed every n seconds.

           \sa valueIncrementUniform()
         */
        ParameterUniform,

        /*!
           Parametrization using the chordal length between two control points

           The chordal length is the most commonly used approximation for
           the curve length.

           \sa valueIncrementChordal()
         */
        ParameterChordal,

        /*!
           Centripetal parametrization

           Based on the square root of the chordal length.

           Its name stems from the physical observations regarding
           the centripetal force, of a body moving along the curve.

           \sa valueIncrementCentripetal()
         */
        ParameterCentripetal,


        /*!
           Parametrization using the manhattan length between two control points

           Approximating the curve length by the manhattan length is faster
           than the chordal length, but usually gives worse results.

           \sa valueIncrementManhattan()
         */
        ParameterManhattan
    };

    // Constructor with parametrization type
    explicit QwtSplineParametrization( int type );
    // Destructor
    virtual ~QwtSplineParametrization();

    // Get parametrization type
    int type() const;

    // Calculate parameter value increment for 2 points
    virtual double valueIncrement( const QPointF&, const QPointF& ) const;

    static double valueIncrementX( const QPointF&, const QPointF& );
    static double valueIncrementY( const QPointF&, const QPointF& );
    static double valueIncrementUniform( const QPointF&, const QPointF& );
    static double valueIncrementChordal( const QPointF&, const QPointF& );
    static double valueIncrementCentripetal( const QPointF&, const QPointF& );
    static double valueIncrementManhattan( const QPointF&, const QPointF& );

  private:
    const int m_type;
};

/**
 * \if ENGLISH
 * @brief Calculate the ParameterX value increment for 2 points
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return point2.x() - point1.x()
 * \endif
 *
 * \if CHINESE
 * @brief 计算两个点的 ParameterX 值增量
 *
 * @param[in] point1 第一个点
 * @param[in] point2 第二个点
 *
 * @return point2.x() - point1.x()
 * \endif
 */
inline double QwtSplineParametrization::valueIncrementX(
    const QPointF& point1, const QPointF& point2 )
{
    return point2.x() - point1.x();
}

/**
 * \if ENGLISH
 * @brief Calculate the ParameterY value increment for 2 points
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return point2.y() - point1.y()
 * \endif
 *
 * \if CHINESE
 * @brief 计算两个点的 ParameterY 值增量
 *
 * @param[in] point1 第一个点
 * @param[in] point2 第二个点
 *
 * @return point2.y() - point1.y()
 * \endif
 */
inline double QwtSplineParametrization::valueIncrementY(
    const QPointF& point1, const QPointF& point2 )
{
    return point2.y() - point1.y();
}

/**
 * \if ENGLISH
 * @brief Calculate the ParameterUniform value increment
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return 1.0
 * \endif
 *
 * \if CHINESE
 * @brief 计算 ParameterUniform 值增量
 *
 * @param[in] point1 第一个点
 * @param[in] point2 第二个点
 *
 * @return 1.0
 * \endif
 */
inline double QwtSplineParametrization::valueIncrementUniform(
    const QPointF& point1, const QPointF& point2 )
{
    Q_UNUSED( point1 )
    Q_UNUSED( point2 )

    return 1.0;
}

/**
 * \if ENGLISH
 * @brief Calculate the ParameterChordal value increment for 2 points
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return qSqrt( dx * dx + dy * dy )
 * \endif
 *
 * \if CHINESE
 * @brief 计算两个点的 ParameterChordal 值增量
 *
 * @param[in] point1 第一个点
 * @param[in] point2 第二个点
 *
 * @return qSqrt( dx * dx + dy * dy )
 * \endif
 */
inline double QwtSplineParametrization::valueIncrementChordal(
    const QPointF& point1, const QPointF& point2 )
{
    const double dx = point2.x() - point1.x();
    const double dy = point2.y() - point1.y();

    return std::sqrt( dx * dx + dy * dy );
}

/**
 * \if ENGLISH
 * @brief Calculate the ParameterCentripetal value increment for 2 points
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return The square root of a chordal increment
 * \endif
 *
 * \if CHINESE
 * @brief 计算两个点的 ParameterCentripetal 值增量
 *
 * @param[in] point1 第一个点
 * @param[in] point2 第二个点
 *
 * @return 弦长增量的平方根
 * \endif
 */
inline double QwtSplineParametrization::valueIncrementCentripetal(
    const QPointF& point1, const QPointF& point2 )
{
    return std::sqrt( valueIncrementChordal( point1, point2 ) );
}

/**
 * \if ENGLISH
 * @brief Calculate the ParameterManhattan value increment for 2 points
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return | point2.x() - point1.x() | + | point2.y() - point1.y() |
 * \endif
 *
 * \if CHINESE
 * @brief 计算两个点的 ParameterManhattan 值增量
 *
 * @param[in] point1 第一个点
 * @param[in] point2 第二个点
 *
 * @return | point2.x() - point1.x() | + | point2.y() - point1.y() |
 * \endif
 */
inline double QwtSplineParametrization::valueIncrementManhattan(
    const QPointF& point1, const QPointF& point2 )
{
    return qAbs( point2.x() - point1.x() ) + qAbs( point2.y() - point1.y() );
}

#endif
