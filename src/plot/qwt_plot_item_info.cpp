/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_item_info.h"

#include <qwt_plot.h>
#include <qwt_plot_item.h>
#include <qwt_plot_seriesitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_barchart.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_tradingcurve.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_plot_vectorfield.h>
#include <qwt_plot_boxchart.h>
#include <qwt_series_store.h>

/**
 * @brief Check whether the item is a QwtPlotSeriesItem subclass
 * @param item Plot item to check, may be nullptr
 * @return true if item is derived from QwtPlotSeriesItem
 */
bool QwtPlotItemInfo::isSeriesItem(const QwtPlotItem* item)
{
    if (!item)
        return false;

    const int rtti = item->rtti();
    switch (rtti) {
    case QwtPlotItem::Rtti_PlotCurve:
    case QwtPlotItem::Rtti_PlotSpectroCurve:
    case QwtPlotItem::Rtti_PlotIntervalCurve:
    case QwtPlotItem::Rtti_PlotHistogram:
    case QwtPlotItem::Rtti_PlotTradingCurve:
    case QwtPlotItem::Rtti_PlotBarChart:
    case QwtPlotItem::Rtti_PlotMultiBarChart:
    case QwtPlotItem::Rtti_PlotVectorField:
    case QwtPlotItem::Rtti_PlotBoxChart:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check whether the item stores QPointF data
 * @param item Plot item to check
 * @return true for QwtPlotCurve and QwtPlotBarChart
 */
bool QwtPlotItemInfo::isXYSeriesItem(const QwtPlotItem* item)
{
    if (!item)
        return false;

    const int rtti = item->rtti();
    return rtti == QwtPlotItem::Rtti_PlotCurve || rtti == QwtPlotItem::Rtti_PlotBarChart;
}

/**
 * @brief Check whether the item stores QwtIntervalSample data
 * @param item Plot item to check
 * @return true for QwtPlotIntervalCurve and QwtPlotHistogram
 */
bool QwtPlotItemInfo::isIntervalSeriesItem(const QwtPlotItem* item)
{
    if (!item)
        return false;

    const int rtti = item->rtti();
    return rtti == QwtPlotItem::Rtti_PlotIntervalCurve || rtti == QwtPlotItem::Rtti_PlotHistogram;
}

/**
 * @brief Check whether the item is a decorator (non-data item)
 * @param item Plot item to check
 * @return true for grid, marker, scale, legend, text label, zone, shape, graphic, arrow marker
 */
bool QwtPlotItemInfo::isDecoratorItem(const QwtPlotItem* item)
{
    if (!item)
        return false;

    const int rtti = item->rtti();
    switch (rtti) {
    case QwtPlotItem::Rtti_PlotGrid:
    case QwtPlotItem::Rtti_PlotScale:
    case QwtPlotItem::Rtti_PlotLegend:
    case QwtPlotItem::Rtti_PlotMarker:
    case QwtPlotItem::Rtti_PlotGraphic:
    case QwtPlotItem::Rtti_PlotShape:
    case QwtPlotItem::Rtti_PlotTextLabel:
    case QwtPlotItem::Rtti_PlotZone:
    case QwtPlotItem::Rtti_PlotArrowMarker:
    case QwtPlotItem::Rtti_PlotSpectrogram:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check whether the item holds any data series
 * @param item Plot item to check
 * @return true for all QwtPlotSeriesItem subclasses
 * @sa isSeriesItem()
 */
bool QwtPlotItemInfo::isDataItem(const QwtPlotItem* item)
{
    return isSeriesItem(item);
}

/**
 * @brief Get all QwtPlotSeriesItem instances from a plot
 * @param plot Plot to query
 * @return List of series items
 */
QwtPlotItemList QwtPlotItemInfo::seriesItems(const QwtPlot* plot)
{
    if (!plot)
        return QwtPlotItemList();

    QwtPlotItemList result;
    const QwtPlotItemList& items = plot->itemList();
    for (QwtPlotItem* item : items) {
        if (isSeriesItem(item))
            result.append(item);
    }
    return result;
}

/**
 * @brief Get all QPointF-based series items from a plot
 * @param plot Plot to query
 * @return List of XY series items (curves and bar charts)
 */
QwtPlotItemList QwtPlotItemInfo::xySeriesItems(const QwtPlot* plot)
{
    if (!plot)
        return QwtPlotItemList();

    QwtPlotItemList result;
    const QwtPlotItemList& items = plot->itemList();
    for (QwtPlotItem* item : items) {
        if (isXYSeriesItem(item))
            result.append(item);
    }
    return result;
}

/**
 * @brief Filter items by a single rtti value
 * @param plot Plot to query
 * @param rtti Runtime type value to match
 * @return List of items matching the rtti
 * @sa QwtPlotDict::itemList(int)
 */
QwtPlotItemList QwtPlotItemInfo::filterByRtti(const QwtPlot* plot, int rtti)
{
    if (!plot)
        return QwtPlotItemList();

    return plot->itemList(rtti);
}

/**
 * @brief Filter items by a set of rtti values
 * @param plot Plot to query
 * @param rttiSet Set of rtti values to match
 * @return List of items whose rtti is in the set
 */
QwtPlotItemList QwtPlotItemInfo::filterByRtti(const QwtPlot* plot, const QSet< int >& rttiSet)
{
    if (!plot)
        return QwtPlotItemList();

    QwtPlotItemList result;
    const QwtPlotItemList& items = plot->itemList();
    for (QwtPlotItem* item : items) {
        if (rttiSet.contains(item->rtti()))
            result.append(item);
    }
    return result;
}

/**
 * @brief Get all currently visible items from a plot
 * @param plot Plot to query
 * @return List of visible items
 */
QwtPlotItemList QwtPlotItemInfo::visibleItems(const QwtPlot* plot)
{
    if (!plot)
        return QwtPlotItemList();

    QwtPlotItemList result;
    const QwtPlotItemList& items = plot->itemList();
    for (QwtPlotItem* item : items) {
        if (item->isVisible())
            result.append(item);
    }
    return result;
}

/**
 * @brief Get the number of data samples in a plot item
 * @param item Plot item to query
 * @return Number of samples, or -1 if the item does not hold a data series
 */
int QwtPlotItemInfo::dataSize(const QwtPlotItem* item)
{
    if (!item)
        return -1;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve:
        return static_cast< int >(static_cast< const QwtPlotCurve* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotBarChart:
        return static_cast< int >(static_cast< const QwtPlotBarChart* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotHistogram:
        return static_cast< int >(static_cast< const QwtPlotHistogram* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        return static_cast< int >(static_cast< const QwtPlotIntervalCurve* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotSpectroCurve:
        return static_cast< int >(static_cast< const QwtPlotSpectroCurve* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotTradingCurve:
        return static_cast< int >(static_cast< const QwtPlotTradingCurve* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotMultiBarChart:
        return static_cast< int >(static_cast< const QwtPlotMultiBarChart* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotVectorField:
        return static_cast< int >(static_cast< const QwtPlotVectorField* >(item)->dataSize());
    case QwtPlotItem::Rtti_PlotBoxChart:
        return static_cast< int >(static_cast< const QwtPlotBoxChart* >(item)->dataSize());
    default:
        return -1;
    }
}

/**
 * @brief Get the bounding rectangle of the item's data
 * @param item Plot item to query
 * @return Bounding rectangle, or an invalid QRectF if the item does not hold data
 */
QRectF QwtPlotItemInfo::dataRect(const QwtPlotItem* item)
{
    if (!item || !isSeriesItem(item))
        return QRectF();

    return item->boundingRect();
}

/**
 * @brief Get a human-readable name for an rtti value
 * @param rtti Runtime type value
 * @return String name such as "PlotCurve", "PlotBarChart", etc.
 */
QString QwtPlotItemInfo::rttiName(int rtti)
{
    switch (rtti) {
    case QwtPlotItem::Rtti_PlotItem:
        return QStringLiteral("PlotItem");
    case QwtPlotItem::Rtti_PlotGrid:
        return QStringLiteral("PlotGrid");
    case QwtPlotItem::Rtti_PlotScale:
        return QStringLiteral("PlotScale");
    case QwtPlotItem::Rtti_PlotLegend:
        return QStringLiteral("PlotLegend");
    case QwtPlotItem::Rtti_PlotMarker:
        return QStringLiteral("PlotMarker");
    case QwtPlotItem::Rtti_PlotCurve:
        return QStringLiteral("PlotCurve");
    case QwtPlotItem::Rtti_PlotSpectroCurve:
        return QStringLiteral("PlotSpectroCurve");
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        return QStringLiteral("PlotIntervalCurve");
    case QwtPlotItem::Rtti_PlotHistogram:
        return QStringLiteral("PlotHistogram");
    case QwtPlotItem::Rtti_PlotSpectrogram:
        return QStringLiteral("PlotSpectrogram");
    case QwtPlotItem::Rtti_PlotGraphic:
        return QStringLiteral("PlotGraphic");
    case QwtPlotItem::Rtti_PlotTradingCurve:
        return QStringLiteral("PlotTradingCurve");
    case QwtPlotItem::Rtti_PlotBarChart:
        return QStringLiteral("PlotBarChart");
    case QwtPlotItem::Rtti_PlotMultiBarChart:
        return QStringLiteral("PlotMultiBarChart");
    case QwtPlotItem::Rtti_PlotShape:
        return QStringLiteral("PlotShape");
    case QwtPlotItem::Rtti_PlotTextLabel:
        return QStringLiteral("PlotTextLabel");
    case QwtPlotItem::Rtti_PlotZone:
        return QStringLiteral("PlotZone");
    case QwtPlotItem::Rtti_PlotVectorField:
        return QStringLiteral("PlotVectorField");
    case QwtPlotItem::Rtti_PlotArrowMarker:
        return QStringLiteral("PlotArrowMarker");
    case QwtPlotItem::Rtti_PlotBoxChart:
        return QStringLiteral("PlotBoxChart");
    default:
        if (rtti >= QwtPlotItem::Rtti_PlotUserItem)
            return QStringLiteral("PlotUserItem(%1)").arg(rtti);
        return QStringLiteral("Unknown(%1)").arg(rtti);
    }
}
