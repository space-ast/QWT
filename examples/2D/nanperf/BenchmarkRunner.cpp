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

    // --- boundingRect timing (cold-cache O(n) scan, NOT autoscale — fixed scales used) ---
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

    // --- replot timing (pure render/mapping; LTTB SIMD cliff visible here) ---
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
    // boundingRect: split by baseline vs NaN (position-independent — the O(n)
    // scan visits every sample; NaN points are skipped after an isfinite check).
    double brBaseSum = 0.0, brNanSum = 0.0;
    int brBaseN = 0, brNanN = 0;
    for (const auto& r : results) {
        if (r.caseId == NanCase::Baseline) {
            brBaseSum += r.boundingRectMs;
            ++brBaseN;
        } else {
            brNanSum += r.boundingRectMs;
            ++brNanN;
        }
    }
    const double brBaseAvg = brBaseN ? brBaseSum / brBaseN : 0.0;
    const double brNanAvg  = brNanN ? brNanSum / brNanN : 0.0;

    auto avgReplot = [](const QVector< BenchResult >& rs) {
        double s = 0.0;
        int n    = 0;
        for (const auto& r : rs) {
            s += r.replotMs;
            ++n;
        }
        return n ? s / n : 0.0;
    };

    // LTTB is the only mode with a SIMD fast path (qwtSimdArgMinMax inside
    // qwtMinMaxBucketReduce). When *any* NaN is present, a pre-scan globally
    // disables SIMD and falls back to scalar. The ratio isolates this:
    //   ratio > 1.0  → SIMD cliff dominates (NaN is slower, visible at low NaN%)
    //   ratio < 1.0  → data reduction dominates (NaN is faster, visible at high NaN%)
    // Other modes have no SIMD path — their ratio reflects pure data reduction.
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

    // The differential (lttbRatio - otherRatio) isolates the SIMD cliff from
    // the data-reduction effect shared by all modes.
    const double simdCliffDelta = lttbRatio - otherRatio;

    QString text;
    text += QString("Bottleneck Analysis:\n");

    // 1. boundingRect — cold-cache O(n) scan (NOT autoscale; fixed scales used).
    text += QString(u8"• boundingRect: baseline %1 ms vs NaN %2 ms — "
                    u8"cold-cache O(n) scan (cached in practice, re-computed only on data change)\n")
                .arg(brBaseAvg, 0, 'f', 3)
                .arg(brNanAvg, 0, 'f', 3);

    // 2. LTTB — SIMD cliff.
    text += QString(u8"• FilterPointsLTTB: NaN %1 ms vs baseline %2 ms (%3x) — "
                    u8"qwtMinMaxBucketReduce disables SIMD on any NaN (scalar fallback); "
                    u8"ratio > 1.0 = SIMD cliff dominates, < 1.0 = data reduction dominates\n")
                .arg(lttbNanAvg, 0, 'f', 3)
                .arg(lttbBaseAvg, 0, 'f', 3)
                .arg(lttbRatio, 0, 'f', 2);

    // 3. Other modes — no SIMD, pure data reduction.
    text += QString(u8"• Other modes: NaN %1 ms vs baseline %2 ms (%3x) — "
                    u8"no SIMD path; ratio reflects pure data reduction (fewer finite points to map)\n")
                .arg(otherNanAvg, 0, 'f', 3)
                .arg(otherBaseAvg, 0, 'f', 3)
                .arg(otherRatio, 0, 'f', 2);

    // 4. Differential — isolates SIMD cliff from data reduction.
    // Based on empirical data across NaN ratios (5%, 90%, 95%):
    //   - delta < 0      → LTTB benefits MORE from NaN (SIMD penalty negligible for small buckets)
    //   - 0 <= delta <= 0.1 → SIMD penalty marginal (barely detectable)
    //   - delta > 0.1    → SIMD cliff clearly visible (not yet observed in any test)
    const QString deltaStr = QString::number(simdCliffDelta, 'f', 2);
    const QString sign      = (simdCliffDelta >= 0.0) ? QStringLiteral("+") + deltaStr : deltaStr;

    if (simdCliffDelta > 0.1) {
        text += QString(u8"• LTTB ratio %1x vs other ratio %2x (delta %3) → "
                        u8"SIMD cliff clearly visible: LTTB penalized by scalar fallback at this NaN ratio\n")
                    .arg(lttbRatio, 0, 'f', 2)
                    .arg(otherRatio, 0, 'f', 2)
                    .arg(sign);
    } else if (simdCliffDelta >= 0.0) {
        text += QString(u8"• LTTB ratio %1x vs other ratio %2x (delta %3) → "
                        u8"SIMD penalty marginal at this NaN ratio (scalar overhead negligible for small buckets)\n")
                    .arg(lttbRatio, 0, 'f', 2)
                    .arg(otherRatio, 0, 'f', 2)
                    .arg(sign);
    } else {
        text += QString(u8"• LTTB ratio %1x vs other ratio %2x (delta %3) → "
                        u8"LTTB benefits more from data reduction (SIMD penalty negligible at this NaN ratio)\n")
                    .arg(lttbRatio, 0, 'f', 2)
                    .arg(otherRatio, 0, 'f', 2)
                    .arg(sign);
    }

    return text;
}
