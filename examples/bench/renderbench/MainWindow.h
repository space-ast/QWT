/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QMainWindow>
#include <QElapsedTimer>
#include <QVector>

class Plot;
class ControlPanel;
class WindowedSeriesData;

struct BenchmarkResult
{
    QString methodName;
    int numPoints    = 0;
    int frameCount   = 0;
    int waveType     = 0;
    double totalTime = 0;
    double avgTime   = 0;
    double minTime   = 0;
    double maxTime   = 0;
    double fps       = 0;
    QVector< double > frameTimes;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void startBenchmark();
    void startBenchmarkAll();
    void onTimerTick();

private:
    void generateData(int numPoints, int waveType);
    BenchmarkResult runBenchmark(int methodIndex);
    QString buildReport(const BenchmarkResult& result) const;
    QString buildCompareReport(const QVector< BenchmarkResult >& results) const;
    void showReport(const QString& markdown);
    QString methodName(int index) const;

    Plot* m_plot;
    ControlPanel* m_panel;
    WindowedSeriesData* m_seriesData;

    int m_currentFrame  = 0;
    int m_targetFrames  = 0;
    int m_currentOffset = 0;
    int m_step          = 0;
    int m_displaySize   = 0;

    QElapsedTimer m_frameTimer;
    QVector< double > m_frameTimes;

    bool m_batchMode = false;
    int m_batchMethodIndex = 0;
    QVector< BenchmarkResult > m_batchResults;
};
