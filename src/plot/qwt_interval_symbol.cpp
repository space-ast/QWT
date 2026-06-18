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

#include "qwt_interval_symbol.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qmath.h>

class QwtIntervalSymbol::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtIntervalSymbol)
public:
    PrivateData(QwtIntervalSymbol* p) : q_ptr(p), style(QwtIntervalSymbol::NoSymbol), width(6)
    {
    }

    bool operator==(const PrivateData& other) const
    {
        return (style == other.style) && (width == other.width) && (brush == other.brush) && (pen == other.pen);
    }

    QwtIntervalSymbol::Style style;
    int width;

    QPen pen;
    QBrush brush;
};

/**
 * @brief Constructor
 * @param[in] style Style of the symbol
 * @sa setStyle(), style(), Style
 */
QwtIntervalSymbol::QwtIntervalSymbol(Style style) : QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    d->style = style;
}

/**
 * @brief Copy constructor
 * @param[in] other Symbol to copy
 */
QwtIntervalSymbol::QwtIntervalSymbol(const QwtIntervalSymbol& other) : QWT_PIMPL_CONSTRUCT
{
    *m_data = *other.m_data;
}

/**
 * @brief Destructor
 */
QwtIntervalSymbol::~QwtIntervalSymbol()
{
}

/**
 * @brief Assignment operator
 * @param[in] other Symbol to assign
 * @return Reference to this symbol
 */
QwtIntervalSymbol& QwtIntervalSymbol::operator=(const QwtIntervalSymbol& other)
{
    QWT_D(d);
    *d = *other.m_data;
    return *this;
}

/**
 * @brief Compare two symbols
 * @param[in] other Symbol to compare
 * @return True if symbols are equal
 */
bool QwtIntervalSymbol::operator==(const QwtIntervalSymbol& other) const
{
    QWT_DC(d);
    return *d == *other.m_data;
}

/**
 * @brief Compare two symbols
 * @param[in] other Symbol to compare
 * @return True if symbols are not equal
 */
bool QwtIntervalSymbol::operator!=(const QwtIntervalSymbol& other) const
{
    QWT_DC(d);
    return !(*d == *other.m_data);
}

/**
 * @brief Specify the symbol style
 * @param[in] style Style
 * @sa style(), Style
 */
void QwtIntervalSymbol::setStyle(Style style)
{
    QWT_D(d);
    d->style = style;
}

/**
 * @brief Get the current symbol style
 * @return Current symbol style
 * @sa setStyle()
 */
QwtIntervalSymbol::Style QwtIntervalSymbol::style() const
{
    QWT_DC(d);
    return d->style;
}

/**
 * @brief Specify the width of the symbol
 * @details It is used depending on the style.
 * @param[in] width Width
 * @sa width(), setStyle()
 */
void QwtIntervalSymbol::setWidth(int width)
{
    QWT_D(d);
    d->width = width;
}

/**
 * @brief Get the width of the symbol
 * @return Width of the symbol
 * @sa setWidth(), setStyle()
 */
int QwtIntervalSymbol::width() const
{
    QWT_DC(d);
    return d->width;
}

/**
 * @brief Assign a brush
 * @details The brush is used for the Box style.
 * @param[in] brush Brush
 * @sa brush()
 */
void QwtIntervalSymbol::setBrush(const QBrush& brush)
{
    QWT_D(d);
    d->brush = brush;
}

/**
 * @brief Get the brush
 * @return Brush
 * @sa setBrush()
 */
const QBrush& QwtIntervalSymbol::brush() const
{
    QWT_DC(d);
    return d->brush;
}

/**
 * @brief Build and assign a pen
 * @details In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
 *          non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 */
void QwtIntervalSymbol::setPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setPen(QPen(color, width, style));
}

/**
 * @brief Assign a pen
 * @param[in] pen Pen
 * @sa pen(), setBrush()
 */
void QwtIntervalSymbol::setPen(const QPen& pen)
{
    QWT_D(d);
    d->pen = pen;
}

/**
 * @brief Get the pen
 * @return Pen
 * @sa setPen(), brush()
 */
const QPen& QwtIntervalSymbol::pen() const
{
    QWT_DC(d);
    return d->pen;
}

/**
 * @brief Draw a symbol depending on its style
 * @param[in] painter Painter
 * @param[in] orientation Orientation
 * @param[in] from Start point of the interval in target device coordinates
 * @param[in] to End point of the interval in target device coordinates
 * @sa setStyle()
 */
void QwtIntervalSymbol::draw(QPainter* painter, Qt::Orientation orientation, const QPointF& from, const QPointF& to) const
{
    QWT_DC(d);
    const qreal pw = QwtPainter::effectivePenWidth(painter->pen());

    QPointF p1 = from;
    QPointF p2 = to;
    if (QwtPainter::roundingAlignment(painter)) {
        p1 = p1.toPoint();
        p2 = p2.toPoint();
    }

    switch (d->style) {
    case QwtIntervalSymbol::Bar: {
        QwtPainter::drawLine(painter, p1, p2);
        if (d->width > pw) {
            if ((orientation == Qt::Horizontal) && (p1.y() == p2.y())) {
                const double sw = d->width;

                const double y = p1.y() - sw / 2;
                QwtPainter::drawLine(painter, p1.x(), y, p1.x(), y + sw);
                QwtPainter::drawLine(painter, p2.x(), y, p2.x(), y + sw);
            } else if ((orientation == Qt::Vertical) && (p1.x() == p2.x())) {
                const double sw = d->width;

                const double x = p1.x() - sw / 2;
                QwtPainter::drawLine(painter, x, p1.y(), x + sw, p1.y());
                QwtPainter::drawLine(painter, x, p2.y(), x + sw, p2.y());
            } else {
                const double sw = d->width;

                const double dx    = p2.x() - p1.x();
                const double dy    = p2.y() - p1.y();
                const double angle = std::atan2(dy, dx) + M_PI_2;
                double dw2         = sw / 2.0;

                const double cx = qFastCos(angle) * dw2;
                const double sy = qFastSin(angle) * dw2;

                QwtPainter::drawLine(painter, p1.x() - cx, p1.y() - sy, p1.x() + cx, p1.y() + sy);
                QwtPainter::drawLine(painter, p2.x() - cx, p2.y() - sy, p2.x() + cx, p2.y() + sy);
            }
        }
        break;
    }
    case QwtIntervalSymbol::Box: {
        if (d->width <= pw) {
            QwtPainter::drawLine(painter, p1, p2);
        } else {
            if ((orientation == Qt::Horizontal) && (p1.y() == p2.y())) {
                const double sw = d->width;

                const double y = p1.y() - d->width / 2;
                QwtPainter::drawRect(painter, p1.x(), y, p2.x() - p1.x(), sw);
            } else if ((orientation == Qt::Vertical) && (p1.x() == p2.x())) {
                const double sw = d->width;

                const double x = p1.x() - d->width / 2;
                QwtPainter::drawRect(painter, x, p1.y(), sw, p2.y() - p1.y());
            } else {
                const double sw = d->width;

                const double dx    = p2.x() - p1.x();
                const double dy    = p2.y() - p1.y();
                const double angle = std::atan2(dy, dx) + M_PI_2;
                double dw2         = sw / 2.0;

                const double cx = qFastCos(angle) * dw2;
                const double sy = qFastSin(angle) * dw2;

                QPolygonF polygon;
                polygon += QPointF(p1.x() - cx, p1.y() - sy);
                polygon += QPointF(p1.x() + cx, p1.y() + sy);
                polygon += QPointF(p2.x() + cx, p2.y() + sy);
                polygon += QPointF(p2.x() - cx, p2.y() - sy);

                QwtPainter::drawPolygon(painter, polygon);
            }
        }
        break;
    }
    default:;
    }
}
