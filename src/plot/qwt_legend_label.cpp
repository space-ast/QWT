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
 *        - QwtGridRasterData (2-d table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 *****************************************************************************/

#include "qwt_legend_label.h"
#include "qwt_legend_data.h"
#include "qwt_graphic.h"
#include "qwt_utils.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qevent.h>
#include <qstyleoption.h>

static const int cs_legendlabel_buttonFrame = 2;
static const int cs_legendlabel_margin      = 2;

static QSize buttonShift(const QwtLegendLabel* w)
{
    QStyleOption option;
    option.initFrom(w);

    const int ph = w->style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, w);
    const int pv = w->style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, w);
    return QSize(ph, pv);
}

class QwtLegendLabel::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtLegendLabel)

public:
    PrivateData(QwtLegendLabel* p) : q_ptr(p), itemMode(QwtLegendData::ReadOnly), isDown(false), spacing(cs_legendlabel_margin)
    {
    }

    QwtLegendData::Mode itemMode;
    QwtLegendData legendData;
    bool isDown;

    QPixmap icon;

    int spacing;
};

/**
 * @brief Set the attributes of the legend label
 * @param legendData Attributes of the label
 * @sa data()
 */
void QwtLegendLabel::setData(const QwtLegendData& legendData)
{
    QWT_D(d);
    d->legendData = legendData;

    const bool doUpdate = updatesEnabled();
    if (doUpdate)
        setUpdatesEnabled(false);

    setText(legendData.title());
    setIcon(legendData.icon().toPixmap());

    if (legendData.hasRole(QwtLegendData::ModeRole))
        setItemMode(legendData.mode());

    if (doUpdate)
        setUpdatesEnabled(true);
}

/**
 * @brief Return the attributes of the label
 * @return Attributes of the label
 * @sa setData(), QwtPlotItem::legendData()
 */
const QwtLegendData& QwtLegendLabel::data() const
{
    QWT_DC(d);
    return d->legendData;
}

/**
 * @brief Constructor for QwtLegendLabel
 * @param parent Parent widget
 */
QwtLegendLabel::QwtLegendLabel(QWidget* parent) : QwtTextLabel(parent), QWT_PIMPL_CONSTRUCT
{
    setMargin(cs_legendlabel_margin);
    setIndent(cs_legendlabel_margin);
}

/**
 * @brief Destructor
 */
QwtLegendLabel::~QwtLegendLabel()
{
}

/**
 * @brief Set the text to the legend item
 * @param text Text label
 * @sa QwtTextLabel::text()
 */
void QwtLegendLabel::setText(const QwtText& text)
{
    const int flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs | Qt::TextWordWrap;

    QwtText txt = text;
    txt.setRenderFlags(flags);

    QwtTextLabel::setText(txt);
}

/**
 * @brief Set the item mode
 * @details The default is QwtLegendData::ReadOnly
 * @param mode Item mode
 * @sa itemMode()
 */
void QwtLegendLabel::setItemMode(QwtLegendData::Mode mode)
{
    QWT_D(d);
    if (mode != d->itemMode) {
        d->itemMode = mode;
        d->isDown   = false;

        setFocusPolicy((mode != QwtLegendData::ReadOnly) ? Qt::TabFocus : Qt::NoFocus);
        setMargin(cs_legendlabel_buttonFrame + cs_legendlabel_margin);

        updateGeometry();
    }
}

/**
 * @brief Return the item mode
 * @return Item mode
 * @sa setItemMode()
 */
QwtLegendData::Mode QwtLegendLabel::itemMode() const
{
    QWT_DC(d);
    return d->itemMode;
}

/**
 * @brief Assign the icon
 * @param icon Pixmap representing a plot item
 * @sa icon(), QwtPlotItem::legendIcon()
 */
void QwtLegendLabel::setIcon(const QPixmap& icon)
{
    QWT_D(d);
    d->icon = icon;

    int indent = margin() + d->spacing;
    if (icon.width() > 0)
        indent += icon.width() + d->spacing;

    setIndent(indent);
}

/**
 * @brief Return the pixmap representing a plot item
 * @return Pixmap representing a plot item
 * @sa setIcon()
 */
QPixmap QwtLegendLabel::icon() const
{
    QWT_DC(d);
    return d->icon;
}

/**
 * @brief Change the spacing between icon and text
 * @param spacing Spacing
 * @sa spacing(), QwtTextLabel::margin()
 */
void QwtLegendLabel::setSpacing(int spacing)
{
    QWT_D(d);
    spacing = qMax(spacing, 0);
    if (spacing != d->spacing) {
        d->spacing = spacing;

        int indent = margin() + d->spacing;
        if (d->icon.width() > 0)
            indent += d->icon.width() + d->spacing;

        setIndent(indent);
    }
}

/**
 * @brief Return the spacing between icon and text
 * @return Spacing between icon and text
 * @sa setSpacing(), QwtTextLabel::margin()
 */
int QwtLegendLabel::spacing() const
{
    QWT_DC(d);
    return d->spacing;
}

/**
 * @brief Check/Uncheck the item
 * @param on check/uncheck
 * @sa setItemMode()
 */
void QwtLegendLabel::setChecked(bool on)
{
    QWT_D(d);
    if (d->itemMode == QwtLegendData::Checkable) {
        const bool isBlocked = signalsBlocked();
        blockSignals(true);

        setDown(on);

        blockSignals(isBlocked);
    }
}

/**
 * @brief Return true if the item is checked
 * @return True if the item is checked
 */
bool QwtLegendLabel::isChecked() const
{
    QWT_DC(d);
    return d->itemMode == QwtLegendData::Checkable && isDown();
}

//! Set the item being down
void QwtLegendLabel::setDown(bool down)
{
    QWT_D(d);
    if (down == d->isDown)
        return;

    d->isDown = down;
    update();

    if (d->itemMode == QwtLegendData::Clickable) {
        if (d->isDown)
            Q_EMIT pressed();
        else {
            Q_EMIT released();
            Q_EMIT clicked();
        }
    }

    if (d->itemMode == QwtLegendData::Checkable)
        Q_EMIT checked(d->isDown);
}

//! Return true, if the item is down
bool QwtLegendLabel::isDown() const
{
    QWT_DC(d);
    return d->isDown;
}

/**
 * @brief Return a size hint
 * @return Size hint
 */
QSize QwtLegendLabel::sizeHint() const
{
    QWT_DC(d);
    QSize sz = QwtTextLabel::sizeHint();
    sz.setHeight(qMax(sz.height(), d->icon.height() + 4));

    if (d->itemMode != QwtLegendData::ReadOnly) {
        sz += buttonShift(this);
        sz = qwtExpandedToGlobalStrut(sz);
    }

    return sz;
}

//! Paint event
void QwtLegendLabel::paintEvent(QPaintEvent* e)
{
    QWT_D(d);
    const QRect cr = contentsRect();

    QPainter painter(this);
    painter.setClipRegion(e->region());

    if (d->isDown) {
        qDrawWinButton(&painter, 0, 0, width(), height(), palette(), true);
    }

    painter.save();

    if (d->isDown) {
        const QSize shiftSize = buttonShift(this);
        painter.translate(shiftSize.width(), shiftSize.height());
    }

    painter.setClipRect(cr);

    drawContents(&painter);

    if (!d->icon.isNull()) {
        QRect iconRect = cr;
        iconRect.setX(iconRect.x() + margin());
        if (d->itemMode != QwtLegendData::ReadOnly)
            iconRect.setX(iconRect.x() + cs_legendlabel_buttonFrame);

        iconRect.setSize(d->icon.size());
        iconRect.moveCenter(QPoint(iconRect.center().x(), cr.center().y()));

        painter.drawPixmap(iconRect, d->icon);
    }

    painter.restore();
}

//! Handle mouse press events
void QwtLegendLabel::mousePressEvent(QMouseEvent* e)
{
    QWT_D(d);
    if (e->button() == Qt::LeftButton) {
        switch (d->itemMode) {
        case QwtLegendData::Clickable: {
            setDown(true);
            return;
        }
        case QwtLegendData::Checkable: {
            setDown(!isDown());
            return;
        }
        default:;
        }
    }
    QwtTextLabel::mousePressEvent(e);
}

//! Handle mouse release events
void QwtLegendLabel::mouseReleaseEvent(QMouseEvent* e)
{
    QWT_D(d);
    if (e->button() == Qt::LeftButton) {
        switch (d->itemMode) {
        case QwtLegendData::Clickable: {
            setDown(false);
            return;
        }
        case QwtLegendData::Checkable: {
            return;  // do nothing, but accept
        }
        default:;
        }
    }
    QwtTextLabel::mouseReleaseEvent(e);
}

//! Handle key press events
void QwtLegendLabel::keyPressEvent(QKeyEvent* e)
{
    QWT_D(d);
    if (e->key() == Qt::Key_Space) {
        switch (d->itemMode) {
        case QwtLegendData::Clickable: {
            if (!e->isAutoRepeat())
                setDown(true);
            return;
        }
        case QwtLegendData::Checkable: {
            if (!e->isAutoRepeat())
                setDown(!isDown());
            return;
        }
        default:;
        }
    }

    QwtTextLabel::keyPressEvent(e);
}

//! Handle key release events
void QwtLegendLabel::keyReleaseEvent(QKeyEvent* e)
{
    QWT_D(d);
    if (e->key() == Qt::Key_Space) {
        switch (d->itemMode) {
        case QwtLegendData::Clickable: {
            if (!e->isAutoRepeat())
                setDown(false);
            return;
        }
        case QwtLegendData::Checkable: {
            return;  // do nothing, but accept
        }
        default:;
        }
    }

    QwtTextLabel::keyReleaseEvent(e);
}
