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

#ifndef QWT_PIXEL_MATRIX_H
#define QWT_PIXEL_MATRIX_H

#include "qwt_global.h"

#include <qbitarray.h>
#include <qrect.h>

/**
 * \if ENGLISH
 * @brief A bit field corresponding to the pixels of a rectangle
 *
 * QwtPixelMatrix is intended to filter out duplicates in an
 * unsorted array of points.
 * \endif
 *
 * \if CHINESE
 * @brief 与矩形像素对应的位域
 *
 * QwtPixelMatrix 用于过滤未排序点数组中的重复项。
 * \endif
 */
class QWT_EXPORT QwtPixelMatrix : public QBitArray
{
  public:
    // Constructor with bounding rectangle
    explicit QwtPixelMatrix( const QRect& rect );
    // Destructor
    ~QwtPixelMatrix();

    // Set the bounding rectangle of the matrix
    void setRect( const QRect& rect );
    // Get the bounding rectangle
    QRect rect() const;

    // Test if a pixel has been set
    bool testPixel( int x, int y ) const;
    // Set a pixel and test if it was set before
    bool testAndSetPixel( int x, int y, bool on );

    // Calculate the index in the bit field for a position
    int index( int x, int y ) const;

  private:
    QRect m_rect;
};

/**
 * \if ENGLISH
 * @brief Test if a pixel has been set
 * @param[in] x X-coordinate
 * @param[in] y Y-coordinate
 * @return true when position is outside of rect(), or when the pixel has already been set
 * \endif
 *
 * \if CHINESE
 * @brief 测试像素是否已设置
 * @param[in] x X坐标
 * @param[in] y Y坐标
 * @return 当位置在 rect() 之外，或像素已设置时返回 true
 * \endif
 */
inline bool QwtPixelMatrix::testPixel( int x, int y ) const
{
    const int idx = index( x, y );
    return ( idx >= 0 ) ? testBit( idx ) : true;
}

/**
 * \if ENGLISH
 * @brief Set a pixel and test if a pixel has been set before
 * @param[in] x X-coordinate
 * @param[in] y Y-coordinate
 * @param[in] on Set/Clear the pixel
 * @return true when position is outside of rect(), or when the pixel was set before
 * \endif
 *
 * \if CHINESE
 * @brief 设置像素并测试之前是否已设置
 * @param[in] x X坐标
 * @param[in] y Y坐标
 * @param[in] on 设置/清除像素
 * @return 当位置在 rect() 之外，或像素之前已设置时返回 true
 * \endif
 */
inline bool QwtPixelMatrix::testAndSetPixel( int x, int y, bool on )
{
    const int idx = index( x, y );
    if ( idx < 0 )
        return true;

    const bool onBefore = testBit( idx );
    setBit( idx, on );

    return onBefore;
}

/**
 * \if ENGLISH
 * @brief Calculate the index in the bit field corresponding to a position
 * @param[in] x X-coordinate
 * @param[in] y Y-coordinate
 * @return Index when rect() contains position, otherwise -1
 * \endif
 *
 * \if CHINESE
 * @brief 计算位域中对应位置的索引
 * @param[in] x X坐标
 * @param[in] y Y坐标
 * @return 当 rect() 包含位置时返回索引，否则返回 -1
 * \endif
 */
inline int QwtPixelMatrix::index( int x, int y ) const
{
    const int dx = x - m_rect.x();
    if ( dx < 0 || dx >= m_rect.width() )
        return -1;

    const int dy = y - m_rect.y();
    if ( dy < 0 || dy >= m_rect.height() )
        return -1;

    return dy * m_rect.width() + dx;
}

#endif
