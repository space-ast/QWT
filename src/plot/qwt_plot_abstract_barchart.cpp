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

#include "qwt_plot_abstract_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"

static inline double qwtTransformWidth(
    const QwtScaleMap& map, double value, double width )
{
    const double w2 = 0.5 * width;

    const double v1 = map.transform( value - w2 );
    const double v2 = map.transform( value + w2 );

    return qAbs( v2 - v1 );
}

class QwtPlotAbstractBarChart::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotAbstractBarChart)

  public:
    PrivateData(QwtPlotAbstractBarChart* p)
        : q_ptr(p)
        , layoutPolicy( QwtPlotAbstractBarChart::AutoAdjustSamples )
        , layoutHint( 0.5 )
        , spacing( 10 )
        , margin( 5 )
        , baseline( 0.0 )
    {
    }

    QwtPlotAbstractBarChart::LayoutPolicy layoutPolicy;
    double layoutHint;
    int spacing;
    int margin;
    double baseline;
};

/**
 * @brief Constructor
 * @param[in] title Title of the chart
 */
QwtPlotAbstractBarChart::QwtPlotAbstractBarChart( const QwtText& title )
    : QwtPlotSeriesItem( title )
    , QWT_PIMPL_CONSTRUCT
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Margins, true );
    setZ( 19.0 );
}

/**
 * @brief Destructor
 */
QwtPlotAbstractBarChart::~QwtPlotAbstractBarChart()
{
}

/**
 * @brief Set the layout policy
 * @details The combination of layoutPolicy() and layoutHint() define how the width
 *          of the bars is calculated.
 * @param[in] policy Layout policy
 * @sa layoutPolicy(), layoutHint()
 */
void QwtPlotAbstractBarChart::setLayoutPolicy( LayoutPolicy policy )
{
    QWT_D(d);
    if ( policy != d->layoutPolicy )
    {
        d->layoutPolicy = policy;
        itemChanged();
    }
}

/**
 * @brief Get the layout policy
 * @details The combination of layoutPolicy() and layoutHint() define how the width
 *          of the bars is calculated.
 * @return Layout policy of the chart item
 * @sa setLayoutPolicy(), layoutHint()
 */
QwtPlotAbstractBarChart::LayoutPolicy QwtPlotAbstractBarChart::layoutPolicy() const
{
    QWT_DC(d);
    return d->layoutPolicy;
}

/**
 * @brief Set the layout hint
 * @details The combination of layoutPolicy() and layoutHint() define how the width
 *          of the bars is calculated.
 * @param[in] hint Layout hint
 * @sa LayoutPolicy, layoutPolicy(), layoutHint()
 */
void QwtPlotAbstractBarChart::setLayoutHint( double hint )
{
    QWT_D(d);
    hint = qwtMaxF( 0.0, hint );
    if ( hint != d->layoutHint )
    {
        d->layoutHint = hint;
        itemChanged();
    }
}

/**
 * @brief Get the layout hint
 * @details The combination of layoutPolicy() and layoutHint() define how the width
 *          of the bars is calculated.
 * @return Layout hint of the chart item
 * @sa LayoutPolicy, setLayoutHint(), layoutPolicy()
 */
double QwtPlotAbstractBarChart::layoutHint() const
{
    QWT_DC(d);
    return d->layoutHint;
}

/**
 * @brief Set the spacing between bars
 * @details The spacing is the distance between 2 samples ( bars for QwtPlotBarChart or
 *          a group of bars for QwtPlotMultiBarChart ) in paint device coordinates.
 * @param[in] spacing Spacing in pixels
 * @sa spacing()
 */
void QwtPlotAbstractBarChart::setSpacing( int spacing )
{
    QWT_D(d);
    spacing = qMax( spacing, 0 );
    if ( spacing != d->spacing )
    {
        d->spacing = spacing;
        itemChanged();
    }
}

/**
 * @brief Get the spacing between bars
 * @return Spacing between 2 samples ( bars or groups of bars )
 * @sa setSpacing(), margin()
 */
int QwtPlotAbstractBarChart::spacing() const
{
    QWT_DC(d);
    return d->spacing;
}
/**
 * @brief Set the margin around the bars
 * @details The margin is the distance between the outmost bars and the contentsRect()
 *          of the canvas. The default setting is 5 pixels.
 * @param[in] margin Margin in pixels
 * @sa spacing(), margin()
 */
void QwtPlotAbstractBarChart::setMargin( int margin )
{
    QWT_D(d);
    margin = qMax( margin, 0 );
    if ( margin != d->margin )
    {
        d->margin = margin;
        itemChanged();
    }
}

/**
 * @brief Get the margin around the bars
 * @return Margin between the outmost bars and the contentsRect() of the canvas.
 * @sa setMargin(), spacing()
 */
int QwtPlotAbstractBarChart::margin() const
{
    QWT_DC(d);
    return d->margin;
}

/**
 * @brief Set the baseline value
 * @details The baseline is the origin for the chart. Each bar is
 *          painted from the baseline in the direction of the sample
 *          value. In case of a horizontal orientation() the baseline
 *          is interpreted as x - otherwise as y - value.
 *          The default value for the baseline is 0.
 * @param[in] value Value for the baseline
 * @sa baseline(), QwtPlotSeriesItem::orientation()
 */
void QwtPlotAbstractBarChart::setBaseline( double value )
{
    QWT_D(d);
    if ( value != d->baseline )
    {
        d->baseline = value;
        itemChanged();
    }
}

/**
 * @brief Get the baseline value
 * @return Value for the origin of the bar chart
 * @sa setBaseline(), QwtPlotSeriesItem::orientation()
 */
double QwtPlotAbstractBarChart::baseline() const
{
    QWT_DC(d);
    return d->baseline;
}

/*!
   Calculate the width for a sample in paint device coordinates

   @param map Scale map for the corresponding scale
   @param canvasSize Size of the canvas in paint device coordinates
   @param boundingSize Bounding size of the chart in plot coordinates
                       ( used in AutoAdjustSamples mode )
   @param value Value of the sample

   @return Sample width
   @sa layoutPolicy(), layoutHint()
 */
double QwtPlotAbstractBarChart::sampleWidth( const QwtScaleMap& map,
    double canvasSize, double boundingSize, double value ) const
{
    QWT_DC(d);
    double width;

    switch( d->layoutPolicy )
    {
        case ScaleSamplesToAxes:
        {
            width = qwtTransformWidth( map, value, d->layoutHint );
            break;
        }
        case ScaleSampleToCanvas:
        {
            width = canvasSize * d->layoutHint;
            break;
        }
        case FixedSampleSize:
        {
            width = d->layoutHint;
            break;
        }
        case AutoAdjustSamples:
        default:
        {
            const size_t numSamples = dataSize();

            double w = 1.0;
            if ( numSamples > 1 )
            {
                w = qAbs( boundingSize / ( numSamples - 1 ) );
            }

            width = qwtTransformWidth( map, value, w );
            width -= d->spacing;
            width = qwtMaxF( width, d->layoutHint );
        }
    }

    return width;
}

/**
 * @brief Calculate a hint for the canvas margin
 * @details Bar charts need to reserve some space for displaying the bars
 *          for the first and the last sample. The hint is calculated
 *          from the layoutHint() depending on the layoutPolicy().
 *          The margins are in target device coordinates ( pixels on screen ).
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * @param[out] left Returns the left margin
 * @param[out] top Returns the top margin
 * @param[out] right Returns the right margin
 * @param[out] bottom Returns the bottom margin
 * @sa layoutPolicy(), layoutHint(), QwtPlotItem::Margins,
 *     QwtPlot::getCanvasMarginsHint(), QwtPlot::updateCanvasMargins()
 */
void QwtPlotAbstractBarChart::getCanvasMarginHint( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& canvasRect,
    double& left, double& top, double& right, double& bottom ) const
{
    QWT_DC(d);
    double hint = -1.0;

    switch( layoutPolicy() )
    {
        case ScaleSampleToCanvas:
        {
            if ( orientation() == Qt::Vertical )
                hint = 0.5 * canvasRect.width() * d->layoutHint;
            else
                hint = 0.5 * canvasRect.height() * d->layoutHint;

            break;
        }
        case FixedSampleSize:
        {
            hint = 0.5 * d->layoutHint;
            break;
        }
        case AutoAdjustSamples:
        case ScaleSamplesToAxes:
        default:
        {
            const size_t numSamples = dataSize();
            if ( numSamples <= 0 )
                break;

            // doesn't work for nonlinear scales

            const QRectF br = dataRect();
            double spacing = 0.0;
            double sampleWidthS = 1.0;

            if ( layoutPolicy() == ScaleSamplesToAxes )
            {
                sampleWidthS = qwtMaxF( d->layoutHint, 0.0 );
            }
            else
            {
                spacing = d->spacing;

                if ( numSamples > 1 )
                {
                    sampleWidthS = qAbs( br.width() / ( numSamples - 1 ) );
                }
            }

            double ds, w;
            if ( orientation() == Qt::Vertical )
            {
                ds = qAbs( xMap.sDist() );
                w = canvasRect.width();
            }
            else
            {
                ds = qAbs( yMap.sDist() );
                w = canvasRect.height();
            }

            const double sampleWidthP = ( w - spacing * ( numSamples - 1 ) )
                * sampleWidthS / ( ds + sampleWidthS );

            hint = 0.5 * sampleWidthP;
            hint += qMax( d->margin, 0 );
        }
    }

    if ( orientation() == Qt::Vertical )
    {
        left = right = hint;
        top = bottom = -1.0; // no hint
    }
    else
    {
        left = right = -1.0; // no hint
        top = bottom = hint;
    }
}
