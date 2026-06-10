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

#include "qwt_plot_textlabel.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpaintengine.h>
#include <qpixmap.h>

static QRect qwtItemRect( int renderFlags,
    const QRectF& rect, const QSizeF& itemSize )
{
    int x;
    if ( renderFlags & Qt::AlignLeft )
    {
        x = rect.left();
    }
    else if ( renderFlags & Qt::AlignRight )
    {
        x = rect.right() - itemSize.width();
    }
    else
    {
        x = rect.center().x() - 0.5 * itemSize.width();
    }

    int y;
    if ( renderFlags & Qt::AlignTop )
    {
        y = rect.top();
    }
    else if ( renderFlags & Qt::AlignBottom )
    {
        y = rect.bottom() - itemSize.height();
    }
    else
    {
        y = rect.center().y() - 0.5 * itemSize.height();
    }

    return QRect( x, y, itemSize.width(), itemSize.height() );
}

class QwtPlotTextLabel::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotTextLabel)
  public:
    PrivateData(QwtPlotTextLabel* p)
        : q_ptr(p)
        , margin( 5 )
    {
    }

    QwtText text;
    int margin;

    mutable QPixmap pixmap;
};

/**
 * @brief Constructor
 * @details Initializes an text label with an empty text.
 *          Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: false
 *          - QwtPlotItem::Legend: false
 *          The z value is initialized by 150.
 * @sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 *
 */
QwtPlotTextLabel::QwtPlotTextLabel()
    : QwtPlotItem( QwtText( "Label" ) ), QWT_PIMPL_CONSTRUCT
{
    setItemAttribute( QwtPlotItem::AutoScale, false );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 150 );
}

/**
 * @brief Destructor
 *
 */
QwtPlotTextLabel::~QwtPlotTextLabel()
{
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotTextLabel
 *
 */
int QwtPlotTextLabel::rtti() const
{
    return QwtPlotItem::Rtti_PlotTextLabel;
}

/**
 * @brief Set the text
 * @param[in] text Text to be displayed
 * @details The label will be aligned to the plot canvas according to the alignment flags of text.
 * @sa text(), QwtText::renderFlags()
 *
 */
void QwtPlotTextLabel::setText( const QwtText& text )
{
    QWT_D(d);
    if ( d->text != text )
    {
        d->text = text;

        invalidateCache();
        itemChanged();
    }
}

/**
 * @brief Get the text
 * @return Text to be displayed
 * @sa setText()
 *
 */
QwtText QwtPlotTextLabel::text() const
{
    QWT_DC(d);
    return d->text;
}

/**
 * @brief Set the margin
 * @param[in] margin Margin
 * @details The margin is the distance between the contentsRect() of the plot canvas
 *          and the rectangle where the label can be displayed.
 * @sa margin(), textRect()
 *
 */
void QwtPlotTextLabel::setMargin( int margin )
{
    QWT_D(d);
    margin = qMax( margin, 0 );
    if ( d->margin != margin )
    {
        d->margin = margin;
        itemChanged();
    }
}

/**
 * @brief Get the margin
 * @return Margin added to the contentsMargins() of the canvas
 * @sa setMargin()
 *
 */
int QwtPlotTextLabel::margin() const
{
    QWT_DC(d);
    return d->margin;
}

/*!
   Draw the text label

   @param painter Painter
   @param xMap x Scale Map
   @param yMap y Scale Map
   @param canvasRect Contents rectangle of the canvas in painter coordinates

   @sa textRect()
 */

void QwtPlotTextLabel::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    QWT_DC(d);
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );

    const int m = d->margin;

    const QRectF rect = textRect( canvasRect.adjusted( m, m, -m, -m ),
        d->text.textSize( painter->font() ) );

    bool doCache = QwtPainter::roundingAlignment( painter );
    if ( doCache )
    {
        switch( painter->paintEngine()->type() )
        {
            case QPaintEngine::Picture:
            case QPaintEngine::User: // usually QwtGraphic
            {
                // don't use a cache for record/replay devices
                doCache = false;
                break;
            }
            default:;
        }
    }

    if ( doCache )
    {
        // when the paint device is aligning it is not one
        // where scalability matters ( PDF, SVG ).
        // As rendering a text label is an expensive operation
        // we use a cache.

        int pw = 0;
        if ( d->text.borderPen().style() != Qt::NoPen )
            pw = qMax( d->text.borderPen().width(), 1 );

        QRect pixmapRect;
        pixmapRect.setLeft( qwtFloor( rect.left() ) - pw );
        pixmapRect.setTop( qwtFloor( rect.top() ) - pw );
        pixmapRect.setRight( qwtCeil( rect.right() ) + pw );
        pixmapRect.setBottom( qwtCeil( rect.bottom() ) + pw );

#if QT_VERSION >= 0x050000
        const qreal pixelRatio = QwtPainter::devicePixelRatio( painter->device() );
        const QSize scaledSize = pixmapRect.size() * pixelRatio;
#else
        const QSize scaledSize = pixmapRect.size();
#endif

        if ( d->pixmap.isNull() ||
            ( scaledSize != d->pixmap.size() ) )
        {
            d->pixmap = QPixmap( scaledSize );
#if QT_VERSION >= 0x050000
            d->pixmap.setDevicePixelRatio( pixelRatio );
#endif
            d->pixmap.fill( Qt::transparent );

            const QRect r( pw, pw,
                pixmapRect.width() - 2 * pw, pixmapRect.height() - 2 * pw );

            QPainter pmPainter( &d->pixmap );
            d->text.draw( &pmPainter, r );
        }

        painter->drawPixmap( pixmapRect, d->pixmap );
    }
    else
    {
        d->text.draw( painter, rect );
    }
}

/**
 * @brief Align the text label
 * @param[in] rect Canvas rectangle with margins subtracted
 * @param[in] textSize Size required to draw the text
 * @return A rectangle aligned according the the alignment flags of the text.
 * @sa setMargin(), QwtText::renderFlags(), QwtText::textSize()
 *
 */
QRectF QwtPlotTextLabel::textRect(
    const QRectF& rect, const QSizeF& textSize ) const
{
    QWT_DC(d);
    return qwtItemRect( d->text.renderFlags(), rect, textSize );
}

/**
 * @brief Invalidate the cached pixmap
 *
 */
void QwtPlotTextLabel::invalidateCache()
{
    QWT_D(d);
    d->pixmap = QPixmap();
}
