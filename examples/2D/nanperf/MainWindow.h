#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QList>
#include <QVector>
#include "NanDataGenerator.h"
#include "BenchmarkRunner.h"

class PlotPanel;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QProgressBar;
class QTableWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onApplyRedraw();
    void onRunSweep();
    void onExport();
    void onProgress(int done, int total);
    void onSweepFinished(const QVector< BenchResult >& results);

private:
    QLayout* buildControls();
    void refreshPanels();
    void fillTable(const QVector< BenchResult >& results);

    QList< PlotPanel* > m_panels;
    QComboBox* m_modeCombo;
    QSpinBox* m_pointsSpin;
    QDoubleSpinBox* m_nanFractionSpin;
    QSpinBox* m_repeatsSpin;
    QPushButton* m_applyBtn;
    QPushButton* m_sweepBtn;
    QPushButton* m_exportBtn;
    QProgressBar* m_progress;
    QTableWidget* m_table;
    QLabel* m_analysisLabel;
    BenchmarkRunner* m_runner;
    QVector< BenchResult > m_lastResults;
};

#endif
