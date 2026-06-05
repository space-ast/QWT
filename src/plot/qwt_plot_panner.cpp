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
 * @brief Constructor
 * @param[in] canvas The plot canvas widget to attach the panner to
 * @details Creates a panner attached to the specified canvas widget.
 *          The panner is automatically enabled and configured to use
 *          left mouse button for panning operations.
 */
QwtPlotPanner::QwtPlotPanner(QWidget* canvas) : QwtPicker(canvas), QWT_PIMPL_CONSTRUCT
{
    if (canvas) {
        init();
    }
}

/**
 * @brief Destructor
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
 * @brief Get the canvas widget
 * @return Pointer to the canvas widget, or nullptr if not attached
 * @details Returns the canvas widget that this panner is attached to.
 *          The canvas must inherit from QwtPlotCanvas.
 */
QWidget* QwtPlotPanner::canvas()
{
    QWidget* w = parentWidget();
    if (w && w->inherits("QwtPlotCanvas"))
        return w;
    return nullptr;
}

/**
 * @brief Get the canvas widget (const version)
 * @return Const pointer to the canvas widget, or nullptr if not attached
 * @details Returns the canvas widget that this panner is attached to.
 *          The canvas must inherit from QwtPlotCanvas.
 */
const QWidget* QwtPlotPanner::canvas() const
{
    const QWidget* w = parentWidget();
    if (w && w->inherits("QwtPlotCanvas"))
        return w;
    return nullptr;
}

/**
 * @brief Get the plot widget
 * @return Pointer to the plot widget, or nullptr if not attached
 * @details Returns the QwtPlot widget that owns the canvas.
 *          This is a convenience method to access the parent plot.
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
 * @brief Get the plot widget (const version)
 * @return Const pointer to the plot widget, or nullptr if not attached
 * @details Returns the QwtPlot widget that owns the canvas.
 *          This is a convenience method to access the parent plot.
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
 * @brief Set the orientations for panning
 * @param[in] o Combination of Qt::Horizontal and Qt::Vertical flags
 * @details Sets the directions in which panning is enabled.
 *          By default, both horizontal and vertical panning are enabled.
 *          Use Qt::Horizontal for left-right panning, Qt::Vertical for
 *          up-down panning, or both for unrestricted panning.
 */
void QwtPlotPanner::setOrientations(Qt::Orientations o)
{
    m_data->orientations = o;
}

/**
 * @brief Get the orientations for panning
 * @return Combination of Qt::Horizontal and Qt::Vertical flags
 * @details Returns the currently enabled panning directions.
 */
Qt::Orientations QwtPlotPanner::orientations() const
{
    return m_data->orientations;
}

/**
 * @brief Check if an orientation is enabled
 * @param[in] o The orientation to check
 * @return True if the orientation is enabled, false otherwise
 * @details Tests whether panning is enabled in the specified direction.
 */
bool QwtPlotPanner::isOrientationEnabled(Qt::Orientation o) const
{
    return m_data->orientations & o;
}

/**
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
 */
void QwtPlotPanner::setMouseButton(Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
    setMousePattern(QwtEventPattern::MouseSelect1, button, modifiers);
}

/**
 * @brief Get the mouse button and modifiers for panning
 * @param[out] button The currently configured mouse button
 * @param[out] modifiers The currently configured keyboard modifiers
 * @details Retrieves the mouse button and keyboard modifiers currently
 *          configured for panning operations.
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
