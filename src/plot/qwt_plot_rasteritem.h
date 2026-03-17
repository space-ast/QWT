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

#ifndef QWT_PLOT_RASTERITEM_H
#define QWT_PLOT_RASTERITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

#include <qstring.h>

class QwtInterval;

/**
 * \if ENGLISH
 * @brief A class, which displays raster data
 * @details Raster data is a grid of pixel values, that can be represented
 *          as a QImage. It is used for many types of information like
 *          spectrograms, cartograms, geographical maps ...
 * 
 *          Often a plot has several types of raster data organized in layers.
 *          (e.g a geographical map, with weather statistics ).
 *          Using setAlpha() raster items can be stacked easily.
 * 
 *          QwtPlotRasterItem is only implemented for images of the following formats:
 *          QImage::Format_Indexed8, QImage::Format_ARGB32.
 * 
 * @sa QwtPlotSpectrogram
 * \endif
 * 
 * \if CHINESE
 * @brief 显示栅格数据的类
 * @details 栅格数据是一组像素值的网格，可以表示为 QImage。它用于许多类型的信息，如
 *           spectrogram、图表、地理地图等。
 * 
 *          通常，一个绘图会有几种类型的栅格数据组织在层中。
 *          （例如带有天气统计数据的地理地图）。
 *          使用 setAlpha() 可以轻松堆叠栅格项。
 * 
 *          QwtPlotRasterItem 仅为以下格式的图像实现：
 *          QImage::Format_Indexed8、QImage::Format_ARGB32。
 * 
 * @sa QwtPlotSpectrogram
 * \endif
 */

class QWT_EXPORT QwtPlotRasterItem : public QwtPlotItem
{
  public:
    /**
     * \if ENGLISH
     * @brief Cache policy
     * @details The default policy is NoCache
     * \endif
     * 
     * \if CHINESE
     * @brief 缓存策略
     * @details 默认策略是 NoCache
     * \endif
     */
    enum CachePolicy
    {
        /**
         * \if ENGLISH
         * renderImage() is called each time the item has to be repainted
         * \endif
         * 
         * \if CHINESE
         * 每次项目需要重绘时调用 renderImage()
         * \endif
         */
        NoCache,

        /**
         * \if ENGLISH
         * renderImage() is called, whenever the image cache is not valid,
         * or the scales, or the size of the canvas has changed.
         * 
         * This type of cache is useful for improving the performance
         * of hide/show operations or manipulations of the alpha value.
         * All other situations are handled by the canvas backing store.
         * \endif
         * 
         * \if CHINESE
         * 当图像缓存无效，或比例尺或画布大小发生变化时，调用 renderImage()。
         * 
         * 这种类型的缓存对于提高隐藏/显示操作或 alpha 值操作的性能很有用。
         * 所有其他情况由画布后备存储处理。
         * \endif
         */
        PaintCache
    };

    /**
     * \if ENGLISH
     * @brief Paint attributes
     * @details Attributes to modify the drawing algorithm.
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     * 
     * \if CHINESE
     * @brief 绘制属性
     * @details 用于修改绘制算法的属性。
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * When the image is rendered according to the data pixels
         * ( QwtRasterData::pixelHint() ) it can be expanded to paint
         * device resolution before it is passed to QPainter.
         * The expansion algorithm rounds the pixel borders in the same
         * way as the axis ticks, what is usually better than the
         * scaling algorithm implemented in Qt.
         * Disabling this flag might make sense, to reduce the size of a
         * document/file. If this is possible for a document format
         * depends on the implementation of the specific QPaintEngine.
         * \endif
         * 
         * \if CHINESE
         * 当图像根据数据像素（QwtRasterData::pixelHint()）渲染时，
         * 在传递给 QPainter 之前，可以将其扩展到绘制设备分辨率。
         * 扩展算法以与轴刻度相同的方式舍入像素边界，这通常比 Qt 中实现的缩放算法更好。
         * 禁用此标志可能有助于减少文档/文件的大小。
         * 这是否可能取决于特定 QPaintEngine 的实现。
         * \endif
         */
        PaintInDeviceResolution = 1
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    /// Constructor
    explicit QwtPlotRasterItem( const QString& title = QString() );
    /// Constructor with title
    explicit QwtPlotRasterItem( const QwtText& title );
    /// Destructor
    virtual ~QwtPlotRasterItem();

    /// Set a paint attribute
    void setPaintAttribute( PaintAttribute, bool on = true );
    /// Test a paint attribute
    bool testPaintAttribute( PaintAttribute ) const;

    /// Set the alpha value
    void setAlpha( int alpha );
    /// Get the alpha value
    int alpha() const;

    /// Set the cache policy
    void setCachePolicy( CachePolicy );
    /// Get the cache policy
    CachePolicy cachePolicy() const;

    /// Invalidate the cache
    void invalidateCache();

    /// Draw the raster item
    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const override;

    /// Get the pixel hint
    virtual QRectF pixelHint( const QRectF& ) const;

    /// Get the interval for a specific axis
    virtual QwtInterval interval(Qt::Axis) const;
    /// Get the bounding rectangle
    virtual QRectF boundingRect() const override;

  protected:
    /**
     * \if ENGLISH
     * @brief Render an image
     * @details An implementation of render() might iterate over all
     *          pixels of imageRect. Each pixel has to be translated into
     *          the corresponding position in scale coordinates using the maps.
     *          This position can be used to look up a value in a implementation
     *          specific way and to map it into a color.
     * 
     * @param xMap X-Scale Map
     * @param yMap Y-Scale Map
     * @param area Requested area for the image in scale coordinates
     * @param imageSize Requested size of the image
     * 
     * @return Rendered image
     * \endif
     * 
     * \if CHINESE
     * @brief 渲染图像
     * @details render() 的实现可能会迭代 imageRect 的所有像素。
     *          每个像素必须使用映射转换为比例尺坐标中的相应位置。
     *          此位置可用于以实现特定的方式查找值并将其映射到颜色。
     * 
     * @param xMap X 比例尺映射
     * @param yMap Y 比例尺映射
     * @param area 图像在比例尺坐标中的请求区域
     * @param imageSize 图像的请求大小
     * 
     * @return 渲染的图像
     * \endif
     */
    virtual QImage renderImage( const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRectF& area,
        const QSize& imageSize ) const = 0;

    /// Get the image map
    virtual QwtScaleMap imageMap( Qt::Orientation,
        const QwtScaleMap& map, const QRectF& area,
        const QSize& imageSize, double pixelSize) const;

  private:
    explicit QwtPlotRasterItem( const QwtPlotRasterItem& );
    QwtPlotRasterItem& operator=( const QwtPlotRasterItem& );

    /// Initialize the raster item
    void init();

    /// Compose the image
    QImage compose( const QwtScaleMap&, const QwtScaleMap&,
        const QRectF& imageArea, const QRectF& paintRect,
        const QSize& imageSize, bool doCache) const;


    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotRasterItem::PaintAttributes )

#endif
