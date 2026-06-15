/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_styling.h"

#include <qwt_plot.h>
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
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_symbol.h>
#include <qwt_column_symbol.h>

/**
 * @brief Get the representative color of a plot item
 * @param item Plot item to query
 * @param defaultColor Color to return if the item type is not recognized
 * @return The primary color of the item (from pen or brush)
 * @details Uses the item's rtti to dispatch to the correct type-specific API.
 *          For curves, returns the pen color; for bar charts, the brush color;
 *          for grids, the major pen color; etc.
 */
QColor QwtPlotStyling::color(const QwtPlotItem* item, const QColor& defaultColor)
{
    if (!item)
        return defaultColor;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve: {
        const auto* curve = static_cast< const QwtPlotCurve* >(item);
        return curve->pen().color();
    }
    case QwtPlotItem::Rtti_PlotBarChart: {
        const auto* bar = static_cast< const QwtPlotBarChart* >(item);
        return bar->brush().color();
    }
    case QwtPlotItem::Rtti_PlotMultiBarChart: {
        // QwtPlotMultiBarChart uses symbols per series, not a single pen
        return defaultColor;
    }
    case QwtPlotItem::Rtti_PlotHistogram: {
        const auto* hist = static_cast< const QwtPlotHistogram* >(item);
        return hist->brush().color();
    }
    case QwtPlotItem::Rtti_PlotIntervalCurve: {
        const auto* curve = static_cast< const QwtPlotIntervalCurve* >(item);
        return curve->brush().color();
    }
    case QwtPlotItem::Rtti_PlotTradingCurve: {
        const auto* curve = static_cast< const QwtPlotTradingCurve* >(item);
        return curve->symbolPen().color();
    }
    case QwtPlotItem::Rtti_PlotSpectroCurve: {
        // SpectroCurve uses color map, not a pen color
        return defaultColor;
    }
    case QwtPlotItem::Rtti_PlotGrid: {
        const auto* grid = static_cast< const QwtPlotGrid* >(item);
        return grid->majorPen().color();
    }
    case QwtPlotItem::Rtti_PlotMarker: {
        const auto* marker = static_cast< const QwtPlotMarker* >(item);
        return marker->linePen().color();
    }
    default:
        return defaultColor;
    }
}

/**
 * @brief Set the color of a plot item
 * @param item Plot item to modify
 * @param color New color to apply
 * @return true if the color was successfully applied
 * @details For curves, sets the pen color; for bar charts and histograms,
 *          sets both pen color and brush color; for grids, sets the major pen color.
 */
bool QwtPlotStyling::setColor(QwtPlotItem* item, const QColor& color)
{
    if (!item)
        return false;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve: {
        auto* curve = static_cast< QwtPlotCurve* >(item);
        QPen p = curve->pen();
        p.setColor(color);
        curve->setPen(p);
        return true;
    }
    case QwtPlotItem::Rtti_PlotBarChart: {
        auto* bar = static_cast< QwtPlotBarChart* >(item);
        QPen p = bar->pen();
        p.setColor(color);
        bar->setPen(p);
        QBrush b = bar->brush();
        b.setColor(color);
        bar->setBrush(b);
        return true;
    }
    case QwtPlotItem::Rtti_PlotHistogram: {
        auto* hist = static_cast< QwtPlotHistogram* >(item);
        QPen p = hist->pen();
        p.setColor(color);
        hist->setPen(p);
        QBrush b = hist->brush();
        b.setColor(color);
        hist->setBrush(b);
        return true;
    }
    case QwtPlotItem::Rtti_PlotIntervalCurve: {
        auto* curve = static_cast< QwtPlotIntervalCurve* >(item);
        QPen p = curve->pen();
        p.setColor(color);
        curve->setPen(p);
        QBrush b = curve->brush();
        b.setColor(color);
        curve->setBrush(b);
        return true;
    }
    case QwtPlotItem::Rtti_PlotTradingCurve: {
        auto* curve = static_cast< QwtPlotTradingCurve* >(item);
        QPen p = curve->symbolPen();
        p.setColor(color);
        curve->setSymbolPen(p);
        return true;
    }
    case QwtPlotItem::Rtti_PlotSpectroCurve:
        // SpectroCurve uses color map, not a pen color
        return false;
    case QwtPlotItem::Rtti_PlotGrid: {
        auto* grid = static_cast< QwtPlotGrid* >(item);
        QPen p = grid->majorPen();
        p.setColor(color);
        grid->setMajorPen(p);
        return true;
    }
    case QwtPlotItem::Rtti_PlotMarker: {
        auto* marker = static_cast< QwtPlotMarker* >(item);
        QPen p = marker->linePen();
        p.setColor(color);
        marker->setLinePen(p);
        return true;
    }
    default:
        return false;
    }
}

/**
 * @brief Get the pen of a plot item
 * @param item Plot item to query
 * @return The primary pen of the item, or a default QPen if not recognized
 */
QPen QwtPlotStyling::pen(const QwtPlotItem* item)
{
    if (!item)
        return QPen();

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve:
        return static_cast< const QwtPlotCurve* >(item)->pen();
    case QwtPlotItem::Rtti_PlotBarChart:
        return static_cast< const QwtPlotBarChart* >(item)->pen();
    case QwtPlotItem::Rtti_PlotHistogram:
        return static_cast< const QwtPlotHistogram* >(item)->pen();
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        return static_cast< const QwtPlotIntervalCurve* >(item)->pen();
    case QwtPlotItem::Rtti_PlotTradingCurve:
        return static_cast< const QwtPlotTradingCurve* >(item)->symbolPen();
    case QwtPlotItem::Rtti_PlotSpectroCurve:
        return QPen(); // SpectroCurve uses color map, not a pen
    case QwtPlotItem::Rtti_PlotGrid:
        return static_cast< const QwtPlotGrid* >(item)->majorPen();
    case QwtPlotItem::Rtti_PlotMarker:
        return static_cast< const QwtPlotMarker* >(item)->linePen();
    default:
        return QPen();
    }
}

/**
 * @brief Set the pen of a plot item
 * @param item Plot item to modify
 * @param pen New pen to apply
 * @return true if the pen was successfully applied
 */
bool QwtPlotStyling::setPen(QwtPlotItem* item, const QPen& pen)
{
    if (!item)
        return false;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve:
        static_cast< QwtPlotCurve* >(item)->setPen(pen);
        return true;
    case QwtPlotItem::Rtti_PlotBarChart:
        static_cast< QwtPlotBarChart* >(item)->setPen(pen);
        return true;
    case QwtPlotItem::Rtti_PlotHistogram:
        static_cast< QwtPlotHistogram* >(item)->setPen(pen);
        return true;
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        static_cast< QwtPlotIntervalCurve* >(item)->setPen(pen);
        return true;
    case QwtPlotItem::Rtti_PlotTradingCurve:
        static_cast< QwtPlotTradingCurve* >(item)->setSymbolPen(pen);
        return true;
    case QwtPlotItem::Rtti_PlotSpectroCurve:
        return false; // SpectroCurve uses color map, not a pen
    case QwtPlotItem::Rtti_PlotGrid:
        static_cast< QwtPlotGrid* >(item)->setMajorPen(pen);
        return true;
    case QwtPlotItem::Rtti_PlotMarker:
        static_cast< QwtPlotMarker* >(item)->setLinePen(pen);
        return true;
    default:
        return false;
    }
}

/**
 * @brief Get the brush of a plot item
 * @param item Plot item to query
 * @return The primary brush of the item, or a default QBrush if not recognized
 */
QBrush QwtPlotStyling::brush(const QwtPlotItem* item)
{
    if (!item)
        return QBrush();

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve:
        return static_cast< const QwtPlotCurve* >(item)->brush();
    case QwtPlotItem::Rtti_PlotBarChart:
        return static_cast< const QwtPlotBarChart* >(item)->brush();
    case QwtPlotItem::Rtti_PlotHistogram:
        return static_cast< const QwtPlotHistogram* >(item)->brush();
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        return static_cast< const QwtPlotIntervalCurve* >(item)->brush();
    default:
        return QBrush();
    }
}

/**
 * @brief Set the brush of a plot item
 * @param item Plot item to modify
 * @param brush New brush to apply
 * @return true if the brush was successfully applied
 */
bool QwtPlotStyling::setBrush(QwtPlotItem* item, const QBrush& brush)
{
    if (!item)
        return false;

    switch (item->rtti()) {
    case QwtPlotItem::Rtti_PlotCurve:
        static_cast< QwtPlotCurve* >(item)->setBrush(brush);
        return true;
    case QwtPlotItem::Rtti_PlotBarChart:
        static_cast< QwtPlotBarChart* >(item)->setBrush(brush);
        return true;
    case QwtPlotItem::Rtti_PlotHistogram:
        static_cast< QwtPlotHistogram* >(item)->setBrush(brush);
        return true;
    case QwtPlotItem::Rtti_PlotIntervalCurve:
        static_cast< QwtPlotIntervalCurve* >(item)->setBrush(brush);
        return true;
    default:
        return false;
    }
}

/**
 * @brief Set a symbol on a curve with style and size
 * @param curve Target curve
 * @param style Symbol style (ellipse, rect, diamond, etc.)
 * @param size Symbol size in pixels
 */
void QwtPlotStyling::setSymbol(QwtPlotCurve* curve, QwtSymbol::Style style, const QSize& size)
{
    if (!curve)
        return;

    auto* symbol = new QwtSymbol(style, QBrush(curve->pen().color()), curve->pen(), size);
    curve->setSymbol(symbol);
}

/**
 * @brief Set a symbol on a curve with style, color and size
 * @param curve Target curve
 * @param style Symbol style
 * @param color Symbol fill color
 * @param size Symbol size in pixels
 */
void QwtPlotStyling::setSymbol(QwtPlotCurve* curve, QwtSymbol::Style style,
                               const QColor& color, const QSize& size)
{
    if (!curve)
        return;

    QPen symbolPen(curve->pen().color());
    auto* symbol = new QwtSymbol(style, QBrush(color), symbolPen, size);
    curve->setSymbol(symbol);
}

/**
 * @brief Set curve line pen style and width
 * @param curve Target curve
 * @param style Pen style (solid, dashed, dotted, etc.)
 * @param width Pen width (0 = cosmetic)
 */
void QwtPlotStyling::setLineStyle(QwtPlotCurve* curve, Qt::PenStyle style, qreal width)
{
    if (!curve)
        return;

    QPen p = curve->pen();
    p.setStyle(style);
    if (width >= 0.0)
        p.setWidthF(width);
    curve->setPen(p);
}

/**
 * @brief Recommend a pen width based on the number of data points
 * @param pointCount Number of data points in the series
 * @return Recommended pen width
 * @details For large datasets (>10000 points), a thinner pen improves rendering
 *          performance and visual clarity. For small datasets, a wider pen
 *          provides better visibility.
 */
qreal QwtPlotStyling::recommendPenWidth(int pointCount)
{
    if (pointCount > 100000)
        return 0.5;
    if (pointCount > 10000)
        return 1.0;
    if (pointCount > 1000)
        return 1.5;
    if (pointCount > 100)
        return 2.0;
    return 2.5;
}

/**
 * @brief Force an immediate replot even when autoReplot is disabled
 * @param plot Target plot
 * @details Temporarily enables ImmediatePaint on the canvas (if it is a QwtPlotCanvas),
 *          performs a replot, and restores the previous paint attributes.
 */
void QwtPlotStyling::forceReplot(QwtPlot* plot)
{
    if (!plot)
        return;

    QwtPlotCanvas* canvas = qobject_cast< QwtPlotCanvas* >(plot->canvas());
    if (canvas) {
        const bool wasImmediate = canvas->testPaintAttribute(QwtPlotCanvas::ImmediatePaint);
        if (!wasImmediate)
            canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);

        plot->replot();

        if (!wasImmediate)
            canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);
    } else {
        plot->replot();
    }
}
