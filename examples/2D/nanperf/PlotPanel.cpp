#include "PlotPanel.h"
#include "FilterModes.h"

#include <QLabel>
#include <QElapsedTimer>
#include <QVBoxLayout>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_axis.h>

PlotPanel::PlotPanel(NanCase cs, QWidget* parent)
    : QWidget(parent)
    , m_case(cs)
    , m_plot(new QwtPlot(this))
    , m_curve(new QwtPlotCurve(NanDataGenerator::caseLabel(cs)))
    , m_titleLabel(new QLabel(NanDataGenerator::caseLabel(cs), this))
    , m_timeLabel(new QLabel(QStringLiteral("replot: -"), this))
{
    m_plot->setCanvasBackground(Qt::white);
    // Synchronous painting so replot timing is accurate.
    static_cast< QwtPlotCanvas* >(m_plot->canvas())->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(Qt::gray, 0.0, Qt::DotLine);
    grid->attach(m_plot);

    m_curve->setPen(QColor("#1f77b4"), 1.5);
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    m_curve->attach(m_plot);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_plot, 1);
    layout->addWidget(m_timeLabel);
}

void PlotPanel::setCurveData(int numPoints, double nanFraction)
{
    QVector< double > x, y;
    NanDataGenerator::generate(m_case, numPoints, nanFraction, x, y);
    // Use the double-array overload so QwtPointArrayData<double> is created,
    // which triggers the raw-pointer SIMD/scalar branch in qwtMinMaxBucketReduce.
    m_curve->setSamples(x, y);
    setFixedScales(numPoints);
    m_plot->replot();
}

void PlotPanel::setMode(int modeIndex)
{
    applyMode(m_curve, modeIndex);
    m_plot->replot();
}

void PlotPanel::setFixedScales(int numPoints)
{
    m_plot->setAxisScale(QwtAxis::XBottom, 0.0, qMax(1, numPoints - 1));
    m_plot->setAxisScale(QwtAxis::YLeft, -1.5, 1.5);
}

void PlotPanel::apply(int numPoints, double nanFraction, int modeIndex)
{
    QVector< double > x, y;
    NanDataGenerator::generate(m_case, numPoints, nanFraction, x, y);
    m_curve->setSamples(x, y);
    setFixedScales(numPoints);
    applyMode(m_curve, modeIndex);

    QElapsedTimer timer;
    timer.start();
    m_plot->replot();
    showReplotTime(timer.nsecsElapsed() / 1.0e6);
}

void PlotPanel::showReplotTime(double ms)
{
    m_timeLabel->setText(QStringLiteral("replot: %1 ms").arg(ms, 0, 'f', 3));
}
