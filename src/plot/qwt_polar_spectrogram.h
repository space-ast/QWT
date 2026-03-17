/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_SPECTROGRAM_H
#define QWT_POLAR_SPECTROGRAM_H

#include "qwt_global.h"
#include "qwt_polar_item.h"
#include <qimage.h>

class QwtRasterData;
class QwtColorMap;

/**
 * \if ENGLISH
 * @brief An item, which displays a spectrogram
 * @details A spectrogram displays 3-dimensional data, where the 3rd dimension
 *          ( the intensity ) is displayed using colors. The colors are calculated
 *          from the values using a color map.
 * 
 * @sa QwtRasterData, QwtColorMap
 * \endif
 * 
 * \if CHINESE
 * @brief 显示光谱图的绘图项
 * @details 光谱图显示三维数据，其中第三维（强度）使用颜色显示。
 *          颜色是通过颜色映射从值计算得出的。
 * 
 * @sa QwtRasterData, QwtColorMap
 * \endif
 */
class QWT_EXPORT QwtPolarSpectrogram : public QwtPolarItem
{
  public:
    /**
     * \if ENGLISH
     * @brief Attributes to modify the drawing algorithm
     * @details The default setting disables ApproximatedAtan
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     * 
     * \if CHINESE
     * @brief 修改绘制算法的属性
     * @details 默认设置禁用 ApproximatedAtan
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * Use qwtFastAtan2 instead of atan2 for translating
         * widget into polar coordinates.
         * \endif
         * 
         * \if CHINESE
         * 使用 qwtFastAtan2 代替 atan2 将控件坐标转换为极坐标。
         * \endif
         */
        ApproximatedAtan = 0x01
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    /// Constructor
    explicit QwtPolarSpectrogram();
    /// Destructor
    virtual ~QwtPolarSpectrogram();

    /// Set the data
    void setData( QwtRasterData* data );
    /// Get the data
    const QwtRasterData* data() const;

    /// Set the color map
    void setColorMap( QwtColorMap* );
    /// Get the color map
    const QwtColorMap* colorMap() const;

    /// Set a paint attribute
    void setPaintAttribute( PaintAttribute, bool on = true );
    /// Test a paint attribute
    bool testPaintAttribute( PaintAttribute ) const;

    /// Get the runtime type information
    virtual int rtti() const override;

    /// Draw the spectrogram
    virtual void draw( QPainter* painter,
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, double radius,
        const QRectF& canvasRect ) const override;

    /// Get the bounding interval for a scale
    virtual QwtInterval boundingInterval( int scaleId ) const override;

  protected:
    /// Render an image
    virtual QImage renderImage(
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, const QRect& rect ) const;

    /// Render a tile
    virtual void renderTile(
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, const QPoint& imagePos,
        const QRect& tile, QImage* image ) const;

  private:
    class TileInfo;
    void renderTileInfo( const QwtScaleMap&, const QwtScaleMap&,
        const QPointF& pole, TileInfo* ) const;

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarSpectrogram::PaintAttributes )

#endif
