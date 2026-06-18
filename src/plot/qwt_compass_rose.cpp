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

#include "qwt_compass_rose.h"
#include "qwt_point_polar.h"

#include <qpainter.h>
#include <qpainterpath.h>

static QPointF qwtIntersection(QPointF p11, QPointF p12, QPointF p21, QPointF p22)
{
    const QLineF line1(p11, p12);
    const QLineF line2(p21, p22);

    QPointF pos;
#if QT_VERSION >= 0x050e00
    if (line1.intersects(line2, &pos) == QLineF::NoIntersection)
#else
    if (line1.intersect(line2, &pos) == QLineF::NoIntersection)
#endif
        return QPointF();

    return pos;
}

/**
 *   @brief Constructor
 */
QwtCompassRose::QwtCompassRose()
{
}

/**
 *   @brief Destructor
 */
QwtCompassRose::~QwtCompassRose()
{
}

/**
 *   @brief Assign a palette
 *   @param[in] p Palette to assign
 */
void QwtCompassRose::setPalette(const QPalette& p)
{
    m_palette = p;
}

/**
 *   @brief Get the current palette
 *   @return Current palette
 */
const QPalette& QwtCompassRose::palette() const
{
    return m_palette;
}

class QwtSimpleCompassRose::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtSimpleCompassRose)
public:
    PrivateData(QwtSimpleCompassRose* p)
        : q_ptr(p), width(0.2), numThorns(8), numThornLevels(-1), shrinkFactor(0.9), flatStyle(true)
    {
    }

    double width;
    int numThorns;
    int numThornLevels;
    double shrinkFactor;
    bool flatStyle;
};

/**
 *   @brief Constructor
 *   @param[in] numThorns Number of thorns
 *   @param[in] numThornLevels Number of thorn levels (-1 means auto)
 */
QwtSimpleCompassRose::QwtSimpleCompassRose(int numThorns, int numThornLevels) : QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    d->numThorns      = numThorns;
    d->numThornLevels = numThornLevels;

    const QColor dark(128, 128, 255);
    const QColor light(192, 255, 255);

    QPalette palette;
    palette.setColor(QPalette::Dark, dark);
    palette.setColor(QPalette::Light, light);

    setPalette(palette);
}

/**
 *   @brief Destructor
 */
QwtSimpleCompassRose::~QwtSimpleCompassRose()
{
}

/**
 *   @brief Set the shrink factor for thorns with each level
 *   @param[in] factor Shrink factor (default: 0.9)
 *   @sa shrinkFactor()
 */
void QwtSimpleCompassRose::setShrinkFactor(double factor)
{
    QWT_D(d);
    d->shrinkFactor = factor;
}

/**
 *   @brief Get the shrink factor for thorns with each level
 *   @return Shrink factor
 *   @sa setShrinkFactor()
 */
double QwtSimpleCompassRose::shrinkFactor() const
{
    QWT_DC(d);
    return d->shrinkFactor;
}

/**
 *   @brief Draw the rose
 *   @param[in] painter Painter
 *   @param[in] center Center point
 *   @param[in] radius Radius of the rose
 *   @param[in] north Position pointing north
 *   @param[in] cg Color group
 */
void QwtSimpleCompassRose::draw(QPainter* painter, const QPointF& center, double radius, double north, QPalette::ColorGroup cg) const
{
    QWT_DC(d);
    QPalette pal = palette();
    pal.setCurrentColorGroup(cg);

    drawRose(painter, pal, center, radius, north, d->width, d->numThorns, d->numThornLevels, d->shrinkFactor, d->flatStyle);
}

/**
 *   @brief Set flat style
 *   @details When enabled (default), thorns are drawn with a single solid color
 *            instead of the two-tone Light/Dark 3D effect.
 *   @param on true for flat style, false for classic 3D style
 *   @sa flatStyle()
 */
void QwtSimpleCompassRose::setFlatStyle(bool on)
{
    QWT_D(d);
    d->flatStyle = on;
}

/**
 *   @brief Return whether flat style is enabled
 *   @sa setFlatStyle()
 */
bool QwtSimpleCompassRose::flatStyle() const
{
    QWT_DC(d);
    return d->flatStyle;
}

/**
 *   @brief Static helper to draw a rose with specified parameters
 *   @param[in] painter Painter
 *   @param[in] palette Palette for drawing
 *   @param[in] center Center of the rose
 *   @param[in] radius Radius of the rose
 *   @param[in] north Position pointing to north
 *   @param[in] width Width of the rose heads
 *   @param[in] numThorns Number of thorns
 *   @param[in] numThornLevels Number of thorn levels
 *   @param[in] shrinkFactor Factor to shrink the thorns with each level
 */
void QwtSimpleCompassRose::drawRose(QPainter* painter,
                                    const QPalette& palette,
                                    const QPointF& center,
                                    double radius,
                                    double north,
                                    double width,
                                    int numThorns,
                                    int numThornLevels,
                                    double shrinkFactor,
                                    bool flatStyle)
{
    if (numThorns < 4)
        numThorns = 4;

    if (numThorns % 4)
        numThorns += 4 - numThorns % 4;

    if (numThornLevels <= 0)
        numThornLevels = numThorns / 4;

    if (shrinkFactor >= 1.0)
        shrinkFactor = 1.0;

    if (shrinkFactor <= 0.5)
        shrinkFactor = 0.5;

    painter->save();

    painter->setPen(Qt::NoPen);

    for (int j = 1; j <= numThornLevels; j++) {
        double step = std::pow(2.0, j) * M_PI / numThorns;
        if (step > M_PI_2)
            break;

        double r = radius;
        for (int k = 0; k < 3; k++) {
            if (j + k < numThornLevels)
                r *= shrinkFactor;
        }

        double leafWidth = r * width;
        if (2.0 * M_PI / step > 32)
            leafWidth = 16;

        const double origin = qwtRadians(north);
        for (double angle = origin; angle < 2.0 * M_PI + origin; angle += step) {
            const QPointF p  = qwtPolar2Pos(center, r, angle);
            const QPointF p1 = qwtPolar2Pos(center, leafWidth, angle + M_PI_2);
            const QPointF p2 = qwtPolar2Pos(center, leafWidth, angle - M_PI_2);
            const QPointF p3 = qwtPolar2Pos(center, r, angle + step / 2.0);
            const QPointF p4 = qwtPolar2Pos(center, r, angle - step / 2.0);

            if (flatStyle) {
                QPainterPath path;
                path.moveTo(center);
                path.lineTo(qwtIntersection(center, p3, p1, p));
                path.lineTo(p);
                path.lineTo(qwtIntersection(center, p4, p2, p));
                path.closeSubpath();

                painter->setBrush(palette.brush(QPalette::Mid));
                painter->drawPath(path);
            } else {
                QPainterPath darkPath;
                darkPath.moveTo(center);
                darkPath.lineTo(p);
                darkPath.lineTo(qwtIntersection(center, p3, p1, p));

                painter->setBrush(palette.brush(QPalette::Dark));
                painter->drawPath(darkPath);

                QPainterPath lightPath;
                lightPath.moveTo(center);
                lightPath.lineTo(p);
                lightPath.lineTo(qwtIntersection(center, p4, p2, p));

                painter->setBrush(palette.brush(QPalette::Light));
                painter->drawPath(lightPath);
            }
        }
    }
    painter->restore();
}

/**
 *   @brief Set the width of the rose heads
 *   @param[in] width Width (range: 0.03 to 0.4, lower values make thinner heads)
 */
void QwtSimpleCompassRose::setWidth(double width)
{
    QWT_D(d);
    d->width = width;
    if (d->width < 0.03)
        d->width = 0.03;

    if (d->width > 0.4)
        d->width = 0.4;
}

/**
 *   @brief Get the width of the rose heads
 *   @return Width of the rose heads
 *   @sa setWidth()
 */
double QwtSimpleCompassRose::width() const
{
    QWT_DC(d);
    return d->width;
}

/**
 *   @brief Set the number of thorns on one level
 *   @param[in] numThorns Number of thorns (aligned to multiple of 4, minimum 4)
 *   @sa numThorns(), setNumThornLevels()
 */
void QwtSimpleCompassRose::setNumThorns(int numThorns)
{
    QWT_D(d);
    if (numThorns < 4)
        numThorns = 4;

    if (numThorns % 4)
        numThorns += 4 - numThorns % 4;

    d->numThorns = numThorns;
}

/**
 *   @brief Get the number of thorns
 *   @return Number of thorns
 *   @sa setNumThorns(), setNumThornLevels()
 */
int QwtSimpleCompassRose::numThorns() const
{
    QWT_DC(d);
    return d->numThorns;
}

/**
 *   @brief Set the number of thorn levels
 *   @param[in] numThornLevels Number of thorn levels
 *   @sa setNumThorns(), numThornLevels()
 */
void QwtSimpleCompassRose::setNumThornLevels(int numThornLevels)
{
    QWT_D(d);
    d->numThornLevels = numThornLevels;
}

/**
 *   @brief Get the number of thorn levels
 *   @return Number of thorn levels
 *   @sa setNumThorns(), setNumThornLevels()
 */
int QwtSimpleCompassRose::numThornLevels() const
{
    QWT_DC(d);
    return d->numThornLevels;
}
