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
#ifndef QWT_POINT_3D_H
#define QWT_POINT_3D_H

#include "qwt_global.h"
#include <qpoint.h>
#include <qmetatype.h>

/**
 * \if ENGLISH
 * @brief QwtPoint3D class defines a 3D point in double coordinates
 * \endif
 * \if CHINESE
 * @brief QwtPoint3D 类定义双精度坐标的 3D 点
 * \endif
 */

class QWT_EXPORT QwtPoint3D
{
public:
    QwtPoint3D();
    QwtPoint3D(double x, double y, double z);
    QwtPoint3D(const QPointF&);

    bool isNull() const;

    double x() const;
    double y() const;
    double z() const;

    double& rx();
    double& ry();
    double& rz();

    void setX(double x);
    void setY(double y);
    void setZ(double y);

    QPointF toPoint() const;

    bool operator==(const QwtPoint3D&) const;
    bool operator!=(const QwtPoint3D&) const;

private:
    double m_x;
    double m_y;
    double m_z;
};

Q_DECLARE_TYPEINFO(QwtPoint3D, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(QwtPoint3D);

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<(QDebug, const QwtPoint3D&);
#endif

/**
 * \if ENGLISH
 * @brief Default constructor - constructs a null point
 * \sa isNull()
 * \endif
 * \if CHINESE
 * @brief 默认构造函数 - 构造一个空点
 * \sa isNull()
 * \endif
 */
inline QwtPoint3D::QwtPoint3D()
    : m_x( 0.0 )
    , m_y( 0.0 )
    , m_z( 0.0 )
{
}

/**
 * \if ENGLISH
 * @brief Constructs a point with coordinates specified by x, y and z
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate (default 0.0)
 * \endif
 * \if CHINESE
 * @brief 构造一个具有指定 x, y, z 坐标的点
 * @param x X 坐标
 * @param y Y 坐标
 * @param z Z 坐标 (默认 0.0)
 * \endif
 */
inline QwtPoint3D::QwtPoint3D( double x, double y, double z = 0.0 )
    : m_x( x )
    , m_y( y )
    , m_z( z )
{
}

/**
 * \if ENGLISH
 * @brief Constructs a point with x and y coordinates from a 2D point, and z coordinate of 0
 * @param other 2D point
 * \endif
 * \if CHINESE
 * @brief 从 2D 点构造一个点，x 和 y 坐标来自 2D 点，z 坐标为 0
 * @param other 2D 点
 * \endif
 */
inline QwtPoint3D::QwtPoint3D( const QPointF& other )
    : m_x( other.x() )
    , m_y( other.y() )
    , m_z( 0.0 )
{
}

/**
 * \if ENGLISH
 * @return True if the point is null (x, y, z all equal to zero); otherwise false
 * \endif
 * \if CHINESE
 * @return 如果点是空的（x, y, z 都等于零）则为 true；否则为 false
 * \endif
 */
inline bool QwtPoint3D::isNull() const
{
    return m_x == 0.0 && m_y == 0.0 && m_z == 0.0;
}

//! \if ENGLISH Return the x-coordinate of the point \endif \if CHINESE 返回点的 X 坐标 \endif
inline double QwtPoint3D::x() const
{
    return m_x;
}

//! \if ENGLISH Return the y-coordinate of the point \endif \if CHINESE 返回点的 Y 坐标 \endif
inline double QwtPoint3D::y() const
{
    return m_y;
}

//! \if ENGLISH Return the z-coordinate of the point \endif \if CHINESE 返回点的 Z 坐标 \endif
inline double QwtPoint3D::z() const
{
    return m_z;
}

//! \if ENGLISH Return a reference to the x-coordinate of the point \endif \if CHINESE 返回点 X 坐标的引用 \endif
inline double& QwtPoint3D::rx()
{
    return m_x;
}

//! \if ENGLISH Return a reference to the y-coordinate of the point \endif \if CHINESE 返回点 Y 坐标的引用 \endif
inline double& QwtPoint3D::ry()
{
    return m_y;
}

//! \if ENGLISH Return a reference to the z-coordinate of the point \endif \if CHINESE 返回点 Z 坐标的引用 \endif
inline double& QwtPoint3D::rz()
{
    return m_z;
}

//! \if ENGLISH Set the x-coordinate of the point to x \endif \if CHINESE 将点的 X 坐标设置为 x \endif
inline void QwtPoint3D::setX( double x )
{
    m_x = x;
}

//! \if ENGLISH Set the y-coordinate of the point to y \endif \if CHINESE 将点的 Y 坐标设置为 y \endif
inline void QwtPoint3D::setY( double y )
{
    m_y = y;
}

//! \if ENGLISH Set the z-coordinate of the point to z \endif \if CHINESE 将点的 Z 坐标设置为 z \endif
inline void QwtPoint3D::setZ( double z )
{
    m_z = z;
}

/**
 * \if ENGLISH
 * @return 2D point with the z coordinate dropped
 * \endif
 * \if CHINESE
 * @return 去掉 z 坐标的 2D 点
 * \endif
 */
inline QPointF QwtPoint3D::toPoint() const
{
    return QPointF( m_x, m_y );
}

//! \if ENGLISH Return true if this point and other are equal \endif \if CHINESE 如果此点与另一点相等则返回 true \endif
inline bool QwtPoint3D::operator==( const QwtPoint3D& other ) const
{
    return ( m_x == other.m_x ) && ( m_y == other.m_y ) && ( m_z == other.m_z );
}

//! \if ENGLISH Return true if this point and other are different \endif \if CHINESE 如果此点与另一点不同则返回 true \endif
inline bool QwtPoint3D::operator!=( const QwtPoint3D& other ) const
{
    return !operator==( other );
}

#endif
