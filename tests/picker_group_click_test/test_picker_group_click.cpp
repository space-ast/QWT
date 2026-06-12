/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#include "test_picker_group_click.h"
#include <QTest>
#include <QSignalSpy>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotSeriesDataPicker>
#include <QwtPlotSeriesDataPickerGroup>

void TestPickerGroupClick::testGroupClickedForward()
{
    // Create two plots with pickers
    QwtPlot plot1;
    plot1.resize(400, 300);
    plot1.show();
    
    QwtPlot plot2;
    plot2.resize(400, 300);
    plot2.show();
    
    QApplication::processEvents();
    
    // Add curves to both plots
    QwtPlotCurve curve1;
    QVector<QPointF> data1;
    data1 << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve1.setSamples(data1);
    curve1.attach(&plot1);
    plot1.replot();
    
    QwtPlotCurve curve2;
    QVector<QPointF> data2;
    data2 << QPointF(0, 0) << QPointF(1, 2) << QPointF(2, 8) << QPointF(3, 18);
    curve2.setSamples(data2);
    curve2.attach(&plot2);
    plot2.replot();
    
    // Create pickers on both canvases
    QwtPlotSeriesDataPicker picker1(plot1.canvas());
    picker1.setEnabled(true);
    
    QwtPlotSeriesDataPicker picker2(plot2.canvas());
    picker2.setEnabled(true);
    
    // Create group and add both pickers
    QwtPlotSeriesDataPickerGroup group;
    group.addPicker(&picker1);
    group.addPicker(&picker2);
    group.setEnabled(true);
    
    // Create signal spy for group's clicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy groupClickedSpy(&group, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse click on picker1's canvas
    QTest::mouseClick(plot1.canvas(), Qt::LeftButton, Qt::NoModifier, 
                     QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify group's clicked signal was emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(groupClickedSpy.count() == 1);
    
    // Verify signal parameters
    QList<QVariant> args = groupClickedSpy.takeFirst();
    QVERIFY(args.at(0).value<QwtPlotSeriesDataPicker*>() == &picker1);
    QVERIFY(args.at(1).value<QPoint>() == QPoint(200, 150));
}

void TestPickerGroupClick::testGroupDoubleClickedForward()
{
    // Create two plots with pickers
    QwtPlot plot1;
    plot1.resize(400, 300);
    plot1.show();
    
    QwtPlot plot2;
    plot2.resize(400, 300);
    plot2.show();
    
    QApplication::processEvents();
    
    // Add curves to both plots
    QwtPlotCurve curve1;
    QVector<QPointF> data1;
    data1 << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve1.setSamples(data1);
    curve1.attach(&plot1);
    plot1.replot();
    
    QwtPlotCurve curve2;
    QVector<QPointF> data2;
    data2 << QPointF(0, 0) << QPointF(1, 2) << QPointF(2, 8) << QPointF(3, 18);
    curve2.setSamples(data2);
    curve2.attach(&plot2);
    plot2.replot();
    
    // Create pickers on both canvases
    QwtPlotSeriesDataPicker picker1(plot1.canvas());
    picker1.setEnabled(true);
    
    QwtPlotSeriesDataPicker picker2(plot2.canvas());
    picker2.setEnabled(true);
    
    // Create group and add both pickers
    QwtPlotSeriesDataPickerGroup group;
    group.addPicker(&picker1);
    group.addPicker(&picker2);
    group.setEnabled(true);
    
    // Create signal spy for group's doubleClicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy groupDoubleClickedSpy(&group, SIGNAL(doubleClicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse double click on picker1's canvas
    QTest::mouseDClick(plot1.canvas(), Qt::LeftButton, Qt::NoModifier,
                        QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify group's doubleClicked signal was emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(groupDoubleClickedSpy.count() == 1);
    
    // Verify signal parameters
    QList<QVariant> args = groupDoubleClickedSpy.takeFirst();
    QVERIFY(args.at(0).value<QwtPlotSeriesDataPicker*>() == &picker1);
    QVERIFY(args.at(1).value<QPoint>() == QPoint(200, 150));
}

void TestPickerGroupClick::testGroupSyncBeforeSignal()
{
    // CRITICAL TEST: Verify sync happens BEFORE signal emission
    
    // Create two plots with pickers
    QwtPlot plot1;
    plot1.resize(400, 300);
    plot1.show();
    
    QwtPlot plot2;
    plot2.resize(400, 300);
    plot2.show();
    
    QApplication::processEvents();
    
    // Add curves to both plots
    QwtPlotCurve curve1;
    QVector<QPointF> data1;
    data1 << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve1.setSamples(data1);
    curve1.attach(&plot1);
    plot1.replot();
    
    QwtPlotCurve curve2;
    QVector<QPointF> data2;
    data2 << QPointF(0, 0) << QPointF(1, 2) << QPointF(2, 8) << QPointF(3, 18);
    curve2.setSamples(data2);
    curve2.attach(&plot2);
    plot2.replot();
    
    // Create pickers on both canvases
    QwtPlotSeriesDataPicker picker1(plot1.canvas());
    picker1.setEnabled(true);
    
    QwtPlotSeriesDataPicker picker2(plot2.canvas());
    picker2.setEnabled(true);
    
    // Create group and add both pickers
    QwtPlotSeriesDataPickerGroup group;
    group.addPicker(&picker1);
    group.addPicker(&picker2);
    group.setEnabled(true);
    
    // Variable to store sync verification data
    QPoint picker2SyncedPosition(-1, -1);
    bool syncVerified = false;
    
    // Connect to group's clicked signal to verify sync happened BEFORE signal
    // NOTE: This signal doesn't exist yet - test should FAIL at compile time
    QObject::connect(&group, &QwtPlotSeriesDataPickerGroup::clicked,
                     [&](QwtPlotSeriesDataPicker* sourcePicker, const QPoint& pos) {
                         // When this slot is called, picker2 should already be synced
                         QPoint trackerPos = picker2.trackerPosition();
                         
                         // Calculate expected proportional position
                         double xPresent = qBound(0.0, (double)pos.x() / plot1.canvas()->width(), 1.0);
                         double yPresent = qBound(0.0, (double)pos.y() / plot1.canvas()->height(), 1.0);
                         
                         int expectedX = plot2.canvas()->width() * xPresent;
                         int expectedY = plot2.canvas()->height() * yPresent;
                         
                         picker2SyncedPosition = trackerPos;
                         syncVerified = (trackerPos.x() == expectedX && trackerPos.y() == expectedY);
                     });
    
    // Create signal spy for group's clicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy groupClickedSpy(&group, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse click on picker1's canvas
    QPoint clickPos(200, 150);
    QTest::mouseClick(plot1.canvas(), Qt::LeftButton, Qt::NoModifier, clickPos);
    QApplication::processEvents();
    
    // Verify group's clicked signal was emitted
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(groupClickedSpy.count() == 1);
    
    // Verify sync happened BEFORE signal (syncVerified should be true)
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(syncVerified);
    
    // Also verify picker2's tracker position is at proportional equivalent
    double xPresent = qBound(0.0, (double)clickPos.x() / plot1.canvas()->width(), 1.0);
    double yPresent = qBound(0.0, (double)clickPos.y() / plot1.canvas()->height(), 1.0);
    
    int expectedX = plot2.canvas()->width() * xPresent;
    int expectedY = plot2.canvas()->height() * yPresent;
    
    QVERIFY(picker2SyncedPosition.x() == expectedX);
    QVERIFY(picker2SyncedPosition.y() == expectedY);
}

void TestPickerGroupClick::testGroupDisabledNoSignal()
{
    // Create two plots with pickers
    QwtPlot plot1;
    plot1.resize(400, 300);
    plot1.show();
    
    QwtPlot plot2;
    plot2.resize(400, 300);
    plot2.show();
    
    QApplication::processEvents();
    
    // Add curves to both plots
    QwtPlotCurve curve1;
    QVector<QPointF> data1;
    data1 << QPointF(0, 0) << QPointF(1, 1) << QPointF(2, 4) << QPointF(3, 9);
    curve1.setSamples(data1);
    curve1.attach(&plot1);
    plot1.replot();
    
    QwtPlotCurve curve2;
    QVector<QPointF> data2;
    data2 << QPointF(0, 0) << QPointF(1, 2) << QPointF(2, 8) << QPointF(3, 18);
    curve2.setSamples(data2);
    curve2.attach(&plot2);
    plot2.replot();
    
    // Create pickers on both canvases
    QwtPlotSeriesDataPicker picker1(plot1.canvas());
    picker1.setEnabled(true);
    
    QwtPlotSeriesDataPicker picker2(plot2.canvas());
    picker2.setEnabled(true);
    
    // Create group, add both pickers, but DISABLE the group
    QwtPlotSeriesDataPickerGroup group;
    group.addPicker(&picker1);
    group.addPicker(&picker2);
    group.setEnabled(false); // Group is disabled
    
    // Create signal spy for group's clicked signal
    // NOTE: This signal doesn't exist yet - test should FAIL
    QSignalSpy groupClickedSpy(&group, SIGNAL(clicked(QwtPlotSeriesDataPicker*, QPoint)));
    
    // Simulate left mouse click on picker1's canvas
    QTest::mouseClick(plot1.canvas(), Qt::LeftButton, Qt::NoModifier, 
                     QPoint(200, 150));
    QApplication::processEvents();
    
    // Verify group's clicked signal was NOT emitted (group is disabled)
    // This will FAIL because signal doesn't exist yet (TDD RED)
    QVERIFY(groupClickedSpy.count() == 0);
}

QTEST_MAIN(TestPickerGroupClick)