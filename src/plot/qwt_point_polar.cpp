/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_point_polar.h"
#include "qwt_math.h"

#if QT_VERSION >= 0x050200

static QwtPointPolar qwtPointToPolar( const QPointF& point )
{
    return QwtPointPolar( point );
}

#endif

namespace
{
    static const struct RegisterQwtPointPolar
    {
        inline RegisterQwtPointPolar()
        {
            qRegisterMetaType< QwtPointPolar >();

#if QT_VERSION >= 0x050200
            QMetaType::registerConverter< QPointF, QwtPointPolar >( qwtPointToPolar );
            QMetaType::registerConverter< QwtPointPolar, QPointF >( &QwtPointPolar::toPoint );
#endif
        }

    } qwtRegisterQwtPointPolar;
}

/**
 * \if ENGLISH
 * @brief Convert and assign values from a point in Cartesian coordinates
 *
 * @param[in] p Point in Cartesian coordinates
 *
 * @sa setPoint(), toPoint()
 * \endif
 *
 * \if CHINESE
 * @brief 从笛卡尔坐标点转换并赋值
 *
 * @param[in] p 笛卡尔坐标中的点
 *
 * @sa setPoint(), toPoint()
 * \endif
 */
QwtPointPolar::QwtPointPolar( const QPointF& p )
{
    m_radius = std::sqrt( qwtSqr( p.x() ) + qwtSqr( p.y() ) );
    m_azimuth = std::atan2( p.y(), p.x() );
}

/**
 * \if ENGLISH
 * @brief Convert and assign values from a point in Cartesian coordinates
 *
 * @param[in] p Point in Cartesian coordinates
 *
 * @sa QwtPointPolar(QPointF), toPoint()
 * \endif
 *
 * \if CHINESE
 * @brief 从笛卡尔坐标点转换并赋值
 *
 * @param[in] p 笛卡尔坐标中的点
 *
 * @sa QwtPointPolar(QPointF), toPoint()
 * \endif
 */
void QwtPointPolar::setPoint( const QPointF& p )
{
    m_radius = std::sqrt( qwtSqr( p.x() ) + qwtSqr( p.y() ) );
    m_azimuth = std::atan2( p.y(), p.x() );
}

/**
 * \if ENGLISH
 * @brief Convert and return values in Cartesian coordinates
 *
 * @return Converted point in Cartesian coordinates
 *
 * @note Invalid or null points will be returned as QPointF(0.0, 0.0)
 *
 * @sa isValid(), isNull()
 * \endif
 *
 * \if CHINESE
 * @brief 转换并返回笛卡尔坐标值
 *
 * @return 转换后的笛卡尔坐标点
 *
 * @note 无效或空点将返回 QPointF(0.0, 0.0)
 *
 * @sa isValid(), isNull()
 * \endif
 */
QPointF QwtPointPolar::toPoint() const
{
    if ( m_radius <= 0.0 )
        return QPointF( 0.0, 0.0 );

    const double x = m_radius * std::cos( m_azimuth );
    const double y = m_radius * std::sin( m_azimuth );

    return QPointF( x, y );
}

/**
 * \if ENGLISH
 * @brief Compare 2 points
 *
 * @details Two points are equal to each other if radius and
 *          azimuth-coordinates are the same. Points are not equal, when
 *          the azimuth differs, but other.azimuth() == azimuth() % (2 * PI).
 *
 * @return True if the point is equal to other; otherwise return false.
 *
 * @sa normalized()
 * \endif
 *
 * \if CHINESE
 * @brief 比较两个点
 *
 * @details 如果半径和方位角坐标相同，则两个点相等。
 *          当方位角不同但 other.azimuth() == azimuth() % (2 * PI) 时，点不相等。
 *
 * @return 如果点与 other 相等则返回 true；否则返回 false。
 *
 * @sa normalized()
 * \endif
 */
bool QwtPointPolar::operator==( const QwtPointPolar& other ) const
{
    return m_radius == other.m_radius && m_azimuth == other.m_azimuth;
}

/**
 * \if ENGLISH
 * @brief Compare 2 points
 *
 * @details Two points are equal to each other if radius and
 *          azimuth-coordinates are the same. Points are not equal, when
 *          the azimuth differs, but other.azimuth() == azimuth() % (2 * PI).
 *
 * @return True if the point is not equal to other; otherwise return false.
 *
 * @sa normalized()
 * \endif
 *
 * \if CHINESE
 * @brief 比较两个点
 *
 * @details 如果半径和方位角坐标相同，则两个点相等。
 *          当方位角不同但 other.azimuth() == azimuth() % (2 * PI) 时，点不相等。
 *
 * @return 如果点与 other 不相等则返回 true；否则返回 false。
 *
 * @sa normalized()
 * \endif
 */
bool QwtPointPolar::operator!=( const QwtPointPolar& other ) const
{
    return m_radius != other.m_radius || m_azimuth != other.m_azimuth;
}

/**
 * \if ENGLISH
 * @brief Normalize radius and azimuth
 *
 * @details When the radius is < 0.0 it is set to 0.0.
 *          The azimuth is a value >= 0.0 and < 2 * M_PI.
 *
 * @return Normalized point
 * \endif
 *
 * \if CHINESE
 * @brief 规范化半径和方位角
 *
 * @details 当半径 < 0.0 时设置为 0.0。
 *          方位角是一个 >= 0.0 且 < 2 * M_PI 的值。
 *
 * @return 规范化后的点
 * \endif
 */
QwtPointPolar QwtPointPolar::normalized() const
{
    const double radius = qwtMaxF( m_radius, 0.0 );

    double azimuth = m_azimuth;
    if ( azimuth < -2.0 * M_PI || azimuth >= 2 * M_PI )
        azimuth = std::fmod( m_azimuth, 2 * M_PI );

    if ( azimuth < 0.0 )
        azimuth += 2 * M_PI;

    return QwtPointPolar( azimuth, radius );
}

#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<( QDebug debug, const QwtPointPolar& point )
{
    debug.nospace() << "QwtPointPolar("
                    << point.azimuth() << "," << point.radius() << ")";

    return debug.space();
}

#endif

