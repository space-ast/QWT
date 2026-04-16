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

#ifndef QWT_SPLINE_POLYNOMIAL_H
#define QWT_SPLINE_POLYNOMIAL_H

#include "qwt_global.h"

#include <qpoint.h>
#include <qmetatype.h>

/**
 * \if ENGLISH
 * @brief A cubic polynomial without constant term
 *
 * QwtSplinePolynomial is a 3rd degree polynomial
 * of the form: y = c3 * x³ + c2 * x² + c1 * x;
 *
 * QwtSplinePolynomial is usually used in combination with polygon
 * interpolation, where it is not necessary to store a constant term ( c0 ),
 * as the translation is known from the corresponding polygon points.
 *
 * @sa QwtSplineC1
 * \endif
 *
 * \if CHINESE
 * @brief 无常数项的三次多项式
 *
 * QwtSplinePolynomial 是一个三次多项式，
 * 形式为：y = c3 * x³ + c2 * x² + c1 * x;
 *
 * QwtSplinePolynomial 通常与多边形插值结合使用，
 * 由于平移可以从对应的多边形点得知，因此不需要存储常数项（c0）。
 *
 * @sa QwtSplineC1
 * \endif
 */
class QWT_EXPORT QwtSplinePolynomial
{
  public:
    QwtSplinePolynomial( double c3 = 0.0, double c2 = 0.0, double c1 = 0.0 );

    bool operator==( const QwtSplinePolynomial& ) const;
    bool operator!=( const QwtSplinePolynomial& ) const;

    double valueAt( double x ) const;
    double slopeAt( double x ) const;
    double curvatureAt( double x ) const;

    static QwtSplinePolynomial fromSlopes(
        const QPointF& p1, double m1,
        const QPointF& p2, double m2 );

    static QwtSplinePolynomial fromSlopes(
        double x, double y, double m1, double m2 );

    static QwtSplinePolynomial fromCurvatures(
        const QPointF& p1, double cv1,
        const QPointF& p2, double cv2 );

    static QwtSplinePolynomial fromCurvatures(
        double dx, double dy, double cv1, double cv2 );

  public:
    //! coefficient of the cubic summand
    double c3;

    //! coefficient of the quadratic summand
    double c2;

    //! coefficient of the linear summand
    double c1;
};

Q_DECLARE_TYPEINFO( QwtSplinePolynomial, Q_MOVABLE_TYPE );
Q_DECLARE_METATYPE( QwtSplinePolynomial )

/**
 * \if ENGLISH
 * @brief Constructor
 *
 * @param[in] a3 Coefficient of the cubic summand
 * @param[in] a2 Coefficient of the quadratic summand
 * @param[in] a1 Coefficient of the linear summand
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 *
 * @param[in] a3 三次项系数
 * @param[in] a2 二次项系数
 * @param[in] a1 线性项系数
 * \endif
 */
inline QwtSplinePolynomial::QwtSplinePolynomial( double a3, double a2, double a1 )
    : c3( a3 )
    , c2( a2 )
    , c1( a1 )
{
}

/**
 * \if ENGLISH
 * @brief Compare two polynomials for equality
 *
 * @param[in] other Other polynomial
 * @return true, when both polynomials have the same coefficients
 * \endif
 *
 * \if CHINESE
 * @brief 比较两个多项式是否相等
 *
 * @param[in] other 另一个多项式
 * @return 当两个多项式具有相同系数时返回 true
 * \endif
 */
inline bool QwtSplinePolynomial::operator==( const QwtSplinePolynomial& other ) const
{
    return ( c3 == other.c3 ) && ( c2 == other.c2 ) && ( c1 == other.c1 );
}

/**
 * \if ENGLISH
 * @brief Compare two polynomials for inequality
 *
 * @param[in] other Other polynomial
 * @return true, when the polynomials have different coefficients
 * \endif
 *
 * \if CHINESE
 * @brief 比较两个多项式是否不相等
 *
 * @param[in] other 另一个多项式
 * @return 当多项式具有不同系数时返回 true
 * \endif
 */
inline bool QwtSplinePolynomial::operator!=( const QwtSplinePolynomial& other ) const
{
    return ( !( *this == other ) );
}

/**
 * \if ENGLISH
 * @brief Calculate the value of a polynomial for a given x
 *
 * @param[in] x Parameter
 * @return Value at x
 * \endif
 *
 * \if CHINESE
 * @brief 计算给定 x 处的多项式值
 *
 * @param[in] x 参数
 * @return x 处的值
 * \endif
 */
inline double QwtSplinePolynomial::valueAt( double x ) const
{
    return ( ( ( c3 * x ) + c2 ) * x + c1 ) * x;
}

/**
 * \if ENGLISH
 * @brief Calculate the value of the first derivate of a polynomial for a given x
 *
 * @param[in] x Parameter
 * @return Slope at x
 * \endif
 *
 * \if CHINESE
 * @brief 计算给定 x 处多项式的一阶导数值
 *
 * @param[in] x 参数
 * @return x 处的斜率
 * \endif
 */
inline double QwtSplinePolynomial::slopeAt( double x ) const
{
    return ( 3.0 * c3 * x + 2.0 * c2 ) * x + c1;
}

/**
 * \if ENGLISH
 * @brief Calculate the value of the second derivate of a polynomial for a given x
 *
 * @param[in] x Parameter
 * @return Curvature at x
 * \endif
 *
 * \if CHINESE
 * @brief 计算给定 x 处多项式的二阶导数值
 *
 * @param[in] x 参数
 * @return x 处的曲率
 * \endif
 */
inline double QwtSplinePolynomial::curvatureAt( double x ) const
{
    return 6.0 * c3 * x + 2.0 * c2;
}

/**
 * \if ENGLISH
 * @brief Find the coefficients for the polynomial including 2 points with
 *        specific values for the 1st derivates at these points.
 *
 * @param[in] p1 First point
 * @param[in] m1 Value of the first derivate at p1
 * @param[in] p2 Second point
 * @param[in] m2 Value of the first derivate at p2
 *
 * @return Coefficients of the polynomials
 * @note The missing constant term of the polynomial is p1.y()
 * \endif
 *
 * \if CHINESE
 * @brief 找出包含两个点及其一阶导数值的多项式系数
 *
 * @param[in] p1 第一个点
 * @param[in] m1 第一个点处的一阶导数值
 * @param[in] p2 第二个点
 * @param[in] m2 第二个点处的一阶导数值
 *
 * @return 多项式系数
 * @note 多项式缺失的常数项为 p1.y()
 * \endif
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromSlopes(
    const QPointF& p1, double m1, const QPointF& p2, double m2 )
{
    return fromSlopes( p2.x() - p1.x(), p2.y() - p1.y(), m1, m2 );
}

/**
 * \if ENGLISH
 * @brief Find the coefficients for the polynomial from the offset between 2 points
 *        and specific values for the 1st derivates at these points.
 *
 * @param[in] dx X-offset
 * @param[in] dy Y-offset
 * @param[in] m1 Value of the first derivate at p1
 * @param[in] m2 Value of the first derivate at p2
 *
 * @return Coefficients of the polynomials
 * \endif
 *
 * \if CHINESE
 * @brief 从两点之间的偏移量及其一阶导数值找出多项式系数
 *
 * @param[in] dx X 偏移量
 * @param[in] dy Y 偏移量
 * @param[in] m1 第一个点处的一阶导数值
 * @param[in] m2 第二个点处的一阶导数值
 *
 * @return 多项式系数
 * \endif
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromSlopes(
    double dx, double dy, double m1, double m2 )
{
    const double c2 = ( 3.0 * dy / dx - 2 * m1 - m2 ) / dx;
    const double c3 = ( ( m2 - m1 ) / dx - 2.0 * c2 ) / ( 3.0 * dx );

    return QwtSplinePolynomial( c3, c2, m1 );
}

/**
 * \if ENGLISH
 * @brief Find the coefficients for the polynomial including 2 points with
 *        specific values for the 2nd derivates at these points.
 *
 * @param[in] p1 First point
 * @param[in] cv1 Value of the second derivate at p1
 * @param[in] p2 Second point
 * @param[in] cv2 Value of the second derivate at p2
 *
 * @return Coefficients of the polynomials
 * @note The missing constant term of the polynomial is p1.y()
 * \endif
 *
 * \if CHINESE
 * @brief 找出包含两个点及其二阶导数值的多项式系数
 *
 * @param[in] p1 第一个点
 * @param[in] cv1 第一个点处的二阶导数值
 * @param[in] p2 第二个点
 * @param[in] cv2 第二个点处的二阶导数值
 *
 * @return 多项式系数
 * @note 多项式缺失的常数项为 p1.y()
 * \endif
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromCurvatures(
    const QPointF& p1, double cv1, const QPointF& p2, double cv2 )
{
    return fromCurvatures( p2.x() - p1.x(), p2.y() - p1.y(), cv1, cv2 );
}

/**
 * \if ENGLISH
 * @brief Find the coefficients for the polynomial from the offset between 2 points
 *        and specific values for the 2nd derivates at these points.
 *
 * @param[in] dx X-offset
 * @param[in] dy Y-offset
 * @param[in] cv1 Value of the second derivate at p1
 * @param[in] cv2 Value of the second derivate at p2
 *
 * @return Coefficients of the polynomials
 * \endif
 *
 * \if CHINESE
 * @brief 从两点之间的偏移量及其二阶导数值找出多项式系数
 *
 * @param[in] dx X 偏移量
 * @param[in] dy Y 偏移量
 * @param[in] cv1 第一个点处的二阶导数值
 * @param[in] cv2 第二个点处的二阶导数值
 *
 * @return 多项式系数
 * \endif
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromCurvatures(
    double dx, double dy, double cv1, double cv2 )
{
    const double c3 = ( cv2 - cv1 ) / ( 6.0 * dx );
    const double c2 = 0.5 * cv1;
    const double c1 = dy / dx - ( c3 * dx + c2 ) * dx;

    return QwtSplinePolynomial( c3, c2, c1 );
}

#ifndef QT_NO_DEBUG_STREAM

class QDebug;
QWT_EXPORT QDebug operator<<( QDebug, const QwtSplinePolynomial& );

#endif

#endif
