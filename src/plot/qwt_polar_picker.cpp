/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_picker.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_scale_map.h"
#include "qwt_picker_machine.h"
#include "qwt_point_polar.h"

class QwtPolarPicker::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPolarPicker)
public:
    PrivateData(QwtPolarPicker* p) : q_ptr(p)
    {
    }
};

/**
 * @brief Create a polar plot picker
 * @param canvas Plot canvas to observe, also the parent object
 */
QwtPolarPicker::QwtPolarPicker(QwtPolarCanvas* canvas) : QwtPicker(canvas), QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Create a plot picker with rubber band and tracker mode
 * @param rubberBand Rubberband style
 * @param trackerMode Tracker mode
 * @param canvas Plot canvas to observe, also the parent object
 * @sa QwtPicker, QwtPicker::setSelectionFlags(), QwtPicker::setRubberBand(),
 *     QwtPicker::setTrackerMode, QwtPolarPlot::autoReplot(), QwtPolarPlot::replot()
 */
QwtPolarPicker::QwtPolarPicker(RubberBand rubberBand, DisplayMode trackerMode, QwtPolarCanvas* canvas)
    : QwtPicker(rubberBand, trackerMode, canvas), QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Destructor
 */
QwtPolarPicker::~QwtPolarPicker()
{
}

/**
 * @brief Get the observed plot canvas
 * @return Observed plot canvas
 */
QwtPolarCanvas* QwtPolarPicker::canvas()
{
    return qobject_cast< QwtPolarCanvas* >(parentWidget());
}

/**
 * @brief Get the observed plot canvas (const version)
 * @return Observed plot canvas
 */
const QwtPolarCanvas* QwtPolarPicker::canvas() const
{
    return qobject_cast< const QwtPolarCanvas* >(parentWidget());
}

/**
 * @brief Get the plot widget containing the observed plot canvas
 * @return Plot widget, containing the observed plot canvas
 */
QwtPolarPlot* QwtPolarPicker::plot()
{
    QwtPolarCanvas* w = canvas();
    if (w)
        return w->plot();

    return nullptr;
}

/**
 * @brief Get the plot widget containing the observed plot canvas (const version)
 * @return Plot widget, containing the observed plot canvas
 */
const QwtPolarPlot* QwtPolarPicker::plot() const
{
    const QwtPolarCanvas* w = canvas();
    if (w)
        return w->plot();

    return nullptr;
}

/*!
   Translate a pixel position into a position string

   @param pos Position in pixel coordinates
   @return Position string
 */
QwtText QwtPolarPicker::trackerText(const QPoint& pos) const
{
    const QwtPointPolar polarPoint = invTransform(pos);
    return trackerTextPolar(polarPoint);
}

/*!
   @brief Translate a position into a position string

   In case of HLineRubberBand the label is the value of the
   y position, in case of VLineRubberBand the value of the x position.
   Otherwise the label contains x and y position separated by a ',' .

   The format for the double to string conversion is "%.4f".

   @param pos Position
   @return Position string
 */
QwtText QwtPolarPicker::trackerTextPolar(const QwtPointPolar& pos) const
{
    const QString text = QString::number(pos.radius(), 'f', 4) + ", " + QString::number(pos.azimuth(), 'f', 4);

    return QwtText(text);
}

/*!
   Append a point to the selection and update rubberband and tracker.

   @param pos Additional point
   @sa isActive, begin(), end(), move(), appended()

   @note The appended(const QPoint &), appended(const QDoublePoint &)
        signals are emitted.
 */
void QwtPolarPicker::append(const QPoint& pos)
{
    QwtPicker::append(pos);
    Q_EMIT appended(invTransform(pos));
}

/*!
   Move the last point of the selection

   @param pos New position
   @sa isActive, begin(), end(), append()

   @note The moved(const QPoint &), moved(const QDoublePoint &)
        signals are emitted.
 */
void QwtPolarPicker::move(const QPoint& pos)
{
    QwtPicker::move(pos);
    Q_EMIT moved(invTransform(pos));
}

/*!
   Close a selection setting the state to inactive.

   @param ok If true, complete the selection and emit selected signals
            otherwise discard the selection.
   @return true if the selection is accepted, false otherwise
 */

bool QwtPolarPicker::end(bool ok)
{
    ok = QwtPicker::end(ok);
    if (!ok)
        return false;

    QwtPolarPlot* plot = QwtPolarPicker::plot();
    if (!plot)
        return false;

    const QPolygon points = selection();
    if (points.count() == 0)
        return false;

    QwtPickerMachine::SelectionType selectionType = QwtPickerMachine::NoSelection;

    if (stateMachine())
        selectionType = stateMachine()->selectionType();

    switch (selectionType) {
    case QwtPickerMachine::PointSelection: {
        const QwtPointPolar pos = invTransform(points[ 0 ]);
        Q_EMIT selected(pos);
        break;
    }
    case QwtPickerMachine::RectSelection:
    case QwtPickerMachine::PolygonSelection: {
        QVector< QwtPointPolar > polarPoints(points.count());
        for (int i = 0; i < points.count(); i++)
            polarPoints[ i ] = invTransform(points[ i ]);

        Q_EMIT selected(polarPoints);
    }
    default:
        break;
    }

    return true;
}

/*!
    Translate a point from widget into plot coordinates

    @param pos Point in widget coordinates of the plot canvas
    @return Point in plot coordinates
    @sa transform(), canvas()
 */
QwtPointPolar QwtPolarPicker::invTransform(const QPoint& pos) const
{
    QwtPointPolar polarPos;
    if (canvas() == nullptr)
        return QwtPointPolar();

    return canvas()->invTransform(pos);
}

/**
 * @brief Get the bounding rectangle of the region where picking is supported
 * @return Bounding rectangle of the region, where picking is supported
 */
QRect QwtPolarPicker::pickRect() const
{
    const QRect cr = canvas()->contentsRect();
    const QRect pr = plot()->plotRect(cr).toRect();

    return cr & pr;
}

QPainterPath QwtPolarPicker::pickArea() const
{
    const QRect cr = canvas()->contentsRect();

    QPainterPath crPath;
    crPath.addRect(cr);

    QPainterPath prPath;
    prPath.addEllipse(plot()->plotRect(cr));

    return crPath.intersected(prPath);
}
