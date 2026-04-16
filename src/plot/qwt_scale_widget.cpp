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

#include "qwt_scale_widget.h"
#include "qwt_painter.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_scale_div.h"
#include "qwt_text.h"
#include "qwt_interval.h"
#include "qwt_scale_engine.h"

#include <qpainter.h>
#include <qevent.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qapplication.h>
#include <qmargins.h>
#include <QDebug>

#ifndef QWTSCALEWIDGET_DEBUG_DRAW
#define QWTSCALEWIDGET_DEBUG_DRAW 0
#endif
class QwtScaleWidget::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtScaleWidget)
public:
    PrivateData(QwtScaleWidget* p) : q_ptr(p)
    {
    }

    ~PrivateData()
    {
    }

    std::unique_ptr< QwtScaleDraw > scaleDraw;

    int borderDist[ 2 ];
    int minBorderDist[ 2 ];
    int scaleLength { 0 };
    int margin { 0 };
    int edgeMargin { 0 };
    int titleOffset { 0 };
    int spacing { 2 };
    QwtText title;

    QwtScaleWidget::LayoutFlags layoutFlags;

    // 内置动作新增的 交互相关成员
    bool isSelected { false };

    double zoomFactor { 1.2 };  ///< 缩放系数
    QwtScaleWidget::BuiltinActionsFlags builtinActions { QwtScaleWidget::ActionAll };
    QColor selectionColor { Qt::blue };     ///< 选中的颜色
    QColor originTextColor { Qt::black };   ///< 记录原始文字颜色
    QColor originScaleColor { Qt::black };  ///< 记录坐标轴颜色

    struct t_colorBar
    {
        bool isEnabled { false };
        int width { 10 };
        QwtInterval interval;
        std::unique_ptr< QwtColorMap > colorMap;
    } colorBar;
};

/**
 * \if ENGLISH
 * @brief Create a scale with the position QwtScaleWidget::Left
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief 创建位置为 QwtScaleWidget::Left 的刻度
 * @param parent 父控件
 * \endif
 */
QwtScaleWidget::QwtScaleWidget(QWidget* parent) : QWidget(parent), QWT_PIMPL_CONSTRUCT
{
    initScale(QwtScaleDraw::LeftScale);
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param align Alignment
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param align 对齐方式
 * @param parent 父控件
 * \endif
 */
QwtScaleWidget::QwtScaleWidget(QwtScaleDraw::Alignment align, QWidget* parent) : QWidget(parent), QWT_PIMPL_CONSTRUCT
{
    initScale(align);
}

//! Destructor
QwtScaleWidget::~QwtScaleWidget()
{
}

//! Initialize the scale
void QwtScaleWidget::initScale(QwtScaleDraw::Alignment align)
{
    QWT_D(d);
    if (align == QwtScaleDraw::RightScale)
        d->layoutFlags |= TitleInverted;

    d->borderDist[ 0 ]    = 0;
    d->borderDist[ 1 ]    = 0;
    d->minBorderDist[ 0 ] = 0;
    d->minBorderDist[ 1 ] = 0;

    d->scaleDraw = qwt_make_unique< QwtScaleDraw >();
    d->scaleDraw->setAlignment(align);
    d->scaleDraw->setLength(10);

    d->scaleDraw->setScaleDiv(QwtLinearScaleEngine().divideScale(0.0, 100.0, 10, 5));

    d->colorBar.colorMap = qwt_make_unique< QwtLinearColorMap >();

    const int flags = Qt::AlignHCenter | Qt::TextExpandTabs | Qt::TextWordWrap;
    d->title.setRenderFlags(flags);
    d->title.setFont(font());

    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    if (d->scaleDraw->orientation() == Qt::Vertical)
        policy.transpose();

    setSizePolicy(policy);

    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

/**
 * \if ENGLISH
 * @brief Check if a mouse position falls on the "pure scale area"
 * @param pos Mouse position (relative to this QWidget's coordinates)
 * @return true if on the scale area, false if on margin, edgeMargin, title, colorBar, etc.
 * \endif
 * \if CHINESE
 * @brief 判断鼠标位置是否落在"纯刻度区域"
 * @param pos 鼠标位置（相对于本 QWidget 的坐标）
 * @return true 落在刻度上，false 落在 margin、edgeMargin、标题、colorBar 等空白处
 * \endif
 */
bool QwtScaleWidget::isOnScale(const QPoint& pos) const
{
    QRect cr = scaleRect();
    return cr.contains(pos);
}

/**
 * \if ENGLISH
 * @brief Toggle a layout flag
 * @param flag Layout flag
 * @param on true/false
 * \sa testLayoutFlag(), LayoutFlag
 * \endif
 * \if CHINESE
 * @brief 切换布局标志
 * @param flag 布局标志
 * @param on true/false
 * \sa testLayoutFlag(), LayoutFlag
 * \endif
 */
void QwtScaleWidget::setLayoutFlag(LayoutFlag flag, bool on)
{
    if (((m_data->layoutFlags & flag) != 0) != on) {
        if (on)
            m_data->layoutFlags |= flag;
        else
            m_data->layoutFlags &= ~flag;

        update();
    }
}

/**
 * \if ENGLISH
 * @brief Test a layout flag
 * @param flag Layout flag
 * @return true/false
 * \sa setLayoutFlag(), LayoutFlag
 * \endif
 * \if CHINESE
 * @brief 测试布局标志
 * @param flag 布局标志
 * @return true/false
 * \sa setLayoutFlag(), LayoutFlag
 * \endif
 */
bool QwtScaleWidget::testLayoutFlag(LayoutFlag flag) const
{
    return (m_data->layoutFlags & flag);
}

/**
 * \if ENGLISH
 * @brief Give title new text contents
 * @param title New title
 * \sa title(), setTitle(const QwtText&)
 * \endif
 * \if CHINESE
 * @brief 设置标题的文本内容
 * @param title 新标题
 * \sa title(), setTitle(const QwtText&)
 * \endif
 */
void QwtScaleWidget::setTitle(const QString& title)
{
    if (m_data->title.text() != title) {
        m_data->title.setText(title);
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Give title new text contents
 * @param title New title
 * \sa title()
 * @warning The title flags are interpreted in direction of the label,
 *          AlignTop, AlignBottom can't be set as the title will always be aligned to the scale.
 * \endif
 * \if CHINESE
 * @brief 设置标题的文本内容
 * @param title 新标题
 * \sa title()
 * @warning 标题标志是按标签方向解释的，AlignTop, AlignBottom 无法设置，
 *          因为标题始终与刻度对齐。
 * \endif
 */
void QwtScaleWidget::setTitle(const QwtText& title)
{
    QwtText t       = title;
    const int flags = title.renderFlags() & ~(Qt::AlignTop | Qt::AlignBottom);
    t.setRenderFlags(flags);

    if (t != m_data->title) {
        m_data->title = t;
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Change the alignment
 * @param alignment New alignment
 * \sa alignment()
 * \endif
 * \if CHINESE
 * @brief 更改对齐方式
 * @param alignment 新的对齐方式
 * \sa alignment()
 * \endif
 */
void QwtScaleWidget::setAlignment(QwtScaleDraw::Alignment alignment)
{
    if (m_data->scaleDraw)
        m_data->scaleDraw->setAlignment(alignment);

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        if (m_data->scaleDraw->orientation() == Qt::Vertical)
            policy.transpose();

        setSizePolicy(policy);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    layoutScale();
}

/**
 * \if ENGLISH
 * @brief Get the alignment
 * @return Alignment of the scale
 * \sa setAlignment()
 * \endif
 * \if CHINESE
 * @brief 获取对齐方式
 * @return 刻度的对齐方式
 * \sa setAlignment()
 * \endif
 */
QwtScaleDraw::Alignment QwtScaleWidget::alignment() const
{
    if (!scaleDraw())
        return QwtScaleDraw::LeftScale;

    return scaleDraw()->alignment();
}

/**
 * \if ENGLISH
 * @brief Specify the margin to the colorBar/base line
 * @param margin Margin
 * \sa margin()
 * \endif
 * \if CHINESE
 * @brief 设置到颜色条/基线的边距
 * @param margin 边距
 * \sa margin()
 * \endif
 */
void QwtScaleWidget::setMargin(int margin)
{
    margin = qMax(0, margin);
    if (margin != m_data->margin) {
        m_data->margin = margin;
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Specify the distance between color bar, scale and title
 * @param spacing Spacing
 * \sa spacing()
 * \endif
 * \if CHINESE
 * @brief 设置颜色条、刻度和标题之间的距离
 * @param spacing 间距
 * \sa spacing()
 * \endif
 */
void QwtScaleWidget::setSpacing(int spacing)
{
    spacing = qMax(0, spacing);
    if (spacing != m_data->spacing) {
        m_data->spacing = spacing;
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Change the alignment for the labels
 * @param alignment Alignment
 * \sa QwtScaleDraw::setLabelAlignment(), setLabelRotation()
 * \endif
 * \if CHINESE
 * @brief 更改标签的对齐方式
 * @param alignment 对齐方式
 * \sa QwtScaleDraw::setLabelAlignment(), setLabelRotation()
 * \endif
 */
void QwtScaleWidget::setLabelAlignment(Qt::Alignment alignment)
{
    m_data->scaleDraw->setLabelAlignment(alignment);
    layoutScale();
}

/**
 * \if ENGLISH
 * @brief Change the rotation for the labels
 * @param rotation Rotation angle in degrees
 * \sa QwtScaleDraw::setLabelRotation(), setLabelAlignment()
 * \endif
 * \if CHINESE
 * @brief 更改标签的旋转角度
 * @param rotation 旋转角度（度）
 * \sa QwtScaleDraw::setLabelRotation(), setLabelAlignment()
 * \endif
 */
void QwtScaleWidget::setLabelRotation(double rotation)
{
    m_data->scaleDraw->setLabelRotation(rotation);
    layoutScale();
}

/**
 * \if ENGLISH
 * @brief Set a scale draw
 * @details scaleDraw has to be created with new and will be deleted in ~QwtScaleWidget()
 *          or the next call of setScaleDraw(). scaleDraw will be initialized with
 *          the attributes of the previous scaleDraw object.
 * @param scaleDraw ScaleDraw object
 * \sa scaleDraw()
 * \endif
 * \if CHINESE
 * @brief 设置刻度绘制对象
 * @details scaleDraw 必须用 new 创建，将在 ~QwtScaleWidget() 或下次调用 setScaleDraw() 时删除。
 *          scaleDraw 将使用前一个 scaleDraw 对象的属性初始化。
 * @param scaleDraw ScaleDraw 对象
 * \sa scaleDraw()
 * \endif
 */
void QwtScaleWidget::setScaleDraw(QwtScaleDraw* scaleDraw)
{
    const QwtScaleDraw* sd = m_data->scaleDraw.get();
    if ((scaleDraw == nullptr) || (scaleDraw == sd)) {
        return;
    }
    if (sd) {
        scaleDraw->setAlignment(sd->alignment());
        scaleDraw->setScaleDiv(sd->scaleDiv());

        QwtTransform* transform = nullptr;
        if (sd->scaleMap().transformation())
            transform = sd->scaleMap().transformation()->copy();

        scaleDraw->setTransformation(transform);
    }

    m_data->scaleDraw.reset(scaleDraw);

    layoutScale();
}

/**
 * \if ENGLISH
 * @brief Get the scale draw (const version)
 * @return scaleDraw of this scale
 * \sa setScaleDraw(), QwtScaleDraw::setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 获取刻度绘制对象（常量版本）
 * @return 此刻度的 scaleDraw
 * \sa setScaleDraw(), QwtScaleDraw::setScaleDraw()
 * \endif
 */
const QwtScaleDraw* QwtScaleWidget::scaleDraw() const
{
    return m_data->scaleDraw.get();
}

/**
 * \if ENGLISH
 * @brief Get the scale draw
 * @return scaleDraw of this scale
 * \sa QwtScaleDraw::setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 获取刻度绘制对象
 * @return 此刻度的 scaleDraw
 * \sa QwtScaleDraw::setScaleDraw()
 * \endif
 */
QwtScaleDraw* QwtScaleWidget::scaleDraw()
{
    return m_data->scaleDraw.get();
}

/**
 * \if ENGLISH
 * @brief Get the title
 * @return Title text
 * \sa setTitle()
 * \endif
 * \if CHINESE
 * @brief 获取标题
 * @return 标题文本
 * \sa setTitle()
 * \endif
 */
QwtText QwtScaleWidget::title() const
{
    return m_data->title;
}

/**
 * \if ENGLISH
 * @brief Get the margin
 * @return Margin value
 * \sa setMargin()
 * \endif
 * \if CHINESE
 * @brief 获取边距
 * @return 边距值
 * \sa setMargin()
 * \endif
 */
int QwtScaleWidget::margin() const
{
    return m_data->margin;
}

/**
 * \if ENGLISH
 * @brief Get the distance between scale and title
 * @return Distance between scale and title
 * \sa setSpacing()
 * \endif
 * \if CHINESE
 * @brief 获取刻度和标题之间的距离
 * @return 刻度和标题之间的距离
 * \sa setSpacing()
 * \endif
 */
int QwtScaleWidget::spacing() const
{
    return m_data->spacing;
}

/**
 * \if ENGLISH
 * @brief Set the edge margin (offset between axis and plot canvas edge)
 * @details edgeMargin and margin are opposite: margin is the offset from the plot,
 *          edgeMargin is the offset from the plot canvas edge.
 * @param offset Edge margin offset
 * \endif
 * \if CHINESE
 * @brief 设置坐标轴和绘图边缘的空白偏移距离
 * @details edgeMargin和margin刚好相反，margin是和绘图的偏移，edgeMargin是和绘图边缘的偏移
 * @param offset 边缘偏移距离
 * \endif
 */
void QwtScaleWidget::setEdgeMargin(int offset)
{
    offset = qMax(0, offset);
    if (offset != m_data->edgeMargin) {
        m_data->edgeMargin = offset;
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Get the edge margin
 * @details The edge margin is the offset between the axis and the plot canvas edge.
 * @return Edge margin value
 * \endif
 * \if CHINESE
 * @brief 获取边缘距离
 * @details 边缘距离是坐标轴和绘图边缘之间的偏移距离。
 * @return 边缘距离值
 * \endif
 */
int QwtScaleWidget::edgeMargin() const
{
    return m_data->edgeMargin;
}

/**
 * \if ENGLISH
 * @brief Set the font color of the coordinate axis
 * @param c Text color
 * \endif
 * \if CHINESE
 * @brief 设置坐标轴的字体颜色
 * @param c 字体颜色
 * \endif
 */
void QwtScaleWidget::setTextColor(const QColor& c)
{
    // 绘制标题时是通过QPalette::Text获取颜色
    // painter->setPen(palette().color(QPalette::Text));
    QPalette p = palette();
    p.setColor(QPalette::Text, c);
    setPalette(p);
}

/**
 * \if ENGLISH
 * @brief Get the font color of the coordinate axis
 * @return Text color
 * \endif
 * \if CHINESE
 * @brief 获取坐标轴的字体颜色
 * @return 字体颜色
 * \endif
 */
QColor QwtScaleWidget::textColor() const
{
    return palette().color(QPalette::Text);
}

/**
 * \if ENGLISH
 * @brief Set the color of the coordinate axis
 * @param c Scale color
 * \endif
 * \if CHINESE
 * @brief 设置坐标轴的颜色
 * @param c 坐标轴颜色
 * \endif
 */
void QwtScaleWidget::setScaleColor(const QColor& c)
{
    // QPalette::WindowText
    QPalette p = palette();
    p.setColor(QPalette::WindowText, c);
    setPalette(p);
}

/**
 * \if ENGLISH
 * @brief Get the color of the coordinate axis
 * @return Scale color
 * \endif
 * \if CHINESE
 * @brief 获取坐标轴的颜色
 * @return 坐标轴颜色
 * \endif
 */
QColor QwtScaleWidget::scaleColor() const
{
    return palette().color(QPalette::WindowText);
}
/*!
   \brief paintEvent
 */
void QwtScaleWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

#if QWTSCALEWIDGET_DEBUG_DRAW
    painter.setPen(QPen(Qt::blue, 0.5));
    painter.drawRect(rect().adjusted(1, 1, -1, -1));
#endif
    draw(&painter);
}

/*!
   \brief draw the scale
 */
void QwtScaleWidget::draw(QPainter* painter) const
{

    m_data->scaleDraw->draw(painter, palette());

    if (m_data->colorBar.isEnabled && m_data->colorBar.width > 0 && m_data->colorBar.interval.isValid()) {
        drawColorBar(painter, colorBarRect(contentsRect()));
    }

    QRect r = contentsRect();
    if (m_data->scaleDraw->orientation() == Qt::Horizontal) {
        r.setLeft(r.left() + m_data->borderDist[ 0 ]);
        r.setWidth(r.width() - m_data->borderDist[ 1 ]);
    } else {
        r.setTop(r.top() + m_data->borderDist[ 0 ]);
        r.setHeight(r.height() - m_data->borderDist[ 1 ]);
    }

    if (!m_data->title.isEmpty())
        drawTitle(painter, m_data->scaleDraw->alignment(), r);
}

/**
 * \if ENGLISH
 * @brief Calculate the rectangle for the color bar
 * @param rect Bounding rectangle for all components of the scale
 * @return Rectangle for the color bar
 * \endif
 * \if CHINESE
 * @brief 计算颜色条的矩形区域
 * @param rect 刻度所有组件的边界矩形
 * @return 颜色条的矩形区域
 * \endif
 */
QRectF QwtScaleWidget::colorBarRect(const QRectF& rect) const
{
    QRectF cr = rect;

    if (m_data->scaleDraw->orientation() == Qt::Horizontal) {
        cr.setLeft(cr.left() + m_data->borderDist[ 0 ]);
        cr.setWidth(cr.width() - m_data->borderDist[ 1 ] + 1);
    } else {
        cr.setTop(cr.top() + m_data->borderDist[ 0 ]);
        cr.setHeight(cr.height() - m_data->borderDist[ 1 ] + 1);
    }

    switch (m_data->scaleDraw->alignment()) {
    case QwtScaleDraw::LeftScale: {
        cr.setLeft(cr.right() - m_data->margin - m_data->colorBar.width);
        cr.setWidth(m_data->colorBar.width);
        break;
    }

    case QwtScaleDraw::RightScale: {
        cr.setLeft(cr.left() + m_data->margin);
        cr.setWidth(m_data->colorBar.width);
        break;
    }

    case QwtScaleDraw::BottomScale: {
        cr.setTop(cr.top() + m_data->margin);
        cr.setHeight(m_data->colorBar.width);
        break;
    }

    case QwtScaleDraw::TopScale: {
        cr.setTop(cr.bottom() - m_data->margin - m_data->colorBar.width);
        cr.setHeight(m_data->colorBar.width);
        break;
    }
    }

    return cr;
}

/**
 * \if ENGLISH
 * @brief Get the rectangle for drawing the scale (excluding colorBar, margin, edgeMargin, borderDist)
 * @return Rectangle used for drawing the scale
 * \endif
 * \if CHINESE
 * @brief 获取用于绘制刻度的矩形区域（不包含 colorBar、margin、edgeMargin、borderDistHint）
 * @return 用于绘制刻度的矩形区域
 * \endif
 */
QRect QwtScaleWidget::scaleRect() const
{
    if (!m_data->scaleDraw)  // 无刻度对象
        return QRect();

    /* 1. 内容区，去掉外围的 contentsMargins() */
    QRect cr = contentsRect();

    /* 2. 再去掉用户设定的 borderDist（刻度两端留空） */
    int bd0, bd1;
    getBorderDistHint(bd0, bd1);  // 最小必需距离
    bd0 = qMax(bd0, m_data->borderDist[ 0 ]);
    bd1 = qMax(bd1, m_data->borderDist[ 1 ]);

    if (m_data->scaleDraw->orientation() == Qt::Vertical) {
        cr.adjust(0, bd0, 0, -bd1);  // 上下两端
    } else {
        cr.adjust(bd0, 0, -bd1, 0);  // 左右两端
    }

    /* 3. 再去掉 colorBar 占用的区域（如果启用） */
    if (m_data->colorBar.isEnabled && m_data->colorBar.width > 0 && m_data->colorBar.interval.isValid()) {
        const int cw = m_data->colorBar.width + m_data->spacing;
        switch (m_data->scaleDraw->alignment()) {
        case QwtScaleDraw::LeftScale:
            cr.adjust(0, 0, -cw, 0);  // 右边减掉
            break;
        case QwtScaleDraw::RightScale:
            cr.adjust(cw, 0, 0, 0);  // 左边减掉
            break;
        case QwtScaleDraw::TopScale:
            cr.adjust(0, cw, 0, 0);  // 下边减掉
            break;
        case QwtScaleDraw::BottomScale:
            cr.adjust(0, 0, 0, -cw);  // 上边减掉
            break;
        }
    }

    /* 4. 去掉margin和edgeMargin */
    switch (m_data->scaleDraw->alignment()) {
    case QwtScaleDraw::LeftScale:
        cr.adjust(m_data->edgeMargin, 0, -(m_data->margin), 0);  // 右边减掉margin,左边减掉edgeMargin
        break;
    case QwtScaleDraw::RightScale:
        cr.adjust(m_data->margin, 0, -(m_data->edgeMargin), 0);  // 左边减掉margin,右边减掉edgeMargin
        break;
    case QwtScaleDraw::TopScale:
        cr.adjust(0, m_data->edgeMargin, 0, -(m_data->margin));  // 下边减掉margin,上边减掉edgeMargin
        break;
    case QwtScaleDraw::BottomScale:
        cr.adjust(0, m_data->margin, 0, -(m_data->edgeMargin));  // 上边减掉margin,下边减掉edgeMargin
        break;
    }

    return cr;
}

/*!
   Change Event handler
   \param event Change event

   Invalidates internal caches if necessary
 */
void QwtScaleWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LocaleChange) {
        m_data->scaleDraw->invalidateCache();
    }

    QWidget::changeEvent(event);
}

/*!
   Event handler for resize events
   \param event Resize event
 */
void QwtScaleWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    layoutScale(false);
}

/**
 * \if ENGLISH
 * @brief Recalculate the scale's geometry and layout based on the current geometry and fonts
 * @param update_geometry Notify the layout system and call update to redraw the scale
 * \endif
 * \if CHINESE
 * @brief 根据当前几何形状和字体重新计算刻度的几何形状和布局
 * @param update_geometry 通知布局系统并调用 update 以重绘刻度
 * \endif
 */
void QwtScaleWidget::layoutScale(bool update_geometry)
{
    int bd0, bd1;
    getBorderDistHint(bd0, bd1);
    if (m_data->borderDist[ 0 ] > bd0)
        bd0 = m_data->borderDist[ 0 ];
    if (m_data->borderDist[ 1 ] > bd1)
        bd1 = m_data->borderDist[ 1 ];

    int colorBarWidth = 0;
    if (m_data->colorBar.isEnabled && m_data->colorBar.interval.isValid())
        colorBarWidth = m_data->colorBar.width + m_data->spacing;

    const QRectF r = contentsRect();
    double x, y, length;
    // 刻度偏移和edgeMargin无关
    if (m_data->scaleDraw->orientation() == Qt::Vertical) {
        y      = r.top() + bd0;
        length = r.height() - (bd0 + bd1);

        if (m_data->scaleDraw->alignment() == QwtScaleDraw::LeftScale) {  // 左对齐的刻度，坐标轴一般是右坐标轴
            x = r.right() - 1.0 - m_data->margin - colorBarWidth;
        } else {
            x = r.left() + m_data->margin + colorBarWidth;
        }
    } else {
        x      = r.left() + bd0;
        length = r.width() - (bd0 + bd1);

        if (m_data->scaleDraw->alignment() == QwtScaleDraw::BottomScale) {
            y = r.top() + m_data->margin + colorBarWidth;
        } else {
            y = r.bottom() - 1.0 - m_data->margin - colorBarWidth;
        }
    }

    m_data->scaleDraw->move(x, y);
    m_data->scaleDraw->setLength(length);

    const int extent = qwtCeil(m_data->scaleDraw->extent(font()));
    // titleoffset和edgeMargin无关
    m_data->titleOffset = m_data->margin + m_data->spacing + colorBarWidth + extent;

    if (update_geometry) {
        updateGeometry();

#if 1
        /*
            for some reason updateGeometry does not send a LayoutRequest event
            when the parent is not visible and has no layout
         */

        if (QWidget* w = parentWidget()) {
            if (!w->isVisible() && w->layout() == nullptr) {
                if (w->testAttribute(Qt::WA_WState_Polished))
                    QApplication::postEvent(w, new QEvent(QEvent::LayoutRequest));
            }
        }
#endif

        update();
    }
}

/**
 * \if ENGLISH
 * @brief Get the axis ID for this scale widget
 * @note This function converts based on QwtScaleDraw alignment. If alignment is not set, returns QwtAxis::AxisPositions.
 * @return Axis ID
 * \endif
 * \if CHINESE
 * @brief 获取此轴窗口对应的 axisID
 * @note 此函数基于 QwtScaleDraw 的对齐方式来转换，如果 QwtScaleDraw 的对齐方式没设置，那么会返回 QwtAxis::AxisPositions
 * @return 轴 ID
 * \endif
 */
QwtAxisId QwtScaleWidget::axisID() const
{
    const QwtScaleDraw* sd = scaleDraw();
    switch (sd->alignment()) {
    case QwtScaleDraw::BottomScale:
        return QwtAxis::XBottom;
    case QwtScaleDraw::TopScale:
        return QwtAxis::XTop;
    case QwtScaleDraw::LeftScale:
        return QwtAxis::YLeft;
    case QwtScaleDraw::RightScale:
        return QwtAxis::YRight;
    default:
        break;
    }
    return QwtAxis::AxisPositions;
}

/**
 * \if ENGLISH
 * @brief Check if this is an X axis
 * @return true if this is an X axis
 * \endif
 * \if CHINESE
 * @brief 是否是 X 坐标轴
 * @return 如果是 X 轴返回 true
 * \endif
 */
bool QwtScaleWidget::isXAxis() const
{
    return QwtAxis::isXAxis(axisID());
}

/**
 * \if ENGLISH
 * @brief Check if this is a Y axis
 * @return true if this is a Y axis
 * \endif
 * \if CHINESE
 * @brief 是否是 Y 坐标轴
 * @return 如果是 Y 轴返回 true
 * \endif
 */
bool QwtScaleWidget::isYAxis() const
{
    return QwtAxis::isYAxis(axisID());
}

/**
 * \if ENGLISH
 * @brief Set the built-in actions
 * @param actions Built-in action flags
 * \sa BuiltinActions
 * \endif
 * \if CHINESE
 * @brief 设置内置的动作
 * @param actions 内置动作标志
 * \sa BuiltinActions
 * \endif
 */
void QwtScaleWidget::setBuildinActions(BuiltinActionsFlags acts)
{
    if (m_data->builtinActions != acts) {
        m_data->builtinActions = acts;
    }
}

/**
 * \if ENGLISH
 * @brief Get the built-in actions flags
 * @return Built-in action flags
 * \endif
 * \if CHINESE
 * @brief 获取内置的动作标志
 * @return 内置动作标志
 * \endif
 */
QwtScaleWidget::BuiltinActionsFlags QwtScaleWidget::buildinActions() const
{
    return m_data->builtinActions;
}

/**
 * \if ENGLISH
 * @brief Test if a built-in action is active
 * @param ba Built-in action to test
 * @return true if the action is active
 * \endif
 * \if CHINESE
 * @brief 检测内置动作是否激活
 * @param ba 要检测的内置动作
 * @return 如果激活返回 true
 * \endif
 */
bool QwtScaleWidget::testBuildinActions(QwtScaleWidget::BuiltinActions ba) const
{
    return m_data->builtinActions.testFlag(ba);
}

/**
 * \if ENGLISH
 * @brief Set the selected state
 * @details This function triggers the selectionChanged signal.
 *          Setting the same state repeatedly will not trigger the signal again.
 * @param selected true to select, false to deselect
 * \endif
 * \if CHINESE
 * @brief 设置当前轴被选中
 * @details 此函数会触发信号 selectionChanged。如果重复设置同一个状态不会重复触发信号。
 * @param selected true 为选中，false 为取消选中
 * \endif
 */
void QwtScaleWidget::setSelected(bool selected)
{
    QWT_D(d);
    if (d->isSelected != selected) {
        if (!(d->isSelected)) {
            d->originTextColor  = textColor();
            d->originScaleColor = scaleColor();
            setTextColor(d->selectionColor);
            setScaleColor(d->selectionColor);
        } else {
            setTextColor(d->originTextColor);
            setScaleColor(d->originScaleColor);
        }
        d->scaleDraw->setSelected(selected);
        d->isSelected = selected;
        update();
        Q_EMIT selectionChanged(selected);
    }
}

/**
 * \if ENGLISH
 * @brief Check if the current axis is selected
 * @return true if selected, false otherwise
 * \endif
 * \if CHINESE
 * @brief 当前轴是否被选中
 * @return 如果被选中返回 true，否则返回 false
 * \endif
 */
bool QwtScaleWidget::isSelected() const
{
    return m_data->isSelected;
}

/**
 * \if ENGLISH
 * @brief Set the selection color
 * @param color Selection color
 * \endif
 * \if CHINESE
 * @brief 设置选中的颜色
 * @param color 选中的颜色
 * \endif
 */
void QwtScaleWidget::setSelectionColor(const QColor& color)
{
    if (m_data->selectionColor != color) {
        m_data->selectionColor = color;
        if (m_data->isSelected) {
            update();
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the selection color
 * @return Selection color
 * \endif
 * \if CHINESE
 * @brief 获取选中的颜色
 * @return 选中的颜色
 * \endif
 */
QColor QwtScaleWidget::selectionColor() const
{
    return m_data->selectionColor;
}

/**
 * \if ENGLISH
 * @brief Set the zoom factor (default 1.2)
 * @param factor Zoom factor (range 0.1 to 10.0)
 * \endif
 * \if CHINESE
 * @brief 设置缩放因子（默认 1.2）
 * @param factor 缩放因子（范围 0.1 到 10.0）
 * \endif
 */
void QwtScaleWidget::setZoomFactor(double factor)
{
    m_data->zoomFactor = qMax(0.1, qMin(10.0, factor));
}

/**
 * \if ENGLISH
 * @brief Get the zoom factor
 * @return Zoom factor
 * \endif
 * \if CHINESE
 * @brief 获取缩放因子
 * @return 缩放因子
 * \endif
 */
double QwtScaleWidget::zoomFactor() const
{
    return m_data->zoomFactor;
}

/**
 * \if ENGLISH
 * @brief Set the pen width offset for the axis when it is in selected state
 * @details When an axis (e.g., X-axis or Y-axis) is selected by the user, the pen width used
 *          for drawing will be increased by this offset value, achieving a visual highlighting effect.
 * @param offset The additional width value to be added when selected (unit: pixels).
 *               This value should be non-negative. If it is 0, the line width in selected state
 *               will be the same as in normal state.
 * \sa QwtScaleWidget::selectedPenWidthOffset, QwtAbstractScaleDraw::setSelectedPenWidthOffset
 * \endif
 * \if CHINESE
 * @brief 设置坐标轴在选中状态下的画笔宽度附加值
 * @details 当坐标轴（如 X 轴或 Y 轴）被用户选中时，绘制的画笔宽度会在原始宽度的基础上
 *          增加这个附加值，实现视觉上的突出显示效果。
 * @param offset 选中时增加的宽度值（单位：像素）。该值应为非负数。
 *               如果为 0，则选中状态下的线宽与普通状态相同。
 * \sa QwtScaleWidget::selectedPenWidthOffset, QwtAbstractScaleDraw::setSelectedPenWidthOffset
 * \endif
 */
void QwtScaleWidget::setSelectedPenWidthOffset(qreal offset)
{
    m_data->scaleDraw->setSelectedPenWidthOffset(offset);
}

/**
 * \if ENGLISH
 * @brief Get the current pen width offset for the axis when it is in selected state
 * @return The current width offset value
 * \sa QwtScaleWidget::setSelectedPenWidthOffset, QwtAbstractScaleDraw::selectedPenWidthOffset
 * \endif
 * \if CHINESE
 * @brief 获取当前坐标轴在选中状态下的画笔宽度附加值
 * @return 当前的宽度附加值
 * \sa QwtScaleWidget::setSelectedPenWidthOffset, QwtAbstractScaleDraw::selectedPenWidthOffset
 * \endif
 */
qreal QwtScaleWidget::selectedPenWidthOffset() const
{
    return m_data->scaleDraw->selectedPenWidthOffset();
}

/**
 * \if ENGLISH
 * @brief Draw the color bar of the scale widget
 * @param painter Painter
 * @param rect Bounding rectangle for the color bar
 * \sa setColorBarEnabled()
 * \endif
 * \if CHINESE
 * @brief 绘制刻度控件的颜色条
 * @param painter 绘制器
 * @param rect 颜色条的边界矩形
 * \sa setColorBarEnabled()
 * \endif
 */
void QwtScaleWidget::drawColorBar(QPainter* painter, const QRectF& rect) const
{
    QWT_DC(d);
    if (!d->colorBar.interval.isValid()) {
        return;
    }

    QwtPainter::drawColorBar(painter,
                             *(d->colorBar.colorMap),
                             d->colorBar.interval.normalized(),
                             d->scaleDraw->scaleMap(),
                             d->scaleDraw->orientation(),
                             rect);
}

/**
 * \if ENGLISH
 * @brief Rotate and paint a title according to its position into a given rectangle
 * @param painter Painter
 * @param align Alignment
 * @param rect Bounding rectangle
 * \endif
 * \if CHINESE
 * @brief 根据标题的位置旋转并绘制标题到指定矩形区域
 * @param painter 绘制器
 * @param align 对齐方式
 * @param rect 边界矩形
 * \endif
 */
void QwtScaleWidget::drawTitle(QPainter* painter, QwtScaleDraw::Alignment align, const QRectF& rect) const
{
    QRectF r = rect;
    double angle;
    int flags = m_data->title.renderFlags() & ~(Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter);

    switch (align) {
    case QwtScaleDraw::LeftScale:
        angle = -90.0;
        flags |= Qt::AlignTop;
        r.setRect(r.left() + m_data->edgeMargin, r.bottom(), r.height(), r.width() - m_data->titleOffset);
        break;

    case QwtScaleDraw::RightScale:
        angle = -90.0;
        flags |= Qt::AlignTop;
        r.setRect(r.left() + m_data->titleOffset, r.bottom(), r.height(), r.width() - m_data->titleOffset - m_data->edgeMargin);
        break;

    case QwtScaleDraw::BottomScale:
        angle = 0.0;
        flags |= Qt::AlignBottom;
        r.setTop(r.top() + m_data->titleOffset);
        r.setBottom(r.bottom() - m_data->edgeMargin);
        break;

    case QwtScaleDraw::TopScale:
    default:
        angle = 0.0;
        flags |= Qt::AlignTop;
        r.setBottom(r.bottom() - m_data->titleOffset);
        r.setTop(r.top() + m_data->edgeMargin);
        break;
    }

    if (m_data->layoutFlags & TitleInverted) {
        if (align == QwtScaleDraw::LeftScale || align == QwtScaleDraw::RightScale) {
            angle = -angle;
            r.setRect(r.x() + r.height(), r.y() - r.width(), r.width(), r.height());
        }
    }

    painter->save();
    painter->setFont(font());
    painter->setPen(palette().color(QPalette::Text));

    painter->translate(r.x(), r.y());
    if (angle != 0.0)
        painter->rotate(angle);

    QwtText title = m_data->title;
    title.setRenderFlags(flags);
    title.draw(painter, QRectF(0.0, 0.0, r.width(), r.height()));

    painter->restore();
}

/*!
   \brief Notify a change of the scale

   This virtual function can be overloaded by derived
   classes. The default implementation updates the geometry
   and repaints the widget.
 */

void QwtScaleWidget::scaleChange()
{
    layoutScale();
}

/**
 * \if ENGLISH
 * @brief Get a size hint
 * @return Size hint
 * \endif
 * \if CHINESE
 * @brief 获取大小提示
 * @return 大小提示
 * \endif
 */
QSize QwtScaleWidget::sizeHint() const
{
    return minimumSizeHint();
}

/**
 * \if ENGLISH
 * @brief Get a minimum size hint
 * @return Minimum size hint
 * \endif
 * \if CHINESE
 * @brief 获取最小大小提示
 * @return 最小大小提示
 * \endif
 */
QSize QwtScaleWidget::minimumSizeHint() const
{
    const Qt::Orientation o = m_data->scaleDraw->orientation();

    // Border Distance cannot be less than the scale borderDistHint
    // Note, the borderDistHint is already included in minHeight/minWidth
    int length = 0;
    int mbd1, mbd2;
    getBorderDistHint(mbd1, mbd2);
    length += qMax(0, m_data->borderDist[ 0 ] - mbd1);
    length += qMax(0, m_data->borderDist[ 1 ] - mbd2);
    length += m_data->scaleDraw->minLength(font());

    int dim = dimForLength(length, font());
    if (length < dim) {
        // compensate for long titles
        length = dim;
        dim    = dimForLength(length, font());
    }

    QSize size(length + 2, dim);
    if (o == Qt::Vertical)
        size.transpose();

    const QMargins m = contentsMargins();
    return size + QSize(m.left() + m.right(), m.top() + m.bottom());
}

/**
 * \if ENGLISH
 * @brief Find the height of the title for a given width
 * @param width Width
 * @return Height
 * \endif
 * \if CHINESE
 * @brief 查找给定宽度下标题的高度
 * @param width 宽度
 * @return 高度
 * \endif
 */
int QwtScaleWidget::titleHeightForWidth(int width) const
{
    return qwtCeil(m_data->title.heightForWidth(width, font()));
}

/**
 * \if ENGLISH
 * @brief Find the minimum dimension for a given length
 * @details dim is the height, length the width seen in direction of the title.
 * @param length Width for horizontal, height for vertical scales
 * @param scaleFont Font of the scale
 * @return Height for horizontal, width for vertical scales
 * \endif
 * \if CHINESE
 * @brief 查找给定长度的最小尺寸
 * @details dim 是高度，length 是标题方向上的宽度。
 * @param length 水平刻度为宽度，垂直刻度为高度
 * @param scaleFont 刻度的字体
 * @return 水平刻度返回高度，垂直刻度返回宽度
 * \endif
 */
int QwtScaleWidget::dimForLength(int length, const QFont& scaleFont) const
{
    const int extent = qwtCeil(m_data->scaleDraw->extent(scaleFont));

    int dim = m_data->margin + extent + 1 + m_data->edgeMargin;

    if (!m_data->title.isEmpty())
        dim += titleHeightForWidth(length) + m_data->spacing;

    if (m_data->colorBar.isEnabled && m_data->colorBar.interval.isValid())
        dim += m_data->colorBar.width + m_data->spacing;

    return dim;
}

/**
 * \if ENGLISH
 * @brief Specify distances of the scale's endpoints from the widget's borders
 * @details The actual borders will never be less than minimum border distance.
 * @param dist1 Left or top distance
 * @param dist2 Right or bottom distance
 * \sa startBorderDist(), endBorderDist()
 * \endif
 * \if CHINESE
 * @brief 设置刻度两端与控件边框的距离
 * @details 实际边框距离不会小于最小边框距离。
 * @param dist1 左侧或顶部距离
 * @param dist2 右侧或底部距离
 * \sa startBorderDist(), endBorderDist()
 * \endif
 */
void QwtScaleWidget::setBorderDist(int dist1, int dist2)
{
    if (dist1 != m_data->borderDist[ 0 ] || dist2 != m_data->borderDist[ 1 ]) {
        m_data->borderDist[ 0 ] = dist1;
        m_data->borderDist[ 1 ] = dist2;
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Get the start border distance
 * @return Start border distance
 * \sa setBorderDist()
 * \endif
 * \if CHINESE
 * @brief 获取起始边框距离
 * @return 起始边框距离
 * \sa setBorderDist()
 * \endif
 */
int QwtScaleWidget::startBorderDist() const
{
    return m_data->borderDist[ 0 ];
}

/**
 * \if ENGLISH
 * @brief Get the end border distance
 * @return End border distance
 * \sa setBorderDist()
 * \endif
 * \if CHINESE
 * @brief 获取末端边框距离
 * @return 末端边框距离
 * \sa setBorderDist()
 * \endif
 */
int QwtScaleWidget::endBorderDist() const
{
    return m_data->borderDist[ 1 ];
}

/**
 * \if ENGLISH
 * @brief Calculate a hint for the border distances
 * @details This member function calculates the distance of the scale's endpoints
 *          from the widget borders which is required for the mark labels to fit
 *          into the widget. The maximum of this distance and the minimum border
 *          distance is returned.
 * @param[out] start Border width at the beginning of the scale
 * @param[out] end Border width at the end of the scale
 * @warning The minimum border distance depends on the font.
 * \sa setMinBorderDist(), getMinBorderDist(), setBorderDist()
 * \endif
 * \if CHINESE
 * @brief 计算边框距离的"建议值"
 * @details 本函数根据刻度标记文字的大小，计算刻度两端与控件边框之间所需的距离，
 *          以确保文字完整显示。最终返回该距离与最小边框距离中的较大值。
 * @param[out] start 刻度起始端与边框的宽度
 * @param[out] end 刻度末端与边框的宽度
 * @warning 最小边框距离与当前字体有关。
 * \sa setMinBorderDist(), getMinBorderDist(), setBorderDist()
 * \endif
 */
void QwtScaleWidget::getBorderDistHint(int& start, int& end) const
{
    m_data->scaleDraw->getBorderDistHint(font(), start, end);

    if (start < m_data->minBorderDist[ 0 ])
        start = m_data->minBorderDist[ 0 ];

    if (end < m_data->minBorderDist[ 1 ])
        end = m_data->minBorderDist[ 1 ];
}

/**
 * \if ENGLISH
 * @brief Set a minimum value for the distances of the scale's endpoints from the widget borders
 * @details This is useful to avoid that the scales are "jumping" when the tick labels
 *          or their positions change often.
 * @param start Minimum for the start border
 * @param end Minimum for the end border
 * \sa getMinBorderDist(), getBorderDistHint(), startMinBorderDist(), endMinBorderDist()
 * \endif
 * \if CHINESE
 * @brief 设置刻度两端与控件边框的最小距离
 * @details 当刻度标签或其位置频繁变化时，可避免刻度出现"跳动"现象。
 * @param start 起始端最小边距
 * @param end 末端最小边距
 * \sa getMinBorderDist(), getBorderDistHint(), startMinBorderDist(), endMinBorderDist()
 * \endif
 */
void QwtScaleWidget::setMinBorderDist(int start, int end)
{
    m_data->minBorderDist[ 0 ] = start;
    m_data->minBorderDist[ 1 ] = end;
}

/**
 * \if ENGLISH
 * @brief Get the minimum value for the distances of the scale's endpoints from the widget borders (left or top)
 * @return Start minimum border distance
 * \sa getMinBorderDist(), getBorderDistHint()
 * \endif
 * \if CHINESE
 * @brief 获取刻度两端与控件边框的最小距离（左侧或顶部）
 * @return 起始端最小边框距离
 * \sa getMinBorderDist(), getBorderDistHint()
 * \endif
 */
int QwtScaleWidget::startMinBorderDist() const
{
    return m_data->minBorderDist[ 0 ];
}

/**
 * \if ENGLISH
 * @brief Get the minimum value for the distances of the scale's endpoints from the widget borders (right or bottom)
 * @return End minimum border distance
 * \sa getMinBorderDist(), getBorderDistHint()
 * \endif
 * \if CHINESE
 * @brief 获取刻度两端与控件边框的最小距离（右侧或底部）
 * @return 末端最小边框距离
 * \sa getMinBorderDist(), getBorderDistHint()
 * \endif
 */
int QwtScaleWidget::endMinBorderDist() const
{
    return m_data->minBorderDist[ 1 ];
}

/**
 * \if ENGLISH
 * @brief Get the minimum value for the distances of the scale's endpoints from the widget borders
 * @param[out] start Border width at the beginning of the scale
 * @param[out] end Border width at the end of the scale
 * \sa setMinBorderDist(), getBorderDistHint()
 * \endif
 * \if CHINESE
 * @brief 获取刻度两端与控件边框的最小距离
 * @param[out] start 刻度起始端的边框宽度
 * @param[out] end 刻度末端的边框宽度
 * \sa setMinBorderDist(), getBorderDistHint()
 * \endif
 */
void QwtScaleWidget::getMinBorderDist(int& start, int& end) const
{
    start = m_data->minBorderDist[ 0 ];
    end   = m_data->minBorderDist[ 1 ];
}

/**
 * \if ENGLISH
 * @brief Assign a scale division
 * @details The scale division determines where to set the tick marks.
 * @param scaleDiv Scale Division
 * \sa For more information about scale divisions, see QwtScaleDiv.
 * \endif
 * \if CHINESE
 * @brief 设置刻度划分
 * @details 刻度划分决定了刻度标记的位置。
 * @param scaleDiv 刻度划分对象
 * \sa 更多关于刻度划分的信息，参见 QwtScaleDiv。
 * \endif
 */
void QwtScaleWidget::setScaleDiv(const QwtScaleDiv& scaleDiv)
{
    QWT_D(d);
    if (d->scaleDraw->scaleDiv() != scaleDiv) {
        d->scaleDraw->setScaleDiv(scaleDiv);
        layoutScale();

        Q_EMIT scaleDivChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set the transformation
 * @param transformation Transformation
 * \sa QwtAbstractScaleDraw::scaleDraw(), QwtScaleMap
 * \endif
 * \if CHINESE
 * @brief 设置变换
 * @param transformation 变换对象
 * \sa QwtAbstractScaleDraw::scaleDraw(), QwtScaleMap
 * \endif
 */
void QwtScaleWidget::setTransformation(QwtTransform* transformation)
{
    m_data->scaleDraw->setTransformation(transformation);
    layoutScale();
}

/**
 * \if ENGLISH
 * @brief Enable/disable a color bar associated to the scale
 * @param on true to enable, false to disable
 * \sa isColorBarEnabled(), setColorBarWidth()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用与刻度关联的颜色条
 * @param on true 启用，false 禁用
 * \sa isColorBarEnabled(), setColorBarWidth()
 * \endif
 */
void QwtScaleWidget::setColorBarEnabled(bool on)
{
    if (on != m_data->colorBar.isEnabled) {
        m_data->colorBar.isEnabled = on;
        layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Check if color bar is enabled
 * @return true when the color bar is enabled
 * \sa setColorBarEnabled(), setColorBarWidth()
 * \endif
 * \if CHINESE
 * @brief 检查颜色条是否已启用
 * @return 如果颜色条已启用返回 true
 * \sa setColorBarEnabled(), setColorBarWidth()
 * \endif
 */
bool QwtScaleWidget::isColorBarEnabled() const
{
    return m_data->colorBar.isEnabled;
}

/**
 * \if ENGLISH
 * @brief Set the width of the color bar
 * @param width Width
 * \sa colorBarWidth(), setColorBarEnabled()
 * \endif
 * \if CHINESE
 * @brief 设置颜色条的宽度
 * @param width 宽度
 * \sa colorBarWidth(), setColorBarEnabled()
 * \endif
 */
void QwtScaleWidget::setColorBarWidth(int width)
{
    if (width != m_data->colorBar.width) {
        m_data->colorBar.width = width;
        if (isColorBarEnabled())
            layoutScale();
    }
}

/**
 * \if ENGLISH
 * @brief Get the width of the color bar
 * @return Width of the color bar
 * \sa setColorBarEnabled(), setColorBarWidth()
 * \endif
 * \if CHINESE
 * @brief 获取颜色条的宽度
 * @return 颜色条的宽度
 * \sa setColorBarEnabled(), setColorBarWidth()
 * \endif
 */
int QwtScaleWidget::colorBarWidth() const
{
    return m_data->colorBar.width;
}

/**
 * \if ENGLISH
 * @brief Get the value interval for the color bar
 * @return Value interval for the color bar
 * \sa setColorMap(), colorMap()
 * \endif
 * \if CHINESE
 * @brief 获取颜色条的值区间
 * @return 颜色条的值区间
 * \sa setColorMap(), colorMap()
 * \endif
 */
QwtInterval QwtScaleWidget::colorBarInterval() const
{
    return m_data->colorBar.interval;
}

/**
 * \if ENGLISH
 * @brief Set the color map and value interval used for displaying the color bar
 * @param interval Value interval
 * @param colorMap Color map
 * \sa colorMap(), colorBarInterval()
 * \endif
 * \if CHINESE
 * @brief 设置用于显示颜色条的颜色映射和值区间
 * @param interval 值区间
 * @param colorMap 颜色映射
 * \sa colorMap(), colorBarInterval()
 * \endif
 */
void QwtScaleWidget::setColorMap(const QwtInterval& interval, QwtColorMap* colorMap)
{
    m_data->colorBar.interval = interval;

    if (colorMap != m_data->colorBar.colorMap.get()) {
        m_data->colorBar.colorMap.reset(colorMap);
    }

    if (isColorBarEnabled())
        layoutScale();
}

/**
 * \if ENGLISH
 * @brief Get the color map
 * @return Color map
 * \sa setColorMap(), colorBarInterval()
 * \endif
 * \if CHINESE
 * @brief 获取颜色映射
 * @return 颜色映射
 * \sa setColorMap(), colorBarInterval()
 * \endif
 */
const QwtColorMap* QwtScaleWidget::colorMap() const
{
    return m_data->colorBar.colorMap.get();
}
