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
  public:

    class ItemList : public QList< QwtPlotItem* >
    {
      public:
        void insertItem( QwtPlotItem* item )
        {
            if ( item == nullptr )
                return;

            QList< QwtPlotItem* >::iterator it =
                std::upper_bound( begin(), end(), item, LessZThan() );
            insert( it, item );
        }

        void removeItem( QwtPlotItem* item )
        {
            if ( item == nullptr )
                return;

            QList< QwtPlotItem* >::iterator it =
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
 * \if ENGLISH
 * @brief Constructor
 * @details Auto deletion is enabled.
 * @sa setAutoDelete(), QwtPlotItem::attach()
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @details 自动删除已启用。
 * @sa setAutoDelete(), QwtPlotItem::attach()
 * \endif
 */
QwtPlotDict::QwtPlotDict()
{
    m_data = new QwtPlotDict::PrivateData;
    m_data->autoDelete = true;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * @details If autoDelete() is on, all attached items will be deleted.
 * @sa setAutoDelete(), autoDelete(), QwtPlotItem::attach()
 * \endif
 * 
 * \if CHINESE
 * @brief 析构函数
 * @details 如果 autoDelete() 开启，所有附加的项目将被删除。
 * @sa setAutoDelete(), autoDelete(), QwtPlotItem::attach()
 * \endif
 */
QwtPlotDict::~QwtPlotDict()
{
    detachItems( QwtPlotItem::Rtti_PlotItem, m_data->autoDelete );
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Enable/disable auto deletion
 * @details If Auto deletion is on all attached plot items will be deleted
 *          in the destructor of QwtPlotDict. The default value is on.
 * @param[in] autoDelete True to enable auto deletion
 * @sa autoDelete(), insertItem()
 * \endif
 * 
 * \if CHINESE
 * @brief 启用/禁用自动删除
 * @details 如果自动删除开启，所有附加的绘图项将在 QwtPlotDict 的析构函数中被删除。
 *          默认值为开启。
 * @param[in] autoDelete true 启用自动删除
 * @sa autoDelete(), insertItem()
 * \endif
 */
void QwtPlotDict::setAutoDelete( bool autoDelete )
{
    m_data->autoDelete = autoDelete;
}

/**
 * \if ENGLISH
 * @brief Check if auto deletion is enabled
 * @return True if auto deletion is enabled
 * @sa setAutoDelete(), insertItem()
 * \endif
 * 
 * \if CHINESE
 * @brief 检查自动删除是否启用
 * @return 如果自动删除启用则返回 true
 * @sa setAutoDelete(), insertItem()
 * \endif
 */
bool QwtPlotDict::autoDelete() const
{
    return m_data->autoDelete;
}

/*!
   Insert a plot item

   \param item PlotItem
   \sa removeItem()
 */
void QwtPlotDict::insertItem( QwtPlotItem* item )
{
    m_data->itemList.insertItem( item );
}

/*!
   Remove a plot item

   \param item PlotItem
   \sa insertItem()
 */
void QwtPlotDict::removeItem( QwtPlotItem* item )
{
    m_data->itemList.removeItem( item );
}

/**
 * \if ENGLISH
 * @brief Detach items from the dictionary
 * @param[in] rtti In case of QwtPlotItem::Rtti_PlotItem detach all items,
 *                 otherwise only those items of the type rtti.
 * @param[in] autoDelete If true, delete all detached items
 * \endif
 * 
 * \if CHINESE
 * @brief 从字典中分离项目
 * @param[in] rtti 如果是 QwtPlotItem::Rtti_PlotItem 则分离所有项目，
 *                 否则只分离指定类型的项目。
 * @param[in] autoDelete 如果为 true，删除所有分离的项目
 * \endif
 */
void QwtPlotDict::detachItems( int rtti, bool autoDelete )
{
    PrivateData::ItemList list = m_data->itemList;
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
 * \if ENGLISH
 * @brief Get the list of all attached plot items
 * @details Use caution when iterating these lists, as removing/detaching an item will
 *          invalidate the iterator. Instead you can place pointers to objects to be
 *          removed in a removal list, and traverse that list later.
 * @return List of all attached plot items
 * \endif
 * 
 * \if CHINESE
 * @brief 获取所有附加绘图项的列表
 * @details 在迭代这些列表时请谨慎，因为移除/分离项目会使迭代器失效。
 *          相反，您可以将要移除的对象指针放在移除列表中，然后遍历该列表。
 * @return 所有附加绘图项的列表
 * \endif
 */
const QwtPlotItemList& QwtPlotDict::itemList() const
{
    return m_data->itemList;
}

/**
 * \if ENGLISH
 * @brief Get the list of all attached plot items of a specific type
 * @param[in] rtti See QwtPlotItem::RttiValues
 * @return List of all attached plot items of a specific type
 * @sa QwtPlotItem::rtti()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取特定类型的所有附加绘图项的列表
 * @param[in] rtti 参见 QwtPlotItem::RttiValues
 * @return 特定类型的所有附加绘图项的列表
 * @sa QwtPlotItem::rtti()
 * \endif
 */
QwtPlotItemList QwtPlotDict::itemList( int rtti ) const
{
    if ( rtti == QwtPlotItem::Rtti_PlotItem )
        return m_data->itemList;

    QwtPlotItemList items;

    PrivateData::ItemList list = m_data->itemList;
    for ( QwtPlotItemIterator it = list.constBegin(); it != list.constEnd(); ++it )
    {
        QwtPlotItem* item = *it;
        if ( item->rtti() == rtti )
            items += item;
    }

    return items;
}
