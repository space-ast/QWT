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

#include "qwt_arrow_button.h"
#include "qwt_utils.h"

#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>

static const int cs_arrowButton_maxNum  = 3;
static const int cs_arrowButton_margin  = 2;
static const int cs_arrowButton_spacing = 1;

class QwtArrowButton::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtArrowButton)
public:
    PrivateData( QwtArrowButton* p )
        : q_ptr( p )
    {
    }

    int num;
    Qt::ArrowType arrowType;
};

static QStyleOptionButton styleOpt(const QwtArrowButton* btn)
{
    QStyleOptionButton option;
    option.initFrom(btn);
    option.features = QStyleOptionButton::None;
    if (btn->isFlat())
        option.features |= QStyleOptionButton::Flat;
    if (btn->menu())
        option.features |= QStyleOptionButton::HasMenu;
    if (btn->autoDefault() || btn->isDefault())
        option.features |= QStyleOptionButton::AutoDefaultButton;
    if (btn->isDefault())
        option.features |= QStyleOptionButton::DefaultButton;
    if (btn->isDown())
        option.state |= QStyle::State_Sunken;
    if (!btn->isFlat() && !btn->isDown())
        option.state |= QStyle::State_Raised;

    return option;
}

/**
 *   @brief Constructor
 *   @param[in] num Number of arrows (1-3)
 *   @param[in] arrowType Direction of arrows (see Qt::ArrowType)
 *   @param[in] parent Parent widget
 */
QwtArrowButton::QwtArrowButton(int num, Qt::ArrowType arrowType, QWidget* parent)
    : QPushButton(parent)
    , QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    d->num       = qBound(1, num, cs_arrowButton_maxNum);
    d->arrowType = arrowType;

    setAutoRepeat(true);
    setAutoDefault(false);

    switch (d->arrowType) {
    case Qt::LeftArrow:
    case Qt::RightArrow:
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        break;
    default:
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
}

/**
 *   @brief Destructor
 */
QwtArrowButton::~QwtArrowButton()
{
}

/**
 *   @brief Get the direction of the arrows
 *   @return Arrow direction
 */
Qt::ArrowType QwtArrowButton::arrowType() const
{
    QWT_DC(d);
    return d->arrowType;
}

/**
 *   @brief Get the number of arrows
 *   @return Number of arrows
 */
int QwtArrowButton::num() const
{
    QWT_DC(d);
    return d->num;
}

/*!
   @return the bounding rectangle for the label
 */
QRect QwtArrowButton::labelRect() const
{
    const int m = cs_arrowButton_margin;

    QRect r = rect();
    r.setRect(r.x() + m, r.y() + m, r.width() - 2 * m, r.height() - 2 * m);

    if (isDown()) {
        QStyleOptionButton option = styleOpt(this);
        const int ph              = style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this);
        const int pv              = style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this);

        r.translate(ph, pv);
    }

    return r;
}

/*!
   Paint event handler
   @param event Paint event
 */
void QwtArrowButton::paintEvent(QPaintEvent* event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);
    drawButtonLabel(&painter);
}

/*!
   @brief Draw the button label

   @param painter Painter
   @sa The Qt Manual for QPushButton
 */
void QwtArrowButton::drawButtonLabel(QPainter* painter)
{
    QWT_D(d);
    const bool isVertical = d->arrowType == Qt::UpArrow || d->arrowType == Qt::DownArrow;

    const QRect r      = labelRect();
    QSize boundingSize = labelRect().size();
    if (isVertical)
        boundingSize.transpose();

    const int w = (boundingSize.width() - (cs_arrowButton_maxNum - 1) * cs_arrowButton_spacing) / cs_arrowButton_maxNum;

    QSize arrow = arrowSize(Qt::RightArrow, QSize(w, boundingSize.height()));

    if (isVertical)
        arrow.transpose();

    QRect contentsSize;  // aligned rect where to paint all arrows
    if (d->arrowType == Qt::LeftArrow || d->arrowType == Qt::RightArrow) {
        contentsSize.setWidth(d->num * arrow.width() + (d->num - 1) * cs_arrowButton_spacing);
        contentsSize.setHeight(arrow.height());
    } else {
        contentsSize.setWidth(arrow.width());
        contentsSize.setHeight(d->num * arrow.height() + (d->num - 1) * cs_arrowButton_spacing);
    }

    QRect arrowRect(contentsSize);
    arrowRect.moveCenter(r.center());
    arrowRect.setSize(arrow);

    painter->save();
    for (int i = 0; i < d->num; i++) {
        drawArrow(painter, arrowRect, d->arrowType);

        int dx = 0;
        int dy = 0;

        if (isVertical)
            dy = arrow.height() + cs_arrowButton_spacing;
        else
            dx = arrow.width() + cs_arrowButton_spacing;

        arrowRect.translate(dx, dy);
    }
    painter->restore();

    if (hasFocus()) {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Window);

        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, this);
    }
}

/*!
    Draw an arrow int a bounding rectangle

    @param painter Painter
    @param r Rectangle where to paint the arrow
    @param arrowType Arrow type
 */
void QwtArrowButton::drawArrow(QPainter* painter, const QRect& r, Qt::ArrowType arrowType) const
{
    QPolygon pa(3);

    switch (arrowType) {
    case Qt::UpArrow:
        pa.setPoint(0, r.bottomLeft());
        pa.setPoint(1, r.bottomRight());
        pa.setPoint(2, r.center().x(), r.top());
        break;
    case Qt::DownArrow:
        pa.setPoint(0, r.topLeft());
        pa.setPoint(1, r.topRight());
        pa.setPoint(2, r.center().x(), r.bottom());
        break;
    case Qt::RightArrow:
        pa.setPoint(0, r.topLeft());
        pa.setPoint(1, r.bottomLeft());
        pa.setPoint(2, r.right(), r.center().y());
        break;
    case Qt::LeftArrow:
        pa.setPoint(0, r.topRight());
        pa.setPoint(1, r.bottomRight());
        pa.setPoint(2, r.left(), r.center().y());
        break;
    default:
        break;
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette().brush(QPalette::ButtonText));
    painter->drawPolygon(pa);

    painter->restore();
}

/**
 *   @brief Get a size hint for the button
 *   @return Size hint
 */
QSize QwtArrowButton::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return qwtExpandedToGlobalStrut(hint);
}

/**
 *   @brief Get a minimum size hint for the button
 *   @return Minimum size hint
 */
QSize QwtArrowButton::minimumSizeHint() const
{
    QWT_DC(d);
    const QSize asz = arrowSize(Qt::RightArrow, QSize());

    QSize sz(2 * cs_arrowButton_margin + (cs_arrowButton_maxNum - 1) * cs_arrowButton_spacing
                 + cs_arrowButton_maxNum * asz.width(),
             2 * cs_arrowButton_margin + asz.height());

    if (d->arrowType == Qt::UpArrow || d->arrowType == Qt::DownArrow)
        sz.transpose();

    QStyleOption styleOption;
    styleOption.initFrom(this);

    sz = style()->sizeFromContents(QStyle::CT_PushButton, &styleOption, sz, this);

    return sz;
}

/*!
   Calculate the size for a arrow that fits into a rectangle of a given size

   @param arrowType Arrow type
   @param boundingSize Bounding size
   @return Size of the arrow
 */
QSize QwtArrowButton::arrowSize(Qt::ArrowType arrowType, const QSize& boundingSize) const
{
    QSize bs = boundingSize;
    if (arrowType == Qt::UpArrow || arrowType == Qt::DownArrow)
        bs.transpose();

    const int MinLen = 2;
    const QSize sz   = bs.expandedTo(QSize(MinLen, 2 * MinLen - 1));  // minimum

    int w = sz.width();
    int h = 2 * w - 1;

    if (h > sz.height()) {
        h = sz.height();
        w = (h + 1) / 2;
    }

    QSize arrSize(w, h);
    if (arrowType == Qt::UpArrow || arrowType == Qt::DownArrow)
        arrSize.transpose();

    return arrSize;
}

/*!
   @brief autoRepeat for the space keys
 */
void QwtArrowButton::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat() && event->key() == Qt::Key_Space)
        Q_EMIT clicked();

    QPushButton::keyPressEvent(event);
}
