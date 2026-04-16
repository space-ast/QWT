/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *****************************************************************************/

#include "qwt_box_statistics.h"
#include "qwt_math.h"

#include <algorithm>

/**
 * \if ENGLISH
 * @brief Constructor with default Tukey method
 * @details Initializes the calculator with Tukey whisker method and coefficient 1.5.
 * \endif
 * \if CHINESE
 * @brief 默认使用 Tukey 方法的构造函数
 * @details 初始化计算器，使用 Tukey 须须方法和系数 1.5。
 * \endif
 */
QwtBoxStatisticsCalculator::QwtBoxStatisticsCalculator()
    : m_method(Tukey)
    , m_coefficient(1.5)
{
}

/**
 * \if ENGLISH
 * @brief Set the whisker calculation method
 * @param[in] method The whisker method to use (Tukey, Percentile, MinMax, StandardDeviation, StandardError)
 * \endif
 * \if CHINESE
 * @brief 设置须须计算方法
 * @param[in] method 要使用的须须方法（Tukey、Percentile、MinMax、StandardDeviation、StandardError）
 * \endif
 */
void QwtBoxStatisticsCalculator::setWhiskerMethod(WhiskerMethod method)
{
    m_method = method;
}

/**
 * \if ENGLISH
 * @brief Get the whisker calculation method
 * @return Current whisker method
 * \endif
 * \if CHINESE
 * @brief 获取须须计算方法
 * @return 当前须须方法
 * \endif
 */
QwtBoxStatisticsCalculator::WhiskerMethod QwtBoxStatisticsCalculator::whiskerMethod() const
{
    return m_method;
}

/**
 * \if ENGLISH
 * @brief Set the whisker coefficient
 * @param[in] coeff Coefficient value (1.5 for Tukey method, percentile value for Percentile method)
 * \endif
 * \if CHINESE
 * @brief 设置须须系数
 * @param[in] coeff 系数值（Tukey 方法为 1.5，Percentile 方法为百分位值）
 * \endif
 */
void QwtBoxStatisticsCalculator::setWhiskerCoefficient(double coeff)
{
    m_coefficient = coeff;
}

/**
 * \if ENGLISH
 * @brief Get the whisker coefficient
 * @return Current whisker coefficient
 * \endif
 * \if CHINESE
 * @brief 获取须须系数
 * @return 当前须须系数
 * \endif
 */
double QwtBoxStatisticsCalculator::whiskerCoefficient() const
{
    return m_coefficient;
}

QVector<double> QwtBoxStatisticsCalculator::sortData(const QVector<double>& data)
{
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}

double QwtBoxStatisticsCalculator::quantile(const QVector<double>& sorted, double p)
{
    if (sorted.isEmpty())
        return 0.0;
    
    const int n = sorted.size();
    const double index = p * (n - 1);
    const int lower = static_cast<int>(std::floor(index));
    const int upper = static_cast<int>(std::ceil(index));
    
    if (lower == upper)
        return sorted[lower];
    
    // Linear interpolation
    const double frac = index - lower;
    return sorted[lower] + frac * (sorted[upper] - sorted[lower]);
}

double QwtBoxStatisticsCalculator::median(const QVector<double>& sorted)
{
    return quantile(sorted, 0.5);
}

double QwtBoxStatisticsCalculator::iqr(const QVector<double>& sorted)
{
    return quantile(sorted, 0.75) - quantile(sorted, 0.25);
}

double QwtBoxStatisticsCalculator::mean(const QVector<double>& data)
{
    if (data.isEmpty())
        return 0.0;
    
    double sum = 0.0;
    for (int i = 0; i < data.size(); ++i)
        sum += data[i];
    
    return sum / data.size();
}

double QwtBoxStatisticsCalculator::standardDeviation(const QVector<double>& data)
{
    if (data.size() < 2)
        return 0.0;
    
    const double m = mean(data);
    double sumSq = 0.0;
    for (int i = 0; i < data.size(); ++i)
    {
        const double diff = data[i] - m;
        sumSq += diff * diff;
    }
    
    return std::sqrt(sumSq / (data.size() - 1));
}

/**
 * \if ENGLISH
 * @brief Compute boxplot statistics from sorted data
 * @details Calculates whisker bounds, quartiles, median, and outlier count
 *          using the specified whisker method.
 * @param[in] position The position value for the resulting sample
 * @param[in] sortedData Pre-sorted data vector
 * @param[in] method Whisker calculation method (default: Tukey)
 * @param[in] coefficient Whisker coefficient (default: 1.5)
 * @return QwtBoxSample containing computed statistics
 * \endif
 * \if CHINESE
 * @brief 从已排序数据计算箱线图统计量
 * @details 使用指定的须须方法计算须须边界、四分位数、中位数和异常值计数。
 * @param[in] position 结果样本的位置值
 * @param[in] sortedData 已排序的数据向量
 * @param[in] method 须须计算方法（默认：Tukey）
 * @param[in] coefficient 须须系数（默认：1.5）
 * @return 包含计算统计量的 QwtBoxSample
 * \endif
 */
QwtBoxSample QwtBoxStatisticsCalculator::calculate(
    double position,
    const QVector<double>& sortedData,
    WhiskerMethod method,
    double coefficient)
{
    if (sortedData.isEmpty())
        return QwtBoxSample(position);
    
    const double minVal = sortedData.first();
    const double maxVal = sortedData.last();
    const double med = median(sortedData);
    const double q1 = quantile(sortedData, 0.25);
    const double q3 = quantile(sortedData, 0.75);
    
    double whiskerLower, whiskerUpper;
    
    switch (method)
    {
        case Tukey:
        {
            const double iqrVal = q3 - q1;
            whiskerLower = qwtMaxF(minVal, q1 - coefficient * iqrVal);
            whiskerUpper = qwtMinF(maxVal, q3 + coefficient * iqrVal);
            break;
        }
        case Percentile:
        {
            whiskerLower = quantile(sortedData, 1.0 - coefficient / 100.0);
            whiskerUpper = quantile(sortedData, coefficient / 100.0);
            break;
        }
        case MinMax:
        {
            whiskerLower = minVal;
            whiskerUpper = maxVal;
            break;
        }
        case StandardDeviation:
        {
            const double m = mean(sortedData);
            const double sd = standardDeviation(sortedData);
            whiskerLower = qwtMaxF(minVal, m - coefficient * sd);
            whiskerUpper = qwtMinF(maxVal, m + coefficient * sd);
            break;
        }
        case StandardError:
        {
            const double m = mean(sortedData);
            const double sd = standardDeviation(sortedData);
            const double se = sd / std::sqrt(static_cast<double>(sortedData.size()));
            whiskerLower = qwtMaxF(minVal, m - coefficient * se);
            whiskerUpper = qwtMinF(maxVal, m + coefficient * se);
            break;
        }
        default:
        {
            whiskerLower = minVal;
            whiskerUpper = maxVal;
            break;
        }
    }
    
    QwtBoxSample sample(position, whiskerLower, q1, med, q3, whiskerUpper);
    
    // Count outliers
    int outlierCount = 0;
    for (int i = 0; i < sortedData.size(); ++i)
    {
        if (sortedData[i] < whiskerLower || sortedData[i] > whiskerUpper)
            outlierCount++;
    }
    sample.outlierCount = outlierCount;
    
    return sample;
}

/**
 * \if ENGLISH
 * @brief Compute boxplot statistics from unsorted raw data
 * @details Sorts the data internally and then calculates statistics.
 * @param[in] position The position value for the resulting sample
 * @param[in] rawData Unsorted raw data vector
 * @param[in] method Whisker calculation method (default: Tukey)
 * @param[in] coefficient Whisker coefficient (default: 1.5)
 * @return QwtBoxSample containing computed statistics
 * \endif
 * \if CHINESE
 * @brief 从未排序的原始数据计算箱线图统计量
 * @details 内部对数据进行排序后计算统计量。
 * @param[in] position 结果样本的位置值
 * @param[in] rawData 未排序的原始数据向量
 * @param[in] method 须须计算方法（默认：Tukey）
 * @param[in] coefficient 须须系数（默认：1.5）
 * @return 包含计算统计量的 QwtBoxSample
 * \endif
 */
QwtBoxSample QwtBoxStatisticsCalculator::calculateFromRaw(
    double position,
    const QVector<double>& rawData,
    WhiskerMethod method,
    double coefficient)
{
    QVector<double> sorted = sortData(rawData);
    return calculate(position, sorted, method, coefficient);
}

/**
 * \if ENGLISH
 * @brief Extract outlier values from sorted data based on box sample
 * @details Identifies all values outside the whisker bounds as outliers.
 * @param[in] sample The box sample containing whisker bounds
 * @param[in] sortedData Pre-sorted data vector
 * @return Vector of outlier values
 * \endif
 * \if CHINESE
 * @brief 根据箱线图样本从已排序数据中提取异常值
 * @details 将所有位于须须边界之外的值识别为异常值。
 * @param[in] sample 包含须须边界的箱线图样本
 * @param[in] sortedData 已排序的数据向量
 * @return 异常值向量
 * \endif
 */
QVector<double> QwtBoxStatisticsCalculator::extractOutliers(
    const QwtBoxSample& sample,
    const QVector<double>& sortedData)
{
    QVector<double> outliers;
    
    for (int i = 0; i < sortedData.size(); ++i)
    {
        const double val = sortedData[i];
        if (val < sample.whiskerLower || val > sample.whiskerUpper)
            outliers.append(val);
    }
    
    return outliers;
}

/**
 * \if ENGLISH
 * @brief Compute full boxplot statistics including outliers
 * @details Calculates both the box sample and extracts outlier values in one call.
 * @param[in] position The position value for the resulting samples
 * @param[in] rawData Unsorted raw data vector
 * @param[out] sample Output parameter for computed box statistics
 * @param[out] outliers Output parameter for extracted outlier values
 * @param[in] method Whisker calculation method (default: Tukey)
 * @param[in] coefficient Whisker coefficient (default: 1.5)
 * \endif
 * \if CHINESE
 * @brief 计算完整的箱线图统计量，包括异常值
 * @details 在一次调用中同时计算箱线图样本并提取异常值。
 * @param[in] position 结果样本的位置值
 * @param[in] rawData 未排序的原始数据向量
 * @param[out] sample 输出参数，用于存储计算的箱线图统计量
 * @param[out] outliers 输出参数，用于存储提取的异常值
 * @param[in] method 须须计算方法（默认：Tukey）
 * @param[in] coefficient 须须系数（默认：1.5）
 * \endif
 */
void QwtBoxStatisticsCalculator::calculateFull(
    double position,
    const QVector<double>& rawData,
    QwtBoxSample& sample,
    QwtBoxOutlierSample& outliers,
    WhiskerMethod method,
    double coefficient)
{
    QVector<double> sorted = sortData(rawData);
    sample = calculate(position, sorted, method, coefficient);
    
    QVector<double> outlierValues = extractOutliers(sample, sorted);
    outliers = QwtBoxOutlierSample(position, std::move(outlierValues));
}