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

#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_math.h"

#include <qpainter.h>

class QwtRoundScaleDraw::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtRoundScaleDraw)
public:
    PrivateData(QwtRoundScaleDraw* p) : q_ptr(p), center(50.0, 50.0), radius(50.0), startAngle(-135.0), endAngle(135.0)
    {
    }

    QPointF center;
    double radius;

    double startAngle;
    double endAngle;
};

/**
 * @brief Constructor
 * @details The range of the scale is initialized to [0, 100], the center is set to (50, 50)
 *          with a radius of 50. The angle range is set to [-135, 135].
 */
QwtRoundScaleDraw::QwtRoundScaleDraw() : QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    setRadius(50);
    scaleMap().setPaintInterval(d->startAngle, d->endAngle);
}

//! Destructor
QwtRoundScaleDraw::~QwtRoundScaleDraw()
{
}

/**
 * @brief Change the radius of the scale
 * @param radius New radius
 * @details Radius is the radius of the backbone without ticks and labels.
 * @sa moveCenter()
 */
void QwtRoundScaleDraw::setRadius(double radius)
{
    QWT_D(d);
    d->radius = radius;
}

/**
 * @brief Get the radius
 * @return Radius of the scale
 * @details Radius is the radius of the backbone without ticks and labels.
 * @sa setRadius(), extent()
 */
double QwtRoundScaleDraw::radius() const
{
    QWT_DC(d);
    return d->radius;
}

/**
 * @brief Move the center of the scale draw, leaving the radius unchanged
 * @param center New center
 * @sa setRadius()
 */
void QwtRoundScaleDraw::moveCenter(const QPointF& center)
{
    QWT_D(d);
    d->center = center;
}

/**
 * @brief Get the center of the scale
 * @return Center point of the scale
 * @sa moveCenter()
 */
QPointF QwtRoundScaleDraw::center() const
{
    QWT_DC(d);
    return d->center;
}

/**
 * @brief Adjust the baseline circle segment for round scales
 * @param angle1 First boundary of the angle interval in degrees
 * @param angle2 Second boundary of the angle interval in degrees
 * @details The baseline will be drawn from min(angle1,angle2) to max(angle1, angle2).
 *          The default setting is [ -135, 135 ]. An angle of 0 degrees corresponds to
 *          the 12 o'clock position, and positive angles count in a clockwise direction.
 *
 * @warning
 * - The angle range is limited to [-360, 360] degrees. Angles exceeding this range will be clipped.
 * - For angles more or equal than 360 degrees above or below min(angle1, angle2), scale marks will not be drawn.
 * - If you need a counterclockwise scale, use QwtScaleDiv::setInterval()
 */
void QwtRoundScaleDraw::setAngleRange(double angle1, double angle2)
{
    QWT_D(d);
#if 0
    angle1 = qBound( -360.0, angle1, 360.0 );
    angle2 = qBound( -360.0, angle2, 360.0 );
#endif

    d->startAngle = angle1;
    d->endAngle   = angle2;

    if (d->startAngle == d->endAngle) {
        d->startAngle -= 1;
        d->endAngle += 1;
    }

    scaleMap().setPaintInterval(d->startAngle, d->endAngle);
}

/**
 * @brief Draws the label for a major scale tick
 * @param painter Painter
 * @param value Value
 * @sa drawTick(), drawBackbone()
 */
void QwtRoundScaleDraw::drawLabel(QPainter* painter, double value) const
{
    QWT_DC(d);
    const double tval = scaleMap().transform(value);
    if ((tval >= d->startAngle + 360.0) || (tval <= d->startAngle - 360.0)) {
        return;
    }

    const QwtText label = tickLabel(painter->font(), value);
    if (label.isEmpty())
        return;

    double radius = d->radius;
    if (hasComponent(QwtAbstractScaleDraw::Ticks) || hasComponent(QwtAbstractScaleDraw::Backbone)) {
        radius += spacing();
    }

    if (hasComponent(QwtAbstractScaleDraw::Ticks))
        radius += tickLength(QwtScaleDiv::MajorTick);

    const QSizeF sz  = label.textSize(painter->font());
    const double arc = qwtRadians(tval);

    const double x = d->center.x() + (radius + sz.width() / 2.0) * std::sin(arc);
    const double y = d->center.y() - (radius + sz.height() / 2.0) * std::cos(arc);

    const QRectF r(x - sz.width() / 2, y - sz.height() / 2, sz.width(), sz.height());
    label.draw(painter, r);
}

/**
 * @brief Draw a tick
 * @param painter Painter
 * @param value Value of the tick
 * @param len Length of the tick
 * @sa drawBackbone(), drawLabel()
 */
void QwtRoundScaleDraw::drawTick(QPainter* painter, double value, double len) const
{
    QWT_DC(d);
    if (len <= 0)
        return;

    const double tval = scaleMap().transform(value);

    const double cx     = d->center.x();
    const double cy     = d->center.y();
    const double radius = d->radius;

    if ((tval < d->startAngle + 360.0) && (tval > d->startAngle - 360.0)) {
        const double arc = qwtRadians(tval);

        const double sinArc = std::sin(arc);
        const double cosArc = std::cos(arc);

        const double x1 = cx + radius * sinArc;
        const double x2 = cx + (radius + len) * sinArc;
        const double y1 = cy - radius * cosArc;
        const double y2 = cy - (radius + len) * cosArc;

        QwtPainter::drawLine(painter, x1, y1, x2, y2);
    }
}

/**
 * @brief Draws the baseline of the scale
 * @param painter Painter
 * @sa drawTick(), drawLabel()
 */
void QwtRoundScaleDraw::drawBackbone(QPainter* painter) const
{
    QWT_DC(d);
    const double deg1 = scaleMap().p1();
    const double deg2 = scaleMap().p2();

    const int a1 = qRound(qwtMinF(deg1, deg2) - 90);
    const int a2 = qRound(qwtMaxF(deg1, deg2) - 90);

    const double radius = d->radius;
    const double x      = d->center.x() - radius;
    const double y      = d->center.y() - radius;

    painter->drawArc(QRectF(x, y, 2 * radius, 2 * radius), -a2 * 16, (a2 - a1 + 1) * 16);  // counterclockwise
}

/**
 * @brief Calculate the extent of the scale
 * @param font Font used for painting the labels
 * @return Calculated extent
 * @details The extent is the distance between the baseline to the outermost pixel of the scale draw.
 *          radius() + extent() is an upper limit for the radius of the bounding circle.
 *
 * @warning The implemented algorithm is not too smart and calculates only an upper limit,
 *          that might be a few pixels too large.
 * @sa setMinimumExtent(), minimumExtent()
 */
double QwtRoundScaleDraw::extent(const QFont& font) const
{
    QWT_DC(d);
    double extent = 0.0;

    if (hasComponent(QwtAbstractScaleDraw::Labels)) {
        const QwtScaleDiv& sd        = scaleDiv();
        const QList< double >& ticks = sd.ticks(QwtScaleDiv::MajorTick);
        for (int i = 0; i < ticks.count(); i++) {
            const double value = ticks[ i ];
            if (!sd.contains(value))
                continue;

            const double tval = scaleMap().transform(value);
            if ((tval < d->startAngle + 360) && (tval > d->startAngle - 360)) {
                const QwtText label = tickLabel(font, value);
                if (label.isEmpty())
                    continue;

                const double arc = qwtRadians(tval);

                const QSizeF sz  = label.textSize(font);
                const double off = qMax(sz.width(), sz.height());

                double x = off * std::sin(arc);
                double y = off * std::cos(arc);

                const double dist = std::sqrt(x * x + y * y);
                if (dist > extent)
                    extent = dist;
            }
        }
    }

    if (hasComponent(QwtAbstractScaleDraw::Ticks)) {
        extent += maxTickLength();
    }

    if (hasComponent(QwtAbstractScaleDraw::Backbone)) {
        extent += qwtMaxF(penWidthF(), 1.0);
    }

    if (hasComponent(QwtAbstractScaleDraw::Labels)
        && (hasComponent(QwtAbstractScaleDraw::Ticks) || hasComponent(QwtAbstractScaleDraw::Backbone))) {
        extent += spacing();
    }

    extent = qwtMaxF(extent, minimumExtent());

    return extent;
}
