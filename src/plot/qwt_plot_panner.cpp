/*******************************************************************************
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 ******************************************************************************/

#include "qwt_plot_panner.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_picker_machine.h"
#include "qwt_plot_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_transform.h"

#include <qevent.h>
#include <qpainter.h>

#include <QDebug>

class QwtPlotPanner::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotPanner)
public:
    PrivateData(QwtPlotPanner* p) : q_ptr(p), orientations(Qt::Vertical | Qt::Horizontal), initialPos(-1, -1)
    {
    }

    Qt::Orientations orientations;

    QPoint beginPos;    ///< Record position at begin event, not updated during move
    QPoint initialPos;  ///< Record position from last move
    QPoint currentPos;  ///< Current position, currentPos-initialPos=current offset
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] canvas The plot canvas widget to attach the panner to
 * @details Creates a panner attached to the specified canvas widget.
 *          The panner is automatically enabled and configured to use
 *          left mouse button for panning operations.
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] canvas 要附加平移器的绘图画布控件
 * @details 创建一个附加到指定画布控件的平移器。
 *          平移器自动启用并配置为使用鼠标左键进行平移操作。
 * \endif
 */
QwtPlotPanner::QwtPlotPanner(QWidget* canvas) : QwtPicker(canvas), QWT_PIMPL_CONSTRUCT
{
    if (canvas) {
        init();
    }
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
QwtPlotPanner::~QwtPlotPanner()
{
}

void QwtPlotPanner::init()
{
    setStateMachine(new QwtPickerDragPointMachine);
    setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);
    setTrackerMode(QwtPicker::AlwaysOff);
    setRubberBand(QwtPicker::NoRubberBand);
    setEnabled(true);
}

/**
 * \if ENGLISH
 * @brief Get the canvas widget
 * @return Pointer to the canvas widget, or nullptr if not attached
 * @details Returns the canvas widget that this panner is attached to.
 *          The canvas must inherit from QwtPlotCanvas.
 * \endif
 *
 * \if CHINESE
 * @brief 获取画布控件
 * @return 画布控件指针，如果未附加则返回 nullptr
 * @details 返回此平移器附加到的画布控件。
 *          画布必须继承自 QwtPlotCanvas。
 * \endif
 */
QWidget* QwtPlotPanner::canvas()
{
    QWidget* w = parentWidget();
    if (w && w->inherits("QwtPlotCanvas"))
        return w;
    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Get the canvas widget (const version)
 * @return Const pointer to the canvas widget, or nullptr if not attached
 * @details Returns the canvas widget that this panner is attached to.
 *          The canvas must inherit from QwtPlotCanvas.
 * \endif
 *
 * \if CHINESE
 * @brief 获取画布控件（常量版本）
 * @return 画布控件的常量指针，如果未附加则返回 nullptr
 * @details 返回此平移器附加到的画布控件。
 *          画布必须继承自 QwtPlotCanvas。
 * \endif
 */
const QWidget* QwtPlotPanner::canvas() const
{
    const QWidget* w = parentWidget();
    if (w && w->inherits("QwtPlotCanvas"))
        return w;
    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Get the plot widget
 * @return Pointer to the plot widget, or nullptr if not attached
 * @details Returns the QwtPlot widget that owns the canvas.
 *          This is a convenience method to access the parent plot.
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图控件
 * @return 绘图控件指针，如果未附加则返回 nullptr
 * @details 返回拥有画布的 QwtPlot 控件。
 *          这是一个访问父绘图的便捷方法。
 * \endif
 */
QwtPlot* QwtPlotPanner::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    if (w && w->inherits("QwtPlot"))
        return static_cast< QwtPlot* >(w);

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Get the plot widget (const version)
 * @return Const pointer to the plot widget, or nullptr if not attached
 * @details Returns the QwtPlot widget that owns the canvas.
 *          This is a convenience method to access the parent plot.
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图控件（常量版本）
 * @return 绘图控件的常量指针，如果未附加则返回 nullptr
 * @details 返回拥有画布的 QwtPlot 控件。
 *          这是一个访问父绘图的便捷方法。
 * \endif
 */
const QwtPlot* QwtPlotPanner::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    if (w && w->inherits("QwtPlot"))
        return static_cast< const QwtPlot* >(w);

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Set the orientations for panning
 * @param[in] o Combination of Qt::Horizontal and Qt::Vertical flags
 * @details Sets the directions in which panning is enabled.
 *          By default, both horizontal and vertical panning are enabled.
 *          Use Qt::Horizontal for left-right panning, Qt::Vertical for
 *          up-down panning, or both for unrestricted panning.
 * \endif
 *
 * \if CHINESE
 * @brief 设置平移方向
 * @param[in] o Qt::Horizontal 和 Qt::Vertical 标志的组合
 * @details 设置启用平移的方向。
 *          默认情况下，水平和垂直平移都是启用的。
 *          使用 Qt::Horizontal 启用左右平移，Qt::Vertical 启用上下平移，
 *          或两者都用以实现无限制平移。
 * \endif
 */
void QwtPlotPanner::setOrientations(Qt::Orientations o)
{
    m_data->orientations = o;
}

/**
 * \if ENGLISH
 * @brief Get the orientations for panning
 * @return Combination of Qt::Horizontal and Qt::Vertical flags
 * @details Returns the currently enabled panning directions.
 * \endif
 *
 * \if CHINESE
 * @brief 获取平移方向
 * @return Qt::Horizontal 和 Qt::Vertical 标志的组合
 * @details 返回当前启用的平移方向。
 * \endif
 */
Qt::Orientations QwtPlotPanner::orientations() const
{
    return m_data->orientations;
}

/**
 * \if ENGLISH
 * @brief Check if an orientation is enabled
 * @param[in] o The orientation to check
 * @return True if the orientation is enabled, false otherwise
 * @details Tests whether panning is enabled in the specified direction.
 * \endif
 *
 * \if CHINESE
 * @brief 检查某个方向是否启用
 * @param[in] o 要检查的方向
 * @return 如果方向已启用则返回 true，否则返回 false
 * @details 测试指定方向的平移是否已启用。
 * \endif
 */
bool QwtPlotPanner::isOrientationEnabled(Qt::Orientation o) const
{
    return m_data->orientations & o;
}

/**
 * \if ENGLISH
 * @brief Set the mouse button and modifiers for panning
 * @param[in] button The mouse button to use for panning
 * @param[in] modifiers Keyboard modifiers (default: Qt::NoModifier)
 * @details Configures the mouse button and optional keyboard modifiers
 *          that trigger panning operations. By default, the left mouse
 *          button with no modifiers is used.
 *
 * @code
 * // Use middle mouse button
 * panner->setMouseButton(Qt::MiddleButton);
 *
 * // Use left mouse button with Ctrl key
 * panner->setMouseButton(Qt::LeftButton, Qt::ControlModifier);
 * @endcode
 * \endif
 *
 * \if CHINESE
 * @brief 设置平移使用的鼠标按键和修饰键
 * @param[in] button 用于平移的鼠标按键
 * @param[in] modifiers 键盘修饰键（默认：Qt::NoModifier）
 * @details 配置触发平移操作的鼠标按键和可选的键盘修饰键。
 *          默认情况下，使用不带修饰键的鼠标左键。
 *
 * @code
 * // 使用鼠标中键
 * panner->setMouseButton(Qt::MiddleButton);
 *
 * // 使用 Ctrl + 鼠标左键
 * panner->setMouseButton(Qt::LeftButton, Qt::ControlModifier);
 * @endcode
 * \endif
 */
void QwtPlotPanner::setMouseButton(Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
    setMousePattern(QwtEventPattern::MouseSelect1, button, modifiers);
}

/**
 * \if ENGLISH
 * @brief Get the mouse button and modifiers for panning
 * @param[out] button The currently configured mouse button
 * @param[out] modifiers The currently configured keyboard modifiers
 * @details Retrieves the mouse button and keyboard modifiers currently
 *          configured for panning operations.
 * \endif
 *
 * \if CHINESE
 * @brief 获取平移使用的鼠标按键和修饰键
 * @param[out] button 当前配置的鼠标按键
 * @param[out] modifiers 当前配置的键盘修饰键
 * @details 获取当前配置用于平移操作的鼠标按键和键盘修饰键。
 * \endif
 */
void QwtPlotPanner::getMouseButton(Qt::MouseButton& button, Qt::KeyboardModifiers& modifiers) const
{
    const QVector< MousePattern >& mp = mousePattern();
    button                            = mp[ QwtEventPattern::MouseSelect1 ].button;
    modifiers                         = mp[ QwtEventPattern::MouseSelect1 ].modifiers;
}

void QwtPlotPanner::move(const QPoint& pos)
{
    if (!isActive()) {
        return;
    }
    QWT_D(d);
    d->currentPos = pos;

    int dx = pos.x() - d->initialPos.x();
    int dy = pos.y() - d->initialPos.y();

    if (!isOrientationEnabled(Qt::Horizontal)) {
        dx = 0;
    }
    if (!isOrientationEnabled(Qt::Vertical)) {
        dy = 0;
    }

    if (dx != 0 || dy != 0) {
        moveCanvas(dx, dy);
        d->initialPos = pos;
    }
}

bool QwtPlotPanner::end(bool ok)
{
    if (!isActive()) {
        return QwtPicker::end(ok);
    }
    QWT_D(d);
    int wholeDx = d->currentPos.x() - d->beginPos.x();
    int wholeDy = d->currentPos.y() - d->beginPos.y();

    if (!isOrientationEnabled(Qt::Horizontal)) {
        wholeDx = 0;
    }
    if (!isOrientationEnabled(Qt::Vertical)) {
        wholeDy = 0;
    }

    if (wholeDx != 0 || wholeDy != 0) {
        Q_EMIT panned(wholeDx, wholeDy);
    }

    d->initialPos = QPoint();
    d->currentPos = QPoint();
    d->beginPos   = QPoint();
    return QwtPicker::end(ok);
}

/**
 * \if ENGLISH
 * @brief Move the canvas by the specified offset
 * @param[in] dx Horizontal offset in pixels
 * @param[in] dy Vertical offset in pixels
 * @details Moves the canvas content by the specified pixel offsets.
 *          This method can be called programmatically to pan the plot.
 *          It handles parasite plots correctly by updating all plots
 *          in the plot list in the correct order.
 *
 * @note For parasite plots that share axes with the host plot,
 *       the host plot is updated last so its changes propagate
 *       correctly to the parasite plots.
 * \endif
 *
 * \if CHINESE
 * @brief 按指定偏移移动画布
 * @param[in] dx 水平偏移量（像素）
 * @param[in] dy 垂直偏移量（像素）
 * @details 按指定的像素偏移量移动画布内容。
 *          此方法可以程序化调用来平移绘图。
 *          它正确处理寄生绘图，以正确的顺序更新绘图列表中的所有绘图。
 *
 * @note 对于与宿主绘图共享轴的寄生绘图，
 *       宿主绘图最后更新，以便其更改正确传播到寄生绘图。
 * \endif
 */
void QwtPlotPanner::moveCanvas(int dx, int dy)
{
    QwtPlot* plt = plot();
    if (!plt) {
        return;
    }
    // Note: For parasite plots sharing axes with host, host should pan last
    // so its updates propagate to parasite plots correctly
    const QList< QwtPlot* > allPlots = plt->plotList(true);
    for (auto* plot : allPlots) {
        plot->panCanvas(QPoint(dx, dy));
    }
    plt->replotAll();
}

void QwtPlotPanner::widgetMousePressEvent(QMouseEvent* mouseEvent)
{
    QWT_D(d);

    // Note: We don't use begin() to record position because in AlwaysOff mode,
    // trackerPosition() cannot get the click position, so we record it here
    if (mouseMatch(QwtEventPattern::MouseSelect1,
                   static_cast< const QMouseEvent* >(mouseEvent))) {
        d->beginPos = d->initialPos = d->currentPos = mouseEvent->pos();
    }

    QwtPicker::widgetMousePressEvent(mouseEvent);
}