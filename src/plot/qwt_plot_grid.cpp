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

#include "qwt_plot_grid.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"

#include <qpainter.h>
#include <qpen.h>

static inline bool qwtFuzzyGreaterOrEqual( double d1, double d2 )
{
    return ( d1 >= d2 ) || qFuzzyCompare( d1, d2 );
}

class QwtPlotGrid::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotGrid)

  public:
    PrivateData( QwtPlotGrid* p )
        : q_ptr( p )
        , xEnabled( true )
        , yEnabled( true )
        , xMinEnabled( false )
        , yMinEnabled( false )
    {
    }

    bool xEnabled;
    bool yEnabled;
    bool xMinEnabled;
    bool yMinEnabled;

    QwtScaleDiv xScaleDiv;
    QwtScaleDiv yScaleDiv;

    QPen majorPen;
    QPen minorPen;
};

/**
 * @brief Constructor
 * @details Enables major grid, disables minor grid
 *
 */
QwtPlotGrid::QwtPlotGrid()
    : QwtPlotItem( QwtText( "Grid" ) )
    , QWT_PIMPL_CONSTRUCT
{
    setItemInterest( QwtPlotItem::ScaleInterest, true );
    setZ( 10.0 );
}

/**
 * @brief Destructor
 *
 */
QwtPlotGrid::~QwtPlotGrid()
{
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotGrid
 *
 */
int QwtPlotGrid::rtti() const
{
    return QwtPlotItem::Rtti_PlotGrid;
}

/**
 * @brief Enable or disable vertical grid lines
 * @param[in] on Enable (true) or disable
 * @sa Minor grid lines can be enabled or disabled with enableXMin()
 *
 */
void QwtPlotGrid::enableX( bool on )
{
    QWT_D(d);
    if ( d->xEnabled != on )
    {
        d->xEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Enable or disable horizontal grid lines
 * @param[in] on Enable (true) or disable
 * @sa Minor grid lines can be enabled or disabled with enableYMin()
 *
 */
void QwtPlotGrid::enableY( bool on )
{
    QWT_D(d);
    if ( d->yEnabled != on )
    {
        d->yEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Enable or disable minor vertical grid lines
 * @param[in] on Enable (true) or disable
 * @sa enableX()
 *
 */
void QwtPlotGrid::enableXMin( bool on )
{
    QWT_D(d);
    if ( d->xMinEnabled != on )
    {
        d->xMinEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Enable or disable minor horizontal grid lines
 * @param[in] on Enable (true) or disable
 * @sa enableY()
 *
 */
void QwtPlotGrid::enableYMin( bool on )
{
    QWT_D(d);
    if ( d->yMinEnabled != on )
    {
        d->yMinEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Assign an x axis scale division
 * @param[in] scaleDiv Scale division
 *
 */
void QwtPlotGrid::setXDiv( const QwtScaleDiv& scaleDiv )
{
    QWT_D(d);
    if ( d->xScaleDiv != scaleDiv )
    {
        d->xScaleDiv = scaleDiv;
        itemChanged();
    }
}

/**
 * @brief Assign a y axis scale division
 * @param[in] scaleDiv Scale division
 *
 */
void QwtPlotGrid::setYDiv( const QwtScaleDiv& scaleDiv )
{
    QWT_D(d);
    if ( d->yScaleDiv != scaleDiv )
    {
        d->yScaleDiv = scaleDiv;
        itemChanged();
    }
}

/**
 * @brief Build and assign a pen for both major and minor grid lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 *
 */
void QwtPlotGrid::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/**
 * @brief Assign a pen for both major and minor grid lines
 * @param[in] pen Pen
 * @sa setMajorPen(), setMinorPen()
 *
 */
void QwtPlotGrid::setPen( const QPen& pen )
{
    QWT_D(d);
    if ( d->majorPen != pen || d->minorPen != pen )
    {
        d->majorPen = pen;
        d->minorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Build and assign a pen for major grid lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 *
 */
void QwtPlotGrid::setMajorPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setMajorPen( QPen( color, width, style ) );
}

/**
 * @brief Assign a pen for the major grid lines
 * @param[in] pen Pen
 * @sa majorPen(), setMinorPen(), setPen()
 *
 */
void QwtPlotGrid::setMajorPen( const QPen& pen )
{
    QWT_D(d);
    if ( d->majorPen != pen )
    {
        d->majorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Build and assign a pen for the minor grid lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 *
 */
void QwtPlotGrid::setMinorPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setMinorPen( QPen( color, width, style ) );
}

/**
 * @brief Assign a pen for the minor grid lines
 * @param[in] pen Pen
 * @sa minorPen(), setMajorPen(), setPen()
 *
 */
void QwtPlotGrid::setMinorPen( const QPen& pen )
{
    QWT_D(d);
    if ( d->minorPen != pen )
    {
        d->minorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Draw the grid
 * @details The grid is drawn into the bounding rectangle such that
 *          grid lines begin and end at the rectangle's borders. The X and Y
 *          maps are used to map the scale divisions into the drawing region screen.
 * @param[in] painter Painter
 * @param[in] xMap X axis map
 * @param[in] yMap Y axis
 * @param[in] canvasRect Contents rectangle of the plot canvas
 *
 */
void QwtPlotGrid::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    QWT_DC(d);

    //  draw minor grid lines
    QPen minorPen = d->minorPen;
    minorPen.setCapStyle( Qt::FlatCap );

    painter->setPen( minorPen );

    if ( d->xEnabled && d->xMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            d->xScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            d->xScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    if ( d->yEnabled && d->yMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            d->yScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            d->yScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    //  draw major grid lines
    QPen majorPen = d->majorPen;
    majorPen.setCapStyle( Qt::FlatCap );

    painter->setPen( majorPen );

    if ( d->xEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            d->xScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }

    if ( d->yEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            d->yScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }
}

void QwtPlotGrid::drawLines( QPainter* painter, const QRectF& canvasRect,
    Qt::Orientation orientation, const QwtScaleMap& scaleMap,
    const QList< double >& values ) const
{
    const double x1 = canvasRect.left();
    const double x2 = canvasRect.right() - 1.0;
    const double y1 = canvasRect.top();
    const double y2 = canvasRect.bottom() - 1.0;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < values.count(); i++ )
    {
        double value = scaleMap.transform( values[i] );
        if ( doAlign )
            value = qRound( value );

        if ( orientation == Qt::Horizontal )
        {
            if ( qwtFuzzyGreaterOrEqual( value, y1 ) &&
                qwtFuzzyGreaterOrEqual( y2, value ) )
            {
                QwtPainter::drawLine( painter, x1, value, x2, value );
            }
        }
        else
        {
            if ( qwtFuzzyGreaterOrEqual( value, x1 ) &&
                qwtFuzzyGreaterOrEqual( x2, value ) )
            {
                QwtPainter::drawLine( painter, value, y1, value, y2 );
            }
        }
    }
}

/**
 * @brief Get the pen for the major grid lines
 * @return Pen for major grid lines
 * @sa setMajorPen(), setMinorPen(), setPen()
 *
 */
const QPen& QwtPlotGrid::majorPen() const
{
    QWT_DC(d);
    return d->majorPen;
}

/**
 * @brief Get the pen for the minor grid lines
 * @return Pen for minor grid lines
 * @sa setMinorPen(), setMajorPen(), setPen()
 *
 */
const QPen& QwtPlotGrid::minorPen() const
{
    QWT_DC(d);
    return d->minorPen;
}

/**
 * @brief Check if vertical grid lines are enabled
 * @return true if vertical grid lines are enabled
 * @sa enableX()
 *
 */
bool QwtPlotGrid::xEnabled() const
{
    QWT_DC(d);
    return d->xEnabled;
}

/**
 * @brief Check if minor vertical grid lines are enabled
 * @return true if minor vertical grid lines are enabled
 * @sa enableXMin()
 *
 */
bool QwtPlotGrid::xMinEnabled() const
{
    QWT_DC(d);
    return d->xMinEnabled;
}

/**
 * @brief Check if horizontal grid lines are enabled
 * @return true if horizontal grid lines are enabled
 * @sa enableY()
 *
 */
bool QwtPlotGrid::yEnabled() const
{
    QWT_DC(d);
    return d->yEnabled;
}

/**
 * @brief Check if minor horizontal grid lines are enabled
 * @return true if minor horizontal grid lines are enabled
 * @sa enableYMin()
 *
 */
bool QwtPlotGrid::yMinEnabled() const
{
    QWT_DC(d);
    return d->yMinEnabled;
}


/**
 * @brief Get the scale division of the x axis
 * @return Scale division of the x axis
 *
 */
const QwtScaleDiv& QwtPlotGrid::xScaleDiv() const
{
    QWT_DC(d);
    return d->xScaleDiv;
}

/**
 * @brief Get the scale division of the y axis
 * @return Scale division of the y axis
 *
 */
const QwtScaleDiv& QwtPlotGrid::yScaleDiv() const
{
    QWT_DC(d);
    return d->yScaleDiv;
}

/**
 * @brief Update the grid to changes of the axes scale division
 * @param[in] xScaleDiv Scale division of the x-axis
 * @param[in] yScaleDiv Scale division of the y-axis
 * @sa QwtPlot::updateAxes()
 *
 */
void QwtPlotGrid::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    setXDiv( xScaleDiv );
    setYDiv( yScaleDiv );
}
