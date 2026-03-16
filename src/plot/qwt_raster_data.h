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

#ifndef QWT_RASTER_DATA_H
#define QWT_RASTER_DATA_H

#include "qwt_global.h"
#include <qnamespace.h>

class QwtInterval;
class QPolygonF;
class QRectF;
class QSize;
template< typename T >
class QList;
template< class Key, class T >
class QMap;

/**
 * \if ENGLISH
 * @brief QwtRasterData defines an interface to any type of raster data.
 * 
 * QwtRasterData is an abstract interface, that is used by
 * QwtPlotRasterItem to find the values at the pixels of its raster.
 * 
 * Gaps inside the bounding rectangle of the data can be indicated by NaN
 * values ( when WithoutGaps is disabled ).
 * 
 * Often a raster item is used to display values from a matrix. Then the
 * derived raster data class needs to implement some sort of resampling,
 * that maps the raster of the matrix into the requested raster of
 * the raster item ( depending on resolution and scales of the canvas ).
 * 
 * QwtMatrixRasterData implements raster data, that returns values from
 * a given 2D matrix.
 * 
 * @sa QwtMatrixRasterData
 * \endif
 * 
 * \if CHINESE
 * @brief QwtRasterData 定义了任何类型栅格数据的接口
 * 
 * QwtRasterData 是一个抽象接口，被 QwtPlotRasterItem 用于查找其栅格像素处的值。
 * 
 * 数据边界矩形内的间隙可以通过 NaN 值表示（当 WithoutGaps 被禁用时）。
 * 
 * 栅格项通常用于显示矩阵中的值。然后派生的栅格数据类需要实现某种重采样，
 * 将矩阵的栅格映射到栅格项的请求栅格中（取决于画布的分辨率和比例）。
 * 
 * QwtMatrixRasterData 实现了栅格数据，从给定的 2D 矩阵返回值。
 * 
 * @sa QwtMatrixRasterData
 * \endif
 */
class QWT_EXPORT QwtRasterData
{
public:
    //! Contour lines
    using ContourLines = QMap< double, QPolygonF >;

    /**
     * \if ENGLISH
     * @brief Raster data attributes
     * 
     * Additional information that is used to improve processing
     * of the data.
     * \endif
     * 
     * \if CHINESE
     * @brief 栅格数据属性
     * 
     * 用于改进数据处理的附加信息。
     * \endif
     */
    enum Attribute
    {
        /**
         * \if ENGLISH
         * The bounding rectangle of the data is spanned by
         * the interval(Qt::XAxis) and interval(Qt::YAxis).
         * 
         * WithoutGaps indicates, that the data has no gaps
         * ( unknown values ) in this area and the result of
         * value() does not need to be checked for NaN values.
         * 
         * Enabling this flag will have an positive effect on
         * the performance of rendering a QwtPlotSpectrogram.
         * 
         * The default setting is false.
         * 
         * @note NaN values indicate an undefined value
         * \endif
         * 
         * \if CHINESE
         * 数据的边界矩形由 interval(Qt::XAxis) 和 interval(Qt::YAxis) 跨越。
         * 
         * WithoutGaps 表示数据在该区域没有间隙（未知值），
         * value() 的结果不需要检查 NaN 值。
         * 
         * 启用此标志将对渲染 QwtPlotSpectrogram 的性能产生积极影响。
         * 
         * 默认设置为 false。
         * 
         * @note NaN 值表示未定义值
         * \endif
         */
        WithoutGaps = 0x01
    };

    Q_DECLARE_FLAGS(Attributes, Attribute)

    /**
     * \if ENGLISH
     * @brief Flags to modify the contour algorithm
     * \endif
     * 
     * \if CHINESE
     * @brief 修改等高线算法的标志
     * \endif
     */
    enum ConrecFlag
    {
        //! Ignore all vertices on the same level
        IgnoreAllVerticesOnLevel = 0x01,

        //! Ignore all values, that are out of range
        IgnoreOutOfRange = 0x02
    };

    Q_DECLARE_FLAGS(ConrecFlags, ConrecFlag)

    /// Constructor
    QwtRasterData();
    /// Destructor
    virtual ~QwtRasterData();

    /// Set an attribute
    void setAttribute(Attribute, bool on = true);
    /// Test an attribute
    bool testAttribute(Attribute) const;

    /**
     * \if ENGLISH
     * @brief Get the bounding interval for an axis
     * @param axis Axis index
     * @return Bounding interval
     * @sa setInterval
     * \endif
     * 
     * \if CHINESE
     * @brief 获取轴的边界区间
     * @param axis 轴索引
     * @return 边界区间
     * @sa setInterval
     * \endif
     */
    virtual QwtInterval interval(Qt::Axis) const = 0;

    /// Return a hint for the raster item, about how to align the pixels
    virtual QRectF pixelHint(const QRectF&) const;

    /// Initialize the raster
    virtual void initRaster(const QRectF&, const QSize& raster);
    /// Discard the raster
    virtual void discardRaster();

    /**
     * \if ENGLISH
     * @brief Get the value at a raster position
     * @param x X value in plot coordinates
     * @param y Y value in plot coordinates
     * @return Value at (x, y)
     * \endif
     * 
     * \if CHINESE
     * @brief 获取栅格位置的值
     * @param x 绘图坐标中的 X 值
     * @param y 绘图坐标中的 Y 值
     * @return (x, y) 处的值
     * \endif
     */
    virtual double value(double x, double y) const = 0;

    /// Calculate contour lines
    virtual ContourLines contourLines(const QRectF& rect, const QSize& raster, const QList< double >& levels, ConrecFlags) const;

    class Contour3DPoint;
    class ContourPlane;

private:
    Q_DISABLE_COPY(QwtRasterData)

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtRasterData::ConrecFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QwtRasterData::Attributes)

#endif
