/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_itemdict.h"

class QwtPolarItemDict::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPolarItemDict)
  public:
    PrivateData(QwtPolarItemDict* p) : q_ptr(p)
    {
    }

    class ItemList : public QList< QwtPolarItem* >
    {
      public:
        void insertItem( QwtPolarItem* item )
        {
            if ( item == nullptr )
                return;

            // Unfortunately there is no inSort operation
            // for lists in Qt4. The implementation below
            // is slow, but there shouldn't be many plot items.

            QList< QwtPolarItem* >::Iterator it;
            for ( it = begin(); it != end(); ++it )
            {
                if ( *it == item )
                    return;

                if ( ( *it )->z() > item->z() )
                {
                    insert( it, item );
                    return;
                }
            }
            append( item );
        }

        void removeItem( QwtPolarItem* item )
        {
            if ( item == nullptr )
                return;

            int i = 0;

            QList< QwtPolarItem* >::Iterator it;
            for ( it = begin(); it != end(); ++it )
            {
                if ( item == *it )
                {
                    removeAt( i );
                    return;
                }
                i++;
            }
        }
    };

    ItemList itemList;
    bool autoDelete;
};

/**
 * @brief Constructor
 * @details Auto deletion is enabled.
 * @sa setAutoDelete, attachItem
 */
QwtPolarItemDict::QwtPolarItemDict()
{
    QWT_PIMPL_CONSTRUCT_INIT();
    QWT_D(d);
    d->autoDelete = true;
}

/**
 * @brief Destructor
 * @details If autoDelete is on, all attached items will be deleted.
 * @sa setAutoDelete, autoDelete, attachItem
 */
QwtPolarItemDict::~QwtPolarItemDict()
{
    QWT_D(d);
    detachItems( QwtPolarItem::Rtti_PolarItem, d->autoDelete );
}

/**
 * @brief Enable/Disable auto deletion
 * @details If Auto deletion is on, all attached plot items will be deleted
 *          in the destructor of QwtPolarItemDict. The default value is on.
 * @param autoDelete Auto delete flag
 * @sa autoDelete, attachItem
 */
void QwtPolarItemDict::setAutoDelete( bool autoDelete )
{
    QWT_D(d);
    d->autoDelete = autoDelete;
}

/**
 * @brief Check if auto deletion is enabled
 * @return true if auto deletion is enabled
 * @sa setAutoDelete, attachItem
 */
bool QwtPolarItemDict::autoDelete() const
{
    QWT_DC(d);
    return d->autoDelete;
}

/**
 * @brief Insert a plot item
 * @param item PlotItem
 * @sa removeItem()
 */
void QwtPolarItemDict::insertItem( QwtPolarItem* item )
{
    QWT_D(d);
    d->itemList.insertItem( item );
}

/**
 * @brief Remove a plot item
 * @param item PlotItem
 * @sa insertItem()
 */
void QwtPolarItemDict::removeItem( QwtPolarItem* item )
{
    QWT_D(d);
    d->itemList.removeItem( item );
}

/**
 * @brief Detach items from the dictionary
 * @param rtti In case of QwtPolarItem::Rtti_PolarItem detach all items,
 *             otherwise only those items of the type rtti.
 * @param autoDelete If true, delete all detached items
 */
void QwtPolarItemDict::detachItems( int rtti, bool autoDelete )
{
    QWT_D(d);
    PrivateData::ItemList list = d->itemList;
    QwtPolarItemIterator it = list.constBegin();
    while ( it != list.constEnd() )
    {
        QwtPolarItem* item = *it;

        ++it; // increment before removing item from the list

        if ( rtti == QwtPolarItem::Rtti_PolarItem || item->rtti() == rtti )
        {
            item->attach( nullptr );
            if ( autoDelete )
                delete item;
        }
    }
}

/**
 * @brief Get a QwtPolarItemList of all attached plot items
 * @return List of all attached plot items
 * @note Use caution when iterating these lists, as removing/detaching an item will invalidate the iterator.
 *       Instead you can place pointers to objects to be removed in a removal list, and traverse that list later.
 */
const QwtPolarItemList& QwtPolarItemDict::itemList() const
{
    QWT_DC(d);
    return d->itemList;
}
