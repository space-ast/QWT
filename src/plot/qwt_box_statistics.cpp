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

QwtBoxStatisticsCalculator::QwtBoxStatisticsCalculator()
    : m_method(Tukey)
    , m_coefficient(1.5)
{
}

void QwtBoxStatisticsCalculator::setWhiskerMethod(WhiskerMethod method)
{
    m_method = method;
}

QwtBoxStatisticsCalculator::WhiskerMethod QwtBoxStatisticsCalculator::whiskerMethod() const
{
    return m_method;
}

void QwtBoxStatisticsCalculator::setWhiskerCoefficient(double coeff)
{
    m_coefficient = coeff;
}

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

QwtBoxSample QwtBoxStatisticsCalculator::calculateFromRaw(
    double position,
    const QVector<double>& rawData,
    WhiskerMethod method,
    double coefficient)
{
    QVector<double> sorted = sortData(rawData);
    return calculate(position, sorted, method, coefficient);
}

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