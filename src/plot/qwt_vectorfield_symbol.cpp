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

#include "qwt_vectorfield_symbol.h"

#include <qpainter.h>
#include <qpainterpath.h>

/**
 * @brief Constructor
 *
 */
QwtVectorFieldSymbol::QwtVectorFieldSymbol()
{
}

/**
 * @brief Destructor
 *
 */
QwtVectorFieldSymbol::~QwtVectorFieldSymbol()
{
}

class QwtVectorFieldArrow::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtVectorFieldArrow)
  public:
    PrivateData( QwtVectorFieldArrow* p, qreal headW, qreal tailW )
        : q_ptr( p )
        , headWidth( headW )
        , tailWidth( tailW )
        , length( headW + 4.0 )
    {
        /*
            Arrow is drawn horizontally, pointing into positive x direction
            with tip at 0,0.
         */

        path.lineTo( -headWidth, headWidth );
        path.lineTo( -headWidth, tailWidth );
        path.lineTo( -length, tailWidth );
        path.lineTo( -length, -tailWidth );
        path.lineTo( -headWidth, -tailWidth );
        path.lineTo( -headWidth, -headWidth );

        path.closeSubpath();
    }

    void setLength( qreal l )
    {
        length = qMax( l, headWidth );

        path.setElementPositionAt( 3, -length, tailWidth );
        path.setElementPositionAt( 4, -length, -tailWidth );
    }

    const qreal headWidth;
    const qreal tailWidth;
    qreal length;

    QPainterPath path;

};

/*!
    @brief Constructor

    The length is initialized by headWidth + 4

    @param headWidth Width of the triangular head
    @param tailWidth Width of the arrow tail

    @sa setLength()
    *
 */
QwtVectorFieldArrow::QwtVectorFieldArrow( qreal headWidth, qreal tailWidth )
    : m_data( qwt_make_unique< PrivateData >( this, headWidth, tailWidth ) )
{
}

/**
 * @brief Destructor
 *
 */
QwtVectorFieldArrow::~QwtVectorFieldArrow()
{
}

/**
 * @brief Set the length of the arrow
 * @param[in] length Length of the arrow in pixels
 * @sa length()
 */
void QwtVectorFieldArrow::setLength( qreal length )
{
    QWT_D(d);
    d->setLength( length );
}

/**
 * @brief Return the length of the arrow
 * @return Length of the arrow in pixels
 * @sa setLength()
 */
qreal QwtVectorFieldArrow::length() const
{
    QWT_DC(d);
    return d->length;
}

/**
 * @brief Draw the arrow using QPainter
 * @param[in] painter Painter to use for drawing
 */
void QwtVectorFieldArrow::paint( QPainter* painter ) const
{
    QWT_DC(d);
    painter->drawPath( d->path );
}

class QwtVectorFieldThinArrow::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtVectorFieldThinArrow)
  public:
    PrivateData( QwtVectorFieldThinArrow* p, qreal headW )
        : q_ptr( p )
        , headWidth( headW )
        , length( headW + 4.0 )
    {
        path.lineTo( -headWidth, headWidth * 0.6 );
        path.moveTo( 0, 0 );
        path.lineTo( -headWidth, -headWidth * 0.6 );
        path.moveTo( 0, 0 );
        path.lineTo( -length, 0 );
    }

    const qreal headWidth;
    qreal length;

    QPainterPath path;
};

/*!
    @brief Constructor

    The length is initialized by headWidth + 4

    @param headWidth Width of the triangular head
    @sa setLength()
    *
 */
QwtVectorFieldThinArrow::QwtVectorFieldThinArrow( qreal headWidth )
    : m_data( qwt_make_unique< PrivateData >( this, headWidth ) )
{
}

/**
 * @brief Destructor
 *
 */
QwtVectorFieldThinArrow::~QwtVectorFieldThinArrow()
{
}

/**
 * @brief Set the length of the arrow
 * @details Adjusts the head width proportionally when length is very small
 * @param[in] length Length of the arrow in pixels
 * @sa length()
 */
void QwtVectorFieldThinArrow::setLength( qreal length )
{
    QWT_D(d);
    d->length = length;

    const qreal headWidth = qMin( d->headWidth, length / 3.0 );

    QPainterPath& path = d->path;

    path.setElementPositionAt( 1, -headWidth, headWidth * 0.6 );
    path.setElementPositionAt( 3, -headWidth, -headWidth * 0.6 );
    path.setElementPositionAt( 5, -length, 0 );
}

/**
 * @brief Return the length of the arrow
 * @return Length of the arrow in pixels
 * @sa setLength()
 */
qreal QwtVectorFieldThinArrow::length() const
{
    QWT_DC(d);
    return d->length;
}

/**
 * @brief Draw the arrow using QPainter
 * @param[in] p Painter to use for drawing
 */
void QwtVectorFieldThinArrow::paint(QPainter* p) const
{
    QWT_DC(d);
    p->drawPath( d->path );
}
