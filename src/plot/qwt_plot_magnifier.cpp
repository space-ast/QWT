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
 *        - QwtPanner -> QwtCachePanner (pixmap-cache version)
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

#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_plot_magnifier.h"

class QwtPlotMagnifier::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotMagnifier)
public:
    PrivateData(QwtPlotMagnifier* p) : q_ptr(p)
    {
        for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
            isAxisEnabled[ axis ] = true;
    }

    bool isAxisEnabled[ QwtAxis::AxisPositions ];
};

/*!
 * @brief Constructor
 * @param canvas Plot canvas to be magnified
 *
 * Creates a magnifier attached to the given plot canvas.
 * All axes are enabled by default.
 */
QwtPlotMagnifier::QwtPlotMagnifier(QWidget* canvas) : QwtMagnifier(canvas), QWT_PIMPL_CONSTRUCT
{
}

/*!
 * @brief Destructor
 */
QwtPlotMagnifier::~QwtPlotMagnifier()
{
}

/*!
 * @brief Enable or disable an axis for magnification
 * @param axisId Axis identifier
 * @param on true to enable, false to disable
 *
 * Only axes that are enabled will be zoomed when the magnifier is triggered.
 * All other axes will remain unchanged.
 *
 * All axes are enabled by default.
 *
 * @sa isAxisEnabled()
 */
void QwtPlotMagnifier::setAxisEnabled(QwtAxisId axisId, bool on)
{
    QWT_D(d);
    if (QwtAxis::isValid(axisId))
        d->isAxisEnabled[ axisId ] = on;
}

/*!
 * @brief Check if an axis is enabled for magnification
 * @param axisId Axis identifier
 * @return true if the axis is enabled, false otherwise
 *
 * @sa setAxisEnabled()
 */
bool QwtPlotMagnifier::isAxisEnabled(QwtAxisId axisId) const
{
    QWT_DC(d);
    if (QwtAxis::isValid(axisId))
        return d->isAxisEnabled[ axisId ];

    return true;
}

/*!
 * @brief Return the observed plot canvas
 * @return Pointer to the canvas widget
 */
QWidget* QwtPlotMagnifier::canvas()
{
    return parentWidget();
}

/*!
 * @brief Return the observed plot canvas (const version)
 * @return Const pointer to the canvas widget
 */
const QWidget* QwtPlotMagnifier::canvas() const
{
    return parentWidget();
}

/*!
 * @brief Return the plot widget containing the observed canvas
 * @return Pointer to the QwtPlot widget, or nullptr if not found
 */
QwtPlot* QwtPlotMagnifier::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/*!
 * @brief Return the plot widget containing the observed canvas (const version)
 * @return Const pointer to the QwtPlot widget, or nullptr if not found
 */
const QwtPlot* QwtPlotMagnifier::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
 * @brief Rescale the plot axes by the given factor
 * @param factor Magnification factor. A value < 1.0 zooms in, a value > 1.0 zooms out.
 *
 * This method zooms in/out the axes scales by applying the given factor
 * to all enabled axes. The zoom is centered around the current viewport center.
 *
 * For parasite plots, this method does nothing.
 *
 * The method handles all plots in the plot list, including parasite plots
 * that share the same canvas area.
 */
void QwtPlotMagnifier::rescale(double factor)
{
    QwtPlot* hostplt = plot();
    if (!hostplt || hostplt->isParasitePlot()) {
        return;
    }

    factor = qAbs(factor);
    if (qFuzzyCompare(factor, 1.0) || qFuzzyCompare(factor, 0.0)) {
        return;
    }

    bool doReplot = false;

    const QList< QwtPlot* > allplts = hostplt->plotList(true);
    for (QwtPlot* plt : allplts) {
        plt->saveAutoReplotState();
        plt->setAutoReplot(false);

        for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
            {
                const QwtAxisId axisId(axisPos);

                if (isAxisEnabled(axisId)) {
                    const QwtScaleMap scaleMap = plt->canvasMap(axisId);

                    double v1 = scaleMap.s1();
                    double v2 = scaleMap.s2();

                    if (scaleMap.transformation()) {
                        // the coordinate system of the paint device is always linear

                        v1 = scaleMap.transform(v1);  // scaleMap.p1()
                        v2 = scaleMap.transform(v2);  // scaleMap.p2()
                    }

                    const double center  = 0.5 * (v1 + v2);
                    const double width_2 = 0.5 * (v2 - v1) * factor;

                    v1 = center - width_2;
                    v2 = center + width_2;

                    if (scaleMap.transformation()) {
                        v1 = scaleMap.invTransform(v1);
                        v2 = scaleMap.invTransform(v2);
                    }

                    plt->setAxisScale(axisId, v1, v2);
                    doReplot = true;
                }
            }
        }

        plt->restoreAutoReplotState();
    }
    if (doReplot) {
        hostplt->replotAll();
    }
}
