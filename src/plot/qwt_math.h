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

#ifndef QWT_MATH_H
#define QWT_MATH_H
// stl
#include <iterator>
#include <type_traits>
#include <algorithm>
// qt
#include <QPointF>
#include <QtMath>
// qwt
#include "qwt_global.h"
/*
   Microsoft says:

   Define _USE_MATH_DEFINES before including math.h to expose these macro
   definitions for common math constants.  These are placed under an #ifdef
   since these commonly-defined names are not part of the C/C++ standards.
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#define undef_USE_MATH_DEFINES
#endif

#include <cmath>

#ifdef undef_USE_MATH_DEFINES
#undef _USE_MATH_DEFINES
#undef undef_USE_MATH_DEFINES
#endif

#ifndef M_E
#define M_E (2.7182818284590452354)
#endif

#ifndef M_LOG2E
#define M_LOG2E (1.4426950408889634074)
#endif

#ifndef M_LOG10E
#define M_LOG10E (0.43429448190325182765)
#endif

#ifndef M_LN2
#define M_LN2 (0.69314718055994530942)
#endif

#ifndef M_LN10
#define M_LN10 (2.30258509299404568402)
#endif

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifndef M_PI_2
#define M_PI_2 (1.57079632679489661923)
#endif

#ifndef M_PI_4
#define M_PI_4 (0.78539816339744830962)
#endif

#ifndef M_1_PI
#define M_1_PI (0.31830988618379067154)
#endif

#ifndef M_2_PI
#define M_2_PI (0.63661977236758134308)
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI (1.12837916709551257390)
#endif

#ifndef M_SQRT2
#define M_SQRT2 (1.41421356237309504880)
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 (0.70710678118654752440)
#endif

#if defined(QT_WARNING_PUSH)
/*
    early Qt versions not having QT_WARNING_PUSH is full of warnings
    so that we do not care of suppressing those from below
 */
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wdouble-promotion")
QT_WARNING_DISABLE_GCC("-Wdouble-promotion")
#endif

/*
    On systems, where qreal is a float you often run into
    compiler issues with qMin/qMax.
 */

//! \return Minimum of a and b.
constexpr inline float qwtMinF(float a, float b)
{
    return (a < b) ? a : b;
}

//! \return Minimum of a and b.
constexpr inline double qwtMinF(double a, double b)
{
    return (a < b) ? a : b;
}

//! \return Minimum of a and b.
constexpr inline qreal qwtMinF(float a, double b)
{
    return (a < b) ? a : b;
}

//! \return Minimum of a and b.
constexpr inline qreal qwtMinF(double a, float b)
{
    return (a < b) ? a : b;
}

//! \return Maximum of a and b.
constexpr inline float qwtMaxF(float a, float b)
{
    return (a < b) ? b : a;
}

//! \return Maximum of a and b.
constexpr inline double qwtMaxF(double a, double b)
{
    return (a < b) ? b : a;
}

//! \return Maximum of a and b.
constexpr inline qreal qwtMaxF(float a, double b)
{
    return (a < b) ? b : a;
}

//! \return Maximum of a and b.
constexpr inline qreal qwtMaxF(double a, float b)
{
    return (a < b) ? b : a;
}

#if defined(QT_WARNING_POP)
QT_WARNING_POP
#endif

QWT_EXPORT double qwtNormalizeRadians(double radians);
QWT_EXPORT double qwtNormalizeDegrees(double degrees);
QWT_EXPORT quint32 qwtRand();

/*!
   \if ENGLISH
   \brief Compare 2 values, relative to an interval
   \details Values are "equal", when:
          value2 - value1 <= abs(intervalSize * 10e^{-6})
   \param value1 First value to compare
   \param value2 Second value to compare
   \param intervalSize interval size
   \return 0: if equal, -1: if value2 > value1, 1: if value1 > value2
   \endif
   \if CHINESE
   \brief 相对于区间比较两个值
   \details 当 value2 - value1 <= abs(intervalSize * 10e^{-6}) 时认为相等
   \param value1 第一个要比较的值
   \param value2 第二个要比较的值
   \param intervalSize 区间大小
   \return 0: 相等; -1: value2 > value1; 1: value1 > value2
   \endif
 */
inline int qwtFuzzyCompare(double value1, double value2, double intervalSize)
{
    const double eps = qAbs(1.0e-6 * intervalSize);

    if (value2 - value1 > eps)
        return -1;

    if (value1 - value2 > eps)
        return 1;

    return 0;
}

//! \if ENGLISH Return the sign \endif \if CHINESE 返回符号值 \endif
inline int qwtSign(double x)
{
    if (x > 0.0)
        return 1;
    else if (x < 0.0)
        return (-1);
    else
        return 0;
}

//! \if ENGLISH Return the square of a number \endif \if CHINESE 返回数字的平方 \endif
inline double qwtSqr(double x)
{
    return x * x;
}

//! \if ENGLISH Approximation of arc tangent (error below 0.005 radians) \endif \if CHINESE 反正切近似（误差小于 0.005 弧度） \endif
inline double qwtFastAtan(double x)
{
    if (x < -1.0)
        return -M_PI_2 - x / (x * x + 0.28);

    if (x > 1.0)
        return M_PI_2 - x / (x * x + 0.28);

    return x / (1.0 + x * x * 0.28);
}

//! \if ENGLISH Approximation of arc tangent 2 (error below 0.005 radians) \endif \if CHINESE 反正切2近似（误差小于 0.005 弧度） \endif
inline double qwtFastAtan2(double y, double x)
{
    if (x > 0)
        return qwtFastAtan(y / x);

    if (x < 0) {
        const double d = qwtFastAtan(y / x);
        return (y >= 0) ? d + M_PI : d - M_PI;
    }

    if (y < 0.0)
        return -M_PI_2;

    if (y > 0.0)
        return M_PI_2;

    return 0.0;
}

/**
 * \if ENGLISH
 * @brief Calculate a value of a cubic polynomial
 * @param[in] x Value
 * @param[in] a Cubic coefficient
 * @param[in] b Quadratic coefficient
 * @param[in] c Linear coefficient
 * @param[in] d Constant offset
 * @return Value of the polynomial for x
 * \endif
 * \if CHINESE
 * @brief 计算三次多项式的值
 * @param[in] x 值
 * @param[in] a 三次系数
 * @param[in] b 二次系数
 * @param[in] c 一次系数
 * @param[in] d 常数偏移
 * @return x 处多项式的值
 * \endif
 */
inline double qwtCubicPolynomial(double x, double a, double b, double c, double d)
{
    return (((a * x) + b) * x + c) * x + d;
}

//! \if ENGLISH Translate degrees into radians \endif \if CHINESE 将度数转换为弧度 \endif
inline double qwtRadians(double degrees)
{
    return degrees * M_PI / 180.0;
}

//! \if ENGLISH Translate radians into degrees \endif \if CHINESE 将弧度转换为度数 \endif
inline double qwtDegrees(double degrees)
{
    return degrees * 180.0 / M_PI;
}

/*!
    \if ENGLISH
    \brief The same as qCeil, but avoids including qmath.h
    \return Ceiling of value.
    \endif
    \if CHINESE
    \brief 与 qCeil 相同，但避免包含 qmath.h
    \return 值的上限（向上取整）。
    \endif
 */
inline int qwtCeil(qreal value)
{
    using std::ceil;
    return int(ceil(value));
}
/*!
    \if ENGLISH
    \brief The same as qFloor, but avoids including qmath.h
    \return Floor of value.
    \endif
    \if CHINESE
    \brief 与 qFloor 相同，但避免包含 qmath.h
    \return 值的下限（向下取整）。
    \endif
 */
inline int qwtFloor(qreal value)
{
    using std::floor;
    return int(floor(value));
}

/**
 * \if ENGLISH
 * @brief Verify and adjust array index range
 * @details Ensures the given index range is within valid bounds and returns the actual number of valid elements.
 *          This function automatically corrects invalid index values to ensure i1 <= i2.
 *          Typically used for array or container range checking to prevent out-of-bounds access.
 * @param size Total size of the array
 * @param i1 Start index (will be corrected to valid range)
 * @param i2 End index (will be corrected to valid range)
 * @return Number of valid elements in range, or 0 if array size is invalid
 * \endif
 * \if CHINESE
 * @brief 验证并调整数组索引范围
 * @details 确保给定的索引范围在有效范围内，并返回实际有效的元素个数。
 *          该函数会自动修正无效的索引值，确保 i1 <= i2。
 *          这个函数通常用于数组或容器的范围检查，确保访问不会越界。
 * @param size 数组的总大小
 * @param i1 起始索引（会被修正到有效范围内）
 * @param i2 结束索引（会被修正到有效范围内）
 * @return 返回有效范围内的元素个数，如果数组大小无效则返回0
 * \endif
 */
inline int qwtVerifyRange(int size, int& i1, int& i2)
{
    if (size < 1)
        return 0;

    i1 = qBound(0, i1, size - 1);
    i2 = qBound(0, i2, size - 1);

    if (i1 > i2)
        qSwap(i1, i2);

    return (i2 - i1 + 1);
}

/**
 * \if ENGLISH
 * @brief Calculate distance between two points
 * @param p1 First point
 * @param p2 Second point
 * @return Distance between p1 and p2
 * \endif
 * \if CHINESE
 * @brief 计算两点之间的距离
 * @param p1 第一个点
 * @param p2 第二个点
 * @return p1 和 p2 之间的距离
 * \endif
 */
inline double qwtDistance(const QPointF& p1, const QPointF& p2)
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return qSqrt(dx * dx + dy * dy);
}

/**
 * \if ENGLISH
 * @brief Check if floating point value is NaN or infinity
 * @details This function checks whether a floating point value is either NaN (Not a Number)
 *          or infinite (positive or negative infinity). It uses std::isfinite() for the check.
 * @tparam T Floating point type (float, double, long double)
 * @param value The value to check
 * @return true if value is NaN or infinite
 * @return false if value is finite
 * @note This overload is enabled only for floating point types
 * @see std::isfinite()
 * \endif
 * \if CHINESE
 * @brief 检查浮点数值是否为 NaN 或无穷大
 * @details 此函数检查浮点数值是否为 NaN（非数字）或无穷大（正无穷大或负无穷大）。
 *          它使用 std::isfinite() 进行检查。
 * @tparam T 浮点数类型（float, double, long double）
 * @param value 要检查的值
 * @return true 如果值是 NaN 或无穷大
 * @return false 如果值是有限数
 * @note 此重载仅对浮点数类型启用
 * @see std::isfinite()
 * \endif
 */
template< typename T >
inline typename std::enable_if< std::is_floating_point< T >::value, bool >::type qwt_is_nan_or_inf(const T& value)
{
    return !std::isfinite(value);
}

/**
 * \if ENGLISH
 * @brief Check if QPointF contains NaN or infinite coordinates
 * @details This function checks whether either the x or y coordinate of a QPointF
 *          is NaN (Not a Number) or infinite. Both coordinates are checked.
 * @param point The QPointF to check
 * @return true if either x or y coordinate is NaN or infinite
 * @return false if both coordinates are finite
 * \endif
 * \if CHINESE
 * @brief 检查 QPointF 点是否包含 NaN 或无穷大坐标
 * @details 此函数检查 QPointF 的 x 或 y 坐标是否为 NaN（非数字）或无穷大。
 *          两个坐标都会被检查。
 * @param point 要检查的 QPointF
 * @return true 如果 x 或 y 坐标是 NaN 或无穷大
 * @return false 如果两个坐标都是有限数
 * \endif
 */
inline bool qwt_is_nan_or_inf(const QPointF& point)
{
    return !std::isfinite(point.x()) || !std::isfinite(point.y());
}

// Default check function - for other types
/**
 * \if ENGLISH
 * @brief Default check function for non-floating point types
 * @details Always returns false for non-floating point types as they cannot be NaN or infinite.
 * \endif
 * \if CHINESE
 * @brief 非浮点类型的默认检查函数
 * @details 对于非浮点类型始终返回 false，因为它们不可能是 NaN 或无穷大。
 * \endif
 */
template< typename T >
typename std::enable_if< !std::is_floating_point< T >::value && !std::is_same< T, QPointF >::value,
                         bool >::type inline qwt_is_nan_or_inf(const T& /*value*/)
{
    return false;
}

/**
 * \if ENGLISH
 * @brief Check if iterator range contains NaN or Inf values
 * @details Checks whether elements in the iterator range [first, last) contain NaN or infinity values.
 *          Supports floating point types, QPointF, etc.
 * @tparam InputIt Input iterator type
 * @param first Start iterator of the range
 * @param last End iterator of the range
 * @return true if range contains at least one NaN or Inf value, false otherwise
 * \endif
 * \if CHINESE
 * @brief 检查指定迭代器范围内是否存在 NaN 或 Inf 值
 * @details 检查迭代器范围 [first, last) 内的元素是否包含 NaN 或无穷大值。
 *          支持浮点数类型、QPointF 等类型。
 * @tparam InputIt 输入迭代器类型
 * @param first 范围的起始迭代器
 * @param last 范围的结束迭代器
 * @return 如果范围内存在至少一个 NaN 或 Inf 值，返回 true；否则返回 false
 * \endif
 */
template< typename InputIt >
inline bool qwtContainsNanOrInf(InputIt first, InputIt last)
{
    // 使用迭代器遍历，对每个元素调用适当的 qwt_is_nan_or_inf 函数
    for (InputIt it = first; it != last; ++it) {
        if (qwt_is_nan_or_inf(*it)) {
            return true;
        }
    }
    return false;
}

/**
 * \if ENGLISH
 * @brief In-place clean array, removing NaN and Inf values
 * @details Uses std::remove_if algorithm to move elements to keep to the front of the container,
 *          then erases the unwanted elements.
 * @tparam Container Container type
 * @param container Container to clean (modified in place)
 * @return Number of removed elements
 * \endif
 * \if CHINESE
 * @brief 就地清理数组，移除 NaN 和 Inf 值
 * @details 使用 std::remove_if 算法将需要保留的元素移动到容器前部，
 *          然后删除不需要的元素。
 * @tparam Container 容器类型
 * @param container 要清理的容器（原地修改）
 * @return 删除的元素数量
 * \endif
 */
template< typename Container >
inline std::size_t qwtRemoveNanOrInf(Container& container)
{
    // 使用 std::remove_if 算法将需要保留的元素移动到容器前部
    auto new_end = std::remove_if(container.begin(), container.end(), [](const typename Container::value_type& value) {
        return qwt_is_nan_or_inf(value);
    });

    // 计算被删除的元素数量
    std::size_t removed_count = std::distance(new_end, container.end());

    // 实际删除不需要的元素
    if (removed_count > 0) {
        container.erase(new_end, container.end());
    }

    return removed_count;
}

/**
 * \if ENGLISH
 * @brief Remove all NaN or Inf values from container (returns new container)
 * @details Creates a new container with only finite values copied from the source.
 * @tparam Container Container type
 * @param container Container to process
 * @return New container without NaN or Inf values
 * \endif
 * \if CHINESE
 * @brief 从容器中删除所有 NaN 或 Inf 值（返回新容器）
 * @details 从源容器复制有限值创建一个新容器。
 * @tparam Container 容器类型
 * @param container 要处理的容器
 * @return 不包含 NaN 或 Inf 值的新容器
 * \endif
 */
template< typename Container >
inline Container qwtRemoveNanOrInfCopy(const Container& container)
{
    Container result;
    result.reserve(container.size());  // 预分配空间以提高效率

    // 只复制不是 NaN 或 Inf 的元素
    std::copy_if(container.begin(),
                 container.end(),
                 std::back_inserter(result),
                 [](const typename Container::value_type& value) { return !qwt_is_nan_or_inf(value); });

    return result;
}

/**
 * \if ENGLISH
 * @brief Compare two floating-point ranges for fuzzy equality using qFuzzyCompare
 * @details Works with any container as long as iterators meet ForwardIterator requirements.
 *          If the lengths differ it returns false immediately; otherwise it performs
 *          element-wise qFuzzyCompare and returns true only if all comparisons succeed.
 * @tparam It1 Type of the first range's iterator
 * @tparam It2 Type of the second range's iterator
 * @param[in] first1 Begin iterator of the first range
 * @param[in] last1 End iterator of the first range
 * @param[in] first2 Begin iterator of the second range
 * @param[in] last2 End iterator of the second range
 * @return true if ranges have equal length and all corresponding elements satisfy qFuzzyCompare
 * @return false if lengths differ or any element pair fails qFuzzyCompare
 * @note Element type must be acceptable to qFuzzyCompare (usually float/double)
 * @sa qFuzzyCompare
 * \endif
 * \if CHINESE
 * @brief 比较两个浮点区间是否"模糊相等"（使用 qFuzzyCompare）
 * @details 只要迭代器类别满足前向迭代器要求，即可用于任何容器（QVector、QList、
 *          std::vector、std::array、原始数组等）。区间长度不同立即返回 false；
 *          否则逐元素调用 qFuzzyCompare，全部通过返回 true。
 * @tparam It1 第一组迭代器类型
 * @tparam It2 第二组迭代器类型
 * @param[in] first1 第一组区间起始迭代器
 * @param[in] last1 第一组区间结束迭代器
 * @param[in] first2 第二组区间起始迭代器
 * @param[in] last2 第二组区间结束迭代器
 * @return true 长度相同且所有对应元素 qFuzzyCompare 返回 true
 * @return false 长度不同或任一元素 qFuzzyCompare 返回 false
 * @note 元素类型必须能被 qFuzzyCompare 接受（一般为 float/double）
 * @sa qFuzzyCompare
 * \endif
 */
template< typename It1, typename It2 >
bool fuzzyRangeEqual(It1 first1, It1 last1, It2 first2, It2 last2)
{
    // 1. 长度不同 => 不相等
    if (std::distance(first1, last1) != std::distance(first2, last2))
        return false;

    // 2. 逐元素 qFuzzyCompare
    for (; first1 != last1; ++first1, ++first2) {
        if (!qFuzzyCompare(*first1, *first2))
            return false;
    }
    return true;
}

#endif
