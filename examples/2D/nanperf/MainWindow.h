#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QList>
#include "NanDataGenerator.h"

class PlotPanel;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onApplyRedraw();

private:
    QLayout* buildControls();
    void refreshPanels();

    QList< PlotPanel* > m_panels;
    QComboBox* m_modeCombo;
    QSpinBox* m_pointsSpin;
    QDoubleSpinBox* m_nanFractionSpin;
    QSpinBox* m_repeatsSpin;
    QPushButton* m_applyBtn;
};

#endif
