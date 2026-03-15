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

#include "qwt_slider.h"
#include "qwt_painter.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_utils.h"

#include <qevent.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qmargins.h>

static QSize qwtHandleSize(const QSize& size, Qt::Orientation orientation, bool hasTrough)
{
    QSize handleSize = size;

    if (handleSize.isEmpty()) {
        const int handleThickness = 16;
        handleSize.setWidth(2 * handleThickness);
        handleSize.setHeight(handleThickness);

        if (!hasTrough)
            handleSize.transpose();

        if (orientation == Qt::Vertical)
            handleSize.transpose();
    }

    return handleSize;
}

static QwtScaleDraw::Alignment qwtScaleDrawAlignment(Qt::Orientation orientation, QwtSlider::ScalePosition scalePos)
{
    QwtScaleDraw::Alignment align;

    if (orientation == Qt::Vertical) {
        // NoScale lays out like Left
        if (scalePos == QwtSlider::LeadingScale)
            align = QwtScaleDraw::RightScale;
        else
            align = QwtScaleDraw::LeftScale;
    } else {
        // NoScale lays out like Bottom
        if (scalePos == QwtSlider::TrailingScale)
            align = QwtScaleDraw::TopScale;
        else
            align = QwtScaleDraw::BottomScale;
    }

    return align;
}

class QwtSlider::PrivateData
{
public:
    PrivateData()
        : repeatTimerId(0)
        , updateInterval(150)
        , stepsIncrement(0)
        , pendingValueChange(false)
        , borderWidth(2)
        , spacing(4)
        , scalePosition(QwtSlider::TrailingScale)
        , hasTrough(true)
        , hasGroove(false)
        , mouseOffset(0)
    {
    }

    int repeatTimerId;
    bool timerTick;
    int updateInterval;
    int stepsIncrement;
    bool pendingValueChange;

    QRect sliderRect;

    QSize handleSize;
    int borderWidth;
    int spacing;

    Qt::Orientation orientation;
    QwtSlider::ScalePosition scalePosition;

    bool hasTrough;
    bool hasGroove;

    int mouseOffset;

    mutable QSize sizeHintCache;
};
/**
 * \if ENGLISH
 * @brief Construct vertical slider in QwtSlider::Trough style
 * @details Construct vertical slider in QwtSlider::Trough style with a scale to the left.
 *          The scale is initialized to [0.0, 100.0] and the value set to 0.0.
 * @param parent Parent widget
 * \sa setOrientation(), setScalePosition()
 * \endif
 * \if CHINESE
 * @brief 构造垂直滑块 (QwtSlider::Trough 样式)
 * @details 构造左侧带刻度的 QwtSlider::Trough 样式垂直滑块。
 *          刻度初始化为 [0.0, 100.0]，值设置为 0.0。
 * @param parent 父控件
 * \sa setOrientation(), setScalePosition()
 * \endif
 */
QwtSlider::QwtSlider(QWidget* parent) : QwtAbstractSlider(parent)
{
    initSlider(Qt::Vertical);
}

/**
 * \if ENGLISH
 * @brief Construct a slider in QwtSlider::Trough style
 * @details Construct a slider in QwtSlider::Trough style.
 *          When orientation is Qt::Vertical the scale will be aligned to the left,
 *          otherwise at the top of the slider.
 *          The scale is initialized to [0.0, 100.0] and the value set to 0.0.
 * @param parent Parent widget
 * @param orientation Orientation of the slider
 * \sa setOrientation(), setScalePosition()
 * \endif
 * \if CHINESE
 * @brief 构造滑块 (QwtSlider::Trough 样式)
 * @details 构造 QwtSlider::Trough 样式的滑块。
 *          当方向为 Qt::Vertical时刻度对齐到左侧，否则在滑块顶部。
 *          刻度初始化为 [0.0, 100.0]，值设置为 0.0。
 * @param parent 父控件
 * @param orientation 滑块方向
 * \sa setOrientation(), setScalePosition()
 * \endif
 */
QwtSlider::QwtSlider(Qt::Orientation orientation, QWidget* parent) : QwtAbstractSlider(parent)
{
    initSlider(orientation);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \sa QwtSlider()
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \sa QwtSlider()
 * \endif
 */
QwtSlider::~QwtSlider()
{
    delete m_data;
}

void QwtSlider::initSlider(Qt::Orientation orientation)
{
    if (orientation == Qt::Vertical)
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    else
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setAttribute(Qt::WA_WState_OwnSizePolicy, false);

    m_data = new QwtSlider::PrivateData;

    m_data->orientation = orientation;

    scaleDraw()->setAlignment(qwtScaleDrawAlignment(orientation, m_data->scalePosition));
    scaleDraw()->setLength(100);

    setScale(0.0, 100.0);
    setValue(0.0);
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
void QwtSlider::setOrientation(Qt::Orientation orientation)
{
    if (orientation == m_data->orientation)
        return;

    m_data->orientation = orientation;

    scaleDraw()->setAlignment(qwtScaleDrawAlignment(orientation, m_data->scalePosition));

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    if (testAttribute(Qt::WA_WState_Polished))
        layoutSlider(true);
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
Qt::Orientation QwtSlider::orientation() const
{
    return m_data->orientation;
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
void QwtSlider::setScalePosition(ScalePosition scalePosition)
{
    if (m_data->scalePosition == scalePosition)
        return;

    m_data->scalePosition = scalePosition;
    scaleDraw()->setAlignment(qwtScaleDrawAlignment(m_data->orientation, scalePosition));

    if (testAttribute(Qt::WA_WState_Polished))
        layoutSlider(true);
}

/**
 * \if ENGLISH
 * @brief Return position of the scale
 * \sa setScalePosition()
 * \endif
 * \if CHINESE
 * @brief 返回刻度位置
 * \sa setScalePosition()
 * \endif
 */
QwtSlider::ScalePosition QwtSlider::scalePosition() const
{
    return m_data->scalePosition;
}

/**
 * \if ENGLISH
 * @brief Change the slider's border width
 * @details The border width is used for drawing the slider handle and the trough.
 * @param width Border width
 * \sa borderWidth()
 * \endif
 * \if CHINESE
 * @brief 更改滑块的边框宽度
 * @details 边框宽度用于绘制滑块手柄和槽。
 * @param width 边框宽度
 * \sa borderWidth()
 * \endif
 */
void QwtSlider::setBorderWidth(int width)
{
    if (width < 0)
        width = 0;

    if (width != m_data->borderWidth) {
        m_data->borderWidth = width;

        if (testAttribute(Qt::WA_WState_Polished))
            layoutSlider(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return the border width
 * \sa setBorderWidth()
 * \endif
 * \if CHINESE
 * @brief 返回边框宽度
 * \sa setBorderWidth()
 * \endif
 */
int QwtSlider::borderWidth() const
{
    return m_data->borderWidth;
}

/**
 * \if ENGLISH
 * @brief Change the spacing between trough and scale
 * @details A spacing of 0 means that the backbone of the scale is covered by the trough.
 *          The default setting is 4 pixels.
 * @param spacing Number of pixels
 * \sa spacing()
 * \endif
 * \if CHINESE
 * @brief 更改槽和刻度之间的间距
 * @details 间距为 0 表示刻度的主干线被槽覆盖。默认设置为 4 像素。
 * @param spacing 像素数
 * \sa spacing()
 * \endif
 */
void QwtSlider::setSpacing(int spacing)
{
    if (spacing <= 0)
        spacing = 0;

    if (spacing != m_data->spacing) {
        m_data->spacing = spacing;

        if (testAttribute(Qt::WA_WState_Polished))
            layoutSlider(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return number of pixels between slider and scale
 * \sa setSpacing()
 * \endif
 * \if CHINESE
 * @brief 返回滑块和刻度之间的像素数
 * \sa setSpacing()
 * \endif
 */
int QwtSlider::spacing() const
{
    return m_data->spacing;
}

/**
 * \if ENGLISH
 * @brief Set the slider's handle size
 * @details When the size is empty the slider handle will be painted with a
 *          default size depending on its orientation() and backgroundStyle().
 * @param size New size
 * \sa handleSize()
 * \endif
 * \if CHINESE
 * @brief 设置滑块手柄大小
 * @details 当大小为 empty 时，滑块手柄将根据其 orientation() 和 backgroundStyle()
 *          使用默认大小绘制。
 * @param size 新大小
 * \sa handleSize()
 * \endif
 */
void QwtSlider::setHandleSize(const QSize& size)
{
    if (size != m_data->handleSize) {
        m_data->handleSize = size;

        if (testAttribute(Qt::WA_WState_Polished))
            layoutSlider(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return size of the handle
 * \sa setHandleSize()
 * \endif
 * \if CHINESE
 * @brief 返回手柄大小
 * \sa setHandleSize()
 * \endif
 */
QSize QwtSlider::handleSize() const
{
    return m_data->handleSize;
}

/**
 * \if ENGLISH
 * @brief Set a scale draw
 * @details For changing the labels of the scales, it is necessary to derive
 *          from QwtScaleDraw and overload QwtScaleDraw::label().
 * @param scaleDraw ScaleDraw object that has to be created with new and will be
 *                  deleted in ~QwtSlider() or the next call of setScaleDraw()
 * \sa scaleDraw()
 * \endif
 * \if CHINESE
 * @brief 设置刻度绘制器
 * @details 要更改刻度标签，需要从 QwtScaleDraw 派生并重载 QwtScaleDraw::label()。
 * @param scaleDraw 刻度绘制对象，必须用 new 创建，将在 ~QwtSlider() 或下次调用
 *                  setScaleDraw() 时删除
 * \sa scaleDraw()
 * \endif
 */
void QwtSlider::setScaleDraw(QwtScaleDraw* scaleDraw)
{
    const QwtScaleDraw* previousScaleDraw = this->scaleDraw();
    if (scaleDraw == nullptr || scaleDraw == previousScaleDraw)
        return;

    if (previousScaleDraw)
        scaleDraw->setAlignment(previousScaleDraw->alignment());

    setAbstractScaleDraw(scaleDraw);

    if (testAttribute(Qt::WA_WState_Polished))
        layoutSlider(true);
}

/**
 * \if ENGLISH
 * @brief Return the scale draw of the slider (const version)
 * \sa setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回滑块的刻度绘制器 (const 版本)
 * \sa setScaleDraw()
 * \endif
 */
const QwtScaleDraw* QwtSlider::scaleDraw() const
{
    return static_cast< const QwtScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Return the scale draw of the slider (non-const version)
 * \sa setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回滑块的刻度绘制器 (非 const 版本)
 * \sa setScaleDraw()
 * \endif
 */
QwtScaleDraw* QwtSlider::scaleDraw()
{
    return static_cast< QwtScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Notify changed scale
 * \sa scaleChange()
 * \endif
 * \if CHINESE
 * @brief 通知刻度变更
 * \sa scaleChange()
 * \endif
 */
void QwtSlider::scaleChange()
{
    QwtAbstractSlider::scaleChange();

    if (testAttribute(Qt::WA_WState_Polished))
        layoutSlider(true);
}

/**
 * \if ENGLISH
 * @brief Specify the update interval for automatic scrolling
 * @details The minimal accepted value is 50 ms.
 * @param interval Update interval in milliseconds
 * \sa updateInterval()
 * \endif
 * \if CHINESE
 * @brief 指定自动刷新的更新间隔
 * @details 最小接受值为 50 毫秒。
 * @param interval 更新间隔 (毫秒)
 * \sa updateInterval()
 * \endif
 */
void QwtSlider::setUpdateInterval(int interval)
{
    m_data->updateInterval = qMax(interval, 50);
}

/**
 * \if ENGLISH
 * @brief Return update interval in milliseconds for automatic scrolling
 * \sa setUpdateInterval()
 * \endif
 * \if CHINESE
 * @brief 返回自动刷新的更新间隔 (毫秒)
 * \sa setUpdateInterval()
 * \endif
 */
int QwtSlider::updateInterval() const
{
    return m_data->updateInterval;
}

/**
 * \if ENGLISH
 * @brief Draw the slider into the specified rectangle
 * @param painter Painter
 * @param sliderRect Bounding rectangle of the slider
 * \sa drawHandle()
 * \endif
 * \if CHINESE
 * @brief 在指定矩形中绘制滑块
 * @param painter 绘制器
 * @param sliderRect 滑块的边界矩形
 * \sa drawHandle()
 * \endif
 */
void QwtSlider::drawSlider(QPainter* painter, const QRect& sliderRect) const
{
    QRect innerRect(sliderRect);

    if (m_data->hasTrough) {
        const int bw = m_data->borderWidth;
        innerRect    = sliderRect.adjusted(bw, bw, -bw, -bw);

        painter->fillRect(innerRect, palette().brush(QPalette::Mid));
        qDrawShadePanel(painter, sliderRect, palette(), true, bw, nullptr);
    }

    if (m_data->hasGroove) {
        const QSize handleSize = qwtHandleSize(m_data->handleSize, m_data->orientation, m_data->hasTrough);

        const int slotExtent = 4;
        const int slotMargin = 4;

        QRect slotRect;
        if (orientation() == Qt::Horizontal) {
            int slotOffset = qMax(1, handleSize.width() / 2 - slotMargin);
            int slotHeight = slotExtent + (innerRect.height() % 2);

            slotRect.setWidth(innerRect.width() - 2 * slotOffset);
            slotRect.setHeight(slotHeight);
        } else {
            int slotOffset = qMax(1, handleSize.height() / 2 - slotMargin);
            int slotWidth  = slotExtent + (innerRect.width() % 2);

            slotRect.setWidth(slotWidth);
            slotRect.setHeight(innerRect.height() - 2 * slotOffset);
        }

        slotRect.moveCenter(innerRect.center());

        QBrush brush = palette().brush(QPalette::Dark);
        qDrawShadePanel(painter, slotRect, palette(), true, 1, &brush);
    }

    if (isValid())
        drawHandle(painter, handleRect(), transform(value()));
}

/**
 * \if ENGLISH
 * @brief Draw the thumb at a position
 * @param painter Painter
 * @param handleRect Bounding rectangle of the handle
 * @param pos Position of the handle marker in widget coordinates
 * \sa drawSlider()
 * \endif
 * \if CHINESE
 * @brief 在指定位置绘制滑块手柄
 * @param painter 绘制器
 * @param handleRect 手柄的边界矩形
 * @param pos 手柄标记在控件坐标中的位置
 * \sa drawSlider()
 * \endif
 */
void QwtSlider::drawHandle(QPainter* painter, const QRect& handleRect, int pos) const
{
    const int bw = m_data->borderWidth;

    qDrawShadePanel(painter, handleRect, palette(), false, bw, &palette().brush(QPalette::Button));

    pos++;  // shade line points one pixel below
    if (orientation() == Qt::Horizontal) {
        qDrawShadeLine(painter, pos, handleRect.top() + bw, pos, handleRect.bottom() - bw, palette(), true, 1);
    } else  // Vertical
    {
        qDrawShadeLine(painter, handleRect.left() + bw, pos, handleRect.right() - bw, pos, palette(), true, 1);
    }
}

/**
 * \if ENGLISH
 * @brief Determine what to do when the user presses a mouse button
 * @param pos Mouse position
 * @returns True when handleRect() contains pos
 * \sa scrolledTo()
 * \endif
 * \if CHINESE
 * @brief 确定当用户按下鼠标按钮时该做什么
 * @param pos 鼠标位置
 * @returns 当 handleRect() 包含 pos 时为 true
 * \sa scrolledTo()
 * \endif
 */
bool QwtSlider::isScrollPosition(const QPoint& pos) const
{
    if (handleRect().contains(pos)) {
        const double v = (orientation() == Qt::Horizontal) ? pos.x() : pos.y();

        m_data->mouseOffset = v - transform(value());
        return true;
    }

    return false;
}

/**
 * \if ENGLISH
 * @brief Determine the value for a new position of the slider handle
 * @param pos Mouse position
 * @returns Value for the mouse position
 * \sa isScrollPosition()
 * \endif
 * \if CHINESE
 * @brief 确定滑块手柄新位置的值
 * @param pos 鼠标位置
 * @returns 鼠标位置对应的值
 * \sa isScrollPosition()
 * \endif
 */
double QwtSlider::scrolledTo(const QPoint& pos) const
{
    int p = (orientation() == Qt::Horizontal) ? pos.x() : pos.y();

    p -= m_data->mouseOffset;

    int min = transform(lowerBound());
    int max = transform(upperBound());
    if (min > max)
        qSwap(min, max);

    p = qBound(min, p, max);

    return scaleMap().invTransform(p);
}

/**
 * \if ENGLISH
 * @brief Mouse press event handler
 * @param event Mouse event
 * \sa mouseReleaseEvent(), QwtAbstractSlider::mousePressEvent()
 * \endif
 * \if CHINESE
 * @brief 鼠标按下事件处理器
 * @param event 鼠标事件
 * \sa mouseReleaseEvent(), QwtAbstractSlider::mousePressEvent()
 * \endif
 */
void QwtSlider::mousePressEvent(QMouseEvent* event)
{
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    const QPoint pos = event->pos();

    if (isValid() && m_data->sliderRect.contains(pos)) {
        if (!handleRect().contains(pos)) {
            const int markerPos = transform(value());

            m_data->stepsIncrement = pageSteps();

            if (m_data->orientation == Qt::Horizontal) {
                if (pos.x() < markerPos)
                    m_data->stepsIncrement = -m_data->stepsIncrement;
            } else {
                if (pos.y() < markerPos)
                    m_data->stepsIncrement = -m_data->stepsIncrement;
            }

            if (isInverted())
                m_data->stepsIncrement = -m_data->stepsIncrement;

            const double v = value();
            incrementValue(m_data->stepsIncrement);

            if (v != value()) {
                if (isTracking())
                    Q_EMIT valueChanged(value());
                else
                    m_data->pendingValueChange = true;

                Q_EMIT sliderMoved(value());
            }

            m_data->timerTick     = false;
            m_data->repeatTimerId = startTimer(qMax(250, 2 * updateInterval()));

            return;
        }
    }

    QwtAbstractSlider::mousePressEvent(event);
}

/**
 * \if ENGLISH
 * @brief Mouse release event handler
 * @param event Mouse event
 * \sa mousePressEvent(), QwtAbstractSlider::mouseReleaseEvent()
 * \endif
 * \if CHINESE
 * @brief 鼠标释放事件处理器
 * @param event 鼠标事件
 * \sa mousePressEvent(), QwtAbstractSlider::mouseReleaseEvent()
 * \endif
 */
void QwtSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_data->repeatTimerId > 0) {
        killTimer(m_data->repeatTimerId);
        m_data->repeatTimerId  = 0;
        m_data->timerTick      = false;
        m_data->stepsIncrement = 0;
    }

    if (m_data->pendingValueChange) {
        m_data->pendingValueChange = false;
        Q_EMIT valueChanged(value());
    }

    QwtAbstractSlider::mouseReleaseEvent(event);
}

/**
 * \if ENGLISH
 * @brief Timer event handler
 * @details Handles the timer when the mouse stays pressed inside the sliderRect().
 * @param event Timer event
 * \sa timerEvent()
 * \endif
 * \if CHINESE
 * @brief 定时器事件处理器
 * @details 当鼠标保持在 sliderRect() 内按下时处理定时器。
 * @param event 定时器事件
 * \sa timerEvent()
 * \endif
 */
void QwtSlider::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != m_data->repeatTimerId) {
        QwtAbstractSlider::timerEvent(event);
        return;
    }

    if (!isValid()) {
        killTimer(m_data->repeatTimerId);
        m_data->repeatTimerId = 0;
        return;
    }

    const double v = value();
    incrementValue(m_data->stepsIncrement);

    if (v != value()) {
        if (isTracking())
            Q_EMIT valueChanged(value());
        else
            m_data->pendingValueChange = true;

        Q_EMIT sliderMoved(value());
    }

    if (!m_data->timerTick) {
        // restart the timer with a shorter interval
        killTimer(m_data->repeatTimerId);
        m_data->repeatTimerId = startTimer(updateInterval());

        m_data->timerTick = true;
    }
}

/**
 * \if ENGLISH
 * @brief Qt paint event handler
 * @param event Paint event
 * \sa resizeEvent(), changeEvent()
 * \endif
 * \if CHINESE
 * @brief Qt 绘制事件处理器
 * @param event 绘制事件
 * \sa resizeEvent(), changeEvent()
 * \endif
 */
void QwtSlider::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (m_data->scalePosition != QwtSlider::NoScale) {
        if (!m_data->sliderRect.contains(event->rect()))
            scaleDraw()->draw(&painter, palette());
    }

    drawSlider(&painter, m_data->sliderRect);

    if (hasFocus())
        QwtPainter::drawFocusRect(&painter, this, m_data->sliderRect);
}

/**
 * \if ENGLISH
 * @brief Qt resize event handler
 * @param event Resize event
 * \sa paintEvent(), layoutSlider()
 * \endif
 * \if CHINESE
 * @brief Qt 调整大小事件处理器
 * @param event 调整大小事件
 * \sa paintEvent(), layoutSlider()
 * \endif
 */
void QwtSlider::resizeEvent(QResizeEvent* event)
{
    layoutSlider(false);
    QwtAbstractSlider::resizeEvent(event);
}

/**
 * \if ENGLISH
 * @brief Qt event handler
 * @param event Event
 * @returns True if event was recognized and processed
 * \sa QwtAbstractSlider::event()
 * \endif
 * \if CHINESE
 * @brief Qt 事件处理器
 * @param event 事件
 * @returns 如果事件被识别和处理则返回 true
 * \sa QwtAbstractSlider::event()
 * \endif
 */
bool QwtSlider::event(QEvent* event)
{
    if (event->type() == QEvent::PolishRequest)
        layoutSlider(false);

    return QwtAbstractSlider::event(event);
}

/**
 * \if ENGLISH
 * @brief Handles QEvent::StyleChange and QEvent::FontChange events
 * @param event Change event
 * \sa QwtAbstractSlider::changeEvent()
 * \endif
 * \if CHINESE
 * @brief 处理 QEvent::StyleChange 和 QEvent::FontChange 事件
 * @param event 变更事件
 * \sa QwtAbstractSlider::changeEvent()
 * \endif
 */
void QwtSlider::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::StyleChange || event->type() == QEvent::FontChange) {
        if (testAttribute(Qt::WA_WState_Polished))
            layoutSlider(true);
    }

    QwtAbstractSlider::changeEvent(event);
}

/**
 * \if ENGLISH
 * @brief Recalculate the slider's geometry and layout
 * @details Recalculates the slider's geometry and layout based on the current geometry and fonts.
 * @param update_geometry Notify the layout system and call update to redraw the scale
 * \sa layoutSlider()
 * \endif
 * \if CHINESE
 * @brief 重新计算滑块的几何布局和布局
 * @details 基于当前几何和字体重新计算滑块的几何和布局。
 * @param update_geometry 通知布局系统并调用 update 重新绘制刻度
 * \sa layoutSlider()
 * \endif
 */
void QwtSlider::layoutSlider(bool update_geometry)
{
    int bw = 0;
    if (m_data->hasTrough)
        bw = m_data->borderWidth;

    const QSize handleSize = qwtHandleSize(m_data->handleSize, m_data->orientation, m_data->hasTrough);

    QRect sliderRect = contentsRect();

    /*
       The marker line of the handle needs to be aligned to
       the scale. But the marker is in the center
       and we need space enough to display the rest of the handle.

       But the scale itself usually needs margins for displaying
       the tick labels, that also might needs space beyond the
       backbone.

       Now it depends on what needs more margins. If it is the
       slider the scale gets shrunk, otherwise the slider.
     */

    int scaleMargin = 0;
    if (m_data->scalePosition != QwtSlider::NoScale) {
        int d1, d2;
        scaleDraw()->getBorderDistHint(font(), d1, d2);

        scaleMargin = qMax(d1, d2) - bw;
    }

    int scaleX, scaleY, scaleLength;

    if (m_data->orientation == Qt::Horizontal) {
        const int handleMargin = handleSize.width() / 2 - 1;
        if (scaleMargin > handleMargin) {
            int off = scaleMargin - handleMargin;
            sliderRect.adjust(off, 0, -off, 0);
        }

        scaleX      = sliderRect.left() + bw + handleSize.width() / 2 - 1;
        scaleLength = sliderRect.width() - handleSize.width();
    } else {
        int handleMargin = handleSize.height() / 2 - 1;
        if (scaleMargin > handleMargin) {
            int off = scaleMargin - handleMargin;
            sliderRect.adjust(0, off, 0, -off);
        }

        scaleY      = sliderRect.top() + bw + handleSize.height() / 2 - 1;
        scaleLength = sliderRect.height() - handleSize.height();
    }

    scaleLength -= 2 * bw;

    // now align slider and scale according to the ScalePosition

    if (m_data->orientation == Qt::Horizontal) {
        const int h = handleSize.height() + 2 * bw;

        if (m_data->scalePosition == QwtSlider::TrailingScale) {
            sliderRect.setTop(sliderRect.bottom() + 1 - h);
            scaleY = sliderRect.top() - m_data->spacing;
        } else {
            sliderRect.setHeight(h);
            scaleY = sliderRect.bottom() + 1 + m_data->spacing;
        }
    } else  // Qt::Vertical
    {
        const int w = handleSize.width() + 2 * bw;

        if (m_data->scalePosition == QwtSlider::LeadingScale) {
            sliderRect.setWidth(w);
            scaleX = sliderRect.right() + 1 + m_data->spacing;
        } else {
            sliderRect.setLeft(sliderRect.right() + 1 - w);
            scaleX = sliderRect.left() - m_data->spacing;
        }
    }

    m_data->sliderRect = sliderRect;

    scaleDraw()->move(scaleX, scaleY);
    scaleDraw()->setLength(scaleLength);

    if (update_geometry) {
        m_data->sizeHintCache = QSize();  // invalidate
        updateGeometry();
        update();
    }
}

/**
 * \if ENGLISH
 * @brief En/Disable the trough
 * @details The slider can be customized by showing a trough for the handle.
 * @param on When true, the trough is visible
 * \sa hasTrough(), setGroove()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用槽
 * @details 滑块可以通过显示手柄的槽来自定义。
 * @param on 当 true 时，槽可见
 * \sa hasTrough(), setGroove()
 * \endif
 */
void QwtSlider::setTrough(bool on)
{
    if (m_data->hasTrough != on) {
        m_data->hasTrough = on;

        if (testAttribute(Qt::WA_WState_Polished))
            layoutSlider(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return true when the trough is visible
 * \sa setTrough(), hasGroove()
 * \endif
 * \if CHINESE
 * @brief 当槽可见时返回 true
 * \sa setTrough(), hasGroove()
 * \endif
 */
bool QwtSlider::hasTrough() const
{
    return m_data->hasTrough;
}

/**
 * \if ENGLISH
 * @brief En/Disable the groove
 * @details The slider can be customized by showing a groove for the handle.
 * @param on When true, the groove is visible
 * \sa hasGroove(), setTrough()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用凹槽
 * @details 滑块可以通过显示手柄的凹槽来自定义。
 * @param on 当 true 时，凹槽可见
 * \sa hasGroove(), setTrough()
 * \endif
 */
void QwtSlider::setGroove(bool on)
{
    if (m_data->hasGroove != on) {
        m_data->hasGroove = on;

        if (testAttribute(Qt::WA_WState_Polished))
            layoutSlider(true);
    }
}

/**
 * \if ENGLISH
 * @brief Return true when the groove is visible
 * \sa setGroove(), hasTrough()
 * \endif
 * \if CHINESE
 * @brief 当凹槽可见时返回 true
 * \sa setGroove(), hasTrough()
 * \endif
 */
bool QwtSlider::hasGroove() const
{
    return m_data->hasGroove;
}

/**
 * \if ENGLISH
 * @brief Return size hint
 * @returns minimumSizeHint()
 * \sa minimumSizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回大小提示
 * @returns minimumSizeHint()
 * \sa minimumSizeHint()
 * \endif
 */
QSize QwtSlider::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return qwtExpandedToGlobalStrut(hint);
}

/**
 * \if ENGLISH
 * @brief Return minimum size hint
 * \sa sizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回最小大小提示
 * \sa sizeHint()
 * \endif
 */
QSize QwtSlider::minimumSizeHint() const
{
    if (!m_data->sizeHintCache.isEmpty())
        return m_data->sizeHintCache;

    const QSize handleSize = qwtHandleSize(m_data->handleSize, m_data->orientation, m_data->hasTrough);

    int bw = 0;
    if (m_data->hasTrough)
        bw = m_data->borderWidth;

    int sliderLength = 0;
    int scaleExtent  = 0;

    if (m_data->scalePosition != QwtSlider::NoScale) {
        int d1, d2;
        scaleDraw()->getBorderDistHint(font(), d1, d2);

        const int scaleBorderDist = 2 * (qMax(d1, d2) - bw);

        int handleBorderDist;
        if (m_data->orientation == Qt::Horizontal)
            handleBorderDist = handleSize.width();
        else
            handleBorderDist = handleSize.height();

        sliderLength = scaleDraw()->minLength(font());
        if (handleBorderDist > scaleBorderDist) {
            // We need additional space for the overlapping handle
            sliderLength += handleBorderDist - scaleBorderDist;
        }

        scaleExtent += m_data->spacing;
        scaleExtent += qwtCeil(scaleDraw()->extent(font()));
    }

    sliderLength = qMax(sliderLength, 84);  // from QSlider

    int w = 0;
    int h = 0;

    if (m_data->orientation == Qt::Horizontal) {
        w = sliderLength;
        h = handleSize.height() + 2 * bw + scaleExtent;
    } else {
        w = handleSize.width() + 2 * bw + scaleExtent;
        h = sliderLength;
    }

    // finally add margins
    const QMargins m = contentsMargins();

    w += m.left() + m.right();
    h += m.top() + m.bottom();

    m_data->sizeHintCache = QSize(w, h);
    return m_data->sizeHintCache;
}

/**
 * \if ENGLISH
 * @brief Return bounding rectangle of the slider handle
 * \sa sliderRect()
 * \endif
 * \if CHINESE
 * @brief 返回滑块手柄的边界矩形
 * \sa sliderRect()
 * \endif
 */
QRect QwtSlider::handleRect() const
{
    if (!isValid())
        return QRect();

    const int markerPos = transform(value());

    QPoint center = m_data->sliderRect.center();
    if (m_data->orientation == Qt::Horizontal)
        center.setX(markerPos);
    else
        center.setY(markerPos);

    QRect rect;
    rect.setSize(qwtHandleSize(m_data->handleSize, m_data->orientation, m_data->hasTrough));
    rect.moveCenter(center);

    return rect;
}

/**
 * \if ENGLISH
 * @brief Return bounding rectangle of the slider - without the scale
 * \sa handleRect()
 * \endif
 * \if CHINESE
 * @brief 返回滑块的边界矩形 - 不含刻度
 * \sa handleRect()
 * \endif
 */
QRect QwtSlider::sliderRect() const
{
    return m_data->sliderRect;
}
