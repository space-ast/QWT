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

#include "qwt_thermo.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qevent.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qmargins.h>

#include <algorithm>
#include <functional>

static inline void qwtDrawLine(QPainter* painter,
                               int pos,
                               const QColor& color,
                               const QRect& pipeRect,
                               const QRect& liquidRect,
                               Qt::Orientation orientation)
{
    painter->setPen(color);
    if (orientation == Qt::Horizontal) {
        if (pos >= liquidRect.left() && pos < liquidRect.right())
            painter->drawLine(pos, pipeRect.top(), pos, pipeRect.bottom());
    } else {
        if (pos >= liquidRect.top() && pos < liquidRect.bottom())
            painter->drawLine(pipeRect.left(), pos, pipeRect.right(), pos);
    }
}

static QVector< double > qwtTickList(const QwtScaleDiv& scaleDiv)
{
    QVector< double > values;

    double lowerLimit = scaleDiv.interval().minValue();
    double upperLimit = scaleDiv.interval().maxValue();

    if (upperLimit < lowerLimit)
        qSwap(lowerLimit, upperLimit);

    values += lowerLimit;

    for (int tickType = QwtScaleDiv::MinorTick; tickType < QwtScaleDiv::NTickTypes; tickType++) {
        const QList< double > ticks = scaleDiv.ticks(tickType);

        for (int i = 0; i < ticks.count(); i++) {
            const double v = ticks[ i ];
            if (v > lowerLimit && v < upperLimit)
                values += v;
        }
    }

    values += upperLimit;

    return values;
}

class QwtThermo::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtThermo)
public:
    PrivateData(QwtThermo* p)
        : q_ptr(p)
        , orientation(Qt::Vertical)
        , scalePosition(QwtThermo::TrailingScale)
        , spacing(3)
        , borderWidth(2)
        , pipeWidth(10)
        , alarmLevel(0.0)
        , alarmEnabled(false)
        , autoFillPipe(true)
        , flatStyle(true)
        , originMode(QwtThermo::OriginMinimum)
        , origin(0.0)
        , colorMap(nullptr)
        , value(0.0)
    {
        rangeFlags = QwtInterval::IncludeBorders;
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    Qt::Orientation orientation;
    QwtThermo::ScalePosition scalePosition;

    int spacing;
    int borderWidth;
    int pipeWidth;

    QwtInterval::BorderFlags rangeFlags;
    double alarmLevel;
    bool alarmEnabled;
    bool autoFillPipe;
    bool flatStyle;
    QwtThermo::OriginMode originMode;
    double origin;

    QwtColorMap* colorMap;

    double value;
};

/**
 * @brief Constructor
 * @param parent Parent widget
 * @sa ~QwtThermo()
 */
QwtThermo::QwtThermo(QWidget* parent) : QwtAbstractScale(parent), QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    if (d->orientation == Qt::Vertical)
        policy.transpose();

    setSizePolicy(policy);

    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    layoutThermo(true);
}

/**
 * @brief Destructor
 * @sa QwtThermo()
 */
QwtThermo::~QwtThermo()
{
}

/**
 * @brief Exclude/Include min/max values
 * @details According to the flags minValue() and maxValue() are included/excluded
 *          from the pipe. In case of an excluded value the corresponding tick
 *          is painted 1 pixel off of the pipeRect().
 *          For example, when a minimum of 0.0 has to be displayed as an empty pipe,
 *          the minValue() needs to be excluded.
 * @param flags Range flags
 * @sa rangeFlags()
 */
void QwtThermo::setRangeFlags(QwtInterval::BorderFlags flags)
{
    QWT_D(d);
    if (d->rangeFlags != flags) {
        d->rangeFlags = flags;
        update();
    }
}

/**
 * @brief Return range flags
 * @sa setRangeFlags()
 */
QwtInterval::BorderFlags QwtThermo::rangeFlags() const
{
    QWT_DC(d);
    return d->rangeFlags;
}

/**
 * @brief Set the current value
 * @param value New value
 * @sa value()
 */
void QwtThermo::setValue(double value)
{
    QWT_D(d);
    if (d->value != value) {
        d->value = value;
        update();
    }
}

/**
 * @brief Return the value
 * @sa setValue()
 */
double QwtThermo::value() const
{
    QWT_DC(d);
    return d->value;
}

/**
 * @brief Set a scale draw
 * @details For changing the labels of the scales, it is necessary to derive
 *          from QwtScaleDraw and overload QwtScaleDraw::label().
 * @param scaleDraw ScaleDraw object that has to be created with new and will be
 *                  deleted in ~QwtThermo() or the next call of setScaleDraw()
 * @sa scaleDraw(), QwtScaleDraw
 */
void QwtThermo::setScaleDraw(QwtScaleDraw* scaleDraw)
{
    setAbstractScaleDraw(scaleDraw);
    layoutThermo(true);
}

/**
 * @brief Return the scale draw of the thermometer (const version)
 * @sa setScaleDraw()
 */
const QwtScaleDraw* QwtThermo::scaleDraw() const
{
    return static_cast< const QwtScaleDraw* >(abstractScaleDraw());
}

/**
 * @brief Return the scale draw of the thermometer (non-const version)
 * @sa setScaleDraw()
 */
QwtScaleDraw* QwtThermo::scaleDraw()
{
    return static_cast< QwtScaleDraw* >(abstractScaleDraw());
}

/**
 * @brief Paint event handler
 * @param event Paint event
 * @sa resizeEvent(), changeEvent()
 */
void QwtThermo::paintEvent(QPaintEvent* event)
{
    QWT_D(d);
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    const QRect tRect = pipeRect();

    if (!tRect.contains(event->rect())) {
        if (d->scalePosition != QwtThermo::NoScale)
            scaleDraw()->draw(&painter, palette());
    }

    const int bw = d->borderWidth;

    if (d->flatStyle) {
        const QRect frameRect = tRect.adjusted(-bw, -bw, bw, bw);
        if (d->autoFillPipe)
            painter.fillRect(frameRect, palette().brush(QPalette::Base));
        painter.save();
        painter.setPen(QPen(palette().color(QPalette::Mid), bw));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(frameRect.adjusted(bw / 2, bw / 2, -bw / 2 - 1, -bw / 2 - 1));
        painter.restore();
    } else {
        const QBrush brush = palette().brush(QPalette::Base);
        qDrawShadePanel(&painter, tRect.adjusted(-bw, -bw, bw, bw), palette(), true, bw, d->autoFillPipe ? &brush : nullptr);
    }

    drawLiquid(&painter, tRect);
}

/**
 * @brief Resize event handler
 * @param event Resize event
 * @sa paintEvent(), layoutThermo()
 */
void QwtThermo::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    layoutThermo(false);
}

/**
 * @brief Qt change event handler
 * @param event Event
 * @sa paintEvent(), resizeEvent()
 */
void QwtThermo::changeEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::StyleChange:
    case QEvent::FontChange: {
        layoutThermo(true);
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Recalculate the QwtThermo geometry and layout
 * @details Recalculates geometry and layout based on pipeRect() and the fonts.
 * @param update_geometry Notify the layout system and call update to redraw the scale
 * @sa pipeRect(), layoutThermo()
 */
void QwtThermo::layoutThermo(bool update_geometry)
{
    QWT_D(d);
    const QRect tRect   = pipeRect();
    const int bw        = d->borderWidth + d->spacing;
    const bool inverted = (upperBound() < lowerBound());

    int from, to;

    if (d->orientation == Qt::Horizontal) {
        from = tRect.left();
        to   = tRect.right();

        if (d->rangeFlags & QwtInterval::ExcludeMinimum) {
            if (inverted)
                to++;
            else
                from--;
        }
        if (d->rangeFlags & QwtInterval::ExcludeMaximum) {
            if (inverted)
                from--;
            else
                to++;
        }

        if (d->scalePosition == QwtThermo::TrailingScale) {
            scaleDraw()->setAlignment(QwtScaleDraw::TopScale);
            scaleDraw()->move(from, tRect.top() - bw);
        } else {
            scaleDraw()->setAlignment(QwtScaleDraw::BottomScale);
            scaleDraw()->move(from, tRect.bottom() + bw);
        }

        scaleDraw()->setLength(qMax(to - from, 0));
    } else  // Qt::Vertical
    {
        from = tRect.top();
        to   = tRect.bottom();

        if (d->rangeFlags & QwtInterval::ExcludeMinimum) {
            if (inverted)
                from--;
            else
                to++;
        }
        if (d->rangeFlags & QwtInterval::ExcludeMaximum) {
            if (inverted)
                to++;
            else
                from--;
        }

        if (d->scalePosition == QwtThermo::LeadingScale) {
            scaleDraw()->setAlignment(QwtScaleDraw::RightScale);
            scaleDraw()->move(tRect.right() + bw, from);
        } else {
            scaleDraw()->setAlignment(QwtScaleDraw::LeftScale);
            scaleDraw()->move(tRect.left() - bw, from);
        }

        scaleDraw()->setLength(qMax(to - from, 0));
    }

    if (update_geometry) {
        updateGeometry();
        update();
    }
}

/**
 * @brief Return bounding rectangle of the pipe
 * @returns Bounding rectangle of the pipe (without borders) in widget coordinates
 * @sa fillRect(), alarmRect()
 */
QRect QwtThermo::pipeRect() const
{
    QWT_DC(d);
    int mbd = 0;
    if (d->scalePosition != QwtThermo::NoScale) {
        int d1, d2;
        scaleDraw()->getBorderDistHint(font(), d1, d2);
        mbd = qMax(d1, d2);
    }
    const int bw       = d->borderWidth;
    const int scaleOff = bw + mbd;

    const QRect cr = contentsRect();

    QRect pipeRect = cr;
    if (d->orientation == Qt::Horizontal) {
        pipeRect.adjust(scaleOff, 0, -scaleOff, 0);

        if (d->scalePosition == QwtThermo::TrailingScale)
            pipeRect.setTop(cr.top() + cr.height() - bw - d->pipeWidth);
        else
            pipeRect.setTop(bw);

        pipeRect.setHeight(d->pipeWidth);
    } else  // Qt::Vertical
    {
        pipeRect.adjust(0, scaleOff, 0, -scaleOff);

        if (d->scalePosition == QwtThermo::LeadingScale)
            pipeRect.setLeft(bw);
        else
            pipeRect.setLeft(cr.left() + cr.width() - bw - d->pipeWidth);

        pipeRect.setWidth(d->pipeWidth);
    }

    return pipeRect;
}

/**
 * @brief Set the orientation
 * @param orientation Allowed values are Qt::Horizontal and Qt::Vertical
 * @sa orientation(), scalePosition()
 */
void QwtThermo::setOrientation(Qt::Orientation orientation)
{
    QWT_D(d);
    if (orientation == d->orientation)
        return;

    d->orientation = orientation;

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);

        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    layoutThermo(true);
}

/**
 * @brief Return orientation
 * @sa setOrientation()
 */
Qt::Orientation QwtThermo::orientation() const
{
    QWT_DC(d);
    return d->orientation;
}

/**
 * @brief Change how the origin is determined
 * @sa originMode(), setOrigin(), origin()
 */
void QwtThermo::setOriginMode(OriginMode m)
{
    QWT_D(d);
    if (m == d->originMode)
        return;

    d->originMode = m;
    update();
}

/**
 * @brief Return mode how the origin is determined
 * @sa setOriginMode(), setOrigin(), origin()
 */
QwtThermo::OriginMode QwtThermo::originMode() const
{
    QWT_DC(d);
    return d->originMode;
}

/**
 * @brief Specify the custom origin
 * @details If originMode is set to OriginCustom this property controls where the liquid starts.
 * @param origin New origin level
 * @sa setOriginMode(), originMode(), origin()
 */
void QwtThermo::setOrigin(double origin)
{
    QWT_D(d);
    if (origin == d->origin)
        return;

    d->origin = origin;
    update();
}

/**
 * @brief Return origin of the thermometer when OriginCustom is enabled
 * @sa setOrigin(), setOriginMode(), originMode()
 */
double QwtThermo::origin() const
{
    QWT_DC(d);
    return d->origin;
}

/**
 * @brief Change the position of the scale
 * @param scalePosition Position of the scale
 * @sa ScalePosition, scalePosition()
 */
void QwtThermo::setScalePosition(ScalePosition scalePosition)
{
    QWT_D(d);
    if (d->scalePosition == scalePosition)
        return;

    d->scalePosition = scalePosition;

    if (testAttribute(Qt::WA_WState_Polished))
        layoutThermo(true);
}

/**
 * @brief Return scale position
 * @sa setScalePosition()
 */
QwtThermo::ScalePosition QwtThermo::scalePosition() const
{
    QWT_DC(d);
    return d->scalePosition;
}

/**
 * @brief Notify a scale change
 * @sa scaleChange()
 */
void QwtThermo::scaleChange()
{
    layoutThermo(true);
}

/**
 * @brief Redraw the liquid in thermometer pipe
 * @param painter Painter
 * @param pipeRect Bounding rectangle of the pipe without borders
 * @sa pipeRect(), fillRect(), alarmRect()
 */
void QwtThermo::drawLiquid(QPainter* painter, const QRect& pipeRect) const
{
    QWT_DC(d);
    painter->save();
    painter->setClipRect(pipeRect, Qt::IntersectClip);
    painter->setPen(Qt::NoPen);

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    QRect liquidRect = fillRect(pipeRect);

    if (d->colorMap != nullptr) {
        const QwtInterval interval = scaleDiv().interval().normalized();

        // Because the positions of the ticks are rounded
        // we calculate the colors for the rounded tick values

        QVector< double > values = qwtTickList(scaleDraw()->scaleDiv());

        if (scaleMap.isInverting())
            std::sort(values.begin(), values.end(), std::greater< double >());
        else
            std::sort(values.begin(), values.end(), std::less< double >());

        int from;
        if (!values.isEmpty()) {
            from = qRound(scaleMap.transform(values[ 0 ]));
            qwtDrawLine(painter, from, d->colorMap->color(interval, values[ 0 ]), pipeRect, liquidRect, d->orientation);
        }

        for (int i = 1; i < values.size(); i++) {
            const int to = qRound(scaleMap.transform(values[ i ]));

            for (int pos = from + 1; pos < to; pos++) {
                const double v = scaleMap.invTransform(pos);

                qwtDrawLine(painter, pos, d->colorMap->color(interval, v), pipeRect, liquidRect, d->orientation);
            }

            qwtDrawLine(painter, to, d->colorMap->color(interval, values[ i ]), pipeRect, liquidRect, d->orientation);

            from = to;
        }
    } else {
        if (!liquidRect.isEmpty() && d->alarmEnabled) {
            const QRect r = alarmRect(liquidRect);
            if (!r.isEmpty()) {
                painter->fillRect(r, palette().brush(QPalette::Highlight));
                liquidRect = QRegion(liquidRect).subtracted(r).boundingRect();
            }
        }

        painter->fillRect(liquidRect, palette().brush(QPalette::ButtonText));
    }

    painter->restore();
}

/**
 * @brief Change the spacing between pipe and scale
 * @details A spacing of 0 means that the backbone of the scale is below the pipe.
 *          The default setting is 3 pixels.
 * @param spacing Number of pixels
 * @sa spacing()
 */
void QwtThermo::setSpacing(int spacing)
{
    QWT_D(d);
    if (spacing <= 0)
        spacing = 0;

    if (spacing != d->spacing) {
        d->spacing = spacing;
        layoutThermo(true);
    }
}

/**
 * @brief Return number of pixels between pipe and scale
 * @sa setSpacing()
 */
int QwtThermo::spacing() const
{
    QWT_DC(d);
    return d->spacing;
}

/**
 * @brief Set the border width of the pipe
 * @param width Border width
 * @sa borderWidth()
 */
void QwtThermo::setBorderWidth(int width)
{
    QWT_D(d);
    if (width <= 0)
        width = 0;

    if (width != d->borderWidth) {
        d->borderWidth = width;
        layoutThermo(true);
    }
}

/**
 * @brief Return border width of the thermometer pipe
 * @sa setBorderWidth()
 */
int QwtThermo::borderWidth() const
{
    QWT_DC(d);
    return d->borderWidth;
}

/**
 * @brief Set flat style
 * @details When enabled (default), the pipe border is drawn with a flat solid-color
 *          line instead of a 3D embossed effect (qDrawShadePanel).
 * @param on true for flat style, false for classic 3D style
 * @sa flatStyle()
 */
void QwtThermo::setFlatStyle(bool on)
{
    QWT_D(d);
    if (d->flatStyle != on) {
        d->flatStyle = on;
        update();
    }
}

/**
 * @brief Return whether flat style is enabled
 * @sa setFlatStyle()
 */
bool QwtThermo::flatStyle() const
{
    QWT_DC(d);
    return d->flatStyle;
}

/**
 * @brief Assign a color map for the fill color
 * @param colorMap Color map
 * @warning The alarm threshold has no effect when a color map has been assigned
 * @sa colorMap()
 */
void QwtThermo::setColorMap(QwtColorMap* colorMap)
{
    QWT_D(d);
    if (colorMap != d->colorMap) {
        delete d->colorMap;
        d->colorMap = colorMap;
    }
}

/**
 * @brief Return color map for the fill color
 * @warning The alarm threshold has no effect when a color map has been assigned
 * @sa setColorMap()
 */
QwtColorMap* QwtThermo::colorMap()
{
    QWT_D(d);
    return d->colorMap;
}

/**
 * @brief Return color map for the fill color (const version)
 * @warning The alarm threshold has no effect when a color map has been assigned
 * @sa setColorMap()
 */
const QwtColorMap* QwtThermo::colorMap() const
{
    QWT_DC(d);
    return d->colorMap;
}

/**
 * @brief Change the brush of the liquid
 * @details Changes the QPalette::ButtonText brush of the palette.
 * @param brush New brush
 * @sa fillBrush(), QWidget::setPalette()
 */
void QwtThermo::setFillBrush(const QBrush& brush)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::ButtonText, brush);
    setPalette(pal);
}

/**
 * @brief Return liquid (QPalette::ButtonText) brush
 * @sa setFillBrush(), QWidget::palette()
 */
QBrush QwtThermo::fillBrush() const
{
    return palette().brush(QPalette::ButtonText);
}

/**
 * @brief Specify the liquid brush above the alarm threshold
 * @details Changes the QPalette::Highlight brush of the palette.
 * @param brush New brush
 * @sa alarmBrush(), QWidget::setPalette()
 * @warning The alarm threshold has no effect when a color map has been assigned
 */
void QwtThermo::setAlarmBrush(const QBrush& brush)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Highlight, brush);
    setPalette(pal);
}

/**
 * @brief Return liquid brush (QPalette::Highlight) above the alarm threshold
 * @sa setAlarmBrush(), QWidget::palette()
 * @warning The alarm threshold has no effect when a color map has been assigned
 */
QBrush QwtThermo::alarmBrush() const
{
    return palette().brush(QPalette::Highlight);
}

/**
 * @brief Specify the alarm threshold
 * @param level Alarm threshold
 * @sa alarmLevel()
 * @warning The alarm threshold has no effect when a color map has been assigned
 */
void QwtThermo::setAlarmLevel(double level)
{
    QWT_D(d);
    d->alarmLevel   = level;
    d->alarmEnabled = 1;
    update();
}

/**
 * @brief Return alarm threshold
 * @sa setAlarmLevel()
 * @warning The alarm threshold has no effect when a color map has been assigned
 */
double QwtThermo::alarmLevel() const
{
    QWT_DC(d);
    return d->alarmLevel;
}

/**
 * @brief Change the width of the pipe
 * @param width Width of the pipe
 * @sa pipeWidth()
 */
void QwtThermo::setPipeWidth(int width)
{
    QWT_D(d);
    if (width > 0) {
        d->pipeWidth = width;
        layoutThermo(true);
    }
}

/**
 * @brief Return width of the pipe
 * @sa setPipeWidth()
 */
int QwtThermo::pipeWidth() const
{
    QWT_DC(d);
    return d->pipeWidth;
}

/**
 * @brief Enable or disable the alarm threshold
 * @param on True to enable, false to disable
 * @sa alarmEnabled()
 * @warning The alarm threshold has no effect when a color map has been assigned
 */
void QwtThermo::setAlarmEnabled(bool on)
{
    QWT_D(d);
    d->alarmEnabled = on;
    update();
}

/**
 * @brief Return true when the alarm threshold is enabled
 * @sa setAlarmEnabled()
 * @warning The alarm threshold has no effect when a color map has been assigned
 */
bool QwtThermo::alarmEnabled() const
{
    QWT_DC(d);
    return d->alarmEnabled;
}

/**
 * @brief Return the minimum size hint
 * @sa minimumSizeHint()
 */
QSize QwtThermo::sizeHint() const
{
    return minimumSizeHint();
}

/**
 * @brief Return minimum size hint
 * @details The return value depends on the font and the scale.
 * @sa sizeHint()
 */
QSize QwtThermo::minimumSizeHint() const
{
    QWT_DC(d);
    int w = 0, h = 0;

    if (d->scalePosition != NoScale) {
        const int sdExtent = qwtCeil(scaleDraw()->extent(font()));
        const int sdLength = scaleDraw()->minLength(font());

        w = sdLength;
        h = d->pipeWidth + sdExtent + d->spacing;

    } else  // no scale
    {
        w = 200;
        h = d->pipeWidth;
    }

    if (d->orientation == Qt::Vertical)
        qSwap(w, h);

    w += 2 * d->borderWidth;
    h += 2 * d->borderWidth;

    // finally add the margins
    const QMargins m = contentsMargins();
    w += m.left() + m.right();
    h += m.top() + m.bottom();

    return QSize(w, h);
}

/**
 * @brief Calculate the filled rectangle of the pipe
 * @param pipeRect Rectangle of the pipe
 * @returns Rectangle to be filled (fill and alarm brush)
 * @sa pipeRect(), alarmRect()
 */
QRect QwtThermo::fillRect(const QRect& pipeRect) const
{
    QWT_DC(d);
    double origin;
    if (d->originMode == OriginMinimum) {
        origin = qMin(lowerBound(), upperBound());
    } else if (d->originMode == OriginMaximum) {
        origin = qMax(lowerBound(), upperBound());
    } else  // OriginCustom
    {
        origin = d->origin;
    }

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    int from = qRound(scaleMap.transform(d->value));
    int to   = qRound(scaleMap.transform(origin));

    if (to < from)
        qSwap(from, to);

    QRect fillRect = pipeRect;
    if (d->orientation == Qt::Horizontal) {
        fillRect.setLeft(from);
        fillRect.setRight(to);
    } else  // Qt::Vertical
    {
        fillRect.setTop(from);
        fillRect.setBottom(to);
    }

    return fillRect.normalized();
}

/**
 * @brief Calculate the alarm rectangle of the pipe
 * @param fillRect Filled rectangle in the pipe
 * @returns Rectangle to be filled with the alarm brush
 * @sa pipeRect(), fillRect(), alarmLevel(), alarmBrush()
 */
QRect QwtThermo::alarmRect(const QRect& fillRect) const
{
    QWT_DC(d);
    QRect alarmRect(0, 0, -1, -1);  // something invalid

    if (!d->alarmEnabled)
        return alarmRect;

    const bool inverted = (upperBound() < lowerBound());

    bool increasing;
    if (d->originMode == OriginCustom) {
        increasing = d->value > d->origin;
    } else {
        increasing = d->originMode == OriginMinimum;
    }

    const QwtScaleMap map = scaleDraw()->scaleMap();
    const int alarmPos    = qRound(map.transform(d->alarmLevel));
    const int valuePos    = qRound(map.transform(d->value));

    if (d->orientation == Qt::Horizontal) {
        int v1, v2;
        if (inverted) {
            v1 = fillRect.left();

            v2 = alarmPos - 1;
            v2 = qMin(v2, increasing ? fillRect.right() : valuePos);
        } else {
            v1 = alarmPos + 1;
            v1 = qMax(v1, increasing ? fillRect.left() : valuePos);

            v2 = fillRect.right();
        }
        alarmRect.setRect(v1, fillRect.top(), v2 - v1 + 1, fillRect.height());
    } else {
        int v1, v2;
        if (inverted) {
            v1 = alarmPos + 1;
            v1 = qMax(v1, increasing ? fillRect.top() : valuePos);

            v2 = fillRect.bottom();
        } else {
            v1 = fillRect.top();

            v2 = alarmPos - 1;
            v2 = qMin(v2, increasing ? fillRect.bottom() : valuePos);
        }
        alarmRect.setRect(fillRect.left(), v1, fillRect.width(), v2 - v1 + 1);
    }

    return alarmRect;
}
