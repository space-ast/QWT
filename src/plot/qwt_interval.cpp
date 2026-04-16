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

#include "qwt_interval.h"

namespace
{
static const struct RegisterQwtInterval
{
    inline RegisterQwtInterval()
    {
        qRegisterMetaType< QwtInterval >();
    }

} qwtRegisterQwtInterval;
}

/**
 * \if ENGLISH
 * @brief Normalize the limits of the interval
 * @details If maxValue() < minValue() the limits will be inverted.
 * \endif
 * \if CHINESE
 * @brief 标准化区间的界限
 * @details 如果 maxValue() < minValue()，则界限将被反转。
 * \endif
 */
QwtInterval QwtInterval::normalized() const
{
    if (m_minValue > m_maxValue) {
        return inverted();
    }
    if (m_minValue == m_maxValue && m_borderFlags == ExcludeMinimum) {
        return inverted();
    }

    return *this;
}

/**
 * \if ENGLISH
 * @brief Invert the interval
 * @details minValue() and maxValue() are swapped, border flags remain unchanged.
 * \endif
 * \if CHINESE
 * @brief 反转区间
 * @details 交换 minValue() 和 maxValue()，边界标志保持不变。
 * \endif
 */
QwtInterval QwtInterval::inverted() const
{
    BorderFlags borderFlags = IncludeBorders;

    if (m_borderFlags & ExcludeMinimum)
        borderFlags |= ExcludeMaximum;

    if (m_borderFlags & ExcludeMaximum)
        borderFlags |= ExcludeMinimum;

    return QwtInterval(m_maxValue, m_minValue, borderFlags);
}

/**
 * \if ENGLISH
 * @brief Test if a value is inside an interval
 * @param[in] value Value to test
 * @return True if value lies inside the boundaries
 * \endif
 * \if CHINESE
 * @brief 测试值是否在区间内
 * @param[in] value 要测试的值
 * @return 如果值在边界内则返回 true
 * \endif
 */
bool QwtInterval::contains(double value) const
{
    if (!isValid())
        return false;

    if ((value < m_minValue) || (value > m_maxValue))
        return false;

    if ((value == m_minValue) && (m_borderFlags & ExcludeMinimum))
        return false;

    if ((value == m_maxValue) && (m_borderFlags & ExcludeMaximum))
        return false;

    return true;
}

/**
 * \if ENGLISH
 * @brief Test if an interval is inside an interval
 * @param[in] interval Interval to test
 * @return True if interval lies inside the boundaries
 * \endif
 * \if CHINESE
 * @brief 测试区间是否在另一个区间内
 * @param[in] interval 要测试的区间
 * @return 如果区间在边界内则返回 true
 * \endif
 */
bool QwtInterval::contains(const QwtInterval& interval) const
{
    if (!isValid() || !interval.isValid())
        return false;

    if ((interval.m_minValue < m_minValue) || (interval.m_maxValue > m_maxValue))
        return false;

    if (m_borderFlags) {
        if (interval.m_minValue == m_minValue) {
            if ((m_borderFlags & ExcludeMinimum) && !(interval.m_borderFlags & ExcludeMinimum)) {
                return false;
            }
        }

        if (interval.m_maxValue == m_maxValue) {
            if ((m_borderFlags & ExcludeMaximum) && !(interval.m_borderFlags & ExcludeMaximum)) {
                return false;
            }
        }
    }

    return true;
}

/**
 * \if ENGLISH
 * @brief Unite two intervals
 * @param[in] other Interval to unite with
 * @return Union of this and other
 * @sa intersect()
 * \endif
 * \if CHINESE
 * @brief 合并两个区间
 * @param[in] other 要合并的区间
 * @return 本区间与 other 的并集
 * @sa intersect()
 * \endif
 */
QwtInterval QwtInterval::unite(const QwtInterval& other) const
{
    /*
       If one of the intervals is invalid return the other one.
       If both are invalid return an invalid default interval
     */
    if (!isValid()) {
        if (!other.isValid())
            return QwtInterval();
        else
            return other;
    }
    if (!other.isValid())
        return *this;

    QwtInterval united;
    BorderFlags flags = IncludeBorders;

    // minimum
    if (m_minValue < other.minValue()) {
        united.setMinValue(m_minValue);
        flags &= m_borderFlags & ExcludeMinimum;
    } else if (other.minValue() < m_minValue) {
        united.setMinValue(other.minValue());
        flags &= other.borderFlags() & ExcludeMinimum;
    } else  // m_minValue == other.minValue()
    {
        united.setMinValue(m_minValue);
        flags &= (m_borderFlags & other.borderFlags()) & ExcludeMinimum;
    }

    // maximum
    if (m_maxValue > other.maxValue()) {
        united.setMaxValue(m_maxValue);
        flags &= m_borderFlags & ExcludeMaximum;
    } else if (other.maxValue() > m_maxValue) {
        united.setMaxValue(other.maxValue());
        flags &= other.borderFlags() & ExcludeMaximum;
    } else  // m_maxValue == other.maxValue() )
    {
        united.setMaxValue(m_maxValue);
        flags &= m_borderFlags & other.borderFlags() & ExcludeMaximum;
    }

    united.setBorderFlags(flags);
    return united;
}

/**
 * \if ENGLISH
 * @brief Intersect two intervals
 * @param[in] other Interval to intersect with
 * @return Intersection of this and other
 * \endif
 * \if CHINESE
 * @brief 交集两个区间
 * @param[in] other 要交集的区间
 * @return 本区间与 other 的交集
 * \endif
 */
QwtInterval QwtInterval::intersect(const QwtInterval& other) const
{
    if (!other.isValid() || !isValid())
        return QwtInterval();

    QwtInterval i1 = *this;
    QwtInterval i2 = other;

    // swap i1/i2, so that the minimum of i1
    // is smaller then the minimum of i2

    if (i1.minValue() > i2.minValue()) {
        qSwap(i1, i2);
    } else if (i1.minValue() == i2.minValue()) {
        if (i1.borderFlags() & ExcludeMinimum)
            qSwap(i1, i2);
    }

    if (i1.maxValue() < i2.minValue()) {
        return QwtInterval();
    }

    if (i1.maxValue() == i2.minValue()) {
        if (i1.borderFlags() & ExcludeMaximum || i2.borderFlags() & ExcludeMinimum) {
            return QwtInterval();
        }
    }

    QwtInterval intersected;
    BorderFlags flags = IncludeBorders;

    intersected.setMinValue(i2.minValue());
    flags |= i2.borderFlags() & ExcludeMinimum;

    if (i1.maxValue() < i2.maxValue()) {
        intersected.setMaxValue(i1.maxValue());
        flags |= i1.borderFlags() & ExcludeMaximum;
    } else if (i2.maxValue() < i1.maxValue()) {
        intersected.setMaxValue(i2.maxValue());
        flags |= i2.borderFlags() & ExcludeMaximum;
    } else  // i1.maxValue() == i2.maxValue()
    {
        intersected.setMaxValue(i1.maxValue());
        flags |= i1.borderFlags() & i2.borderFlags() & ExcludeMaximum;
    }

    intersected.setBorderFlags(flags);
    return intersected;
}

/**
 * \if ENGLISH
 * @brief Unite this interval with the given interval
 * @param[in] other Interval to be united with
 * @return Reference to this interval
 * \endif
 * \if CHINESE
 * @brief 将本区间与给定区间合并
 * @param[in] other 要合并的区间
 * @return 本区间的引用
 * \endif
 */
QwtInterval& QwtInterval::operator|=(const QwtInterval& other)
{
    *this = *this | other;
    return *this;
}

/**
 * \if ENGLISH
 * @brief Intersect this interval with the given interval
 * @param[in] other Interval to be intersected with
 * @return Reference to this interval
 * \endif
 * \if CHINESE
 * @brief 将本区间与给定区间取交集
 * @param[in] other 要交集的区间
 * @return 本区间的引用
 * \endif
 */
QwtInterval& QwtInterval::operator&=(const QwtInterval& other)
{
    *this = *this & other;
    return *this;
}

/**
 * \if ENGLISH
 * @brief Test if two intervals overlap
 * @param[in] other Interval to test
 * @return True when the intervals are intersecting
 * \endif
 * \if CHINESE
 * @brief 测试两个区间是否重叠
 * @param[in] other 要测试的区间
 * @return 如果区间相交则返回 true
 * \endif
 */
bool QwtInterval::intersects(const QwtInterval& other) const
{
    if (!isValid() || !other.isValid())
        return false;

    QwtInterval i1 = *this;
    QwtInterval i2 = other;

    // swap i1/i2, so that the minimum of i1
    // is smaller then the minimum of i2

    if (i1.minValue() > i2.minValue()) {
        qSwap(i1, i2);
    } else if (i1.minValue() == i2.minValue() && i1.borderFlags() & ExcludeMinimum) {
        qSwap(i1, i2);
    }

    if (i1.maxValue() > i2.minValue()) {
        return true;
    }
    if (i1.maxValue() == i2.minValue()) {
        return !((i1.borderFlags() & ExcludeMaximum) || (i2.borderFlags() & ExcludeMinimum));
    }
    return false;
}

/**
 * \if ENGLISH
 * @brief Symmetrize the interval around a value
 * @details Adjust the limit that is closer to value, so that value becomes
 *          the center of the interval.
 * @param[in] value Center value
 * @return Interval with value as center
 * \endif
 * \if CHINESE
 * @brief 使区间围绕某个值对称
 * @details 调整离值更近的界限，使值成为区间的中心。
 * @param[in] value 中心值
 * @return 以 value 为中心的区间
 * \endif
 */
QwtInterval QwtInterval::symmetrize(double value) const
{
    if (!isValid())
        return *this;

    const double delta = qMax(qAbs(value - m_maxValue), qAbs(value - m_minValue));

    return QwtInterval(value - delta, value + delta);
}

/**
 * \if ENGLISH
 * @brief Limit the interval, keeping the border modes
 * @param[in] lowerBound Lower limit
 * @param[in] upperBound Upper limit
 * @return Limited interval
 * \endif
 * \if CHINESE
 * @brief 限制区间，保持边界模式
 * @param[in] lowerBound 下限
 * @param[in] upperBound 上限
 * @return 限制后的区间
 * \endif
 */
QwtInterval QwtInterval::limited(double lowerBound, double upperBound) const
{
    if (!isValid() || lowerBound > upperBound)
        return QwtInterval();

    double minValue = qMax(m_minValue, lowerBound);
    minValue        = qMin(minValue, upperBound);

    double maxValue = qMax(m_maxValue, lowerBound);
    maxValue        = qMin(maxValue, upperBound);

    return QwtInterval(minValue, maxValue, m_borderFlags);
}

/**
 * \if ENGLISH
 * @brief Extend the interval
 * @details If value is below minValue(), value becomes the lower limit.
 *          If value is above maxValue(), value becomes the upper limit.
 *          extend() has no effect for invalid intervals.
 * @param[in] value Value to extend with
 * @return Extended interval
 * @sa isValid()
 * \endif
 * \if CHINESE
 * @brief 扩展区间
 * @details 如果 value 小于 minValue()，value 成为下限。
 *          如果 value 大于 maxValue()，value 成为上限。
 *          extend() 对无效区间没有影响。
 * @param[in] value 用于扩展的值
 * @return 扩展后的区间
 * @sa isValid()
 * \endif
 */
QwtInterval QwtInterval::extend(double value) const
{
    if (!isValid())
        return *this;

    return QwtInterval(qMin(value, m_minValue), qMax(value, m_maxValue), m_borderFlags);
}

/**
 * \if ENGLISH
 * @brief Extend an interval
 * @param[in] value Value to extend with
 * @return Reference to the extended interval
 * @sa extend()
 * \endif
 * \if CHINESE
 * @brief 扩展区间
 * @param[in] value 用于扩展的值
 * @return 扩展后区间的引用
 * @sa extend()
 * \endif
 */
QwtInterval& QwtInterval::operator|=(double value)
{
    *this = *this | value;
    return *this;
}

#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<(QDebug debug, const QwtInterval& interval)
{
    const int flags = interval.borderFlags();

    debug.nospace() << "QwtInterval(" << ((flags & QwtInterval::ExcludeMinimum) ? "]" : "[") << interval.minValue()
                    << "," << interval.maxValue() << ((flags & QwtInterval::ExcludeMaximum) ? "[" : "]") << ")";

    return debug.space();
}

#endif
