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

#include "qwt_plot_axis_zoomer.h"
#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_interval.h"
#include "qwt_picker_machine.h"

#include <qstack.h>

static QwtInterval qwtExpandedZoomInterval(double v1, double v2, double minRange, const QwtTransform* transform)
{
    double min = v1;
    double max = v2;

    if (max - min < minRange) {
        min = 0.5 * (min + max - minRange);
        max = min + minRange;

        if (transform) {
            // f.e the logarithmic scale doesn't allow values
            // outside [QwtLogTransform::LogMin/QwtLogTransform::LogMax]

            double minBounded = transform->bounded(min);
            double maxBounded = transform->bounded(max);

            if (minBounded != min) {
                maxBounded = transform->bounded(minBounded + minRange);
            } else if (maxBounded != max) {
                minBounded = transform->bounded(maxBounded - minRange);
            }

            min = minBounded;
            max = maxBounded;
        }
    }

    return QwtInterval(min, max);
}

static QRectF qwtExpandedZoomRect(const QRectF& zoomRect,
                                  const QSizeF& minSize,
                                  const QwtTransform* transformX,
                                  const QwtTransform* transformY)
{
    QRectF r = zoomRect;

    if (minSize.width() > r.width()) {
        const QwtInterval intv = qwtExpandedZoomInterval(r.left(), r.right(), minSize.width(), transformX);

        r.setLeft(intv.minValue());
        r.setRight(intv.maxValue());
    }

    if (minSize.height() > r.height()) {
        const QwtInterval intv = qwtExpandedZoomInterval(zoomRect.top(), zoomRect.bottom(), minSize.height(), transformY);

        r.setTop(intv.minValue());
        r.setBottom(intv.maxValue());
    }

    return r;
}

class QwtPlotAxisZoomer::PrivateData
{
public:
    uint zoomRectIndex;
    QStack< QRectF > zoomStack;

    int maxStackDepth;
};

/**
 * \if ENGLISH
 * @brief Creates a zoomer for a plot canvas.
 * @details The zoomer is set to those x- and y-axis of the parent plot of the
 *          canvas that are enabled. If both or no x-axis are enabled, the picker
 *          is set to QwtAxis::XBottom. If both or no y-axis are
 *          enabled, it is set to QwtAxis::YLeft.
 * 
 *          The zoomer is initialized with a QwtPickerDragRectMachine,
 *          the tracker mode is set to QwtPicker::ActiveOnly and the rubber band
 *          is set to QwtPicker::RectRubberBand.
 * @param[in] canvas Plot canvas to observe, also the parent object.
 * @param[in] doReplot Call QwtPlot::replot() for the attached plot before initializing
 *                     the zoomer with its scales. This might be necessary,
 *                     when the plot is in a state with pending scale changes.
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), setZoomBase()
 * \endif
 * 
 * \if CHINESE
 * @brief 为绘图画布创建缩放器。
 * @details 缩放器被设置为画布父绘图中启用的 x 轴和 y 轴。
 *          如果两个 x 轴都启用或都未启用，拾取器设置为 QwtAxis::XBottom。
 *          如果两个 y 轴都启用或都未启用，则设置为 QwtAxis::YLeft。
 * 
 *          缩放器使用 QwtPickerDragRectMachine 初始化，
 *          追踪模式设置为 QwtPicker::ActiveOnly，橡皮筋设置为 QwtPicker::RectRubberBand。
 * @param[in] canvas 要观察的绘图画布，也是父对象。
 * @param[in] doReplot 是否在初始化缩放器之前调用 QwtPlot::replot()。
 *                     当绘图处于有待处理的刻度更改状态时，这可能是必要的。
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), setZoomBase()
 * \endif
 */
QwtPlotAxisZoomer::QwtPlotAxisZoomer(QWidget* canvas, bool doReplot) : QwtPlotPicker(canvas)
{
    if (canvas)
        init(doReplot);
}

/**
 * \if ENGLISH
 * @brief Creates a zoomer for a plot canvas with specified axes.
 * @details The zoomer is initialized with a QwtPickerDragRectMachine,
 *          the tracker mode is set to QwtPicker::ActiveOnly and the rubber band
 *          is set to QwtPicker::RectRubberBand.
 * @param[in] xAxisId X axis of the zoomer.
 * @param[in] yAxisId Y axis of the zoomer.
 * @param[in] canvas Plot canvas to observe, also the parent object.
 * @param[in] doReplot Call QwtPlot::replot() for the attached plot before initializing
 *                     the zoomer with its scales. This might be necessary,
 *                     when the plot is in a state with pending scale changes.
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), setZoomBase()
 * \endif
 * 
 * \if CHINESE
 * @brief 使用指定的坐标轴为绘图画布创建缩放器。
 * @details 缩放器使用 QwtPickerDragRectMachine 初始化，
 *          追踪模式设置为 QwtPicker::ActiveOnly，橡皮筋设置为 QwtPicker::RectRubberBand。
 * @param[in] xAxisId 缩放器的 X 轴。
 * @param[in] yAxisId 缩放器的 Y 轴。
 * @param[in] canvas 要观察的绘图画布，也是父对象。
 * @param[in] doReplot 是否在初始化缩放器之前调用 QwtPlot::replot()。
 *                     当绘图处于有待处理的刻度更改状态时，这可能是必要的。
 * @sa QwtPlot::autoReplot(), QwtPlot::replot(), setZoomBase()
 * \endif
 */

QwtPlotAxisZoomer::QwtPlotAxisZoomer(QwtAxisId xAxisId, QwtAxisId yAxisId, QWidget* canvas, bool doReplot)
    : QwtPlotPicker(xAxisId, yAxisId, canvas)
{
    if (canvas)
        init(doReplot);
}

//! Init the zoomer, used by the constructors
void QwtPlotAxisZoomer::init(bool doReplot)
{
    m_data = new PrivateData;

    m_data->maxStackDepth = -1;

    setTrackerMode(ActiveOnly);
    setRubberBand(RectRubberBand);
    setStateMachine(new QwtPickerDragRectMachine());

    if (doReplot && plot())
        plot()->replot();

    setZoomBase(scaleRect());
}

QwtPlotAxisZoomer::~QwtPlotAxisZoomer()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Limits the number of recursive zoom operations to depth.
 * @details A value of -1 sets the depth to unlimited, 0 disables zooming.
 *          If the current zoom rectangle is below depth, the plot is unzoomed.
 * @param[in] depth Maximum for the stack depth.
 * @sa maxStackDepth()
 * @note depth doesn't include the zoom base, so zoomStack().count() might be
 *       maxStackDepth() + 1.
 * \endif
 * 
 * \if CHINESE
 * @brief 限制递归缩放操作的数量。
 * @details 值为 -1 表示深度无限制，0 表示禁用缩放。
 *          如果当前缩放矩形低于深度，则取消缩放。
 * @param[in] depth 堆栈的最大深度。
 * @sa maxStackDepth()
 * @note depth 不包括缩放基准，因此 zoomStack().count() 可能是 maxStackDepth() + 1。
 * \endif
 */
void QwtPlotAxisZoomer::setMaxStackDepth(int depth)
{
    m_data->maxStackDepth = depth;

    if (depth >= 0) {
        // unzoom if the current depth is below m_data->maxStackDepth

        const int zoomOut = m_data->zoomStack.count() - 1 - depth;  // -1 for the zoom base

        if (zoomOut > 0) {
            zoom(-zoomOut);
            for (int i = m_data->zoomStack.count() - 1; i > int(m_data->zoomRectIndex); i--) {
                (void)m_data->zoomStack.pop();  // remove trailing rects
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Returns the maximal depth of the zoom stack.
 * @return Maximal depth of the zoom stack.
 * @sa setMaxStackDepth()
 * \endif
 * 
 * \if CHINESE
 * @brief 返回缩放堆栈的最大深度。
 * @return 缩放堆栈的最大深度。
 * @sa setMaxStackDepth()
 * \endif
 */
int QwtPlotAxisZoomer::maxStackDepth() const
{
    return m_data->maxStackDepth;
}

/**
 * \if ENGLISH
 * @brief Returns the zoom stack.
 * @return The zoom stack. zoomStack()[0] is the zoom base,
 *         zoomStack()[1] the first zoomed rectangle.
 * @sa setZoomStack(), zoomRectIndex()
 * \endif
 * 
 * \if CHINESE
 * @brief 返回缩放堆栈。
 * @return 缩放堆栈。zoomStack()[0] 是缩放基准，
 *         zoomStack()[1] 是第一个缩放的矩形。
 * @sa setZoomStack(), zoomRectIndex()
 * \endif
 */
const QStack< QRectF >& QwtPlotAxisZoomer::zoomStack() const
{
    return m_data->zoomStack;
}

/**
 * \if ENGLISH
 * @brief Returns the initial rectangle of the zoomer.
 * @return Initial rectangle of the zoomer.
 * @sa setZoomBase(), zoomRect()
 * \endif
 * 
 * \if CHINESE
 * @brief 返回缩放器的初始矩形。
 * @return 缩放器的初始矩形。
 * @sa setZoomBase(), zoomRect()
 * \endif
 */
QRectF QwtPlotAxisZoomer::zoomBase() const
{
    return m_data->zoomStack[ 0 ];
}

/**
 * \if ENGLISH
 * @brief Reinitializes the zoom stack with scaleRect() as base.
 * @param[in] doReplot Call QwtPlot::replot() for the attached plot before initializing
 *                      the zoomer with its scales. This might be necessary,
 *                      when the plot is in a state with pending scale changes.
 * @sa zoomBase(), scaleRect(), QwtPlot::autoReplot(), QwtPlot::replot()
 * \endif
 * 
 * \if CHINESE
 * @brief 使用 scaleRect() 作为基准重新初始化缩放堆栈。
 * @param[in] doReplot 是否在初始化缩放器之前调用 QwtPlot::replot()。
 *                     当绘图处于有待处理的刻度更改状态时，这可能是必要的。
 * @sa zoomBase(), scaleRect(), QwtPlot::autoReplot(), QwtPlot::replot()
 * \endif
 */
void QwtPlotAxisZoomer::setZoomBase(bool doReplot)
{
    QwtPlot* plt = plot();
    if (plt == nullptr)
        return;

    if (doReplot)
        plt->replot();

    m_data->zoomStack.clear();
    m_data->zoomStack.push(scaleRect());
    m_data->zoomRectIndex = 0;

    rescale();
}

/**
 * \if ENGLISH
 * @brief Sets the initial size of the zoomer.
 * @details base is united with the current scaleRect() and the zoom stack is
 *          reinitialized with it as zoom base. plot is zoomed to scaleRect().
 * @param[in] base Zoom base rectangle.
 * @sa zoomBase(), scaleRect()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置缩放器的初始大小。
 * @details base 与当前 scaleRect() 合并，并以它作为缩放基准重新初始化缩放堆栈。
 *          绘图被缩放到 scaleRect()。
 * @param[in] base 缩放基准矩形。
 * @sa zoomBase(), scaleRect()
 * \endif
 */
void QwtPlotAxisZoomer::setZoomBase(const QRectF& base)
{
    const QwtPlot* plt = plot();
    if (!plt)
        return;

    const QRectF sRect = scaleRect();
    const QRectF bRect = base | sRect;

    m_data->zoomStack.clear();
    m_data->zoomStack.push(bRect);
    m_data->zoomRectIndex = 0;

    if (base != sRect) {
        m_data->zoomStack.push(sRect);
        m_data->zoomRectIndex++;
    }

    rescale();
}

/**
 * \if ENGLISH
 * @brief Returns the rectangle at the current position on the zoom stack.
 * @return Rectangle at the current position on the zoom stack.
 * @sa zoomRectIndex(), scaleRect()
 * \endif
 * 
 * \if CHINESE
 * @brief 返回缩放堆栈当前位置的矩形。
 * @return 缩放堆栈当前位置的矩形。
 * @sa zoomRectIndex(), scaleRect()
 * \endif
 */
QRectF QwtPlotAxisZoomer::zoomRect() const
{
    return m_data->zoomStack[ m_data->zoomRectIndex ];
}

/**
 * \if ENGLISH
 * @brief Returns the index of current position of zoom stack.
 * @return Index of current position of zoom stack.
 * \endif
 * 
 * \if CHINESE
 * @brief 返回缩放堆栈当前位置的索引。
 * @return 缩放堆栈当前位置的索引。
 * \endif
 */
uint QwtPlotAxisZoomer::zoomRectIndex() const
{
    return m_data->zoomRectIndex;
}

/**
 * \if ENGLISH
 * @brief Zooms in to a rectangle.
 * @details Clears all rectangles above the current position of the
 *          zoom stack and pushes the normalized rectangle on it.
 * @param[in] rect Rectangle to zoom to.
 * @note If the maximal stack depth is reached, zoom is ignored.
 * @note The zoomed signal is emitted.
 * \endif
 * 
 * \if CHINESE
 * @brief 放大到指定矩形。
 * @details 清除缩放堆栈当前位置上方的所有矩形，并将规范化后的矩形压入堆栈。
 * @param[in] rect 要缩放到的矩形。
 * @note 如果达到最大堆栈深度，缩放将被忽略。
 * @note 会发出 zoomed 信号。
 * \endif
 */

void QwtPlotAxisZoomer::zoom(const QRectF& rect)
{
    if (m_data->maxStackDepth >= 0 && int(m_data->zoomRectIndex) >= m_data->maxStackDepth) {
        return;
    }

    const QRectF zoomRect = rect.normalized();
    if (zoomRect != m_data->zoomStack[ m_data->zoomRectIndex ]) {
        for (uint i = m_data->zoomStack.count() - 1; i > m_data->zoomRectIndex; i--) {
            (void)m_data->zoomStack.pop();
        }

        m_data->zoomStack.push(zoomRect);
        m_data->zoomRectIndex++;

        rescale();

        Q_EMIT zoomed(zoomRect);
    }
}

/**
 * \if ENGLISH
 * @brief Zooms in or out by an offset.
 * @details Activates a rectangle on the zoom stack with an offset relative
 *          to the current position. Negative values of offset will zoom out,
 *          positive zoom in. A value of 0 zooms out to the zoom base.
 * @param[in] offset Offset relative to the current position of the zoom stack.
 * @note The zoomed signal is emitted.
 * @sa zoomRectIndex()
 * \endif
 * 
 * \if CHINESE
 * @brief 通过偏移量放大或缩小。
 * @details 激活缩放堆栈上相对于当前位置有偏移的矩形。
 *          偏移量为负值将缩小，正值将放大。值为 0 表示缩放到基准视图。
 * @param[in] offset 相对于缩放堆栈当前位置的偏移量。
 * @note 会发出 zoomed 信号。
 * @sa zoomRectIndex()
 * \endif
 */
void QwtPlotAxisZoomer::zoom(int offset)
{
    int newIndex;

    if (offset == 0) {
        newIndex = 0;
    } else {
        newIndex = m_data->zoomRectIndex + offset;
        newIndex = qBound(0, newIndex, m_data->zoomStack.count() - 1);
    }

    if (newIndex != static_cast< int >(m_data->zoomRectIndex)) {
        m_data->zoomRectIndex = newIndex;
        rescale();
        Q_EMIT zoomed(zoomRect());
    }
}

/**
 * \if ENGLISH
 * @brief Assigns a zoom stack.
 * @details In combination with other types of navigation it might be useful to
 *          modify to manipulate the complete zoom stack.
 * @param[in] zoomStack New zoom stack.
 * @param[in] zoomRectIndex Index of the current position of zoom stack.
 *                          In case of -1 the current position is at the top
 *                          of the stack.
 * @note The zoomed signal might be emitted.
 * @sa zoomStack(), zoomRectIndex()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置缩放堆栈。
 * @details 与其他类型的导航结合使用时，操作完整的缩放堆栈可能很有用。
 * @param[in] zoomStack 新的缩放堆栈。
 * @param[in] zoomRectIndex 缩放堆栈当前位置的索引。
 *                          如果为 -1，当前位置在堆栈顶部。
 * @note 可能会发出 zoomed 信号。
 * @sa zoomStack(), zoomRectIndex()
 * \endif
 */
void QwtPlotAxisZoomer::setZoomStack(const QStack< QRectF >& zoomStack, int zoomRectIndex)
{
    if (zoomStack.isEmpty())
        return;

    if (m_data->maxStackDepth >= 0 && zoomStack.count() > m_data->maxStackDepth) {
        return;
    }

    if (zoomRectIndex < 0 || zoomRectIndex > zoomStack.count())
        zoomRectIndex = zoomStack.count() - 1;

    const bool doRescale = zoomStack[ zoomRectIndex ] != zoomRect();

    m_data->zoomStack     = zoomStack;
    m_data->zoomRectIndex = uint(zoomRectIndex);

    if (doRescale) {
        rescale();
        Q_EMIT zoomed(zoomRect());
    }
}

/*!
   Adjust the observed plot to zoomRect()

   \note Initiates QwtPlot::replot()
 */

void QwtPlotAxisZoomer::rescale()
{
    QwtPlot* plt = plot();
    if (!plt)
        return;

    const QRectF& rect = m_data->zoomStack[ m_data->zoomRectIndex ];
    if (rect != scaleRect()) {
        plt->saveAutoReplotState();
        plt->setAutoReplot(false);

        double x1 = rect.left();
        double x2 = rect.right();
        if (!plt->axisScaleDiv(xAxis()).isIncreasing())
            qSwap(x1, x2);

        plt->setAxisScale(xAxis(), x1, x2);

        double y1 = rect.top();
        double y2 = rect.bottom();
        if (!plt->axisScaleDiv(yAxis()).isIncreasing())
            qSwap(y1, y2);

        plt->setAxisScale(yAxis(), y1, y2);

        plt->restoreAutoReplotState();

        plt->replot();
    }
}

/**
 * \if ENGLISH
 * @brief Reinitializes the axes, and sets the zoom base to their scales.
 * @param[in] xAxisId X axis.
 * @param[in] yAxisId Y axis.
 * \endif
 * 
 * \if CHINESE
 * @brief 重新初始化坐标轴，并将缩放基准设置为它们的刻度。
 * @param[in] xAxisId X 轴。
 * @param[in] yAxisId Y 轴。
 * \endif
 */

void QwtPlotAxisZoomer::setAxes(QwtAxisId xAxisId, QwtAxisId yAxisId)
{
    if (xAxisId != QwtPlotPicker::xAxis() || yAxisId != QwtPlotPicker::yAxis()) {
        QwtPlotPicker::setAxes(xAxisId, yAxisId);
        setZoomBase(scaleRect());
    }
}

/*!
   Qt::MidButton zooms out one position on the zoom stack,
   Qt::RightButton to the zoom base.

   Changes the current position on the stack, but doesn't pop
   any rectangle.

   \note The mouse events can be changed, using
         QwtEventPattern::setMousePattern: 2, 1
 */
void QwtPlotAxisZoomer::widgetMouseReleaseEvent(QMouseEvent* me)
{
    if (mouseMatch(MouseSelect2, me))
        zoom(0);
    else if (mouseMatch(MouseSelect3, me))
        zoom(-1);
    else if (mouseMatch(MouseSelect6, me))
        zoom(+1);
    else
        QwtPlotPicker::widgetMouseReleaseEvent(me);
}

/*!
   Qt::Key_Plus zooms in, Qt::Key_Minus zooms out one position on the
   zoom stack, Qt::Key_Escape zooms out to the zoom base.

   Changes the current position on the stack, but doesn't pop
   any rectangle.

   \note The keys codes can be changed, using
         QwtEventPattern::setKeyPattern: 3, 4, 5
 */

void QwtPlotAxisZoomer::widgetKeyPressEvent(QKeyEvent* ke)
{
    if (!isActive()) {
        if (keyMatch(KeyUndo, ke))  // Qt::Key_Minus
            zoom(-1);
        else if (keyMatch(KeyRedo, ke))  // Qt::Key_Plus
            zoom(+1);
        else if (keyMatch(KeyHome, ke))  // Qt::Key_Escape
            zoom(0);
    }

    QwtPlotPicker::widgetKeyPressEvent(ke);
}

/**
 * \if ENGLISH
 * @brief Moves the current zoom rectangle by an offset.
 * @param[in] dx X offset.
 * @param[in] dy Y offset.
 * @note The changed rectangle is limited by the zoom base.
 * \endif
 * 
 * \if CHINESE
 * @brief 通过偏移量移动当前缩放矩形。
 * @param[in] dx X 方向偏移量。
 * @param[in] dy Y 方向偏移量。
 * @note 更改后的矩形受缩放基准限制。
 * \endif
 */
void QwtPlotAxisZoomer::moveBy(double dx, double dy)
{
    const QRectF& rect = m_data->zoomStack[ m_data->zoomRectIndex ];
    moveTo(QPointF(rect.left() + dx, rect.top() + dy));
}

/**
 * \if ENGLISH
 * @brief Moves the current zoom rectangle.
 * @param[in] pos New position.
 * @sa QRectF::moveTo()
 * @note The changed rectangle is limited by the zoom base.
 * \endif
 * 
 * \if CHINESE
 * @brief 移动当前缩放矩形。
 * @param[in] pos 新位置。
 * @sa QRectF::moveTo()
 * @note 更改后的矩形受缩放基准限制。
 * \endif
 */
void QwtPlotAxisZoomer::moveTo(const QPointF& pos)
{
    double x = pos.x();
    double y = pos.y();

    if (x < zoomBase().left())
        x = zoomBase().left();
    if (x > zoomBase().right() - zoomRect().width())
        x = zoomBase().right() - zoomRect().width();

    if (y < zoomBase().top())
        y = zoomBase().top();
    if (y > zoomBase().bottom() - zoomRect().height())
        y = zoomBase().bottom() - zoomRect().height();

    if (x != zoomRect().left() || y != zoomRect().top()) {
        m_data->zoomStack[ m_data->zoomRectIndex ].moveTo(x, y);
        rescale();
    }
}

/*!
   \brief Check and correct a selected rectangle

   Reject rectangles with a height or width < 2, otherwise
   expand the selected rectangle to a minimum size of 11x11
   and accept it.

   \return true If the rectangle is accepted, or has been changed
          to an accepted one.
 */

bool QwtPlotAxisZoomer::accept(QPolygon& pa) const
{
    if (pa.count() < 2)
        return false;

    QRect rect = QRect(pa.first(), pa.last());
    rect       = rect.normalized();

    const int minSize = 2;
    if (rect.width() < minSize && rect.height() < minSize)
        return false;

    const int minZoomSize = 11;

    const QPoint center = rect.center();
    rect.setSize(rect.size().expandedTo(QSize(minZoomSize, minZoomSize)));
    rect.moveCenter(center);

    pa.resize(2);
    pa[ 0 ] = rect.topLeft();
    pa[ 1 ] = rect.bottomRight();

    return true;
}

/*!
   \brief Limit zooming by a minimum rectangle

   \return zoomBase().width() / 10e4, zoomBase().height() / 10e4
 */
QSizeF QwtPlotAxisZoomer::minZoomSize() const
{
    return QSizeF(m_data->zoomStack[ 0 ].width() / 10e4, m_data->zoomStack[ 0 ].height() / 10e4);
}

/*!
   Rejects selections, when the stack depth is too deep, or
   the zoomed rectangle is minZoomSize().

   \sa minZoomSize(), maxStackDepth()
 */
void QwtPlotAxisZoomer::begin()
{
    if (m_data->maxStackDepth >= 0) {
        if (m_data->zoomRectIndex >= uint(m_data->maxStackDepth))
            return;
    }

    const QSizeF minSize = minZoomSize();
    if (minSize.isValid()) {
        const QSizeF sz = m_data->zoomStack[ m_data->zoomRectIndex ].size() * 0.9999;

        if (minSize.width() >= sz.width() && minSize.height() >= sz.height()) {
            return;
        }
    }

    QwtPlotPicker::begin();
}

/*!
   Expand the selected rectangle to minZoomSize() and zoom in
   if accepted.

   \param ok If true, complete the selection and emit selected signals
            otherwise discard the selection.

   \sa accept(), minZoomSize()
   \return True if the selection has been accepted, false otherwise
 */
bool QwtPlotAxisZoomer::end(bool ok)
{
    ok = QwtPlotPicker::end(ok);
    if (!ok)
        return false;

    QwtPlot* plot = QwtPlotAxisZoomer::plot();
    if (!plot)
        return false;

    const QPolygon& pa = selection();
    if (pa.count() < 2)
        return false;

    QRect rect = QRect(pa.first(), pa.last());
    rect       = rect.normalized();

    const QwtScaleMap xMap = plot->canvasMap(xAxis());
    const QwtScaleMap yMap = plot->canvasMap(yAxis());

    QRectF zoomRect = QwtScaleMap::invTransform(xMap, yMap, rect).normalized();

    zoomRect = qwtExpandedZoomRect(zoomRect, minZoomSize(), xMap.transformation(), yMap.transformation());

    zoom(zoomRect);

    return true;
}
