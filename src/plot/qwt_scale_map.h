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

#ifndef QWT_SCALE_MAP_H
#define QWT_SCALE_MAP_H

#include "qwt_global.h"
#include "qwt_transform.h"

class QPointF;
class QRectF;

/**
 * \if ENGLISH
 * @brief A scale map
 * @details QwtScaleMap offers transformations from the coordinate system
 *          of a scale into the linear coordinate system of a paint device
 *          and vice versa.
 * @sa QwtTransform, QwtScaleDiv
 * \endif
 * \if CHINESE
 * @brief 刻度映射
 * @details QwtScaleMap 提供从刻度坐标系到绘制设备线性坐标系之间的变换，
 *          以及反向变换。
 * @sa QwtTransform, QwtScaleDiv
 * \endif
 */
class QWT_EXPORT QwtScaleMap
{
public:
    //! Default constructor
    QwtScaleMap();
    //! Copy constructor
    QwtScaleMap(const QwtScaleMap&);
    //! Move constructor
    QwtScaleMap(QwtScaleMap&&);

    //! Destructor
    ~QwtScaleMap();

    //! Copy assignment operator
    QwtScaleMap& operator=(const QwtScaleMap&);
    //! Move assignment operator
    QwtScaleMap& operator=(QwtScaleMap&&);

    //! Set the transformation (takes ownership)
    void setTransformation(QwtTransform*);
    //! Return the transformation
    const QwtTransform* transformation() const;

    //! Set the paint device interval boundaries
    void setPaintInterval(double p1, double p2);
    //! Set the scale interval boundaries
    void setScaleInterval(double s1, double s2);

    //! Transform a scale value to paint device coordinate
    double transform(double s) const;
    //! Transform a paint device coordinate to scale value
    double invTransform(double p) const;

    //! Return first border of paint interval
    double p1() const;
    //! Return second border of paint interval
    double p2() const;

    //! Return first border of scale interval
    double s1() const;
    //! Return second border of scale interval
    double s2() const;

    //! Return distance between paint interval boundaries
    double pDist() const;
    //! Return distance between scale interval boundaries
    double sDist() const;

    //! Transform a rectangle from scale to paint coordinates
    static QRectF transform(const QwtScaleMap&, const QwtScaleMap&, const QRectF&);

    //! Transform a rectangle from paint to scale coordinates
    static QRectF invTransform(const QwtScaleMap&, const QwtScaleMap&, const QRectF&);

    //! Transform a point from scale to paint coordinates
    static QPointF transform(const QwtScaleMap&, const QwtScaleMap&, const QPointF&);

    //! Transform a point from paint to scale coordinates
    static QPointF invTransform(const QwtScaleMap&, const QwtScaleMap&, const QPointF&);

    //! Check if the scale is linear (no transformation)
    static bool isLinerScale(const QwtScaleMap& sm);

    //! Check if the mapping direction is inverted
    bool isInverting() const;

protected:
    void swap(QwtScaleMap& other) noexcept;  // 辅助
private:
    void updateFactor();

    double m_s1, m_s2;  // scale interval boundaries
    double m_p1, m_p2;  // paint device interval boundaries

    double m_cnv;  // conversion factor
    double m_ts1;

    QwtTransform* m_transform;
};

/**
 * \if ENGLISH
 * @brief Return first border of the scale interval
 * \endif
 * \if CHINESE
 * @brief 返回刻度区间的第一个边界
 * \endif
 */
inline double QwtScaleMap::s1() const
{
    return m_s1;
}

/**
 * \if ENGLISH
 * @brief Return second border of the scale interval
 * \endif
 * \if CHINESE
 * @brief 返回刻度区间的第二个边界
 * \endif
 */
inline double QwtScaleMap::s2() const
{
    return m_s2;
}

/**
 * \if ENGLISH
 * @brief Return first border of the paint interval
 * \endif
 * \if CHINESE
 * @brief 返回绘制区间的第一个边界
 * \endif
 */
inline double QwtScaleMap::p1() const
{
    return m_p1;
}

/**
 * \if ENGLISH
 * @brief Return second border of the paint interval
 * \endif
 * \if CHINESE
 * @brief 返回绘制区间的第二个边界
 * \endif
 */
inline double QwtScaleMap::p2() const
{
    return m_p2;
}

/**
 * \if ENGLISH
 * @brief Return qwtAbs(p2() - p1())
 * \endif
 * \if CHINESE
 * @brief 返回 qwtAbs(p2() - p1())
 * \endif
 */
inline double QwtScaleMap::pDist() const
{
    return qAbs(m_p2 - m_p1);
}

/**
 * \if ENGLISH
 * @brief Return qwtAbs(s2() - s1())
 * \endif
 * \if CHINESE
 * @brief 返回 qwtAbs(s2() - s1())
 * \endif
 */
inline double QwtScaleMap::sDist() const
{
    return qAbs(m_s2 - m_s1);
}

/**
 * \if ENGLISH
 * @brief Transform a point related to the scale interval into a point related to the paint device interval
 * @param s Value relative to the coordinates of the scale
 * @return Transformed value
 * \sa invTransform()
 * \endif
 * \if CHINESE
 * @brief 将相对于刻度区间的点转换为相对于绘制设备区间的点
 * @param s 相对于刻度坐标的值
 * @return 转换后的值
 * \sa invTransform()
 * \endif
 */
inline double QwtScaleMap::transform(double s) const
{
    if (m_transform)
        s = m_transform->transform(s);

    return m_p1 + (s - m_ts1) * m_cnv;
}

/**
 * \if ENGLISH
 * @brief Transform a paint device value into a value in the interval of the scale
 * @param p Value relative to the coordinates of the paint device
 * @return Transformed value
 * \sa transform()
 * \endif
 * \if CHINESE
 * @brief 将绘制设备值转换为刻度区间中的值
 * @param p 相对于绘制设备坐标的值
 * @return 转换后的值
 * \sa transform()
 * \endif
 */
inline double QwtScaleMap::invTransform(double p) const
{
    double s = m_ts1 + (p - m_p1) / m_cnv;
    if (m_transform)
        s = m_transform->invTransform(s);

    return s;
}

//! \if ENGLISH Return true when ( p1() < p2() ) != ( s1() < s2() ) \endif \if CHINESE 当 ( p1() < p2() ) != ( s1() < s2() ) 时返回 true \endif
inline bool QwtScaleMap::isInverting() const
{
    return ((m_p1 < m_p2) != (m_s1 < m_s2));
}

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<(QDebug, const QwtScaleMap&);
#endif

#endif
