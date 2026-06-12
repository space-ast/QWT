/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_polar_item.h"
#include "qwt_legend.h"
#include "qwt_scale_div.h"

#include <qpainter.h>

class QwtPolarItem::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPolarItem)
  public:
    PrivateData(QwtPolarItem* p)
        : q_ptr(p)
        , plot( nullptr )
        , isVisible( true )
        , renderThreadCount( 1 )
        , z( 0.0 )
        , legendIconSize( 8, 8 )
    {
    }

    mutable QwtPolarPlot* plot;

    bool isVisible;
    QwtPolarItem::ItemAttributes attributes;
    QwtPolarItem::RenderHints renderHints;
    uint renderThreadCount;

    double z;

    QwtText title;
    QSize legendIconSize;
};

/**
 * @brief Constructor
 * @param title Item title, f.e used on a legend
 * @sa setTitle()
 */
QwtPolarItem::QwtPolarItem( const QwtText& title )
{
    QWT_PIMPL_CONSTRUCT_INIT();
    QWT_D(d);
    d->title = title;
}

/**
 * @brief Destructor
 */
QwtPolarItem::~QwtPolarItem()
{
    attach( nullptr );
}

/**
 * @brief Attach the item to a plot
 * @details This method will attach a QwtPolarItem to the QwtPolarPlot argument.
 *          It will first detach the QwtPolarItem from any plot from a previous
 *          call to attach (if necessary). If a nullptr argument is passed,
 *          it will detach from any QwtPolarPlot it was attached to.
 * @param plot Plot widget
 * @sa QwtPolarItem::detach()
 */
void QwtPolarItem::attach( QwtPolarPlot* plot )
{
    QWT_D(d);

    if ( plot == d->plot )
        return;

    if ( d->plot )
        d->plot->attachItem( this, false );

    d->plot = plot;

    if ( d->plot )
        d->plot->attachItem( this, true );
}

/**
 * @brief Detach the item from its associated plot
 * @details This method detaches a QwtPolarItem from the QwtPolarPlot it has been associated with.
 *          detach() is equivalent to calling attach( nullptr ).
 * @sa attach()
 */
void QwtPolarItem::detach()
{
    attach( nullptr );
}

/**
 * @brief Get runtime type information for the specific class represented
 * @details QwtPolarItem is simply a virtual interface class, and base classes will implement this method
 *          with specific rtti values so a user can differentiate them. The rtti value is useful for
 *          environments where the runtime type information is disabled and it is not possible
 *          to do a dynamic_cast<...>.
 * @return rtti value
 * @sa RttiValues
 */
int QwtPolarItem::rtti() const
{
    return Rtti_PolarItem;
}

/**
 * @brief Get the attached plot
 * @return Attached plot
 */
QwtPolarPlot* QwtPolarItem::plot() const
{
    QWT_DC(d);
    return d->plot;
}

/**
 * @brief Get the z value
 * @details Plot items are painted in increasing z-order.
 * @return Z value
 * @sa setZ(), QwtPolarItemDict::itemList()
 */
double QwtPolarItem::z() const
{
    QWT_DC(d);
    return d->z;
}

/**
 * @brief Set the z value
 * @details Plot items are painted in increasing z-order.
 * @param z Z-value
 * @sa z(), QwtPolarItemDict::itemList()
 */
void QwtPolarItem::setZ( double z )
{
    QWT_D(d);

    if ( d->z != z )
    {
        if ( d->plot )
            d->plot->attachItem( this, false );

        d->z = z;

        if ( d->plot )
            d->plot->attachItem( this, true );

        itemChanged();
    }
}

/**
 * @brief Set a new title
 * @param title Title
 * @sa title()
 */
void QwtPolarItem::setTitle( const QString& title )
{
    setTitle( QwtText( title ) );
}

/**
 * @brief Set a new title
 * @param title Title
 * @sa title()
 */
void QwtPolarItem::setTitle( const QwtText& title )
{
    QWT_D(d);

    if ( d->title != title )
    {
        d->title = title;
        itemChanged();
    }
}

/**
 * @brief Get the title of the item
 * @return Title of the item
 * @sa setTitle()
 */
const QwtText& QwtPolarItem::title() const
{
    QWT_DC(d);
    return d->title;
}

/**
 * @brief Toggle an item attribute
 * @param attribute Attribute type
 * @param on true/false
 * @sa testItemAttribute(), ItemAttribute
 */
void QwtPolarItem::setItemAttribute( ItemAttribute attribute, bool on )
{
    QWT_D(d);

    if ( bool( d->attributes & attribute ) != on )
    {
        if ( on )
            d->attributes |= attribute;
        else
            d->attributes &= ~attribute;

        itemChanged();
    }
}

/**
 * @brief Test an item attribute
 * @param attribute Attribute type
 * @return true/false
 * @sa setItemAttribute(), ItemAttribute
 */
bool QwtPolarItem::testItemAttribute( ItemAttribute attribute ) const
{
    QWT_DC(d);
    return d->attributes & attribute;
}

/**
 * @brief Toggle a render hint
 * @param hint Render hint
 * @param on true/false
 * @sa testRenderHint(), RenderHint
 */
void QwtPolarItem::setRenderHint( RenderHint hint, bool on )
{
    QWT_D(d);

    if ( ( ( d->renderHints & hint ) != 0 ) != on )
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
 * @param hint Render hint
 * @return true/false
 * @sa setRenderHint(), RenderHint
 */
bool QwtPolarItem::testRenderHint( RenderHint hint ) const
{
    QWT_DC(d);
    return ( d->renderHints & hint );
}

/**
 * @brief Set the number of render threads
 * @details On multi core systems rendering of certain plot item (f.e QwtPolarSpectrogram)
 *          can be done in parallel in several threads. The default setting is set to 1.
 * @param numThreads Number of threads to be used for rendering.
 *                   If numThreads is set to 0, the system specific ideal thread count is used.
 *                   The default thread count is 1 ( = no additional threads ).
 */
void QwtPolarItem::setRenderThreadCount( uint numThreads )
{
    QWT_D(d);
    d->renderThreadCount = numThreads;
}

/**
 * @brief Get the number of render threads
 * @return Number of threads to be used for rendering.
 *         If numThreads() is set to 0, the system specific ideal thread count is used.
 */
uint QwtPolarItem::renderThreadCount() const
{
    QWT_DC(d);
    return d->renderThreadCount;
}

/**
 * @brief Set the size of the legend icon
 * @details The default setting is 8x8 pixels.
 * @param size Size
 * @sa legendIconSize(), legendIcon()
 */
void QwtPolarItem::setLegendIconSize( const QSize& size )
{
    QWT_D(d);

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
QSize QwtPolarItem::legendIconSize() const
{
    QWT_DC(d);
    return d->legendIconSize;
}

//! Show the item
void QwtPolarItem::show()
{
    setVisible( true );
}

//! Hide the item
void QwtPolarItem::hide()
{
    setVisible( false );
}

/**
 * @brief Show/Hide the item
 * @param on Show if true, otherwise hide
 * @sa isVisible(), show(), hide()
 */
void QwtPolarItem::setVisible( bool on )
{
    QWT_D(d);

    if ( on != d->isVisible )
    {
        d->isVisible = on;
        itemChanged();
    }
}

/**
 * @brief Check if the item is visible
 * @return true if visible
 * @sa setVisible(), show(), hide()
 */
bool QwtPolarItem::isVisible() const
{
    QWT_DC(d);
    return d->isVisible;
}

/**
 * @brief Update the legend and call QwtPolarPlot::autoRefresh for the parent plot
 * @sa updateLegend()
 */
void QwtPolarItem::itemChanged()
{
    QWT_D(d);
    if ( d->plot )
        d->plot->autoRefresh();
}

/**
 * @brief Update the legend of the parent plot
 * @sa QwtPolarPlot::updateLegend(), itemChanged()
 */
void QwtPolarItem::legendChanged()
{
    QWT_D(d);
    if ( testItemAttribute( QwtPolarItem::Legend ) && d->plot )
        d->plot->updateLegend( this );
}

/**
 * @brief Get the bounding interval for a scale
 * @details Interval, that is necessary to display the item.
 *          This interval can be useful for operations like clipping or autoscaling.
 *          For items (like the grid), where a bounding interval makes no sense, an invalid interval is returned.
 * @param scaleId Scale id ( QwtPolar::Scale )
 * @return Bounding interval of the plot item for a specific scale
 */
QwtInterval QwtPolarItem::boundingInterval( int scaleId ) const
{
    Q_UNUSED( scaleId );

    return QwtInterval(); // invalid
}

/**
 * @brief Update the item to changes of the axes scale division
 * @details Update the item, when the axes of plot have changed.
 *          The default implementation does nothing, but items that depend on the scale division
 *          (like QwtPolarGrid()) have to reimplement updateScaleDiv().
 * @param azimuthScaleDiv Scale division of the azimuth-scale
 * @param radialScaleDiv Scale division of the radius-axis
 * @param interval The interval of the radius-axis, that is visible on the canvas
 * @sa QwtPolarPlot::updateAxes()
 */
void QwtPolarItem::updateScaleDiv( const QwtScaleDiv& azimuthScaleDiv,
    const QwtScaleDiv& radialScaleDiv, const QwtInterval& interval )
{
    Q_UNUSED( azimuthScaleDiv );
    Q_UNUSED( radialScaleDiv );
    Q_UNUSED( interval );
}

/**
 * @brief Return all information that is needed to represent the item on the legend
 * @details Most items are represented by one entry on the legend showing an icon and a text.
 *          QwtLegendData is basically a list of QVariants that makes it possible to overload
 *          and reimplement legendData() to return almost any type of information, that is
 *          understood by the receiver that acts as the legend.
 *          The default implementation returns one entry with the title() of the item and the legendIcon().
 * @return List of legend data
 * @sa title(), legendIcon(), QwtLegend
 */
QList< QwtLegendData > QwtPolarItem::legendData() const
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
 * @brief Get icon representing the item on the legend
 * @details The default implementation returns an invalid icon.
 * @param index Index of the legend entry (usually there is only one)
 * @param size Icon size
 * @return Icon representing the item on the legend
 * @sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPolarItem::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index )
    Q_UNUSED( size )

    return QwtGraphic();
}

/**
 * @brief Get the margin hint
 * @details Some items like to display something (f.e. the azimuth axis) outside of the area
 *          of the interval of the radial scale. The default implementation returns 0 pixels.
 * @return Hint for the margin
 */
int QwtPolarItem::marginHint() const
{
    return 0;
}
