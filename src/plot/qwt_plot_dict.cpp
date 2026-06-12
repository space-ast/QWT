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

#include "qwt_plot_dict.h"
#include <algorithm>

class QwtPlotDict::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotDict)
  public:
    PrivateData(QwtPlotDict* p)
        : q_ptr(p)
        , autoDelete(true)
    {
    }

    class ItemList : public QList< QwtPlotItem* >
    {
      public:
        void insertItem( QwtPlotItem* item )
        {
            if ( item == nullptr )
                return;

            auto it =
                std::upper_bound( begin(), end(), item, LessZThan() );
            insert( it, item );
        }

        void removeItem( QwtPlotItem* item )
        {
            if ( item == nullptr )
                return;

            auto it =
                std::lower_bound( begin(), end(), item, LessZThan() );

            for ( ; it != end(); ++it )
            {
                if ( item == *it )
                {
                    erase( it );
                    break;
                }
            }
        }
      private:
        class LessZThan
        {
          public:
            inline bool operator()( const QwtPlotItem* item1,
                const QwtPlotItem* item2 ) const
            {
                return item1->z() < item2->z();
            }
        };
    };

    ItemList itemList;
    bool autoDelete;
};

/**
 * @brief Constructor
 * @details Auto deletion is enabled.
 * @sa setAutoDelete(), QwtPlotItem::attach()
 */
QwtPlotDict::QwtPlotDict()
    : QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Destructor
 * @details If autoDelete() is on, all attached items will be deleted.
 * @sa setAutoDelete(), autoDelete(), QwtPlotItem::attach()
 */
QwtPlotDict::~QwtPlotDict()
{
    QWT_D(d);
    detachItems( QwtPlotItem::Rtti_PlotItem, d->autoDelete );
}

/**
 * @brief Enable/disable auto deletion
 * @details If Auto deletion is on all attached plot items will be deleted
 *          in the destructor of QwtPlotDict. The default value is on.
 * @param[in] autoDelete True to enable auto deletion
 * @sa autoDelete(), insertItem()
 */
void QwtPlotDict::setAutoDelete( bool autoDelete )
{
    QWT_D(d);
    d->autoDelete = autoDelete;
}

/**
 * @brief Check if auto deletion is enabled
 * @return True if auto deletion is enabled
 * @sa setAutoDelete(), insertItem()
 */
bool QwtPlotDict::autoDelete() const
{
    QWT_DC(d);
    return d->autoDelete;
}

/*!
   Insert a plot item

   @param item PlotItem
   @sa removeItem()
 */
void QwtPlotDict::insertItem( QwtPlotItem* item )
{
    QWT_D(d);
    d->itemList.insertItem( item );
}

/*!
   Remove a plot item

   @param item PlotItem
   @sa insertItem()
 */
void QwtPlotDict::removeItem( QwtPlotItem* item )
{
    QWT_D(d);
    d->itemList.removeItem( item );
}

/**
 * @brief Detach items from the dictionary
 * @param[in] rtti In case of QwtPlotItem::Rtti_PlotItem detach all items,
 *                 otherwise only those items of the type rtti.
 * @param[in] autoDelete If true, delete all detached items
 */
void QwtPlotDict::detachItems( int rtti, bool autoDelete )
{
    QWT_D(d);
    PrivateData::ItemList list = d->itemList;
    QwtPlotItemIterator it = list.constBegin();
    while ( it != list.constEnd() )
    {
        QwtPlotItem* item = *it;

        ++it; // increment before removing item from the list

        if ( rtti == QwtPlotItem::Rtti_PlotItem || item->rtti() == rtti )
        {
            item->attach( nullptr );
            if ( autoDelete )
                delete item;
        }
    }
}

/**
 * @brief Get the list of all attached plot items
 * @details Use caution when iterating these lists, as removing/detaching an item will
 *          invalidate the iterator. Instead you can place pointers to objects to be
 *          removed in a removal list, and traverse that list later.
 * @return List of all attached plot items
 */
const QwtPlotItemList& QwtPlotDict::itemList() const
{
    QWT_DC(d);
    return d->itemList;
}

/**
 * @brief Get the list of all attached plot items of a specific type
 * @param[in] rtti See QwtPlotItem::RttiValues
 * @return List of all attached plot items of a specific type
 * @sa QwtPlotItem::rtti()
 */
QwtPlotItemList QwtPlotDict::itemList( int rtti ) const
{
    QWT_DC(d);
    if ( rtti == QwtPlotItem::Rtti_PlotItem )
        return d->itemList;

    QwtPlotItemList items;

    PrivateData::ItemList list = d->itemList;
    for ( QwtPlotItemIterator it = list.constBegin(); it != list.constEnd(); ++it )
    {
        QwtPlotItem* item = *it;
        if ( item->rtti() == rtti )
            items += item;
    }

    return items;
}
