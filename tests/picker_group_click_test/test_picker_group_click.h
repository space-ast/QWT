/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#ifndef TEST_PICKER_GROUP_CLICK_H
#define TEST_PICKER_GROUP_CLICK_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotSeriesDataPicker>
#include <QwtPlotSeriesDataPickerGroup>

/**
 * \if ENGLISH
 * @brief Test class for QwtPlotSeriesDataPickerGroup click/double-click signal forwarding and sync-before-signal behavior
 * @details This test class verifies the clicked() and doubleClicked() signal forwarding
 *          and sync-before-signal behavior of QwtPlotSeriesDataPickerGroup.
 *          Tests are written in TDD RED phase - they should FAIL because
 *          the implementation doesn't exist yet.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotSeriesDataPickerGroup 点击/双击信号转发和同步前信号行为的测试类
 * @details 这个测试类验证 QwtPlotSeriesDataPickerGroup 的 clicked() 和 doubleClicked() 信号转发
 *          以及同步前信号行为。
 *          测试写在 TDD RED 阶段 - 它们应该失败，因为实现还不存在。
 * \endif
 */
class TestPickerGroupClick : public QObject
{
    Q_OBJECT

private slots:
    /// Test group clicked signal forwarding
    void testGroupClickedForward();
    
    /// Test group double clicked signal forwarding  
    void testGroupDoubleClickedForward();
    
    /// Test group sync-before-signal behavior (critical)
    void testGroupSyncBeforeSignal();
    
    /// Test group disabled state prevents signal forwarding
    void testGroupDisabledNoSignal();
};

#endif // TEST_PICKER_GROUP_CLICK_H