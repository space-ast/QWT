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
    void testSimdCliffClearlyVisible();
    void testSimdPenaltyMarginal();
    void testLttbBenefitsMore();
};

/// Scenario: large positive delta (> 0.1) — SIMD cliff clearly visible.
/// LTTB with NaN is *slower* than baseline; scalar fallback dominates.
void TestBenchmarkAnalysis::testSimdCliffClearlyVisible()
{
    QVector< BenchResult > results;
    // LTTB: base 10ms, NaN 30ms -> ratio 3.00 (NaN is slower)
    // other modes: base 10ms, NaN 12ms -> ratio 1.20
    // delta = 3.00 - 1.20 = +1.80
    const QVector< NanCase > nanCases = { NanCase::Leading, NanCase::LeadingTrailing, NanCase::Middle, NanCase::Trailing, NanCase::XyNan, NanCase::XyInterleavedNan };
    for (NanCase cs : nanCases) {
        results << mk(cs, "FilterPointsLTTB", 5.0, 30.0);
        results << mk(cs, "ClipPolygons", 5.0, 12.0);
        results << mk(cs, "FilterPoints", 5.0, 12.0);
    }
    results << mk(NanCase::Baseline, "FilterPointsLTTB", 5.0, 10.0);
    results << mk(NanCase::Baseline, "ClipPolygons", 5.0, 10.0);
    results << mk(NanCase::Baseline, "FilterPoints", 5.0, 10.0);

    const QString text = BenchmarkRunner::analyze(results);

    QVERIFY(text.contains(QStringLiteral("cold-cache")));
    QVERIFY(text.contains(QStringLiteral("3.00")));
    QVERIFY(text.contains(QStringLiteral("SIMD")));
    QVERIFY(text.contains(QStringLiteral("1.20")));
    QVERIFY(text.contains(QStringLiteral("+1.80")));
    QVERIFY(text.contains(QStringLiteral("SIMD cliff clearly visible")));
}

/// Scenario: small positive delta (0 <= delta <= 0.1) — SIMD penalty marginal.
/// Matches high-NaN-ratio empirical data (e.g. 90-95% NaN).
void TestBenchmarkAnalysis::testSimdPenaltyMarginal()
{
    QVector< BenchResult > results;
    // LTTB: base 12.0ms, NaN 4.0ms -> ratio 0.33
    // other modes: base 64.0ms, NaN 18.0ms -> ratio 0.28
    // delta = 0.33 - 0.28 = +0.05
    const QVector< NanCase > nanCases = { NanCase::Leading, NanCase::LeadingTrailing, NanCase::Middle, NanCase::Trailing, NanCase::XyNan, NanCase::XyInterleavedNan };
    for (NanCase cs : nanCases) {
        results << mk(cs, "FilterPointsLTTB", 14.0, 4.0);
        results << mk(cs, "ClipPolygons", 14.0, 18.0);
        results << mk(cs, "FilterPoints", 14.0, 18.0);
    }
    results << mk(NanCase::Baseline, "FilterPointsLTTB", 25.0, 12.0);
    results << mk(NanCase::Baseline, "ClipPolygons", 25.0, 64.0);
    results << mk(NanCase::Baseline, "FilterPoints", 25.0, 64.0);

    const QString text = BenchmarkRunner::analyze(results);

    QVERIFY(text.contains(QStringLiteral("0.33")));
    QVERIFY(text.contains(QStringLiteral("data reduction dominates")));
    QVERIFY(text.contains(QStringLiteral("0.28")));
    QVERIFY(text.contains(QStringLiteral("+0.05")));
    QVERIFY(text.contains(QStringLiteral("SIMD penalty marginal")));
}

/// Scenario: negative delta (< 0) — LTTB benefits more from data reduction.
/// Matches low-NaN-ratio empirical data (e.g. 5% NaN).
void TestBenchmarkAnalysis::testLttbBenefitsMore()
{
    QVector< BenchResult > results;
    // LTTB: base 16.0ms, NaN 13.0ms -> ratio 0.81
    // other modes: base 63.0ms, NaN 60.0ms -> ratio 0.95
    // delta = 0.81 - 0.95 = -0.14
    const QVector< NanCase > nanCases = { NanCase::Leading, NanCase::LeadingTrailing, NanCase::Middle, NanCase::Trailing, NanCase::XyNan, NanCase::XyInterleavedNan };
    for (NanCase cs : nanCases) {
        results << mk(cs, "FilterPointsLTTB", 10.0, 13.0);
        results << mk(cs, "ClipPolygons", 10.0, 60.0);
        results << mk(cs, "FilterPoints", 10.0, 60.0);
    }
    results << mk(NanCase::Baseline, "FilterPointsLTTB", 10.0, 16.0);
    results << mk(NanCase::Baseline, "ClipPolygons", 10.0, 63.0);
    results << mk(NanCase::Baseline, "FilterPoints", 10.0, 63.0);

    const QString text = BenchmarkRunner::analyze(results);

    QVERIFY(text.contains(QStringLiteral("0.81")));
    QVERIFY(text.contains(QStringLiteral("data reduction dominates")));
    QVERIFY(text.contains(QStringLiteral("0.95")));
    QVERIFY(text.contains(QStringLiteral("-0.14")));
    QVERIFY(text.contains(QStringLiteral("LTTB benefits more from data reduction")));
}

QTEST_MAIN(TestBenchmarkAnalysis)

#include "tst_BenchmarkAnalysis.moc"
