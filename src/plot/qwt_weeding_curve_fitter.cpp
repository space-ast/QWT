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

#include "qwt_weeding_curve_fitter.h"
#include "qwt_math.h"

#include <qpainterpath.h>
#include <qpolygon.h>
#include <qstack.h>
#include <qvector.h>

class QwtWeedingCurveFitter::PrivateData
{
  public:
    PrivateData()
        : tolerance( 1.0 )
        , chunkSize( 0 )
    {
    }

    double tolerance;
    uint chunkSize;
};

class QwtWeedingCurveFitter::Line
{
  public:
    Line( int i1 = 0, int i2 = 0 )
        : from( i1 )
        , to( i2 )
    {
    }

    int from;
    int to;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param tolerance Tolerance
 * \sa setTolerance(), tolerance()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param tolerance 容差
 * \sa setTolerance(), tolerance()
 * \endif
 */
QwtWeedingCurveFitter::QwtWeedingCurveFitter( double tolerance )
    : QwtCurveFitter( QwtCurveFitter::Polygon )
{
    m_data = new PrivateData;
    setTolerance( tolerance );
}

//! Destructor
QwtWeedingCurveFitter::~QwtWeedingCurveFitter()
{
    delete m_data;
}

/*!
   \if ENGLISH
   Assign the tolerance

   The tolerance is the maximum distance, that is acceptable
   between the original curve and the smoothed curve.

   Increasing the tolerance will reduce the number of the
   resulting points.

   \param tolerance Tolerance

   \sa tolerance()
   \endif
   *
   \if CHINESE
   设置容差

   容差是原始曲线和平滑曲线之间可接受的最大距离。

   增加容差将减少结果点的数量。

   \param tolerance 容差

   \sa tolerance()
   \endif
 */
void QwtWeedingCurveFitter::setTolerance( double tolerance )
{
    m_data->tolerance = qwtMaxF( tolerance, 0.0 );
}

/*!
   \if ENGLISH
   \return Tolerance
   \sa setTolerance()
   \endif
   *
   \if CHINESE
   \return 容差
   \sa setTolerance()
   \endif
 */
double QwtWeedingCurveFitter::tolerance() const
{
    return m_data->tolerance;
}

/*!
   \if ENGLISH
   Limit the number of points passed to a run of the algorithm

   The runtime of the Douglas Peucker algorithm increases non linear
   with the number of points. For a chunk size > 0 the polygon
   is split into pieces passed to the algorithm one by one.

   \param numPoints Maximum for the number of points passed to the algorithm

   \sa chunkSize()
   \endif
   *
   \if CHINESE
   限制传递给算法运行的点数

   Douglas Peucker 算法的运行时间随点数非线性增长。
   对于块大小 > 0，多边形被拆分成块，逐个传递给算法。

   \param numPoints 传递给算法的点数最大值

   \sa chunkSize()
   \endif
 */
void QwtWeedingCurveFitter::setChunkSize( uint numPoints )
{
    if ( numPoints > 0 )
        numPoints = qMax( numPoints, 3U );

    m_data->chunkSize = numPoints;
}

/*!
   \if ENGLISH
   \return Maximum for the number of points passed to a run
          of the algorithm - or 0, when unlimited
   \sa setChunkSize()
   \endif
   *
   \if CHINESE
   \return 传递给算法运行的点数最大值 - 如果无限制则为 0
   \sa setChunkSize()
   \endif
 */
uint QwtWeedingCurveFitter::chunkSize() const
{
    return m_data->chunkSize;
}

/*!
   \if ENGLISH
   @param points Series of data points
   @return Curve points
   \sa fitCurvePath()
   \endif
   *
   \if CHINESE
   @param points 数据点序列
   @return 曲线点
   \sa fitCurvePath()
   \endif
 */
QPolygonF QwtWeedingCurveFitter::fitCurve( const QPolygonF& points ) const
{
    if ( points.isEmpty() )
        return points;

    QPolygonF fittedPoints;
    if ( m_data->chunkSize == 0 )
    {
        fittedPoints = simplify( points );
    }
    else
    {
        for ( int i = 0; i < points.size(); i += m_data->chunkSize )
        {
            const QPolygonF p = points.mid( i, m_data->chunkSize );
            fittedPoints += simplify( p );
        }
    }

    return fittedPoints;
}

/*!
   \if ENGLISH
   @param points Series of data points
   @return Curve path
   \sa fitCurve()
   \endif
   *
   \if CHINESE
   @param points 数据点序列
   @return 曲线路径
   \sa fitCurve()
   \endif
 */
QPainterPath QwtWeedingCurveFitter::fitCurvePath( const QPolygonF& points ) const
{
    QPainterPath path;
    path.addPolygon( fitCurve( points ) );
    return path;
}

QPolygonF QwtWeedingCurveFitter::simplify( const QPolygonF& points ) const
{
    const double toleranceSqr = m_data->tolerance * m_data->tolerance;

    QStack< Line > stack;
    stack.reserve( 500 );

    const QPointF* p = points.data();
    const int nPoints = points.size();

    QVector< bool > usePoint( nPoints, false );

    stack.push( Line( 0, nPoints - 1 ) );

    while ( !stack.isEmpty() )
    {
        const Line r = stack.pop();

        // initialize line segment
        const double vecX = p[r.to].x() - p[r.from].x();
        const double vecY = p[r.to].y() - p[r.from].y();

        const double vecLength = std::sqrt( vecX * vecX + vecY * vecY );

        const double unitVecX = ( vecLength != 0.0 ) ? vecX / vecLength : 0.0;
        const double unitVecY = ( vecLength != 0.0 ) ? vecY / vecLength : 0.0;

        double maxDistSqr = 0.0;
        int nVertexIndexMaxDistance = r.from + 1;
        for ( int i = r.from + 1; i < r.to; i++ )
        {
            //compare to anchor
            const double fromVecX = p[i].x() - p[r.from].x();
            const double fromVecY = p[i].y() - p[r.from].y();

            double distToSegmentSqr;
            if ( fromVecX * unitVecX + fromVecY * unitVecY < 0.0 )
            {
                distToSegmentSqr = fromVecX * fromVecX + fromVecY * fromVecY;
            }
            else
            {
                const double toVecX = p[i].x() - p[r.to].x();
                const double toVecY = p[i].y() - p[r.to].y();
                const double toVecLength = toVecX * toVecX + toVecY * toVecY;

                const double s = toVecX * ( -unitVecX ) + toVecY * ( -unitVecY );
                if ( s < 0.0 )
                {
                    distToSegmentSqr = toVecLength;
                }
                else
                {
                    distToSegmentSqr = std::fabs( toVecLength - s * s );
                }
            }

            if ( maxDistSqr < distToSegmentSqr )
            {
                maxDistSqr = distToSegmentSqr;
                nVertexIndexMaxDistance = i;
            }
        }
        if ( maxDistSqr <= toleranceSqr )
        {
            usePoint[r.from] = true;
            usePoint[r.to] = true;
        }
        else
        {
            stack.push( Line( r.from, nVertexIndexMaxDistance ) );
            stack.push( Line( nVertexIndexMaxDistance, r.to ) );
        }
    }

    QPolygonF stripped;
    for ( int i = 0; i < nPoints; i++ )
    {
        if ( usePoint[i] )
            stripped += p[i];
    }

    return stripped;
}
