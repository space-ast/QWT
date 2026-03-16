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

#include "qwt_scale_div.h"
#include "qwt_interval.h"
#include "qwt_math.h"

/**
 * \if ENGLISH
 * @brief Construct a division without ticks
 *
 * @param lowerBound First boundary
 * @param upperBound Second boundary
 *
 * @note lowerBound might be greater than upperBound for inverted scales
 * \endif
 *
 * \if CHINESE
 * @brief 构造一个没有刻度的刻度划分
 *
 * @param lowerBound 第一个边界
 * @param upperBound 第二个边界
 *
 * @note 对于反向刻度，lowerBound 可能大于 upperBound
 * \endif
 */
QwtScaleDiv::QwtScaleDiv(double lowerBound, double upperBound) : m_lowerBound(lowerBound), m_upperBound(upperBound)
{
}

/**
 * \if ENGLISH
 * @brief Construct a scale division
 *
 * @param interval Interval
 * @param ticks List of major, medium and minor ticks
 * \endif
 *
 * \if CHINESE
 * @brief 构造一个刻度划分
 *
 * @param interval 间隔
 * @param ticks 主刻度、中刻度和次刻度列表
 * \endif
 */
QwtScaleDiv::QwtScaleDiv(const QwtInterval& interval, QList< double > ticks[ NTickTypes ])
    : m_lowerBound(interval.minValue()), m_upperBound(interval.maxValue())
{
    for (int i = 0; i < NTickTypes; i++)
        m_ticks[ i ] = ticks[ i ];
}

/**
 * \if ENGLISH
 * @brief Construct a scale division
 *
 * @param lowerBound First boundary
 * @param upperBound Second boundary
 * @param ticks List of major, medium and minor ticks
 *
 * @note lowerBound might be greater than upperBound for inverted scales
 * \endif
 *
 * \if CHINESE
 * @brief 构造一个刻度划分
 *
 * @param lowerBound 第一个边界
 * @param upperBound 第二个边界
 * @param ticks 主刻度、中刻度和次刻度列表
 *
 * @note 对于反向刻度，lowerBound 可能大于 upperBound
 * \endif
 */
QwtScaleDiv::QwtScaleDiv(double lowerBound, double upperBound, QList< double > ticks[ NTickTypes ])
    : m_lowerBound(lowerBound), m_upperBound(upperBound)
{
    for (int i = 0; i < NTickTypes; i++)
        m_ticks[ i ] = ticks[ i ];
}

/**
 * \if ENGLISH
 * @brief Construct a scale division
 *
 * @param lowerBound First boundary
 * @param upperBound Second boundary
 * @param minorTicks List of minor ticks
 * @param mediumTicks List of medium ticks
 * @param majorTicks List of major ticks
 *
 * @note lowerBound might be greater than upperBound for inverted scales
 * \endif
 *
 * \if CHINESE
 * @brief 构造一个刻度划分
 *
 * @param lowerBound 第一个边界
 * @param upperBound 第二个边界
 * @param minorTicks 次刻度列表
 * @param mediumTicks 中刻度列表
 * @param majorTicks 主刻度列表
 *
 * @note 对于反向刻度，lowerBound 可能大于 upperBound
 * \endif
 */
QwtScaleDiv::QwtScaleDiv(double lowerBound,
                         double upperBound,
                         const QList< double >& minorTicks,
                         const QList< double >& mediumTicks,
                         const QList< double >& majorTicks)
    : m_lowerBound(lowerBound), m_upperBound(upperBound)
{
    m_ticks[ MinorTick ]  = minorTicks;
    m_ticks[ MediumTick ] = mediumTicks;
    m_ticks[ MajorTick ]  = majorTicks;
}

/**
 * \if ENGLISH
 * @brief Change the interval
 *
 * @param lowerBound First boundary
 * @param upperBound Second boundary
 *
 * @note lowerBound might be greater than upperBound for inverted scales
 * \endif
 *
 * \if CHINESE
 * @brief 更改间隔
 *
 * @param lowerBound 第一个边界
 * @param upperBound 第二个边界
 *
 * @note 对于反向刻度，lowerBound 可能大于 upperBound
 * \endif
 */
void QwtScaleDiv::setInterval(double lowerBound, double upperBound)
{
    m_lowerBound = lowerBound;
    m_upperBound = upperBound;
}

/**
 * \if ENGLISH
 * @brief Change the interval
 *
 * @param interval Interval
 * \endif
 *
 * \if CHINESE
 * @brief 更改间隔
 *
 * @param interval 间隔
 * \endif
 */
void QwtScaleDiv::setInterval(const QwtInterval& interval)
{
    m_lowerBound = interval.minValue();
    m_upperBound = interval.maxValue();
}

/**
 * \if ENGLISH
 * @brief Get the interval from lowerBound to upperBound
 * @return Interval from lowerBound to upperBound
 * \endif
 *
 * \if CHINESE
 * @brief 获取从 lowerBound 到 upperBound 的间隔
 * @return 从 lowerBound 到 upperBound 的间隔
 * \endif
 */
QwtInterval QwtScaleDiv::interval() const
{
    return QwtInterval(m_lowerBound, m_upperBound);
}

/**
 * \if ENGLISH
 * @brief Set the first boundary
 *
 * @param lowerBound First boundary
 * @sa lowerBound(), setUpperBound()
 * \endif
 *
 * \if CHINESE
 * @brief 设置第一个边界
 *
 * @param lowerBound 第一个边界
 * @sa lowerBound(), setUpperBound()
 * \endif
 */
void QwtScaleDiv::setLowerBound(double lowerBound)
{
    m_lowerBound = lowerBound;
}

/**
 * \if ENGLISH
 * @brief Get the first boundary
 * @return First boundary
 * @sa upperBound()
 * \endif
 *
 * \if CHINESE
 * @brief 获取第一个边界
 * @return 第一个边界
 * @sa upperBound()
 * \endif
 */
double QwtScaleDiv::lowerBound() const
{
    return m_lowerBound;
}

/**
 * \if ENGLISH
 * @brief Set the second boundary
 *
 * @param upperBound Second boundary
 * @sa upperBound(), setLowerBound()
 * \endif
 *
 * \if CHINESE
 * @brief 设置第二个边界
 *
 * @param upperBound 第二个边界
 * @sa upperBound(), setLowerBound()
 * \endif
 */
void QwtScaleDiv::setUpperBound(double upperBound)
{
    m_upperBound = upperBound;
}

/**
 * \if ENGLISH
 * @brief Get the upper bound
 * @return Upper bound
 * @sa lowerBound()
 * \endif
 *
 * \if CHINESE
 * @brief 获取上边界
 * @return 上边界
 * @sa lowerBound()
 * \endif
 */
double QwtScaleDiv::upperBound() const
{
    return m_upperBound;
}

/**
 * \if ENGLISH
 * @brief Get the range (upperBound - lowerBound)
 * @return upperBound() - lowerBound()
 * \endif
 *
 * \if CHINESE
 * @brief 获取范围（上边界 - 下边界）
 * @return upperBound() - lowerBound()
 * \endif
 */
double QwtScaleDiv::range() const
{
    return m_upperBound - m_lowerBound;
}

/**
 * \if ENGLISH
 * @brief Equality operator
 * @return true if this instance is equal to other
 * \endif
 *
 * \if CHINESE
 * @brief 相等运算符
 * @return 如果此实例等于 other 则返回 true
 * \endif
 */
bool QwtScaleDiv::operator==(const QwtScaleDiv& other) const
{
    if (m_lowerBound != other.m_lowerBound || m_upperBound != other.m_upperBound) {
        return false;
    }

    for (int i = 0; i < NTickTypes; i++) {
        if (m_ticks[ i ] != other.m_ticks[ i ])
            return false;
    }

    return true;
}

/**
 * \if ENGLISH
 * @brief Inequality operator
 * @return true if this instance is not equal to other
 * \endif
 *
 * \if CHINESE
 * @brief 不等运算符
 * @return 如果此实例不等于 other 则返回 true
 * \endif
 */
bool QwtScaleDiv::operator!=(const QwtScaleDiv& other) const
{
    return (!(*this == other));
}

/**
 * \if ENGLISH
 * @brief Fuzzy comparison
 * @param other Other scale division
 * @return true if this instance is approximately equal to other
 * \endif
 *
 * \if CHINESE
 * @brief 模糊比较
 * @param other 其他刻度划分
 * @return 如果此实例近似等于 other 则返回 true
 * \endif
 */
bool QwtScaleDiv::fuzzyCompare(const QwtScaleDiv& other) const
{
    if (qFuzzyCompare(m_lowerBound, other.m_lowerBound) && qFuzzyCompare(m_upperBound, other.m_upperBound)) {
        for (int i = 0; i < NTickTypes; i++) {
            if (!fuzzyRangeEqual(
                    m_ticks[ i ].begin(), m_ticks[ i ].end(), other.m_ticks[ i ].begin(), other.m_ticks[ i ].end())) {
                return false;
            }
        }
        return true;
    }
    return false;
}

/**
 * \if ENGLISH
 * @brief Check if the scale division is empty
 * @return true if lowerBound() == upperBound()
 * \endif
 *
 * \if CHINESE
 * @brief 检查刻度划分是否为空
 * @return 如果 lowerBound() == upperBound() 则返回 true
 * \endif
 */
bool QwtScaleDiv::isEmpty() const
{
    return (m_lowerBound == m_upperBound);
}

/**
 * \if ENGLISH
 * @brief Check if the scale division is increasing
 * @return true if lowerBound() <= upperBound()
 * \endif
 *
 * \if CHINESE
 * @brief 检查刻度划分是否是递增的
 * @return 如果 lowerBound() <= upperBound() 则返回 true
 * \endif
 */
bool QwtScaleDiv::isIncreasing() const
{
    return m_lowerBound <= m_upperBound;
}

/**
 * \if ENGLISH
 * @brief Check if a value is between lowerBound() and upperBound()
 *
 * @param value Value
 * @return true if value is within bounds
 * \endif
 *
 * \if CHINESE
 * @brief 检查值是否在 lowerBound() 和 upperBound() 之间
 *
 * @param value 值
 * @return 如果值在范围内则返回 true
 * \endif
 */
bool QwtScaleDiv::contains(double value) const
{
    const double min = qMin(m_lowerBound, m_upperBound);
    const double max = qMax(m_lowerBound, m_upperBound);

    return value >= min && value <= max;
}

/**
 * \if ENGLISH
 * @brief Invert the scale division
 * @sa inverted()
 * \endif
 *
 * \if CHINESE
 * @brief 反转刻度划分
 * @sa inverted()
 * \endif
 */
void QwtScaleDiv::invert()
{
    qSwap(m_lowerBound, m_upperBound);

    for (int i = 0; i < NTickTypes; i++) {
        QList< double >& ticks = m_ticks[ i ];

        const int size  = ticks.count();
        const int size2 = size / 2;

        for (int j = 0; j < size2; j++)
            qSwap(ticks[ j ], ticks[ size - 1 - j ]);
    }
}

/**
 * \if ENGLISH
 * @brief Get a scale division with inverted boundaries and ticks
 * @return A scale division with inverted boundaries and ticks
 * @sa invert()
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界和刻度反转的刻度划分
 * @return 边界和刻度反转的刻度划分
 * @sa invert()
 * \endif
 */
QwtScaleDiv QwtScaleDiv::inverted() const
{
    QwtScaleDiv other = *this;
    other.invert();

    return other;
}

/**
 * \if ENGLISH
 * @brief Return a scale division with an interval [lowerBound, upperBound]
 *        where all ticks outside this interval are removed
 *
 * @param lowerBound Lower bound
 * @param upperBound Upper bound
 *
 * @return Scale division with all ticks inside of the given interval
 *
 * @note lowerBound might be greater than upperBound for inverted scales
 * \endif
 *
 * \if CHINESE
 * @brief 返回一个间隔为 [lowerBound, upperBound] 的刻度划分，
 *        其中所有在此间隔外的刻度都被移除
 *
 * @param lowerBound 下边界
 * @param upperBound 上边界
 *
 * @return 所有刻度都在给定间隔内的刻度划分
 *
 * @note 对于反向刻度，lowerBound 可能大于 upperBound
 * \endif
 */
QwtScaleDiv QwtScaleDiv::bounded(double lowerBound, double upperBound) const
{
    const double min = qMin(lowerBound, upperBound);
    const double max = qMax(lowerBound, upperBound);

    QwtScaleDiv sd;
    sd.setInterval(lowerBound, upperBound);

    for (int tickType = 0; tickType < QwtScaleDiv::NTickTypes; tickType++) {
        const QList< double >& ticks = m_ticks[ tickType ];

        QList< double > boundedTicks;
        for (int i = 0; i < ticks.size(); i++) {
            const double tick = ticks[ i ];
            if (tick >= min && tick <= max)
                boundedTicks += tick;
        }

        sd.setTicks(tickType, boundedTicks);
    }

    return sd;
}

/**
 * \if ENGLISH
 * @brief Assign ticks
 *
 * @param tickType MinorTick, MediumTick or MajorTick
 * @param ticks Values of the tick positions
 * \endif
 *
 * \if CHINESE
 * @brief 分配刻度
 *
 * @param tickType MinorTick、MediumTick 或 MajorTick
 * @param ticks 刻度位置的值
 * \endif
 */
void QwtScaleDiv::setTicks(int tickType, const QList< double >& ticks)
{
    if (tickType >= 0 && tickType < NTickTypes)
        m_ticks[ tickType ] = ticks;
}

/**
 * \if ENGLISH
 * @brief Return a list of ticks
 *
 * @param tickType MinorTick, MediumTick or MajorTick
 * @return Tick list
 * \endif
 *
 * \if CHINESE
 * @brief 返回刻度列表
 *
 * @param tickType MinorTick、MediumTick 或 MajorTick
 * @return 刻度列表
 * \endif
 */
QList< double > QwtScaleDiv::ticks(int tickType) const
{
    if (tickType >= 0 && tickType < NTickTypes)
        return m_ticks[ tickType ];

    return QList< double >();
}

#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<(QDebug debug, const QwtScaleDiv& scaleDiv)
{
    debug << scaleDiv.lowerBound() << "<->" << scaleDiv.upperBound();
    debug << "Major: " << scaleDiv.ticks(QwtScaleDiv::MajorTick);
    debug << "Medium: " << scaleDiv.ticks(QwtScaleDiv::MediumTick);
    debug << "Minor: " << scaleDiv.ticks(QwtScaleDiv::MinorTick);

    return debug;
}

#endif
