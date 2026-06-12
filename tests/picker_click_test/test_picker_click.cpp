/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#include "test_picker_click.h"
#include <QTest>
#include <QSignalSpy>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotSeriesDataPicker>

void TestPickerClick::testClickedBasic()
{
    // Create plot and canvas
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    // Add a curve with some data
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    // Create picker on canvas
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Create signal spy for clicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy clickedSpy(&picker, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse click at center of canvas
    QTest::mouseClick(plot.canvas(), Qt::LeftButton, Qt::NoModifier, 
                     QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify signal was emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(clickedSpy.count() == 1);
    
    // Verify signal parameters
    QList<QVariant> args = clickedSpy.takeFirst();
    QVERIFY(args.at(0).value<QwtPlotSeriesDataPicker*>() == &picker);
    QVERIFY(args.at(1).value<QPoint>() == QPoint(200, 150));
}

void TestPickerClick::testDoubleClickedBasic()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Create signal spy for doubleClicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy doubleClickedSpy(&picker, SIGNAL(doubleClicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse double click
    QTest::mouseDClick(plot.canvas(), Qt::LeftButton, Qt::NoModifier,
                        QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify signal was emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(doubleClickedSpy.count() == 1);
    
    // Verify signal parameters
    QList<QVariant> args = doubleClickedSpy.takeFirst();
    QVERIFY(args.at(0).value<QwtPlotSeriesDataPicker*>() == &picker);
    QVERIFY(args.at(1).value<QPoint>() == QPoint(200, 150));
}

void TestPickerClick::testDoubleClickAlsoFiresClicked()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Create spies for both signals
    // NOTE: These signals don't exist yet - test should FAIL
    QSignalSpy clickedSpy(&picker, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    QSignalSpy doubleClickedSpy(&picker, SIGNAL(doubleClicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse double click
    QTest::mouseDClick(plot.canvas(), Qt::LeftButton, Qt::NoModifier,
                        QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify both signals were emitted (double click should also emit single click)
    // This will FAIL because signals don't exist yet (TDD RED)
    QVERIFY(clickedSpy.count() == 1);
    QVERIFY(doubleClickedSpy.count() == 1);
    
    // Verify parameters match
    QList<QVariant> clickArgs = clickedSpy.takeFirst();
    QList<QVariant> doubleClickArgs = doubleClickedSpy.takeFirst();
    
    QVERIFY(clickArgs.at(0).value<QwtPlotSeriesDataPicker*>() == &picker);
    QVERIFY(clickArgs.at(1).value<QPoint>() == QPoint(200, 150));
    QVERIFY(doubleClickArgs.at(0).value<QwtPlotSeriesDataPicker*>() == &picker);
    QVERIFY(doubleClickArgs.at(1).value<QPoint>() == QPoint(200, 150));
}

void TestPickerClick::testRightClickNoSignal()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Create signal spy
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy clickedSpy(&picker, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate right mouse click (should NOT trigger signal)
    QTest::mouseClick(plot.canvas(), Qt::RightButton, Qt::NoModifier,
                     QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify signal was NOT emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(clickedSpy.count() == 0);
}

void TestPickerClick::testMiddleClickNoSignal()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Create signal spy
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy clickedSpy(&picker, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate middle mouse click (should NOT trigger signal)
    QTest::mouseClick(plot.canvas(), Qt::MiddleButton, Qt::NoModifier,
                     QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify signal was NOT emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(clickedSpy.count() == 0);
}

void TestPickerClick::testDisabledPickerNoSignal()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(false); // Disabled picker
    
    // Create signal spy
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy clickedSpy(&picker, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse click
    QTest::mouseClick(plot.canvas(), Qt::LeftButton, Qt::NoModifier,
                     QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify signal was NOT emitted (picker is disabled)
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(clickedSpy.count() == 0);
}

void TestPickerClick::testFeaturePointsGetterReturnsData()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Call featurePoints() getter
    // NOTE: This method doesn't exist yet - test should FAIL at compile time
    QList<QwtPlotSeriesDataPicker::FeaturePoint> featurePoints = picker.featurePoints();
    
    // Verify it returns something (implementation should return collected points)
    // This will FAIL because method doesn't exist yet (TDD RED)
    QVERIFY(!featurePoints.isEmpty());
}

void TestPickerClick::testFeaturePointsGetterEmpty()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // No curve attached, so no data to pick
    
    // Call featurePoints() getter
    // NOTE: This method doesn't exist yet - test should FAIL at compile time
    QList<QwtPlotSeriesDataPicker::FeaturePoint> featurePoints = picker.featurePoints();
    
    // Verify it returns empty when no data
    // This will FAIL because method doesn't exist yet (TDD RED)
    QVERIFY(featurePoints.isEmpty());
}

void TestPickerClick::testFeaturePointsGetterAtClickPosition()
{
    QwtPlot plot;
    plot.resize(400, 300);
    plot.show();
    QApplication::processEvents();
    
    QwtPlotCurve curve;
    QVector<QPointF> data;
    data << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve.setSamples(data);
    curve.attach(&plot);
    plot.replot();
    
    QwtPlotSeriesDataPicker picker(plot.canvas());
    picker.setEnabled(true);
    
    // Create signal spy for clicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy clickedSpy(&picker, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Click at a position
    QPoint clickPos(200, 150);
    QTest::mouseClick(plot.canvas(), Qt::LeftButton, Qt::NoModifier, clickPos);
    QApplication::processEvents();
    
    // Verify signal was emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(clickedSpy.count() == 1);
    
    // Call featurePoints() getter after click
    // NOTE: This method doesn't exist yet - test should FAIL at compile time
    QList<QwtPlotSeriesDataPicker::FeaturePoint> featurePoints = picker.featurePoints();
    
    // Verify it returns points at click position
    // This will FAIL because method doesn't exist yet (TDD RED)
    QVERIFY(!featurePoints.isEmpty());
    
    // Verify at least one feature point has the curve as item
    bool hasCurvePoint = false;
    for (const auto& fp : featurePoints) {
        if (fp.item == &curve) {
            hasCurvePoint = true;
            break;
        }
    }
    QVERIFY(hasCurvePoint);
}

QTEST_MAIN(TestPickerClick)