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

#include "qwt_series_data.h"
#include "qwt_point_polar.h"
#include "qwt_math.h"

bool isSampleNanOrInf(const QPointF& sample)
{
    return qwt_is_nan_or_inf(sample);
}

bool isSampleNanOrInf(const QwtPoint3D& sample)
{
    return qwt_is_nan_or_inf(sample.x()) || qwt_is_nan_or_inf(sample.y()) || qwt_is_nan_or_inf(sample.z());
}

bool isSampleNanOrInf(const QwtPointPolar& sample)
{
    return qwt_is_nan_or_inf(sample.azimuth()) || qwt_is_nan_or_inf(sample.radius());
}

bool isSampleNanOrInf(const QwtSetSample& sample)
{
    // Check the position (value) field; set elements are checked inside qwtBoundingRect
    return qwt_is_nan_or_inf(sample.value);
}

bool isSampleNanOrInf(const QwtIntervalSample& sample)
{
    return qwt_is_nan_or_inf(sample.value) || qwt_is_nan_or_inf(sample.interval.minValue())
           || qwt_is_nan_or_inf(sample.interval.maxValue());
}

bool isSampleNanOrInf(const QwtOHLCSample& sample)
{
    return qwt_is_nan_or_inf(sample.open) || qwt_is_nan_or_inf(sample.high) || qwt_is_nan_or_inf(sample.low)
           || qwt_is_nan_or_inf(sample.close) || qwt_is_nan_or_inf(sample.time);
}

bool isSampleNanOrInf(const QwtVectorFieldSample& sample)
{
    return qwt_is_nan_or_inf(sample.x) || qwt_is_nan_or_inf(sample.y) || qwt_is_nan_or_inf(sample.vx)
           || qwt_is_nan_or_inf(sample.vy);
}

bool isSampleNanOrInf(const QwtBoxSample& sample)
{
    return qwt_is_nan_or_inf(sample.position) || qwt_is_nan_or_inf(sample.whiskerLower) || qwt_is_nan_or_inf(sample.q1)
           || qwt_is_nan_or_inf(sample.median) || qwt_is_nan_or_inf(sample.q3)
           || qwt_is_nan_or_inf(sample.whiskerUpper);
}

bool isSampleNanOrInf(const QwtBoxOutlierSample& sample)
{
    if (qwt_is_nan_or_inf(sample.boxPosition))
        return true;
    for (int i = 0; i < sample.values.size(); ++i) {
        if (qwt_is_nan_or_inf(sample.values[ i ]))
            return true;
    }
    return false;
}

static inline QRectF qwtBoundingRect(const QPointF& sample)
{
    return QRectF(sample.x(), sample.y(), 0.0, 0.0);
}

static inline QRectF qwtBoundingRect(const QwtPoint3D& sample)
{
    return QRectF(sample.x(), sample.y(), 0.0, 0.0);
}

static inline QRectF qwtBoundingRect(const QwtPointPolar& sample)
{
    return QRectF(sample.azimuth(), sample.radius(), 0.0, 0.0);
}

static inline QRectF qwtBoundingRect(const QwtIntervalSample& sample)
{
    return QRectF(sample.interval.minValue(), sample.value, sample.interval.maxValue() - sample.interval.minValue(), 0.0);
}

static inline QRectF qwtBoundingRect(const QwtSetSample& sample)
{
    if (sample.set.empty()) {
        return QRectF(sample.value, 0.0, 0.0, -1.0);
    }
    // Avoid first point being nan or inf
    int begin   = 0;
    double minY = sample.set[ begin ];
    double maxY = sample.set[ begin ];
    while (begin < sample.set.size() && qwt_is_nan_or_inf(minY)) {
        ++begin;
        if (begin < sample.set.size()) {
            minY = sample.set[ begin ];
            maxY = sample.set[ begin ];
        }
    }
    for (int i = begin + 1; i < sample.set.size(); ++i) {
        // modify by czy at 2025-12
        if (qwt_is_nan_or_inf(sample.set[ i ])) {
            continue;
        }
        if (sample.set[ i ] < minY)
            minY = sample.set[ i ];

        if (sample.set[ i ] > maxY)
            maxY = sample.set[ i ];
    }

    return QRectF(sample.value, minY, 0.0, maxY - minY);
}

static inline QRectF qwtBoundingRect(const QwtOHLCSample& sample)
{
    const QwtInterval interval = sample.boundingInterval();
    return QRectF(interval.minValue(), sample.time, interval.width(), 0.0);
}

static inline QRectF qwtBoundingRect(const QwtVectorFieldSample& sample)
{
    /*
        When displaying a sample as an arrow its length will be
        proportional to the magnitude - but not the same.
        As the factor between length and magnitude is not known
        we can't include vx/vy into the bounding rectangle.
     */

    return QRectF(sample.x, sample.y, 0, 0);
}

static inline QRectF qwtBoundingRect(const QwtBoxSample& sample)
{
    return QRectF(sample.position, sample.whiskerLower, 0.0, sample.whiskerUpper - sample.whiskerLower);
}

static inline QRectF qwtBoundingRect(const QwtBoxOutlierSample& sample)
{
    if (sample.values.isEmpty())
        return QRectF(sample.boxPosition, 0.0, 0.0, -1.0);  // invalid

    double minVal = 0.0;
    double maxVal = 0.0;
    bool found = false;
    for (int i = 0; i < sample.values.size(); ++i) {
        if (qwt_is_nan_or_inf(sample.values[ i ]))
            continue;
        if (!found) {
            minVal = sample.values[ i ];
            maxVal = sample.values[ i ];
            found = true;
        } else {
            if (sample.values[ i ] < minVal)
                minVal = sample.values[ i ];
            if (sample.values[ i ] > maxVal)
                maxVal = sample.values[ i ];
        }
    }
    if (!found)
        return QRectF(sample.boxPosition, 0.0, 0.0, -1.0);  // invalid

    return QRectF(sample.boxPosition, minVal, 0.0, maxVal - minVal);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */

template< class T >
QRectF qwtBoundingRectT(const QwtSeriesData< T >& series, size_t from, size_t to)
{
    QRectF boundingRect(1.0, 1.0, -2.0, -2.0);  // invalid;

    // Check for empty series first
    if (series.size() == 0) {
        return boundingRect;
    }

    if (to == 0) {
        to = series.size() - 1;
    }

    if (to < from) {
        return boundingRect;
    }

    size_t i;
    for (i = from; i <= to; i++) {
        // chenzongyan modify at 202512: add nan checking
        if (isSampleNanOrInf(series.sample(i))) {
            continue;
        }
        const QRectF rect = qwtBoundingRect(series.sample(i));
        if (rect.width() >= 0.0 && rect.height() >= 0.0) {
            boundingRect = rect;
            i++;
            break;
        }
    }

    for (; i <= to; i++) {
        // chenzongyan modify at 202512: add nan checking
        if (isSampleNanOrInf(series.sample(i))) {
            continue;
        }
        const QRectF rect = qwtBoundingRect(series.sample(i));
        if (rect.width() >= 0.0 && rect.height() >= 0.0) {
            boundingRect.setLeft(qMin(boundingRect.left(), rect.left()));
            boundingRect.setRight(qMax(boundingRect.right(), rect.right()));
            boundingRect.setTop(qMin(boundingRect.top(), rect.top()));
            boundingRect.setBottom(qMax(boundingRect.bottom(), rect.bottom()));
        }
    }

    return boundingRect;
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QPointF >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QPointF >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtPoint3D >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtPoint3D >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details The horizontal coordinates represent the azimuth, the vertical coordinates the radius.
 *
 *          Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtPointPolar >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtPointPolar >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtIntervalSample >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtIntervalSample >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtOHLCSample >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtOHLCSample >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtSetSample >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtSetSample >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtVectorFieldSample >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtVectorFieldSample >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtBoxSample >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtBoxSample >(series, from, to);
}

/**
 * @brief Calculate the bounding rectangle of a series subset
 * @details Slow implementation, that iterates over the series.
 *
 * @param series Series
 * @param from Index of the first sample, <= 0 means from the beginning
 * @param to Index of the last sample, < 0 means to the end
 *
 * @return Bounding rectangle
 */
QRectF qwtBoundingRect(const QwtSeriesData< QwtBoxOutlierSample >& series, size_t from, size_t to)
{
    return qwtBoundingRectT< QwtBoxOutlierSample >(series, from, to);
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtPointSeriesData::QwtPointSeriesData(const QVector< QPointF >& samples) : QwtArraySeriesData< QPointF >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtPointSeriesData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtPoint3DSeriesData::QwtPoint3DSeriesData(const QVector< QwtPoint3D >& samples)
    : QwtArraySeriesData< QwtPoint3D >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtPoint3DSeriesData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtIntervalSeriesData::QwtIntervalSeriesData(const QVector< QwtIntervalSample >& samples)
    : QwtArraySeriesData< QwtIntervalSample >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtIntervalSeriesData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtVectorFieldData::QwtVectorFieldData(const QVector< QwtVectorFieldSample >& samples)
    : QwtArraySeriesData< QwtVectorFieldSample >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtVectorFieldData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtSetSeriesData::QwtSetSeriesData(const QVector< QwtSetSample >& samples) : QwtArraySeriesData< QwtSetSample >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtSetSeriesData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtTradingChartData::QwtTradingChartData(const QVector< QwtOHLCSample >& samples)
    : QwtArraySeriesData< QwtOHLCSample >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtTradingChartData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtBoxChartData::QwtBoxChartData(const QVector< QwtBoxSample >& samples) : QwtArraySeriesData< QwtBoxSample >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtBoxChartData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Constructor
 * @param samples Samples
 */
QwtBoxOutlierChartData::QwtBoxOutlierChartData(const QVector< QwtBoxOutlierSample >& samples)
    : QwtArraySeriesData< QwtBoxOutlierSample >(samples)
{
}

/**
 * @brief Calculate the bounding rectangle
 * @details The bounding rectangle is calculated once by iterating over all
 *          points and is stored for all following requests.
 *
 * @return Bounding rectangle
 */
QRectF QwtBoxOutlierChartData::boundingRect() const
{
    if (cachedBoundingRect.width() < 0.0)
        cachedBoundingRect = qwtBoundingRect(*this);

    return cachedBoundingRect;
}

/**
 * @brief Get total outlier count across all boxes
 * @return Total count of all outlier values
 */
int QwtBoxOutlierChartData::totalOutlierCount() const
{
    int count = 0;
    for (size_t i = 0; i < size(); ++i)
        count += sample(i).count();
    return count;
}
