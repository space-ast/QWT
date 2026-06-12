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

#include "qwt_plot_directpainter.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_seriesitem.h"

#include <qpainter.h>
#include <qevent.h>
#include <qpixmap.h>

static inline void qwtRenderItem(
    QPainter* painter, const QRect& canvasRect,
    QwtPlotSeriesItem* seriesItem, int from, int to )
{
    // A minor performance improvement is possible
    // with caching the maps. TODO ...

    QwtPlot* plot = seriesItem->plot();
    const QwtScaleMap xMap = plot->canvasMap( seriesItem->xAxis() );
    const QwtScaleMap yMap = plot->canvasMap( seriesItem->yAxis() );

    painter->setRenderHint( QPainter::Antialiasing,
        seriesItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );
    seriesItem->drawSeries( painter, xMap, yMap, canvasRect, from, to );
}

static inline bool qwtHasBackingStore( const QwtPlotCanvas* canvas )
{
    return canvas->testPaintAttribute( QwtPlotCanvas::BackingStore )
           && canvas->backingStore() && !canvas->backingStore()->isNull();
}

class QwtPlotDirectPainter::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotDirectPainter)
  public:
    PrivateData(QwtPlotDirectPainter* p)
        : q_ptr(p)
        , hasClipping( false )
        , seriesItem( nullptr )
        , from( 0 )
        , to( 0 )
    {
    }

    QwtPlotDirectPainter::Attributes attributes;

    bool hasClipping;
    QRegion clipRegion;

    QPainter painter;

    QwtPlotSeriesItem* seriesItem;
    int from;
    int to;
};

/**
 * @brief Constructor
 * @param[in] parent Parent object
 */
QwtPlotDirectPainter::QwtPlotDirectPainter( QObject* parent )
    : QObject( parent ), QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Destructor
 */
QwtPlotDirectPainter::~QwtPlotDirectPainter()
{
}

/**
 * @brief Change an attribute
 * @param[in] attribute Attribute to change
 * @param[in] on On/Off
 * @sa Attribute, testAttribute()
 */
void QwtPlotDirectPainter::setAttribute( Attribute attribute, bool on )
{
    QWT_D(d);
    if ( bool( d->attributes & attribute ) != on )
    {
        if ( on )
            d->attributes |= attribute;
        else
            d->attributes &= ~attribute;

        if ( ( attribute == AtomicPainter ) && on )
            reset();
    }
}

/**
 * @brief Test an attribute
 * @param[in] attribute Attribute to be tested
 * @return True, when attribute is enabled
 * @sa Attribute, setAttribute()
 */
bool QwtPlotDirectPainter::testAttribute( Attribute attribute ) const
{
    QWT_DC(d);
    return d->attributes & attribute;
}

/**
 * @brief Enable or disable clipping
 * @param[in] enable Enables clipping if true, disables it otherwise
 * @sa hasClipping(), clipRegion(), setClipRegion()
 */
void QwtPlotDirectPainter::setClipping( bool enable )
{
    QWT_D(d);
    d->hasClipping = enable;
}

/**
 * @brief Check if clipping is enabled
 * @return true, when clipping is enabled
 * @sa setClipping(), clipRegion(), setClipRegion()
 */
bool QwtPlotDirectPainter::hasClipping() const
{
    QWT_DC(d);
    return d->hasClipping;
}

/**
 * @brief Assign a clip region and enable clipping
 * @details Depending on the environment setting a proper clip region might improve
 *          the performance heavily. E.g. on Qt embedded only the clipped part of
 *          the backing store will be copied to a (maybe unaccelerated) frame buffer device.
 * @param[in] region Clip region
 * @sa clipRegion(), hasClipping(), setClipping()
 */
void QwtPlotDirectPainter::setClipRegion( const QRegion& region )
{
    QWT_D(d);
    d->clipRegion = region;
    d->hasClipping = true;
}

/**
 * @brief Get the currently set clip region
 * @return Currently set clip region
 * @sa setClipRegion(), setClipping(), hasClipping()
 */
QRegion QwtPlotDirectPainter::clipRegion() const
{
    QWT_DC(d);
    return d->clipRegion;
}

/**
 * @brief Draw a set of points of a seriesItem
 * @details When observing a measurement while it is running, new points have to be
 *          added to an existing seriesItem. drawSeries() can be used to display them
 *          avoiding a complete redraw of the canvas.
 *          Setting plot()->canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
 *          will result in faster painting, if the paint engine of the canvas widget
 *          supports this feature.
 * @param[in] seriesItem Item to be painted
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the series will be painted to its last point.
 */
void QwtPlotDirectPainter::drawSeries(
    QwtPlotSeriesItem* seriesItem, int from, int to )
{
    QWT_D(d);
    if ( seriesItem == nullptr || seriesItem->plot() == nullptr )
        return;

    QWidget* canvas = seriesItem->plot()->canvas();
    const QRect canvasRect = canvas->contentsRect();

    QwtPlotCanvas* plotCanvas = qobject_cast< QwtPlotCanvas* >( canvas );

    if ( plotCanvas && qwtHasBackingStore( plotCanvas ) )
    {
        QPainter painter( const_cast< QPixmap* >( plotCanvas->backingStore() ) );

        if ( d->hasClipping )
            painter.setClipRegion( d->clipRegion );

        qwtRenderItem( &painter, canvasRect, seriesItem, from, to );

        painter.end();

        if ( testAttribute( QwtPlotDirectPainter::FullRepaint ) )
        {
            plotCanvas->repaint();
            return;
        }
    }

    bool immediatePaint = true;
    if ( !canvas->testAttribute( Qt::WA_WState_InPaintEvent ) )
    {
#if QT_VERSION < 0x050000
        if ( !canvas->testAttribute( Qt::WA_PaintOutsidePaintEvent ) )
#endif
        immediatePaint = false;
    }

    if ( immediatePaint )
    {
        if ( !d->painter.isActive() )
        {
            reset();

            d->painter.begin( canvas );
            canvas->installEventFilter( this );
        }

        if ( d->hasClipping )
        {
            d->painter.setClipRegion(
                QRegion( canvasRect ) & d->clipRegion );
        }
        else
        {
            if ( !d->painter.hasClipping() )
                d->painter.setClipRect( canvasRect );
        }

        qwtRenderItem( &d->painter, canvasRect, seriesItem, from, to );

        if ( d->attributes & QwtPlotDirectPainter::AtomicPainter )
        {
            reset();
        }
        else
        {
            if ( d->hasClipping )
                d->painter.setClipping( false );
        }
    }
    else
    {
        reset();

        d->seriesItem = seriesItem;
        d->from = from;
        d->to = to;

        QRegion clipRegion = canvasRect;
        if ( d->hasClipping )
            clipRegion &= d->clipRegion;

        canvas->installEventFilter( this );
        canvas->repaint(clipRegion);
        canvas->removeEventFilter( this );

        d->seriesItem = nullptr;
    }
}

/**
 * @brief Close the internal QPainter
 */
void QwtPlotDirectPainter::reset()
{
    QWT_D(d);
    if ( d->painter.isActive() )
    {
        QWidget* w = static_cast< QWidget* >( d->painter.device() );
        if ( w )
            w->removeEventFilter( this );

        d->painter.end();
    }
}

/**
 * @brief Event filter
 * @param[in] object Object
 * @param[in] event Event
 * @return True if the event was handled
 */
bool QwtPlotDirectPainter::eventFilter( QObject*, QEvent* event )
{
    QWT_D(d);
    if ( event->type() == QEvent::Paint )
    {
        reset();

        if ( d->seriesItem )
        {
            const QPaintEvent* pe = static_cast< QPaintEvent* >( event );

            QWidget* canvas = d->seriesItem->plot()->canvas();

            QPainter painter( canvas );
            painter.setClipRegion( pe->region() );

            bool doCopyCache = testAttribute( CopyBackingStore );

            if ( doCopyCache )
            {
                QwtPlotCanvas* plotCanvas =
                    qobject_cast< QwtPlotCanvas* >( canvas );
                if ( plotCanvas )
                {
                    doCopyCache = qwtHasBackingStore( plotCanvas );
                    if ( doCopyCache )
                    {
                        painter.drawPixmap( plotCanvas->rect().topLeft(),
                            *plotCanvas->backingStore() );
                    }
                }
            }

            if ( !doCopyCache )
            {
                qwtRenderItem( &painter, canvas->contentsRect(),
                    d->seriesItem, d->from, d->to );
            }

            return true; // don't call QwtPlotCanvas::paintEvent()
        }
    }

    return false;
}
