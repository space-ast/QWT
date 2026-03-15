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

/*! \file */
#ifndef QWT_POINT_POLAR_H
#define QWT_POINT_POLAR_H

#include "qwt_global.h"
#include "qwt_math.h"

#include <qpoint.h>
#include <qmetatype.h>
#include <qmath.h>

/**
 * \if ENGLISH
 * @brief A point in polar coordinates
 * @details In polar coordinates a point is determined by an angle and a distance.
 *          See http://en.wikipedia.org/wiki/Polar_coordinate_system
 * \endif
 * \if CHINESE
 * @brief 极坐标点
 * @details 在极坐标中，点由角度和距离确定。
 *          参见 http://en.wikipedia.org/wiki/Polar_coordinate_system
 * \endif
 */

class QWT_EXPORT QwtPointPolar
{
  public:
    QwtPointPolar();
    QwtPointPolar( double azimuth, double radius );
    QwtPointPolar( const QPointF& );

    void setPoint( const QPointF& );
    QPointF toPoint() const;

    bool isValid() const;
    bool isNull() const;

    double radius() const;
    double azimuth() const;

    double& rRadius();
    double& rAzimuth();

    void setRadius( double );
    void setAzimuth( double );

    bool operator==( const QwtPointPolar& ) const;
    bool operator!=( const QwtPointPolar& ) const;

    QwtPointPolar normalized() const;

  private:
    double m_azimuth;
    double m_radius;
};

Q_DECLARE_TYPEINFO( QwtPointPolar, Q_MOVABLE_TYPE );
Q_DECLARE_METATYPE( QwtPointPolar );

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<( QDebug, const QwtPointPolar& );
#endif

/**
 * \if ENGLISH
 * @brief Default constructor - constructs a null point with radius and azimuth set to 0.0
 * \sa QPointF::isNull()
 * \endif
 * \if CHINESE
 * @brief 默认构造函数 - 构造一个空点，半径和方位角都设置为 0.0
 * \sa QPointF::isNull()
 * \endif
 */
inline QwtPointPolar::QwtPointPolar()
    : m_azimuth( 0.0 )
    , m_radius( 0.0 )
{
}

/**
 * \if ENGLISH
 * @brief Constructs a point with coordinates specified by radius and azimuth
 * @param azimuth Azimuth (angle)
 * @param radius Radius (distance)
 * \endif
 * \if CHINESE
 * @brief 构造一个由半径和方位角指定坐标的点
 * @param azimuth 方位角（角度）
 * @param radius 半径（距离）
 * \endif
 */
inline QwtPointPolar::QwtPointPolar( double azimuth, double radius )
    : m_azimuth( azimuth )
    , m_radius( radius )
{
}

//! \if ENGLISH Return true if radius() >= 0.0 \endif \if CHINESE 如果 radius() >= 0.0 则返回 true \endif
inline bool QwtPointPolar::isValid() const
{
    return m_radius >= 0.0;
}

//! \if ENGLISH Return true if radius() == 0.0 \endif \if CHINESE 如果 radius() == 0.0 则返回 true \endif
inline bool QwtPointPolar::isNull() const
{
    return m_radius == 0.0;
}

//! \if ENGLISH Return the radius \endif \if CHINESE 返回半径 \endif
inline double QwtPointPolar::radius() const
{
    return m_radius;
}

//! \if ENGLISH Return the azimuth \endif \if CHINESE 返回方位角 \endif
inline double QwtPointPolar::azimuth() const
{
    return m_azimuth;
}

//! \if ENGLISH Return a reference to the radius \endif \if CHINESE 返回半径的引用 \endif
inline double& QwtPointPolar::rRadius()
{
    return m_radius;
}

//! \if ENGLISH Return a reference to the azimuth \endif \if CHINESE 返回方位角的引用 \endif
inline double& QwtPointPolar::rAzimuth()
{
    return m_azimuth;
}

//! \if ENGLISH Set the radius to radius \endif \if CHINESE 将半径设置为 radius \endif
inline void QwtPointPolar::setRadius( double radius )
{
    m_radius = radius;
}

//! \if ENGLISH Set the azimuth to azimuth \endif \if CHINESE 将方位角设置为 azimuth \endif
inline void QwtPointPolar::setAzimuth( double azimuth )
{
    m_azimuth = azimuth;
}

inline QPoint qwtPolar2Pos( const QPoint& pole,
    double radius, double angle )
{
    const double x = pole.x() + radius * std::cos( angle );
    const double y = pole.y() - radius * std::sin( angle );

    return QPoint( qRound( x ), qRound( y ) );
}

inline QPoint qwtDegree2Pos( const QPoint& pole,
    double radius, double angle )
{
    return qwtPolar2Pos( pole, radius, angle / 180.0 * M_PI );
}

inline QPointF qwtPolar2Pos( const QPointF& pole,
    double radius, double angle )
{
    const double x = pole.x() + radius * std::cos( angle );
    const double y = pole.y() - radius * std::sin( angle );

    return QPointF( x, y);
}

inline QPointF qwtDegree2Pos( const QPointF& pole,
    double radius, double angle )
{
    return qwtPolar2Pos( pole, radius, angle / 180.0 * M_PI );
}

inline QPointF qwtFastPolar2Pos( const QPointF& pole,
    double radius, double angle )
{
    const double x = pole.x() + radius * qFastCos( angle );
    const double y = pole.y() - radius * qFastSin( angle );

    return QPointF( x, y);
}

inline QPointF qwtFastDegree2Pos( const QPointF& pole,
    double radius, double angle )
{
    return qwtFastPolar2Pos( pole, radius, angle / 180.0 * M_PI );
}

inline QwtPointPolar qwtFastPos2Polar( const QPointF& pos )
{
    return QwtPointPolar( qwtFastAtan2( pos.y(), pos.x() ),
        qSqrt( qwtSqr( pos.x() ) + qwtSqr( pos.y() ) ) );
}

#endif
