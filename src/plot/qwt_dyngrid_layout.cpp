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
 * @brief Constructor with parent widget, margin and spacing
 *
 * @param[in] parent Parent widget
 * @param[in] margin Margin around the layout
 * @param[in] spacing Spacing between items
 *
 */
QwtDynGridLayout::QwtDynGridLayout(QWidget* parent, int margin, int spacing) : QLayout(parent)
{
    init();

    setSpacing(spacing);
    setContentsMargins(margin, margin, margin, margin);
}

/**
 * @brief Constructor with spacing only
 *
 * @param[in] spacing Spacing between items
 *
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
 * @brief Destructor
 *
 * @details Deletes all layout items and private data.
 *
 */
QwtDynGridLayout::~QwtDynGridLayout()
{
    qDeleteAll(m_data->itemList);
    delete m_data;
}

/**
 * @brief Invalidate all internal caches
 *
 * @details Marks the layout as dirty so it will recalculate on next layout operation.
 *
 */
void QwtDynGridLayout::invalidate()
{
    m_data->isDirty = true;
    QLayout::invalidate();
}

/**
 * @brief Set the upper limit for the number of columns
 *
 * @param[in] maxColumns Upper limit, 0 means unlimited
 *
 */
void QwtDynGridLayout::setMaxColumns(uint maxColumns)
{
    m_data->maxColumns = maxColumns;
}

/**
 * @brief Get the upper limit for the number of columns
 *
 * @return Upper limit for the number of columns, 0 means unlimited
 *
 */
uint QwtDynGridLayout::maxColumns() const
{
    return m_data->maxColumns;
}

/**
 * @brief Add an item to the next free position
 *
 * @param[in] item Layout item to add
 *
 */
void QwtDynGridLayout::addItem(QLayoutItem* item)
{
    m_data->itemList.append(item);
    invalidate();
}

/**
 * @brief Check if the layout is empty
 *
 * @return true if the layout contains no items
 *
 */
bool QwtDynGridLayout::isEmpty() const
{
    return m_data->itemList.isEmpty();
}

/**
 * @brief Get the number of layout items
 *
 * @return Number of items in the layout
 *
 */
uint QwtDynGridLayout::itemCount() const
{
    return m_data->itemList.count();
}

/**
 * @brief Get the item at a specific index
 *
 * @param[in] index Index of the item
 * @return Item at the specified index, or nullptr if index is out of range
 *
 */
QLayoutItem* QwtDynGridLayout::itemAt(int index) const
{
    if (index < 0 || index >= m_data->itemList.count())
        return nullptr;

    return m_data->itemList.at(index);
}

/**
 * @brief Remove and return the item at a specific index
 *
 * @param[in] index Index of the item to remove
 * @return Removed item, or nullptr if index is out of range
 *
 */
QLayoutItem* QwtDynGridLayout::takeAt(int index)
{
    if (index < 0 || index >= m_data->itemList.count())
        return nullptr;

    m_data->isDirty = true;
    return m_data->itemList.takeAt(index);
}

/**
 * @brief Get the number of items in the layout
 *
 * @return Number of layout items
 *
 */
int QwtDynGridLayout::count() const
{
    return m_data->itemList.count();
}

/**
 * @brief Set the expanding directions for the layout
 *
 * @details A value of Qt::Vertical or Qt::Horizontal means that it wants to grow
 * in only one dimension, while Qt::Vertical | Qt::Horizontal means that it wants
 * to grow in both dimensions.
 *
 * @param[in] expanding Or'd orientations
 *
 */
void QwtDynGridLayout::setExpandingDirections(Qt::Orientations expanding)
{
    m_data->expanding = expanding;
}

/**
 * @brief Get the expanding directions for the layout
 *
 * @return Orientations where the layout expands
 *
 */
Qt::Orientations QwtDynGridLayout::expandingDirections() const
{
    return m_data->expanding;
}

/**
 * @brief Set the geometry for the layout
 *
 * @details Reorganizes columns and rows and resizes managed items within the rectangle.
 *
 * @param[in] rect Layout geometry
 *
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
 * @brief Calculate the number of columns for a given width
 *
 * @details The calculation tries to use as many columns as possible, limited by maxColumns().
 *
 * @param[in] width Available width for all columns
 * @return Number of columns for the given width
 *
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

   @param numColumns Given number of columns
   @param itemWidth Array of the width hints for all items
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
 * @brief Get the maximum width of all layout items
 *
 * @return Maximum width of all items
 *
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
 * @brief Calculate the geometries of the layout items
 *
 * @param[in] rect Rectangle where to place the items
 * @param[in] numColumns Number of columns
 * @return List of item geometries
 *
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

   @param numColumns Number of columns.
   @param rowHeight Array where to fill in the calculated row heights.
   @param colWidth Array where to fill in the calculated column widths.
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

   @param rect Bounding rectangle
   @param numColumns Number of columns
   @param rowHeight Array to be filled with the calculated row heights
   @param colWidth Array to be filled with the calculated column widths

   @sa setExpanding(), expanding()
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
 * @brief Check if the layout has height for width
 *
 * @return true, indicating QwtDynGridLayout implements heightForWidth()
 *
 */
bool QwtDynGridLayout::hasHeightForWidth() const
{
    return true;
}

/**
 * @brief Get the preferred height for a given width
 *
 * @param[in] width Width to calculate height for
 * @return Preferred height for the given width
 *
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
 * @brief Get the size hint for the layout
 *
 * @details If maxColumns() > 0 it is the size for a grid with maxColumns() columns,
 * otherwise it is the size for a grid with only one row.
 *
 * @return Size hint
 *
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
 * @brief Get the number of rows in the current layout
 *
 * @warning The number of rows might change whenever the geometry changes.
 *
 * @return Number of rows
 *
 */
uint QwtDynGridLayout::numRows() const
{
    return m_data->numRows;
}

/**
 * @brief Get the number of columns in the current layout
 *
 * @warning The number of columns might change whenever the geometry changes.
 *
 * @return Number of columns
 *
 */
uint QwtDynGridLayout::numColumns() const
{
    return m_data->numColumns;
}
