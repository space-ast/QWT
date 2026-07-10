#include <QtTest/QtTest>
#include <cmath>
#include "NanDataGenerator.h"

class TestNanDataGenerator : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLeadingNan();
    void testLeadingTrailingNan();
    void testMiddleNan();
    void testTrailingNan();
    void testBaselineNoNan();
    void testXAlwaysFinite();
};

void TestNanDataGenerator::testLeadingNan()
{
    QVector< double > x, y;
    NanDataGenerator::generate(NanCase::Leading, 1000, 0.5, x, y);
    QCOMPARE(x.size(), 1000);
    QCOMPARE(y.size(), 1000);
    QCOMPARE(NanDataGenerator::nanCount(NanCase::Leading, 1000, 0.5), 500);
    QVERIFY(std::isnan(y[ 0 ]));
    QVERIFY(std::isnan(y[ 499 ]));
    QVERIFY(!std::isnan(y[ 500 ]));
    QVERIFY(!std::isnan(y[ 999 ]));
}

void TestNanDataGenerator::testLeadingTrailingNan()
{
    QVector< double > x, y;
    NanDataGenerator::generate(NanCase::LeadingTrailing, 1000, 0.5, x, y);
    const int expected = 2 * (500 / 2);  // == 500
    QCOMPARE(NanDataGenerator::nanCount(NanCase::LeadingTrailing, 1000, 0.5), expected);
    QVERIFY(std::isnan(y[ 0 ]));
    QVERIFY(!std::isnan(y[ 250 ]));
    QVERIFY(!std::isnan(y[ 749 ]));
    QVERIFY(std::isnan(y[ 999 ]));
}

void TestNanDataGenerator::testMiddleNan()
{
    QVector< double > x, y;
    NanDataGenerator::generate(NanCase::Middle, 1000, 0.5, x, y);
    QCOMPARE(NanDataGenerator::nanCount(NanCase::Middle, 1000, 0.5), 500);
    // midStart = (1000-500)/2 = 250 -> NaN in [250,750)
    QVERIFY(!std::isnan(y[ 249 ]));
    QVERIFY(std::isnan(y[ 250 ]));
    QVERIFY(std::isnan(y[ 749 ]));
    QVERIFY(!std::isnan(y[ 750 ]));
}

void TestNanDataGenerator::testTrailingNan()
{
    QVector< double > x, y;
    NanDataGenerator::generate(NanCase::Trailing, 1000, 0.5, x, y);
    QCOMPARE(NanDataGenerator::nanCount(NanCase::Trailing, 1000, 0.5), 500);
    QVERIFY(!std::isnan(y[ 499 ]));
    QVERIFY(std::isnan(y[ 500 ]));
    QVERIFY(std::isnan(y[ 999 ]));
}

void TestNanDataGenerator::testBaselineNoNan()
{
    QVector< double > x, y;
    NanDataGenerator::generate(NanCase::Baseline, 1000, 0.5, x, y);
    QCOMPARE(NanDataGenerator::nanCount(NanCase::Baseline, 1000, 0.5), 0);
    for (int i = 0; i < y.size(); ++i)
        QVERIFY2(!std::isnan(y[ i ]), qPrintable(QString("y[%1] is NaN").arg(i)));
}

void TestNanDataGenerator::testXAlwaysFinite()
{
    QVector< double > x, y;
    NanDataGenerator::generate(NanCase::Leading, 1000, 0.5, x, y);
    for (int i = 0; i < x.size(); ++i)
        QCOMPARE(x[ i ], double(i));
}

QTEST_MAIN(TestNanDataGenerator)

#include "tst_NanDataGenerator.moc"
