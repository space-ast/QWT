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

#include "qwt_scale_map.h"
#include "qwt_math.h"

#include <qrect.h>
#include <qdebug.h>

/**
 * \if ENGLISH
 * @brief Default constructor
 * @details The scale and paint device intervals are both set to [0,1].
 * \endif
 * \if CHINESE
 * @brief 默认构造函数
 * @details 刻度和绘制设备区间都设置为 [0,1]。
 * \endif
 */
QwtScaleMap::QwtScaleMap()
    : m_s1(0.0), m_s2(1.0), m_p1(0.0), m_p2(1.0), m_cnv(1.0), m_ts1(0.0), m_transform(nullptr)
{
}

/**
 * \if ENGLISH
 * @brief Copy constructor
 * \endif
 * \if CHINESE
 * @brief 复制构造函数
 * \endif
 */
QwtScaleMap::QwtScaleMap(const QwtScaleMap& other)
    : m_s1(other.m_s1), m_s2(other.m_s2), m_p1(other.m_p1), m_p2(other.m_p2), m_cnv(other.m_cnv), m_ts1(other.m_ts1), m_transform(nullptr)
{
    if (other.m_transform)
        m_transform = other.m_transform->copy();
}

QwtScaleMap::QwtScaleMap(QwtScaleMap&& other) : QwtScaleMap()
{
    swap(other);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtScaleMap::~QwtScaleMap()
{
    delete m_transform;
}

/**
 * \if ENGLISH
 * @brief Assignment operator
 * \endif
 * \if CHINESE
 * @brief 赋值运算符
 * \endif
 */
QwtScaleMap& QwtScaleMap::operator=(const QwtScaleMap& other)
{
    m_s1  = other.m_s1;
    m_s2  = other.m_s2;
    m_p1  = other.m_p1;
    m_p2  = other.m_p2;
    m_cnv = other.m_cnv;
    m_ts1 = other.m_ts1;

    delete m_transform;
    m_transform = nullptr;

    if (other.m_transform)
        m_transform = other.m_transform->copy();

    return *this;
}

QwtScaleMap& QwtScaleMap::operator=(QwtScaleMap&& other)
{
    // 交换后，other会析构，这时原来的m_transform会在other那里析构掉
    swap(other);
    return *this;
}
/**
 * \if ENGLISH
 * @brief Initialize the map with a transformation
 * @param transform Transformation object (takes ownership)
 * \endif
 * \if CHINESE
 * @brief 使用变换初始化映射
 * @param transform 变换对象（获取所有权）
 * \endif
 */
void QwtScaleMap::setTransformation(QwtTransform* transform)
{
    if (transform != m_transform) {
        delete m_transform;
        m_transform = transform;
    }

    setScaleInterval(m_s1, m_s2);
}

/**
 * \if ENGLISH
 * @brief Get the transformation
 * @return Transformation object
 * \endif
 * \if CHINESE
 * @brief 获取变换
 * @return 变换对象
 * \endif
 */
const QwtTransform* QwtScaleMap::transformation() const
{
    return m_transform;
}

/**
 * \if ENGLISH
 * @brief Specify the borders of the scale interval
 * @details Scales might be aligned to transformation depending boundaries
 * @param s1 First border
 * @param s2 Second border
 * \endif
 * \if CHINESE
 * @brief 指定刻度区间的边界
 * @details 刻度可能会根据变换的依赖边界进行对齐
 * @param s1 第一个边界
 * @param s2 第二个边界
 * \endif
 */
void QwtScaleMap::setScaleInterval(double s1, double s2)
{
    m_s1 = s1;
    m_s2 = s2;

    if (m_transform) {
        m_s1 = m_transform->bounded(m_s1);
        m_s2 = m_transform->bounded(m_s2);
    }

    updateFactor();
}

/**
 * \if ENGLISH
 * @brief Specify the borders of the paint device interval
 * @param p1 First border
 * @param p2 Second border
 * \endif
 * \if CHINESE
 * @brief 指定绘制设备区间的边界
 * @param p1 第一个边界
 * @param p2 第二个边界
 * \endif
 */
void QwtScaleMap::setPaintInterval(double p1, double p2)
{
    m_p1 = p1;
    m_p2 = p2;

    updateFactor();
}

void QwtScaleMap::updateFactor()
{
    m_ts1      = m_s1;
    double ts2 = m_s2;

    if (m_transform) {
        m_ts1 = m_transform->transform(m_ts1);
        ts2   = m_transform->transform(ts2);
    }

    m_cnv = 1.0;
    if (m_ts1 != ts2)
        m_cnv = (m_p2 - m_p1) / (ts2 - m_ts1);
}

/**
 * \if ENGLISH
 * @brief Transform a rectangle from scale to paint coordinates
 * @param xMap X axis scale map
 * @param yMap Y axis scale map
 * @param rect Rectangle in scale coordinates
 * @return Rectangle in paint coordinates
 * @sa invTransform()
 * \endif
 * \if CHINESE
 * @brief 将矩形从刻度坐标转换为绘制坐标
 * @param xMap X 轴刻度映射
 * @param yMap Y 轴刻度映射
 * @param rect 刻度坐标系中的矩形
 * @return 绘制坐标系中的矩形
 * @sa invTransform()
 * \endif
 */
QRectF QwtScaleMap::transform(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect)
{
    double x1 = xMap.transform(rect.left());
    double x2 = xMap.transform(rect.right());
    double y1 = yMap.transform(rect.top());
    double y2 = yMap.transform(rect.bottom());

    if (x2 < x1)
        qSwap(x1, x2);
    if (y2 < y1)
        qSwap(y1, y2);

    if (qwtFuzzyCompare(x1, 0.0, x2 - x1) == 0)
        x1 = 0.0;
    if (qwtFuzzyCompare(x2, 0.0, x2 - x1) == 0)
        x2 = 0.0;
    if (qwtFuzzyCompare(y1, 0.0, y2 - y1) == 0)
        y1 = 0.0;
    if (qwtFuzzyCompare(y2, 0.0, y2 - y1) == 0)
        y2 = 0.0;

    return QRectF(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

/**
 * \if ENGLISH
 * @brief Transform a point from paint to scale coordinates
 * @param xMap X axis scale map
 * @param yMap Y axis scale map
 * @param pos Position in paint coordinates
 * @return Position in scale coordinates
 * @sa transform()
 * \endif
 * \if CHINESE
 * @brief 将点从绘制坐标转换为刻度坐标
 * @param xMap X 轴刻度映射
 * @param yMap Y 轴刻度映射
 * @param pos 绘制坐标系中的位置
 * @return 刻度坐标系中的位置
 * @sa transform()
 * \endif
 */
QPointF QwtScaleMap::invTransform(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QPointF& pos)
{
    return QPointF(xMap.invTransform(pos.x()), yMap.invTransform(pos.y()));
}

/**
 * \if ENGLISH
 * @brief Check if the scale is linear (no transformation)
 * @param sm Scale map to check
 * @return True if the scale has no transformation (linear scale)
 * \endif
 * \if CHINESE
 * @brief 检查刻度是否为线性（无变换）
 * @param sm 要检查的刻度映射
 * @return 如果刻度没有变换（线性刻度）则返回 true
 * \endif
 */
bool QwtScaleMap::isLinerScale(const QwtScaleMap& sm)
{
    // 检查变换是否为对数变换
    return sm.transformation() == nullptr;
}

void QwtScaleMap::swap(QwtScaleMap& other) noexcept
{
    // 基本类型，直接换
    std::swap(m_s1, other.m_s1);
    std::swap(m_s2, other.m_s2);
    std::swap(m_p1, other.m_p1);
    std::swap(m_p2, other.m_p2);
    std::swap(m_cnv, other.m_cnv);
    std::swap(m_ts1, other.m_ts1);
    std::swap(m_transform, other.m_transform);
}

/**
 * \if ENGLISH
 * @brief Transform a point from scale to paint coordinates
 * @param xMap X axis scale map
 * @param yMap Y axis scale map
 * @param pos Position in scale coordinates
 * @return Position in paint coordinates
 * @sa invTransform()
 * \endif
 * \if CHINESE
 * @brief 将点从刻度坐标转换为绘制坐标
 * @param xMap X 轴刻度映射
 * @param yMap Y 轴刻度映射
 * @param pos 刻度坐标系中的位置
 * @return 绘制坐标系中的位置
 * @sa invTransform()
 * \endif
 */
QPointF QwtScaleMap::transform(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QPointF& pos)
{
    return QPointF(xMap.transform(pos.x()), yMap.transform(pos.y()));
}

/**
 * \if ENGLISH
 * @brief Transform a rectangle from paint to scale coordinates
 * @param xMap X axis scale map
 * @param yMap Y axis scale map
 * @param rect Rectangle in paint coordinates
 * @return Rectangle in scale coordinates
 * @sa transform()
 * \endif
 * \if CHINESE
 * @brief 将矩形从绘制坐标转换为刻度坐标
 * @param xMap X 轴刻度映射
 * @param yMap Y 轴刻度映射
 * @param rect 绘制坐标系中的矩形
 * @return 刻度坐标系中的矩形
 * @sa transform()
 * \endif
 */
QRectF QwtScaleMap::invTransform(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect)
{
    const double x1 = xMap.invTransform(rect.left());
    const double x2 = xMap.invTransform(rect.right() - 1);
    const double y1 = yMap.invTransform(rect.top());
    const double y2 = yMap.invTransform(rect.bottom() - 1);

    const QRectF r(x1, y1, x2 - x1, y2 - y1);
    return r.normalized();
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug debug, const QwtScaleMap& map)
{
    debug.nospace() << "QwtScaleMap(" << map.transformation() << ", s:" << map.s1() << "->" << map.s2()
                    << ", p:" << map.p1() << "->" << map.p2() << ")";

    return debug.space();
}

#endif
