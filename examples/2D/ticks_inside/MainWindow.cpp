/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "MainWindow.h"

#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotGrid>
#include <QwtLegend>

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_plot(nullptr)
    , m_curve(nullptr)
    , m_chkYLeft(nullptr)
    , m_chkYRight(nullptr)
    , m_chkXTop(nullptr)
    , m_chkXBottom(nullptr)
{
    setupPlot();

    QWidget* centralWidget  = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    mainLayout->addWidget(m_plot, 1);
    mainLayout->addWidget(createControlPanel());

    setCentralWidget(centralWidget);
    setWindowTitle("Ticks Inside Demo");
}

void MainWindow::setupPlot()
{
    m_plot = new QwtPlot(this);
    m_plot->setTitle("Tick Direction Demo");
    m_plot->setCanvasBackground(Qt::white);

    // Enable all axes
    m_plot->setAxisVisible(QwtAxis::YLeft, true);
    m_plot->setAxisVisible(QwtAxis::YRight, true);
    m_plot->setAxisVisible(QwtAxis::XTop, true);
    m_plot->setAxisVisible(QwtAxis::XBottom, true);

    // Set axis scales
    m_plot->setAxisScale(QwtAxis::YLeft, 0.0, 10.0);
    m_plot->setAxisScale(QwtAxis::YRight, 0.0, 10.0);
    m_plot->setAxisScale(QwtAxis::XBottom, 0.0, 10.0);
    m_plot->setAxisScale(QwtAxis::XTop, 0.0, 10.0);

    // Add legend
    m_plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    // Add grid
    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(Qt::gray, 0.5, Qt::DotLine);
    grid->attach(m_plot);

    // Add curve
    m_curve = new QwtPlotCurve();
    m_curve->setTitle("Sample Curve");
    m_curve->setPen(Qt::blue, 2);
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    QPolygonF points;
    for (int i = 0; i <= 10; i++) {
        double x = i;
        double y = 4.0 + 3.0 * sin(i * 0.5);
        points << QPointF(x, y);
    }
    m_curve->setSamples(points);
    m_curve->attach(m_plot);
}

QWidget* MainWindow::createControlPanel()
{
    QGroupBox* groupBox = new QGroupBox("Tick Direction", this);
    QVBoxLayout* layout = new QVBoxLayout(groupBox);

    m_chkYLeft   = new QCheckBox("YLeft Inside", this);
    m_chkYRight  = new QCheckBox("YRight Inside", this);
    m_chkXTop    = new QCheckBox("XTop Inside", this);
    m_chkXBottom = new QCheckBox("XBottom Inside", this);

    layout->addWidget(m_chkYLeft);
    layout->addWidget(m_chkYRight);
    layout->addWidget(m_chkXTop);
    layout->addWidget(m_chkXBottom);

    layout->addSpacing(10);

    QHBoxLayout* buttonLayout  = new QHBoxLayout();
    QPushButton* btnAllInside  = new QPushButton("All Inside", this);
    QPushButton* btnAllOutside = new QPushButton("All Outside", this);
    buttonLayout->addWidget(btnAllInside);
    buttonLayout->addWidget(btnAllOutside);
    layout->addLayout(buttonLayout);

    connect(m_chkYLeft, &QCheckBox::toggled, this, &MainWindow::onYLeftToggled);
    connect(m_chkYRight, &QCheckBox::toggled, this, &MainWindow::onYRightToggled);
    connect(m_chkXTop, &QCheckBox::toggled, this, &MainWindow::onXTopToggled);
    connect(m_chkXBottom, &QCheckBox::toggled, this, &MainWindow::onXBottomToggled);
    connect(btnAllInside, &QPushButton::clicked, this, &MainWindow::onAllInsideClicked);
    connect(btnAllOutside, &QPushButton::clicked, this, &MainWindow::onAllOutsideClicked);

    return groupBox;
}

void MainWindow::onYLeftToggled(bool checked)
{
    m_plot->setAxisTickDirection(QwtAxis::YLeft, checked ? QwtPlot::TickInside : QwtPlot::TickOutside);
    m_plot->replot();
}

void MainWindow::onYRightToggled(bool checked)
{
    m_plot->setAxisTickDirection(QwtAxis::YRight, checked ? QwtPlot::TickInside : QwtPlot::TickOutside);
    m_plot->replot();
}

void MainWindow::onXTopToggled(bool checked)
{
    m_plot->setAxisTickDirection(QwtAxis::XTop, checked ? QwtPlot::TickInside : QwtPlot::TickOutside);
    m_plot->replot();
}

void MainWindow::onXBottomToggled(bool checked)
{
    m_plot->setAxisTickDirection(QwtAxis::XBottom, checked ? QwtPlot::TickInside : QwtPlot::TickOutside);
    m_plot->replot();
}

void MainWindow::onAllInsideClicked()
{
    m_chkYLeft->setChecked(true);
    m_chkYRight->setChecked(true);
    m_chkXTop->setChecked(true);
    m_chkXBottom->setChecked(true);
}

void MainWindow::onAllOutsideClicked()
{
    m_chkYLeft->setChecked(false);
    m_chkYRight->setChecked(false);
    m_chkXTop->setChecked(false);
    m_chkXBottom->setChecked(false);
}
