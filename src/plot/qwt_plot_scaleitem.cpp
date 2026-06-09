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

#include "qwt_plot_scaleitem.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_interval.h"
#include "qwt_text.h"

#include <qpalette.h>
#include <qpainter.h>

class QwtPlotScaleItem::PrivateData
{
  public:
    PrivateData()
        : position( 0.0 )
        , borderDistance( -1 )
        , scaleDivFromAxis( true )
        , scaleDraw( new QwtScaleDraw() )
    {
    }

    ~PrivateData()
    {
        delete scaleDraw;
    }

    QwtInterval scaleInterval( const QRectF&,
        const QwtScaleMap&, const QwtScaleMap& ) const;

    QPalette palette;
    QFont font;
    double position;
    int borderDistance;
    bool scaleDivFromAxis;
    QwtScaleDraw* scaleDraw;
};

QwtInterval QwtPlotScaleItem::PrivateData::scaleInterval( const QRectF& canvasRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap ) const
{
    QwtInterval interval;
    if ( scaleDraw->orientation() == Qt::Horizontal )
    {
        interval.setMinValue( xMap.invTransform( canvasRect.left() ) );
        interval.setMaxValue( xMap.invTransform( canvasRect.right() - 1 ) );
    }
    else
    {
        interval.setMinValue( yMap.invTransform( canvasRect.bottom() - 1 ) );
        interval.setMaxValue( yMap.invTransform( canvasRect.top() ) );
    }

    return interval;
}

/**
 * @brief Constructor for scale item at the position pos
 * @param[in] alignment In case of QwtScaleDraw::BottomScale or QwtScaleDraw::TopScale,
 *                       the scale item is corresponding to the xAxis(),
 *                       otherwise it corresponds to the yAxis().
 * @param[in] pos x or y position, depending on the corresponding axis.
 * @sa setPosition(), setAlignment()
 *
 */
QwtPlotScaleItem::QwtPlotScaleItem(
        QwtScaleDraw::Alignment alignment, const double pos )
    : QwtPlotItem( QwtText( "Scale" ) )
{
    m_data = new PrivateData;
    m_data->position = pos;
    m_data->scaleDraw->setAlignment( alignment );

    setItemInterest( QwtPlotItem::ScaleInterest, true );
    setZ( 11.0 );
}

/**
 * @brief Destructor
 *
 */
QwtPlotScaleItem::~QwtPlotScaleItem()
{
    delete m_data;
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotScale
 *
 */
int QwtPlotScaleItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotScale;
}

/**
 * @brief Assign a scale division
 * @param[in] scaleDiv Scale division
 * @details When assigning a scaleDiv the scale division won't be synchronized
 *          with the corresponding axis anymore.
 * @sa scaleDiv(), setScaleDivFromAxis(), isScaleDivFromAxis()
 *
 */
void QwtPlotScaleItem::setScaleDiv( const QwtScaleDiv& scaleDiv )
{
    m_data->scaleDivFromAxis = false;
    m_data->scaleDraw->setScaleDiv( scaleDiv );
}

/**
 * @brief Get the scale division
 * @return Scale division
 *
 */
const QwtScaleDiv& QwtPlotScaleItem::scaleDiv() const
{
    return m_data->scaleDraw->scaleDiv();
}

/**
 * @brief Enable/Disable the synchronization of the scale division with the corresponding axis
 * @param[in] on true/false
 * @sa isScaleDivFromAxis()
 *
 */
void QwtPlotScaleItem::setScaleDivFromAxis( bool on )
{
    if ( on != m_data->scaleDivFromAxis )
    {
        m_data->scaleDivFromAxis = on;
        if ( on )
        {
            const QwtPlot* plt = plot();
            if ( plt )
            {
                updateScaleDiv( plt->axisScaleDiv( xAxis() ),
                    plt->axisScaleDiv( yAxis() ) );
                itemChanged();
            }
        }
    }
}

/**
 * @brief Check if synchronization of the scale division with the corresponding axis is enabled
 * @return True, if the synchronization of the scale division with the corresponding axis is enabled.
 * @sa setScaleDiv(), setScaleDivFromAxis()
 *
 */
bool QwtPlotScaleItem::isScaleDivFromAxis() const
{
    return m_data->scaleDivFromAxis;
}

/**
 * @brief Set the palette
 * @param[in] palette Palette
 * @sa QwtAbstractScaleDraw::draw(), palette()
 *
 */
void QwtPlotScaleItem::setPalette( const QPalette& palette )
{
    if ( palette != m_data->palette )
    {
        m_data->palette = palette;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the palette
 * @return Palette
 * @sa setPalette()
 *
 */
QPalette QwtPlotScaleItem::palette() const
{
    return m_data->palette;
}

/**
 * @brief Change the tick label font
 * @param[in] font Font
 * @sa font()
 *
 */
void QwtPlotScaleItem::setFont( const QFont& font )
{
    if ( font != m_data->font )
    {
        m_data->font = font;
        itemChanged();
    }
}

/**
 * @brief Get the tick label font
 * @return Font
 * @sa setFont()
 *
 */
QFont QwtPlotScaleItem::font() const
{
    return m_data->font;
}

/**
 * @brief Set a scale draw
 * @param[in] scaleDraw Object responsible for drawing scales
 * @details The main use case for replacing the default QwtScaleDraw is
 *          to overload QwtAbstractScaleDraw::label, to replace or swallow tick labels.
 * @sa scaleDraw()
 *
 */
void QwtPlotScaleItem::setScaleDraw( QwtScaleDraw* scaleDraw )
{
    if ( scaleDraw == nullptr )
        return;

    if ( scaleDraw != m_data->scaleDraw )
        delete m_data->scaleDraw;

    m_data->scaleDraw = scaleDraw;

    const QwtPlot* plt = plot();
    if ( plt )
    {
        updateScaleDiv( plt->axisScaleDiv( xAxis() ),
            plt->axisScaleDiv( yAxis() ) );
    }

    itemChanged();
}

/**
 * @brief Get the scale draw (const version)
 * @return Scale draw
 * @sa setScaleDraw()
 *
 */
const QwtScaleDraw* QwtPlotScaleItem::scaleDraw() const
{
    return m_data->scaleDraw;
}

/**
 * @brief Get the scale draw
 * @return Scale draw
 * @sa setScaleDraw()
 *
 */
QwtScaleDraw* QwtPlotScaleItem::scaleDraw()
{
    return m_data->scaleDraw;
}

/**
 * @brief Change the position of the scale
 * @param[in] pos New position
 * @details The position is interpreted as y value for horizontal axes
 *          and as x value for vertical axes. The border distance is set to -1.
 * @sa position(), setAlignment()
 *
 */
void QwtPlotScaleItem::setPosition( double pos )
{
    if ( m_data->position != pos )
    {
        m_data->position = pos;
        m_data->borderDistance = -1;
        itemChanged();
    }
}

/**
 * @brief Get the position of the scale
 * @return Position of the scale
 * @sa setPosition(), setAlignment()
 *
 */
double QwtPlotScaleItem::position() const
{
    return m_data->position;
}

/**
 * @brief Align the scale to the canvas
 * @param[in] distance Number of pixels between the canvas border and the backbone of the scale.
 * @details If distance is >= 0 the scale will be aligned to a border of the contents rectangle
 *          of the canvas. If alignment() is QwtScaleDraw::LeftScale, the scale will be aligned
 *          to the right border, if it is QwtScaleDraw::TopScale it will be aligned to the bottom
 *          (and vice versa). If distance is < 0 the scale will be at the position().
 * @sa setPosition(), borderDistance()
 *
 */
void QwtPlotScaleItem::setBorderDistance( int distance )
{
    if ( distance < 0 )
        distance = -1;

    if ( distance != m_data->borderDistance )
    {
        m_data->borderDistance = distance;
        itemChanged();
    }
}

/**
 * @brief Get the distance from a canvas border
 * @return Distance from a canvas border
 * @sa setBorderDistance(), setPosition()
 *
 */
int QwtPlotScaleItem::borderDistance() const
{
    return m_data->borderDistance;
}

/**
 * @brief Change the alignment of the scale
 * @param[in] alignment Alignment
 * @details The alignment sets the orientation of the scale and the position of the ticks:
 *          - QwtScaleDraw::BottomScale: horizontal, ticks below
 *          - QwtScaleDraw::TopScale: horizontal, ticks above
 *          - QwtScaleDraw::LeftScale: vertical, ticks left
 *          - QwtScaleDraw::RightScale: vertical, ticks right
 *          For horizontal scales the position corresponds to QwtPlotItem::yAxis(),
 *          otherwise to QwtPlotItem::xAxis().
 * @sa scaleDraw(), QwtScaleDraw::alignment(), setPosition()
 *
 */
void QwtPlotScaleItem::setAlignment( QwtScaleDraw::Alignment alignment )
{
    QwtScaleDraw* sd = m_data->scaleDraw;
    if ( sd->alignment() != alignment )
    {
        sd->setAlignment( alignment );
        itemChanged();
    }
}

/**
 * @brief Draw the scale
 * @param[in] painter Painter
 * @param[in] xMap X Scale Map
 * @param[in] yMap Y Scale Map
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 *
 */
void QwtPlotScaleItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    QwtScaleDraw* sd = m_data->scaleDraw;

    if ( m_data->scaleDivFromAxis )
    {
        const QwtInterval interval =
            m_data->scaleInterval( canvasRect, xMap, yMap );

        if ( interval != sd->scaleDiv().interval() )
        {
            QwtScaleDiv scaleDiv = sd->scaleDiv();
            scaleDiv.setInterval( interval );
            sd->setScaleDiv( scaleDiv );
        }
    }

    QPen pen = painter->pen();
    pen.setStyle( Qt::SolidLine );
    painter->setPen( pen );

    if ( sd->orientation() == Qt::Horizontal )
    {
        double y;
        if ( m_data->borderDistance >= 0 )
        {
            if ( sd->alignment() == QwtScaleDraw::BottomScale )
                y = canvasRect.top() + m_data->borderDistance;
            else
            {
                y = canvasRect.bottom() - m_data->borderDistance;
            }

        }
        else
        {
            y = yMap.transform( m_data->position );
        }

        if ( y < canvasRect.top() || y > canvasRect.bottom() )
            return;

        sd->move( canvasRect.left(), y );
        sd->setLength( canvasRect.width() - 1 );

        QwtTransform* transform = nullptr;
        if ( xMap.transformation() )
            transform = xMap.transformation()->copy();

        sd->setTransformation( transform );
    }
    else // == Qt::Vertical
    {
        double x;
        if ( m_data->borderDistance >= 0 )
        {
            if ( sd->alignment() == QwtScaleDraw::RightScale )
                x = canvasRect.left() + m_data->borderDistance;
            else
            {
                x = canvasRect.right() - m_data->borderDistance;
            }
        }
        else
        {
            x = xMap.transform( m_data->position );
        }
        if ( x < canvasRect.left() || x > canvasRect.right() )
            return;

        sd->move( x, canvasRect.top() );
        sd->setLength( canvasRect.height() - 1 );

        QwtTransform* transform = nullptr;
        if ( yMap.transformation() )
            transform = yMap.transformation()->copy();

        sd->setTransformation( transform );
    }

    painter->setFont( m_data->font );

    sd->draw( painter, m_data->palette );
}

/**
 * @brief Update the item to changes of the axes scale division
 * @param[in] xScaleDiv Scale division of the x-axis
 * @param[in] yScaleDiv Scale division of the y-axis
 * @details In case of isScaleDivFromAxis(), the scale draw is synchronized to the correspond axis.
 * @sa QwtPlot::updateAxes()
 *
 */
void QwtPlotScaleItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    QwtScaleDraw* scaleDraw = m_data->scaleDraw;

    if ( m_data->scaleDivFromAxis && scaleDraw )
    {
        const QwtScaleDiv& scaleDiv =
            scaleDraw->orientation() == Qt::Horizontal ? xScaleDiv : yScaleDiv;

        const QwtPlot* plt = plot();
        if ( plt != nullptr )
        {
            const QRectF canvasRect = plt->canvas()->contentsRect();

            const QwtInterval interval = m_data->scaleInterval(
                canvasRect, plt->canvasMap( xAxis() ), plt->canvasMap( yAxis() ) );

            QwtScaleDiv sd = scaleDiv;
            sd.setInterval( interval );

            if ( sd != scaleDraw->scaleDiv() )
            {
                // the internal label cache of QwtScaleDraw
                // is cleared here, so better avoid pointless
                // assignments.

                scaleDraw->setScaleDiv( sd );
            }
        }
        else
        {
            scaleDraw->setScaleDiv( scaleDiv );
        }
    }
}
