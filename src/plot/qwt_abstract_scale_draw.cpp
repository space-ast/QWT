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

#include "qwt_abstract_scale_draw.h"
#include "qwt_text.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpalette.h>
#include <qmap.h>
#include <qlist.h>
#include <qlocale.h>

class QwtAbstractScaleDraw::PrivateData
{
public:
    PrivateData() : spacing(4.0), penWidthF(0.0), minExtent(0.0)
    {
        components = QwtAbstractScaleDraw::Backbone | QwtAbstractScaleDraw::Ticks | QwtAbstractScaleDraw::Labels;

        tickLength[ QwtScaleDiv::MinorTick ]  = 4.0;
        tickLength[ QwtScaleDiv::MediumTick ] = 6.0;
        tickLength[ QwtScaleDiv::MajorTick ]  = 8.0;
    }

    ScaleComponents components;

    QwtScaleMap map;
    QwtScaleDiv scaleDiv;

    double spacing;
    double tickLength[ QwtScaleDiv::NTickTypes ];
    qreal penWidthF { 0.0 };
    qreal penWidthOffset { 1.0 };

    bool isSelected { false };

    double minExtent;

    QMap< double, QwtText > labelCache;
};

/**
 * \if ENGLISH
 * @brief Constructor for QwtAbstractScaleDraw
 * @details The range of the scale is initialized to [0, 100],
 *          the spacing (distance between ticks and labels) is set to 4,
 *          the tick lengths are set to 4, 6 and 8 pixels.
 * \endif
 * \if CHINESE
 * @brief QwtAbstractScaleDraw 构造函数
 * @details 刻度范围初始化为 [0, 100]，
 *          间距（刻度线和标签之间的距离）设置为 4，
 *          刻度线长度设置为 4、6 和 8 像素。
 * \endif
 */
QwtAbstractScaleDraw::QwtAbstractScaleDraw()
{
    m_data = new QwtAbstractScaleDraw::PrivateData;
}

/**
 * \if ENGLISH
 * @brief Destructor for QwtAbstractScaleDraw
 * \endif
 * \if CHINESE
 * @brief QwtAbstractScaleDraw 析构函数
 * \endif
 */
QwtAbstractScaleDraw::~QwtAbstractScaleDraw()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Enable or disable a scale component
 * @param component Scale component (Backbone, Ticks, or Labels)
 * @param enable True to enable, false to disable
 * \sa hasComponent()
 * \endif
 * \if CHINESE
 * @brief 启用或禁用刻度组件
 * @param component 刻度组件（主干、刻度线或标签）
 * @param enable true 启用，false 禁用
 * \sa hasComponent()
 * \endif
 */
void QwtAbstractScaleDraw::enableComponent(ScaleComponent component, bool enable)
{
    if (enable)
        m_data->components |= component;
    else
        m_data->components &= ~component;
}

/**
 * \if ENGLISH
 * @brief Check if a component is enabled
 * @param component Component type
 * @return True if component is enabled
 * \sa enableComponent()
 * \endif
 * \if CHINESE
 * @brief 检查组件是否已启用
 * @param component 组件类型
 * @return 如果组件已启用则返回 true
 * \sa enableComponent()
 * \endif
 */
bool QwtAbstractScaleDraw::hasComponent(ScaleComponent component) const
{
    return (m_data->components & component);
}

/**
 * \if ENGLISH
 * @brief Set the scale division
 * @param scaleDiv New scale division object
 * @details This also updates the scale map and clears the label cache.
 * \endif
 * \if CHINESE
 * @brief 设置刻度划分
 * @param scaleDiv 新的刻度划分对象
 * @details 这还会更新刻度映射并清空标签缓存。
 * \endif
 */
void QwtAbstractScaleDraw::setScaleDiv(const QwtScaleDiv& scaleDiv)
{
    m_data->scaleDiv = scaleDiv;
    m_data->map.setScaleInterval(scaleDiv.lowerBound(), scaleDiv.upperBound());
    m_data->labelCache.clear();
}

/**
 * \if ENGLISH
 * @brief Set the scale transformation
 * @param transformation New scale transformation object
 * \endif
 * \if CHINESE
 * @brief 设置刻度变换
 * @param transformation 新的刻度变换对象
 * \endif
 */
void QwtAbstractScaleDraw::setTransformation(QwtTransform* transformation)
{
    m_data->map.setTransformation(transformation);
}

/**
 * \if ENGLISH
 * @brief Return the scale map (const version)
 * @return Map for translating between scale and pixel values
 * \endif
 * \if CHINESE
 * @brief 返回刻度映射（常量版本）
 * @return 用于在刻度和像素值之间转换的映射
 * \endif
 */
const QwtScaleMap& QwtAbstractScaleDraw::scaleMap() const
{
    return m_data->map;
}

/**
 * \if ENGLISH
 * @brief Return the scale map (non-const version)
 * @return Map for translating between scale and pixel values
 * \endif
 * \if CHINESE
 * @brief 返回刻度映射（非常量版本）
 * @return 用于在刻度和像素值之间转换的映射
 * \endif
 */
QwtScaleMap& QwtAbstractScaleDraw::scaleMap()
{
    return m_data->map;
}

/**
 * \if ENGLISH
 * @brief Return the scale division
 * @return Current scale division object
 * \endif
 * \if CHINESE
 * @brief 返回刻度划分
 * @return 当前刻度划分对象
 * \endif
 */
const QwtScaleDiv& QwtAbstractScaleDraw::scaleDiv() const
{
    return m_data->scaleDiv;
}

/**
 * \if ENGLISH
 * @brief Specify the width of the scale pen
 * @param width Pen width
 * \sa penWidth()
 * \endif
 * \if CHINESE
 * @brief 设置刻度画笔宽度
 * @param width 画笔宽度
 * \sa penWidth()
 * \endif
 */
void QwtAbstractScaleDraw::setPenWidthF(qreal width)
{
    if (width < 0.0)
        width = 0.0;

    m_data->penWidthF = width;
}

/**
 * \if ENGLISH
 * @brief Get the scale pen width
 * @return Scale pen width
 * \sa setPenWidth()
 * \endif
 * \if CHINESE
 * @brief 获取刻度画笔宽度
 * @return 刻度画笔宽度
 * \sa setPenWidth()
 * \endif
 */
qreal QwtAbstractScaleDraw::penWidthF() const
{
    return m_data->penWidthF;
}

/**
 * \if ENGLISH
 * @brief Set whether the scale draw is selected
 * @param on True to select, false to deselect
 * \endif
 * \if CHINESE
 * @brief 设置刻度绘制是否被选中
 * @param on true 为选中，false 为取消选中
 * \endif
 */
void QwtAbstractScaleDraw::setSelected(bool on)
{
    m_data->isSelected = on;
}

/**
 * \if ENGLISH
 * @brief Check if the scale draw is selected
 * @return True if selected, false otherwise
 * \endif
 * \if CHINESE
 * @brief 检查刻度绘制是否被选中
 * @return 如果被选中返回 true，否则返回 false
 * \endif
 */
bool QwtAbstractScaleDraw::isSelected() const
{
    return m_data->isSelected;
}

/**
 * \if ENGLISH
 * @brief Set the pen width offset for the axis when it is in selected state
 * @details When an axis (e.g., X-axis or Y-axis) is selected by the user, the pen width used
 *          for drawing will be increased by this offset value, achieving a visual highlighting effect.
 * @param offset The additional width value to be added when selected (unit: pixels).
 *               This value should be non-negative. If it is 0, the line width in selected state
 *               will be the same as in normal state.
 * \sa selectedPenWidthOffset()
 * \endif
 * \if CHINESE
 * @brief 设置坐标轴在选中状态下的画笔宽度附加值
 * @details 当坐标轴（如 X 轴或 Y 轴）被用户选中时，绘制的画笔宽度会在原始宽度的基础上
 *          增加这个附加值，实现视觉上的突出显示效果。
 * @param offset 选中时增加的宽度值（单位：像素）。该值应为非负数。
 *               如果为 0，则选中状态下的线宽与普通状态相同。
 * \sa selectedPenWidthOffset()
 * \endif
 */
void QwtAbstractScaleDraw::setSelectedPenWidthOffset(qreal offset)
{
    m_data->penWidthOffset = offset;
}

/**
 * \if ENGLISH
 * @brief Get the current pen width offset for the axis when it is in selected state
 * @return The current width offset value
 * \sa setSelectedPenWidthOffset()
 * \endif
 * \if CHINESE
 * @brief 获取当前坐标轴在选中状态下的画笔宽度附加值
 * @return 当前的宽度附加值
 * \sa setSelectedPenWidthOffset()
 * \endif
 */
qreal QwtAbstractScaleDraw::selectedPenWidthOffset() const
{
    return m_data->penWidthOffset;
}

/**
 * \if ENGLISH
 * @brief Draw the scale
 * @param painter The painter
 * @param palette Palette, text color is used for the labels, foreground color for ticks and backbone
 * \endif
 * \if CHINESE
 * @brief 绘制刻度
 * @param painter 绘制器
 * @param palette 调色板，文本颜色用于标签，前景色用于刻度线和主干
 * \endif
 */
void QwtAbstractScaleDraw::draw(QPainter* painter, const QPalette& palette) const
{
    painter->save();

    QPen pen = painter->pen();
    pen.setWidthF(m_data->penWidthF);
    if (isSelected()) {
        if (qFuzzyIsNull(m_data->penWidthF)) {
            pen.setWidthF(1.0 + m_data->penWidthOffset);
        } else {
            pen.setWidthF(m_data->penWidthF + m_data->penWidthOffset);
        }
    }
    painter->setPen(pen);

    if (hasComponent(QwtAbstractScaleDraw::Labels)) {
        painter->save();
        painter->setPen(palette.color(QPalette::Text));  // ignore pen style

        const QList< double >& majorTicks = m_data->scaleDiv.ticks(QwtScaleDiv::MajorTick);

        for (int i = 0; i < majorTicks.count(); i++) {
            const double v = majorTicks[ i ];
            if (m_data->scaleDiv.contains(v))
                drawLabel(painter, v);
        }

        painter->restore();
    }

    if (hasComponent(QwtAbstractScaleDraw::Ticks)) {
        painter->save();

        pen = painter->pen();
        pen.setColor(palette.color(QPalette::WindowText));
        pen.setCapStyle(Qt::FlatCap);

        painter->setPen(pen);

        for (int tickType = QwtScaleDiv::MinorTick; tickType < QwtScaleDiv::NTickTypes; tickType++) {
            const double tickLen = m_data->tickLength[ tickType ];
            if (tickLen <= 0.0)
                continue;

            const QList< double >& ticks = m_data->scaleDiv.ticks(tickType);
            for (int i = 0; i < ticks.count(); i++) {
                const double v = ticks[ i ];
                if (m_data->scaleDiv.contains(v))
                    drawTick(painter, v, tickLen);
            }
        }

        painter->restore();
    }

    if (hasComponent(QwtAbstractScaleDraw::Backbone)) {
        painter->save();

        pen = painter->pen();
        pen.setColor(palette.color(QPalette::WindowText));
        pen.setCapStyle(Qt::FlatCap);

        painter->setPen(pen);

        drawBackbone(painter);

        painter->restore();
    }

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Set the spacing between tick and labels
 * @details The spacing is the distance between ticks and labels. The default spacing is 4 pixels.
 * @param spacing Spacing
 * \sa spacing()
 * \endif
 * \if CHINESE
 * @brief 设置刻度线和标签之间的间距
 * @details 间距是刻度线和标签之间的距离。默认间距为 4 像素。
 * @param spacing 间距
 * \sa spacing()
 * \endif
 */
void QwtAbstractScaleDraw::setSpacing(double spacing)
{
    if (spacing < 0)
        spacing = 0;

    m_data->spacing = spacing;
}

/**
 * \if ENGLISH
 * @brief Get the spacing
 * @details The spacing is the distance between ticks and labels. The default spacing is 4 pixels.
 * @return Spacing
 * \sa setSpacing()
 * \endif
 * \if CHINESE
 * @brief 获取间距
 * @details 间距是刻度线和标签之间的距离。默认间距为 4 像素。
 * @return 间距
 * \sa setSpacing()
 * \endif
 */
double QwtAbstractScaleDraw::spacing() const
{
    return m_data->spacing;
}

/**
 * \if ENGLISH
 * @brief Set a minimum for the extent
 * @details The extent is calculated from the components of the scale draw.
 *          In situations where the labels are changing and the layout depends on the extent
 *          (e.g., scrolling a scale), setting an upper limit as minimum extent will avoid jumps of the layout.
 * @param minExtent Minimum extent
 * \sa extent(), minimumExtent()
 * \endif
 * \if CHINESE
 * @brief 设置范围的最小值
 * @details 范围是从刻度绘制的组件计算的。在标签变化且布局依赖于范围的情况下
 *          （如滚动刻度），设置最小范围的上限将避免布局跳动。
 * @param minExtent 最小范围
 * \sa extent(), minimumExtent()
 * \endif
 */
void QwtAbstractScaleDraw::setMinimumExtent(double minExtent)
{
    if (minExtent < 0.0)
        minExtent = 0.0;

    m_data->minExtent = minExtent;
}

/**
 * \if ENGLISH
 * @brief Get the minimum extent
 * @return Minimum extent
 * \sa extent(), setMinimumExtent()
 * \endif
 * \if CHINESE
 * @brief 获取最小范围
 * @return 最小范围
 * \sa extent(), setMinimumExtent()
 * \endif
 */
double QwtAbstractScaleDraw::minimumExtent() const
{
    return m_data->minExtent;
}

/**
 * \if ENGLISH
 * @brief Set the length of the ticks
 * @param tickType Tick type
 * @param length New length
 * @warning The length is limited to [0..1000]
 * \endif
 * \if CHINESE
 * @brief 设置刻度线长度
 * @param tickType 刻度类型
 * @param length 新的长度
 * @warning 长度限制在 [0..1000]
 * \endif
 */
void QwtAbstractScaleDraw::setTickLength(QwtScaleDiv::TickType tickType, double length)
{
    if (tickType < QwtScaleDiv::MinorTick || tickType > QwtScaleDiv::MajorTick) {
        return;
    }

    if (length < 0.0)
        length = 0.0;

    const double maxTickLen = 1000.0;
    if (length > maxTickLen)
        length = maxTickLen;

    m_data->tickLength[ tickType ] = length;
}

/**
 * \if ENGLISH
 * @brief Get the length of the ticks
 * @return Length of the ticks
 * \sa setTickLength(), maxTickLength()
 * \endif
 * \if CHINESE
 * @brief 获取刻度线长度
 * @return 刻度线长度
 * \sa setTickLength(), maxTickLength()
 * \endif
 */
double QwtAbstractScaleDraw::tickLength(QwtScaleDiv::TickType tickType) const
{
    if (tickType < QwtScaleDiv::MinorTick || tickType > QwtScaleDiv::MajorTick) {
        return 0;
    }

    return m_data->tickLength[ tickType ];
}

/**
 * \if ENGLISH
 * @brief Get the length of the longest tick
 * @return Length of the longest tick
 * @details Useful for layout calculations
 * \sa tickLength(), setTickLength()
 * \endif
 * \if CHINESE
 * @brief 获取最长刻度线的长度
 * @return 最长刻度线的长度
 * @details 对布局计算有用
 * \sa tickLength(), setTickLength()
 * \endif
 */
double QwtAbstractScaleDraw::maxTickLength() const
{
    double length = 0.0;
    for (int i = 0; i < QwtScaleDiv::NTickTypes; i++)
        length = qwtMaxF(length, m_data->tickLength[ i ]);

    return length;
}

/**
 * \if ENGLISH
 * @brief Convert a value into its representing label
 * @details The value is converted to a plain text using QLocale().toString(value).
 *          This method is often overloaded by applications to have individual labels.
 * @param value Value
 * @return Label string
 * \endif
 * \if CHINESE
 * @brief 将值转换为表示标签
 * @details 值通过 QLocale().toString(value) 转换为纯文本。
 *          此方法常被应用程序重载以获得自定义标签。
 * @param value 值
 * @return 标签字符串
 * \endif
 */
QwtText QwtAbstractScaleDraw::label(double value) const
{
    return QLocale().toString(value);
}

/**
 * \if ENGLISH
 * @brief Convert a value into its representing label and cache it
 * @details The conversion between value and label is called very often in the layout and painting code.
 *          Unfortunately the calculation of the label sizes might be slow (really slow for rich text in Qt4),
 *          so it's necessary to cache the labels.
 * @param font Font
 * @param value Value
 * @return Tick label
 * \endif
 * \if CHINESE
 * @brief 将值转换为表示标签并缓存
 * @details 值和标签之间的转换在布局和绘制代码中经常被调用。
 *          不幸的是，标签大小的计算可能很慢（Qt4 中的富文本确实很慢），因此需要缓存标签。
 * @param font 字体
 * @param value 值
 * @return 刻度标签
 * \endif
 */
const QwtText& QwtAbstractScaleDraw::tickLabel(const QFont& font, double value) const
{
    QMap< double, QwtText >::const_iterator it1 = m_data->labelCache.constFind(value);
    if (it1 != m_data->labelCache.constEnd())
        return *it1;

    QwtText lbl = label(value);
    lbl.setRenderFlags(0);
    lbl.setLayoutAttribute(QwtText::MinimumLayout);

    (void)lbl.textSize(font);  // initialize the internal cache

    QMap< double, QwtText >::iterator it2 = m_data->labelCache.insert(value, lbl);
    return *it2;
}

/**
 * \if ENGLISH
 * @brief Invalidate the cache used by tickLabel()
 * @details The cache is invalidated when a new QwtScaleDiv is set.
 *          If the labels need to be changed while the same QwtScaleDiv is set,
 *          invalidateCache() needs to be called manually.
 * \endif
 * \if CHINESE
 * @brief 清除 tickLabel() 使用的缓存
 * @details 当设置新的 QwtScaleDiv 时缓存会被清除。
 *          如果在相同的 QwtScaleDiv 下需要更改标签，需要手动调用 invalidateCache()。
 * \endif
 */
void QwtAbstractScaleDraw::invalidateCache()
{
    m_data->labelCache.clear();
}
