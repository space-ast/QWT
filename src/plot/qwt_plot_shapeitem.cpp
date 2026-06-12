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

#include "qwt_plot_shapeitem.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_painter.h"
#include "qwt_weeding_curve_fitter.h"
#include "qwt_clipper.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>

static QPainterPath qwtTransformPath( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QPainterPath& path, bool doAlign )
{
    QPainterPath shape;
    shape.setFillRule( path.fillRule() );

    for ( int i = 0; i < path.elementCount(); i++ )
    {
        const QPainterPath::Element element = path.elementAt( i );

        double x = xMap.transform( element.x );
        double y = yMap.transform( element.y );

        switch( element.type )
        {
            case QPainterPath::MoveToElement:
            {
                if ( doAlign )
                {
                    x = qRound( x );
                    y = qRound( y );
                }

                shape.moveTo( x, y );
                break;
            }
            case QPainterPath::LineToElement:
            {
                if ( doAlign )
                {
                    x = qRound( x );
                    y = qRound( y );
                }

                shape.lineTo( x, y );
                break;
            }
            case QPainterPath::CurveToElement:
            {
                const QPainterPath::Element element1 = path.elementAt( ++i );
                const double x1 = xMap.transform( element1.x );
                const double y1 = yMap.transform( element1.y );

                const QPainterPath::Element element2 = path.elementAt( ++i );
                const double x2 = xMap.transform( element2.x );
                const double y2 = yMap.transform( element2.y );

                shape.cubicTo( x, y, x1, y1, x2, y2 );
                break;
            }
            case QPainterPath::CurveToDataElement:
            {
                break;
            }
        }
    }

    return shape;
}


class QwtPlotShapeItem::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotShapeItem)
  public:
    PrivateData(QwtPlotShapeItem* p)
        : q_ptr(p)
        , legendMode( QwtPlotShapeItem::LegendColor )
        , renderTolerance( 0.0 )
    {
    }

    QwtPlotShapeItem::PaintAttributes paintAttributes;
    QwtPlotShapeItem::LegendMode legendMode;

    double renderTolerance;
    QRectF boundingRect;

    QPen pen;
    QBrush brush;
    QPainterPath shape;
};

/**
 * @brief Constructor
 * @param[in] title Title
 * @details Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 *
 */
QwtPlotShapeItem::QwtPlotShapeItem( const QString& title )
    : QwtPlotItem( QwtText( title ) ), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Constructor
 * @param[in] title Title
 * @details Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 *
 */
QwtPlotShapeItem::QwtPlotShapeItem( const QwtText& title )
    : QwtPlotItem( title ), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Destructor
 *
 */
QwtPlotShapeItem::~QwtPlotShapeItem()
{
}

void QwtPlotShapeItem::init()
{
    QWT_D(d);
    d->boundingRect = QwtPlotItem::boundingRect();

    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 8.0 );
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotShape
 *
 */
int QwtPlotShapeItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotShape;
}

/**
 * @brief Specify an attribute how to draw the shape
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 *
 */
void QwtPlotShapeItem::setPaintAttribute( PaintAttribute attribute, bool on )
{
    QWT_D(d);
    if ( on )
        d->paintAttributes |= attribute;
    else
        d->paintAttributes &= ~attribute;
}

/**
 * @brief Check if a paint attribute is enabled
 * @param[in] attribute Paint attribute
 * @return True, when attribute is enabled
 * @sa setPaintAttribute()
 *
 */
bool QwtPlotShapeItem::testPaintAttribute( PaintAttribute attribute ) const
{
    QWT_DC(d);
    return ( d->paintAttributes & attribute );
}

/**
 * @brief Set the mode how to represent the item on the legend
 * @param[in] mode Mode
 * @sa legendMode()
 *
 */
void QwtPlotShapeItem::setLegendMode( LegendMode mode )
{
    QWT_D(d);
    if ( mode != d->legendMode )
    {
        d->legendMode = mode;
        legendChanged();
    }
}

/*!
   @return Mode how to represent the item on the legend
   @sa legendMode()
 */
QwtPlotShapeItem::LegendMode QwtPlotShapeItem::legendMode() const
{
    QWT_DC(d);
    return d->legendMode;
}

//! Bounding rectangle of the shape
QRectF QwtPlotShapeItem::boundingRect() const
{
    QWT_DC(d);
    return d->boundingRect;
}

/*!
   @brief Set a path built from a rectangle

   @param rect Rectangle
   @sa setShape(), setPolygon(), shape()
 */
void QwtPlotShapeItem::setRect( const QRectF& rect )
{
    QPainterPath path;
    path.addRect( rect );

    setShape( path );
}

/*!
   @brief Set a path built from a polygon

   @param polygon Polygon
   @sa setShape(), setRect(), shape()
 */
void QwtPlotShapeItem::setPolygon( const QPolygonF& polygon )
{
    QPainterPath shape;
    shape.addPolygon( polygon );

    setShape( shape );
}

/*!
   @brief Set the shape to be displayed

   @param shape Shape
   @sa setShape(), shape()
 */
void QwtPlotShapeItem::setShape( const QPainterPath& shape )
{
    QWT_D(d);
    if ( shape != d->shape )
    {
        d->shape = shape;
        if ( shape.isEmpty() )
        {
            d->boundingRect = QwtPlotItem::boundingRect();
        }
        else
        {
            d->boundingRect = shape.boundingRect();
        }

        itemChanged();
    }
}

/*!
   @return Shape to be displayed
   @sa setShape()
 */
QPainterPath QwtPlotShapeItem::shape() const
{
    QWT_DC(d);
    return d->shape;
}

/*!
   Build and assign a pen

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   @param color Pen color
   @param width Pen width
   @param style Pen style

   @sa pen(), brush()
 */
void QwtPlotShapeItem::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   @brief Assign a pen

   The pen is used to draw the outline of the shape

   @param pen Pen
   @sa pen(), brush()
 */
void QwtPlotShapeItem::setPen( const QPen& pen )
{
    QWT_D(d);
    if ( pen != d->pen )
    {
        d->pen = pen;
        itemChanged();
    }
}

/*!
    @return Pen used to draw the outline of the shape
    @sa setPen(), brush()
 */
QPen QwtPlotShapeItem::pen() const
{
    QWT_DC(d);
    return d->pen;
}

/*!
   Assign a brush.

   The brush is used to fill the path

   @param brush Brush
   @sa brush(), pen()
 */
void QwtPlotShapeItem::setBrush( const QBrush& brush )
{
    QWT_D(d);
    if ( brush != d->brush )
    {
        d->brush = brush;
        itemChanged();
    }
}

/*!
   @return Brush used to fill the shape
   @sa setBrush(), pen()
 */
QBrush QwtPlotShapeItem::brush() const
{
    QWT_DC(d);
    return d->brush;
}

/*!
   @brief Set the tolerance for the weeding optimization

   After translating the shape into target device coordinate
   ( usually widget geometries ) the painter path can be simplified
   by a point weeding algorithm ( Douglas-Peucker ).

   For shapes built from curves and ellipses weeding might
   have the opposite effect because they have to be expanded
   to polygons.

   @param tolerance Accepted error when reducing the number of points
                   A value <= 0.0 disables weeding.

   @sa renderTolerance(), QwtWeedingCurveFitter
 */
void QwtPlotShapeItem::setRenderTolerance( double tolerance )
{
    QWT_D(d);
    tolerance = qwtMaxF( tolerance, 0.0 );

    if ( tolerance != d->renderTolerance )
    {
        d->renderTolerance = tolerance;
        itemChanged();
    }
}

/*!
   @return Tolerance for the weeding optimization
   @sa setRenderTolerance()
 */
double QwtPlotShapeItem::renderTolerance() const
{
    QWT_DC(d);
    return d->renderTolerance;
}

/*!
   Draw the shape item

   @param painter Painter
   @param xMap X-Scale Map
   @param yMap Y-Scale Map
   @param canvasRect Contents rect of the plot canvas
 */
void QwtPlotShapeItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    QWT_DC(d);
    if ( d->shape.isEmpty() )
        return;

    if ( d->pen.style() == Qt::NoPen
        && d->brush.style() == Qt::NoBrush )
    {
        return;
    }

    const QRectF cr = QwtScaleMap::invTransform(
        xMap, yMap, canvasRect.toRect() );

    const QRectF& br = d->boundingRect;

    if ( ( br.left() > cr.right() ) || ( br.right() < cr.left() )
        || ( br.top() > cr.bottom() ) || ( br.bottom() < cr.top() ) )
    {
        // outside the visible area
        return;
    }

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    QPainterPath path = qwtTransformPath( xMap, yMap,
        d->shape, doAlign );

    if ( testPaintAttribute( QwtPlotShapeItem::ClipPolygons ) )
    {
        const qreal pw = QwtPainter::effectivePenWidth( painter->pen() );
        const QRectF clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );

        QPainterPath clippedPath;
        clippedPath.setFillRule( path.fillRule() );

        QList< QPolygonF > polygons = path.toSubpathPolygons();
        for ( int i = 0; i < polygons.size(); i++ )
        {
            QwtClipper::clipPolygonF( clipRect, polygons[i], true );
            clippedPath.addPolygon( polygons[i] );

        }

        path = clippedPath;
    }

    if ( d->renderTolerance > 0.0 )
    {
        QwtWeedingCurveFitter fitter( d->renderTolerance );

        QPainterPath fittedPath;
        fittedPath.setFillRule( path.fillRule() );

        const QList< QPolygonF > polygons = path.toSubpathPolygons();
        for ( int i = 0; i < polygons.size(); i++ )
            fittedPath.addPolygon( fitter.fitCurve( polygons[ i ] ) );

        path = fittedPath;
    }

    painter->setPen( d->pen );
    painter->setBrush( d->brush );

    painter->drawPath( path );
}

/*!
   @return A rectangle filled with the color of the brush ( or the pen )

   @param index Index of the legend entry
                ( usually there is only one )
   @param size Icon size

   @sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPlotShapeItem::legendIcon( int index,
    const QSizeF& size ) const
{
    QWT_DC(d);
    Q_UNUSED( index );

    QwtGraphic icon;
    icon.setDefaultSize( size );

    if ( size.isEmpty() )
        return icon;

    if ( d->legendMode == QwtPlotShapeItem::LegendShape )
    {
        const QRectF& br = d->boundingRect;

        QPainter painter( &icon );
        painter.setRenderHint( QPainter::Antialiasing,
            testRenderHint( QwtPlotItem::RenderAntialiased ) );

        painter.translate( -br.topLeft() );

        painter.setPen( d->pen );
        painter.setBrush( d->brush );
        painter.drawPath( d->shape );
    }
    else
    {
        QColor iconColor;
        if ( d->brush.style() != Qt::NoBrush )
            iconColor = d->brush.color();
        else
            iconColor = d->pen.color();

        icon = defaultIcon( iconColor, size );
    }

    return icon;
}

