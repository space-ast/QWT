/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_fitter.h"
#include <qpolygon.h>
#include <qpainterpath.h>

class QwtPolarFitter::PrivateData
{
  public:
    PrivateData()
        : stepCount( 5 )
    {
    }

    int stepCount;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] stepCount Number of points that will be inserted between 2 points
 * @sa setStepCount()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] stepCount 在两个点之间插入的点数
 * @sa setStepCount()
 * \endif
 */
QwtPolarFitter::QwtPolarFitter( int stepCount )
    : QwtCurveFitter( QwtPolarFitter::Polygon )
{
    m_data = new PrivateData;
    m_data->stepCount = stepCount;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPolarFitter::~QwtPolarFitter()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Assign the number of points that will be inserted between 2 points
 * @details The default value is 5.
 * @param[in] stepCount Number of steps
 * @sa stepCount()
 * \endif
 *
 * \if CHINESE
 * @brief 设置在两个点之间插入的点数
 * @details 默认值为 5。
 * @param[in] stepCount 步数
 * @sa stepCount()
 * \endif
 */
void QwtPolarFitter::setStepCount( int stepCount )
{
    m_data->stepCount = qMax( stepCount, 0 );
}

/**
 * \if ENGLISH
 * @brief Get the number of points that will be inserted between 2 points
 * @return Number of points that will be inserted between 2 points
 * @sa setStepCount()
 * \endif
 *
 * \if CHINESE
 * @brief 获取在两个点之间插入的点数
 * @return 在两个点之间插入的点数
 * @sa setStepCount()
 * \endif
 */
int QwtPolarFitter::stepCount() const
{
    return m_data->stepCount;
}

/**
 * \if ENGLISH
 * @brief Insert stepCount() number of additional points between 2 elements of points
 * @param[in] points Array of points
 * @return Array of points including the additional points
 * \endif
 *
 * \if CHINESE
 * @brief 在两个点元素之间插入 stepCount() 个附加点
 * @param[in] points 点数组
 * @return 包含附加点的点数组
 * \endif
 */
QPolygonF QwtPolarFitter::fitCurve( const QPolygonF& points ) const
{
    if ( m_data->stepCount <= 0 || points.size() <= 1 )
        return points;

    QPolygonF fittedPoints;

    int numPoints = points.size() + ( points.size() - 1 ) * m_data->stepCount;

    fittedPoints.resize( numPoints );

    int index = 0;
    fittedPoints[index++] = points[0];
    for ( int i = 1; i < points.size(); i++ )
    {
        const QPointF& p1 = points[i - 1];
        const QPointF& p2 = points[i];

        const double dx = ( p2.x() - p1.x() ) / m_data->stepCount;
        const double dy = ( p2.y() - p1.y() ) / m_data->stepCount;
        for ( int j = 1; j <= m_data->stepCount; j++ )
        {
            const double x = p1.x() + j * dx;
            const double y = p1.y() + j * dy;

            fittedPoints[index++] = QPointF( x, y );
        }
    }
    fittedPoints.resize( index );

    return fittedPoints;
}

/**
 * \if ENGLISH
 * @brief Create a curve path from the data points
 * @param[in] points Series of data points
 * @return Curve path
 * @sa fitCurve()
 * \endif
 *
 * \if CHINESE
 * @brief 从数据点创建曲线路径
 * @param[in] points 数据点序列
 * @return 曲线路径
 * @sa fitCurve()
 * \endif
 */
QPainterPath QwtPolarFitter::fitCurvePath( const QPolygonF& points ) const
{
    QPainterPath path;
    path.addPolygon( fitCurve( points ) );
    return path;
}
