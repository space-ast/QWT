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
  public:
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
 * \if ENGLISH
 * @brief Constructor
 * @details Auto deletion is enabled.
 * @sa setAutoDelete, attachItem
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 自动删除已启用。
 * @sa setAutoDelete, attachItem
 * \endif
 */
QwtPolarItemDict::QwtPolarItemDict()
{
    m_data = new QwtPolarItemDict::PrivateData;
    m_data->autoDelete = true;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * @details If autoDelete is on, all attached items will be deleted.
 * @sa setAutoDelete, autoDelete, attachItem
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * @details 如果 autoDelete 开启，所有附加的项将被删除。
 * @sa setAutoDelete, autoDelete, attachItem
 * \endif
 */
QwtPolarItemDict::~QwtPolarItemDict()
{
    detachItems( QwtPolarItem::Rtti_PolarItem, m_data->autoDelete );
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Enable/Disable auto deletion
 * @details If Auto deletion is on, all attached plot items will be deleted
 *          in the destructor of QwtPolarItemDict. The default value is on.
 * @param[in] autoDelete Auto delete flag
 * @sa autoDelete, attachItem
 * \endif
 *
 * \if CHINESE
 * @brief 启用/禁用自动删除
 * @details 如果自动删除开启，所有附加的绘图项将在 QwtPolarItemDict 的析构函数中被删除。
 *          默认值为开启。
 * @param[in] autoDelete 自动删除标志
 * @sa autoDelete, attachItem
 * \endif
 */
void QwtPolarItemDict::setAutoDelete( bool autoDelete )
{
    m_data->autoDelete = autoDelete;
}

/**
 * \if ENGLISH
 * @brief Check if auto deletion is enabled
 * @return true if auto deletion is enabled
 * @sa setAutoDelete, attachItem
 * \endif
 *
 * \if CHINESE
 * @brief 检查是否启用了自动删除
 * @return 如果启用了自动删除则返回 true
 * @sa setAutoDelete, attachItem
 * \endif
 */
bool QwtPolarItemDict::autoDelete() const
{
    return m_data->autoDelete;
}

/**
 * \if ENGLISH
 * @brief Insert a plot item
 * @param[in] item PlotItem
 * @sa removeItem()
 * \endif
 *
 * \if CHINESE
 * @brief 插入绘图项
 * @param[in] item 绘图项
 * @sa removeItem()
 * \endif
 */
void QwtPolarItemDict::insertItem( QwtPolarItem* item )
{
    m_data->itemList.insertItem( item );
}

/**
 * \if ENGLISH
 * @brief Remove a plot item
 * @param[in] item PlotItem
 * @sa insertItem()
 * \endif
 *
 * \if CHINESE
 * @brief 移除绘图项
 * @param[in] item 绘图项
 * @sa insertItem()
 * \endif
 */
void QwtPolarItemDict::removeItem( QwtPolarItem* item )
{
    m_data->itemList.removeItem( item );
}

/**
 * \if ENGLISH
 * @brief Detach items from the dictionary
 * @param[in] rtti In case of QwtPolarItem::Rtti_PolarItem detach all items,
 *                 otherwise only those items of the type rtti.
 * @param[in] autoDelete If true, delete all detached items
 * \endif
 *
 * \if CHINESE
 * @brief 从字典分离项
 * @param[in] rtti 如果是 QwtPolarItem::Rtti_PolarItem，则分离所有项，
 *                 否则只分离类型为 rtti 的项。
 * @param[in] autoDelete 如果为 true，删除所有分离的项
 * \endif
 */
void QwtPolarItemDict::detachItems( int rtti, bool autoDelete )
{
    PrivateData::ItemList list = m_data->itemList;
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
 * \if ENGLISH
 * @brief Get a QwtPolarItemList of all attached plot items
 * @return List of all attached plot items
 * @note Use caution when iterating these lists, as removing/detaching an item will invalidate the iterator.
 *       Instead you can place pointers to objects to be removed in a removal list, and traverse that list later.
 * \endif
 *
 * \if CHINESE
 * @brief 获取所有附加绘图项的 QwtPolarItemList
 * @return 所有附加绘图项的列表
 * @note 迭代这些列表时要小心，因为移除/分离项会使迭代器失效。
 *       您可以将要移除的对象指针放入移除列表中，稍后遍历该列表。
 * \endif
 */
const QwtPolarItemList& QwtPolarItemDict::itemList() const
{
    return m_data->itemList;
}
