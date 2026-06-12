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
    QWT_DECLARE_PUBLIC(QwtPlotAxisZoomer)
public:
    PrivateData(QwtPlotAxisZoomer* p)
        : q_ptr(p)
    {
    }

    uint zoomRectIndex;
    QStack< QRectF > zoomStack;

    int maxStackDepth;
};

/**
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
 * 
 */
QwtPlotAxisZoomer::QwtPlotAxisZoomer(QWidget* canvas, bool doReplot) : QwtPlotPicker(canvas)
{
    if (canvas)
        init(doReplot);
}

/**
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
 * 
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
    QWT_PIMPL_CONSTRUCT_INIT();

    QWT_D(d);
    d->maxStackDepth = -1;

    setTrackerMode(ActiveOnly);
    setRubberBand(RectRubberBand);
    setStateMachine(new QwtPickerDragRectMachine());

    if (doReplot && plot())
        plot()->replot();

    setZoomBase(scaleRect());
}

QwtPlotAxisZoomer::~QwtPlotAxisZoomer()
{
}

/**
 * @brief Limits the number of recursive zoom operations to depth.
 * @details A value of -1 sets the depth to unlimited, 0 disables zooming.
 *          If the current zoom rectangle is below depth, the plot is unzoomed.
 * @param[in] depth Maximum for the stack depth.
 * @sa maxStackDepth()
 * @note depth doesn't include the zoom base, so zoomStack().count() might be
 *       maxStackDepth() + 1.
 * 
 */
void QwtPlotAxisZoomer::setMaxStackDepth(int depth)
{
    QWT_D(d);
    d->maxStackDepth = depth;

    if (depth >= 0) {
        // unzoom if the current depth is below d->maxStackDepth

        const int zoomOut = d->zoomStack.count() - 1 - depth;  // -1 for the zoom base

        if (zoomOut > 0) {
            zoom(-zoomOut);
            for (int i = d->zoomStack.count() - 1; i > int(d->zoomRectIndex); i--) {
                (void)d->zoomStack.pop();  // remove trailing rects
            }
        }
    }
}

/**
 * @brief Returns the maximal depth of the zoom stack.
 * @return Maximal depth of the zoom stack.
 * @sa setMaxStackDepth()
 * 
 */
int QwtPlotAxisZoomer::maxStackDepth() const
{
    QWT_DC(d);
    return d->maxStackDepth;
}

/**
 * @brief Returns the zoom stack.
 * @return The zoom stack. zoomStack()[0] is the zoom base,
 *         zoomStack()[1] the first zoomed rectangle.
 * @sa setZoomStack(), zoomRectIndex()
 * 
 */
const QStack< QRectF >& QwtPlotAxisZoomer::zoomStack() const
{
    QWT_DC(d);
    return d->zoomStack;
}

/**
 * @brief Returns the initial rectangle of the zoomer.
 * @return Initial rectangle of the zoomer.
 * @sa setZoomBase(), zoomRect()
 * 
 */
QRectF QwtPlotAxisZoomer::zoomBase() const
{
    QWT_DC(d);
    return d->zoomStack[ 0 ];
}

/**
 * @brief Reinitializes the zoom stack with scaleRect() as base.
 * @param[in] doReplot Call QwtPlot::replot() for the attached plot before initializing
 *                      the zoomer with its scales. This might be necessary,
 *                      when the plot is in a state with pending scale changes.
 * @sa zoomBase(), scaleRect(), QwtPlot::autoReplot(), QwtPlot::replot()
 * 
 */
void QwtPlotAxisZoomer::setZoomBase(bool doReplot)
{
    QWT_D(d);
    QwtPlot* plt = plot();
    if (plt == nullptr)
        return;

    if (doReplot)
        plt->replot();

    d->zoomStack.clear();
    d->zoomStack.push(scaleRect());
    d->zoomRectIndex = 0;

    rescale();
}

/**
 * @brief Sets the initial size of the zoomer.
 * @details base is united with the current scaleRect() and the zoom stack is
 *          reinitialized with it as zoom base. plot is zoomed to scaleRect().
 * @param[in] base Zoom base rectangle.
 * @sa zoomBase(), scaleRect()
 * 
 */
void QwtPlotAxisZoomer::setZoomBase(const QRectF& base)
{
    QWT_D(d);
    const QwtPlot* plt = plot();
    if (!plt)
        return;

    const QRectF sRect = scaleRect();
    const QRectF bRect = base | sRect;

    d->zoomStack.clear();
    d->zoomStack.push(bRect);
    d->zoomRectIndex = 0;

    if (base != sRect) {
        d->zoomStack.push(sRect);
        d->zoomRectIndex++;
    }

    rescale();
}

/**
 * @brief Returns the rectangle at the current position on the zoom stack.
 * @return Rectangle at the current position on the zoom stack.
 * @sa zoomRectIndex(), scaleRect()
 * 
 */
QRectF QwtPlotAxisZoomer::zoomRect() const
{
    QWT_DC(d);
    return d->zoomStack[ d->zoomRectIndex ];
}

/**
 * @brief Returns the index of current position of zoom stack.
 * @return Index of current position of zoom stack.
 * 
 */
uint QwtPlotAxisZoomer::zoomRectIndex() const
{
    QWT_DC(d);
    return d->zoomRectIndex;
}

/**
 * @brief Zooms in to a rectangle.
 * @details Clears all rectangles above the current position of the
 *          zoom stack and pushes the normalized rectangle on it.
 * @param[in] rect Rectangle to zoom to.
 * @note If the maximal stack depth is reached, zoom is ignored.
 * @note The zoomed signal is emitted.
 * 
 */

void QwtPlotAxisZoomer::zoom(const QRectF& rect)
{
    QWT_D(d);
    if (d->maxStackDepth >= 0 && int(d->zoomRectIndex) >= d->maxStackDepth) {
        return;
    }

    const QRectF zoomRect = rect.normalized();
    if (zoomRect != d->zoomStack[ d->zoomRectIndex ]) {
        for (uint i = d->zoomStack.count() - 1; i > d->zoomRectIndex; i--) {
            (void)d->zoomStack.pop();
        }

        d->zoomStack.push(zoomRect);
        d->zoomRectIndex++;

        rescale();

        Q_EMIT zoomed(zoomRect);
    }
}

/**
 * @brief Zooms in or out by an offset.
 * @details Activates a rectangle on the zoom stack with an offset relative
 *          to the current position. Negative values of offset will zoom out,
 *          positive zoom in. A value of 0 zooms out to the zoom base.
 * @param[in] offset Offset relative to the current position of the zoom stack.
 * @note The zoomed signal is emitted.
 * @sa zoomRectIndex()
 * 
 */
void QwtPlotAxisZoomer::zoom(int offset)
{
    QWT_D(d);
    int newIndex;

    if (offset == 0) {
        newIndex = 0;
    } else {
        newIndex = d->zoomRectIndex + offset;
        newIndex = qBound(0, newIndex, d->zoomStack.count() - 1);
    }

    if (newIndex != static_cast< int >(d->zoomRectIndex)) {
        d->zoomRectIndex = newIndex;
        rescale();
        Q_EMIT zoomed(zoomRect());
    }
}

/**
 * @brief Assigns a zoom stack.
 * @details In combination with other types of navigation it might be useful to
 *          modify to manipulate the complete zoom stack.
 * @param[in] zoomStack New zoom stack.
 * @param[in] zoomRectIndex Index of the current position of zoom stack.
 *                          In case of -1 the current position is at the top
 *                          of the stack.
 * @note The zoomed signal might be emitted.
 * @sa zoomStack(), zoomRectIndex()
 * 
 */
void QwtPlotAxisZoomer::setZoomStack(const QStack< QRectF >& zoomStack, int zoomRectIndex)
{
    QWT_D(d);
    if (zoomStack.isEmpty())
        return;

    if (d->maxStackDepth >= 0 && zoomStack.count() > d->maxStackDepth) {
        return;
    }

    if (zoomRectIndex < 0 || zoomRectIndex > zoomStack.count())
        zoomRectIndex = zoomStack.count() - 1;

    const bool doRescale = zoomStack[ zoomRectIndex ] != zoomRect();

    d->zoomStack     = zoomStack;
    d->zoomRectIndex = uint(zoomRectIndex);

    if (doRescale) {
        rescale();
        Q_EMIT zoomed(zoomRect());
    }
}

/*!
   Adjust the observed plot to zoomRect()

   @note Initiates QwtPlot::replot()
 */

void QwtPlotAxisZoomer::rescale()
{
    QWT_D(d);
    QwtPlot* plt = plot();
    if (!plt)
        return;

    const QRectF& rect = d->zoomStack[ d->zoomRectIndex ];
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
 * @brief Reinitializes the axes, and sets the zoom base to their scales.
 * @param[in] xAxisId X axis.
 * @param[in] yAxisId Y axis.
 * 
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

   @note The mouse events can be changed, using
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

   @note The keys codes can be changed, using
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
 * @brief Moves the current zoom rectangle by an offset.
 * @param[in] dx X offset.
 * @param[in] dy Y offset.
 * @note The changed rectangle is limited by the zoom base.
 * 
 */
void QwtPlotAxisZoomer::moveBy(double dx, double dy)
{
    QWT_D(d);
    const QRectF& rect = d->zoomStack[ d->zoomRectIndex ];
    moveTo(QPointF(rect.left() + dx, rect.top() + dy));
}

/**
 * @brief Moves the current zoom rectangle.
 * @param[in] pos New position.
 * @sa QRectF::moveTo()
 * @note The changed rectangle is limited by the zoom base.
 * 
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
        QWT_D(d);
        d->zoomStack[ d->zoomRectIndex ].moveTo(x, y);
        rescale();
    }
}

/*!
   @brief Check and correct a selected rectangle

   Reject rectangles with a height or width < 2, otherwise
   expand the selected rectangle to a minimum size of 11x11
   and accept it.

   @return true If the rectangle is accepted, or has been changed
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
   @brief Limit zooming by a minimum rectangle

   @return zoomBase().width() / 10e4, zoomBase().height() / 10e4
 */
QSizeF QwtPlotAxisZoomer::minZoomSize() const
{
    QWT_DC(d);
    return QSizeF(d->zoomStack[ 0 ].width() / 10e4, d->zoomStack[ 0 ].height() / 10e4);
}

/*!
   Rejects selections, when the stack depth is too deep, or
   the zoomed rectangle is minZoomSize().

   @sa minZoomSize(), maxStackDepth()
 */
void QwtPlotAxisZoomer::begin()
{
    QWT_D(d);
    if (d->maxStackDepth >= 0) {
        if (d->zoomRectIndex >= uint(d->maxStackDepth))
            return;
    }

    const QSizeF minSize = minZoomSize();
    if (minSize.isValid()) {
        const QSizeF sz = d->zoomStack[ d->zoomRectIndex ].size() * 0.9999;

        if (minSize.width() >= sz.width() && minSize.height() >= sz.height()) {
            return;
        }
    }

    QwtPlotPicker::begin();
}

/*!
   Expand the selected rectangle to minZoomSize() and zoom in
   if accepted.

   @param ok If true, complete the selection and emit selected signals
            otherwise discard the selection.

   @sa accept(), minZoomSize()
   @return True if the selection has been accepted, false otherwise
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
