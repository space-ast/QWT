/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_factory.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_barchart.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot_tradingcurve.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_vectorfield.h>
#include <qwt_plot_boxchart.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoneitem.h>
#include <qwt_plot_arrowmarker.h>
#include <qwt_plot_graphicitem.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_graphic.h>
#include <qwt_interval.h>
#include <qwt_samples.h>
#include <qwt_point_3d.h>
#include <qwt_text.h>
#include <qwt_scale_div.h>

/**
 * @brief Create a curve from QPointF data and attach it to a plot
 * @param plot Target plot
 * @param title Curve title
 * @param data Sample data as QPointF vector
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached curve
 */
QwtPlotCurve* QwtPlotFactory::createCurve(QwtPlot* plot, const QString& title,
                                          const QVector< QPointF >& data,
                                          QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* curve = new QwtPlotCurve(title);
    curve->setSamples(data);
    curve->setAxes(xAxis, yAxis);
    curve->attach(plot);
    return curve;
}

/**
 * @brief Create a curve from separate x and y vectors and attach it to a plot
 * @param plot Target plot
 * @param title Curve title
 * @param x X coordinate vector
 * @param y Y coordinate vector
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached curve
 */
QwtPlotCurve* QwtPlotFactory::createCurve(QwtPlot* plot, const QString& title,
                                          const QVector< double >& x, const QVector< double >& y,
                                          QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* curve = new QwtPlotCurve(title);
    curve->setSamples(x, y);
    curve->setAxes(xAxis, yAxis);
    curve->attach(plot);
    return curve;
}

/**
 * @brief Create a curve from y-only data (x = index) and attach it to a plot
 * @param plot Target plot
 * @param title Curve title
 * @param y Y coordinate vector (x is auto-generated as 0, 1, 2, ...)
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached curve
 */
QwtPlotCurve* QwtPlotFactory::createCurve(QwtPlot* plot, const QString& title,
                                          const QVector< double >& y,
                                          QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* curve = new QwtPlotCurve(title);
    curve->setSamples(y);
    curve->setAxes(xAxis, yAxis);
    curve->attach(plot);
    return curve;
}

/**
 * @brief Create a bar chart from y-only values and attach it to a plot
 * @param plot Target plot
 * @param title Bar chart title
 * @param values Y values (x is auto-generated as 0, 1, 2, ...)
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached bar chart
 */
QwtPlotBarChart* QwtPlotFactory::createBarChart(QwtPlot* plot, const QString& title,
                                                const QVector< double >& values,
                                                QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* bar = new QwtPlotBarChart(title);
    bar->setSamples(values);
    bar->setAxes(xAxis, yAxis);
    bar->attach(plot);
    return bar;
}

/**
 * @brief Create a bar chart from QPointF data and attach it to a plot
 * @param plot Target plot
 * @param title Bar chart title
 * @param data Sample data as QPointF vector
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached bar chart
 */
QwtPlotBarChart* QwtPlotFactory::createBarChart(QwtPlot* plot, const QString& title,
                                                const QVector< QPointF >& data,
                                                QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* bar = new QwtPlotBarChart(title);
    bar->setSamples(data);
    bar->setAxes(xAxis, yAxis);
    bar->attach(plot);
    return bar;
}

/**
 * @brief Create a multi bar chart from QwtSetSample data
 * @param plot Target plot
 * @param title Multi bar chart title
 * @param data Set sample data
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached multi bar chart
 */
QwtPlotMultiBarChart* QwtPlotFactory::createMultiBarChart(QwtPlot* plot, const QString& title,
                                                          const QVector< QwtSetSample >& data,
                                                          QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* bar = new QwtPlotMultiBarChart(title);
    bar->setSamples(data);
    bar->setAxes(xAxis, yAxis);
    bar->attach(plot);
    return bar;
}

/**
 * @brief Create a multi bar chart from nested vectors
 * @param plot Target plot
 * @param title Multi bar chart title
 * @param data Nested vector where each inner vector is one bar group
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached multi bar chart
 */
QwtPlotMultiBarChart* QwtPlotFactory::createMultiBarChart(QwtPlot* plot, const QString& title,
                                                          const QVector< QVector< double > >& data,
                                                          QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* bar = new QwtPlotMultiBarChart(title);
    bar->setSamples(data);
    bar->setAxes(xAxis, yAxis);
    bar->attach(plot);
    return bar;
}

/**
 * @brief Create a histogram from interval samples
 * @param plot Target plot
 * @param title Histogram title
 * @param data Interval sample data
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached histogram
 */
QwtPlotHistogram* QwtPlotFactory::createHistogram(QwtPlot* plot, const QString& title,
                                                  const QVector< QwtIntervalSample >& data,
                                                  QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* hist = new QwtPlotHistogram(title);
    hist->setSamples(data);
    hist->setAxes(xAxis, yAxis);
    hist->attach(plot);
    return hist;
}

/**
 * @brief Create an interval curve
 * @param plot Target plot
 * @param title Interval curve title
 * @param data Interval sample data
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached interval curve
 */
QwtPlotIntervalCurve* QwtPlotFactory::createIntervalCurve(QwtPlot* plot, const QString& title,
                                                          const QVector< QwtIntervalSample >& data,
                                                          QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* curve = new QwtPlotIntervalCurve(title);
    curve->setSamples(data);
    curve->setAxes(xAxis, yAxis);
    curve->attach(plot);
    return curve;
}

/**
 * @brief Create a trading curve (K-line / OHLC)
 * @param plot Target plot
 * @param title Trading curve title
 * @param data OHLC sample data
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached trading curve
 */
QwtPlotTradingCurve* QwtPlotFactory::createTradingCurve(QwtPlot* plot, const QString& title,
                                                        const QVector< QwtOHLCSample >& data,
                                                        QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* curve = new QwtPlotTradingCurve(title);
    curve->setSamples(data);
    curve->setAxes(xAxis, yAxis);
    curve->attach(plot);
    return curve;
}

/**
 * @brief Create a spectro curve from 3D point data
 * @param plot Target plot
 * @param title Spectro curve title
 * @param data 3D point data (x, y, z where z is typically mapped to color)
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached spectro curve
 */
QwtPlotSpectroCurve* QwtPlotFactory::createSpectroCurve(QwtPlot* plot, const QString& title,
                                                        const QVector< QwtPoint3D >& data,
                                                        QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* curve = new QwtPlotSpectroCurve(title);
    curve->setSamples(data);
    curve->setAxes(xAxis, yAxis);
    curve->attach(plot);
    return curve;
}

/**
 * @brief Create a vector field from samples
 * @param plot Target plot
 * @param title Vector field title
 * @param data Vector field sample data (position + vector)
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached vector field
 */
QwtPlotVectorField* QwtPlotFactory::createVectorField(QwtPlot* plot, const QString& title,
                                                      const QVector< QwtVectorFieldSample >& data,
                                                      QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* field = new QwtPlotVectorField(title);
    field->setSamples(data);
    field->setAxes(xAxis, yAxis);
    field->attach(plot);
    return field;
}

/**
 * @brief Create a box chart from box samples
 * @param plot Target plot
 * @param title Box chart title
 * @param data Box sample data
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached box chart
 */
QwtPlotBoxChart* QwtPlotFactory::createBoxChart(QwtPlot* plot, const QString& title,
                                                const QVector< QwtBoxSample >& data,
                                                QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* box = new QwtPlotBoxChart(title);
    box->setSamples(data);
    box->setAxes(xAxis, yAxis);
    box->attach(plot);
    return box;
}

/**
 * @brief Create and attach a grid to a plot
 * @param plot Target plot
 * @param enableMinor Whether to enable minor grid lines
 * @param majorPen Pen for major grid lines
 * @param minorPen Pen for minor grid lines
 * @return The newly created and attached grid
 */
QwtPlotGrid* QwtPlotFactory::createGrid(QwtPlot* plot, bool enableMinor,
                                        const QPen& majorPen, const QPen& minorPen)
{
    auto* grid = new QwtPlotGrid();
    grid->setMajorPen(majorPen);
    if (enableMinor) {
        grid->enableXMin(true);
        grid->enableYMin(true);
        grid->setMinorPen(minorPen);
    }
    grid->attach(plot);
    return grid;
}

/**
 * @brief Create a point marker with optional label
 * @param plot Target plot
 * @param pos Marker position in data coordinates
 * @param label Optional text label
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached marker
 */
QwtPlotMarker* QwtPlotFactory::createMarker(QwtPlot* plot, const QPointF& pos,
                                            const QString& label,
                                            QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* marker = new QwtPlotMarker();
    marker->setValue(pos);
    marker->setAxes(xAxis, yAxis);
    if (!label.isEmpty())
        marker->setLabel(QwtText(label));
    marker->attach(plot);
    return marker;
}

/**
 * @brief Create a horizontal line marker
 * @param plot Target plot
 * @param y Y position of the line
 * @param pen Line pen
 * @return The newly created and attached horizontal line marker
 */
QwtPlotMarker* QwtPlotFactory::createHLine(QwtPlot* plot, double y, const QPen& pen)
{
    auto* marker = new QwtPlotMarker();
    marker->setLineStyle(QwtPlotMarker::HLine);
    marker->setYValue(y);
    marker->setLinePen(pen);
    marker->attach(plot);
    return marker;
}

/**
 * @brief Create a vertical line marker
 * @param plot Target plot
 * @param x X position of the line
 * @param pen Line pen
 * @return The newly created and attached vertical line marker
 */
QwtPlotMarker* QwtPlotFactory::createVLine(QwtPlot* plot, double x, const QPen& pen)
{
    auto* marker = new QwtPlotMarker();
    marker->setLineStyle(QwtPlotMarker::VLine);
    marker->setXValue(x);
    marker->setLinePen(pen);
    marker->attach(plot);
    return marker;
}

/**
 * @brief Create a highlighted zone
 * @param plot Target plot
 * @param interval The interval range to highlight
 * @param orientation Vertical or horizontal zone
 * @param brush Fill brush for the zone
 * @return The newly created and attached zone item
 */
QwtPlotZoneItem* QwtPlotFactory::createZone(QwtPlot* plot, const QwtInterval& interval,
                                            Qt::Orientation orientation, const QBrush& brush)
{
    auto* zone = new QwtPlotZoneItem();
    zone->setInterval(interval.minValue(), interval.maxValue());
    zone->setOrientation(orientation);
    zone->setBrush(brush);
    zone->attach(plot);
    return zone;
}

/**
 * @brief Create an arrow marker
 * @param plot Target plot
 * @param start Arrow start point in data coordinates
 * @param end Arrow end point in data coordinates
 * @param xAxis X axis to bind to
 * @param yAxis Y axis to bind to
 * @return The newly created and attached arrow marker
 */
QwtPlotArrowMarker* QwtPlotFactory::createArrowMarker(QwtPlot* plot,
                                                      const QPointF& start, const QPointF& end,
                                                      QwtAxisId xAxis, QwtAxisId yAxis)
{
    auto* arrow = new QwtPlotArrowMarker();
    arrow->setPoints(start, end);
    arrow->setAxes(xAxis, yAxis);
    arrow->attach(plot);
    return arrow;
}

/**
 * @brief Create a graphic item
 * @param plot Target plot
 * @param rect Bounding rectangle in data coordinates
 * @param graphic The graphic to display
 * @return The newly created and attached graphic item
 */
QwtPlotGraphicItem* QwtPlotFactory::createGraphic(QwtPlot* plot, const QRectF& rect,
                                                  const QwtGraphic& graphic)
{
    auto* item = new QwtPlotGraphicItem();
    item->setGraphic(rect, graphic);
    item->attach(plot);
    return item;
}

/**
 * @brief Create a text label on the canvas
 * @param plot Target plot
 * @param text Label text
 * @param alignment Alignment within the canvas (applied via QwtText render flags)
 * @return The newly created and attached text label
 */
QwtPlotTextLabel* QwtPlotFactory::createTextLabel(QwtPlot* plot, const QString& text,
                                                  Qt::Alignment alignment)
{
    auto* label = new QwtPlotTextLabel();
    QwtText textObj(text);
    textObj.setRenderFlags(alignment);
    label->setText(textObj);
    label->attach(plot);
    return label;
}

/**
 * @brief Create an in-canvas legend item
 * @param plot Target plot
 * @return The newly created and attached legend item
 */
QwtPlotLegendItem* QwtPlotFactory::createLegend(QwtPlot* plot)
{
    auto* legend = new QwtPlotLegendItem();
    legend->attach(plot);
    return legend;
}

/**
 * @brief Create a scale item at a specific axis
 * @param plot Target plot
 * @param axis Axis to display the scale on
 * @return The newly created and attached scale item
 */
QwtPlotScaleItem* QwtPlotFactory::createScaleItem(QwtPlot* plot, QwtAxisId axis)
{
    QwtScaleDraw::Alignment alignment = QwtScaleDraw::BottomScale;
    if (axis == QwtAxis::YLeft)
        alignment = QwtScaleDraw::LeftScale;
    else if (axis == QwtAxis::YRight)
        alignment = QwtScaleDraw::RightScale;
    else if (axis == QwtAxis::XTop)
        alignment = QwtScaleDraw::TopScale;

    auto* scale = new QwtPlotScaleItem(alignment);
    scale->setScaleDiv(plot->axisScaleDiv(axis));
    scale->setScaleDivFromAxis(false);
    scale->attach(plot);
    return scale;
}
