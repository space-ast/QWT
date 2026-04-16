/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *   Summary of major modifications (see ChangeLog.md for full history):
 *   1. CMake build system & C++11 throughout.
 *   2. Core panner/ zoomer refactored:
 *        - QwtPanner → QwtCachePanner (pixmap-cache version)
 *        - New real-time QwtPlotPanner derived from QwtPicker.
 *   3. Zoomer supports multi-axis.
 *   4. Parasite-plot framework:
 *        - QwtFigure, QwtPlotParasiteLayout, QwtPlotTransparentCanvas,
 *        - QwtPlotScaleEventDispatcher, built-in pan/zoom on axis.
 *   5. New picker: QwtPlotSeriesDataPicker (works with date axis).
 *   6. Raster & color-map extensions:
 *        - QwtGridRasterData (2-D table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 *****************************************************************************/

#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_plot_magnifier.h"

class QwtPlotMagnifier::PrivateData
{
public:
    PrivateData()
    {
        for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
            isAxisEnabled[ axis ] = true;
    }

    bool isAxisEnabled[ QwtAxis::AxisPositions ];
};

/*!
 * \if ENGLISH
 * @brief Constructor
 * @param canvas Plot canvas to be magnified
 * 
 * Creates a magnifier attached to the given plot canvas.
 * All axes are enabled by default.
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param canvas 要进行放大操作的绘图画布
 * 
 * 创建一个附加到指定绘图画布的放大器。
 * 默认情况下所有坐标轴都是启用状态。
 * \endif
 */
QwtPlotMagnifier::QwtPlotMagnifier(QWidget* canvas) : QwtMagnifier(canvas)
{
    m_data = new PrivateData();
}

/*!
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * 
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPlotMagnifier::~QwtPlotMagnifier()
{
    delete m_data;
}

/*!
 * \if ENGLISH
 * @brief Enable or disable an axis for magnification
 * @param axisId Axis identifier
 * @param on true to enable, false to disable
 * 
 * Only axes that are enabled will be zoomed when the magnifier is triggered.
 * All other axes will remain unchanged.
 * 
 * All axes are enabled by default.
 * 
 * @sa isAxisEnabled()
 * \endif
 * 
 * \if CHINESE
 * @brief 启用或禁用坐标轴的放大功能
 * @param axisId 坐标轴标识符
 * @param on true 表示启用，false 表示禁用
 * 
 * 只有启用的坐标轴才会在放大器触发时进行缩放。
 * 其他未启用的坐标轴将保持不变。
 * 
 * 默认情况下所有坐标轴都是启用状态。
 * 
 * @sa isAxisEnabled()
 * \endif
 */
void QwtPlotMagnifier::setAxisEnabled(QwtAxisId axisId, bool on)
{
    if (QwtAxis::isValid(axisId))
        m_data->isAxisEnabled[ axisId ] = on;
}

/*!
 * \if ENGLISH
 * @brief Check if an axis is enabled for magnification
 * @param axisId Axis identifier
 * @return true if the axis is enabled, false otherwise
 * 
 * @sa setAxisEnabled()
 * \endif
 * 
 * \if CHINESE
 * @brief 检查坐标轴是否启用放大功能
 * @param axisId 坐标轴标识符
 * @return 如果坐标轴已启用则返回 true，否则返回 false
 * 
 * @sa setAxisEnabled()
 * \endif
 */
bool QwtPlotMagnifier::isAxisEnabled(QwtAxisId axisId) const
{
    if (QwtAxis::isValid(axisId))
        return m_data->isAxisEnabled[ axisId ];

    return true;
}

/*!
 * \if ENGLISH
 * @brief Return the observed plot canvas
 * @return Pointer to the canvas widget
 * \endif
 * 
 * \if CHINESE
 * @brief 返回被观察的绘图画布
 * @return 画布控件的指针
 * \endif
 */
QWidget* QwtPlotMagnifier::canvas()
{
    return parentWidget();
}

/*!
 * \if ENGLISH
 * @brief Return the observed plot canvas (const version)
 * @return Const pointer to the canvas widget
 * \endif
 * 
 * \if CHINESE
 * @brief 返回被观察的绘图画布（常量版本）
 * @return 画布控件的常量指针
 * \endif
 */
const QWidget* QwtPlotMagnifier::canvas() const
{
    return parentWidget();
}

/*!
 * \if ENGLISH
 * @brief Return the plot widget containing the observed canvas
 * @return Pointer to the QwtPlot widget, or nullptr if not found
 * \endif
 * 
 * \if CHINESE
 * @brief 返回包含被观察画布的绘图控件
 * @return QwtPlot 控件的指针，如果未找到则返回 nullptr
 * \endif
 */
QwtPlot* QwtPlotMagnifier::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/*!
 * \if ENGLISH
 * @brief Return the plot widget containing the observed canvas (const version)
 * @return Const pointer to the QwtPlot widget, or nullptr if not found
 * \endif
 * 
 * \if CHINESE
 * @brief 返回包含被观察画布的绘图控件（常量版本）
 * @return QwtPlot 控件的常量指针，如果未找到则返回 nullptr
 * \endif
 */
const QwtPlot* QwtPlotMagnifier::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
 * \if ENGLISH
 * @brief Rescale the plot axes by the given factor
 * @param factor Magnification factor. A value < 1.0 zooms in, a value > 1.0 zooms out.
 * 
 * This method zooms in/out the axes scales by applying the given factor
 * to all enabled axes. The zoom is centered around the current viewport center.
 * 
 * For parasite plots, this method does nothing.
 * 
 * The method handles all plots in the plot list, including parasite plots
 * that share the same canvas area.
 * \endif
 * 
 * \if CHINESE
 * @brief 按给定因子重新缩放绘图坐标轴
 * @param factor 放大因子。值 < 1.0 时放大，值 > 1.0 时缩小。
 * 
 * 此方法通过对所有启用的坐标轴应用给定因子来进行放大/缩小操作。
 * 缩放以当前视口中心为中心进行。
 * 
 * 对于寄生绘图，此方法不执行任何操作。
 * 
 * 该方法处理绘图列表中的所有绘图，包括共享同一画布区域的寄生绘图。
 * \endif
 */
void QwtPlotMagnifier::rescale(double factor)
{
    QwtPlot* hostplt = plot();
    if (!hostplt || hostplt->isParasitePlot()) {
        return;
    }

    factor = qAbs(factor);
    if (qFuzzyCompare(factor, 1.0) || qFuzzyCompare(factor, 0.0)) {
        return;
    }

    bool doReplot = false;

    const QList< QwtPlot* > allplts = hostplt->plotList(true);
    for (QwtPlot* plt : allplts) {
        plt->saveAutoReplotState();
        plt->setAutoReplot(false);

        for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
            {
                const QwtAxisId axisId(axisPos);

                if (isAxisEnabled(axisId)) {
                    const QwtScaleMap scaleMap = plt->canvasMap(axisId);

                    double v1 = scaleMap.s1();
                    double v2 = scaleMap.s2();

                    if (scaleMap.transformation()) {
                        // the coordinate system of the paint device is always linear

                        v1 = scaleMap.transform(v1);  // scaleMap.p1()
                        v2 = scaleMap.transform(v2);  // scaleMap.p2()
                    }

                    const double center  = 0.5 * (v1 + v2);
                    const double width_2 = 0.5 * (v2 - v1) * factor;

                    v1 = center - width_2;
                    v2 = center + width_2;

                    if (scaleMap.transformation()) {
                        v1 = scaleMap.invTransform(v1);
                        v2 = scaleMap.invTransform(v2);
                    }

                    plt->setAxisScale(axisId, v1, v2);
                    doReplot = true;
                }
            }
        }

        plt->restoreAutoReplotState();
    }
    if (doReplot) {
        hostplt->replotAll();
    }
}
