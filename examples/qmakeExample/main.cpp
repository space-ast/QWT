/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

// This example demonstrates how to integrate Qwt into a qmake project via
// the amalgamated single file. A single #include "QwtPlot.h" exposes the
// whole Qwt API (core + 2D plot + 3D plot); no individual Qwt headers and
// no qwt library are needed. See qmakeExample.pro for the project setup.

#include "QwtPlot.h"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFrame>
#include <QPen>
#include <QPointF>
#include <QSize>
#include <QVector>

#include <cmath>

// ---------------------------------------------------------------------------
// Data generation helpers
// ---------------------------------------------------------------------------

static QVector<QPointF> sineSamples(int count, double amplitude, double frequency)
{
    QVector<QPointF> samples;
    samples.reserve(count);
    for (int i = 0; i < count; ++i) {
        const double x = i * 10.0 / (count - 1);
        samples.append(QPointF(x, amplitude * std::sin(frequency * x)));
    }
    return samples;
}

static QVector<QPointF> cosineSamples(int count, double amplitude, double frequency)
{
    QVector<QPointF> samples;
    samples.reserve(count);
    for (int i = 0; i < count; ++i) {
        const double x = i * 10.0 / (count - 1);
        samples.append(QPointF(x, amplitude * std::cos(frequency * x)));
    }
    return samples;
}

// ---------------------------------------------------------------------------
// Build a plot with curves, markers, grid and legend
// ---------------------------------------------------------------------------

static QwtPlot* createPlot()
{
    auto* plot = new QwtPlot();
    plot->setTitle("qmake Single-File Demo");
    plot->setCanvasBackground(Qt::white);
    plot->setFrameStyle(QFrame::NoFrame);

    // Grid with minor lines
    auto* grid = new QwtPlotGrid();
    grid->setMajorPen(QColor("#d0d0d0"), 1.0, Qt::SolidLine);
    grid->setMinorPen(QColor("#ececec"), 1.0, Qt::DotLine);
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->attach(plot);

    // Curve 1: sine, smooth solid line
    auto* sine = new QwtPlotCurve("Sine");
    sine->setSamples(sineSamples(200, 1.0, 1.5));
    sine->setPen(Qt::blue, 2);
    sine->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    sine->attach(plot);

    // Curve 2: cosine, dashed line with ellipse symbols
    auto* cosine = new QwtPlotCurve("Cosine");
    cosine->setSamples(cosineSamples(80, 0.8, 1.0));
    cosine->setPen(QPen(Qt::red, 1.5, Qt::DashLine));
    cosine->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
        QBrush(QColor(255, 200, 200)), QPen(Qt::red, 1), QSize(6, 6)));
    cosine->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    cosine->attach(plot);

    // Vertical marker annotating a point of interest
    auto* marker = new QwtPlotMarker();
    marker->setLineStyle(QwtPlotMarker::VLine);
    marker->setLinePen(QPen(QColor(120, 120, 120), 1, Qt::DashDotLine));
    marker->setValue(5.0, 0.0);
    QwtText label("x = 5.0");
    label.setColor(QColor(120, 120, 120));
    label.setFont(QFont("Arial", 8));
    marker->setLabel(label);
    marker->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    marker->attach(plot);

    // Axis titles and legend
    plot->setAxisTitle(QwtAxis::XBottom, "X");
    plot->setAxisTitle(QwtAxis::YLeft, "Y");
    plot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);

    return plot;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QwtPlot* plot = createPlot();

    // Interactive tools, all attached to the canvas:
    //   - Canvas zoomer: drag a rectangle (left button) to zoom in on the
    //     whole canvas; right-click steps back through the zoom stack.
    //   - Panner: drag with the middle button to pan the canvas.
    //   - Magnifier: mouse wheel zooms in/out around the cursor.
    new QwtPlotCanvasZoomer(plot->canvas());

    auto* panner = new QwtPlotPanner(plot->canvas());
    panner->setMouseButton(Qt::MiddleButton);

    new QwtPlotMagnifier(plot->canvas());

    plot->resize(600, 400);
    plot->show();

    return app.exec();
}
