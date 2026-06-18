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

#include "qwt_plot_marker.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>

class QwtPlotMarker::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotMarker)
public:
    PrivateData(QwtPlotMarker* p)
        : q_ptr(p)
        , labelAlignment(Qt::AlignCenter)
        , labelOrientation(Qt::Horizontal)
        , spacing(2)
        , pen(QColor("#555555"), 0)
        , symbol(nullptr)
        , style(QwtPlotMarker::NoLine)
        , xValue(0.0)
        , yValue(0.0)
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtText label;
    Qt::Alignment labelAlignment;
    Qt::Orientation labelOrientation;
    int spacing;

    QPen pen;
    const QwtSymbol* symbol;
    LineStyle style;

    double xValue;
    double yValue;
};

/**
 * @brief Constructor
 * @details Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
 *
 */
QwtPlotMarker::QwtPlotMarker() : QWT_PIMPL_CONSTRUCT
{
    setZ(30.0);
}

/**
 * @brief Constructor with QString title
 * @details Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
 * @param[in] title Title of the marker
 *
 */
QwtPlotMarker::QwtPlotMarker(const QString& title) : QwtPlotItem(QwtText(title)), QWT_PIMPL_CONSTRUCT
{
    setZ(30.0);
}

/**
 * @brief Constructor with QwtText title
 * @details Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
 * @param[in] title Title of the marker
 *
 */
QwtPlotMarker::QwtPlotMarker(const QwtText& title) : QwtPlotItem(title), QWT_PIMPL_CONSTRUCT
{
    setZ(30.0);
}

/**
 * @brief Destructor
 *
 */
QwtPlotMarker::~QwtPlotMarker()
{
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotMarker
 *
 */
int QwtPlotMarker::rtti() const
{
    return QwtPlotItem::Rtti_PlotMarker;
}

/**
 * @brief Get the value as a point
 * @return Value as QPointF
 *
 */
QPointF QwtPlotMarker::value() const
{
    QWT_DC(d);
    return QPointF(d->xValue, d->yValue);
}

/**
 * @brief Get the x-value
 * @return X value
 *
 */
double QwtPlotMarker::xValue() const
{
    QWT_DC(d);
    return d->xValue;
}

/**
 * @brief Get the y-value
 * @return Y value
 *
 */
double QwtPlotMarker::yValue() const
{
    QWT_DC(d);
    return d->yValue;
}

/**
 * @brief Set the value from a point
 * @param[in] pos Position as QPointF
 *
 */
void QwtPlotMarker::setValue(const QPointF& pos)
{
    setValue(pos.x(), pos.y());
}

/**
 * @brief Set the value
 * @param[in] x X value
 * @param[in] y Y value
 *
 */
void QwtPlotMarker::setValue(double x, double y)
{
    QWT_D(d);
    if (x != d->xValue || y != d->yValue) {
        d->xValue = x;
        d->yValue = y;
        itemChanged();
    }
}

/**
 * @brief Set the x-value
 * @param[in] x X value
 *
 */
void QwtPlotMarker::setXValue(double x)
{
    QWT_D(d);
    setValue(x, d->yValue);
}

/**
 * @brief Set the y-value
 * @param[in] y Y value
 *
 */
void QwtPlotMarker::setYValue(double y)
{
    QWT_D(d);
    setValue(d->xValue, y);
}

/**
 * @brief Draw the marker
 * @param[in] painter Painter
 * @param[in] xMap X Scale Map
 * @param[in] yMap Y Scale Map
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 *
 */
void QwtPlotMarker::draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const
{
    QWT_DC(d);
    const QPointF pos(xMap.transform(d->xValue), yMap.transform(d->yValue));

    drawLines(painter, canvasRect, pos);
    drawSymbol(painter, canvasRect, pos);
    drawLabel(painter, canvasRect, pos);
}

/*!
   Draw the lines marker

   @param painter Painter
   @param canvasRect Contents rectangle of the canvas in painter coordinates
   @param pos Position of the marker, translated into widget coordinates

   @sa drawLabel(), drawSymbol()
 */
void QwtPlotMarker::drawLines(QPainter* painter, const QRectF& canvasRect, const QPointF& pos) const
{
    QWT_DC(d);
    if (d->style == NoLine)
        return;

    const bool doAlign = QwtPainter::roundingAlignment(painter);

    painter->setPen(d->pen);
    if (d->style == QwtPlotMarker::HLine || d->style == QwtPlotMarker::Cross) {
        double y = pos.y();
        if (doAlign)
            y = qRound(y);

        QwtPainter::drawLine(painter, canvasRect.left(), y, canvasRect.right() - 1.0, y);
    }
    if (d->style == QwtPlotMarker::VLine || d->style == QwtPlotMarker::Cross) {
        double x = pos.x();
        if (doAlign)
            x = qRound(x);

        QwtPainter::drawLine(painter, x, canvasRect.top(), x, canvasRect.bottom() - 1.0);
    }
}

/*!
   Draw the symbol of the marker

   @param painter Painter
   @param canvasRect Contents rectangle of the canvas in painter coordinates
   @param pos Position of the marker, translated into widget coordinates

   @sa drawLabel(), QwtSymbol::drawSymbol()
 */
void QwtPlotMarker::drawSymbol(QPainter* painter, const QRectF& canvasRect, const QPointF& pos) const
{
    QWT_DC(d);
    if (d->symbol == nullptr)
        return;

    const QwtSymbol& symbol = *d->symbol;

    if (symbol.style() != QwtSymbol::NoSymbol) {
        const QSizeF sz = symbol.size();

        const QRectF clipRect = canvasRect.adjusted(-sz.width(), -sz.height(), sz.width(), sz.height());

        if (clipRect.contains(pos))
            symbol.drawSymbol(painter, pos);
    }
}

/*!
   Align and draw the text label of the marker

   @param painter Painter
   @param canvasRect Contents rectangle of the canvas in painter coordinates
   @param pos Position of the marker, translated into widget coordinates

   @sa drawLabel(), drawSymbol()
 */
void QwtPlotMarker::drawLabel(QPainter* painter, const QRectF& canvasRect, const QPointF& pos) const
{
    QWT_DC(d);
    if (d->label.isEmpty())
        return;

    Qt::Alignment align = d->labelAlignment;
    QPointF alignPos    = pos;

    QSizeF symbolOff(0, 0);

    switch (d->style) {
    case QwtPlotMarker::VLine: {
        // In VLine-style the y-position is pointless and
        // the alignment flags are relative to the canvas

        if (d->labelAlignment & Qt::AlignTop) {
            alignPos.setY(canvasRect.top());
            align &= ~Qt::AlignTop;
            align |= Qt::AlignBottom;
        } else if (d->labelAlignment & Qt::AlignBottom) {
            // In HLine-style the x-position is pointless and
            // the alignment flags are relative to the canvas

            alignPos.setY(canvasRect.bottom() - 1);
            align &= ~Qt::AlignBottom;
            align |= Qt::AlignTop;
        } else {
            alignPos.setY(canvasRect.center().y());
        }
        break;
    }
    case QwtPlotMarker::HLine: {
        if (d->labelAlignment & Qt::AlignLeft) {
            alignPos.setX(canvasRect.left());
            align &= ~Qt::AlignLeft;
            align |= Qt::AlignRight;
        } else if (d->labelAlignment & Qt::AlignRight) {
            alignPos.setX(canvasRect.right() - 1);
            align &= ~Qt::AlignRight;
            align |= Qt::AlignLeft;
        } else {
            alignPos.setX(canvasRect.center().x());
        }
        break;
    }
    default: {
        if (d->symbol && (d->symbol->style() != QwtSymbol::NoSymbol)) {
            symbolOff = d->symbol->size() + QSizeF(1, 1);
            symbolOff /= 2;
        }
    }
    }

    qreal pw2 = d->pen.widthF() / 2.0;
    if (pw2 == 0.0)
        pw2 = 0.5;

    const int spacing = d->spacing;

    const qreal xOff = qwtMaxF(pw2, symbolOff.width());
    const qreal yOff = qwtMaxF(pw2, symbolOff.height());

    const QSizeF textSize = d->label.textSize(painter->font());

    if (align & Qt::AlignLeft) {
        alignPos.rx() -= xOff + spacing;
        if (d->labelOrientation == Qt::Vertical)
            alignPos.rx() -= textSize.height();
        else
            alignPos.rx() -= textSize.width();
    } else if (align & Qt::AlignRight) {
        alignPos.rx() += xOff + spacing;
    } else {
        if (d->labelOrientation == Qt::Vertical)
            alignPos.rx() -= textSize.height() / 2;
        else
            alignPos.rx() -= textSize.width() / 2;
    }

    if (align & Qt::AlignTop) {
        alignPos.ry() -= yOff + spacing;
        if (d->labelOrientation != Qt::Vertical)
            alignPos.ry() -= textSize.height();
    } else if (align & Qt::AlignBottom) {
        alignPos.ry() += yOff + spacing;
        if (d->labelOrientation == Qt::Vertical)
            alignPos.ry() += textSize.width();
    } else {
        if (d->labelOrientation == Qt::Vertical)
            alignPos.ry() += textSize.width() / 2;
        else
            alignPos.ry() -= textSize.height() / 2;
    }

    painter->translate(alignPos.x(), alignPos.y());
    if (d->labelOrientation == Qt::Vertical)
        painter->rotate(-90.0);

    const QRectF textRect(0, 0, textSize.width(), textSize.height());
    d->label.draw(painter, textRect);
}

/**
 * @brief Set the line style
 * @param[in] style Line style
 * @sa lineStyle()
 *
 */
void QwtPlotMarker::setLineStyle(LineStyle style)
{
    QWT_D(d);
    if (style != d->style) {
        d->style = style;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the line style
 * @return Line style
 * @sa setLineStyle()
 *
 */
QwtPlotMarker::LineStyle QwtPlotMarker::lineStyle() const
{
    QWT_DC(d);
    return d->style;
}

/**
 * @brief Assign a symbol
 * @param[in] symbol New symbol
 * @sa symbol()
 *
 */
void QwtPlotMarker::setSymbol(const QwtSymbol* symbol)
{
    QWT_D(d);
    if (symbol != d->symbol) {
        delete d->symbol;
        d->symbol = symbol;

        if (symbol)
            setLegendIconSize(symbol->boundingRect().size());

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the symbol
 * @return The symbol
 * @sa setSymbol(), QwtSymbol
 *
 */
const QwtSymbol* QwtPlotMarker::symbol() const
{
    QWT_DC(d);
    return d->symbol;
}

/**
 * @brief Set the label
 * @param[in] label Label text
 * @sa label()
 *
 */
void QwtPlotMarker::setLabel(const QwtText& label)
{
    QWT_D(d);
    if (label != d->label) {
        d->label = label;
        itemChanged();
    }
}

/**
 * @brief Get the label
 * @return Label text
 * @sa setLabel()
 *
 */
QwtText QwtPlotMarker::label() const
{
    QWT_DC(d);
    return d->label;
}

/**
 * @brief Set the alignment of the label
 * @details In case of QwtPlotMarker::HLine the alignment is relative to the
 *          y position of the marker, but the horizontal flags correspond to the
 *          canvas rectangle. In case of QwtPlotMarker::VLine the alignment is
 *          relative to the x position of the marker, but the vertical flags
 *          correspond to the canvas rectangle.
 *          In all other styles the alignment is relative to the marker's position.
 * @param[in] align Alignment
 * @sa labelAlignment(), labelOrientation()
 *
 */
void QwtPlotMarker::setLabelAlignment(Qt::Alignment align)
{
    QWT_D(d);
    if (align != d->labelAlignment) {
        d->labelAlignment = align;
        itemChanged();
    }
}

/**
 * @brief Get the label alignment
 * @return Label alignment
 * @sa setLabelAlignment(), setLabelOrientation()
 *
 */
Qt::Alignment QwtPlotMarker::labelAlignment() const
{
    QWT_DC(d);
    return d->labelAlignment;
}

/**
 * @brief Set the orientation of the label
 * @details When orientation is Qt::Vertical the label is rotated by 90.0 degrees
 *          (from bottom to top).
 * @param[in] orientation Orientation of the label
 * @sa labelOrientation(), setLabelAlignment()
 *
 */
void QwtPlotMarker::setLabelOrientation(Qt::Orientation orientation)
{
    QWT_D(d);
    if (orientation != d->labelOrientation) {
        d->labelOrientation = orientation;
        itemChanged();
    }
}

/**
 * @brief Get the label orientation
 * @return Label orientation
 * @sa setLabelOrientation(), labelAlignment()
 *
 */
Qt::Orientation QwtPlotMarker::labelOrientation() const
{
    QWT_DC(d);
    return d->labelOrientation;
}

/**
 * @brief Set the spacing
 * @details When the label is not centered on the marker position, the spacing
 *          is the distance between the position and the label.
 * @param[in] spacing Spacing
 * @sa spacing(), setLabelAlignment()
 *
 */
void QwtPlotMarker::setSpacing(int spacing)
{
    QWT_D(d);
    if (spacing < 0)
        spacing = 0;

    if (spacing == d->spacing)
        return;

    d->spacing = spacing;
    itemChanged();
}

/**
 * @brief Get the spacing
 * @return Spacing
 * @sa setSpacing()
 *
 */
int QwtPlotMarker::spacing() const
{
    QWT_DC(d);
    return d->spacing;
}

/**
 * @brief Build and assign a line pen
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 *
 */
void QwtPlotMarker::setLinePen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setLinePen(QPen(color, width, style));
}

/**
 * @brief Specify a pen for the line
 * @param[in] pen New pen
 * @sa linePen()
 *
 */
void QwtPlotMarker::setLinePen(const QPen& pen)
{
    QWT_D(d);
    if (pen != d->pen) {
        d->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the line pen
 * @return Line pen
 * @sa setLinePen()
 *
 */
const QPen& QwtPlotMarker::linePen() const
{
    QWT_DC(d);
    return d->pen;
}

/**
 * @brief Get the bounding rectangle
 * @return Bounding rectangle
 * @details width/height of -1 does not affect the autoscale calculation
 *
 */
QRectF QwtPlotMarker::boundingRect() const
{
    QWT_DC(d);
    // width/height of -1 does not affect the autoscale calculation

    switch (d->style) {
    case QwtPlotMarker::HLine:
        return QRectF(d->xValue, d->yValue, -1.0, 0.0);

    case QwtPlotMarker::VLine:
        return QRectF(d->xValue, d->yValue, 0.0, -1.0);

    default:
        return QRectF(d->xValue, d->yValue, 0.0, 0.0);
    }
}

/**
 * @brief Get the icon representing the marker on the legend
 * @param[in] index Index of the legend entry (usually there is only one)
 * @param[in] size Icon size
 * @return Icon representing the marker on the legend
 * @sa setLegendIconSize(), legendData()
 *
 */
QwtGraphic QwtPlotMarker::legendIcon(int index, const QSizeF& size) const
{
    QWT_DC(d);
    Q_UNUSED(index);

    if (size.isEmpty())
        return QwtGraphic();

    QwtGraphic icon;
    icon.setDefaultSize(size);
    icon.setRenderHint(QwtGraphic::RenderPensUnscaled, true);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPlotItem::RenderAntialiased));

    if (d->style != QwtPlotMarker::NoLine) {
        painter.setPen(d->pen);

        if (d->style == QwtPlotMarker::HLine || d->style == QwtPlotMarker::Cross) {
            const double y = 0.5 * size.height();

            QwtPainter::drawLine(&painter, 0.0, y, size.width(), y);
        }

        if (d->style == QwtPlotMarker::VLine || d->style == QwtPlotMarker::Cross) {
            const double x = 0.5 * size.width();

            QwtPainter::drawLine(&painter, x, 0.0, x, size.height());
        }
    }

    if (d->symbol) {
        const QRect r(0.0, 0.0, size.width(), size.height());
        d->symbol->drawSymbol(&painter, r);
    }

    return icon;
}
