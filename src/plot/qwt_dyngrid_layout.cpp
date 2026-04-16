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

#include "qwt_dyngrid_layout.h"

#include <qvector.h>
#include <qlist.h>

class QwtDynGridLayout::PrivateData
{
public:
    PrivateData() : isDirty(true)
    {
    }

    void updateLayoutCache();

    mutable QList< QLayoutItem* > itemList;

    uint maxColumns;
    uint numRows;
    uint numColumns;

    Qt::Orientations expanding;

    bool isDirty;
    QVector< QSize > itemSizeHints;
};

void QwtDynGridLayout::PrivateData::updateLayoutCache()
{
    itemSizeHints.resize(itemList.count());

    int index = 0;

    for (QList< QLayoutItem* >::const_iterator it = itemList.constBegin(); it != itemList.constEnd(); ++it, index++) {
        itemSizeHints[ index ] = (*it)->sizeHint();
    }

    isDirty = false;
}

/**
 * \if ENGLISH
 * @brief Constructor with parent widget, margin and spacing
 *
 * @param[in] parent Parent widget
 * @param[in] margin Margin around the layout
 * @param[in] spacing Spacing between items
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数，带父控件、边距和间距
 *
 * @param[in] parent 父控件
 * @param[in] margin 布局边距
 * @param[in] spacing 项目之间的间距
 * \endif
 */
QwtDynGridLayout::QwtDynGridLayout(QWidget* parent, int margin, int spacing) : QLayout(parent)
{
    init();

    setSpacing(spacing);
    setContentsMargins(margin, margin, margin, margin);
}

/**
 * \if ENGLISH
 * @brief Constructor with spacing only
 *
 * @param[in] spacing Spacing between items
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数，仅带间距
 *
 * @param[in] spacing 项目之间的间距
 * \endif
 */
QwtDynGridLayout::QwtDynGridLayout(int spacing)
{
    init();
    setSpacing(spacing);
}

/*!
   Initialize the layout with default values.
 */
void QwtDynGridLayout::init()
{
    m_data             = new QwtDynGridLayout::PrivateData;
    m_data->maxColumns = m_data->numRows = m_data->numColumns = 0;
}

/**
 * \if ENGLISH
 * @brief Destructor
 *
 * @details Deletes all layout items and private data.
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 *
 * @details 删除所有布局项目和私有数据。
 * \endif
 */
QwtDynGridLayout::~QwtDynGridLayout()
{
    qDeleteAll(m_data->itemList);
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Invalidate all internal caches
 *
 * @details Marks the layout as dirty so it will recalculate on next layout operation.
 * \endif
 *
 * \if CHINESE
 * @brief 使所有内部缓存失效
 *
 * @details 将布局标记为脏，以便在下次布局操作时重新计算。
 * \endif
 */
void QwtDynGridLayout::invalidate()
{
    m_data->isDirty = true;
    QLayout::invalidate();
}

/**
 * \if ENGLISH
 * @brief Set the upper limit for the number of columns
 *
 * @param[in] maxColumns Upper limit, 0 means unlimited
 * \endif
 *
 * \if CHINESE
 * @brief 设置列数的上限
 *
 * @param[in] maxColumns 上限，0 表示无限制
 * \endif
 */
void QwtDynGridLayout::setMaxColumns(uint maxColumns)
{
    m_data->maxColumns = maxColumns;
}

/**
 * \if ENGLISH
 * @brief Get the upper limit for the number of columns
 *
 * @return Upper limit for the number of columns, 0 means unlimited
 * \endif
 *
 * \if CHINESE
 * @brief 获取列数的上限
 *
 * @return 列数的上限，0 表示无限制
 * \endif
 */
uint QwtDynGridLayout::maxColumns() const
{
    return m_data->maxColumns;
}

/**
 * \if ENGLISH
 * @brief Add an item to the next free position
 *
 * @param[in] item Layout item to add
 * \endif
 *
 * \if CHINESE
 * @brief 将项目添加到下一个空闲位置
 *
 * @param[in] item 要添加的布局项目
 * \endif
 */
void QwtDynGridLayout::addItem(QLayoutItem* item)
{
    m_data->itemList.append(item);
    invalidate();
}

/**
 * \if ENGLISH
 * @brief Check if the layout is empty
 *
 * @return true if the layout contains no items
 * \endif
 *
 * \if CHINESE
 * @brief 检查布局是否为空
 *
 * @return 如果布局不包含任何项目则返回 true
 * \endif
 */
bool QwtDynGridLayout::isEmpty() const
{
    return m_data->itemList.isEmpty();
}

/**
 * \if ENGLISH
 * @brief Get the number of layout items
 *
 * @return Number of items in the layout
 * \endif
 *
 * \if CHINESE
 * @brief 获取布局项目的数量
 *
 * @return 布局中的项目数量
 * \endif
 */
uint QwtDynGridLayout::itemCount() const
{
    return m_data->itemList.count();
}

/**
 * \if ENGLISH
 * @brief Get the item at a specific index
 *
 * @param[in] index Index of the item
 * @return Item at the specified index, or nullptr if index is out of range
 * \endif
 *
 * \if CHINESE
 * @brief 获取指定索引处的项目
 *
 * @param[in] index 项目的索引
 * @return 指定索引处的项目，如果索引超出范围则返回 nullptr
 * \endif
 */
QLayoutItem* QwtDynGridLayout::itemAt(int index) const
{
    if (index < 0 || index >= m_data->itemList.count())
        return nullptr;

    return m_data->itemList.at(index);
}

/**
 * \if ENGLISH
 * @brief Remove and return the item at a specific index
 *
 * @param[in] index Index of the item to remove
 * @return Removed item, or nullptr if index is out of range
 * \endif
 *
 * \if CHINESE
 * @brief 移除并返回指定索引处的项目
 *
 * @param[in] index 要移除的项目的索引
 * @return 已移除的项目，如果索引超出范围则返回 nullptr
 * \endif
 */
QLayoutItem* QwtDynGridLayout::takeAt(int index)
{
    if (index < 0 || index >= m_data->itemList.count())
        return nullptr;

    m_data->isDirty = true;
    return m_data->itemList.takeAt(index);
}

/**
 * \if ENGLISH
 * @brief Get the number of items in the layout
 *
 * @return Number of layout items
 * \endif
 *
 * \if CHINESE
 * @brief 获取布局中的项目数量
 *
 * @return 布局项目的数量
 * \endif
 */
int QwtDynGridLayout::count() const
{
    return m_data->itemList.count();
}

/**
 * \if ENGLISH
 * @brief Set the expanding directions for the layout
 *
 * @details A value of Qt::Vertical or Qt::Horizontal means that it wants to grow
 * in only one dimension, while Qt::Vertical | Qt::Horizontal means that it wants
 * to grow in both dimensions.
 *
 * @param[in] expanding Or'd orientations
 * \endif
 *
 * \if CHINESE
 * @brief 设置布局的扩展方向
 *
 * @details Qt::Vertical 或 Qt::Horizontal 表示只想在一个维度上增长，
 * Qt::Vertical | Qt::Horizontal 表示想在两个维度上增长。
 *
 * @param[in] expanding 组合的方向标志
 * \endif
 */
void QwtDynGridLayout::setExpandingDirections(Qt::Orientations expanding)
{
    m_data->expanding = expanding;
}

/**
 * \if ENGLISH
 * @brief Get the expanding directions for the layout
 *
 * @return Orientations where the layout expands
 * \endif
 *
 * \if CHINESE
 * @brief 获取布局的扩展方向
 *
 * @return 布局扩展的方向
 * \endif
 */
Qt::Orientations QwtDynGridLayout::expandingDirections() const
{
    return m_data->expanding;
}

/**
 * \if ENGLISH
 * @brief Set the geometry for the layout
 *
 * @details Reorganizes columns and rows and resizes managed items within the rectangle.
 *
 * @param[in] rect Layout geometry
 * \endif
 *
 * \if CHINESE
 * @brief 设置布局的几何区域
 *
 * @details 重新组织列和行，并在矩形内调整管理项目的大小。
 *
 * @param[in] rect 布局的几何区域
 * \endif
 */
void QwtDynGridLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);

    if (isEmpty())
        return;

    m_data->numColumns = columnsForWidth(rect.width());
    m_data->numRows    = itemCount() / m_data->numColumns;
    if (itemCount() % m_data->numColumns)
        m_data->numRows++;

    const QList< QRect > itemGeometries = layoutItems(rect, m_data->numColumns);

    int index = 0;
    for (auto* item : qwt_as_const(m_data->itemList)) {
        item->setGeometry(itemGeometries[ index ]);
        index++;
    }
}

/**
 * \if ENGLISH
 * @brief Calculate the number of columns for a given width
 *
 * @details The calculation tries to use as many columns as possible, limited by maxColumns().
 *
 * @param[in] width Available width for all columns
 * @return Number of columns for the given width
 * \endif
 *
 * \if CHINESE
 * @brief 计算给定宽度的列数
 *
 * @details 计算尽量使用最多的列数，受 maxColumns() 限制。
 *
 * @param[in] width 所有列的可用宽度
 * @return 给定宽度的列数
 * \endif
 */
uint QwtDynGridLayout::columnsForWidth(int width) const
{
    if (isEmpty())
        return 0;

    uint maxColumns = itemCount();
    if (m_data->maxColumns > 0)
        maxColumns = qMin(m_data->maxColumns, maxColumns);

    if (maxRowWidth(maxColumns) <= width)
        return maxColumns;

    for (uint numColumns = 2; numColumns <= maxColumns; numColumns++) {
        const int rowWidth = maxRowWidth(numColumns);
        if (rowWidth > width)
            return numColumns - 1;
    }

    return 1;  // At least 1 column
}

/*!
   Calculate the width of a layout for a given number of
   columns.

   \param numColumns Given number of columns
   \param itemWidth Array of the width hints for all items
  */
int QwtDynGridLayout::maxRowWidth(int numColumns) const
{
    int col;

    QVector< int > colWidth(numColumns);
    for (col = 0; col < numColumns; col++)
        colWidth[ col ] = 0;

    if (m_data->isDirty)
        m_data->updateLayoutCache();

    for (int index = 0; index < m_data->itemSizeHints.count(); index++) {
        col             = index % numColumns;
        colWidth[ col ] = qMax(colWidth[ col ], m_data->itemSizeHints[ index ].width());
    }

    const QMargins m = contentsMargins();

    int rowWidth = m.left() + m.right() + (numColumns - 1) * spacing();
    for (col = 0; col < numColumns; col++)
        rowWidth += colWidth[ col ];

    return rowWidth;
}

/**
 * \if ENGLISH
 * @brief Get the maximum width of all layout items
 *
 * @return Maximum width of all items
 * \endif
 *
 * \if CHINESE
 * @brief 获取所有布局项目的最大宽度
 *
 * @return 所有项目的最大宽度
 * \endif
 */
int QwtDynGridLayout::maxItemWidth() const
{
    if (isEmpty())
        return 0;

    if (m_data->isDirty)
        m_data->updateLayoutCache();

    int w = 0;
    for (int i = 0; i < m_data->itemSizeHints.count(); i++) {
        const int itemW = m_data->itemSizeHints[ i ].width();
        if (itemW > w)
            w = itemW;
    }

    return w;
}

/**
 * \if ENGLISH
 * @brief Calculate the geometries of the layout items
 *
 * @param[in] rect Rectangle where to place the items
 * @param[in] numColumns Number of columns
 * @return List of item geometries
 * \endif
 *
 * \if CHINESE
 * @brief 计算布局项目的几何区域
 *
 * @param[in] rect 放置项目的矩形区域
 * @param[in] numColumns 列数
 * @return 项目几何区域列表
 * \endif
 */
QList< QRect > QwtDynGridLayout::layoutItems(const QRect& rect, uint numColumns) const
{
    QList< QRect > itemGeometries;
    if (numColumns == 0 || isEmpty())
        return itemGeometries;

    uint numRows = itemCount() / numColumns;
    if (numColumns % itemCount())
        numRows++;

    if (numRows == 0)
        return itemGeometries;

    QVector< int > rowHeight(numRows);
    QVector< int > colWidth(numColumns);

    layoutGrid(numColumns, rowHeight, colWidth);

    bool expandH, expandV;
    expandH = expandingDirections() & Qt::Horizontal;
    expandV = expandingDirections() & Qt::Vertical;

    if (expandH || expandV)
        stretchGrid(rect, numColumns, rowHeight, colWidth);

    const int maxColumns    = m_data->maxColumns;
    m_data->maxColumns      = numColumns;
    const QRect alignedRect = alignmentRect(rect);
    m_data->maxColumns      = maxColumns;

    const int xOffset = expandH ? 0 : alignedRect.x();
    const int yOffset = expandV ? 0 : alignedRect.y();

    QVector< int > colX(numColumns);
    QVector< int > rowY(numRows);

    const int xySpace = spacing();

    const QMargins m = contentsMargins();

    rowY[ 0 ] = yOffset + m.top();
    for (uint r = 1; r < numRows; r++)
        rowY[ r ] = rowY[ r - 1 ] + rowHeight[ r - 1 ] + xySpace;

    colX[ 0 ] = xOffset + m.left();
    for (uint c = 1; c < numColumns; c++)
        colX[ c ] = colX[ c - 1 ] + colWidth[ c - 1 ] + xySpace;

    const int itemCount = m_data->itemList.size();
    itemGeometries.reserve(itemCount);

    for (int i = 0; i < itemCount; i++) {
        const int row = i / numColumns;
        const int col = i % numColumns;

        const QRect itemGeometry(colX[ col ], rowY[ row ], colWidth[ col ], rowHeight[ row ]);
        itemGeometries.append(itemGeometry);
    }

    return itemGeometries;
}

/*!
   Calculate the dimensions for the columns and rows for a grid
   of numColumns columns.

   \param numColumns Number of columns.
   \param rowHeight Array where to fill in the calculated row heights.
   \param colWidth Array where to fill in the calculated column widths.
  */

void QwtDynGridLayout::layoutGrid(uint numColumns, QVector< int >& rowHeight, QVector< int >& colWidth) const
{
    if (numColumns <= 0)
        return;

    if (m_data->isDirty)
        m_data->updateLayoutCache();

    for (int index = 0; index < m_data->itemSizeHints.count(); index++) {
        const int row = index / numColumns;
        const int col = index % numColumns;

        const QSize& size = m_data->itemSizeHints[ index ];

        rowHeight[ row ] = (col == 0) ? size.height() : qMax(rowHeight[ row ], size.height());
        colWidth[ col ]  = (row == 0) ? size.width() : qMax(colWidth[ col ], size.width());
    }
}

/*!
   Stretch columns in case of expanding() & QSizePolicy::Horizontal and
   rows in case of expanding() & QSizePolicy::Vertical to fill the entire
   rect. Rows and columns are stretched with the same factor.

   \param rect Bounding rectangle
   \param numColumns Number of columns
   \param rowHeight Array to be filled with the calculated row heights
   \param colWidth Array to be filled with the calculated column widths

   \sa setExpanding(), expanding()
  */
void QwtDynGridLayout::stretchGrid(const QRect& rect, uint numColumns, QVector< int >& rowHeight, QVector< int >& colWidth) const
{
    if (numColumns == 0 || isEmpty())
        return;

    bool expandH, expandV;
    expandH = expandingDirections() & Qt::Horizontal;
    expandV = expandingDirections() & Qt::Vertical;

    const QMargins m = contentsMargins();

    if (expandH) {
        int xDelta = rect.width() - m.left() - m.right() - (numColumns - 1) * spacing();
        for (uint col = 0; col < numColumns; col++)
            xDelta -= colWidth[ col ];

        if (xDelta > 0) {
            for (uint col = 0; col < numColumns; col++) {
                const int space = xDelta / (numColumns - col);
                colWidth[ col ] += space;
                xDelta -= space;
            }
        }
    }

    if (expandV) {
        uint numRows = itemCount() / numColumns;
        if (itemCount() % numColumns)
            numRows++;

        int yDelta = rect.height() - m.top() - m.bottom() - (numRows - 1) * spacing();
        for (uint row = 0; row < numRows; row++)
            yDelta -= rowHeight[ row ];

        if (yDelta > 0) {
            for (uint row = 0; row < numRows; row++) {
                const int space = yDelta / (numRows - row);
                rowHeight[ row ] += space;
                yDelta -= space;
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Check if the layout has height for width
 *
 * @return true, indicating QwtDynGridLayout implements heightForWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 检查布局是否有高度随宽度变化的特性
 *
 * @return true，表示 QwtDynGridLayout 实现了 heightForWidth()
 * \endif
 */
bool QwtDynGridLayout::hasHeightForWidth() const
{
    return true;
}

/**
 * \if ENGLISH
 * @brief Get the preferred height for a given width
 *
 * @param[in] width Width to calculate height for
 * @return Preferred height for the given width
 * \endif
 *
 * \if CHINESE
 * @brief 获取给定宽度的首选高度
 *
 * @param[in] width 要计算高度的宽度
 * @return 给定宽度的首选高度
 * \endif
 */
int QwtDynGridLayout::heightForWidth(int width) const
{
    if (isEmpty())
        return 0;

    const uint numColumns = columnsForWidth(width);
    uint numRows          = itemCount() / numColumns;
    if (itemCount() % numColumns)
        numRows++;

    QVector< int > rowHeight(numRows);
    QVector< int > colWidth(numColumns);

    layoutGrid(numColumns, rowHeight, colWidth);

    const QMargins m = contentsMargins();

    int h = m.top() + m.bottom() + (numRows - 1) * spacing();
    for (uint row = 0; row < numRows; row++)
        h += rowHeight[ row ];

    return h;
}

/**
 * \if ENGLISH
 * @brief Get the size hint for the layout
 *
 * @details If maxColumns() > 0 it is the size for a grid with maxColumns() columns,
 * otherwise it is the size for a grid with only one row.
 *
 * @return Size hint
 * \endif
 *
 * \if CHINESE
 * @brief 获取布局的大小提示
 *
 * @details 如果 maxColumns() > 0，则返回 maxColumns() 列网格的大小，
 * 否则返回只有一行的网格的大小。
 *
 * @return 大小提示
 * \endif
 */
QSize QwtDynGridLayout::sizeHint() const
{
    if (isEmpty())
        return QSize();

    uint numColumns = itemCount();
    if (m_data->maxColumns > 0)
        numColumns = qMin(m_data->maxColumns, numColumns);

    uint numRows = itemCount() / numColumns;
    if (itemCount() % numColumns)
        numRows++;

    QVector< int > rowHeight(numRows);
    QVector< int > colWidth(numColumns);

    layoutGrid(numColumns, rowHeight, colWidth);

    const QMargins m = contentsMargins();

    int h = m.top() + m.bottom() + (numRows - 1) * spacing();
    for (uint row = 0; row < numRows; row++)
        h += rowHeight[ row ];

    int w = m.left() + m.right() + (numColumns - 1) * spacing();
    for (uint col = 0; col < numColumns; col++)
        w += colWidth[ col ];

    return QSize(w, h);
}

/**
 * \if ENGLISH
 * @brief Get the number of rows in the current layout
 *
 * @warning The number of rows might change whenever the geometry changes.
 *
 * @return Number of rows
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前布局的行数
 *
 * @warning 行数可能在几何区域变化时发生改变。
 *
 * @return 行数
 * \endif
 */
uint QwtDynGridLayout::numRows() const
{
    return m_data->numRows;
}

/**
 * \if ENGLISH
 * @brief Get the number of columns in the current layout
 *
 * @warning The number of columns might change whenever the geometry changes.
 *
 * @return Number of columns
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前布局的列数
 *
 * @warning 列数可能在几何区域变化时发生改变。
 *
 * @return 列数
 * \endif
 */
uint QwtDynGridLayout::numColumns() const
{
    return m_data->numColumns;
}
