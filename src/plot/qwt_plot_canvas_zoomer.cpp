/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/

#include "qwt_plot_canvas_zoomer.h"
#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_picker_machine.h"

#include <qstack.h>
#include <limits>

/**
 * @brief Default constructor creating an invalid zoom state
 */
QwtPlotCanvasZoomState::QwtPlotCanvasZoomState()
{
}

/**
 * @brief Constructor creating a zoom state with specified axis intervals
 * @param[in] p Plot widget associated with this zoom state
 * @param[in] yLeft Interval for the left Y axis
 * @param[in] yRight Interval for the right Y axis
 * @param[in] xBottom Interval for the bottom X axis
 * @param[in] xTop Interval for the top X axis
 */
QwtPlotCanvasZoomState::QwtPlotCanvasZoomState(QwtPlot* p,
                                               const QwtInterval& yLeft,
                                               const QwtInterval& yRight,
                                               const QwtInterval& xBottom,
                                               const QwtInterval& xTop)
    : plot(p)
{
    axisInterval[ QwtAxis::YLeft ]   = yLeft;
    axisInterval[ QwtAxis::YRight ]  = yRight;
    axisInterval[ QwtAxis::XBottom ] = xBottom;
    axisInterval[ QwtAxis::XTop ]    = xTop;
}

/**
 * @brief Create a zoom state from the current axis ranges of a plot
 * @param[in] p Plot widget to capture axis ranges from
 * @return QwtPlotCanvasZoomState containing current axis intervals
 * @details This static method captures the current scale divisions of all four axes
 *          from the given plot and stores them in a new zoom state object.
 */
QwtPlotCanvasZoomState QwtPlotCanvasZoomState::fromPlot(QwtPlot* p)
{
    QwtPlotCanvasZoomState state;
    if (!p) {
        return state;
    }
    state.plot = p;
    // Get current ranges for all four axes
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        const QwtScaleDiv& scaleDiv  = p->axisScaleDiv(axisId);
        state.axisInterval[ axisId ] = scaleDiv.interval();
    }

    return state;
}

/**
 * @brief Apply this zoom state to the associated plot
 * @details Sets all four axis scales to the intervals stored in this zoom state.
 *          Does nothing if the plot pointer is null.
 */
void QwtPlotCanvasZoomState::apply() const
{
    if (!plot) {
        return;
    }

    // Apply ranges for all four axes
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        plot->setAxisScale(axisId, axisInterval[ axisId ].minValue(), axisInterval[ axisId ].maxValue());
    }
}

/**
 * @brief Compare two zoom states for equality
 * @param[in] other Another zoom state to compare with
 * @return true if both states have the same plot and axis intervals
 */
bool QwtPlotCanvasZoomState::operator==(const QwtPlotCanvasZoomState& other) const
{
    if (plot != other.plot) {
        return false;
    }
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        if (axisInterval[ axisId ] != other.axisInterval[ axisId ]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Compare two zoom states for inequality
 * @param[in] other Another zoom state to compare with
 * @return true if the states differ in plot or any axis interval
 */
bool QwtPlotCanvasZoomState::operator!=(const QwtPlotCanvasZoomState& other) const
{
    return !(*this == other);
}

/**
 * @brief Check if this zoom state is valid
 * @return true if the zoom state has an associated plot pointer
 */
bool QwtPlotCanvasZoomState::isValid() const
{
    return (!plot.isNull());
}
//----------------------------------------------------
// QwtCanvasZoomer::PrivateData
//----------------------------------------------------
class QwtPlotCanvasZoomer::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotCanvasZoomer)
public:
    PrivateData(QwtPlotCanvasZoomer* p);
    uint zoomStateIndex;
    QStack< QList< QwtPlotCanvasZoomState > > zoomStack;
    int maxStackDepth { -1 };  ///< Maximum zoom depth recorded; exceeding this prevents further zooming
    bool replot { true };      ///< Whether to replot
};

QwtPlotCanvasZoomer::PrivateData::PrivateData(QwtPlotCanvasZoomer* p) : q_ptr(p)
{
}

//----------------------------------------------------
// QwtCanvasZoomer
//----------------------------------------------------

/**
 * @brief Constructor creating a zoomer for a plot canvas
 * @param[in] canvas Plot canvas to observe, also the parent object
 * @param[in] doReplot If true, call QwtPlot::replot() before initializing
 * @details The zoomer will zoom all axes of the plot simultaneously.
 *          It uses a drag rectangle picker machine for selecting zoom regions.
 *          The tracker mode is set to ActiveOnly and the rubber band is set to RectRubberBand.
 */
QwtPlotCanvasZoomer::QwtPlotCanvasZoomer(QWidget* canvas, bool doReplot) : QwtCanvasPicker(canvas), QWT_PIMPL_CONSTRUCT
{
    if (canvas) {
        init(doReplot);
    }
}

/**
 * @brief Destructor
 */
QwtPlotCanvasZoomer::~QwtPlotCanvasZoomer()
{
}

void QwtPlotCanvasZoomer::init(bool doReplot)
{
    m_data->maxStackDepth = -1;
    m_data->replot        = doReplot;
    setTrackerMode(ActiveOnly);
    setRubberBand(RectRubberBand);
    setStateMachine(new QwtPickerDragRectMachine());
    setZoomBase(doReplot);
}

QList< QwtPlotCanvasZoomState > QwtPlotCanvasZoomer::canvasRectToZoomStateList(const QRect& pixelRect) const
{
    const QwtPlot* plt = plot();
    if (!plt) {
        return QList< QwtPlotCanvasZoomState >();
    }
    QList< QwtPlotCanvasZoomState > states;
    // Put host axis last; parasite axes are usually bound to a host axis,
    // changing the host last resyncs parasite axes with the host
    const QList< QwtPlot* > plts = plt->plotList(true);
    for (QwtPlot* p : plts) {
        QwtPlotCanvasZoomState s = canvasRectToZoomState(p, pixelRect);
        states.append(s);
    }
    return states;
}

QwtPlotCanvasZoomState QwtPlotCanvasZoomer::canvasRectToZoomState(QwtPlot* plt, const QRect& pixelRect) const
{
    if (!plt) {
        return QwtPlotCanvasZoomState();
    }
    // Convert pixel rectangle to intervals for all four axes
    QRect normalizedRect = pixelRect.normalized();
    QwtPlotCanvasZoomState state;
    state.plot = plt;
    for (QwtAxisId axisId = 0; axisId < QwtAxis::AxisPositions; ++axisId) {
        const QwtScaleMap& scaleMap = plt->canvasMap(axisId);
        double v1 { 0.0 }, v2 { 0.0 };
        if (QwtAxis::isXAxis(axisId)) {
            v1 = scaleMap.invTransform(normalizedRect.left());
            v2 = scaleMap.invTransform(normalizedRect.right());
        } else {
            v1 = scaleMap.invTransform(normalizedRect.bottom());
            v2 = scaleMap.invTransform(normalizedRect.top());
        }
        if (const QwtTransform* transform = scaleMap.transformation()) {
            // If a transform exists, confirm its range; bounded is effectively qBound(LogMin, value, LogMax)
            v1 = transform->bounded(v1);
            v2 = transform->bounded(v2);
        }
        state.axisInterval[ axisId ].setInterval(v1, v2);
    }
    return state;
}

/**
 * @brief Set the maximum depth of the zoom stack
 * @param[in] depth Maximum number of zoom levels, -1 for unlimited
 * @details When the stack exceeds this depth, older zoom states are removed.
 *          If the current zoom level is beyond the new limit, it will be adjusted.
 */
void QwtPlotCanvasZoomer::setMaxStackDepth(int depth)
{
    m_data->maxStackDepth = depth;

    if (depth >= 0) {
        const int zoomOut = m_data->zoomStack.count() - 1 - depth;
        if (zoomOut > 0) {
            zoom(-zoomOut);
            for (int i = m_data->zoomStack.count() - 1; i > int(m_data->zoomStateIndex); i--) {
                (void)m_data->zoomStack.pop();
            }
        }
    }
}

/**
 * @brief Get the maximum depth of the zoom stack
 * @return Maximum number of zoom levels, -1 means unlimited
 */
int QwtPlotCanvasZoomer::maxStackDepth() const
{
    return m_data->maxStackDepth;
}

/**
 * @brief Get the zoom stack containing all zoom states
 * @return Const reference to the internal zoom stack
 * @details The stack contains the complete history of zoom operations,
 *          with index 0 being the base (unzoomed) state.
 */
const QStack< QList< QwtPlotCanvasZoomState > >& QwtPlotCanvasZoomer::zoomStack() const
{
    return m_data->zoomStack;
}

/**
 * @brief Get the base zoom state (initial unzoomed state)
 * @return List of zoom states for all plots at the base level
 * @details Returns an empty list if the zoom stack is empty.
 */
QList< QwtPlotCanvasZoomState > QwtPlotCanvasZoomer::zoomBase() const
{
    if (m_data->zoomStack.isEmpty()) {
        return QList< QwtPlotCanvasZoomState >();
    }
    return m_data->zoomStack[ 0 ];
}

/**
 * @brief Get the current zoom state
 * @return List of zoom states for all plots at the current zoom level
 * @details Returns an empty list if the zoom stack is empty.
 */
QList< QwtPlotCanvasZoomState > QwtPlotCanvasZoomer::zoomState() const
{
    if (m_data->zoomStack.isEmpty()) {
        return QList< QwtPlotCanvasZoomState >();
    }
    return m_data->zoomStack[ m_data->zoomStateIndex ];
}

/**
 * @brief Set the zoom base to the current axis ranges
 * @param[in] doReplot If true, call QwtPlot::replotAll() before capturing state
 * @details This method clears the zoom stack and sets the current view as the base state.
 *          It captures the axis ranges of the main plot and all parasite plots.
 */
void QwtPlotCanvasZoomer::setZoomBase(bool doReplot)
{
    QwtPlot* plt = plot();
    if (!plt) {
        return;
    }

    if (doReplot) {
        plt->replotAll();
    }

    m_data->zoomStack.clear();
    const QList< QwtPlot* > plts = plt->plotList(true);
    QList< QwtPlotCanvasZoomState > base;
    for (QwtPlot* p : plts) {
        base.append(QwtPlotCanvasZoomState::fromPlot(p));
    }
    m_data->zoomStack.push(base);
    m_data->zoomStateIndex = 0;
}

/**
 * @brief Enable or disable automatic replot after zoom operations
 * @param[in] on If true, the plot will be automatically replotted after zoom
 * @details When enabled, the plot will automatically replot after each zoom operation.
 *          This is enabled by default.
 */
void QwtPlotCanvasZoomer::setAutoReplot(bool on)
{
    m_data->replot = on;
}

/**
 * @brief Check if automatic replot is enabled
 * @return true if automatic replot is enabled
 */
bool QwtPlotCanvasZoomer::isAutoReplot() const
{
    return m_data->replot;
}

/**
 * @brief Navigate in the zoom stack by the specified offset
 * @param[in] offset Number of positions to move (negative = zoom out, positive = zoom in)
 * @details Moving by 0 resets to the base zoom level.
 *          The zoomed() signal is emitted if the zoom state changes.
 */
void QwtPlotCanvasZoomer::zoom(int offset)
{
    int newIndex;
    if (offset == 0) {
        newIndex = 0;
    } else {
        newIndex = m_data->zoomStateIndex + offset;
        newIndex = qBound(0, newIndex, m_data->zoomStack.count() - 1);
    }

    if (newIndex != static_cast< int >(m_data->zoomStateIndex)) {
        m_data->zoomStateIndex = newIndex;
        rescale();
        Q_EMIT zoomed(zoomState());
    }
}

/**
 * @brief Append a new zoom state to the zoom stack
 * @param[in] rect List of zoom states for all plots
 * @details This method adds a new zoom state to the stack and makes it the current state.
 *          The zoomed() signal is emitted after the zoom is applied.
 */
void QwtPlotCanvasZoomer::appendZoom(const QList< QwtPlotCanvasZoomState >& rect)
{
    m_data->zoomStack.push(rect);
    m_data->zoomStateIndex = m_data->zoomStack.count() - 1;
    rescale();
    Q_EMIT zoomed(rect);
}

void QwtPlotCanvasZoomer::rescale()
{
    const QList< QwtPlotCanvasZoomState >& states = m_data->zoomStack[ m_data->zoomStateIndex ];
    for (const QwtPlotCanvasZoomState& state : states) {
        state.apply();
    }
    if (m_data->replot) {
        QwtPlot* plt = plot();
        plt->replotAll();
    }
}

void QwtPlotCanvasZoomer::widgetMouseReleaseEvent(QMouseEvent* me)
{
    if (mouseMatch(MouseSelect2, me))
        zoom(0);
    else if (mouseMatch(MouseSelect3, me))
        zoom(-1);
    else if (mouseMatch(MouseSelect6, me))
        zoom(+1);
    else
        QwtPicker::widgetMouseReleaseEvent(me);
}

void QwtPlotCanvasZoomer::widgetKeyPressEvent(QKeyEvent* ke)
{
    if (!isActive()) {
        if (keyMatch(KeyUndo, ke))
            zoom(-1);
        else if (keyMatch(KeyRedo, ke))
            zoom(+1);
        else if (keyMatch(KeyHome, ke))
            zoom(0);
    }

    QwtPicker::widgetKeyPressEvent(ke);
}

bool QwtPlotCanvasZoomer::accept(QPolygon& pa) const
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

void QwtPlotCanvasZoomer::begin()
{
    if (m_data->maxStackDepth >= 0) {
        if (m_data->zoomStateIndex >= uint(m_data->maxStackDepth))
            return;
    }

    QwtPicker::begin();
}

bool QwtPlotCanvasZoomer::end(bool ok)
{
    ok = QwtPicker::end(ok);
    if (!ok) {
        return false;
    }

    const QPolygon& pa = selection();
    if (pa.count() < 2) {
        return false;
    }

    QRect rect = QRect(pa.first(), pa.last());
    rect       = rect.normalized();

    // Directly convert canvas rectangle to zoom states for all four axes
    QList< QwtPlotCanvasZoomState > newState = canvasRectToZoomStateList(rect);

    appendZoom(newState);
    return true;
}
