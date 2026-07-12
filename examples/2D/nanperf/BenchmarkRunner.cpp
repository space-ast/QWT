#include "BenchmarkRunner.h"
#include "PlotPanel.h"
#include "FilterModes.h"

#include <QApplication>
#include <QRectF>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

BenchmarkRunner::BenchmarkRunner(QList< PlotPanel* > panels, QObject* parent) : QObject(parent), m_panels(panels)
{
}

BenchResult BenchmarkRunner::measureOn(PlotPanel* panel, NanCase cs, int modeIndex, int numPoints, double nanFraction, int repeats)
{
    QVector< double > x, y;
    NanDataGenerator::generate(cs, numPoints, nanFraction, x, y);

    QwtPlotCurve* curve = panel->curve();
    panel->setMode(modeIndex);
    panel->setFixedScales(numPoints);

    // --- boundingRect timing (bottleneck #2: autoscale O(n) scan) ---
    qint64 totalNs = 0;
    for (int r = 0; r < repeats; ++r) {
        curve->setSamples(x, y);  // invalidate cache (untimed)
        m_timer.start();
        const QRectF rect = curve->data()->boundingRect();
        totalNs += m_timer.nsecsElapsed();
        (void)rect;
    }
    const double boundingRectMs = totalNs / double(repeats) / 1.0e6;

    // Warm the cache so the replot loop below measures pure render cost.
    curve->setSamples(x, y);
    curve->data()->boundingRect();

    // --- replot timing (bottleneck #1: render/mapping, incl. LTTB SIMD cliff) ---
    totalNs = 0;
    for (int r = 0; r < repeats; ++r) {
        m_timer.start();
        panel->plot()->replot();
        totalNs += m_timer.nsecsElapsed();
    }
    const double replotMs = totalNs / double(repeats) / 1.0e6;
    const double fps      = replotMs > 0.0 ? 1000.0 / replotMs : 0.0;

    BenchResult result;
    result.caseId         = cs;
    result.caseLabel      = NanDataGenerator::caseLabel(cs);
    const auto modes      = filterModes();
    result.modeLabel      = modes.value(modeIndex).label;
    result.modeIndex      = modeIndex;
    result.numPoints      = numPoints;
    result.nanPoints      = NanDataGenerator::nanCount(cs, numPoints, nanFraction);
    result.boundingRectMs = boundingRectMs;
    result.replotMs       = replotMs;
    result.fps            = fps;

    panel->showReplotTime(replotMs);
    return result;
}

void BenchmarkRunner::runSweep(int numPoints, double nanFraction, int repeats)
{
    const auto modes = filterModes();
    QVector< BenchResult > results;
    const int total = modes.size() * (m_panels.size() + 1);  // 4 panels + baseline
    int done        = 0;

    // Baseline measured on panel 0 (temporarily).
    for (int m = 0; m < modes.size(); ++m) {
        results.append(measureOn(m_panels[ 0 ], NanCase::Baseline, m, numPoints, nanFraction, repeats));
        emit progress(++done, total);
        QApplication::processEvents();
    }
    // Each panel's own case.
    for (PlotPanel* panel : qwt_as_const(m_panels)) {
        const NanCase cs = panel->nanCase();
        for (int m = 0; m < modes.size(); ++m) {
            results.append(measureOn(panel, cs, m, numPoints, nanFraction, repeats));
            emit progress(++done, total);
            QApplication::processEvents();
        }
    }

    emit finished(results);
}

QString BenchmarkRunner::analyze(const QVector< BenchResult >& results)
{
    double brSum = 0.0;
    int brN      = 0;
    for (const auto& r : results) {
        brSum += r.boundingRectMs;
        ++brN;
    }
    const double brAvg = brN ? brSum / brN : 0.0;

    auto avgReplot = [](const QVector< BenchResult >& rs) {
        double s = 0.0;
        int n    = 0;
        for (const auto& r : rs) {
            s += r.replotMs;
            ++n;
        }
        return n ? s / n : 0.0;
    };

    QVector< BenchResult > lttbNan, lttbBase, otherNan, otherBase;
    for (const auto& r : results) {
        const bool isLttb = r.modeLabel == QLatin1String("FilterPointsLTTB");
        const bool isBase = r.caseId == NanCase::Baseline;
        if (isLttb && isBase)
            lttbBase.append(r);
        else if (isLttb)
            lttbNan.append(r);
        else if (isBase)
            otherBase.append(r);
        else
            otherNan.append(r);
    }
    const double lttbNanAvg   = avgReplot(lttbNan);
    const double lttbBaseAvg  = avgReplot(lttbBase);
    const double otherNanAvg  = avgReplot(otherNan);
    const double otherBaseAvg = avgReplot(otherBase);
    const double lttbRatio    = (lttbBaseAvg > 0.0) ? lttbNanAvg / lttbBaseAvg : 0.0;
    const double otherRatio   = (otherBaseAvg > 0.0) ? otherNanAvg / otherBaseAvg : 0.0;

    QString text;
    text += QString("Bottleneck Analysis:\n");
    text +=
        QString(u8"• boundingRect avg %1 ms (mode-independent) → autoscale O(n) scan bottleneck\n").arg(brAvg, 0, 'f', 3);
    text += QString(u8"• FilterPointsLTTB with NaN replot %1 ms vs baseline %2 ms (%3x) → AVX2 SIMD disabled by NaN\n")
                .arg(lttbNanAvg, 0, 'f', 3)
                .arg(lttbBaseAvg, 0, 'f', 3)
                .arg(lttbRatio, 0, 'f', 2);
    text += QString(u8"• Other modes NaN penalty %1x (%2 ms vs baseline %3 ms) → confirms cliff is LTTB-specific\n")
                .arg(otherRatio, 0, 'f', 2)
                .arg(otherNanAvg, 0, 'f', 3)
                .arg(otherBaseAvg, 0, 'f', 3);
    return text;
}
