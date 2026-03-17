/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_ITEMDICT_H
#define QWT_POLAR_ITEMDICT_H

#include "qwt_global.h"
#include "qwt_polar_item.h"
#include <qlist.h>

using QwtPolarItemIterator = QList< QwtPolarItem* >::ConstIterator;
using QwtPolarItemList     = QList< QwtPolarItem* >;

/**
 * \if ENGLISH
 * @brief A dictionary for polar plot items
 * @details QwtPolarItemDict organizes polar plot items in increasing z-order.
 *          If autoDelete() is enabled, all attached items will be deleted
 *          in the destructor of the dictionary.
 * 
 * @sa QwtPolarItem::attach(), QwtPolarItem::detach(), QwtPolarItem::z()
 * \endif
 * 
 * \if CHINESE
 * @brief 极坐标绘图项的字典
 * @details QwtPolarItemDict 按递增的 z 顺序组织极坐标绘图项。
 *          如果启用了 autoDelete()，所有附加的项将在字典的析构函数中被删除。
 * 
 * @sa QwtPolarItem::attach(), QwtPolarItem::detach(), QwtPolarItem::z()
 * \endif
 */
class QWT_EXPORT QwtPolarItemDict
{
public:
    /// Constructor
    explicit QwtPolarItemDict();
    /// Destructor
    ~QwtPolarItemDict();

    /// Set auto delete
    void setAutoDelete(bool);
    /// Get auto delete
    bool autoDelete() const;

    /// Get the item list
    const QwtPolarItemList& itemList() const;

    /// Detach items
    void detachItems(int rtti = QwtPolarItem::Rtti_PolarItem, bool autoDelete = true);

protected:
    /// Insert an item
    void insertItem(QwtPolarItem*);
    /// Remove an item
    void removeItem(QwtPolarItem*);

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
