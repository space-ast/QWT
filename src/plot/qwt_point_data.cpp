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

#include "qwt_point_data.h"

/**
 * @brief Constructor
 *
 * @param[in] size Number of points
 * @param[in] interval Bounding interval for the points
 *
 * @sa setInterval(), setSize()
 */
QwtSyntheticPointData::QwtSyntheticPointData(size_t size, const QwtInterval& interval)
    : m_size(size), m_interval(interval)
{
}

/**
 * @brief Change the number of points
 *
 * @param[in] size Number of points
 *
 * @sa size(), setInterval()
 */
void QwtSyntheticPointData::setSize(size_t size)
{
    m_size = size;
}

/**
 * @brief Get the number of points
 *
 * @return Number of points
 *
 * @sa setSize(), interval()
 */
size_t QwtSyntheticPointData::size() const
{
    return m_size;
}

/**
 * @brief Set the bounding interval
 *
 * @param[in] interval Interval
 *
 * @sa interval(), setSize()
 */
void QwtSyntheticPointData::setInterval(const QwtInterval& interval)
{
    m_interval = interval.normalized();
}

/**
 * @brief Get the bounding interval
 *
 * @return Bounding interval
 *
 * @sa setInterval(), size()
 */
QwtInterval QwtSyntheticPointData::interval() const
{
    return m_interval;
}

/**
 * @brief Set the "rectangle of interest"
 *
 * @details QwtPlotSeriesItem defines the current area of the plot canvas
 *          as "rect of interest" ( QwtPlotSeriesItem::updateScaleDiv() ).
 *          If interval().isValid() == false the x values are calculated
 *          in the interval rect.left() -> rect.right().
 *
 * @param[in] rect Rectangle of interest
 *
 * @sa rectOfInterest()
 */
void QwtSyntheticPointData::setRectOfInterest(const QRectF& rect)
{
    m_rectOfInterest     = rect;
    m_intervalOfInterest = QwtInterval(rect.left(), rect.right()).normalized();
}

/**
 * @brief Get the "rectangle of interest"
 *
 * @return Rectangle of interest
 *
 * @sa setRectOfInterest()
 */
QRectF QwtSyntheticPointData::rectOfInterest() const
{
    return m_rectOfInterest;
}

/**
 * @brief Calculate the bounding rectangle
 *
 * @details This implementation iterates over all points, which could often
 *          be implemented much faster using the characteristics of the series.
 *          When there are many points it is recommended to overload and
 *          reimplement this method using the characteristics of the series
 *          (if possible).
 *
 * @return Bounding rectangle
 */
QRectF QwtSyntheticPointData::boundingRect() const
{
    if (m_size == 0 || !(m_interval.isValid() || m_intervalOfInterest.isValid())) {
        return QRectF(1.0, 1.0, -2.0, -2.0);  // something invalid
    }

    return qwtBoundingRect(*this);
}

/**
 * @brief Calculate the point from an index
 *
 * @param[in] index Index
 *
 * @return QPointF(x(index), y(x(index)))
 *
 * @warning For invalid indices (index < 0 || index >= size()) (0, 0) is returned.
 */
QPointF QwtSyntheticPointData::sample(size_t index) const
{
    if (index >= m_size)
        return QPointF(0, 0);

    const double xValue = x(index);
    const double yValue = y(xValue);

    return QPointF(xValue, yValue);
}

/**
 * @brief Calculate a x-value from an index
 *
 * @details x values are calculated by dividing an interval into
 *          equidistant steps. If !interval().isValid() the
 *          interval is calculated from the "rectangle of interest".
 *
 * @param[in] index Index of the requested point
 *
 * @return Calculated x coordinate
 *
 * @sa interval(), rectOfInterest(), y()
 */
double QwtSyntheticPointData::x(size_t index) const
{
    const QwtInterval& interval = m_interval.isValid() ? m_interval : m_intervalOfInterest;

    if (!interval.isValid())
        return 0.0;

    if (m_size <= 1)
        return interval.minValue();

    const double dx = interval.width() / (m_size - 1);
    return interval.minValue() + index * dx;
}
