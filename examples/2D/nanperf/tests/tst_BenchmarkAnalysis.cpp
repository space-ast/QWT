#include <QtTest/QtTest>
#include "BenchmarkRunner.h"

static BenchResult mk(NanCase cs, const QString& mode, double br, double rp)
{
    BenchResult r;
    r.caseId         = cs;
    r.caseLabel      = NanDataGenerator::caseLabel(cs);
    r.modeLabel      = mode;
    r.modeIndex      = 0;
    r.numPoints      = 100000;
    r.nanPoints      = 0;
    r.boundingRectMs = br;
    r.replotMs       = rp;
    r.fps            = rp > 0 ? 1000.0 / rp : 0;
    return r;
}

class TestBenchmarkAnalysis : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAnalyzeDetectsLttbCliff();
};

void TestBenchmarkAnalysis::testAnalyzeDetectsLttbCliff()
{
    QVector< BenchResult > results;
    // boundingRect mode-independent at 5.0 ms
    // LTTB: base 10ms, NaN 30ms -> ratio 3.00
    // other modes: base 10ms, NaN 12ms -> ratio 1.20
    const QVector< NanCase > nanCases = { NanCase::Leading, NanCase::LeadingTrailing, NanCase::Middle, NanCase::Trailing };
    for (NanCase cs : nanCases) {
        results << mk(cs, "FilterPointsLTTB", 5.0, 30.0);
        results << mk(cs, "ClipPolygons", 5.0, 12.0);
        results << mk(cs, "FilterPoints", 5.0, 12.0);
    }
    results << mk(NanCase::Baseline, "FilterPointsLTTB", 5.0, 10.0);
    results << mk(NanCase::Baseline, "ClipPolygons", 5.0, 10.0);
    results << mk(NanCase::Baseline, "FilterPoints", 5.0, 10.0);

    const QString text = BenchmarkRunner::analyze(results);
    QVERIFY(text.contains(QStringLiteral("SIMD")));
    QVERIFY(text.contains(QStringLiteral("3.00")));   // LTTB cliff ratio
    QVERIFY(text.contains(QStringLiteral("1.20")));   // other modes ratio
    QVERIFY(text.contains(QStringLiteral("5.000")));  // boundingRect avg
}

QTEST_MAIN(TestBenchmarkAnalysis)

#include "tst_BenchmarkAnalysis.moc"
