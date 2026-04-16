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

#ifndef QWT_INTERVAL_H
#define QWT_INTERVAL_H

#include "qwt_global.h"
#include <qmetatype.h>

/**
 * \if ENGLISH
 * @brief A class representing an interval
 * @details The interval is represented by 2 doubles, the lower and the upper limit.
 * \endif
 * \if CHINESE
 * @brief 表示区间的类
 * @details 区间由两个双精度数表示，下限和上限。
 * \endif
 */

class QWT_EXPORT QwtInterval
{
public:
    /*!
       \if ENGLISH
       \brief Flag indicating if a border is included or excluded
       \sa setBorderFlags(), borderFlags()
       \endif
       \if CHINESE
       \brief 标志位，指示边界是否包含或排除
       \sa setBorderFlags(), borderFlags()
       \endif
     */
    enum BorderFlag
    {
        //! \if ENGLISH Min/Max values are inside the interval \endif \if CHINESE 最小值/最大值在区间内 \endif
        IncludeBorders = 0x00,

        //! \if ENGLISH Min value is not included in the interval \endif \if CHINESE 最小值不包含在区间内 \endif
        ExcludeMinimum = 0x01,

        //! \if ENGLISH Max value is not included in the interval \endif \if CHINESE 最大值不包含在区间内 \endif
        ExcludeMaximum = 0x02,

        //! \if ENGLISH Min/Max values are not included in the interval \endif \if CHINESE 最小值/最大值都不包含在区间内 \endif
        ExcludeBorders = ExcludeMinimum | ExcludeMaximum
    };

    //! Border flags
    Q_DECLARE_FLAGS(BorderFlags, BorderFlag)

    QwtInterval();
    QwtInterval(double minValue, double maxValue, BorderFlags = IncludeBorders);

    // Assign the limits of the interval
    void setInterval(double minValue, double maxValue, BorderFlags = IncludeBorders);

    // Normalize the limits of the interval
    QwtInterval normalized() const;
    // Invert the interval
    QwtInterval inverted() const;
    // Limit the interval, keeping the border modes
    QwtInterval limited(double lowerBound, double upperBound) const;

    // Compare two intervals for equality
    bool operator==(const QwtInterval&) const;
    // Compare two intervals for inequality
    bool operator!=(const QwtInterval&) const;

    void setBorderFlags(BorderFlags);
    BorderFlags borderFlags() const;

    double minValue() const;
    double maxValue() const;
    double centerValue() const;

    double width() const;
    long double widthL() const;

    void setMinValue(double);
    void setMaxValue(double);

    // Test if a value is inside the interval
    bool contains(double value) const;
    // Test if an interval is inside the interval
    bool contains(const QwtInterval&) const;

    // Test if two intervals overlap
    bool intersects(const QwtInterval&) const;
    // Intersect two intervals
    QwtInterval intersect(const QwtInterval&) const;
    // Unite two intervals
    QwtInterval unite(const QwtInterval&) const;

    // Union of two intervals
    QwtInterval operator|(const QwtInterval&) const;
    // Intersection of two intervals
    QwtInterval operator&(const QwtInterval&) const;

    // Unite this interval with the given interval
    QwtInterval& operator|=(const QwtInterval&);
    // Intersect this interval with the given interval
    QwtInterval& operator&=(const QwtInterval&);

    // Extend the interval with a value
    QwtInterval extend(double value) const;
    // Extend an interval with a value
    QwtInterval operator|(double) const;
    // Extend an interval with a value
    QwtInterval& operator|=(double);

    bool isValid() const;
    bool isNull() const;
    void invalidate();

    // Symmetrize the interval around a value
    QwtInterval symmetrize(double value) const;

private:
    double m_minValue;
    double m_maxValue;
    BorderFlags m_borderFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtInterval::BorderFlags)
Q_DECLARE_METATYPE(QwtInterval)
Q_DECLARE_TYPEINFO(QwtInterval, Q_MOVABLE_TYPE);

/**
 * \if ENGLISH
 * @brief Default Constructor
 * @details Creates an invalid interval [0.0, -1.0]
 * \sa setInterval(), isValid()
 * \endif
 * \if CHINESE
 * @brief 默认构造函数
 * @details 创建一个无效区间 [0.0, -1.0]
 * \sa setInterval(), isValid()
 * \endif
 */
inline QwtInterval::QwtInterval() : m_minValue(0.0), m_maxValue(-1.0), m_borderFlags(IncludeBorders)
{
}

/**
 * \if ENGLISH
 * @brief Constructor with min/max values
 * @param minValue Minimum value
 * @param maxValue Maximum value
 * @param borderFlags Include/Exclude borders
 * \endif
 * \if CHINESE
 * @brief 带最小/最大值的构造函数
 * @param minValue 最小值
 * @param maxValue 最大值
 * @param borderFlags 包含/排除边界
 * \endif
 */
inline QwtInterval::QwtInterval(double minValue, double maxValue, BorderFlags borderFlags)
    : m_minValue(minValue), m_maxValue(maxValue), m_borderFlags(borderFlags)
{
}

/**
 * \if ENGLISH
 * @brief Assign the limits of the interval
 * @param minValue Minimum value
 * @param maxValue Maximum value
 * @param borderFlags Include/Exclude borders
 * \endif
 * \if CHINESE
 * @brief 设置区间的界限
 * @param minValue 最小值
 * @param maxValue 最大值
 * @param borderFlags 包含/排除边界
 * \endif
 */
inline void QwtInterval::setInterval(double minValue, double maxValue, BorderFlags borderFlags)
{
    m_minValue    = minValue;
    m_maxValue    = maxValue;
    m_borderFlags = borderFlags;
}

/**
 * \if ENGLISH
 * @brief Change the border flags
 * @param borderFlags Or'd BorderMode flags
 * \sa borderFlags()
 * \endif
 * \if CHINESE
 * @brief 更改边界标志
 * @param borderFlags 边界模式标志的或运算结果
 * \sa borderFlags()
 * \endif
 */
inline void QwtInterval::setBorderFlags(BorderFlags borderFlags)
{
    m_borderFlags = borderFlags;
}

/**
 * \if ENGLISH
 * @return Border flags
 * \sa setBorderFlags()
 * \endif
 * \if CHINESE
 * @return 边界标志
 * \sa setBorderFlags()
 * \endif
 */
inline QwtInterval::BorderFlags QwtInterval::borderFlags() const
{
    return m_borderFlags;
}

/**
 * \if ENGLISH
 * @brief Assign the lower limit of the interval
 * @param minValue Minimum value
 * \endif
 * \if CHINESE
 * @brief 设置区间的下限
 * @param minValue 最小值
 * \endif
 */
inline void QwtInterval::setMinValue(double minValue)
{
    m_minValue = minValue;
}

/**
 * \if ENGLISH
 * @brief Assign the upper limit of the interval
 * @param maxValue Maximum value
 * \endif
 * \if CHINESE
 * @brief 设置区间的上限
 * @param maxValue 最大值
 * \endif
 */
inline void QwtInterval::setMaxValue(double maxValue)
{
    m_maxValue = maxValue;
}

/**
 * \if ENGLISH
 * @return Minimum value of the interval
 * \sa setMinValue()
 * \endif
 * \if CHINESE
 * @return 区间的最小值
 * \sa setMinValue()
 * \endif
 */
inline double QwtInterval::minValue() const
{
    return m_minValue;
}

/**
 * \if ENGLISH
 * @return Maximum value of the interval
 * \sa setMaxValue()
 * \endif
 * \if CHINESE
 * @return 区间的最大值
 * \sa setMaxValue()
 * \endif
 */
inline double QwtInterval::maxValue() const
{
    return m_maxValue;
}

/**
 * \if ENGLISH
 * @return Center of the interval
 * \sa width()
 * \endif
 * \if CHINESE
 * @return 区间的中心
 * \sa width()
 * \endif
 */
inline double QwtInterval::centerValue() const
{
    return isValid() ? (m_minValue + (m_maxValue - m_minValue) * 0.5) : 0.0;
}
/**
 * \if ENGLISH
 * @brief Check if the interval is valid
 * @details A interval is valid when minValue() <= maxValue().
 * In case of QwtInterval::ExcludeBorders it is true
 * when minValue() < maxValue()
 * @return True, if the interval is valid
 * \sa isValid()
 * \endif
 * \if CHINESE
 * @brief 检查区间是否有效
 * @details 当 minValue() <= maxValue() 时区间有效。
 * 对于 QwtInterval::ExcludeBorders，当 minValue() < maxValue() 时为 true
 * @return 如果区间有效则为 true
 * \sa isValid()
 * \endif
 */
inline bool QwtInterval::isValid() const
{
    if ((m_borderFlags & ExcludeBorders) == 0)
        return m_minValue <= m_maxValue;
    else
        return m_minValue < m_maxValue;
}

/**
 * \if ENGLISH
 * @brief Return the width of an interval
 * @details The width of invalid intervals is 0.0, otherwise the result is maxValue() - minValue().
 * @return Interval width
 * \sa isValid()
 * \endif
 * \if CHINESE
 * @brief 返回区间的宽度
 * @details 无效区间的宽度为 0.0，否则结果为 maxValue() - minValue()。
 * @return 区间宽度
 * \sa isValid()
 * \endif
 */
inline double QwtInterval::width() const
{
    return isValid() ? (m_maxValue - m_minValue) : 0.0;
}

/**
 * \if ENGLISH
 * @brief Return the width of an interval as long double
 * @details The width of invalid intervals is 0.0, otherwise the result is
 *          maxValue() - minValue().
 * @return Interval width
 * @sa isValid()
 * \endif
 * \if CHINESE
 * @brief 以 long double 返回区间的宽度
 * @details 无效区间的宽度为 0.0，否则结果为 maxValue() - minValue()。
 * @return 区间宽度
 * @sa isValid()
 * \endif
 */
inline long double QwtInterval::widthL() const
{
    if (!isValid())
        return 0.0;

    return static_cast< long double >(m_maxValue) - static_cast< long double >(m_minValue);
}

/**
 * \if ENGLISH
 * @brief Intersection of two intervals
 * @param[in] other Interval to intersect with
 * @return Intersection of this and other
 * @sa intersect()
 * \endif
 * \if CHINESE
 * @brief 两个区间的交集
 * @param[in] other 要交集的区间
 * @return 本区间与 other 的交集
 * @sa intersect()
 * \endif
 */
inline QwtInterval QwtInterval::operator&(const QwtInterval& other) const
{
    return intersect(other);
}

/**
 * \if ENGLISH
 * @brief Union of two intervals
 * @param[in] other Interval to unite with
 * @return Union of this and other
 * @sa unite()
 * \endif
 * \if CHINESE
 * @brief 两个区间的并集
 * @param[in] other 要合并的区间
 * @return 本区间与 other 的并集
 * @sa unite()
 * \endif
 */
inline QwtInterval QwtInterval::operator|(const QwtInterval& other) const
{
    return unite(other);
}

/**
 * \if ENGLISH
 * @brief Compare two intervals
 * @param[in] other Interval to compare with
 * @return True when this and other are equal
 * \endif
 * \if CHINESE
 * @brief 比较两个区间
 * @param[in] other 要比较的区间
 * @return 如果本区间与 other 相等则返回 true
 * \endif
 */
inline bool QwtInterval::operator==(const QwtInterval& other) const
{
    return (m_minValue == other.m_minValue) && (m_maxValue == other.m_maxValue) && (m_borderFlags == other.m_borderFlags);
}

/**
 * \if ENGLISH
 * @brief Compare two intervals
 * @param[in] other Interval to compare with
 * @return True when this and other are not equal
 * \endif
 * \if CHINESE
 * @brief 比较两个区间
 * @param[in] other 要比较的区间
 * @return 如果本区间与 other 不相等则返回 true
 * \endif
 */
inline bool QwtInterval::operator!=(const QwtInterval& other) const
{
    return (!(*this == other));
}

/**
 * \if ENGLISH
 * @brief Extend an interval
 * @param[in] value Value to extend with
 * @return Extended interval
 * @sa extend()
 * \endif
 * \if CHINESE
 * @brief 扩展区间
 * @param[in] value 用于扩展的值
 * @return 扩展后的区间
 * @sa extend()
 * \endif
 */
inline QwtInterval QwtInterval::operator|(double value) const
{
    return extend(value);
}

/**
 * \if ENGLISH
 * @brief Check if interval is null
 * @return True if isValid() && (minValue() >= maxValue())
 * \endif
 * \if CHINESE
 * @brief 检查区间是否为空
 * @return 如果 isValid() && (minValue() >= maxValue()) 则返回 true
 * \endif
 */
inline bool QwtInterval::isNull() const
{
    return isValid() && m_minValue >= m_maxValue;
}

/**
 * \if ENGLISH
 * @brief Invalidate the interval
 * @details The limits are set to interval [0.0, -1.0]
 * @sa isValid()
 * \endif
 * \if CHINESE
 * @brief 使区间无效
 * @details 界限设置为区间 [0.0, -1.0]
 * @sa isValid()
 * \endif
 */
inline void QwtInterval::invalidate()
{
    m_minValue = 0.0;
    m_maxValue = -1.0;
}

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<(QDebug, const QwtInterval&);
#endif

#endif
