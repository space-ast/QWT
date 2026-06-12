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
 ******************************************************************************/

#include "qwt_plot_item.h"
#include "qwt_text.h"
#include "qwt_plot.h"
#include "qwt_legend_data.h"
#include "qwt_scale_map.h"
#include "qwt_graphic.h"

#include <qpainter.h>

class QwtPlotItem::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotItem)

  public:
    PrivateData( QwtPlotItem* p )
        : q_ptr( p )
        , plot( nullptr )
        , isVisible( true )
        , renderThreadCount( 1 )
        , z( 0.0 )
        , xAxisId( QwtAxis::XBottom )
        , yAxisId( QwtAxis::YLeft )
        , legendIconSize( 8, 8 )
    {
    }

    QwtPlot* plot;

    bool isVisible;

    QwtPlotItem::ItemAttributes attributes;
    QwtPlotItem::ItemInterests interests;

    QwtPlotItem::RenderHints renderHints;
    uint renderThreadCount;

    double z;

    QwtAxisId xAxisId;
    QwtAxisId yAxisId;

    QwtText title;
    QSize legendIconSize;
};

/**
 * @brief Default constructor
 * @details Constructs a QwtPlotItem with default settings.
 */
QwtPlotItem::QwtPlotItem()
    : QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Constructor with title as QString
 * @param[in] title Title of the item
 * @details Constructs a QwtPlotItem with the given title.
 */
QwtPlotItem::QwtPlotItem( const QString& title )
    : QWT_PIMPL_CONSTRUCT
{
    m_data->title = title;
}

/**
 * @brief Constructor with title as QwtText
 * @param[in] title Title of the item
 * @details Constructs a QwtPlotItem with the given title.
 */
QwtPlotItem::QwtPlotItem( const QwtText& title )
    : QWT_PIMPL_CONSTRUCT
{
    m_data->title = title;
}

/**
 * @brief Destructor
 * @details Destroys the QwtPlotItem and detaches it from any plot.
 */
QwtPlotItem::~QwtPlotItem()
{
    attach( nullptr );
}

/**
 * @brief Attach the item to a plot
 * @param[in] plot Plot widget to attach to
 * @details This method will attach a QwtPlotItem to the QwtPlot argument.
 *          It will first detach the QwtPlotItem from any plot from a previous
 *          call to attach (if necessary). If a nullptr argument is passed,
 *          it will detach from any QwtPlot it was attached to.
 * @sa detach()
 */
void QwtPlotItem::attach( QwtPlot* plot )
{
    QWT_D( d );

    if ( plot == d->plot )
        return;

    if ( d->plot )
        d->plot->attachItem( this, false );

    d->plot = plot;

    if ( d->plot )
        d->plot->attachItem( this, true );
}

/**
 * @brief Detach the item from the plot
 * @details This method detaches a QwtPlotItem from any QwtPlot it has been
 *          associated with. detach() is equivalent to calling attach( nullptr ).
 * @sa attach()
 */
void QwtPlotItem::detach()
{
    attach( nullptr );
}

/**
 * @brief Get runtime type information
 * @return rtti value for the specific class
 * @details Return rtti for the specific class represented. QwtPlotItem is simply
 *          a virtual interface class, and base classes will implement this method
 *          with specific rtti values so a user can differentiate them.
 *          The rtti value is useful for environments, where the runtime type
 *          information is disabled and it is not possible to do a dynamic_cast<...>.
 * @sa RttiValues
 */
int QwtPlotItem::rtti() const
{
    return Rtti_PlotItem;
}

/**
 * @brief Get the plot the item is attached to
 * @return Attached plot, or nullptr if not attached
 */
QwtPlot* QwtPlotItem::plot() const
{
    QWT_DC( d );
    return d->plot;
}

/**
 * @brief Get the z-value
 * @return Z-value of the item
 * @details Plot items are painted in increasing z-order.
 * @sa setZ(), QwtPlotDict::itemList()
 */
double QwtPlotItem::z() const
{
    QWT_DC( d );
    return d->z;
}

/**
 * @brief Set the z-value
 * @param[in] z Z-value for the item
 * @details Plot items are painted in increasing z-order.
 * @sa z(), QwtPlotDict::itemList()
 */
void QwtPlotItem::setZ( double z )
{
    QWT_D( d );

    if ( d->z != z )
    {
        if ( d->plot ) // update the z order
            d->plot->attachItem( this, false );

        d->z = z;

        if ( d->plot )
            d->plot->attachItem( this, true );

        itemChanged();
    }
}

/**
 * @brief Set the title using a QString
 * @param[in] title Title text
 * @sa title()
 */
void QwtPlotItem::setTitle( const QString& title )
{
    setTitle( QwtText( title ) );
}

/**
 * @brief Set the title using a QwtText
 * @param[in] title Title text
 * @sa title()
 */
void QwtPlotItem::setTitle( const QwtText& title )
{
    QWT_D( d );

    if ( d->title != title )
    {
        d->title = title;

        legendChanged();
#if 0
        itemChanged();
#endif
    }
}

/**
 * @brief Get the title of the item
 * @return Title of the item
 * @sa setTitle()
 */
const QwtText& QwtPlotItem::title() const
{
    QWT_DC( d );
    return d->title;
}

/**
 * @brief Set an item attribute
 * @param[in] attribute Attribute type to set
 * @param[in] on true to enable, false to disable
 * @details Toggle an item attribute.
 * @sa testItemAttribute(), ItemInterest
 */
void QwtPlotItem::setItemAttribute( ItemAttribute attribute, bool on )
{
    QWT_D( d );

    if ( d->attributes.testFlag( attribute ) != on )
    {
        if ( on )
            d->attributes |= attribute;
        else
            d->attributes &= ~attribute;

        if ( attribute == QwtPlotItem::Legend )
        {
            if ( on )
            {
                legendChanged();
            }
            else
            {
                /*
                    In the special case of taking an item from
                    the legend we can't use legendChanged() as
                    it depends on QwtPlotItem::Legend being enabled
                 */
                if ( d->plot )
                    d->plot->updateLegend( this );
            }
        }

        itemChanged();
    }
}

/**
 * @brief Test an item attribute
 * @param[in] attribute Attribute type to test
 * @return true if the attribute is enabled, false otherwise
 * @sa setItemAttribute(), ItemInterest
 */
bool QwtPlotItem::testItemAttribute( ItemAttribute attribute ) const
{
    QWT_DC( d );
    return d->attributes.testFlag( attribute );
}

/**
 * @brief Set an item interest
 * @param[in] interest Interest type to set
 * @param[in] on true to enable, false to disable
 * @details Toggle an item interest.
 * @sa testItemInterest(), ItemAttribute
 */
void QwtPlotItem::setItemInterest( ItemInterest interest, bool on )
{
    QWT_D( d );

    if ( d->interests.testFlag( interest ) != on )
    {
        if ( on )
            d->interests |= interest;
        else
            d->interests &= ~interest;

        itemChanged();
    }
}

/**
 * @brief Test an item interest
 * @param[in] interest Interest type to test
 * @return true if the interest is enabled, false otherwise
 * @sa setItemInterest(), ItemAttribute
 */
bool QwtPlotItem::testItemInterest( ItemInterest interest ) const
{
    QWT_DC( d );
    return d->interests.testFlag( interest );
}

/**
 * @brief Set a render hint
 * @param[in] hint Render hint to set
 * @param[in] on true to enable, false to disable
 * @details Toggle a render hint.
 * @sa testRenderHint(), RenderHint
 */
void QwtPlotItem::setRenderHint( RenderHint hint, bool on )
{
    QWT_D( d );

    if ( d->renderHints.testFlag( hint ) != on )
    {
        if ( on )
            d->renderHints |= hint;
        else
            d->renderHints &= ~hint;

        itemChanged();
    }
}

/**
 * @brief Test a render hint
 * @param[in] hint Render hint to test
 * @return true if the render hint is enabled, false otherwise
 * @sa setRenderHint(), RenderHint
 */
bool QwtPlotItem::testRenderHint( RenderHint hint ) const
{
    QWT_DC( d );
    return d->renderHints.testFlag( hint );
}

/**
 * @brief Set the number of render threads
 * @param[in] numThreads Number of threads to be used for rendering.
 *                       If numThreads is set to 0, the system specific
 *                       ideal thread count is used.
 * @details On multi core systems rendering of certain plot item
 *          (e.g. QwtPlotRasterItem) can be done in parallel in several threads.
 *          The default setting is set to 1.
 */
void QwtPlotItem::setRenderThreadCount( uint numThreads )
{
    QWT_D( d );
    d->renderThreadCount = numThreads;
}

/**
 * @brief Get the number of render threads
 * @return Number of threads to be used for rendering.
 *         If renderThreadCount() is set to 0, the system specific
 *         ideal thread count is used.
 */
uint QwtPlotItem::renderThreadCount() const
{
    QWT_DC( d );
    return d->renderThreadCount;
}

/**
 * @brief Set the size of the legend icon
 * @param[in] size Icon size
 * @details The default setting is 8x8 pixels.
 * @sa legendIconSize(), legendIcon()
 */
void QwtPlotItem::setLegendIconSize( const QSize& size )
{
    QWT_D( d );

    if ( d->legendIconSize != size )
    {
        d->legendIconSize = size;
        legendChanged();
    }
}

/**
 * @brief Get the legend icon size
 * @return Legend icon size
 * @sa setLegendIconSize(), legendIcon()
 */
QSize QwtPlotItem::legendIconSize() const
{
    QWT_DC( d );
    return d->legendIconSize;
}

/**
 * @brief Get the icon representing the item on the legend
 * @param[in] index Index of the legend entry (usually there is only one)
 * @param[in] size Icon size
 * @return Icon representing the item on the legend
 * @details The default implementation returns an invalid icon.
 * @sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPlotItem::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index )
    Q_UNUSED( size )

    return QwtGraphic();
}

//! Return a default icon from a brush
QwtGraphic QwtPlotItem::defaultIcon(
    const QBrush& brush, const QSizeF& size ) const
{
    QwtGraphic icon;
    if ( !size.isEmpty() )
    {
        icon.setDefaultSize( size );

        QRectF r( 0, 0, size.width(), size.height() );

        QPainter painter( &icon );
        painter.fillRect( r, brush );
    }

    return icon;
}

/**
 * @brief Show the item
 * @details Equivalent to calling setVisible( true ).
 * @sa hide(), setVisible(), isVisible()
 */
void QwtPlotItem::show()
{
    setVisible( true );
}

/**
 * @brief Hide the item
 * @details Equivalent to calling setVisible( false ).
 * @sa show(), setVisible(), isVisible()
 */
void QwtPlotItem::hide()
{
    setVisible( false );
}

/**
 * @brief Set the visibility of the item
 * @param[in] on Show if true, otherwise hide
 * @sa isVisible(), show(), hide()
 */
void QwtPlotItem::setVisible( bool on )
{
    QWT_D( d );

    if ( on != d->isVisible )
    {
        d->isVisible = on;
        itemChanged();
    }
}

/**
 * @brief Check if the item is visible
 * @return true if visible, false otherwise
 * @sa setVisible(), show(), hide()
 */
bool QwtPlotItem::isVisible() const
{
    QWT_DC( d );
    return d->isVisible;
}

/**
 * @brief Notify the plot that the item has changed
 * @details Update the legend and call QwtPlot::autoRefresh() for the parent plot.
 * @sa QwtPlot::legendChanged(), QwtPlot::autoRefresh()
 */
void QwtPlotItem::itemChanged()
{
    QWT_D( d );

    if ( d->plot )
        d->plot->autoRefresh();
}

/**
 * @brief Notify that the legend has changed
 * @details Update the legend of the parent plot.
 * @sa QwtPlot::updateLegend(), itemChanged()
 */
void QwtPlotItem::legendChanged()
{
    QWT_D( d );

    if ( d->attributes.testFlag( QwtPlotItem::Legend ) && d->plot )
        d->plot->updateLegend( this );
}

/**
 * @brief Set X and Y axes
 * @param[in] xAxisId X Axis identifier
 * @param[in] yAxisId Y Axis identifier
 * @details The item will be painted according to the coordinates of its axes.
 * @sa setXAxis(), setYAxis(), xAxis(), yAxis()
 */
void QwtPlotItem::setAxes( QwtAxisId xAxisId, QwtAxisId yAxisId )
{
    QWT_D( d );

    if ( QwtAxis::isXAxis( xAxisId ) )
        d->xAxisId = xAxisId;

    if ( QwtAxis::isYAxis( yAxisId ) )
        d->yAxisId = yAxisId;

    itemChanged();
}

/**
 * @brief Set the X axis
 * @param[in] axisId X Axis identifier
 * @details The item will be painted according to the coordinates of its axis.
 * @sa setAxes(), setYAxis(), xAxis()
 */
void QwtPlotItem::setXAxis( QwtAxisId axisId )
{
    QWT_D( d );

    if ( QwtAxis::isXAxis( axisId ) )
    {
        d->xAxisId = axisId;
        itemChanged();
    }
}

/**
 * @brief Set the Y axis
 * @param[in] axisId Y Axis identifier
 * @details The item will be painted according to the coordinates of its axis.
 * @sa setAxes(), setXAxis(), yAxis()
 */
void QwtPlotItem::setYAxis( QwtAxisId axisId )
{
    QWT_D( d );

    if ( QwtAxis::isYAxis( axisId ) )
    {
        d->yAxisId = axisId;
        itemChanged();
    }
}

/**
 * @brief Get the X axis
 * @return X Axis identifier
 */
QwtAxisId QwtPlotItem::xAxis() const
{
    QWT_DC( d );
    return d->xAxisId;
}

/**
 * @brief Get the Y axis
 * @return Y Axis identifier
 */
QwtAxisId QwtPlotItem::yAxis() const
{
    QWT_DC( d );
    return d->yAxisId;
}

/**
 * @brief Get the bounding rectangle
 * @return An invalid bounding rect: QRectF(1.0, 1.0, -2.0, -2.0)
 * @note A width or height < 0.0 is ignored by the autoscaler.
 */
QRectF QwtPlotItem::boundingRect() const
{
    return QRectF( 1.0, 1.0, -2.0, -2.0 ); // invalid
}

/**
 * @brief Calculate a hint for the canvas margin
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * @param[out] left Returns the left margin
 * @param[out] top Returns the top margin
 * @param[out] right Returns the right margin
 * @param[out] bottom Returns the bottom margin
 * @details When the QwtPlotItem::Margins flag is enabled the plot item
 *          indicates, that it needs some margins at the borders of the canvas.
 *          This is e.g. used by bar charts to reserve space for displaying the bars.
 *          The margins are in target device coordinates (pixels on screen).
 *          The default implementation returns 0 for all margins.
 * @sa QwtPlot::getCanvasMarginsHint(), QwtPlot::updateCanvasMargins()
 */
void QwtPlotItem::getCanvasMarginHint( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& canvasRect,
    double& left, double& top, double& right, double& bottom ) const
{
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );
    Q_UNUSED( canvasRect );

    // use QMargins, when we don't need to support Qt < 4.6 anymore
    left = top = right = bottom = 0.0;
}

/**
 * @brief Return all information needed to represent the item on the legend
 * @return Data that is needed to represent the item on the legend
 * @details Most items are represented by one entry on the legend showing an icon
 *          and a text, but e.g. QwtPlotMultiBarChart displays one entry for each bar.
 *          QwtLegendData is basically a list of QVariants that makes it possible
 *          to overload and reimplement legendData() to return almost any type of
 *          information, that is understood by the receiver that acts as the legend.
 *          The default implementation returns one entry with the title() of the item
 *          and the legendIcon().
 * @sa title(), legendIcon(), QwtLegend, QwtPlotLegendItem
 */
QList< QwtLegendData > QwtPlotItem::legendData() const
{
    QwtLegendData data;

    QwtText label = title();
    label.setRenderFlags( label.renderFlags() & Qt::AlignLeft );

    data.setValue( QwtLegendData::TitleRole,
        QVariant::fromValue( label ) );

    const QwtGraphic graphic = legendIcon( 0, legendIconSize() );
    if ( !graphic.isNull() )
    {
        data.setValue( QwtLegendData::IconRole,
            QVariant::fromValue( graphic ) );
    }

    QList< QwtLegendData > list;
    list += data;

    return list;
}

/**
 * @brief Update the item to changes of the axes scale division
 * @param[in] xScaleDiv Scale division of the x-axis
 * @param[in] yScaleDiv Scale division of the y-axis
 * @details Update the item, when the axes of plot have changed.
 *          The default implementation does nothing, but items that depend
 *          on the scale division (like QwtPlotGrid()) have to reimplement updateScaleDiv().
 *          updateScaleDiv() is only called when the ScaleInterest interest is enabled.
 *          The default implementation does nothing.
 * @sa QwtPlot::updateAxes(), ScaleInterest
 */
void QwtPlotItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    Q_UNUSED( xScaleDiv );
    Q_UNUSED( yScaleDiv );
}

/**
 * @brief Update the item to changes of the legend info
 * @param[in] item Plot item to be displayed on a legend
 * @param[in] data Attributes how to display item on the legend
 * @details Plot items that want to display a legend (not those, that want to
 *          be displayed on a legend!) will have to implement updateLegend().
 *          updateLegend() is only called when the LegendInterest interest is enabled.
 *          The default implementation does nothing.
 * @note Plot items, that want to be displayed on a legend need to enable
 *       the QwtPlotItem::Legend flag and to implement legendData() and legendIcon().
 * @sa QwtPlotLegendItem
 */
void QwtPlotItem::updateLegend( const QwtPlotItem* item,
    const QList< QwtLegendData >& data )
{
    Q_UNUSED( item );
    Q_UNUSED( data );
}

/**
 * @brief Calculate the bounding scale rectangle of 2 maps
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @return Bounding scale rect of the scale maps, not normalized
 */
QRectF QwtPlotItem::scaleRect( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap ) const
{
    return QRectF( xMap.s1(), yMap.s1(),
        xMap.sDist(), yMap.sDist() );
}

/**
 * @brief Calculate the bounding paint rectangle of 2 maps
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @return Bounding paint rectangle of the scale maps, not normalized
 */
QRectF QwtPlotItem::paintRect( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap ) const
{
    const QRectF rect( xMap.p1(), yMap.p1(),
        xMap.pDist(), yMap.pDist() );

    return rect;
}
