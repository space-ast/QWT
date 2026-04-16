/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_polar_layout.h"
#include "qwt_painter.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_text_label.h"
#include "qwt_round_scale_draw.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_math.h"
#include <qpointer.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qevent.h>

namespace
{
class QwtPolarPlotScaleData
{
public:
    QwtPolarPlotScaleData() : isValid(false), scaleEngine(nullptr)
    {
    }

    ~QwtPolarPlotScaleData()
    {
        delete scaleEngine;
    }

    bool doAutoScale;

    double minValue;
    double maxValue;
    double stepSize;

    int maxMajor;
    int maxMinor;

    bool isValid;

    QwtScaleDiv scaleDiv;
    QwtScaleEngine* scaleEngine;
};
}

class QwtPolarPlot::PrivateData
{
public:
    QBrush canvasBrush;

    bool autoReplot;

    QwtPointPolar zoomPos;
    double zoomFactor;

    QwtPolarPlotScaleData scaleData[ QwtPolar::ScaleCount ];
    QPointer< QwtTextLabel > titleLabel;
    QPointer< QwtPolarCanvas > canvas;
    QPointer< QwtAbstractLegend > legend;
    double azimuthOrigin;

    QwtPolarLayout* layout;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] parent Parent widget
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] parent 父控件
 * \endif
 */
QwtPolarPlot::QwtPolarPlot(QWidget* parent) : QFrame(parent)
{
    initPlot(QwtText());
}

/**
 * \if ENGLISH
 * @brief Constructor with title
 * @param[in] title Title text
 * @param[in] parent Parent widget
 * \endif
 *
 * \if CHINESE
 * @brief 带标题的构造函数
 * @param[in] title 标题文本
 * @param[in] parent 父控件
 * \endif
 */
QwtPolarPlot::QwtPolarPlot(const QwtText& title, QWidget* parent) : QFrame(parent)
{
    initPlot(title);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPolarPlot::~QwtPolarPlot()
{
    detachItems(QwtPolarItem::Rtti_PolarItem, autoDelete());

    delete m_data->layout;
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Change the plot's title
 * @param[in] title New title
 * \endif
 *
 * \if CHINESE
 * @brief 更改绘图的标题
 * @param[in] title 新标题
 * \endif
 */
void QwtPolarPlot::setTitle(const QString& title)
{
    if (title != m_data->titleLabel->text().text()) {
        m_data->titleLabel->setText(title);
        if (!title.isEmpty())
            m_data->titleLabel->show();
        else
            m_data->titleLabel->hide();
    }
}

/**
 * \if ENGLISH
 * @brief Change the plot's title
 * @param[in] title New title
 * \endif
 *
 * \if CHINESE
 * @brief 更改绘图的标题
 * @param[in] title 新标题
 * \endif
 */
void QwtPolarPlot::setTitle(const QwtText& title)
{
    if (title != m_data->titleLabel->text()) {
        m_data->titleLabel->setText(title);
        if (!title.isEmpty())
            m_data->titleLabel->show();
        else
            m_data->titleLabel->hide();
    }
}

/**
 * \if ENGLISH
 * @brief Get the plot's title
 * @return Title text
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图的标题
 * @return 标题文本
 * \endif
 */
QwtText QwtPolarPlot::title() const
{
    return m_data->titleLabel->text();
}

/**
 * \if ENGLISH
 * @brief Get the title label widget
 * @return Title label widget
 * \endif
 *
 * \if CHINESE
 * @brief 获取标题标签控件
 * @return 标题标签控件
 * \endif
 */
QwtTextLabel* QwtPolarPlot::titleLabel()
{
    return m_data->titleLabel;
}

/**
 * \if ENGLISH
 * @brief Get the title label widget (const version)
 * @return Title label widget
 * \endif
 *
 * \if CHINESE
 * @brief 获取标题标签控件（常量版本）
 * @return 标题标签控件
 * \endif
 */
const QwtTextLabel* QwtPolarPlot::titleLabel() const
{
    return m_data->titleLabel;
}

/**
 * \if ENGLISH
 * @brief Insert a legend
 * @details If the position legend is \c QwtPolarPlot::LeftLegend or \c QwtPolarPlot::RightLegend
 *          the legend will be organized in one column from top to down.
 *          Otherwise the legend items will be placed in a table
 *          with a best fit number of columns from left to right.
 *          If pos != QwtPolarPlot::ExternalLegend the plot widget will become
 *          parent of the legend. It will be deleted when the plot is deleted,
 *          or another legend is set with insertLegend().
 * @param[in] legend Legend widget
 * @param[in] pos The legend's position. For top/left position the number
 *                of columns will be limited to 1, otherwise it will be set to unlimited.
 * @param[in] ratio Ratio between legend and the bounding rect of title, canvas and axes.
 *                   The legend will be shrunk if it would need more space than the given ratio.
 *                   The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0 it will be reset to the default ratio.
 *                   The default vertical/horizontal ratio is 0.33/0.5.
 * @sa legend(), QwtPolarLayout::legendPosition(), QwtPolarLayout::setLegendPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 插入图例
 * @details 如果图例位置为 \c QwtPolarPlot::LeftLegend 或 \c QwtPolarPlot::RightLegend，
 *          图例将从上到下按单列排列。否则，图例项将按最佳列数从左到右排列在表格中。
 *          如果 pos != QwtPolarPlot::ExternalLegend，绘图控件将成为图例的父控件。
 *          当绘图被删除或使用 insertLegend() 设置另一个图例时，图例将被删除。
 * @param[in] legend 图例控件
 * @param[in] pos 图例的位置。对于顶部/左侧位置，列数将限制为1，否则设置为无限制。
 * @param[in] ratio 图例与标题、画布和轴的边界矩形之间的比例。
 *                   如果图例需要的空间超过给定比例，它将被缩小。
 *                   比例限制在 ]0.0 .. 1.0]。如果 <= 0.0，将重置为默认比例。
 *                   默认的垂直/水平比例为 0.33/0.5。
 * @sa legend(), QwtPolarLayout::legendPosition(), QwtPolarLayout::setLegendPosition()
 * \endif
 */
void QwtPolarPlot::insertLegend(QwtAbstractLegend* legend, QwtPolarPlot::LegendPosition pos, double ratio)
{
    m_data->layout->setLegendPosition(pos, ratio);

    if (legend != m_data->legend) {
        if (m_data->legend && m_data->legend->parent() == this)
            delete m_data->legend;

        m_data->legend = legend;

        if (m_data->legend) {
            connect(this,
                    SIGNAL(legendDataChanged(const QVariant&, const QList< QwtLegendData >&)),
                    m_data->legend,
                    SLOT(updateLegend(const QVariant&, const QList< QwtLegendData >&)));

            if (m_data->legend->parent() != this)
                m_data->legend->setParent(this);

            updateLegend();

            QwtLegend* lgd = qobject_cast< QwtLegend* >(legend);
            if (lgd) {
                switch (m_data->layout->legendPosition()) {
                case LeftLegend:
                case RightLegend: {
                    if (lgd->maxColumns() == 0)
                        lgd->setMaxColumns(1);  // 1 column: align vertical
                    break;
                }
                case TopLegend:
                case BottomLegend: {
                    lgd->setMaxColumns(0);  // unlimited
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    updateLayout();
}

/**
 * \if ENGLISH
 * @brief Emit legendDataChanged() for all plot items
 * @sa QwtPlotItem::legendData(), legendDataChanged()
 * \endif
 *
 * \if CHINESE
 * @brief 为所有绘图项发出 legendDataChanged() 信号
 * @sa QwtPlotItem::legendData(), legendDataChanged()
 * \endif
 */
void QwtPolarPlot::updateLegend()
{
    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        updateLegend(*it);
    }
}

/**
 * \if ENGLISH
 * @brief Emit legendDataChanged() for a plot item
 * @param[in] plotItem Plot item
 * @sa QwtPlotItem::legendData(), legendDataChanged()
 * \endif
 *
 * \if CHINESE
 * @brief 为绘图项发出 legendDataChanged() 信号
 * @param[in] plotItem 绘图项
 * @sa QwtPlotItem::legendData(), legendDataChanged()
 * \endif
 */
void QwtPolarPlot::updateLegend(const QwtPolarItem* plotItem)
{
    if (plotItem == nullptr)
        return;

    QList< QwtLegendData > legendData;

    if (plotItem->testItemAttribute(QwtPolarItem::Legend))
        legendData = plotItem->legendData();

    const QVariant itemInfo = itemToInfo(const_cast< QwtPolarItem* >(plotItem));
    Q_EMIT legendDataChanged(itemInfo, legendData);
}

/**
 * \if ENGLISH
 * @brief Get the plot's legend
 * @return Legend widget
 * @sa insertLegend()
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图的图例
 * @return 图例控件
 * @sa insertLegend()
 * \endif
 */
QwtAbstractLegend* QwtPolarPlot::legend()
{
    return m_data->legend;
}

/**
 * \if ENGLISH
 * @brief Get the plot's legend (const version)
 * @return Legend widget
 * @sa insertLegend()
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图的图例（常量版本）
 * @return 图例控件
 * @sa insertLegend()
 * \endif
 */
const QwtAbstractLegend* QwtPolarPlot::legend() const
{
    return m_data->legend;
}

/**
 * \if ENGLISH
 * @brief Set the background of the plot area
 * @details The plot area is the circle around the pole. Its radius is defined by the radial scale.
 * @param[in] brush Background brush
 * @sa plotBackground(), plotArea()
 * \endif
 *
 * \if CHINESE
 * @brief 设置绘图区域的背景
 * @details 绘图区域是围绕极点的圆。其半径由径向刻度定义。
 * @param[in] brush 背景画刷
 * @sa plotBackground(), plotArea()
 * \endif
 */
void QwtPolarPlot::setPlotBackground(const QBrush& brush)
{
    if (brush != m_data->canvasBrush) {
        m_data->canvasBrush = brush;
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Get the plot background brush
 * @return Background brush
 * @sa plotBackground(), plotArea()
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图背景画刷
 * @return 背景画刷
 * @sa plotBackground(), plotArea()
 * \endif
 */
const QBrush& QwtPolarPlot::plotBackground() const
{
    return m_data->canvasBrush;
}

/**
 * \if ENGLISH
 * @brief Set or reset the autoReplot option
 * @details If the autoReplot option is set, the plot will be updated implicitly by manipulating member functions.
 *          Since this may be time-consuming, it is recommended to leave this option switched off and call replot()
 *          explicitly if necessary.
 *          The autoReplot option is set to false by default, which means that the user has to call replot()
 *          in order to make changes visible.
 * @param[in] enable \c true or \c false. Defaults to \c true.
 * @sa replot()
 * \endif
 *
 * \if CHINESE
 * @brief 设置或重置自动重绘选项
 * @details 如果设置了自动重绘选项，绘图将在操作成员函数时隐式更新。
 *          由于这可能耗时，建议关闭此选项并在必要时显式调用 replot()。
 *          默认情况下自动重绘选项为 false，这意味着用户必须调用 replot() 才能使更改可见。
 * @param[in] enable \c true 或 \c false。默认为 \c true。
 * @sa replot()
 * \endif
 */
void QwtPolarPlot::setAutoReplot(bool enable)
{
    m_data->autoReplot = enable;
}

/**
 * \if ENGLISH
 * @brief Check if autoReplot option is set
 * @return \c true if autoReplot option is set
 * @sa setAutoReplot()
 * \endif
 *
 * \if CHINESE
 * @brief 检查是否设置了自动重绘选项
 * @return \c true 如果设置了自动重绘选项
 * @sa setAutoReplot()
 * \endif
 */
bool QwtPolarPlot::autoReplot() const
{
    return m_data->autoReplot;
}

/**
 * \if ENGLISH
 * @brief Enable autoscaling
 * @details This member function is used to switch back to autoscaling mode after a fixed scale has been set.
 *          Autoscaling calculates a useful scale division from the bounding interval of all plot items
 *          with the QwtPolarItem::AutoScale attribute.
 *          Autoscaling is only supported for the radial scale and enabled as default.
 * @param[in] scaleId Scale index
 * @sa hasAutoScale(), setScale(), setScaleDiv(), QwtPolarItem::boundingInterval()
 * \endif
 *
 * \if CHINESE
 * @brief 启用自动缩放
 * @details 此成员函数用于在设置了固定刻度后切换回自动缩放模式。
 *          自动缩放从所有带有 QwtPolarItem::AutoScale 属性的绘图项的边界区间计算有用的刻度划分。
 *          自动缩放仅支持径向刻度，默认启用。
 * @param[in] scaleId 刻度索引
 * @sa hasAutoScale(), setScale(), setScaleDiv(), QwtPolarItem::boundingInterval()
 * \endif
 */
void QwtPolarPlot::setAutoScale(int scaleId)
{
    if (scaleId != QwtPolar::ScaleRadius)
        return;

    QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];
    if (!scaleData.doAutoScale) {
        scaleData.doAutoScale = true;
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Check if autoscaling is enabled
 * @param[in] scaleId Scale index
 * @return \c true if autoscaling is enabled
 * @sa setAutoScale()
 * \endif
 *
 * \if CHINESE
 * @brief 检查是否启用了自动缩放
 * @param[in] scaleId 刻度索引
 * @return \c true 如果启用了自动缩放
 * @sa setAutoScale()
 * \endif
 */
bool QwtPolarPlot::hasAutoScale(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return false;

    return m_data->scaleData[ scaleId ].doAutoScale;
}

/**
 * \if ENGLISH
 * @brief Set the maximum number of minor scale intervals for a specified scale
 * @param[in] scaleId Scale index
 * @param[in] maxMinor Maximum number of minor steps
 * @sa scaleMaxMajor()
 * \endif
 *
 * \if CHINESE
 * @brief 设置指定刻度的最大次要刻度间隔数
 * @param[in] scaleId 刻度索引
 * @param[in] maxMinor 最大次要步数
 * @sa scaleMaxMajor()
 * \endif
 */
void QwtPolarPlot::setScaleMaxMinor(int scaleId, int maxMinor)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    maxMinor = qBound(0, maxMinor, 100);

    QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];

    if (maxMinor != scaleData.maxMinor) {
        scaleData.maxMinor = maxMinor;
        scaleData.isValid  = false;
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum number of minor ticks for a specified axis
 * @param[in] scaleId Scale index
 * @return Maximum number of minor ticks
 * @sa setScaleMaxMinor()
 * \endif
 *
 * \if CHINESE
 * @brief 获取指定轴的最大次要刻度数
 * @param[in] scaleId 刻度索引
 * @return 最大次要刻度数
 * @sa setScaleMaxMinor()
 * \endif
 */
int QwtPolarPlot::scaleMaxMinor(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return 0;

    return m_data->scaleData[ scaleId ].maxMinor;
}

/**
 * \if ENGLISH
 * @brief Set the maximum number of major scale intervals for a specified scale
 * @param[in] scaleId Scale index
 * @param[in] maxMajor Maximum number of major steps
 * @sa scaleMaxMajor()
 * \endif
 *
 * \if CHINESE
 * @brief 设置指定刻度的最大主要刻度间隔数
 * @param[in] scaleId 刻度索引
 * @param[in] maxMajor 最大主要步数
 * @sa scaleMaxMajor()
 * \endif
 */
void QwtPolarPlot::setScaleMaxMajor(int scaleId, int maxMajor)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    maxMajor = qBound(1, maxMajor, 10000);

    QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];
    if (maxMajor != scaleData.maxMinor) {
        scaleData.maxMajor = maxMajor;
        scaleData.isValid  = false;
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum number of major ticks for a specified axis
 * @param[in] scaleId Scale index
 * @return Maximum number of major ticks
 * @sa setScaleMaxMajor()
 * \endif
 *
 * \if CHINESE
 * @brief 获取指定轴的最大主要刻度数
 * @param[in] scaleId 刻度索引
 * @return 最大主要刻度数
 * @sa setScaleMaxMajor()
 * \endif
 */
int QwtPolarPlot::scaleMaxMajor(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return 0;

    return m_data->scaleData[ scaleId ].maxMajor;
}

/**
 * \if ENGLISH
 * @brief Change the scale engine for an axis
 * @param[in] scaleId Scale index
 * @param[in] scaleEngine Scale engine
 * @sa axisScaleEngine()
 * \endif
 *
 * \if CHINESE
 * @brief 更改轴的刻度引擎
 * @param[in] scaleId 刻度索引
 * @param[in] scaleEngine 刻度引擎
 * @sa axisScaleEngine()
 * \endif
 */
void QwtPolarPlot::setScaleEngine(int scaleId, QwtScaleEngine* scaleEngine)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];
    if (scaleEngine == nullptr || scaleEngine == scaleData.scaleEngine)
        return;

    delete scaleData.scaleEngine;
    scaleData.scaleEngine = scaleEngine;

    scaleData.isValid = false;

    autoRefresh();
}

/**
 * \if ENGLISH
 * @brief Get scale engine for a specific scale
 * @param[in] scaleId Scale index
 * @return Scale engine
 * @sa setScaleEngine()
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定刻度的刻度引擎
 * @param[in] scaleId 刻度索引
 * @return 刻度引擎
 * @sa setScaleEngine()
 * \endif
 */
QwtScaleEngine* QwtPolarPlot::scaleEngine(int scaleId)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return m_data->scaleData[ scaleId ].scaleEngine;
}

/**
 * \if ENGLISH
 * @brief Get scale engine for a specific scale (const version)
 * @param[in] scaleId Scale index
 * @return Scale engine
 * @sa setScaleEngine()
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定刻度的刻度引擎（常量版本）
 * @param[in] scaleId 刻度索引
 * @return 刻度引擎
 * @sa setScaleEngine()
 * \endif
 */
const QwtScaleEngine* QwtPolarPlot::scaleEngine(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return m_data->scaleData[ scaleId ].scaleEngine;
}

/**
 * \if ENGLISH
 * @brief Disable autoscaling and specify a fixed scale for a selected scale
 * @param[in] scaleId Scale index
 * @param[in] min Minimum value of the scale
 * @param[in] max Maximum value of the scale
 * @param[in] stepSize Major step size. If <code>step == 0</code>, the step size is calculated automatically using the maxMajor setting.
 * @sa setScaleMaxMajor(), setAutoScale()
 * \endif
 *
 * \if CHINESE
 * @brief 禁用自动缩放并指定选定刻度的固定刻度
 * @param[in] scaleId 刻度索引
 * @param[in] min 刻度的最小值
 * @param[in] max 刻度的最大值
 * @param[in] stepSize 主要步长。如果 <code>step == 0</code>，则使用 maxMajor 设置自动计算步长。
 * @sa setScaleMaxMajor(), setAutoScale()
 * \endif
 */
void QwtPolarPlot::setScale(int scaleId, double min, double max, double stepSize)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];

    scaleData.isValid = false;

    scaleData.minValue    = min;
    scaleData.maxValue    = max;
    scaleData.stepSize    = stepSize;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/**
 * \if ENGLISH
 * @brief Disable autoscaling and specify a fixed scale for a selected scale
 * @param[in] scaleId Scale index
 * @param[in] scaleDiv Scale division
 * @sa setScale(), setAutoScale()
 * \endif
 *
 * \if CHINESE
 * @brief 禁用自动缩放并指定选定刻度的固定刻度
 * @param[in] scaleId 刻度索引
 * @param[in] scaleDiv 刻度划分
 * @sa setScale(), setAutoScale()
 * \endif
 */
void QwtPolarPlot::setScaleDiv(int scaleId, const QwtScaleDiv& scaleDiv)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];

    scaleData.scaleDiv    = scaleDiv;
    scaleData.isValid     = true;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/**
 * \if ENGLISH
 * @brief Return the scale division of a specified scale
 * @details scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound() are the current limits of the scale.
 * @param[in] scaleId Scale index
 * @return Scale division
 * @sa QwtScaleDiv, setScaleDiv(), setScale()
 * \endif
 *
 * \if CHINESE
 * @brief 返回指定刻度的刻度划分
 * @details scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound() 是刻度的当前界限。
 * @param[in] scaleId 刻度索引
 * @return 刻度划分
 * @sa QwtScaleDiv, setScaleDiv(), setScale()
 * \endif
 */
const QwtScaleDiv* QwtPolarPlot::scaleDiv(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return &m_data->scaleData[ scaleId ].scaleDiv;
}

/**
 * \if ENGLISH
 * @brief Return the scale division of a specified scale
 * @details scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound() are the current limits of the scale.
 * @param[in] scaleId Scale index
 * @return Scale division
 * @sa QwtScaleDiv, setScaleDiv(), setScale()
 * \endif
 *
 * \if CHINESE
 * @brief 返回指定刻度的刻度划分
 * @details scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound() 是刻度的当前界限。
 * @param[in] scaleId 刻度索引
 * @return 刻度划分
 * @sa QwtScaleDiv, setScaleDiv(), setScale()
 * \endif
 */
QwtScaleDiv* QwtPolarPlot::scaleDiv(int scaleId)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return &m_data->scaleData[ scaleId ].scaleDiv;
}

/**
 * \if ENGLISH
 * @brief Change the origin of the azimuth scale
 * @details The azimuth origin is the angle where the azimuth scale shows the value 0.0. The default origin is 0.0.
 * @param[in] origin New origin
 * @sa azimuthOrigin()
 * \endif
 *
 * \if CHINESE
 * @brief 更改方位角刻度的原点
 * @details 方位角原点是方位角刻度显示值 0.0 的角度。默认原点为 0.0。
 * @param[in] origin 新原点
 * @sa azimuthOrigin()
 * \endif
 */
void QwtPolarPlot::setAzimuthOrigin(double origin)
{
    origin = ::fmod(origin, 2 * M_PI);
    if (origin != m_data->azimuthOrigin) {
        m_data->azimuthOrigin = origin;
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Get the origin of the azimuth scale
 * @details The azimuth origin is the angle where the azimuth scale shows the value 0.0.
 * @return Origin of the azimuth scale
 * @sa setAzimuthOrigin()
 * \endif
 *
 * \if CHINESE
 * @brief 获取方位角刻度的原点
 * @details 方位角原点是方位角刻度显示值 0.0 的角度。
 * @return 方位角刻度的原点
 * @sa setAzimuthOrigin()
 * \endif
 */
double QwtPolarPlot::azimuthOrigin() const
{
    return m_data->azimuthOrigin;
}

/**
 * \if ENGLISH
 * @brief Translate and in/decrease the zoom factor
 * @details In zoom mode the zoom position is in the center of the canvas.
 *          The radius of the circle depends on the size of the plot canvas, that is divided by the zoom factor.
 *          Thus a factor < 1.0 zooms in.
 *          Setting an invalid zoom position disables zooming.
 * @param[in] zoomPos Center of the translation
 * @param[in] zoomFactor Zoom factor
 * @sa unzoom(), zoomPos(), zoomFactor()
 * \endif
 *
 * \if CHINESE
 * @brief 平移和增/减缩放因子
 * @details 在缩放模式下，缩放位置位于画布中心。
 *          圆的半径取决于绘图画布的大小，除以缩放因子。
 *          因此因子 < 1.0 会放大。
 *          设置无效的缩放位置将禁用缩放。
 * @param[in] zoomPos 平移的中心
 * @param[in] zoomFactor 缩放因子
 * @sa unzoom(), zoomPos(), zoomFactor()
 * \endif
 */
void QwtPolarPlot::zoom(const QwtPointPolar& zoomPos, double zoomFactor)
{
    zoomFactor = qAbs(zoomFactor);
    if (zoomPos != m_data->zoomPos || zoomFactor != m_data->zoomFactor) {
        m_data->zoomPos    = zoomPos;
        m_data->zoomFactor = zoomFactor;
        updateLayout();
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Unzoom the plot
 * @sa zoom()
 * \endif
 *
 * \if CHINESE
 * @brief 取消绘图的缩放
 * @sa zoom()
 * \endif
 */
void QwtPolarPlot::unzoom()
{
    if (m_data->zoomFactor != 1.0 || m_data->zoomPos.isValid()) {
        m_data->zoomFactor = 1.0;
        m_data->zoomPos    = QwtPointPolar();
        autoRefresh();
    }
}

/**
 * \if ENGLISH
 * @brief Get the zoom position
 * @return Zoom position
 * @sa zoom(), zoomFactor()
 * \endif
 *
 * \if CHINESE
 * @brief 获取缩放位置
 * @return 缩放位置
 * @sa zoom(), zoomFactor()
 * \endif
 */
QwtPointPolar QwtPolarPlot::zoomPos() const
{
    return m_data->zoomPos;
}

/**
 * \if ENGLISH
 * @brief Get the zoom factor
 * @return Zoom factor
 * @sa zoom(), zoomPos()
 * \endif
 *
 * \if CHINESE
 * @brief 获取缩放因子
 * @return 缩放因子
 * @sa zoom(), zoomPos()
 * \endif
 */
double QwtPolarPlot::zoomFactor() const
{
    return m_data->zoomFactor;
}

/**
 * \if ENGLISH
 * @brief Build a scale map
 * @details The azimuth map translates between the scale values and angles from [0.0, 2 * PI[.
 *          The radial map translates scale values into the distance from the pole.
 *          The radial map is calculated from the current geometry of the canvas.
 * @param[in] scaleId Scale index
 * @return Map for the scale on the canvas. With this map pixel coordinates can be translated to plot coordinates and vice versa.
 * @sa QwtScaleMap, transform(), invTransform()
 * \endif
 *
 * \if CHINESE
 * @brief 构建刻度映射
 * @details 方位角映射将刻度值转换为 [0.0, 2 * PI[ 范围内的角度。
 *          径向映射将刻度值转换为距极点的距离。
 *          径向映射根据画布的当前几何形状计算。
 * @param[in] scaleId 刻度索引
 * @return 画布上刻度的映射。使用此映射可以将像素坐标转换为绘图坐标，反之亦然。
 * @sa QwtScaleMap, transform(), invTransform()
 * \endif
 */
QwtScaleMap QwtPolarPlot::scaleMap(int scaleId) const
{
    const QRectF pr = plotRect();
    return scaleMap(scaleId, pr.width() / 2.0);
}

/**
 * \if ENGLISH
 * @brief Build a scale map with specified radius
 * @details The azimuth map translates between the scale values and angles from [0.0, 2 * PI[.
 *          The radial map translates scale values into the distance from the pole.
 * @param[in] scaleId Scale index
 * @param[in] radius Radius of the plot area in pixels
 * @return Map for the scale on the canvas. With this map pixel coordinates can be translated to plot coordinates and vice versa.
 * @sa QwtScaleMap, transform(), invTransform()
 * \endif
 *
 * \if CHINESE
 * @brief 构建带有指定半径的刻度映射
 * @details 方位角映射将刻度值转换为 [0.0, 2 * PI[ 范围内的角度。
 *          径向映射将刻度值转换为距极点的距离。
 * @param[in] scaleId 刻度索引
 * @param[in] radius 绘图区域的半径（像素）
 * @return 画布上刻度的映射。使用此映射可以将像素坐标转换为绘图坐标，反之亦然。
 * @sa QwtScaleMap, transform(), invTransform()
 * \endif
 */
QwtScaleMap QwtPolarPlot::scaleMap(int scaleId, const double radius) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return QwtScaleMap();

    QwtScaleMap map;
    map.setTransformation(scaleEngine(scaleId)->transformation());

    const QwtScaleDiv* sd = scaleDiv(scaleId);
    map.setScaleInterval(sd->lowerBound(), sd->upperBound());

    if (scaleId == QwtPolar::Azimuth) {
        map.setPaintInterval(m_data->azimuthOrigin, m_data->azimuthOrigin + 2 * M_PI);
    } else {
        map.setPaintInterval(0.0, radius);
    }

    return map;
}

/**
 * \if ENGLISH
 * @brief Qt event handler
 * @details Handles QEvent::LayoutRequest and QEvent::PolishRequest
 * @param[in] e Qt Event
 * @return True when the event was processed
 * \endif
 *
 * \if CHINESE
 * @brief Qt 事件处理器
 * @details 处理 QEvent::LayoutRequest 和 QEvent::PolishRequest
 * @param[in] e Qt 事件
 * @return 当事件被处理时返回 true
 * \endif
 */
bool QwtPolarPlot::event(QEvent* e)
{
    bool ok = QWidget::event(e);
    switch (e->type()) {
    case QEvent::LayoutRequest: {
        updateLayout();
        break;
    }
    case QEvent::PolishRequest: {
        updateLayout();
        replot();
        break;
    }
    default:;
    }
    return ok;
}

/**
 * \if ENGLISH
 * @brief Resize and update internal layout
 * @param[in] e Resize event
 * \endif
 *
 * \if CHINESE
 * @brief 调整大小并更新内部布局
 * @param[in] e 调整大小事件
 * \endif
 */
void QwtPolarPlot::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);
    updateLayout();
}

void QwtPolarPlot::initPlot(const QwtText& title)
{
    m_data         = new PrivateData;
    m_data->layout = new QwtPolarLayout;

    QwtText text(title);
    text.setRenderFlags(Qt::AlignCenter | Qt::TextWordWrap);

    m_data->titleLabel = new QwtTextLabel(text, this);
    m_data->titleLabel->setFont(QFont(fontInfo().family(), 14, QFont::Bold));
    if (!text.isEmpty())
        m_data->titleLabel->show();
    else
        m_data->titleLabel->hide();

    m_data->canvas = new QwtPolarCanvas(this);

    m_data->autoReplot  = false;
    m_data->canvasBrush = QBrush(Qt::white);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++) {
        QwtPolarPlotScaleData& scaleData = m_data->scaleData[ scaleId ];

        if (scaleId == QwtPolar::Azimuth) {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 360.0;
            scaleData.stepSize = 30.0;
        } else {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 1000.0;
            scaleData.stepSize = 0.0;
        }

        scaleData.doAutoScale = true;

        scaleData.maxMinor = 5;
        scaleData.maxMajor = 8;

        scaleData.isValid = false;

        scaleData.scaleEngine = new QwtLinearScaleEngine;
    }
    m_data->zoomFactor    = 1.0;
    m_data->azimuthOrigin = 0.0;

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++)
        updateScale(scaleId);
}

/**
 * \if ENGLISH
 * @brief Replots the plot if autoReplot() is \c true
 * \endif
 *
 * \if CHINESE
 * @brief 如果 autoReplot() 为 \c true 则重新绘制绘图
 * \endif
 */
void QwtPolarPlot::autoRefresh()
{
    if (m_data->autoReplot)
        replot();
}

/**
 * \if ENGLISH
 * @brief Rebuild the layout
 * \endif
 *
 * \if CHINESE
 * @brief 重建布局
 * \endif
 */
void QwtPolarPlot::updateLayout()
{
    m_data->layout->activate(this, contentsRect());

    // resize and show the visible widgets
    if (m_data->titleLabel) {
        if (!m_data->titleLabel->text().isEmpty()) {
            m_data->titleLabel->setGeometry(m_data->layout->titleRect().toRect());
            if (!m_data->titleLabel->isVisible())
                m_data->titleLabel->show();
        } else
            m_data->titleLabel->hide();
    }

    if (m_data->legend) {
        if (m_data->legend->isEmpty()) {
            m_data->legend->hide();
        } else {
            const QRectF legendRect = m_data->layout->legendRect();
            m_data->legend->setGeometry(legendRect.toRect());
            m_data->legend->show();
        }
    }

    m_data->canvas->setGeometry(m_data->layout->canvasRect().toRect());
    Q_EMIT layoutChanged();
}

/**
 * \if ENGLISH
 * @brief Redraw the plot
 * @details If the autoReplot option is not set (which is the default) or if any curves are attached to raw data,
 *          the plot has to be refreshed explicitly in order to make changes visible.
 * @warning Calls canvas()->repaint, take care of infinite recursions
 * @sa setAutoReplot()
 * \endif
 *
 * \if CHINESE
 * @brief 重绘绘图
 * @details 如果未设置自动重绘选项（默认）或任何曲线附加到原始数据，
 *          必须显式刷新绘图才能使更改可见。
 * @warning 调用 canvas()->repaint，注意无限递归
 * @sa setAutoReplot()
 * \endif
 */
void QwtPolarPlot::replot()
{
    bool doAutoReplot = autoReplot();
    setAutoReplot(false);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++)
        updateScale(scaleId);

    m_data->canvas->invalidateBackingStore();
    m_data->canvas->repaint();

    setAutoReplot(doAutoReplot);
}

/**
 * \if ENGLISH
 * @brief Get the plot's canvas
 * @return Canvas widget
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图的画布
 * @return 画布控件
 * \endif
 */
QwtPolarCanvas* QwtPolarPlot::canvas()
{
    return m_data->canvas;
}

/**
 * \if ENGLISH
 * @brief Get the plot's canvas (const version)
 * @return Canvas widget
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图的画布（常量版本）
 * @return 画布控件
 * \endif
 */
const QwtPolarCanvas* QwtPolarPlot::canvas() const
{
    return m_data->canvas;
}

/**
 * \if ENGLISH
 * @brief Redraw the canvas
 * @param[in] painter Painter used for drawing
 * @param[in] canvasRect Contents rect of the canvas
 * \endif
 *
 * \if CHINESE
 * @brief 重绘画布
 * @param[in] painter 用于绘制的画师
 * @param[in] canvasRect 画布的内容矩形
 * \endif
 */
void QwtPolarPlot::drawCanvas(QPainter* painter, const QRectF& canvasRect) const
{
    const QRectF cr = canvasRect;
    const QRectF pr = plotRect(cr);

    const double radius = pr.width() / 2.0;

    if (m_data->canvasBrush.style() != Qt::NoBrush) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_data->canvasBrush);

        if (qwtDistance(pr.center(), cr.topLeft()) < radius && qwtDistance(pr.center(), cr.topRight()) < radius
            && qwtDistance(pr.center(), cr.bottomRight()) < radius && qwtDistance(pr.center(), cr.bottomLeft()) < radius) {
            QwtPainter::drawRect(painter, cr);
        } else {
            painter->setRenderHint(QPainter::Antialiasing, true);
            QwtPainter::drawEllipse(painter, pr);
        }
        painter->restore();
    }

    drawItems(painter, scaleMap(QwtPolar::Azimuth, radius), scaleMap(QwtPolar::Radius, radius), pr.center(), radius, canvasRect);
}

/**
 * \if ENGLISH
 * @brief Redraw the canvas items
 * @param[in] painter Painter used for drawing
 * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param[in] radialMap Maps radius values into painter coordinates
 * @param[in] pole Position of the pole in painter coordinates
 * @param[in] radius Radius of the complete plot area in painter coordinates
 * @param[in] canvasRect Contents rect of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 重绘画布项
 * @param[in] painter 用于绘制的画师
 * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
 * @param[in] radialMap 将半径值映射到画师坐标
 * @param[in] pole 画师坐标中极点的位置
 * @param[in] radius 画师坐标中完整绘图区域的半径
 * @param[in] canvasRect 画师坐标中画布的内容矩形
 * \endif
 */
void QwtPolarPlot::drawItems(QPainter* painter,
                             const QwtScaleMap& azimuthMap,
                             const QwtScaleMap& radialMap,
                             const QPointF& pole,
                             double radius,
                             const QRectF& canvasRect) const
{
    const QRectF pr = plotRect(canvasRect);

    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        QwtPolarItem* item = *it;
        if (item && item->isVisible()) {
            painter->save();

            // Unfortunately circular clipping slows down
            // painting a lot. So we better try to avoid it.

            bool doClipping = false;
            if (item->rtti() != QwtPolarItem::Rtti_PolarGrid) {
                const QwtInterval intv = item->boundingInterval(QwtPolar::Radius);

                if (!intv.isValid())
                    doClipping = true;
                else {
                    if (radialMap.s1() < radialMap.s2())
                        doClipping = intv.maxValue() > radialMap.s2();
                    else
                        doClipping = intv.minValue() < radialMap.s2();
                }
            }

            if (doClipping) {
                const int margin = item->marginHint();

                const QRectF clipRect = pr.adjusted(-margin, -margin, margin, margin);
                if (!clipRect.contains(canvasRect)) {
                    QRegion clipRegion(clipRect.toRect(), QRegion::Ellipse);
                    painter->setClipRegion(clipRegion, Qt::IntersectClip);
                }
            }

            painter->setRenderHint(QPainter::Antialiasing, item->testRenderHint(QwtPolarItem::RenderAntialiased));

            item->draw(painter, azimuthMap, radialMap, pole, radius, canvasRect);

            painter->restore();
        }
    }
}

/**
 * \if ENGLISH
 * @brief Rebuild the scale
 * @param[in] scaleId Scale index
 * \endif
 *
 * \if CHINESE
 * @brief 重建刻度
 * @param[in] scaleId 刻度索引
 * \endif
 */
void QwtPolarPlot::updateScale(int scaleId)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& d = m_data->scaleData[ scaleId ];

    double minValue = d.minValue;
    double maxValue = d.maxValue;
    double stepSize = d.stepSize;

    if (scaleId == QwtPolar::ScaleRadius && d.doAutoScale) {
        QwtInterval interval;

        const QwtPolarItemList& itmList = itemList();
        for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
            const QwtPolarItem* item = *it;
            if (item->testItemAttribute(QwtPolarItem::AutoScale))
                interval |= item->boundingInterval(scaleId);
        }

        minValue = interval.minValue();
        maxValue = interval.maxValue();

        d.scaleEngine->autoScale(d.maxMajor, minValue, maxValue, stepSize);
        d.isValid = false;
    }

    if (!d.isValid) {
        d.scaleDiv = d.scaleEngine->divideScale(minValue, maxValue, d.maxMajor, d.maxMinor, stepSize);
        d.isValid  = true;
    }

    const QwtInterval interval = visibleInterval();

    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        QwtPolarItem* item = *it;
        item->updateScaleDiv(*scaleDiv(QwtPolar::Azimuth), *scaleDiv(QwtPolar::Radius), interval);
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum of all item margin hints
 * @return Maximum margin hint
 * @sa QwtPolarItem::marginHint()
 * \endif
 *
 * \if CHINESE
 * @brief 获取所有项边距提示的最大值
 * @return 最大边距提示
 * @sa QwtPolarItem::marginHint()
 * \endif
 */
int QwtPolarPlot::plotMarginHint() const
{
    int margin                      = 0;
    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        QwtPolarItem* item = *it;
        if (item && item->isVisible()) {
            const int hint = item->marginHint();
            if (hint > margin)
                margin = hint;
        }
    }
    return margin;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rect of the plot area
 * @details The plot area depends on the size of the canvas and the zoom parameters.
 * @return Bounding rect of the plot area
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图区域的边界矩形
 * @details 绘图区域取决于画布的大小和缩放参数。
 * @return 绘图区域的边界矩形
 * \endif
 */
QRectF QwtPolarPlot::plotRect() const
{
    return plotRect(canvas()->contentsRect());
}

/**
 * \if ENGLISH
 * @brief Calculate the bounding rect of the plot area
 * @details The plot area depends on the zoom parameters.
 * @param[in] canvasRect Rectangle of the canvas
 * @return Rectangle for displaying 100% of the plot
 * \endif
 *
 * \if CHINESE
 * @brief 计算绘图区域的边界矩形
 * @details 绘图区域取决于缩放参数。
 * @param[in] canvasRect 画布的矩形
 * @return 显示 100% 绘图的矩形
 * \endif
 */
QRectF QwtPolarPlot::plotRect(const QRectF& canvasRect) const
{
    const QwtScaleDiv* sd    = scaleDiv(QwtPolar::Radius);
    const QwtScaleEngine* se = scaleEngine(QwtPolar::Radius);

    const int margin = plotMarginHint();
    const QRectF cr  = canvasRect;
    const int radius = qMin(cr.width(), cr.height()) / 2 - margin;

    QwtScaleMap map;
    map.setTransformation(se->transformation());
    map.setPaintInterval(0.0, radius / m_data->zoomFactor);
    map.setScaleInterval(sd->lowerBound(), sd->upperBound());

    double v = map.s1();
    if (map.s1() <= map.s2())
        v += m_data->zoomPos.radius();
    else
        v -= m_data->zoomPos.radius();
    v = map.transform(v);

    const QPointF off = QwtPointPolar(m_data->zoomPos.azimuth(), v).toPoint();

    QPointF center(cr.center().x(), cr.top() + margin + radius);
    center -= QPointF(off.x(), -off.y());

    QRectF rect(0, 0, 2 * map.p2(), 2 * map.p2());
    rect.moveCenter(center);

    return rect;
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval of the radial scale visible on canvas
 * @return Bounding interval of the radial scale that is visible on the canvas
 * \endif
 *
 * \if CHINESE
 * @brief 获取画布上可见的径向刻度的边界区间
 * @return 画布上可见的径向刻度的边界区间
 * \endif
 */
QwtInterval QwtPolarPlot::visibleInterval() const
{
    const QwtScaleDiv* sd = scaleDiv(QwtPolar::Radius);

    const QRectF cRect = canvas()->contentsRect();
    const QRectF pRect = plotRect(cRect);
    if (cRect.contains(pRect) || !cRect.intersects(pRect)) {
        return QwtInterval(sd->lowerBound(), sd->upperBound());
    }

    const QPointF pole     = pRect.center();
    const QRectF scaleRect = pRect & cRect;

    const QwtScaleMap map = scaleMap(QwtPolar::Radius);

    double dmin = 0.0;
    double dmax = 0.0;
    if (scaleRect.contains(pole)) {
        dmin = 0.0;

        QPointF corners[ 4 ];
        corners[ 0 ] = scaleRect.bottomRight();
        corners[ 1 ] = scaleRect.topRight();
        corners[ 2 ] = scaleRect.topLeft();
        corners[ 3 ] = scaleRect.bottomLeft();

        dmax = 0.0;
        for (int i = 0; i < 4; i++) {
            const double dist = qwtDistance(pole, corners[ i ]);
            if (dist > dmax)
                dmax = dist;
        }
    } else {
        if (pole.x() < scaleRect.left()) {
            if (pole.y() < scaleRect.top()) {
                dmin = qwtDistance(pole, scaleRect.topLeft());
                dmax = qwtDistance(pole, scaleRect.bottomRight());
            } else if (pole.y() > scaleRect.bottom()) {
                dmin = qwtDistance(pole, scaleRect.bottomLeft());
                dmax = qwtDistance(pole, scaleRect.topRight());
            } else {
                dmin = scaleRect.left() - pole.x();
                dmax = qMax(qwtDistance(pole, scaleRect.bottomRight()), qwtDistance(pole, scaleRect.topRight()));
            }
        } else if (pole.x() > scaleRect.right()) {
            if (pole.y() < scaleRect.top()) {
                dmin = qwtDistance(pole, scaleRect.topRight());
                dmax = qwtDistance(pole, scaleRect.bottomLeft());
            } else if (pole.y() > scaleRect.bottom()) {
                dmin = qwtDistance(pole, scaleRect.bottomRight());
                dmax = qwtDistance(pole, scaleRect.topLeft());
            } else {
                dmin = pole.x() - scaleRect.right();
                dmax = qMax(qwtDistance(pole, scaleRect.bottomLeft()), qwtDistance(pole, scaleRect.topLeft()));
            }
        } else if (pole.y() < scaleRect.top()) {
            dmin = scaleRect.top() - pole.y();
            dmax = qMax(qwtDistance(pole, scaleRect.bottomLeft()), qwtDistance(pole, scaleRect.bottomRight()));
        } else if (pole.y() > scaleRect.bottom()) {
            dmin = pole.y() - scaleRect.bottom();
            dmax = qMax(qwtDistance(pole, scaleRect.topLeft()), qwtDistance(pole, scaleRect.topRight()));
        }
    }

    const double radius = pRect.width() / 2.0;
    if (dmax > radius)
        dmax = radius;

    QwtInterval interval;
    interval.setMinValue(map.invTransform(dmin));
    interval.setMaxValue(map.invTransform(dmax));

    return interval;
}

/**
 * \if ENGLISH
 * @brief Get the layout responsible for geometry of plot components
 * @return Plot layout
 * \endif
 *
 * \if CHINESE
 * @brief 获取负责绘图组件几何形状的布局
 * @return 绘图布局
 * \endif
 */
QwtPolarLayout* QwtPolarPlot::plotLayout()
{
    return m_data->layout;
}

/**
 * \if ENGLISH
 * @brief Get the layout responsible for geometry of plot components (const version)
 * @return Plot layout
 * \endif
 *
 * \if CHINESE
 * @brief 获取负责绘图组件几何形状的布局（常量版本）
 * @return 绘图布局
 * \endif
 */
const QwtPolarLayout* QwtPolarPlot::plotLayout() const
{
    return m_data->layout;
}

/**
 * \if ENGLISH
 * @brief Attach/Detach a plot item
 * @param[in] plotItem Plot item
 * @param[in] on When true attach the item, otherwise detach it
 * \endif
 *
 * \if CHINESE
 * @brief 附加/分离绘图项
 * @param[in] plotItem 绘图项
 * @param[in] on 当为 true 时附加项，否则分离它
 * \endif
 */
void QwtPolarPlot::attachItem(QwtPolarItem* plotItem, bool on)
{
    if (on)
        insertItem(plotItem);
    else
        removeItem(plotItem);

    Q_EMIT itemAttached(plotItem, on);

    if (plotItem->testItemAttribute(QwtPolarItem::Legend)) {
        // the item wants to be represented on the legend

        if (on) {
            updateLegend(plotItem);
        } else {
            const QVariant itemInfo = itemToInfo(plotItem);
            Q_EMIT legendDataChanged(itemInfo, QList< QwtLegendData >());
        }
    }

    if (autoReplot())
        update();
}

/**
 * \if ENGLISH
 * @brief Build an information object to identify a plot item on the legend
 * @details The default implementation simply wraps the plot item into a QVariant object.
 *          When overloading itemToInfo() usually infoToItem() needs to be reimplemented too.
 * @param[in] plotItem Plot item
 * @return QVariant containing the plot item information
 * @sa infoToItem()
 * \endif
 *
 * \if CHINESE
 * @brief 构建信息对象以在图例上标识绘图项
 * @details 默认实现只是将绘图项包装到 QVariant 对象中。
 *          重载 itemToInfo() 时通常也需要重载 infoToItem()。
 * @param[in] plotItem 绘图项
 * @return 包含绘图项信息的 QVariant
 * @sa infoToItem()
 * \endif
 */
QVariant QwtPolarPlot::itemToInfo(QwtPolarItem* plotItem) const
{
    return QVariant::fromValue(plotItem);
}

/**
 * \if ENGLISH
 * @brief Identify the plot item according to an item info object
 * @details The default implementation simply tries to unwrap a QwtPolarItem pointer.
 * @param[in] itemInfo Item info object generated from itemToInfo()
 * @return A plot item when successful, otherwise nullptr
 * @sa itemToInfo()
 * \endif
 *
 * \if CHINESE
 * @brief 根据项信息对象标识绘图项
 * @details 默认实现只是尝试解包 QwtPolarItem 指针。
 * @param[in] itemInfo 由 itemToInfo() 生成的项信息对象
 * @return 成功时返回绘图项，否则返回 nullptr
 * @sa itemToInfo()
 * \endif
 */
QwtPolarItem* QwtPolarPlot::infoToItem(const QVariant& itemInfo) const
{
    if (itemInfo.canConvert< QwtPolarItem* >())
        return qvariant_cast< QwtPolarItem* >(itemInfo);

    return nullptr;
}
