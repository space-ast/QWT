/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_magnifier.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_point_polar.h"

#include <qevent.h>

class QwtPolarMagnifier::PrivateData
{
public:
    PrivateData() : unzoomKey(Qt::Key_Home), unzoomKeyModifiers(Qt::NoModifier)
    {
    }

    int unzoomKey;
    int unzoomKeyModifiers;
};

/**
 * @brief Constructor
 * @param canvas Plot canvas to be magnified
 */
QwtPolarMagnifier::QwtPolarMagnifier(QwtPolarCanvas* canvas) : QwtMagnifier(canvas)
{
    m_data = new PrivateData();
}

/**
 * @brief Destructor
 */
QwtPolarMagnifier::~QwtPolarMagnifier()
{
    delete m_data;
}

/**
 * @brief Assign key and modifiers that are used for unzooming
 * @details The default combination is Qt::Key_Home + Qt::NoModifier.
 * @param key Key code
 * @param modifiers Modifiers
 * @sa getUnzoomKey(), QwtPolarPlot::unzoom()
 */
void QwtPolarMagnifier::setUnzoomKey(int key, int modifiers)
{
    m_data->unzoomKey          = key;
    m_data->unzoomKeyModifiers = modifiers;
}

/**
 * @brief Get the key and modifiers that are used for unzooming
 * @param[out] key Key code
 * @param[out] modifiers Modifiers
 * @sa setUnzoomKey(), QwtPolarPlot::unzoom()
 */
void QwtPolarMagnifier::getUnzoomKey(int& key, int& modifiers) const
{
    key       = m_data->unzoomKey;
    modifiers = m_data->unzoomKeyModifiers;
}

/**
 * @brief Get the observed plot canvas
 * @return Observed plot canvas
 */
QwtPolarCanvas* QwtPolarMagnifier::canvas()
{
    return qobject_cast< QwtPolarCanvas* >(parent());
}

/**
 * @brief Get the observed plot canvas (const version)
 * @return Observed plot canvas
 */
const QwtPolarCanvas* QwtPolarMagnifier::canvas() const
{
    return qobject_cast< const QwtPolarCanvas* >(parent());
}

/**
 * @brief Get the observed plot
 * @return Observed plot
 */
QwtPolarPlot* QwtPolarMagnifier::plot()
{
    QwtPolarCanvas* c = canvas();
    if (c)
        return c->plot();

    return nullptr;
}

/**
 * @brief Get the observed plot (const version)
 * @return Observed plot
 */
const QwtPolarPlot* QwtPolarMagnifier::plot() const
{
    const QwtPolarCanvas* c = canvas();
    if (c)
        return c->plot();

    return nullptr;
}

/*!
   Handle a key press event for the observed widget.

   \param event Key event
 */
void QwtPolarMagnifier::widgetKeyPressEvent(QKeyEvent* event)
{
    const int key   = event->key();
    const int state = event->modifiers();

    if (key == m_data->unzoomKey && state == m_data->unzoomKeyModifiers) {
        unzoom();
        return;
    }

    QwtMagnifier::widgetKeyPressEvent(event);
}

/**
 * @brief Zoom in/out the zoomed area
 * @param factor A value < 1.0 zooms in, a value > 1.0 zooms out
 */
void QwtPolarMagnifier::rescale(double factor)
{
    factor = qAbs(factor);
    if (factor == 1.0 || factor == 0.0)
        return;

    QwtPolarPlot* plt = plot();
    if (plt == nullptr)
        return;

    QwtPointPolar zoomPos;
    double newZoomFactor = plt->zoomFactor() * factor;

    if (newZoomFactor >= 1.0)
        newZoomFactor = 1.0;
    else
        zoomPos = plt->zoomPos();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->zoom(zoomPos, newZoomFactor);

    plt->setAutoReplot(autoReplot);
    plt->replot();
}

/**
 * @brief Unzoom the plot widget
 */
void QwtPolarMagnifier::unzoom()
{
    QwtPolarPlot* plt = plot();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->unzoom();

    plt->setAutoReplot(autoReplot);
    plt->replot();
}
