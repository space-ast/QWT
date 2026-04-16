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

#include "qwt_plot_rescaler.h"
#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include "qwt_interval.h"
#include "qwt_plot_canvas.h"

#include <qevent.h>

class QwtPlotRescaler::AxisData
{
public:
    AxisData() : aspectRatio(1.0), expandingDirection(QwtPlotRescaler::ExpandUp)
    {
    }

    double aspectRatio;
    QwtInterval intervalHint;
    QwtPlotRescaler::ExpandingDirection expandingDirection;
    mutable QwtScaleDiv scaleDiv;
};

class QwtPlotRescaler::PrivateData
{
public:
    PrivateData()
        : referenceAxis(QwtAxis::XBottom), rescalePolicy(QwtPlotRescaler::Expanding), isEnabled(false), inReplot(0)
    {
    }

    QwtPlotRescaler::AxisData* axisData(QwtAxisId axisId)
    {
        if (!QwtAxis::isValid(axisId))
            return nullptr;

        return &m_axisData[ axisId ];
    }

    QwtAxisId referenceAxis;
    RescalePolicy rescalePolicy;
    bool isEnabled;

    mutable int inReplot;

private:
    QwtPlotRescaler::AxisData m_axisData[ QwtAxis::AxisPositions ];
};

/*!
   \if ENGLISH
   @brief Constructs a rescaler for the given canvas
   @param[in] canvas Canvas widget
   @param[in] referenceAxis Reference axis, see RescalePolicy
   @param[in] policy Rescale policy
   @sa setRescalePolicy(), setReferenceAxis()
   \endif

   \if CHINESE
   @brief 为给定的画布构造重缩放器
   @param[in] canvas 画布控件
   @param[in] referenceAxis 参考轴，参见 RescalePolicy
   @param[in] policy 重缩放策略
   @sa setRescalePolicy(), setReferenceAxis()
   \endif
 */
QwtPlotRescaler::QwtPlotRescaler(QWidget* canvas, QwtAxisId referenceAxis, RescalePolicy policy) : QObject(canvas)
{
    m_data                = new PrivateData;
    m_data->referenceAxis = referenceAxis;
    m_data->rescalePolicy = policy;

    setEnabled(true);
}

/*!
   \if ENGLISH
   @brief Destructs the rescaler
   \endif

   \if CHINESE
   @brief 析构重缩放器
   \endif
 */
QwtPlotRescaler::~QwtPlotRescaler()
{
    delete m_data;
}

/*!
   \if ENGLISH
   @brief Enables or disables the rescaler
   @details When enabled is true, an event filter is installed for the canvas,
            otherwise the event filter is removed.
   @param[in] on true to enable, false to disable
   @sa isEnabled(), eventFilter()
   \endif

   \if CHINESE
   @brief 启用或禁用重缩放器
   @details 当 enabled 为 true 时，为画布安装事件过滤器，否则移除事件过滤器。
   @param[in] on true 启用，false 禁用
   @sa isEnabled(), eventFilter()
   \endif
 */
void QwtPlotRescaler::setEnabled(bool on)
{
    if (m_data->isEnabled != on) {
        m_data->isEnabled = on;

        QWidget* w = canvas();
        if (w) {
            if (m_data->isEnabled)
                w->installEventFilter(this);
            else
                w->removeEventFilter(this);
        }
    }
}

/*!
   \if ENGLISH
   @brief Checks if the rescaler is enabled
   @return true if enabled, false otherwise
   @sa setEnabled(), eventFilter()
   \endif

   \if CHINESE
   @brief 检查重缩放器是否启用
   @return 如果启用则返回 true，否则返回 false
   @sa setEnabled(), eventFilter()
   \endif
 */
bool QwtPlotRescaler::isEnabled() const
{
    return m_data->isEnabled;
}

/*!
   \if ENGLISH
   @brief Sets the rescale policy
   @param[in] policy Rescale policy
   @sa rescalePolicy()
   \endif

   \if CHINESE
   @brief 设置重缩放策略
   @param[in] policy 重缩放策略
   @sa rescalePolicy()
   \endif
 */
void QwtPlotRescaler::setRescalePolicy(RescalePolicy policy)
{
    m_data->rescalePolicy = policy;
}

/*!
   \if ENGLISH
   @brief Gets the rescale policy
   @return Rescale policy
   @sa setRescalePolicy()
   \endif

   \if CHINESE
   @brief 获取重缩放策略
   @return 重缩放策略
   @sa setRescalePolicy()
   \endif
 */
QwtPlotRescaler::RescalePolicy QwtPlotRescaler::rescalePolicy() const
{
    return m_data->rescalePolicy;
}

/*!
   \if ENGLISH
   @brief Sets the reference axis
   @details The reference axis is used as the basis for rescale calculations.
   @param[in] axisId Axis identifier
   @sa referenceAxis()
   \endif

   \if CHINESE
   @brief 设置参考轴
   @details 参考轴用作重缩放计算的基础。
   @param[in] axisId 轴标识符
   @sa referenceAxis()
   \endif
 */
void QwtPlotRescaler::setReferenceAxis(QwtAxisId axisId)
{
    m_data->referenceAxis = axisId;
}

/*!
   \if ENGLISH
   @brief Gets the reference axis
   @return Reference axis identifier
   @sa setReferenceAxis()
   \endif

   \if CHINESE
   @brief 获取参考轴
   @return 参考轴标识符
   @sa setReferenceAxis()
   \endif
 */
QwtAxisId QwtPlotRescaler::referenceAxis() const
{
    return m_data->referenceAxis;
}

/*!
   \if ENGLISH
   @brief Sets the expanding direction for all axes
   @param[in] direction Expanding direction
   @sa expandingDirection()
   \endif

   \if CHINESE
   @brief 设置所有轴的扩展方向
   @param[in] direction 扩展方向
   @sa expandingDirection()
   \endif
 */
void QwtPlotRescaler::setExpandingDirection(ExpandingDirection direction)
{
    for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
        setExpandingDirection(axis, direction);
}

/*!
   \if ENGLISH
   @brief Sets the expanding direction for a specific axis
   @param[in] axisId Axis identifier
   @param[in] direction Expanding direction
   @sa expandingDirection()
   \endif

   \if CHINESE
   @brief 设置指定轴的扩展方向
   @param[in] axisId 轴标识符
   @param[in] direction 扩展方向
   @sa expandingDirection()
   \endif
 */
void QwtPlotRescaler::setExpandingDirection(QwtAxisId axisId, ExpandingDirection direction)
{
    if (AxisData* axisData = m_data->axisData(axisId))
        axisData->expandingDirection = direction;
}

/*!
   \if ENGLISH
   @brief Gets the expanding direction for a specific axis
   @param[in] axisId Axis identifier
   @return Expanding direction for the specified axis
   @sa setExpandingDirection()
   \endif

   \if CHINESE
   @brief 获取指定轴的扩展方向
   @param[in] axisId 轴标识符
   @return 指定轴的扩展方向
   @sa setExpandingDirection()
   \endif
 */
QwtPlotRescaler::ExpandingDirection QwtPlotRescaler::expandingDirection(QwtAxisId axisId) const
{
    if (const AxisData* axisData = m_data->axisData(axisId))
        return axisData->expandingDirection;

    return ExpandBoth;
}

/*!
   \if ENGLISH
   @brief Sets the aspect ratio for all axes
   @details Sets the aspect ratio between the scale of the reference axis
            and other scales. The default ratio is 1.0.
   @param[in] ratio Aspect ratio
   @sa aspectRatio()
   \endif

   \if CHINESE
   @brief 设置所有轴的宽高比
   @details 设置参考轴与其他轴之间的宽高比。默认比值为 1.0。
   @param[in] ratio 宽高比
   @sa aspectRatio()
   \endif
 */
void QwtPlotRescaler::setAspectRatio(double ratio)
{
    for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
        setAspectRatio(axis, ratio);
}

/*!
   \if ENGLISH
   @brief Sets the aspect ratio for a specific axis
   @details Sets the aspect ratio between the scale of the reference axis
            and another scale. The default ratio is 1.0.
   @param[in] axisId Axis identifier
   @param[in] ratio Aspect ratio
   @sa aspectRatio()
   \endif

   \if CHINESE
   @brief 设置指定轴的宽高比
   @details 设置参考轴与指定轴之间的宽高比。默认比值为 1.0。
   @param[in] axisId 轴标识符
   @param[in] ratio 宽高比
   @sa aspectRatio()
   \endif
 */
void QwtPlotRescaler::setAspectRatio(QwtAxisId axisId, double ratio)
{
    if (AxisData* axisData = m_data->axisData(axisId)) {
        if (ratio < 0.0)
            ratio = 0.0;

        axisData->aspectRatio = ratio;
    }
}

/*!
   \if ENGLISH
   @brief Gets the aspect ratio for a specific axis
   @param[in] axisId Axis identifier
   @return Aspect ratio between the specified axis and the reference axis
   @sa setAspectRatio()
   \endif

   \if CHINESE
   @brief 获取指定轴的宽高比
   @param[in] axisId 轴标识符
   @return 指定轴与参考轴之间的宽高比
   @sa setAspectRatio()
   \endif
 */
double QwtPlotRescaler::aspectRatio(QwtAxisId axisId) const
{
    if (AxisData* axisData = m_data->axisData(axisId))
        return axisData->aspectRatio;

    return 0.0;
}

/*!
   \if ENGLISH
   @brief Sets an interval hint for an axis
   @details In Fitting mode, the hint is used as the minimal interval
            that always needs to be displayed.
   @param[in] axisId Axis identifier
   @param[in] interval Interval hint
   @sa intervalHint(), RescalePolicy
   \endif

   \if CHINESE
   @brief 设置轴的区间提示
   @details 在 Fitting 模式下，提示用作始终需要显示的最小区间。
   @param[in] axisId 轴标识符
   @param[in] interval 区间提示
   @sa intervalHint(), RescalePolicy
   \endif
 */
void QwtPlotRescaler::setIntervalHint(QwtAxisId axisId, const QwtInterval& interval)
{
    if (AxisData* axisData = m_data->axisData(axisId))
        axisData->intervalHint = interval;
}

/*!
   \if ENGLISH
   @brief Gets the interval hint for an axis
   @param[in] axisId Axis identifier
   @return Interval hint
   @sa setIntervalHint(), RescalePolicy
   \endif

   \if CHINESE
   @brief 获取轴的区间提示
   @param[in] axisId 轴标识符
   @return 区间提示
   @sa setIntervalHint(), RescalePolicy
   \endif
 */
QwtInterval QwtPlotRescaler::intervalHint(QwtAxisId axisId) const
{
    if (AxisData* axisData = m_data->axisData(axisId))
        return axisData->intervalHint;

    return QwtInterval();
}

/*!
   \if ENGLISH
   @brief Gets the canvas widget
   @return Canvas widget
   \endif

   \if CHINESE
   @brief 获取画布控件
   @return 画布控件
   \endif
 */
QWidget* QwtPlotRescaler::canvas()
{
    return qobject_cast< QWidget* >(parent());
}

/*!
   \if ENGLISH
   @brief Gets the canvas widget (const version)
   @return Canvas widget (const)
   \endif

   \if CHINESE
   @brief 获取画布控件（常量版本）
   @return 画布控件（常量）
   \endif
 */
const QWidget* QwtPlotRescaler::canvas() const
{
    return qobject_cast< const QWidget* >(parent());
}

/*!
   \if ENGLISH
   @brief Gets the plot widget
   @return Plot widget
   \endif

   \if CHINESE
   @brief 获取绘图控件
   @return 绘图控件
   \endif
 */
QwtPlot* QwtPlotRescaler::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/*!
   \if ENGLISH
   @brief Gets the plot widget (const version)
   @return Plot widget (const)
   \endif

   \if CHINESE
   @brief 获取绘图控件（常量版本）
   @return 绘图控件（常量）
   \endif
 */
const QwtPlot* QwtPlotRescaler::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
   \if ENGLISH
   @brief Event filter for the plot canvas
   @details Handles resize and polish request events for the canvas.
   @param[in] object Object that received the event
   @param[in] event Event that was received
   @return false to allow further processing of the event
   \endif

   \if CHINESE
   @brief 画布事件过滤器
   @details 处理画布的调整大小和整理请求事件。
   @param[in] object 接收事件的对象
   @param[in] event 接收到的事件
   @return false 以允许事件的进一步处理
   \endif
 */
bool QwtPlotRescaler::eventFilter(QObject* object, QEvent* event)
{
    if (object && object == canvas()) {
        switch (event->type()) {
        case QEvent::Resize: {
            canvasResizeEvent(static_cast< QResizeEvent* >(event));
            break;
        }
        case QEvent::PolishRequest: {
            rescale();
            break;
        }
        default:;
        }
    }

    return false;
}

/*!
   Event handler for resize events of the plot canvas

   \param event Resize event
   \sa rescale()
 */
void QwtPlotRescaler::canvasResizeEvent(QResizeEvent* event)
{
    const QMargins m = canvas()->contentsMargins();
    const QSize marginSize(m.left() + m.right(), m.top() + m.bottom());

    const QSize newSize = event->size() - marginSize;
    const QSize oldSize = event->oldSize() - marginSize;

    rescale(oldSize, newSize);
}

/*!
   \if ENGLISH
   @brief Rescales the plot axes
   @details Adjusts the plot axes scales based on the current canvas size.
   \endif

   \if CHINESE
   @brief 重缩放绘图轴
   @details 根据当前画布大小调整绘图轴的刻度。
   \endif
 */
void QwtPlotRescaler::rescale() const
{
    const QSize size = canvas()->contentsRect().size();
    rescale(size, size);
}

/*!
   Adjust the plot axes scales

   \param oldSize Previous size of the canvas
   \param newSize New size of the canvas
 */
void QwtPlotRescaler::rescale(const QSize& oldSize, const QSize& newSize) const
{
    if (newSize.isEmpty())
        return;

    QwtInterval intervals[ QwtAxis::AxisPositions ];
    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        const QwtAxisId axisId(axisPos);
        intervals[ axisPos ] = interval(axisId);
    }

    const QwtAxisId refAxis = referenceAxis();
    intervals[ refAxis ]    = expandScale(refAxis, oldSize, newSize);

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        const QwtAxisId axisId(axisPos);
        if (aspectRatio(axisId) > 0.0 && axisId != refAxis) {
            intervals[ axisPos ] = syncScale(axisId, intervals[ refAxis ], newSize);
        }
    }

    updateScales(intervals);
}

/*!
   Calculate the new scale interval of a plot axis

   \param axisId Axis
   \param oldSize Previous size of the canvas
   \param newSize New size of the canvas

   \return Calculated new interval for the axis
 */
QwtInterval QwtPlotRescaler::expandScale(QwtAxisId axisId, const QSize& oldSize, const QSize& newSize) const
{
    const QwtInterval oldInterval = interval(axisId);

    QwtInterval expanded = oldInterval;
    switch (rescalePolicy()) {
    case Fixed: {
        break;  // do nothing
    }
    case Expanding: {
        if (!oldSize.isEmpty()) {
            double width = oldInterval.width();
            if (orientation(axisId) == Qt::Horizontal)
                width *= double(newSize.width()) / oldSize.width();
            else
                width *= double(newSize.height()) / oldSize.height();

            expanded = expandInterval(oldInterval, width, expandingDirection(axisId));
        }
        break;
    }
    case Fitting: {
        double dist = 0.0;
        for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
            const QwtAxisId axisId(axisPos);
            const double d = pixelDist(axisId, newSize);
            if (d > dist)
                dist = d;
        }
        if (dist > 0.0) {
            double width;
            if (orientation(axisId) == Qt::Horizontal)
                width = newSize.width() * dist;
            else
                width = newSize.height() * dist;

            expanded = expandInterval(intervalHint(axisId), width, expandingDirection(axisId));
        }
        break;
    }
    }

    return expanded;
}

/*!
   Synchronize an axis scale according to the scale of the reference axis

   \param axisId Axis
   \param reference Interval of the reference axis
   \param size Size of the canvas

   \return New interval for axis
 */
QwtInterval QwtPlotRescaler::syncScale(QwtAxisId axisId, const QwtInterval& reference, const QSize& size) const
{
    double dist;
    if (orientation(referenceAxis()) == Qt::Horizontal)
        dist = reference.width() / size.width();
    else
        dist = reference.width() / size.height();

    if (orientation(axisId) == Qt::Horizontal)
        dist *= size.width();
    else
        dist *= size.height();

    dist /= aspectRatio(axisId);

    QwtInterval intv;
    if (rescalePolicy() == Fitting)
        intv = intervalHint(axisId);
    else
        intv = interval(axisId);

    intv = expandInterval(intv, dist, expandingDirection(axisId));

    return intv;
}

/*!
   \return Orientation of an axis
   \param axisId Axis
 */
Qt::Orientation QwtPlotRescaler::orientation(QwtAxisId axisId) const
{
    return QwtAxis::isYAxis(axisId) ? Qt::Vertical : Qt::Horizontal;
}

/*!
   \param axisId Axis
   \return Normalized interval of an axis
 */
QwtInterval QwtPlotRescaler::interval(QwtAxisId axisId) const
{
    if (!plot()->isAxisValid(axisId))
        return QwtInterval();

    return plot()->axisScaleDiv(axisId).interval().normalized();
}

/*!
   Expand the interval

   \param interval Interval to be expanded
   \param width Distance to be added to the interval
   \param direction Direction of the expand operation

   \return Expanded interval
 */
QwtInterval QwtPlotRescaler::expandInterval(const QwtInterval& interval, double width, ExpandingDirection direction) const
{
    QwtInterval expanded = interval;

    switch (direction) {
    case ExpandUp:
        expanded.setMinValue(interval.minValue());
        expanded.setMaxValue(interval.minValue() + width);
        break;

    case ExpandDown:
        expanded.setMaxValue(interval.maxValue());
        expanded.setMinValue(interval.maxValue() - width);
        break;

    case ExpandBoth:
    default:
        expanded.setMinValue(interval.minValue() + interval.width() / 2.0 - width / 2.0);
        expanded.setMaxValue(expanded.minValue() + width);
    }
    return expanded;
}

double QwtPlotRescaler::pixelDist(QwtAxisId axisId, const QSize& size) const
{
    const QwtInterval intv = intervalHint(axisId);

    double dist = 0.0;
    if (!intv.isNull()) {
        if (axisId == referenceAxis()) {
            dist = intv.width();
        } else {
            const double r = aspectRatio(axisId);
            if (r > 0.0)
                dist = intv.width() * r;
        }
    }

    if (dist > 0.0) {
        if (orientation(axisId) == Qt::Horizontal)
            dist /= size.width();
        else
            dist /= size.height();
    }

    return dist;
}

/*!
   Update the axes scales

   \param intervals Scale intervals
 */
void QwtPlotRescaler::updateScales(QwtInterval intervals[ QwtAxis::AxisPositions ]) const
{
    if (m_data->inReplot >= 5) {
        return;
    }

    QwtPlot* plt = const_cast< QwtPlot* >(plot());
    plt->saveAutoReplotState();
    plt->setAutoReplot(false);

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        {
            const QwtAxisId axisId(axisPos);

            if (axisId == referenceAxis() || aspectRatio(axisId) > 0.0) {
                double v1 = intervals[ axisPos ].minValue();
                double v2 = intervals[ axisPos ].maxValue();

                if (!plt->axisScaleDiv(axisId).isIncreasing())
                    qSwap(v1, v2);

                if (m_data->inReplot >= 1)
                    m_data->axisData(axisId)->scaleDiv = plt->axisScaleDiv(axisId);

                if (m_data->inReplot >= 2) {
                    QList< double > ticks[ QwtScaleDiv::NTickTypes ];
                    for (int t = 0; t < QwtScaleDiv::NTickTypes; t++)
                        ticks[ t ] = m_data->axisData(axisId)->scaleDiv.ticks(t);

                    plt->setAxisScaleDiv(axisId, QwtScaleDiv(v1, v2, ticks));
                } else {
                    plt->setAxisScale(axisId, v1, v2);
                }
            }
        }
    }

    QwtPlotCanvas* canvas = qobject_cast< QwtPlotCanvas* >(plt->canvas());

    bool immediatePaint = false;
    if (canvas) {
        immediatePaint = canvas->testPaintAttribute(QwtPlotCanvas::ImmediatePaint);
        canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);
    }

    plt->restoreAutoReplotState();

    m_data->inReplot++;
    plt->replot();
    m_data->inReplot--;

    if (canvas && immediatePaint) {
        canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
    }
}