/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#ifndef TEST_PICKER_CLICK_H
#define TEST_PICKER_CLICK_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotSeriesDataPicker>

/**
 * \if ENGLISH
 * @brief Test class for QwtPlotSeriesDataPicker click/double-click signals and featurePoints getter
 * @details This test class verifies the clicked() and doubleClicked() signals
 *          and the featurePoints() getter method of QwtPlotSeriesDataPicker.
 *          Tests are written in TDD RED phase - they should FAIL because
 *          the implementation doesn't exist yet.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotSeriesDataPicker 点击/双击信号和 featurePoints getter 的测试类
 * @details 这个测试类验证 QwtPlotSeriesDataPicker 的 clicked() 和 doubleClicked() 信号
 *          以及 featurePoints() getter 方法。
 *          测试写在 TDD RED 阶段 - 它们应该失败，因为实现还不存在。
 * \endif
 */
class TestPickerClick : public QObject
{
    Q_OBJECT

private slots:
    /// Test basic left click signal
    void testClickedBasic();
    
    /// Test basic left double click signal  
    void testDoubleClickedBasic();
    
    /// Test that double click also fires single click signal
    void testDoubleClickAlsoFiresClicked();
    
    /// Test that right click does NOT fire signal
    void testRightClickNoSignal();
    
    /// Test that middle click does NOT fire signal
    void testMiddleClickNoSignal();
    
    /// Test that disabled picker does NOT fire signals
    void testDisabledPickerNoSignal();
    
    /// Test featurePoints getter returns data
    void testFeaturePointsGetterReturnsData();
    
    /// Test featurePoints getter returns empty when no data
    void testFeaturePointsGetterEmpty();
    
    /// Test featurePoints getter returns points at click position
    void testFeaturePointsGetterAtClickPosition();
};

#endif // TEST_PICKER_CLICK_H