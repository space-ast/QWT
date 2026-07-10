#ifndef BENCHMARK_RUNNER_H
#define BENCHMARK_RUNNER_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QList>
#include "NanDataGenerator.h"

class PlotPanel;

/// One measured (case x mode) data point.
struct BenchResult
{
    NanCase caseId;
    QString caseLabel;
    QString modeLabel;
    int modeIndex;
    int numPoints;
    int nanPoints;
    double boundingRectMs;
    double replotMs;
    double fps;
};

/// Measures boundingRect + replot cost on the displayed panels and runs the
/// full 5-mode x 5-case sweep. The analyze() static is pure and unit-tested.
class BenchmarkRunner : public QObject
{
    Q_OBJECT
public:
    BenchmarkRunner(QList< PlotPanel* > panels, QObject* parent = nullptr);

    /// Run the full sweep on the GUI thread (calls processEvents for progress).
    void runSweep(int numPoints, double nanFraction, int repeats);

    /// Pure bottleneck analysis from collected results.
    static QString analyze(const QVector< BenchResult >& results);

Q_SIGNALS:
    void progress(int done, int total);
    void finished(const QVector< BenchResult >& results);

private:
    BenchResult measureOn(PlotPanel* panel, NanCase cs, int modeIndex, int numPoints, double nanFraction, int repeats);

    QList< PlotPanel* > m_panels;
    QElapsedTimer m_timer;
};

#endif
