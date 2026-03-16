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

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H

#include "qwt_global.h"
#include "qwt_spline.h"

class QwtSplineParametrization;
class QwtSplinePolynomial;
class QPainterPath;
class QLineF;
class QPolygonF;

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/**
 * \if ENGLISH
 * @brief Base class for all splines
 *
 * A spline is a curve represented by a sequence of polynomials. Spline approximation
 * is the process of finding polynomials for a given set of points.
 * When the algorithm preserves the initial points it is called interpolating.
 *
 * Splines can be classified according to conditions of the polynomials that
 * are met at the start/endpoints of the pieces:
 *
 * - Geometric Continuity
 *   - G0: polynomials are joined
 *   - G1: first derivatives are proportional at the join point
 *         The curve tangents thus have the same direction, but not necessarily the
 *         same magnitude. i.e., C1'(1) = (a,b,c) and C2'(0) = (k*a, k*b, k*c).
 *   - G2: first and second derivatives are proportional at join point
 *
 * - Parametric Continuity
 *   - C0: curves are joined
 *   - C1: first derivatives equal
 *   - C2: first and second derivatives are equal
 *
 * Geometric continuity requires the geometry to be continuous, while parametric
 * continuity requires that the underlying parameterization be continuous as well.
 * Parametric continuity of order n implies geometric continuity of order n,
 * but not vice-versa.
 *
 * QwtSpline is the base class for spline approximations of any continuity.
 * \endif
 *
 * \if CHINESE
 * @brief 所有样条曲线的基类
 *
 * 样条曲线是由一系列多项式表示的曲线。样条逼近是为给定点集寻找多项式的过程。
 * 当算法保留初始点时，称为插值。
 *
 * 样条可以根据在片段起点/终点处满足的多项式条件进行分类：
 *
 * - 几何连续性
 *   - G0: 多项式连接
 *   - G1: 在连接点处一阶导数成比例
 *         因此曲线切线具有相同的方向，但不一定具有相同的大小。
 *         即 C1'(1) = (a,b,c) 和 C2'(0) = (k*a, k*b, k*c)。
 *   - G2: 在连接点处一阶和二阶导数成比例
 *
 * - 参数连续性
 *   - C0: 曲线连接
 *   - C1: 一阶导数相等
 *   - C2: 一阶和二阶导数相等
 *
 * 几何连续性要求几何形状连续，而参数连续性要求底层参数化也连续。
 * n 阶参数连续性意味着 n 阶几何连续性，但反之不成立。
 *
 * QwtSpline 是任意连续性样条逼近的基类。
 * \endif
 */
class QWT_EXPORT QwtSpline
{
  public:
    /**
     * \if ENGLISH
     * @brief Boundary type specifying the spline at its endpoints
     *
     * \sa setBoundaryType(), boundaryType()
     * \endif
     * \if CHINESE
     * @brief 指定样条端点处的边界类型
     *
     * \sa setBoundaryType(), boundaryType()
     * \endif
     */
    enum BoundaryType
    {
        /**
         * \if ENGLISH
         * The polynomials at the start/endpoint depend on specific conditions
         *
         * \sa QwtSpline::BoundaryCondition
         * \endif
         * \if CHINESE
         * 起点/终点处的多项式取决于特定条件
         *
         * \sa QwtSpline::BoundaryCondition
         * \endif
         */
        ConditionalBoundaries,

        /**
         * \if ENGLISH
         * The polynomials at the start/endpoint are found by using
         * imaginary additional points. Additional points at the end
         * are found by translating points from the beginning or v.v.
         * \endif
         * \if CHINESE
         * 起点/终点处的多项式通过使用虚拟附加点来确定。
         * 终点的附加点通过平移起始点的点或反之来找到。
         * \endif
         */
        PeriodicPolygon,

        /**
         * \if ENGLISH
         * ClosedPolygon is similar to PeriodicPolygon beside, that
         * the interpolation includes the connection between the last
         * and the first control point.
         *
         * \note Only works for parametrizations, where the parameter increment
         *      for the the final closing line is positive.
         *      This excludes QwtSplineParametrization::ParameterX and
         *      QwtSplineParametrization::ParameterY
         * \endif
         * \if CHINESE
         * ClosedPolygon 与 PeriodicPolygon 类似，但插值包含
         * 最后一个和第一个控制点之间的连接。
         *
         * \note 仅适用于参数增量为正的参数化，
         *      即最终闭合线的参数增量为正。
         *      这不包括 QwtSplineParametrization::ParameterX 和
         *      QwtSplineParametrization::ParameterY
         * \endif
         */

        ClosedPolygon
    };

    /**
     * \if ENGLISH
     * @brief position of a boundary condition
     * \sa boundaryCondition(), boundaryValue()
     * \endif
     * \if CHINESE
     * @brief 边界条件的位置
     * \sa boundaryCondition(), boundaryValue()
     * \endif
     */
    enum BoundaryPosition
    {
        //! \if ENGLISH the condition is at the beginning of the polynomial \endif \if CHINESE 条件在多项式的起始处 \endif
        AtBeginning,

        //! \if ENGLISH the condition is at the end of the polynomial \endif \if CHINESE 条件在多项式的结束处 \endif
        AtEnd
    };

    /**
     * \if ENGLISH
     * @brief Boundary condition
     *
     * A spline algorithm calculates polynomials by looking
     * a couple of points back/ahead ( locality() ). At the ends
     * additional rules are necessary to compensate the missing
     * points.
     *
     * \sa boundaryCondition(), boundaryValue()
     * \sa QwtSplineC2::BoundaryConditionC2
     * \endif
     * \if CHINESE
     * @brief 边界条件
     *
     * 样条算法通过查看前后几个点（locality()）来计算多项式。
     * 在端点处需要额外的规则来补偿缺失的点。
     *
     * \sa boundaryCondition(), boundaryValue()
     * \sa QwtSplineC2::BoundaryConditionC2
     * \endif
     */
    enum BoundaryCondition
    {
        /**
         * \if ENGLISH
         * The first derivative at the end point is given
         * \sa boundaryValue()
         * \endif
         * \if CHINESE
         * 给定终点处的一阶导数
         * \sa boundaryValue()
         * \endif
         */
        Clamped1,

        /**
         * \if ENGLISH
         * The second derivative at the end point is given
         *
         * \sa boundaryValue()
         * \note a condition having a second derivative of 0
         *      is also called "natural".
         * \endif
         * \if CHINESE
         * 给定终点处的二阶导数
         *
         * \sa boundaryValue()
         * \note 二阶导数为 0 的条件也称为"自然"条件。
         * \endif
         */
        Clamped2,

        /**
         * \if ENGLISH
         * The third derivative at the end point is given
         *
         * \sa boundaryValue()
         * \note a condition having a third derivative of 0
         *      is also called "parabolic runout".
         * \endif
         * \if CHINESE
         * 给定终点处的三阶导数
         *
         * \sa boundaryValue()
         * \note 三阶导数为 0 的条件也称为"抛物线延伸"。
         * \endif
         */
        Clamped3,

        /**
         * \if ENGLISH
         * The first derivate at the endpoint is related to the first derivative
         * at its neighbour by the boundary value. F,e when the boundary
         * value at the end is 1.0 then the slope at the last 2 points is
         * the same.
         *
         * \sa boundaryValue().
         * \endif
         * \if CHINESE
         * 终点处的一阶导数与其相邻点的一阶导数通过边界值相关联。
         * 例如，当终点的边界值为 1.0 时，最后两个点的斜率相同。
         *
         * \sa boundaryValue().
         * \endif
         */
        LinearRunout
    };

    /// \if ENGLISH Constructor \endif \if CHINESE 构造函数 \endif
    QwtSpline();
    /// \if ENGLISH Destructor \endif \if CHINESE 析构函数 \endif
    virtual ~QwtSpline();

    /// \if ENGLISH Set parametrization by type \endif \if CHINESE 按类型设置参数化 \endif
    void setParametrization( int type );
    /// \if ENGLISH Set parametrization object \endif \if CHINESE 设置参数化对象 \endif
    void setParametrization( QwtSplineParametrization* );
    /// \if ENGLISH Get parametrization \endif \if CHINESE 获取参数化 \endif
    const QwtSplineParametrization* parametrization() const;

    /// \if ENGLISH Set boundary type \endif \if CHINESE 设置边界类型 \endif
    void setBoundaryType( BoundaryType );
    /// \if ENGLISH Get boundary type \endif \if CHINESE 获取边界类型 \endif
    BoundaryType boundaryType() const;

    /// \if ENGLISH Set boundary value \endif \if CHINESE 设置边界值 \endif
    void setBoundaryValue( BoundaryPosition, double value );
    /// \if ENGLISH Get boundary value \endif \if CHINESE 获取边界值 \endif
    double boundaryValue( BoundaryPosition ) const;

    /// \if ENGLISH Set boundary condition \endif \if CHINESE 设置边界条件 \endif
    void setBoundaryCondition( BoundaryPosition, int condition );
    /// \if ENGLISH Get boundary condition \endif \if CHINESE 获取边界条件 \endif
    int boundaryCondition( BoundaryPosition ) const;

    /// \if ENGLISH Set boundary conditions for both ends \endif \if CHINESE 为两端设置边界条件 \endif
    void setBoundaryConditions( int condition,
        double valueBegin = 0.0, double valueEnd = 0.0 );

    /// \if ENGLISH Get polygon approximation with tolerance \endif \if CHINESE 获取带容差的多边形逼近 \endif
    virtual QPolygonF polygon( const QPolygonF&, double tolerance ) const;
    /// \if ENGLISH Get painter path from polygon \endif \if CHINESE 从多边形获取绘制路径 \endif
    virtual QPainterPath painterPath( const QPolygonF& ) const = 0;

    /// \if ENGLISH Get locality (number of points used for calculation) \endif \if CHINESE 获取局部性（用于计算的点数） \endif
    virtual uint locality() const;

  private:
    Q_DISABLE_COPY(QwtSpline)

    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief Base class for a spline interpolation

   Spline interpolation is the process of interpolating a set of points
   piecewise with polynomials. The initial set of points is preserved.
 */
class QWT_EXPORT QwtSplineInterpolating : public QwtSpline
{
  public:
    QwtSplineInterpolating();
    virtual ~QwtSplineInterpolating();

    virtual QPolygonF equidistantPolygon( const QPolygonF&,
        double distance, bool withNodes ) const;

    virtual QPolygonF polygon(
        const QPolygonF&, double tolerance ) const override;

    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const = 0;

  private:
    Q_DISABLE_COPY(QwtSplineInterpolating)
};

/*!
   \brief Base class for spline interpolations providing a
         first order geometric continuity ( G1 ) between adjoining curves
 */
class QWT_EXPORT QwtSplineG1 : public QwtSplineInterpolating
{
  public:
    QwtSplineG1();
    virtual ~QwtSplineG1();
};

/*!
   \brief Base class for spline interpolations providing a
         first order parametric continuity ( C1 ) between adjoining curves

   All interpolations with C1 continuity are based on rules for finding
   the 1. derivate at some control points.

   In case of non parametric splines those points are the curve points, while
   for parametric splines the calculation is done twice using a parameter value t.

   \sa QwtSplineParametrization
 */
class QWT_EXPORT QwtSplineC1 : public QwtSplineG1
{
  public:
    QwtSplineC1();
    virtual ~QwtSplineC1();

    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const override;

    virtual QPolygonF equidistantPolygon( const QPolygonF&,
        double distance, bool withNodes ) const override;

    // these methods are the non parametric part
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const;
    virtual QVector< double > slopes( const QPolygonF& ) const = 0;

    virtual double slopeAtBeginning( const QPolygonF&, double slopeNext ) const;
    virtual double slopeAtEnd( const QPolygonF&, double slopeBefore ) const;
};

/*!
   \brief Base class for spline interpolations providing a
         second order parametric continuity ( C2 ) between adjoining curves

   All interpolations with C2 continuity are based on rules for finding
   the 2. derivate at some control points.

   In case of non parametric splines those points are the curve points, while
   for parametric splines the calculation is done twice using a parameter value t.

   \sa QwtSplineParametrization
 */
class QWT_EXPORT QwtSplineC2 : public QwtSplineC1
{
  public:
    /*!
       Boundary condition that requires C2 continuity

       \sa QwtSpline::boundaryCondition, QwtSpline::BoundaryCondition
     */
    enum BoundaryConditionC2
    {
        /*!
           The second derivate at the endpoint is related to the second derivatives
           at the 2 neighbours: cv[0] := 2.0 * cv[1] - cv[2].

           \note boundaryValue() is ignored
         */
        CubicRunout = LinearRunout + 1,

        /*!
           The 3rd derivate at the endpoint matches the 3rd derivate at its neighbours.
           Or in other words: the first/last curve segment extents the polynomial of its
           neighboured polynomial

           \note boundaryValue() is ignored
         */
        NotAKnot
    };

    QwtSplineC2();
    virtual ~QwtSplineC2();

    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const override;

    virtual QPolygonF equidistantPolygon( const QPolygonF&,
        double distance, bool withNodes ) const override;

    // calculating the parametric equations
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const override;
    virtual QVector< double > slopes( const QPolygonF& ) const override;
    virtual QVector< double > curvatures( const QPolygonF& ) const = 0;
};

#endif
