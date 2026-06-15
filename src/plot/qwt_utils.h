/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

/*!
 * \file qwt_utils.h
 * \brief Qwt 绘图工具类集合和兼容性函数
 *
 * 本文件提供两个主要功能：
 *
 * 1. **绘图工具类的前向包含**：通过包含此文件，可以一次性访问所有 Qwt 绘图工具类。
 *    这些工具类替代了旧的 Qwt::plotItemColor() 函数，提供了更强大和类型安全的功能。
 *
 * 2. **Qt 版本兼容性函数**：qwtExpandedToGlobalStrut() 用于处理 Qt 5 和 Qt 6 之间的
 *    globalStrut API 差异，主要供 Qwt 内部控件类使用。
 *
 * \section 新的绘图工具类
 *
 * 从 Qwt 7.x 开始，推荐使用以下 5 个专用工具类来处理绘图项：
 *
 * - **QwtPlotFactory**：工厂方法，用于创建各种绘图项（曲线、柱状图、标记等）
 * - **QwtPlotItemInfo**：类型查询，用于判断绘图项类型和获取基本信息
 * - **QwtPlotDataAccess**：数据访问，用于读取和设置绘图项的数据
 * - **QwtPlotTransform**：坐标变换，用于像素坐标和数据坐标之间的转换
 * - **QwtPlotStyling**：样式操作，用于获取和设置绘图项的颜色、画笔、画刷等样式
 *
 * 这些类都位于 Qwt 命名空间中，提供了类型安全、rtti 分发的 API，比旧的动态转换方式
 * 更安全、更高效。
 *
 * \section 使用示例
 *
 * \code
 * #include <qwt_utils.h>
 *
 * // 创建绘图
 * QwtPlot* plot = new QwtPlot();
 *
 * // 使用工厂方法创建曲线
 * QVector<QPointF> points = {{0,0}, {1,1}, {2,4}};
 * QwtPlotCurve* curve = QwtPlotFactory::createCurve(plot, "Temperature", points);
 *
 * // 查询类型
 * if (QwtPlotItemInfo::isCurve(item)) {
 *     // 处理曲线
 * }
 *
 * // 读取数据
 * QVector<QPointF> data = QwtPlotDataAccess::getCurveData(curve);
 *
 * // 设置样式
 * QwtPlotStyling::setColor(curve, Qt::blue);
 * QwtPlotStyling::setPenWidth(curve, 2.0);
 *
 * // 坐标变换
 * QPointF plotPos = QwtPlotTransform::toPlotCoordinates(plot, screenPos);
 * \endcode
 *
 * \section 迁移指南
 *
 * 如果你之前使用 Qwt::plotItemColor()，现在应该使用 QwtPlotStyling::color()：
 *
 * \code
 * // 旧方式（已弃用）
 * QColor color = Qwt::plotItemColor(item);
 *
 * // 新方式（推荐）
 * QColor color = QwtPlotStyling::color(item);
 * \endcode
 *
 * 新方式不仅支持更多绘图项类型，而且使用 rtti 分发而非动态转换，性能更好。
 *
 * \note 本文件保留 qwtExpandedToGlobalStrut() 函数仅供 Qwt 内部使用，
 *       应用程序代码不应直接调用此函数。
 */

#ifndef QWT_UTILS_H
#define QWT_UTILS_H

#include "qwt_global.h"

#include <qsize.h>

/*!
 * \name 绘图工具类前向包含
 *
 * 包含以下头文件，提供对所有 Qwt 绘图工具类的访问：
 *
 * - qwt_plot_factory.h：工厂方法
 * - qwt_plot_item_info.h：类型查询
 * - qwt_plot_data_access.h：数据访问
 * - qwt_plot_transform.h：坐标变换
 * - qwt_plot_styling.h：样式操作
 *
 * \note 如果只需要特定的工具类，可以单独包含对应的头文件以减少编译依赖。
 */
//!@{
#include "qwt_plot_factory.h"
#include "qwt_plot_item_info.h"
#include "qwt_plot_data_access.h"
#include "qwt_plot_transform.h"
#include "qwt_plot_styling.h"
//!@}

/*!
 * \brief 将尺寸扩展到全局最小尺寸（Qt5/Qt6 兼容性函数）
 *
 * 在 Qt 5.0-5.14 中，QApplication::globalStrut() 返回应用程序定义的最小控件尺寸。
 * 此函数将给定的尺寸扩展到至少该最小尺寸。
 *
 * 在 Qt 5.15+ 和 Qt 6 中，globalStrut() 已被移除，此函数直接返回原始尺寸不变。
 *
 * \param size 原始尺寸
 * \return 扩展到全局最小尺寸后的尺寸，或在 Qt 5.15+/Qt 6 中返回原始尺寸
 *
 * \internal
 * \warning 此函数仅供 Qwt 内部控件类使用（如 QwtDial、QwtKnob、QwtSlider 等），
 *          应用程序代码不应调用此函数。
 *
 * \since Qwt 6.0
 */
QSize QWT_EXPORT qwtExpandedToGlobalStrut(const QSize& size);

#endif // QWT_UTILS_H
