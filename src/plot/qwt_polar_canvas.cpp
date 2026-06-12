/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_canvas.h"
#include "qwt_polar_plot.h"
#include "qwt_painter.h"

#include <qpainter.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qstyleoption.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

class QwtPolarCanvas::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPolarCanvas)
  public:
    PrivateData(QwtPolarCanvas* p) : q_ptr(p), backingStore(nullptr)
    {
    }

    ~PrivateData()
    {
        delete backingStore;
    }

    QwtPolarCanvas::PaintAttributes paintAttributes;
    QPixmap* backingStore;
};

/**
 * @brief Constructor
 * @param plot Parent polar plot widget
 */
QwtPolarCanvas::QwtPolarCanvas(QwtPolarPlot* plot) : QFrame(plot), QWT_PIMPL_CONSTRUCT
{
#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif
    setFocusPolicy(Qt::WheelFocus);

    setPaintAttribute(BackingStore, true);
}

/**
 * @brief Destructor
 */
QwtPolarCanvas::~QwtPolarCanvas()
{
}

/**
 * @brief Get the parent plot widget
 * @return Parent polar plot widget
 */
QwtPolarPlot* QwtPolarCanvas::plot()
{
    return qobject_cast< QwtPolarPlot* >(parent());
}

/**
 * @brief Get the parent plot widget (const version)
 * @return Parent polar plot widget
 */
const QwtPolarPlot* QwtPolarCanvas::plot() const
{
    return qobject_cast< QwtPolarPlot* >(parent());
}

/**
 * @brief Change the paint attributes
 * @param attribute Paint attribute to modify
 * @param on True to enable, false to disable
 * @details The default setting enables BackingStore.
 * @sa testPaintAttribute(), backingStore()
 */
void QwtPolarCanvas::setPaintAttribute(PaintAttribute attribute, bool on)
{
    QWT_D(d);

    if (bool(d->paintAttributes & attribute) == on)
        return;

    if (on)
        d->paintAttributes |= attribute;
    else
        d->paintAttributes &= ~attribute;

    switch (attribute) {
    case BackingStore: {
        if (on) {
            if (d->backingStore == nullptr)
                d->backingStore = new QPixmap();

            if (isVisible()) {
                const QRect cr = contentsRect();
#if QT_VERSION >= 0x050000
                *d->backingStore = grab(cr);
#else
                *d->backingStore = QPixmap::grabWidget(this, cr);
#endif
            }
        } else {
            delete d->backingStore;
            d->backingStore = nullptr;
        }
        break;
    }
    }
}

/**
 * @brief Test whether a paint attribute is enabled
 * @param attribute Paint attribute to test
 * @return True if the attribute is enabled
 * @sa setPaintAttribute()
 */
bool QwtPolarCanvas::testPaintAttribute(PaintAttribute attribute) const
{
    QWT_DC(d);
    return (d->paintAttributes & attribute) != 0;
}

/**
 * @brief Get the backing store pixmap
 * @return Backing store pixmap, might be null if not enabled
 * @sa setPaintAttribute(), invalidateBackingStore()
 */
const QPixmap* QwtPolarCanvas::backingStore() const
{
    QWT_DC(d);
    return d->backingStore;
}

/**
 * @brief Invalidate the internal backing store
 * @details Clears the backing store pixmap, forcing a full repaint on the next paint event.
 * @sa backingStore()
 */
void QwtPolarCanvas::invalidateBackingStore()
{
    QWT_D(d);
    if (d->backingStore)
        *d->backingStore = QPixmap();
}

/*!
   Paint event
   @param event Paint event
 */
void QwtPolarCanvas::paintEvent(QPaintEvent* event)
{
    QWT_D(d);

    QPainter painter(this);
    painter.setClipRegion(event->region());

    if ((d->paintAttributes & BackingStore) && d->backingStore != nullptr) {
        QPixmap& bs = *d->backingStore;
        if (bs.size() != size()) {
            bs = QPixmap(size());
#ifdef Q_WS_X11
            if (bs.x11Info().screen() != x11Info().screen())
                bs.x11SetScreen(x11Info().screen());
#endif

            QPainter p;

            if (testAttribute(Qt::WA_StyledBackground)) {
                p.begin(&bs);
                QwtPainter::drawStyledBackground(this, &p);
            } else {
                if (autoFillBackground()) {
                    p.begin(&bs);
                    p.fillRect(rect(), palette().brush(backgroundRole()));
                } else {
                    QWidget* bgWidget = QwtPainter::findBackgroundWidget(plot());

                    QwtPainter::fillPixmap(bgWidget, bs, mapTo(bgWidget, rect().topLeft()));

                    p.begin(&bs);
                }
            }

            plot()->drawCanvas(&p, contentsRect());

            if (frameWidth() > 0)
                drawFrame(&p);
        }

        painter.drawPixmap(0, 0, *d->backingStore);
    } else {
        QwtPainter::drawStyledBackground(this, &painter);

        plot()->drawCanvas(&painter, contentsRect());

        if (frameWidth() > 0)
            drawFrame(&painter);
    }
}

/*!
   Resize event
   @param event Resize event
 */
void QwtPolarCanvas::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++)
        plot()->updateScale(scaleId);
}

/**
 * @brief Translate a point from widget coordinates to polar coordinates
 * @param pos Point in widget coordinates of the plot canvas
 * @return Point in polar coordinates
 * @sa transform()
 */
QwtPointPolar QwtPolarCanvas::invTransform(const QPoint& pos) const
{
    const QwtPolarPlot* pl = plot();

    const QwtScaleMap azimuthMap = pl->scaleMap(QwtPolar::Azimuth);
    const QwtScaleMap radialMap  = pl->scaleMap(QwtPolar::Radius);

    const QPointF center = pl->plotRect().center();

    double dx = pos.x() - center.x();
    double dy = -(pos.y() - center.y());

    const QwtPointPolar polarPos = QwtPointPolar(QPoint(dx, dy)).normalized();

    double azimuth = azimuthMap.invTransform(polarPos.azimuth());

    // normalize the azimuth
    double min = azimuthMap.s1();
    double max = azimuthMap.s2();
    if (max < min)
        qSwap(min, max);

    if (azimuth < min) {
        azimuth += max - min;
    } else if (azimuth > max) {
        azimuth -= max - min;
    }

    const double radius = radialMap.invTransform(polarPos.radius());

    return QwtPointPolar(azimuth, radius);
}

/**
 * @brief Translate a point from polar coordinates to widget coordinates
 * @param polarPos Point in polar coordinates
 * @return Point in widget coordinates
 * @sa invTransform()
 */
QPoint QwtPolarCanvas::transform(const QwtPointPolar& polarPos) const
{
    const QwtPolarPlot* pl = plot();

    const QwtScaleMap azimuthMap = pl->scaleMap(QwtPolar::Azimuth);
    const QwtScaleMap radialMap  = pl->scaleMap(QwtPolar::Radius);

    const double radius  = radialMap.transform(polarPos.radius());
    const double azimuth = azimuthMap.transform(polarPos.azimuth());

    const QPointF pos = qwtPolar2Pos(pl->plotRect().center(), radius, azimuth);

    return pos.toPoint();
}
