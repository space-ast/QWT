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

#include "qwt_plot_spectrocurve.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"

#include <qpainter.h>

class QwtPlotSpectroCurve::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotSpectroCurve)
  public:
    PrivateData(QwtPlotSpectroCurve* p)
        : q_ptr(p)
        , colorRange( 0.0, 1000.0 )
        , penWidth( 0.0 )
        , paintAttributes( QwtPlotSpectroCurve::ClipPoints )
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    QwtColorMap* colorMap;
    mutable QwtInterval colorRange;
    mutable QVector< QRgb > colorTable;
    double penWidth;
    QwtPlotSpectroCurve::PaintAttributes paintAttributes;
};

/**
 * @brief Constructor
 * @param[in] title Title of the curve
 * 
 */
QwtPlotSpectroCurve::QwtPlotSpectroCurve( const QwtText& title )
    : QwtPlotSeriesItem( title ), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Constructor
 * @param[in] title Title of the curve
 *
 */
QwtPlotSpectroCurve::QwtPlotSpectroCurve( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) ), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Destructor
 *
 */
QwtPlotSpectroCurve::~QwtPlotSpectroCurve()
{
}

/**
 * @brief Initialize data members
 *
 */
void QwtPlotSpectroCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    setData( new QwtPoint3DSeriesData() );

    setZ( 20.0 );
}

/**
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotSpectroCurve
 * 
 */
int QwtPlotSpectroCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectroCurve;
}

/**
 * @brief Specify an attribute how to draw the curve
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa PaintAttribute, testPaintAttribute()
 * 
 */
void QwtPlotSpectroCurve::setPaintAttribute( PaintAttribute attribute, bool on )
{
    QWT_D(d);
    if ( on )
        d->paintAttributes |= attribute;
    else
        d->paintAttributes &= ~attribute;
}

/**
 * @brief Test if a paint attribute is enabled
 * @return True when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 * 
 */
bool QwtPlotSpectroCurve::testPaintAttribute( PaintAttribute attribute ) const
{
    QWT_DC(d);
    return ( d->paintAttributes & attribute );
}

/**
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of points
 * 
 */
void QwtPlotSpectroCurve::setSamples( const QVector< QwtPoint3D >& samples )
{
    setData( new QwtPoint3DSeriesData( samples ) );
}

/**
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting
 *          it when its not used anymore.
 * 
 */
void QwtPlotSpectroCurve::setSamples(
    QwtSeriesData< QwtPoint3D >* data )
{
    setData( data );
}

/**
 * @brief Change the color map
 * @details Often it is useful to display the mapping between intensities and
 *          colors as an additional plot axis, showing a color bar.
 * @param[in] colorMap Color map
 * @sa colorMap(), setColorRange(), QwtColorMap::color(),
 *     QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * 
 */
void QwtPlotSpectroCurve::setColorMap( QwtColorMap* colorMap )
{
    QWT_D(d);
    if ( colorMap != d->colorMap )
    {
        delete d->colorMap;
        d->colorMap = colorMap;
    }

    legendChanged();
    itemChanged();
}

/**
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color map used for mapping the intensity values to colors
 * @sa setColorMap(), setColorRange(), QwtColorMap::color()
 * 
 */
const QwtColorMap* QwtPlotSpectroCurve::colorMap() const
{
    QWT_DC(d);
    return d->colorMap;
}

/**
 * @brief Set the value interval that corresponds to the color map
 * @param[in] interval interval.minValue() corresponds to 0.0,
 *                     interval.maxValue() to 1.0 on the color map.
 * @sa colorRange(), setColorMap(), QwtColorMap::color()
 * 
 */
void QwtPlotSpectroCurve::setColorRange( const QwtInterval& interval )
{
    QWT_D(d);
    if ( interval != d->colorRange )
    {
        d->colorRange = interval;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the value interval that corresponds to the color map
 * @return Value interval that corresponds to the color map
 * @sa setColorRange(), setColorMap(), QwtColorMap::color()
 * 
 */
const QwtInterval& QwtPlotSpectroCurve::colorRange() const
{
    QWT_DC(d);
    return d->colorRange;
}

/**
 * @brief Assign a pen width
 * @param[in] penWidth New pen width
 * @sa penWidth()
 * 
 */
void QwtPlotSpectroCurve::setPenWidth(double penWidth)
{
    QWT_D(d);
    if ( penWidth < 0.0 )
        penWidth = 0.0;

    if ( d->penWidth != penWidth )
    {
        d->penWidth = penWidth;

        legendChanged();
        itemChanged();
    }
}

/**
 * @brief Get the pen width used to draw a dot
 * @return Pen width used to draw a dot
 * @sa setPenWidth()
 * 
 */
double QwtPlotSpectroCurve::penWidth() const
{
    QWT_DC(d);
    return d->penWidth;
}

/**
 * @brief Draw a subset of the points
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first sample to be painted
 * @param[in] to Index of the last sample to be painted. If to < 0, the series will be painted to its last sample.
 * @sa drawDots()
 * 
 */
void QwtPlotSpectroCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    drawDots( painter, xMap, yMap, canvasRect, from, to );
}

/*!
   Draw a subset of the points

   @param painter Painter
   @param xMap Maps x-values into pixel coordinates.
   @param yMap Maps y-values into pixel coordinates.
   @param canvasRect Contents rectangle of the canvas
   @param from Index of the first sample to be painted
   @param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

   @sa drawSeries()
 */
void QwtPlotSpectroCurve::drawDots( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    QWT_DC(d);
    if ( !d->colorRange.isValid() )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    const QwtColorMap::Format format = d->colorMap->format();
    if ( format == QwtColorMap::Indexed )
        d->colorTable = d->colorMap->colorTable256();

    const QwtSeriesData< QwtPoint3D >* series = data();

    for ( int i = from; i <= to; i++ )
    {
        const QwtPoint3D sample = series->sample( i );

        double xi = xMap.transform( sample.x() );
        double yi = yMap.transform( sample.y() );
        if ( doAlign )
        {
            xi = qRound( xi );
            yi = qRound( yi );
        }

        if ( d->paintAttributes & QwtPlotSpectroCurve::ClipPoints )
        {
            if ( !canvasRect.contains( xi, yi ) )
                continue;
        }

        if ( format == QwtColorMap::RGB )
        {
            const QRgb rgb = d->colorMap->rgb(
                d->colorRange.minValue(), d->colorRange.maxValue(), sample.z() );

            painter->setPen( QPen( QColor::fromRgba( rgb ), d->penWidth ) );
        }
        else
        {
            const unsigned char index = d->colorMap->colorIndex(
                256, d->colorRange.minValue(), d->colorRange.maxValue(), sample.z() );

            painter->setPen( QPen( QColor::fromRgba( d->colorTable[index] ),
                d->penWidth ) );
        }

        QwtPainter::drawPoint( painter, QPointF( xi, yi ) );
    }

    d->colorTable.clear();
}
