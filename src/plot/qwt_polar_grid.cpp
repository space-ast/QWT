/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_grid.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_clipper.h"
#include "qwt_scale_map.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_round_scale_draw.h"

#include <qpainter.h>
#include <qpen.h>
#include <float.h>

static inline bool isClose(double value1, double value2)
{
    return qAbs(value1 - value2) < DBL_EPSILON;
}

namespace
{
class QwtPolarGrid_AxisData
{
public:
    QwtPolarGrid_AxisData() : isVisible(false), scaleDraw(nullptr)
    {
    }

    ~QwtPolarGrid_AxisData()
    {
        delete scaleDraw;
    }

    bool isVisible;
    mutable QwtAbstractScaleDraw* scaleDraw;
    QPen pen;
    QFont font;
};

class QwtPolarGrid_GridData
{
public:
    QwtPolarGrid_GridData() : isVisible(true), isMinorVisible(false)
    {
    }

    bool isVisible;
    bool isMinorVisible;
    QwtScaleDiv scaleDiv;

    QPen majorPen;
    QPen minorPen;
};
}

class QwtPolarGrid::PrivateData
{
public:
    QwtPolarGrid_GridData gridData[ QwtPolar::ScaleCount ];
    QwtPolarGrid_AxisData axisData[ QwtPolar::AxesCount ];
    QwtPolarGrid::DisplayFlags displayFlags;
    QwtPolarGrid::GridAttributes attributes;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Enables major and disables minor grid lines.
 *          The azimuth and right radial axis are visible. All other axes are hidden.
 *          AutoScaling is enabled.
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 启用主网格线并禁用次网格线。方位角和右侧径向轴可见，其他轴隐藏。启用自动缩放。
 * \endif
 */
QwtPolarGrid::QwtPolarGrid() : QwtPolarItem(QwtText("Grid"))
{
    m_data = new PrivateData;

    for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
        QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];
        switch (axisId) {
        case QwtPolar::AxisAzimuth: {
            axis.scaleDraw = new QwtRoundScaleDraw;
            axis.scaleDraw->setTickLength(QwtScaleDiv::MinorTick, 2);
            axis.scaleDraw->setTickLength(QwtScaleDiv::MediumTick, 2);
            axis.scaleDraw->setTickLength(QwtScaleDiv::MajorTick, 4);
            axis.isVisible = true;
            break;
        }
        case QwtPolar::AxisLeft: {
            QwtScaleDraw* scaleDraw = new QwtScaleDraw;
            scaleDraw->setAlignment(QwtScaleDraw::BottomScale);

            axis.scaleDraw = scaleDraw;
            axis.isVisible = false;
            break;
        }
        case QwtPolar::AxisRight: {
            QwtScaleDraw* scaleDraw = new QwtScaleDraw;
            scaleDraw->setAlignment(QwtScaleDraw::BottomScale);

            axis.scaleDraw = scaleDraw;
            axis.isVisible = true;
            break;
        }
        case QwtPolar::AxisTop: {
            QwtScaleDraw* scaleDraw = new QwtScaleDraw;
            scaleDraw->setAlignment(QwtScaleDraw::LeftScale);

            axis.scaleDraw = scaleDraw;
            axis.isVisible = false;
            break;
        }
        case QwtPolar::AxisBottom: {
            QwtScaleDraw* scaleDraw = new QwtScaleDraw;
            scaleDraw->setAlignment(QwtScaleDraw::LeftScale);

            axis.scaleDraw = scaleDraw;
            axis.isVisible = true;
            break;
        }
        default:;
        }
    }

    m_data->attributes = AutoScaling;

    m_data->displayFlags = DisplayFlags();
    m_data->displayFlags |= SmartOriginLabel;
    m_data->displayFlags |= HideMaxRadiusLabel;
    m_data->displayFlags |= ClipAxisBackground;
    m_data->displayFlags |= SmartScaleDraw;
    m_data->displayFlags |= ClipGridLines;

    setZ(10.0);
    setRenderHint(RenderAntialiased, true);
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
QwtPolarGrid::~QwtPolarGrid()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PolarGrid
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PolarGrid
 * \endif
 */
int QwtPolarGrid::rtti() const
{
    return QwtPolarItem::Rtti_PolarGrid;
}

/**
 * \if ENGLISH
 * @brief Change the display flags
 * @param[in] flag Display flag to modify
 * @param[in] on True to enable, false to disable
 * @sa DisplayFlag
 * \endif
 *
 * \if CHINESE
 * @brief 更改显示标志
 * @param[in] flag 要修改的显示标志
 * @param[in] on true 启用，false 禁用
 * @sa DisplayFlag
 * \endif
 */
void QwtPolarGrid::setDisplayFlag(DisplayFlag flag, bool on)
{
    if (((m_data->displayFlags & flag) != 0) != on) {
        if (on)
            m_data->displayFlags |= flag;
        else
            m_data->displayFlags &= ~flag;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test a display flag
 * @param[in] flag Display flag to test
 * @return True if flag is enabled
 * @sa DisplayFlag
 * \endif
 *
 * \if CHINESE
 * @brief 测试显示标志
 * @param[in] flag 要测试的显示标志
 * @return 如果标志已启用则返回 true
 * @sa DisplayFlag
 * \endif
 */
bool QwtPolarGrid::testDisplayFlag(DisplayFlag flag) const
{
    return (m_data->displayFlags & flag);
}

/**
 * \if ENGLISH
 * @brief Specify an attribute for the grid
 * @param[in] attribute Grid attribute to set
 * @param[in] on True to enable, false to disable
 * @sa GridAttribute, testGridAttribute(), updateScaleDiv(), QwtPolarPlot::zoom(), QwtPolarPlot::scaleDiv()
 * \endif
 *
 * \if CHINESE
 * @brief 指定网格的属性
 * @param[in] attribute 要设置的网格属性
 * @param[in] on true 启用，false 禁用
 * @sa GridAttribute, testGridAttribute(), updateScaleDiv(), QwtPolarPlot::zoom(), QwtPolarPlot::scaleDiv()
 * \endif
 */
void QwtPolarGrid::setGridAttribute(GridAttribute attribute, bool on)
{
    if (bool(m_data->attributes & attribute) == on)
        return;

    if (on)
        m_data->attributes |= attribute;
    else
        m_data->attributes &= ~attribute;

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Test a grid attribute
 * @param[in] attribute Grid attribute to test
 * @return True if attribute is enabled
 * @sa GridAttribute, setGridAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试网格属性
 * @param[in] attribute 要测试的网格属性
 * @return 如果属性已启用则返回 true
 * @sa GridAttribute, setGridAttribute()
 * \endif
 */
bool QwtPolarGrid::testGridAttribute(GridAttribute attribute) const
{
    return m_data->attributes & attribute;
}

/**
 * \if ENGLISH
 * @brief Assign a pen for painting an axis
 * @param[in] axisId Axis id (QwtPolar::Axis)
 * @param[in] pen Pen for the axis
 * @sa axisPen()
 * \endif
 *
 * \if CHINESE
 * @brief 为绘制轴分配画笔
 * @param[in] axisId 轴标识 (QwtPolar::Axis)
 * @param[in] pen 轴的画笔
 * @sa axisPen()
 * \endif
 */
void QwtPolarGrid::setAxisPen(int axisId, const QPen& pen)
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return;

    QwtPolarGrid_AxisData& axisData = m_data->axisData[ axisId ];
    if (axisData.pen != pen) {
        axisData.pen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Show/hide grid lines for a scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @param[in] show True to show, false to hide
 * @sa QwtPolar::Scale, isGridVisible()
 * \endif
 *
 * \if CHINESE
 * @brief 显示/隐藏刻度的网格线
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @param[in] show true 显示，false 隐藏
 * @sa QwtPolar::Scale, isGridVisible()
 * \endif
 */
void QwtPolarGrid::showGrid(int scaleId, bool show)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
    if (grid.isVisible != show) {
        grid.isVisible = show;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Check if grid lines are visible for a scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @return True if grid lines are enabled
 * @sa QwtPolar::Scale, showGrid()
 * \endif
 *
 * \if CHINESE
 * @brief 检查刻度的网格线是否可见
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @return 如果网格线已启用则返回 true
 * @sa QwtPolar::Scale, showGrid()
 * \endif
 */
bool QwtPolarGrid::isGridVisible(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return false;

    return m_data->gridData[ scaleId ].isVisible;
}

/**
 * \if ENGLISH
 * @brief Show/hide minor grid lines for a scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @param[in] show True to show, false to hide
 * @details To display minor grid lines, showGrid() needs to be enabled too.
 * @sa showGrid()
 * \endif
 *
 * \if CHINESE
 * @brief 显示/隐藏刻度的次网格线
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @param[in] show true 显示，false 隐藏
 * @details 要显示次网格线，showGrid() 也需要启用。
 * @sa showGrid()
 * \endif
 */
void QwtPolarGrid::showMinorGrid(int scaleId, bool show)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
    if (grid.isMinorVisible != show) {
        grid.isMinorVisible = show;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Check if minor grid lines are visible for a scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @return True if minor grid lines are enabled
 * @sa showMinorGrid()
 * \endif
 *
 * \if CHINESE
 * @brief 检查刻度的次网格线是否可见
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @return 如果次网格线已启用则返回 true
 * @sa showMinorGrid()
 * \endif
 */
bool QwtPolarGrid::isMinorGridVisible(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return false;

    return m_data->gridData[ scaleId ].isMinorVisible;
}

/**
 * \if ENGLISH
 * @brief Show/hide an axis
 * @param[in] axisId Axis id (QwtPolar::Axis)
 * @param[in] show True to show, false to hide
 * @sa isAxisVisible()
 * \endif
 *
 * \if CHINESE
 * @brief 显示/隐藏轴
 * @param[in] axisId 轴标识 (QwtPolar::Axis)
 * @param[in] show true 显示，false 隐藏
 * @sa isAxisVisible()
 * \endif
 */
void QwtPolarGrid::showAxis(int axisId, bool show)
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return;

    QwtPolarGrid_AxisData& axisData = m_data->axisData[ axisId ];
    if (axisData.isVisible != show) {
        axisData.isVisible = show;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Check if an axis is visible
 * @param[in] axisId Axis id (QwtPolar::Axis)
 * @return True if the axis is visible
 * @sa showAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 检查轴是否可见
 * @param[in] axisId 轴标识 (QwtPolar::Axis)
 * @return 如果轴可见则返回 true
 * @sa showAxis()
 * \endif
 */
bool QwtPolarGrid::isAxisVisible(int axisId) const
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return false;

    return m_data->axisData[ axisId ].isVisible;
}

/**
 * \if ENGLISH
 * @brief Assign a pen for all axes and grid lines
 * @param[in] pen Pen for all axes and grid lines
 * @sa setMajorGridPen(), setMinorGridPen(), setAxisPen()
 * \endif
 *
 * \if CHINESE
 * @brief 为所有轴和网格线分配画笔
 * @param[in] pen 所有轴和网格线的画笔
 * @sa setMajorGridPen(), setMinorGridPen(), setAxisPen()
 * \endif
 */
void QwtPolarGrid::setPen(const QPen& pen)
{
    bool isChanged = false;

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++) {
        QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
        if (grid.majorPen != pen || grid.minorPen != pen) {
            grid.majorPen = pen;
            grid.minorPen = pen;
            isChanged     = true;
        }
    }
    for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
        QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];
        if (axis.pen != pen) {
            axis.pen  = pen;
            isChanged = true;
        }
    }
    if (isChanged)
        itemChanged();
}

/**
 * \if ENGLISH
 * @brief Assign a font for all scale tick labels
 * @param[in] font Font for all scale tick labels
 * @sa setAxisFont()
 * \endif
 *
 * \if CHINESE
 * @brief 为所有刻度标签分配字体
 * @param[in] font 所有刻度标签的字体
 * @sa setAxisFont()
 * \endif
 */
void QwtPolarGrid::setFont(const QFont& font)
{
    bool isChanged = false;
    for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
        QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];
        if (axis.font != font) {
            axis.font = font;
            isChanged = true;
        }
    }
    if (isChanged)
        itemChanged();
}

/**
 * \if ENGLISH
 * @brief Assign a pen for the major grid lines
 * @param[in] pen Pen for major grid lines
 * @sa setPen(), setMinorGridPen(), majorGridPen()
 * \endif
 *
 * \if CHINESE
 * @brief 为主网格线分配画笔
 * @param[in] pen 主网格线的画笔
 * @sa setPen(), setMinorGridPen(), majorGridPen()
 * \endif
 */
void QwtPolarGrid::setMajorGridPen(const QPen& pen)
{
    bool isChanged = false;

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++) {
        QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
        if (grid.majorPen != pen) {
            grid.majorPen = pen;
            isChanged     = true;
        }
    }
    if (isChanged)
        itemChanged();
}

/**
 * \if ENGLISH
 * @brief Assign a pen for the major grid lines of a specific scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @param[in] pen Pen for major grid lines
 * @sa setPen(), setMinorGridPen(), majorGridPen()
 * \endif
 *
 * \if CHINESE
 * @brief 为特定刻度的主网格线分配画笔
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @param[in] pen 主网格线的画笔
 * @sa setPen(), setMinorGridPen(), majorGridPen()
 * \endif
 */
void QwtPolarGrid::setMajorGridPen(int scaleId, const QPen& pen)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
    if (grid.majorPen != pen) {
        grid.majorPen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen for painting the major grid lines of a specific scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @return Pen for major grid lines
 * @sa setMajorGridPen(), minorGridPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定刻度主网格线的画笔
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @return 主网格线的画笔
 * @sa setMajorGridPen(), minorGridPen()
 * \endif
 */
QPen QwtPolarGrid::majorGridPen(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return QPen();

    const QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
    return grid.majorPen;
}

/**
 * \if ENGLISH
 * @brief Assign a pen for the minor grid lines
 * @param[in] pen Pen for minor grid lines
 * @sa setPen(), setMajorGridPen(), minorGridPen()
 * \endif
 *
 * \if CHINESE
 * @brief 为次网格线分配画笔
 * @param[in] pen 次网格线的画笔
 * @sa setPen(), setMajorGridPen(), minorGridPen()
 * \endif
 */
void QwtPolarGrid::setMinorGridPen(const QPen& pen)
{
    bool isChanged = false;

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++) {
        QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
        if (grid.minorPen != pen) {
            grid.minorPen = pen;
            isChanged     = true;
        }
    }
    if (isChanged)
        itemChanged();
}

/**
 * \if ENGLISH
 * @brief Assign a pen for the minor grid lines of a specific scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @param[in] pen Pen for minor grid lines
 * @sa setPen(), setMajorGridPen(), minorGridPen()
 * \endif
 *
 * \if CHINESE
 * @brief 为特定刻度的次网格线分配画笔
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @param[in] pen 次网格线的画笔
 * @sa setPen(), setMajorGridPen(), minorGridPen()
 * \endif
 */
void QwtPolarGrid::setMinorGridPen(int scaleId, const QPen& pen)
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
    if (grid.minorPen != pen) {
        grid.minorPen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen for painting the minor grid lines of a specific scale
 * @param[in] scaleId Scale id (QwtPolar::Scale)
 * @return Pen for minor grid lines
 * @sa setMinorGridPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定刻度次网格线的画笔
 * @param[in] scaleId 刻度标识 (QwtPolar::Scale)
 * @return 次网格线的画笔
 * @sa setMinorGridPen()
 * \endif
 */
QPen QwtPolarGrid::minorGridPen(int scaleId) const
{
    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return QPen();

    const QwtPolarGrid_GridData& grid = m_data->gridData[ scaleId ];
    return grid.minorPen;
}

/**
 * \if ENGLISH
 * @brief Get the pen for painting a specific axis
 * @param[in] axisId Axis id (QwtPolar::Axis)
 * @return Pen for the axis
 * @sa setAxisPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定轴的画笔
 * @param[in] axisId 轴标识 (QwtPolar::Axis)
 * @return 轴的画笔
 * @sa setAxisPen()
 * \endif
 */
QPen QwtPolarGrid::axisPen(int axisId) const
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return QPen();

    return m_data->axisData[ axisId ].pen;
}

/**
 * \if ENGLISH
 * @brief Assign a font for the tick labels of a specific axis
 * @param[in] axisId Axis id (QwtPolar::Axis)
 * @param[in] font New font
 * @sa axisFont()
 * \endif
 *
 * \if CHINESE
 * @brief 为特定轴的刻度标签分配字体
 * @param[in] axisId 轴标识 (QwtPolar::Axis)
 * @param[in] font 新字体
 * @sa axisFont()
 * \endif
 */
void QwtPolarGrid::setAxisFont(int axisId, const QFont& font)
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return;

    QwtPolarGrid_AxisData& axisData = m_data->axisData[ axisId ];
    if (axisData.font != font) {
        axisData.font = font;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the font for the tick labels of a specific axis
 * @param[in] axisId Axis id (QwtPolar::Axis)
 * @return Font for the axis
 * @sa setAxisFont()
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定轴的刻度标签字体
 * @param[in] axisId 轴标识 (QwtPolar::Axis)
 * @return 轴的字体
 * @sa setAxisFont()
 * \endif
 */
QFont QwtPolarGrid::axisFont(int axisId) const
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return QFont();

    return m_data->axisData[ axisId ].font;
}

/**
 * \if ENGLISH
 * @brief Draw the grid and axes
 * @param[in] painter Painter
 * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param[in] radialMap Maps radius values into painter coordinates
 * @param[in] pole Position of the pole in painter coordinates
 * @param[in] radius Radius of the complete plot area in painter coordinates
 * @param[in] canvasRect Contents rect of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制网格和轴
 * @param[in] painter 绘图器
 * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
 * @param[in] radialMap 将半径值映射到绘图器坐标
 * @param[in] pole 绘图器坐标中极点的位置
 * @param[in] radius 绘图器坐标中完整绘图区域的半径
 * @param[in] canvasRect 绘图器坐标中画布的内容矩形
 * \endif
 */
void QwtPolarGrid::draw(QPainter* painter,
                        const QwtScaleMap& azimuthMap,
                        const QwtScaleMap& radialMap,
                        const QPointF& pole,
                        double radius,
                        const QRectF& canvasRect) const
{
    updateScaleDraws(azimuthMap, radialMap, pole, radius);

    painter->save();

    if (testDisplayFlag(ClipAxisBackground)) {
        QRegion clipRegion(canvasRect.toRect());
        for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
            const QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];
            if (axisId != QwtPolar::AxisAzimuth && axis.isVisible) {
                QwtScaleDraw* scaleDraw = static_cast< QwtScaleDraw* >(axis.scaleDraw);
                if (scaleDraw->hasComponent(QwtScaleDraw::Labels)) {
                    const QList< double >& ticks = scaleDraw->scaleDiv().ticks(QwtScaleDiv::MajorTick);
                    for (int i = 0; i < int(ticks.size()); i++) {
                        if (!scaleDraw->scaleDiv().contains(ticks[ i ]))
                            continue;

                        QRect labelRect = scaleDraw->boundingLabelRect(axis.font, ticks[ i ]);

                        const int margin = 2;
                        labelRect.adjust(-margin, -margin, margin, margin);

                        if (labelRect.isValid())
                            clipRegion -= QRegion(labelRect);
                    }
                }
            }
        }
        painter->setClipRegion(clipRegion);
    }

    //  draw radial grid

    const QwtPolarGrid_GridData& radialGrid = m_data->gridData[ QwtPolar::Radius ];
    if (radialGrid.isVisible && radialGrid.isMinorVisible) {
        painter->setPen(radialGrid.minorPen);

        drawCircles(painter, canvasRect, pole, radialMap, radialGrid.scaleDiv.ticks(QwtScaleDiv::MinorTick));
        drawCircles(painter, canvasRect, pole, radialMap, radialGrid.scaleDiv.ticks(QwtScaleDiv::MediumTick));
    }
    if (radialGrid.isVisible) {
        painter->setPen(radialGrid.majorPen);

        drawCircles(painter, canvasRect, pole, radialMap, radialGrid.scaleDiv.ticks(QwtScaleDiv::MajorTick));
    }

    // draw azimuth grid

    const QwtPolarGrid_GridData& azimuthGrid = m_data->gridData[ QwtPolar::Azimuth ];

    if (azimuthGrid.isVisible && azimuthGrid.isMinorVisible) {
        painter->setPen(azimuthGrid.minorPen);

        drawRays(painter, canvasRect, pole, radius, azimuthMap, azimuthGrid.scaleDiv.ticks(QwtScaleDiv::MinorTick));
        drawRays(painter, canvasRect, pole, radius, azimuthMap, azimuthGrid.scaleDiv.ticks(QwtScaleDiv::MediumTick));
    }
    if (azimuthGrid.isVisible) {
        painter->setPen(azimuthGrid.majorPen);

        drawRays(painter, canvasRect, pole, radius, azimuthMap, azimuthGrid.scaleDiv.ticks(QwtScaleDiv::MajorTick));
    }
    painter->restore();

    for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
        const QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];
        if (axis.isVisible) {
            painter->save();
            drawAxis(painter, axisId);
            painter->restore();
        }
    }
}

/*!
   Draw lines from the pole

   \param painter Painter
   \param canvasRect Contents rect of the canvas in painter coordinates
   \param pole Position of the pole in painter coordinates
   \param radius Length of the lines in painter coordinates
   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param values Azimuth values, indicating the direction of the lines
 */
void QwtPolarGrid::drawRays(QPainter* painter,
                            const QRectF& canvasRect,
                            const QPointF& pole,
                            double radius,
                            const QwtScaleMap& azimuthMap,
                            const QList< double >& values) const
{
    for (int i = 0; i < int(values.size()); i++) {
        double azimuth = azimuthMap.transform(values[ i ]);
        azimuth        = ::fmod(azimuth, 2 * M_PI);

        bool skipLine = false;
        if (testDisplayFlag(SmartScaleDraw)) {
            const QwtAbstractScaleDraw::ScaleComponent bone = QwtAbstractScaleDraw::Backbone;
            if (isClose(azimuth, 0.0)) {
                const QwtPolarGrid_AxisData& axis = m_data->axisData[ QwtPolar::AxisRight ];
                if (axis.isVisible && axis.scaleDraw->hasComponent(bone))
                    skipLine = true;
            } else if (isClose(azimuth, M_PI / 2)) {
                const QwtPolarGrid_AxisData& axis = m_data->axisData[ QwtPolar::AxisTop ];
                if (axis.isVisible && axis.scaleDraw->hasComponent(bone))
                    skipLine = true;
            } else if (isClose(azimuth, M_PI)) {
                const QwtPolarGrid_AxisData& axis = m_data->axisData[ QwtPolar::AxisLeft ];
                if (axis.isVisible && axis.scaleDraw->hasComponent(bone))
                    skipLine = true;
            } else if (isClose(azimuth, 3 * M_PI / 2.0)) {
                const QwtPolarGrid_AxisData& axis = m_data->axisData[ QwtPolar::AxisBottom ];
                if (axis.isVisible && axis.scaleDraw->hasComponent(bone))
                    skipLine = true;
            }
        }
        if (!skipLine) {
            const QPointF pos = qwtPolar2Pos(pole, radius, azimuth);

            /*
                Qt4 is horrible slow, when painting primitives,
                with coordinates far outside the visible area.
             */

            QPolygonF polygon(2);
            polygon[ 0 ] = pole.toPoint();
            polygon[ 1 ] = pos.toPoint();

            if (testDisplayFlag(ClipGridLines))
                QwtClipper::clipPolygonF(canvasRect, polygon);

            QwtPainter::drawPolyline(painter, polygon);
        }
    }
}

/*!
   Draw circles

   \param painter Painter
   \param canvasRect Contents rect of the canvas in painter coordinates
   \param pole Position of the pole in painter coordinates
   \param radialMap Maps radius values into painter coordinates.
   \param values Radial values, indicating the distances from the pole
 */
void QwtPolarGrid::drawCircles(QPainter* painter,
                               const QRectF& canvasRect,
                               const QPointF& pole,
                               const QwtScaleMap& radialMap,
                               const QList< double >& values) const
{
    for (int i = 0; i < int(values.size()); i++) {
        const double val = values[ i ];

        const QwtPolarGrid_GridData& gridData = m_data->gridData[ QwtPolar::Radius ];

        bool skipLine = false;
        if (testDisplayFlag(SmartScaleDraw)) {
            const QwtPolarGrid_AxisData& axis = m_data->axisData[ QwtPolar::AxisAzimuth ];
            if (axis.isVisible && axis.scaleDraw->hasComponent(QwtAbstractScaleDraw::Backbone)) {
                if (isClose(val, gridData.scaleDiv.upperBound()))
                    skipLine = true;
            }
        }

        if (isClose(val, gridData.scaleDiv.lowerBound()))
            skipLine = true;

        if (!skipLine) {
            const double radius = radialMap.transform(val);

            QRectF outerRect(0, 0, 2 * radius, 2 * radius);
            outerRect.moveCenter(pole);

            if (testDisplayFlag(ClipGridLines)) {
                /*
                    Qt4 is horrible slow, when painting primitives,
                    with coordinates far outside the visible area.
                    We need to clip.
                 */

                const QVector< QwtInterval > angles = QwtClipper::clipCircle(canvasRect, pole, radius);

                for (int j = 0; j < angles.size(); j++) {
                    const QwtInterval intv = angles[ j ];

                    if (intv.minValue() == 0 && intv.maxValue() == 2 * M_PI) {
                        QwtPainter::drawEllipse(painter, outerRect);
                    } else {
                        const double from = qwtDegrees(intv.minValue());
                        const double to   = qwtDegrees(intv.maxValue());

                        double span = to - from;
                        if (span < 0.0)
                            span += 360.0;

                        painter->drawArc(outerRect, qRound(from * 16), qRound(span * 16));
                    }
                }
            } else {
                QwtPainter::drawEllipse(painter, outerRect);
            }
        }
    }
}

/*!
   Paint an axis

   \param painter Painter
   \param axisId Axis id (QwtPolar::Axis)
 */
void QwtPolarGrid::drawAxis(QPainter* painter, int axisId) const
{
    if (axisId < 0 || axisId >= QwtPolar::AxesCount)
        return;

    QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];

    painter->setPen(axis.pen);
    painter->setFont(axis.font);

    QPalette pal;
    pal.setColor(QPalette::WindowText, axis.pen.color());
    pal.setColor(QPalette::Text, axis.pen.color());

    axis.scaleDraw->draw(painter, pal);
}

/*!
   Update the axis scale draw geometries

   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param radius Radius of the complete plot area in painter coordinates

   \sa updateScaleDiv()
 */
void QwtPolarGrid::updateScaleDraws(const QwtScaleMap& azimuthMap,
                                    const QwtScaleMap& radialMap,
                                    const QPointF& pole,
                                    double radius) const
{
    const QPoint p = pole.toPoint();

    const QwtInterval interval = m_data->gridData[ QwtPolar::ScaleRadius ].scaleDiv.interval();

    const int min = radialMap.transform(interval.minValue());
    const int max = radialMap.transform(interval.maxValue());
    const int l   = max - min;

    for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
        QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];

        if (axisId == QwtPolar::AxisAzimuth) {
            QwtRoundScaleDraw* scaleDraw = static_cast< QwtRoundScaleDraw* >(axis.scaleDraw);

            scaleDraw->setRadius(qRound(radius));
            scaleDraw->moveCenter(p);

            double from = ::fmod(90.0 - qwtDegrees(azimuthMap.p1()), 360.0);
            if (from < 0.0)
                from += 360.0;

            scaleDraw->setAngleRange(from, from - 360.0);

            const QwtTransform* transform = azimuthMap.transformation();
            if (transform)
                scaleDraw->setTransformation(transform->copy());
            else
                scaleDraw->setTransformation(nullptr);
        } else {
            QwtScaleDraw* scaleDraw = static_cast< QwtScaleDraw* >(axis.scaleDraw);

            switch (axisId) {
            case QwtPolar::AxisLeft: {
                scaleDraw->move(p.x() - min, p.y());
                scaleDraw->setLength(-l);
                break;
            }
            case QwtPolar::AxisRight: {
                scaleDraw->move(p.x() + min, p.y());
                scaleDraw->setLength(l);
                break;
            }
            case QwtPolar::AxisTop: {
                scaleDraw->move(p.x(), p.y() - max);
                scaleDraw->setLength(l);
                break;
            }
            case QwtPolar::AxisBottom: {
                scaleDraw->move(p.x(), p.y() + max);
                scaleDraw->setLength(-l);
                break;
            }
            }
            const QwtTransform* transform = radialMap.transformation();
            if (transform)
                scaleDraw->setTransformation(transform->copy());
            else
                scaleDraw->setTransformation(nullptr);
        }
    }
}

/**
 * \if ENGLISH
 * @brief Update the item to changes of the axes scale division
 * @param[in] azimuthScaleDiv Scale division of the azimuth-scale
 * @param[in] radialScaleDiv Scale division of the radius-axis
 * @param[in] interval The interval of the radius-axis that is visible on the canvas
 * @details If AutoScaling is enabled the radial scale is calculated from the interval,
 *          otherwise the scales are adopted to the plot scales.
 * @sa QwtPolarPlot::setGridAttributes()
 * \endif
 *
 * \if CHINESE
 * @brief 更新项以响应轴刻度分度的变化
 * @param[in] azimuthScaleDiv 方位角刻度的刻度分度
 * @param[in] radialScaleDiv 径向轴的刻度分度
 * @param[in] interval 画布上可见的径向轴区间
 * @details 如果启用自动缩放，径向刻度将根据区间计算；否则刻度将采用绘图刻度。
 * @sa QwtPolarPlot::setGridAttributes()
 * \endif
 */
void QwtPolarGrid::updateScaleDiv(const QwtScaleDiv& azimuthScaleDiv,
                                  const QwtScaleDiv& radialScaleDiv,
                                  const QwtInterval& interval)
{
    QwtPolarGrid_GridData& radialGrid = m_data->gridData[ QwtPolar::Radius ];

    const QwtPolarPlot* plt = plot();
    if (plt && testGridAttribute(AutoScaling)) {
        const QwtScaleEngine* se = plt->scaleEngine(QwtPolar::Radius);
        radialGrid.scaleDiv      = se->divideScale(interval.minValue(),
                                              interval.maxValue(),
                                              plt->scaleMaxMajor(QwtPolar::Radius),
                                              plt->scaleMaxMinor(QwtPolar::Radius),
                                              0);
    } else {
        if (radialGrid.scaleDiv != radialScaleDiv)
            radialGrid.scaleDiv = radialScaleDiv;
    }

    QwtPolarGrid_GridData& azimuthGrid = m_data->gridData[ QwtPolar::Azimuth ];
    if (azimuthGrid.scaleDiv != azimuthScaleDiv) {
        azimuthGrid.scaleDiv = azimuthScaleDiv;
    }

    bool hasOrigin = false;
    for (int axisId = 0; axisId < QwtPolar::AxesCount; axisId++) {
        QwtPolarGrid_AxisData& axis = m_data->axisData[ axisId ];
        if (axis.isVisible && axis.scaleDraw) {
            if (axisId == QwtPolar::AxisAzimuth) {
                axis.scaleDraw->setScaleDiv(azimuthGrid.scaleDiv);
                if (testDisplayFlag(SmartScaleDraw)) {
                    axis.scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, !azimuthGrid.isVisible);
                }
            } else {
                QwtScaleDiv sd = radialGrid.scaleDiv;

                QList< double > ticks = sd.ticks(QwtScaleDiv::MajorTick);

                if (testDisplayFlag(SmartOriginLabel)) {
                    bool skipOrigin = hasOrigin;
                    if (!skipOrigin) {
                        if (axisId == QwtPolar::AxisLeft || axisId == QwtPolar::AxisRight) {
                            if (m_data->axisData[ QwtPolar::AxisBottom ].isVisible)
                                skipOrigin = true;
                        } else {
                            if (m_data->axisData[ QwtPolar::AxisLeft ].isVisible)
                                skipOrigin = true;
                        }
                    }
                    if (ticks.size() > 0 && ticks.first() == sd.lowerBound()) {
                        if (skipOrigin)
                            ticks.removeFirst();
                        else
                            hasOrigin = true;
                    }
                }

                if (testDisplayFlag(HideMaxRadiusLabel)) {
                    if (ticks.size() > 0 && ticks.last() == sd.upperBound())
                        ticks.removeLast();
                }

                sd.setTicks(QwtScaleDiv::MajorTick, ticks);
                axis.scaleDraw->setScaleDiv(sd);

                if (testDisplayFlag(SmartScaleDraw)) {
                    axis.scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, !radialGrid.isVisible);
                }
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the margin hint
 * @return Number of pixels necessary to paint the azimuth scale
 * @sa QwtRoundScaleDraw::extent()
 * \endif
 *
 * \if CHINESE
 * @brief 获取边距提示
 * @return 绘制方位角刻度所需的像素数
 * @sa QwtRoundScaleDraw::extent()
 * \endif
 */
int QwtPolarGrid::marginHint() const
{
    const QwtPolarGrid_AxisData& axis = m_data->axisData[ QwtPolar::AxisAzimuth ];
    if (axis.isVisible) {
        const int extent = axis.scaleDraw->extent(axis.font);
        return extent;
    }

    return 0;
}

/**
 * \if ENGLISH
 * @brief Get the scale draw of a specified axis
 * @param[in] axisId Axis index (QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
 * @return Scale draw for the axis, or nullptr if axis is invalid
 * @sa azimuthScaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 获取指定轴的刻度绘制器
 * @param[in] axisId 轴索引 (QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
 * @return 轴的刻度绘制器，如果轴无效则返回 nullptr
 * @sa azimuthScaleDraw()
 * \endif
 */
const QwtScaleDraw* QwtPolarGrid::scaleDraw(int axisId) const
{
    if (axisId >= QwtPolar::AxisLeft && axisId <= QwtPolar::AxisBottom)
        return static_cast< QwtScaleDraw* >(m_data->axisData[ axisId ].scaleDraw);

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Get the scale draw of a specified axis
 * @param[in] axisId Axis index (QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
 * @return Scale draw for the axis, or nullptr if axis is invalid
 * @sa setScaleDraw(), azimuthScaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 获取指定轴的刻度绘制器
 * @param[in] axisId 轴索引 (QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
 * @return 轴的刻度绘制器，如果轴无效则返回 nullptr
 * @sa setScaleDraw(), azimuthScaleDraw()
 * \endif
 */
QwtScaleDraw* QwtPolarGrid::scaleDraw(int axisId)
{
    if (axisId >= QwtPolar::AxisLeft && axisId <= QwtPolar::AxisBottom)
        return static_cast< QwtScaleDraw* >(m_data->axisData[ axisId ].scaleDraw);

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Set a scale draw for a specified axis
 * @param[in] axisId Axis index (QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
 * @param[in] scaleDraw Object responsible for drawing scales (ownership is transferred)
 * @sa scaleDraw(), setAzimuthScaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 为指定轴设置刻度绘制器
 * @param[in] axisId 轴索引 (QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
 * @param[in] scaleDraw 负责绘制刻度的对象（所有权转移）
 * @sa scaleDraw(), setAzimuthScaleDraw()
 * \endif
 */
void QwtPolarGrid::setScaleDraw(int axisId, QwtScaleDraw* scaleDraw)
{
    if (axisId < QwtPolar::AxisLeft || axisId > QwtPolar::AxisBottom)
        return;

    QwtPolarGrid_AxisData& axisData = m_data->axisData[ axisId ];
    if (axisData.scaleDraw != scaleDraw) {
        delete axisData.scaleDraw;
        axisData.scaleDraw = scaleDraw;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the scale draw for the azimuth scale
 * @return Scale draw for the azimuth scale
 * @sa setAzimuthScaleDraw(), scaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 获取方位角刻度的刻度绘制器
 * @return 方位角刻度的刻度绘制器
 * @sa setAzimuthScaleDraw(), scaleDraw()
 * \endif
 */
const QwtRoundScaleDraw* QwtPolarGrid::azimuthScaleDraw() const
{
    return static_cast< QwtRoundScaleDraw* >(m_data->axisData[ QwtPolar::AxisAzimuth ].scaleDraw);
}

/**
 * \if ENGLISH
 * @brief Get the scale draw for the azimuth scale
 * @return Scale draw for the azimuth scale
 * @sa setAzimuthScaleDraw(), scaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 获取方位角刻度的刻度绘制器
 * @return 方位角刻度的刻度绘制器
 * @sa setAzimuthScaleDraw(), scaleDraw()
 * \endif
 */
QwtRoundScaleDraw* QwtPolarGrid::azimuthScaleDraw()
{
    return static_cast< QwtRoundScaleDraw* >(m_data->axisData[ QwtPolar::AxisAzimuth ].scaleDraw);
}

/**
 * \if ENGLISH
 * @brief Set a scale draw for the azimuth scale
 * @param[in] scaleDraw Object responsible for drawing scales (ownership is transferred)
 * @sa azimuthScaleDraw(), setScaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 为方位角刻度设置刻度绘制器
 * @param[in] scaleDraw 负责绘制刻度的对象（所有权转移）
 * @sa azimuthScaleDraw(), setScaleDraw()
 * \endif
 */
void QwtPolarGrid::setAzimuthScaleDraw(QwtRoundScaleDraw* scaleDraw)
{
    QwtPolarGrid_AxisData& axisData = m_data->axisData[ QwtPolar::AxisAzimuth ];
    if (axisData.scaleDraw != scaleDraw) {
        delete axisData.scaleDraw;
        axisData.scaleDraw = scaleDraw;
        itemChanged();
    }
}
