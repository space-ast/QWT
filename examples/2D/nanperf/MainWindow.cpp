#include "MainWindow.h"
#include "PlotPanel.h"
#include "FilterModes.h"

#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_modeCombo(new QComboBox(this))
    , m_pointsSpin(new QSpinBox(this))
    , m_nanFractionSpin(new QDoubleSpinBox(this))
    , m_repeatsSpin(new QSpinBox(this))
    , m_applyBtn(new QPushButton(QStringLiteral("应用并重绘"), this))
{
    const NanCase cases[ 4 ] = { NanCase::Leading, NanCase::LeadingTrailing, NanCase::Middle, NanCase::Trailing };
    auto* grid               = new QGridLayout;
    for (int i = 0; i < 4; ++i) {
        auto* panel = new PlotPanel(cases[ i ]);
        m_panels.append(panel);
        grid->addWidget(panel, i / 2, i % 2);
    }

    auto* central = new QWidget(this);
    auto* outer   = new QVBoxLayout(central);
    outer->addLayout(buildControls());
    outer->addLayout(grid, 1);
    setCentralWidget(central);

    refreshPanels();
    setWindowTitle(QStringLiteral("nanperf — NaN 渲染与性能"));
    resize(1200, 900);
}

QLayout* MainWindow::buildControls()
{
    const auto modes = filterModes();
    for (const auto& m : modes)
        m_modeCombo->addItem(m.label);
    m_modeCombo->setCurrentIndex(modes.size() - 1);  // FilterPointsLTTB default

    m_pointsSpin->setRange(1000, 10000000);
    m_pointsSpin->setSingleStep(10000);
    m_pointsSpin->setValue(100000);

    m_nanFractionSpin->setRange(0.0, 0.99);
    m_nanFractionSpin->setSingleStep(0.05);
    m_nanFractionSpin->setValue(0.5);

    m_repeatsSpin->setRange(1, 200);
    m_repeatsSpin->setValue(20);

    auto* bar = new QHBoxLayout;
    bar->addWidget(new QLabel(QStringLiteral("模式:")));
    bar->addWidget(m_modeCombo);
    bar->addWidget(new QLabel(QStringLiteral("点数:")));
    bar->addWidget(m_pointsSpin);
    bar->addWidget(new QLabel(QStringLiteral("NaN比例:")));
    bar->addWidget(m_nanFractionSpin);
    bar->addWidget(new QLabel(QStringLiteral("重复:")));
    bar->addWidget(m_repeatsSpin);
    bar->addWidget(m_applyBtn);
    bar->addStretch(1);

    connect(m_applyBtn, &QPushButton::clicked, this, &MainWindow::onApplyRedraw);
    return bar;
}

void MainWindow::refreshPanels()
{
    const int n    = m_pointsSpin->value();
    const double f = m_nanFractionSpin->value();
    const int mode = m_modeCombo->currentIndex();
    for (auto* panel : qwt_as_const(m_panels)) {
        panel->setCurveData(n, f);
        panel->setMode(mode);
    }
}

void MainWindow::onApplyRedraw()
{
    refreshPanels();
}
