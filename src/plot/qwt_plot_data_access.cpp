/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_data_access.h"

#include <qwt_plot_item.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_barchart.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_tradingcurve.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_plot_vectorfield.h>
#include <qwt_plot_boxchart.h>
#include <qwt_samples.h>
#include <qwt_point_3d.h>

#include <qpainterpath.h>

// Helper: extract samples from a QwtSeriesStore<T>
template< typename T >
static QVector< T > extractSamples(const QwtSeriesStore< T >* store)
{
    QVector< T > result;
    if (!store)
        return result;

    const size_t size = store->dataSize();
    result.reserve(static_cast< int >(size));
    for (size_t i = 0; i < size; ++i)
        result.append(store->sample(i));

    return result;
}

/**
 * @brief Extract XY samples from a plot item
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @return Vector of QPointF samples, empty if item type is not supported
 */
QVector< QPointF > QwtPlotDataAccess::xySamples(const QwtPlotItem* item)
{
    if (!item)
        return QVector< QPointF >();

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve: {
        const auto* curve = static_cast< const QwtPlotCurve* >(item);
        return extractSamples(static_cast< const QwtSeriesStore< QPointF >* >(curve));
    }
    case QwtPlotItem::Rtti_PlotBarChart: {
        const auto* bar = static_cast< const QwtPlotBarChart* >(item);
        return extractSamples(static_cast< const QwtSeriesStore< QPointF >* >(bar));
    }
    default:
        return QVector< QPointF >();
    }
}

/**
 * @brief Extract XY samples into separate x and y vectors
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @param[out] x Output vector for x coordinates
 * @param[out] y Output vector for y coordinates
 */
void QwtPlotDataAccess::xySamples(const QwtPlotItem* item, QVector< double >& x, QVector< double >& y)
{
    const QVector< QPointF > pts = xySamples(item);
    x.clear();
    y.clear();
    x.reserve(pts.size());
    y.reserve(pts.size());
    for (const QPointF& p : pts) {
        x.append(p.x());
        y.append(p.y());
    }
}

/**
 * @brief Set XY samples on a plot item
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @param data Sample data to set
 * @return true if successful, false if item type is not supported
 */
bool QwtPlotDataAccess::setXYSamples(QwtPlotItem* item, const QVector< QPointF >& data)
{
    if (!item)
        return false;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve:
        static_cast< QwtPlotCurve* >(item)->setSamples(data);
        return true;
    case QwtPlotItem::Rtti_PlotBarChart:
        static_cast< QwtPlotBarChart* >(item)->setSamples(data);
        return true;
    default:
        return false;
    }
}

/**
 * @brief Set XY samples from separate x and y vectors
 * @param item Plot item (must be QwtPlotCurve)
 * @param x X coordinate vector
 * @param y Y coordinate vector
 * @return true if successful, false if item type is not supported
 * @note Only QwtPlotCurve supports separate x/y vectors; QwtPlotBarChart is not supported here
 */
bool QwtPlotDataAccess::setXYSamples(QwtPlotItem* item, const QVector< double >& x, const QVector< double >& y)
{
    if (!item || x.size() != y.size())
        return false;

    if (item->rtti() == QwtPlotItem::Rtti_PlotCurve) {
        static_cast< QwtPlotCurve* >(item)->setSamples(x, y);
        return true;
    }
    return false;
}

/**
 * @brief Extract interval samples from a plot item
 * @param item Plot item (must be QwtPlotIntervalCurve or QwtPlotHistogram)
 * @return Vector of QwtIntervalSample, empty if not supported
 */
QVector< QwtIntervalSample > QwtPlotDataAccess::intervalSamples(const QwtPlotItem* item)
{
    if (!item)
        return QVector< QwtIntervalSample >();

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotIntervalCurve: {
        const auto* curve = static_cast< const QwtPlotIntervalCurve* >(item);
        return extractSamples(static_cast< const QwtSeriesStore< QwtIntervalSample >* >(curve));
    }
    case QwtPlotItem::Rtti_PlotHistogram: {
        const auto* hist = static_cast< const QwtPlotHistogram* >(item);
        return extractSamples(static_cast< const QwtSeriesStore< QwtIntervalSample >* >(hist));
    }
    default:
        return QVector< QwtIntervalSample >();
    }
}

/**
 * @brief Set interval samples on a plot item
 * @param item Plot item (must be QwtPlotIntervalCurve or QwtPlotHistogram)
 * @param data Interval sample data
 * @return true if successful
 */
bool QwtPlotDataAccess::setIntervalSamples(QwtPlotItem* item, const QVector< QwtIntervalSample >& data)
{
    if (!item)
        return false;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        static_cast< QwtPlotIntervalCurve* >(item)->setSamples(data);
        return true;
    case QwtPlotItem::Rtti_PlotHistogram:
        static_cast< QwtPlotHistogram* >(item)->setSamples(data);
        return true;
    default:
        return false;
    }
}

/**
 * @brief Extract 3D point samples from a plot item
 * @param item Plot item (must be QwtPlotSpectroCurve)
 * @return Vector of QwtPoint3D samples
 */
QVector< QwtPoint3D > QwtPlotDataAccess::xyzSamples(const QwtPlotItem* item)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotSpectroCurve)
        return QVector< QwtPoint3D >();

    const auto* curve = static_cast< const QwtPlotSpectroCurve* >(item);
    return extractSamples(static_cast< const QwtSeriesStore< QwtPoint3D >* >(curve));
}

/**
 * @brief Set 3D point samples on a plot item
 * @param item Plot item (must be QwtPlotSpectroCurve)
 * @param data 3D point data
 * @return true if successful
 */
bool QwtPlotDataAccess::setXyzSamples(QwtPlotItem* item, const QVector< QwtPoint3D >& data)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotSpectroCurve)
        return false;

    static_cast< QwtPlotSpectroCurve* >(item)->setSamples(data);
    return true;
}

/**
 * @brief Extract OHLC samples from a plot item
 * @param item Plot item (must be QwtPlotTradingCurve)
 * @return Vector of QwtOHLCSample
 */
QVector< QwtOHLCSample > QwtPlotDataAccess::ohlcSamples(const QwtPlotItem* item)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotTradingCurve)
        return QVector< QwtOHLCSample >();

    const auto* curve = static_cast< const QwtPlotTradingCurve* >(item);
    return extractSamples(static_cast< const QwtSeriesStore< QwtOHLCSample >* >(curve));
}

/**
 * @brief Set OHLC samples on a plot item
 * @param item Plot item (must be QwtPlotTradingCurve)
 * @param data OHLC sample data
 * @return true if successful
 */
bool QwtPlotDataAccess::setOhlcSamples(QwtPlotItem* item, const QVector< QwtOHLCSample >& data)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotTradingCurve)
        return false;

    static_cast< QwtPlotTradingCurve* >(item)->setSamples(data);
    return true;
}

/**
 * @brief Extract set samples from a plot item
 * @param item Plot item (must be QwtPlotMultiBarChart)
 * @return Vector of QwtSetSample
 */
QVector< QwtSetSample > QwtPlotDataAccess::setSamples(const QwtPlotItem* item)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotMultiBarChart)
        return QVector< QwtSetSample >();

    const auto* bar = static_cast< const QwtPlotMultiBarChart* >(item);
    return extractSamples(static_cast< const QwtSeriesStore< QwtSetSample >* >(bar));
}

/**
 * @brief Set set samples on a plot item
 * @param item Plot item (must be QwtPlotMultiBarChart)
 * @param data Set sample data
 * @return true if successful
 */
bool QwtPlotDataAccess::setSetSamples(QwtPlotItem* item, const QVector< QwtSetSample >& data)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotMultiBarChart)
        return false;

    static_cast< QwtPlotMultiBarChart* >(item)->setSamples(data);
    return true;
}

/**
 * @brief Extract vector field samples from a plot item
 * @param item Plot item (must be QwtPlotVectorField)
 * @return Vector of QwtVectorFieldSample
 */
QVector< QwtVectorFieldSample > QwtPlotDataAccess::vectorFieldSamples(const QwtPlotItem* item)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotVectorField)
        return QVector< QwtVectorFieldSample >();

    const auto* field = static_cast< const QwtPlotVectorField* >(item);
    return extractSamples(static_cast< const QwtSeriesStore< QwtVectorFieldSample >* >(field));
}

/**
 * @brief Set vector field samples on a plot item
 * @param item Plot item (must be QwtPlotVectorField)
 * @param data Vector field sample data
 * @return true if successful
 */
bool QwtPlotDataAccess::setVectorFieldSamples(QwtPlotItem* item, const QVector< QwtVectorFieldSample >& data)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotVectorField)
        return false;

    static_cast< QwtPlotVectorField* >(item)->setSamples(data);
    return true;
}

/**
 * @brief Extract box samples from a plot item
 * @param item Plot item (must be QwtPlotBoxChart)
 * @return Vector of QwtBoxSample
 */
QVector< QwtBoxSample > QwtPlotDataAccess::boxSamples(const QwtPlotItem* item)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotBoxChart)
        return QVector< QwtBoxSample >();

    const auto* box = static_cast< const QwtPlotBoxChart* >(item);
    return extractSamples(static_cast< const QwtSeriesStore< QwtBoxSample >* >(box));
}

/**
 * @brief Set box samples on a plot item
 * @param item Plot item (must be QwtPlotBoxChart)
 * @param data Box sample data
 * @return true if successful
 */
bool QwtPlotDataAccess::setBoxSamples(QwtPlotItem* item, const QVector< QwtBoxSample >& data)
{
    if (!item || item->rtti() != QwtPlotItem::Rtti_PlotBoxChart)
        return false;

    static_cast< QwtPlotBoxChart* >(item)->setSamples(data);
    return true;
}

/**
 * @brief Extract XY samples within a rectangular range
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @param range Rectangular range in data coordinates
 * @return Filtered samples that fall within the range
 */
QVector< QPointF > QwtPlotDataAccess::xySamplesInRange(const QwtPlotItem* item, const QRectF& range)
{
    const QVector< QPointF > all = xySamples(item);
    QVector< QPointF > result;
    result.reserve(all.size());
    for (const QPointF& p : all) {
        if (range.contains(p))
            result.append(p);
    }
    return result;
}

/**
 * @brief Extract XY samples within a path range
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @param range Path range in data coordinates
 * @return Filtered samples that fall within the path
 */
QVector< QPointF > QwtPlotDataAccess::xySamplesInRange(const QwtPlotItem* item, const QPainterPath& range)
{
    const QVector< QPointF > all = xySamples(item);
    QVector< QPointF > result;
    result.reserve(all.size());
    for (const QPointF& p : all) {
        if (range.contains(p))
            result.append(p);
    }
    return result;
}

/**
 * @brief Remove XY samples within a rectangular range
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @param range Rectangular range in data coordinates
 * @return Number of samples removed
 */
int QwtPlotDataAccess::removeSamplesInRange(QwtPlotItem* item, const QRectF& range)
{
    if (!item)
        return 0;

    const QVector< QPointF > all = xySamples(item);
    QVector< QPointF > remaining;
    remaining.reserve(all.size());

    int removed = 0;
    for (const QPointF& p : all) {
        if (range.contains(p))
            ++removed;
        else
            remaining.append(p);
    }

    if (removed > 0)
        setXYSamples(item, remaining);

    return removed;
}

/**
 * @brief Remove XY samples within a path range
 * @param item Plot item (must be QwtPlotCurve or QwtPlotBarChart)
 * @param range Path range in data coordinates
 * @return Number of samples removed
 */
int QwtPlotDataAccess::removeSamplesInRange(QwtPlotItem* item, const QPainterPath& range)
{
    if (!item)
        return 0;

    const QVector< QPointF > all = xySamples(item);
    QVector< QPointF > remaining;
    remaining.reserve(all.size());

    int removed = 0;
    for (const QPointF& p : all) {
        if (range.contains(p))
            ++removed;
        else
            remaining.append(p);
    }

    if (removed > 0)
        setXYSamples(item, remaining);

    return removed;
}
