/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_transform.h"

#include <qwt_plot.h>
#include <qwt_plot_item.h>
#include <qwt_plot_dict.h>
#include <qwt_scale_map.h>
#include <qwt_scale_div.h>

#include <qpainterpath.h>

/**
 * @brief Transform a point from one axis pair to another
 * @param plot Plot to query
 * @param point Data coordinates in the source axis system
 * @param fromX Source X axis
 * @param fromY Source Y axis
 * @param toX Target X axis
 * @param toY Target Y axis
 * @return Data coordinates in the target axis system (same screen position)
 * @details This method first transforms the data point to screen coordinates
 *          using the source axes, then transforms back to data coordinates
 *          using the target axes.
 */
QPointF QwtPlotTransform::transformPoint(const QwtPlot* plot,
                                         const QPointF& point,
                                         QwtAxisId fromX,
                                         QwtAxisId fromY,
                                         QwtAxisId toX,
                                         QwtAxisId toY)
{
    if (!plot)
        return point;

    if (fromX == toX && fromY == toY)
        return point;

    double x = point.x();
    double y = point.y();

    if (fromX != toX) {
        const QwtScaleMap xMap1 = plot->canvasMap(fromX);
        const QwtScaleMap xMap2 = plot->canvasMap(toX);
        const double screenX    = xMap1.transform(x);
        x                       = xMap2.invTransform(screenX);
    }

    if (fromY != toY) {
        const QwtScaleMap yMap1 = plot->canvasMap(fromY);
        const QwtScaleMap yMap2 = plot->canvasMap(toY);
        const double screenY    = yMap1.transform(y);
        y                       = yMap2.invTransform(screenY);
    }

    return QPointF(x, y);
}

/**
 * @brief Transform a path from one axis pair to another
 * @param plot Plot to query
 * @param path Path in source axis data coordinates
 * @param fromX Source X axis
 * @param fromY Source Y axis
 * @param toX Target X axis
 * @param toY Target Y axis
 * @return Transformed path in target axis data coordinates
 */
QPainterPath QwtPlotTransform::transformPath(const QwtPlot* plot,
                                             const QPainterPath& path,
                                             QwtAxisId fromX,
                                             QwtAxisId fromY,
                                             QwtAxisId toX,
                                             QwtAxisId toY)
{
    if (!plot || (fromX == toX && fromY == toY))
        return path;

    QPainterPath result;
    result.setFillRule(path.fillRule());

    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element& el = path.elementAt(i);
        const QPointF transformed       = transformPoint(plot, QPointF(el.x, el.y), fromX, fromY, toX, toY);
        switch (el.type) {
        case QPainterPath::MoveToElement:
            result.moveTo(transformed);
            break;
        case QPainterPath::LineToElement:
            result.lineTo(transformed);
            break;
        case QPainterPath::CurveToElement:
            result.cubicTo(
                transformed,
                transformPoint(plot, QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y), fromX, fromY, toX, toY),
                transformPoint(plot, QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y), fromX, fromY, toX, toY));
            i += 2;
            break;
        case QPainterPath::CurveToDataElement:
            // Skip - handled by CurveToElement
            break;
        }
    }

    return result;
}

/**
 * @brief Convert screen position to data coordinates
 * @param plot Plot to query
 * @param screenPos Position in canvas widget pixel coordinates
 * @param xAxis X axis to use for conversion
 * @param yAxis Y axis to use for conversion
 * @return Data coordinates corresponding to the screen position
 */
QPointF QwtPlotTransform::toPlotPoint(const QwtPlot* plot, const QPointF& screenPos, QwtAxisId xAxis, QwtAxisId yAxis)
{
    if (!plot)
        return QPointF();

    const QwtScaleMap xMap = plot->canvasMap(xAxis);
    const QwtScaleMap yMap = plot->canvasMap(yAxis);

    return QPointF(xMap.invTransform(screenPos.x()), yMap.invTransform(screenPos.y()));
}

/**
 * @brief Convert data coordinates to screen position
 * @param plot Plot to query
 * @param plotPoint Data coordinates
 * @param xAxis X axis to use for conversion
 * @param yAxis Y axis to use for conversion
 * @return Screen position in canvas widget pixel coordinates
 */
QPointF QwtPlotTransform::toScreenPoint(const QwtPlot* plot, const QPointF& plotPoint, QwtAxisId xAxis, QwtAxisId yAxis)
{
    if (!plot)
        return QPointF();

    const QwtScaleMap xMap = plot->canvasMap(xAxis);
    const QwtScaleMap yMap = plot->canvasMap(yAxis);

    return QPointF(xMap.transform(plotPoint.x()), yMap.transform(plotPoint.y()));
}

/**
 * @brief Compute the data-coordinate offset for 1 pixel displacement
 * @param plot Plot to query
 * @param xAxis X axis
 * @param yAxis Y axis
 * @return Positive offset in data coordinates that corresponds to 1 pixel
 *         at the center of the plot
 */
QPointF QwtPlotTransform::onePixelOffset(const QwtPlot* plot, QwtAxisId xAxis, QwtAxisId yAxis)
{
    if (!plot)
        return QPointF(0.0, 0.0);

    const QPoint center = plot->rect().center();
    const double x1     = plot->invTransform(xAxis, center.x());
    const double y1     = plot->invTransform(yAxis, center.y());
    const double x2     = plot->invTransform(xAxis, center.x() + 1);
    const double y2     = plot->invTransform(yAxis, center.y() + 1);

    return QPointF(x2 - x1, y2 - y1);
}

/**
 * @brief Get the currently visible data range
 * @param plot Plot to query
 * @param xAxis X axis
 * @param yAxis Y axis
 * @return Rectangle in data coordinates representing the visible area
 */
QRectF QwtPlotTransform::visibleRange(const QwtPlot* plot, QwtAxisId xAxis, QwtAxisId yAxis)
{
    if (!plot)
        return QRectF();

    const QwtScaleDiv& xDiv = plot->axisScaleDiv(xAxis);
    const QwtScaleDiv& yDiv = plot->axisScaleDiv(yAxis);

    const double xMin = xDiv.lowerBound();
    const double xMax = xDiv.upperBound();
    const double yMin = yDiv.lowerBound();
    const double yMax = yDiv.upperBound();

    return QRectF(xMin, yMin, xMax - xMin, yMax - yMin);
}

/**
 * @brief Get the union of all data item bounding rectangles
 * @param plot Plot to query
 * @param onlyVisible If true, only include visible items
 * @return Union of all data bounding rectangles, or an invalid rect if no data items exist
 */
QRectF QwtPlotTransform::totalDataRange(const QwtPlot* plot, bool onlyVisible)
{
    if (!plot)
        return QRectF();

    QRectF totalRect;
    bool first = true;

    const QwtPlotItemList& items = plot->itemList();
    for (QwtPlotItem* item : items) {
        if (onlyVisible && !item->isVisible())
            continue;

        const QRectF itemRect = item->boundingRect();
        if (!itemRect.isValid())
            continue;

        if (first) {
            totalRect = itemRect;
            first     = false;
        } else {
            totalRect = totalRect.united(itemRect);
        }
    }

    return totalRect;
}
