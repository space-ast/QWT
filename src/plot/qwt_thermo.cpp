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

#include "qwt_thermo.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qevent.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qmargins.h>

#include <algorithm>
#include <functional>

static inline void qwtDrawLine(QPainter* painter,
                               int pos,
                               const QColor& color,
                               const QRect& pipeRect,
                               const QRect& liquidRect,
                               Qt::Orientation orientation)
{
    painter->setPen(color);
    if (orientation == Qt::Horizontal) {
        if (pos >= liquidRect.left() && pos < liquidRect.right())
            painter->drawLine(pos, pipeRect.top(), pos, pipeRect.bottom());
    } else {
        if (pos >= liquidRect.top() && pos < liquidRect.bottom())
            painter->drawLine(pipeRect.left(), pos, pipeRect.right(), pos);
    }
}

static QVector< double > qwtTickList(const QwtScaleDiv& scaleDiv)
{
    QVector< double > values;

    double lowerLimit = scaleDiv.interval().minValue();
    double upperLimit = scaleDiv.interval().maxValue();

    if (upperLimit < lowerLimit)
        qSwap(lowerLimit, upperLimit);

    values += lowerLimit;

    for (int tickType = QwtScaleDiv::MinorTick; tickType < QwtScaleDiv::NTickTypes; tickType++) {
        const QList< double > ticks = scaleDiv.ticks(tickType);

        for (int i = 0; i < ticks.count(); i++) {
            const double v = ticks[ i ];
            if (v > lowerLimit && v < upperLimit)
                values += v;
        }
    }

    values += upperLimit;

    return values;
}

class QwtThermo::PrivateData
{
public:
    PrivateData()
        : orientation(Qt::Vertical)
        , scalePosition(QwtThermo::TrailingScale)
        , spacing(3)
        , borderWidth(2)
        , pipeWidth(10)
        , alarmLevel(0.0)
        , alarmEnabled(false)
        , autoFillPipe(true)
        , originMode(QwtThermo::OriginMinimum)
        , origin(0.0)
        , colorMap(nullptr)
        , value(0.0)
    {
        rangeFlags = QwtInterval::IncludeBorders;
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    Qt::Orientation orientation;
    QwtThermo::ScalePosition scalePosition;

    int spacing;
    int borderWidth;
    int pipeWidth;

    QwtInterval::BorderFlags rangeFlags;
    double alarmLevel;
    bool alarmEnabled;
    bool autoFillPipe;
    QwtThermo::OriginMode originMode;
    double origin;

    QwtColorMap* colorMap;

    double value;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param parent Parent widget
 * \sa ~QwtThermo()
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param parent 父控件
 * \sa ~QwtThermo()
 * \endif
 */
QwtThermo::QwtThermo(QWidget* parent) : QwtAbstractScale(parent)
{
    m_data = new PrivateData;

    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    if (m_data->orientation == Qt::Vertical)
        policy.transpose();

    setSizePolicy(policy);

    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    layoutThermo(true);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \sa QwtThermo()
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \sa QwtThermo()
 * \endif
 */
QwtThermo::~QwtThermo()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Exclude/Include min/max values
 * @details According to the flags minValue() and maxValue() are included/excluded
 *          from the pipe. In case of an excluded value the corresponding tick
 *          is painted 1 pixel off of the pipeRect().
 *          For example, when a minimum of 0.0 has to be displayed as an empty pipe,
 *          the minValue() needs to be excluded.
 * @param flags Range flags
 * \sa rangeFlags()
 * \endif
 * \if CHINESE
 * @brief 排除/包含最小/最大值
 * @details 根据标志，minValue() 和 maxValue() 从管道中排除/包含。
 *          对于排除的值，对应的刻度绘制在 pipeRect() 外 1 像素处。
 *          例如，当最小值 0.0 需要显示为空管道时，需要排除 minValue()。
 * @param flags 范围标志
 * \sa rangeFlags()
 * \endif
 */
void QwtThermo::setRangeFlags(QwtInterval::BorderFlags flags)
{
    if (m_data->rangeFlags != flags) {
        m_data->rangeFlags = flags;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return range flags
 * \sa setRangeFlags()
 * \endif
 * \if CHINESE
 * @brief 返回范围标志
 * \sa setRangeFlags()
 * \endif
 */
QwtInterval::BorderFlags QwtThermo::rangeFlags() const
{
    return m_data->rangeFlags;
}

/**
 * \if ENGLISH
 * @brief Set the current value
 * @param value New value
 * \sa value()
 * \endif
 * \if CHINESE
 * @brief 设置当前值
 * @param value 新值
 * \sa value()
 * \endif
 */
void QwtThermo::setValue(double value)
{
    if (m_data->value != value) {
        m_data->value = value;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return the value
 * \sa setValue()
 * \endif
 * \if CHINESE
 * @brief 返回当前值
 * \sa setValue()
 * \endif
 */
double QwtThermo::value() const
{
    return m_data->value;
}

/**
 * \if ENGLISH
 * @brief Set a scale draw
 * @details For changing the labels of the scales, it is necessary to derive
 *          from QwtScaleDraw and overload QwtScaleDraw::label().
 * @param scaleDraw ScaleDraw object that has to be created with new and will be
 *                  deleted in ~QwtThermo() or the next call of setScaleDraw()
 * \sa scaleDraw(), QwtScaleDraw
 * \endif
 * \if CHINESE
 * @brief 设置刻度绘制器
 * @details 要更改刻度标签，需要从 QwtScaleDraw 派生并重载 QwtScaleDraw::label()。
 * @param scaleDraw 刻度绘制对象，必须用 new 创建，将在 ~QwtThermo() 或下次调用
 *                  setScaleDraw() 时删除
 * \sa scaleDraw(), QwtScaleDraw
 * \endif
 */
void QwtThermo::setScaleDraw(QwtScaleDraw* scaleDraw)
{
    setAbstractScaleDraw(scaleDraw);
    layoutThermo(true);
}

/**
 * \if ENGLISH
 * @brief Return the scale draw of the thermometer (const version)
 * \sa setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回温度计的刻度绘制器 (const 版本)
 * \sa setScaleDraw()
 * \endif
 */
const QwtScaleDraw* QwtThermo::scaleDraw() const
{
    return static_cast< const QwtScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Return the scale draw of the thermometer (non-const version)
 * \sa setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回温度计的刻度绘制器 (非 const 版本)
 * \sa setScaleDraw()
 * \endif
 */
QwtScaleDraw* QwtThermo::scaleDraw()
{
    return static_cast< QwtScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Paint event handler
 * @param event Paint event
 * \sa resizeEvent(), changeEvent()
 * \endif
 * \if CHINESE
 * @brief 绘制事件处理器
 * @param event 绘制事件
 * \sa resizeEvent(), changeEvent()
 * \endif
 */
void QwtThermo::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    const QRect tRect = pipeRect();

    if (!tRect.contains(event->rect())) {
        if (m_data->scalePosition != QwtThermo::NoScale)
            scaleDraw()->draw(&painter, palette());
    }

    const int bw = m_data->borderWidth;

    const QBrush brush = palette().brush(QPalette::Base);
    qDrawShadePanel(&painter, tRect.adjusted(-bw, -bw, bw, bw), palette(), true, bw, m_data->autoFillPipe ? &brush : nullptr);

    drawLiquid(&painter, tRect);
}

/**
 * \if ENGLISH
 * @brief Resize event handler
 * @param event Resize event
 * \sa paintEvent(), layoutThermo()
 * \endif
 * \if CHINESE
 * @brief 调整大小事件处理器
 * @param event 调整大小事件
 * \sa paintEvent(), layoutThermo()
 * \endif
 */
void QwtThermo::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    layoutThermo(false);
}

/**
 * \if ENGLISH
 * @brief Qt change event handler
 * @param event Event
 * \sa paintEvent(), resizeEvent()
 * \endif
 * \if CHINESE
 * @brief Qt 变更事件处理器
 * @param event 事件
 * \sa paintEvent(), resizeEvent()
 * \endif
 */
void QwtThermo::changeEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::StyleChange:
    case QEvent::FontChange: {
        layoutThermo(true);
        break;
    }
    default:
        break;
    }
}

/**
 * \if ENGLISH
 * @brief Recalculate the QwtThermo geometry and layout
 * @details Recalculates geometry and layout based on pipeRect() and the fonts.
 * @param update_geometry Notify the layout system and call update to redraw the scale
 * \sa pipeRect(), layoutThermo()
 * \endif
 * \if CHINESE
 * @brief 重新计算 QwtThermo 几何布局和布局
 * @details 基于 pipeRect() 和字体重新计算几何和布局。
 * @param update_geometry 通知布局系统并调用 update 重新绘制刻度
 * \sa pipeRect(), layoutThermo()
 * \endif
 */
void QwtThermo::layoutThermo(bool update_geometry)
{
    const QRect tRect   = pipeRect();
    const int bw        = m_data->borderWidth + m_data->spacing;
    const bool inverted = (upperBound() < lowerBound());

    int from, to;

    if (m_data->orientation == Qt::Horizontal) {
        from = tRect.left();
        to   = tRect.right();

        if (m_data->rangeFlags & QwtInterval::ExcludeMinimum) {
            if (inverted)
                to++;
            else
                from--;
        }
        if (m_data->rangeFlags & QwtInterval::ExcludeMaximum) {
            if (inverted)
                from--;
            else
                to++;
        }

        if (m_data->scalePosition == QwtThermo::TrailingScale) {
            scaleDraw()->setAlignment(QwtScaleDraw::TopScale);
            scaleDraw()->move(from, tRect.top() - bw);
        } else {
            scaleDraw()->setAlignment(QwtScaleDraw::BottomScale);
            scaleDraw()->move(from, tRect.bottom() + bw);
        }

        scaleDraw()->setLength(qMax(to - from, 0));
    } else  // Qt::Vertical
    {
        from = tRect.top();
        to   = tRect.bottom();

        if (m_data->rangeFlags & QwtInterval::ExcludeMinimum) {
            if (inverted)
                from--;
            else
                to++;
        }
        if (m_data->rangeFlags & QwtInterval::ExcludeMaximum) {
            if (inverted)
                to++;
            else
                from--;
        }

        if (m_data->scalePosition == QwtThermo::LeadingScale) {
            scaleDraw()->setAlignment(QwtScaleDraw::RightScale);
            scaleDraw()->move(tRect.right() + bw, from);
        } else {
            scaleDraw()->setAlignment(QwtScaleDraw::LeftScale);
            scaleDraw()->move(tRect.left() - bw, from);
        }

        scaleDraw()->setLength(qMax(to - from, 0));
    }

    if (update_geometry) {
        updateGeometry();
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return bounding rectangle of the pipe
 * @returns Bounding rectangle of the pipe (without borders) in widget coordinates
 * \sa fillRect(), alarmRect()
 * \endif
 * \if CHINESE
 * @brief 返回管道的边界矩形
 * @returns 管道边界矩形 (不含边框),坐标为控件坐标
 * \sa fillRect(), alarmRect()
 * \endif
 */
QRect QwtThermo::pipeRect() const
{
    int mbd = 0;
    if (m_data->scalePosition != QwtThermo::NoScale) {
        int d1, d2;
        scaleDraw()->getBorderDistHint(font(), d1, d2);
        mbd = qMax(d1, d2);
    }
    const int bw       = m_data->borderWidth;
    const int scaleOff = bw + mbd;

    const QRect cr = contentsRect();

    QRect pipeRect = cr;
    if (m_data->orientation == Qt::Horizontal) {
        pipeRect.adjust(scaleOff, 0, -scaleOff, 0);

        if (m_data->scalePosition == QwtThermo::TrailingScale)
            pipeRect.setTop(cr.top() + cr.height() - bw - m_data->pipeWidth);
        else
            pipeRect.setTop(bw);

        pipeRect.setHeight(m_data->pipeWidth);
    } else  // Qt::Vertical
    {
        pipeRect.adjust(0, scaleOff, 0, -scaleOff);

        if (m_data->scalePosition == QwtThermo::LeadingScale)
            pipeRect.setLeft(bw);
        else
            pipeRect.setLeft(cr.left() + cr.width() - bw - m_data->pipeWidth);

        pipeRect.setWidth(m_data->pipeWidth);
    }

    return pipeRect;
}

/**
 * \if ENGLISH
 * @brief Set the orientation
 * @param orientation Allowed values are Qt::Horizontal and Qt::Vertical
 * \sa orientation(), scalePosition()
 * \endif
 * \if CHINESE
 * @brief 设置方向
 * @param orientation 允许的值为 Qt::Horizontal 和 Qt::Vertical
 * \sa orientation(), scalePosition()
 * \endif
 */
void QwtThermo::setOrientation(Qt::Orientation orientation)
{
    if (orientation == m_data->orientation)
        return;

    m_data->orientation = orientation;

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    layoutThermo(true);
}

/**
 * \if ENGLISH
 * @brief Return orientation
 * \sa setOrientation()
 * \endif
 * \if CHINESE
 * @brief 返回方向
 * \sa setOrientation()
 * \endif
 */
Qt::Orientation QwtThermo::orientation() const
{
    return m_data->orientation;
}

/**
 * \if ENGLISH
 * @brief Change how the origin is determined
 * \sa originMode(), setOrigin(), origin()
 * \endif
 * \if CHINESE
 * @brief 更改如何确定原点
 * \sa originMode(), setOrigin(), origin()
 * \endif
 */
void QwtThermo::setOriginMode(OriginMode m)
{
    if (m == m_data->originMode)
        return;

    m_data->originMode = m;
    update();
}

/**
 * \if ENGLISH
 * @brief Return mode how the origin is determined
 * \sa setOriginMode(), setOrigin(), origin()
 * \endif
 * \if CHINESE
 * @brief 返回如何确定原点的模式
 * \sa setOriginMode(), setOrigin(), origin()
 * \endif
 */
QwtThermo::OriginMode QwtThermo::originMode() const
{
    return m_data->originMode;
}

/**
 * \if ENGLISH
 * @brief Specify the custom origin
 * @details If originMode is set to OriginCustom this property controls where the liquid starts.
 * @param origin New origin level
 * \sa setOriginMode(), originMode(), origin()
 * \endif
 * \if CHINESE
 * @brief 指定自定义原点
 * @details 如果 originMode 设置为 OriginCustom，此属性控制液体从哪里开始。
 * @param origin 新原点水平
 * \sa setOriginMode(), originMode(), origin()
 * \endif
 */
void QwtThermo::setOrigin(double origin)
{
    if (origin == m_data->origin)
        return;

    m_data->origin = origin;
    update();
}

/**
 * \if ENGLISH
 * @brief Return origin of the thermometer when OriginCustom is enabled
 * \sa setOrigin(), setOriginMode(), originMode()
 * \endif
 * \if CHINESE
 * @brief 当启用 OriginCustom 时返回温度计的原点
 * \sa setOrigin(), setOriginMode(), originMode()
 * \endif
 */
double QwtThermo::origin() const
{
    return m_data->origin;
}

/**
 * \if ENGLISH
 * @brief Change the position of the scale
 * @param scalePosition Position of the scale
 * \sa ScalePosition, scalePosition()
 * \endif
 * \if CHINESE
 * @brief 更改刻度位置
 * @param scalePosition 刻度位置
 * \sa ScalePosition, scalePosition()
 * \endif
 */
void QwtThermo::setScalePosition(ScalePosition scalePosition)
{
    if (m_data->scalePosition == scalePosition)
        return;

    m_data->scalePosition = scalePosition;

    if (testAttribute(Qt::WA_WState_Polished))
        layoutThermo(true);
}

/**
 * \if ENGLISH
 * @brief Return scale position
 * \sa setScalePosition()
 * \endif
 * \if CHINESE
 * @brief 返回刻度位置
 * \sa setScalePosition()
 * \endif
 */
QwtThermo::ScalePosition QwtThermo::scalePosition() const
{
    return m_data->scalePosition;
}

/**
 * \if ENGLISH
 * @brief Notify a scale change
 * \sa scaleChange()
 * \endif
 * \if CHINESE
 * @brief 通知刻度变更
 * \sa scaleChange()
 * \endif
 */
void QwtThermo::scaleChange()
{
    layoutThermo(true);
}

/**
 * \if ENGLISH
 * @brief Redraw the liquid in thermometer pipe
 * @param painter Painter
 * @param pipeRect Bounding rectangle of the pipe without borders
 * \sa pipeRect(), fillRect(), alarmRect()
 * \endif
 * \if CHINESE
 * @brief 重新绘制温度计管道中的液体
 * @param painter 绘制器
 * @param pipeRect 管道边界矩形 (不含边框)
 * \sa pipeRect(), fillRect(), alarmRect()
 * \endif
 */
void QwtThermo::drawLiquid(QPainter* painter, const QRect& pipeRect) const
{
    painter->save();
    painter->setClipRect(pipeRect, Qt::IntersectClip);
    painter->setPen(Qt::NoPen);

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    QRect liquidRect = fillRect(pipeRect);

    if (m_data->colorMap != nullptr) {
        const QwtInterval interval = scaleDiv().interval().normalized();

        // Because the positions of the ticks are rounded
        // we calculate the colors for the rounded tick values

        QVector< double > values = qwtTickList(scaleDraw()->scaleDiv());

        if (scaleMap.isInverting())
            std::sort(values.begin(), values.end(), std::greater< double >());
        else
            std::sort(values.begin(), values.end(), std::less< double >());

        int from;
        if (!values.isEmpty()) {
            from = qRound(scaleMap.transform(values[ 0 ]));
            qwtDrawLine(painter, from, m_data->colorMap->color(interval, values[ 0 ]), pipeRect, liquidRect, m_data->orientation);
        }

        for (int i = 1; i < values.size(); i++) {
            const int to = qRound(scaleMap.transform(values[ i ]));

            for (int pos = from + 1; pos < to; pos++) {
                const double v = scaleMap.invTransform(pos);

                qwtDrawLine(painter, pos, m_data->colorMap->color(interval, v), pipeRect, liquidRect, m_data->orientation);
            }

            qwtDrawLine(painter, to, m_data->colorMap->color(interval, values[ i ]), pipeRect, liquidRect, m_data->orientation);

            from = to;
        }
    } else {
        if (!liquidRect.isEmpty() && m_data->alarmEnabled) {
            const QRect r = alarmRect(liquidRect);
            if (!r.isEmpty()) {
                painter->fillRect(r, palette().brush(QPalette::Highlight));
                liquidRect = QRegion(liquidRect).subtracted(r).boundingRect();
            }
        }

        painter->fillRect(liquidRect, palette().brush(QPalette::ButtonText));
    }

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Change the spacing between pipe and scale
 * @details A spacing of 0 means that the backbone of the scale is below the pipe.
 *          The default setting is 3 pixels.
 * @param spacing Number of pixels
 * \sa spacing()
 * \endif
 * \if CHINESE
 * @brief 更改管道和刻度之间的间距
 * @details 间距为 0 表示刻度的主干线在管道下方。默认设置为 3 像素。
 * @param spacing 像素数
 * \sa spacing()
 * \endif
 */
void QwtThermo::setSpacing(int spacing)
{
    if (spacing <= 0)
        spacing = 0;

    if (spacing != m_data->spacing) {
        m_data->spacing = spacing;
        layoutThermo(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return number of pixels between pipe and scale
 * \sa setSpacing()
 * \endif
 * \if CHINESE
 * @brief 返回管道和刻度之间的像素数
 * \sa setSpacing()
 * \endif
 */
int QwtThermo::spacing() const
{
    return m_data->spacing;
}

/**
 * \if ENGLISH
 * @brief Set the border width of the pipe
 * @param width Border width
 * \sa borderWidth()
 * \endif
 * \if CHINESE
 * @brief 设置管道的边框宽度
 * @param width 边框宽度
 * \sa borderWidth()
 * \endif
 */
void QwtThermo::setBorderWidth(int width)
{
    if (width <= 0)
        width = 0;

    if (width != m_data->borderWidth) {
        m_data->borderWidth = width;
        layoutThermo(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return border width of the thermometer pipe
 * \sa setBorderWidth()
 * \endif
 * \if CHINESE
 * @brief 返回温度计管道的边框宽度
 * \sa setBorderWidth()
 * \endif
 */
int QwtThermo::borderWidth() const
{
    return m_data->borderWidth;
}

/**
 * \if ENGLISH
 * @brief Assign a color map for the fill color
 * @param colorMap Color map
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \sa colorMap()
 * \endif
 * \if CHINESE
 * @brief 为填充颜色分配颜色映射
 * @param colorMap 颜色映射
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \sa colorMap()
 * \endif
 */
void QwtThermo::setColorMap(QwtColorMap* colorMap)
{
    if (colorMap != m_data->colorMap) {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }
}

/**
 * \if ENGLISH
 * @brief Return color map for the fill color
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \sa setColorMap()
 * \endif
 * \if CHINESE
 * @brief 返回填充颜色的颜色映射
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \sa setColorMap()
 * \endif
 */
QwtColorMap* QwtThermo::colorMap()
{
    return m_data->colorMap;
}

/**
 * \if ENGLISH
 * @brief Return color map for the fill color (const version)
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \sa setColorMap()
 * \endif
 * \if CHINESE
 * @brief 返回填充颜色的颜色映射 (const 版本)
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \sa setColorMap()
 * \endif
 */
const QwtColorMap* QwtThermo::colorMap() const
{
    return m_data->colorMap;
}

/**
 * \if ENGLISH
 * @brief Change the brush of the liquid
 * @details Changes the QPalette::ButtonText brush of the palette.
 * @param brush New brush
 * \sa fillBrush(), QWidget::setPalette()
 * \endif
 * \if CHINESE
 * @brief 更改液体的刷子
 * @details 更改调色板的 QPalette::ButtonText 刷子。
 * @param brush 新刷子
 * \sa fillBrush(), QWidget::setPalette()
 * \endif
 */
void QwtThermo::setFillBrush(const QBrush& brush)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::ButtonText, brush);
    setPalette(pal);
}

/**
 * \if ENGLISH
 * @brief Return liquid (QPalette::ButtonText) brush
 * \sa setFillBrush(), QWidget::palette()
 * \endif
 * \if CHINESE
 * @brief 返回液体 (QPalette::ButtonText) 刷子
 * \sa setFillBrush(), QWidget::palette()
 * \endif
 */
QBrush QwtThermo::fillBrush() const
{
    return palette().brush(QPalette::ButtonText);
}

/**
 * \if ENGLISH
 * @brief Specify the liquid brush above the alarm threshold
 * @details Changes the QPalette::Highlight brush of the palette.
 * @param brush New brush
 * \sa alarmBrush(), QWidget::setPalette()
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \endif
 * \if CHINESE
 * @brief 指定报警阈值以上的液体刷子
 * @details 更改调色板的 QPalette::Highlight 刷子。
 * @param brush 新刷子
 * \sa alarmBrush(), QWidget::setPalette()
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \endif
 */
void QwtThermo::setAlarmBrush(const QBrush& brush)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Highlight, brush);
    setPalette(pal);
}

/**
 * \if ENGLISH
 * @brief Return liquid brush (QPalette::Highlight) above the alarm threshold
 * \sa setAlarmBrush(), QWidget::palette()
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \endif
 * \if CHINESE
 * @brief 返回报警阈值以上的液体刷子 (QPalette::Highlight)
 * \sa setAlarmBrush(), QWidget::palette()
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \endif
 */
QBrush QwtThermo::alarmBrush() const
{
    return palette().brush(QPalette::Highlight);
}

/**
 * \if ENGLISH
 * @brief Specify the alarm threshold
 * @param level Alarm threshold
 * \sa alarmLevel()
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \endif
 * \if CHINESE
 * @brief 指定报警阈值
 * @param level 报警阈值
 * \sa alarmLevel()
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \endif
 */
void QwtThermo::setAlarmLevel(double level)
{
    m_data->alarmLevel   = level;
    m_data->alarmEnabled = 1;
    update();
}

/**
 * \if ENGLISH
 * @brief Return alarm threshold
 * \sa setAlarmLevel()
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \endif
 * \if CHINESE
 * @brief 返回报警阈值
 * \sa setAlarmLevel()
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \endif
 */
double QwtThermo::alarmLevel() const
{
    return m_data->alarmLevel;
}

/**
 * \if ENGLISH
 * @brief Change the width of the pipe
 * @param width Width of the pipe
 * \sa pipeWidth()
 * \endif
 * \if CHINESE
 * @brief 更改管道的宽度
 * @param width 管道宽度
 * \sa pipeWidth()
 * \endif
 */
void QwtThermo::setPipeWidth(int width)
{
    if (width > 0) {
        m_data->pipeWidth = width;
        layoutThermo(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return width of the pipe
 * \sa setPipeWidth()
 * \endif
 * \if CHINESE
 * @brief 返回管道的宽度
 * \sa setPipeWidth()
 * \endif
 */
int QwtThermo::pipeWidth() const
{
    return m_data->pipeWidth;
}

/**
 * \if ENGLISH
 * @brief Enable or disable the alarm threshold
 * @param on True to enable, false to disable
 * \sa alarmEnabled()
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \endif
 * \if CHINESE
 * @brief 启用或禁用报警阈值
 * @param on true 启用，false 禁用
 * \sa alarmEnabled()
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \endif
 */
void QwtThermo::setAlarmEnabled(bool on)
{
    m_data->alarmEnabled = on;
    update();
}

/**
 * \if ENGLISH
 * @brief Return true when the alarm threshold is enabled
 * \sa setAlarmEnabled()
 * @warning The alarm threshold has no effect when a color map has been assigned
 * \endif
 * \if CHINESE
 * @brief 当启用报警阈值时返回 true
 * \sa setAlarmEnabled()
 * @warning 分配颜色映射后，报警阈值将不起作用
 * \endif
 */
bool QwtThermo::alarmEnabled() const
{
    return m_data->alarmEnabled;
}

/**
 * \if ENGLISH
 * @brief Return the minimum size hint
 * \sa minimumSizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回最小尺寸提示
 * \sa minimumSizeHint()
 * \endif
 */
QSize QwtThermo::sizeHint() const
{
    return minimumSizeHint();
}

/**
 * \if ENGLISH
 * @brief Return minimum size hint
 * @details The return value depends on the font and the scale.
 * \sa sizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回最小尺寸提示
 * @details 返回值取决于字体和刻度。
 * \sa sizeHint()
 * \endif
 */
QSize QwtThermo::minimumSizeHint() const
{
    int w = 0, h = 0;

    if (m_data->scalePosition != NoScale) {
        const int sdExtent = qwtCeil(scaleDraw()->extent(font()));
        const int sdLength = scaleDraw()->minLength(font());

        w = sdLength;
        h = m_data->pipeWidth + sdExtent + m_data->spacing;

    } else  // no scale
    {
        w = 200;
        h = m_data->pipeWidth;
    }

    if (m_data->orientation == Qt::Vertical)
        qSwap(w, h);

    w += 2 * m_data->borderWidth;
    h += 2 * m_data->borderWidth;

    // finally add the margins
    const QMargins m = contentsMargins();
    w += m.left() + m.right();
    h += m.top() + m.bottom();

    return QSize(w, h);
}

/**
 * \if ENGLISH
 * @brief Calculate the filled rectangle of the pipe
 * @param pipeRect Rectangle of the pipe
 * @returns Rectangle to be filled (fill and alarm brush)
 * \sa pipeRect(), alarmRect()
 * \endif
 * \if CHINESE
 * @brief 计算管道的填充矩形
 * @param pipeRect 管道矩形
 * @returns 要填充的矩形 (填充和报警刷子)
 * \sa pipeRect(), alarmRect()
 * \endif
 */
QRect QwtThermo::fillRect(const QRect& pipeRect) const
{
    double origin;
    if (m_data->originMode == OriginMinimum) {
        origin = qMin(lowerBound(), upperBound());
    } else if (m_data->originMode == OriginMaximum) {
        origin = qMax(lowerBound(), upperBound());
    } else  // OriginCustom
    {
        origin = m_data->origin;
    }

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    int from = qRound(scaleMap.transform(m_data->value));
    int to   = qRound(scaleMap.transform(origin));

    if (to < from)
        qSwap(from, to);

    QRect fillRect = pipeRect;
    if (m_data->orientation == Qt::Horizontal) {
        fillRect.setLeft(from);
        fillRect.setRight(to);
    } else  // Qt::Vertical
    {
        fillRect.setTop(from);
        fillRect.setBottom(to);
    }

    return fillRect.normalized();
}

/**
 * \if ENGLISH
 * @brief Calculate the alarm rectangle of the pipe
 * @param fillRect Filled rectangle in the pipe
 * @returns Rectangle to be filled with the alarm brush
 * \sa pipeRect(), fillRect(), alarmLevel(), alarmBrush()
 * \endif
 * \if CHINESE
 * @brief 计算管道的报警矩形
 * @param fillRect 管道中的填充矩形
 * @returns 要用报警刷子填充的矩形
 * \sa pipeRect(), fillRect(), alarmLevel(), alarmBrush()
 * \endif
 */
QRect QwtThermo::alarmRect(const QRect& fillRect) const
{
    QRect alarmRect(0, 0, -1, -1);  // something invalid

    if (!m_data->alarmEnabled)
        return alarmRect;

    const bool inverted = (upperBound() < lowerBound());

    bool increasing;
    if (m_data->originMode == OriginCustom) {
        increasing = m_data->value > m_data->origin;
    } else {
        increasing = m_data->originMode == OriginMinimum;
    }

    const QwtScaleMap map = scaleDraw()->scaleMap();
    const int alarmPos    = qRound(map.transform(m_data->alarmLevel));
    const int valuePos    = qRound(map.transform(m_data->value));

    if (m_data->orientation == Qt::Horizontal) {
        int v1, v2;
        if (inverted) {
            v1 = fillRect.left();

            v2 = alarmPos - 1;
            v2 = qMin(v2, increasing ? fillRect.right() : valuePos);
        } else {
            v1 = alarmPos + 1;
            v1 = qMax(v1, increasing ? fillRect.left() : valuePos);

            v2 = fillRect.right();
        }
        alarmRect.setRect(v1, fillRect.top(), v2 - v1 + 1, fillRect.height());
    } else {
        int v1, v2;
        if (inverted) {
            v1 = alarmPos + 1;
            v1 = qMax(v1, increasing ? fillRect.top() : valuePos);

            v2 = fillRect.bottom();
        } else {
            v1 = fillRect.top();

            v2 = alarmPos - 1;
            v2 = qMin(v2, increasing ? fillRect.bottom() : valuePos);
        }
        alarmRect.setRect(fillRect.left(), v1, fillRect.width(), v2 - v1 + 1);
    }

    return alarmRect;
}
