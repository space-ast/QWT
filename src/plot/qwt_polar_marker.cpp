/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_marker.h"
#include "qwt_polar.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <qpainter.h>

static const int cs_polarMarker_labelDist = 2;

class QwtPolarMarker::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPolarMarker)
public:
    PrivateData(QwtPolarMarker* p) : q_ptr(p), align(Qt::AlignCenter)
    {
        symbol = new QwtSymbol();
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtText label;
    Qt::Alignment align;
    QPen pen;
    const QwtSymbol* symbol;

    QwtPointPolar pos;
};

/**
 * @brief Constructor
 * @details Sets alignment to Qt::AlignCenter, and style to NoLine.
 */
QwtPolarMarker::QwtPolarMarker() : QwtPolarItem(QwtText("Marker")), QWT_PIMPL_CONSTRUCT
{
    setItemAttribute(QwtPolarItem::AutoScale);
    setZ(30.0);
}

/**
 * @brief Destructor
 */
QwtPolarMarker::~QwtPolarMarker()
{
}

/**
 * @brief Get the runtime type information
 * @return QwtPolarItem::Rtti_PolarMarker
 */
int QwtPolarMarker::rtti() const
{
    return QwtPolarItem::Rtti_PolarMarker;
}

/**
 * @brief Get the position of the marker
 * @return Position of the marker
 * @sa setPosition()
 */
QwtPointPolar QwtPolarMarker::position() const
{
    QWT_DC(d);
    return d->pos;
}

/**
 * @brief Set the position of the marker
 * @param pos New position of the marker
 * @sa position()
 */
void QwtPolarMarker::setPosition(const QwtPointPolar& pos)
{
    QWT_D(d);

    if (d->pos != pos) {
        d->pos = pos;
        itemChanged();
    }
}

/**
 * @brief Draw the marker
 * @param painter Painter
 * @param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param radialMap Maps radius values into painter coordinates
 * @param pole Position of the pole in painter coordinates
 * @param radius Radius of the complete plot area in painter coordinates
 * @param canvasRect Contents rect of the canvas in painter coordinates
 */
void QwtPolarMarker::draw(QPainter* painter,
                          const QwtScaleMap& azimuthMap,
                          const QwtScaleMap& radialMap,
                          const QPointF& pole,
                          double radius,
                          const QRectF& canvasRect) const
{
    QWT_DC(d);

    Q_UNUSED(radius);
    Q_UNUSED(canvasRect);

    const double r = radialMap.transform(d->pos.radius());
    const double a = azimuthMap.transform(d->pos.azimuth());

    const QPointF pos = qwtPolar2Pos(pole, r, a);

    // draw symbol
    QSize sSym(0, 0);
    if (d->symbol->style() != QwtSymbol::NoSymbol) {
        sSym = d->symbol->size();
        d->symbol->drawSymbol(painter, pos);
    }

    // draw label
    if (!d->label.isEmpty()) {
        int xlw = qMax(int(d->pen.width()), 1);
        int ylw = xlw;

        int xlw1 = qMax((xlw + 1) / 2, (sSym.width() + 1) / 2) + cs_polarMarker_labelDist;
        xlw      = qMax(xlw / 2, (sSym.width() + 1) / 2) + cs_polarMarker_labelDist;
        int ylw1 = qMax((ylw + 1) / 2, (sSym.height() + 1) / 2) + cs_polarMarker_labelDist;
        ylw      = qMax(ylw / 2, (sSym.height() + 1) / 2) + cs_polarMarker_labelDist;

        QRect tr(QPoint(0, 0), d->label.textSize(painter->font()).toSize());
        tr.moveCenter(QPoint(0, 0));

        int dx = pos.x();
        int dy = pos.y();

        if (d->align & Qt::AlignTop)
            dy += tr.y() - ylw1;
        else if (d->align & Qt::AlignBottom)
            dy -= tr.y() - ylw1;

        if (d->align & Qt::AlignLeft)
            dx += tr.x() - xlw1;
        else if (d->align & Qt::AlignRight)
            dx -= tr.x() - xlw1;

        tr.translate(dx, dy);
        d->label.draw(painter, tr);
    }
}

/**
 * @brief Assign a symbol
 * @param symbol New symbol (ownership is transferred)
 * @sa symbol(), QwtSymbol
 */
void QwtPolarMarker::setSymbol(const QwtSymbol* symbol)
{
    QWT_D(d);

    if (d->symbol != symbol) {
        delete d->symbol;
        d->symbol = symbol;
        itemChanged();
    }
}

/**
 * @brief Get the current symbol
 * @return The current symbol
 * @sa setSymbol(), QwtSymbol
 */
const QwtSymbol* QwtPolarMarker::symbol() const
{
    QWT_DC(d);
    return d->symbol;
}

/**
 * @brief Set the label text
 * @param label Label text
 * @sa label()
 */
void QwtPolarMarker::setLabel(const QwtText& label)
{
    QWT_D(d);

    if (label != d->label) {
        d->label = label;
        itemChanged();
    }
}

/**
 * @brief Get the label text
 * @return The label text
 * @sa setLabel()
 */
QwtText QwtPolarMarker::label() const
{
    QWT_DC(d);
    return d->label;
}

/**
 * @brief Set the alignment of the label
 * @param align Alignment. A combination of AlignTop, AlignBottom, AlignLeft, AlignRight, AlignCenter, AlignHCenter, AlignVCenter.
 * @details The alignment determines where the label is drawn relative to the marker's position.
 * @sa labelAlignment()
 */
void QwtPolarMarker::setLabelAlignment(Qt::Alignment align)
{
    QWT_D(d);

    if (align == d->align)
        return;

    d->align = align;
    itemChanged();
}

/**
 * @brief Get the label alignment
 * @return The label alignment
 * @sa setLabelAlignment()
 */
Qt::Alignment QwtPolarMarker::labelAlignment() const
{
    QWT_DC(d);
    return d->align;
}

/**
 * @brief Get the bounding interval necessary to display the item
 * @param scaleId Scale index
 * @return Bounding interval (equals position)
 * @details This interval can be useful for operations like clipping or autoscaling.
 * @sa position()
 */
QwtInterval QwtPolarMarker::boundingInterval(int scaleId) const
{
    QWT_DC(d);

    const double v = (scaleId == QwtPolar::ScaleRadius) ? d->pos.radius() : d->pos.azimuth();

    return QwtInterval(v, v);
}
