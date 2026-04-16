/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_spectrogram.h"
#include "qwt_polar.h"
#include "qwt_polar_plot.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_raster_data.h"
#include "qwt_math.h"
#include "qwt_clipper.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>

#if QT_VERSION < 0x050000
#include <qnumeric.h>
#endif

class QwtPolarSpectrogram::TileInfo
{
  public:
    QPoint imagePos;
    QRect rect;
    QImage* image;
};

class QwtPolarSpectrogram::PrivateData
{
  public:
    PrivateData()
        : data( nullptr )
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete data;
        delete colorMap;
    }

    QwtRasterData* data;
    QwtColorMap* colorMap;

    QwtPolarSpectrogram::PaintAttributes paintAttributes;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Creates a spectrogram item with default settings.
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 创建具有默认设置的光谱图项。
 * \endif
 */
QwtPolarSpectrogram::QwtPolarSpectrogram()
    : QwtPolarItem( QwtText( "Spectrogram" ) )
{
    m_data = new PrivateData;

    setItemAttribute( QwtPolarItem::AutoScale );
    setItemAttribute( QwtPolarItem::Legend, false );

    setZ( 20.0 );
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPolarSpectrogram::~QwtPolarSpectrogram()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get runtime type information
 * @return QwtPolarItem::Rtti_PolarSpectrogram
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPolarItem::Rtti_PolarSpectrogram
 * \endif
 */
int QwtPolarSpectrogram::rtti() const
{
    return QwtPolarItem::Rtti_PolarSpectrogram;
}

/**
 * \if ENGLISH
 * @brief Set the data to be displayed
 * @param[in] data Spectrogram Data
 * @sa data()
 * @warning QwtRasterData::initRaster() is called each time before the image is rendered,
 *          but without any useful parameters. Also QwtRasterData::rasterHint() is not used.
 * \endif
 *
 * \if CHINESE
 * @brief 设置要显示的数据
 * @param[in] data 光谱图数据
 * @sa data()
 * @warning 每次渲染图像前都会调用 QwtRasterData::initRaster()，但没有传入有用的参数。
 *          同时 QwtRasterData::rasterHint() 不会被使用。
 * \endif
 */
void QwtPolarSpectrogram::setData( QwtRasterData* data )
{
    if ( data != m_data->data )
    {
        delete m_data->data;
        m_data->data = data;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the spectrogram data
 * @return Spectrogram data
 * @sa setData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取光谱图数据
 * @return 光谱图数据
 * @sa setData()
 * \endif
 */
const QwtRasterData* QwtPolarSpectrogram::data() const
{
    return m_data->data;
}

/**
 * \if ENGLISH
 * @brief Change the color map
 * @details Often it is useful to display the mapping between intensities and colors
 *          as an additional plot axis, showing a color bar.
 * @param[in] colorMap Color Map
 * @sa colorMap(), QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * \endif
 *
 * \if CHINESE
 * @brief 更改颜色映射
 * @details 通常将强度和颜色之间的映射显示为附加的绘图轴（颜色条）是有用的。
 * @param[in] colorMap 颜色映射
 * @sa colorMap(), QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * \endif
 */
void QwtPolarSpectrogram::setColorMap( QwtColorMap* colorMap )
{
    if ( m_data->colorMap != colorMap )
    {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color Map
 * @sa setColorMap()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于将强度值映射到颜色的颜色映射
 * @return 颜色映射
 * @sa setColorMap()
 * \endif
 */
const QwtColorMap* QwtPolarSpectrogram::colorMap() const
{
    return m_data->colorMap;
}

/**
 * \if ENGLISH
 * @brief Specify an attribute how to draw the curve
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 指定绘制曲线的属性
 * @param[in] attribute 绘制属性
 * @param[in] on 开启/关闭
 * @sa testPaintAttribute()
 * \endif
 */
void QwtPolarSpectrogram::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/**
 * \if ENGLISH
 * @brief Test a paint attribute
 * @param[in] attribute Paint attribute
 * @return True when attribute has been set
 * @sa setPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试绘制属性
 * @param[in] attribute 绘制属性
 * @return 属性已设置时返回 true
 * @sa setPaintAttribute()
 * \endif
 */
bool QwtPolarSpectrogram::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/**
 * \if ENGLISH
 * @brief Draw the spectrogram
 * @param[in] painter Painter
 * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param[in] radialMap Maps radius values into painter coordinates
 * @param[in] pole Position of the pole in painter coordinates
 * @param[in] radius Radius of the complete plot area in painter coordinates
 * @param[in] canvasRect Contents rect of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制光谱图
 * @param[in] painter 绘制器
 * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
 * @param[in] radialMap 将半径值映射到绘制器坐标
 * @param[in] pole 绘制器坐标中极点的位置
 * @param[in] radius 绘制器坐标中完整绘图区域的半径
 * @param[in] canvasRect 绘制器坐标中画布的内容矩形
 * \endif
 */
void QwtPolarSpectrogram::draw( QPainter* painter,
    const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
    const QPointF& pole, double,
    const QRectF& canvasRect ) const
{
    const QRectF plotRect = plot()->plotRect( canvasRect.toRect() );
    QRect imageRect = canvasRect.toRect();

    painter->save();

    painter->setClipRect( canvasRect );

    QPainterPath clipPathCanvas;
    clipPathCanvas.addEllipse( plotRect );
    painter->setClipPath( clipPathCanvas, Qt::IntersectClip );

    imageRect &= plotRect.toAlignedRect(); // outer rect

    const QwtInterval radialInterval = boundingInterval( QwtPolar::ScaleRadius );
    if ( radialInterval.isValid() )
    {
        const double radius = radialMap.transform( radialInterval.maxValue() ) -
            radialMap.transform( radialInterval.minValue() );

        QRectF clipRect( 0, 0, 2 * radius, 2 * radius );
        clipRect.moveCenter( pole );

        imageRect &= clipRect.toRect(); // inner rect, we don't have points outside

        QPainterPath clipPathRadial;
        clipPathRadial.addEllipse( clipRect );
        painter->setClipPath( clipPathRadial, Qt::IntersectClip );
    }

    const QImage image = renderImage( azimuthMap, radialMap, pole, imageRect );
    painter->drawImage( imageRect, image );

    painter->restore();
}

/*!
   \brief Render an image from the data and color map.

   The area is translated into a rect of the paint device.
   For each pixel of this rect the intensity is mapped
   into a color.

   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param rect Target rectangle of the image in painter coordinates

   \return A QImage::Format_Indexed8 or QImage::Format_ARGB32 depending
           on the color map.

   \sa QwtRasterData::intensity(), QwtColorMap::rgb(),
       QwtColorMap::colorIndex()
 */
QImage QwtPolarSpectrogram::renderImage(
    const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
    const QPointF& pole, const QRect& rect ) const
{
    if ( m_data->data == nullptr || m_data->colorMap == nullptr )
        return QImage();

    QImage image( rect.size(), m_data->colorMap->format() == QwtColorMap::RGB
                  ? QImage::Format_ARGB32 : QImage::Format_Indexed8 );

    const QwtInterval intensityRange = m_data->data->interval( Qt::ZAxis );
    if ( !intensityRange.isValid() )
        return image;

    if ( m_data->colorMap->format() == QwtColorMap::Indexed )
        image.setColorTable( m_data->colorMap->colorTable256() );

    /*
       For the moment we only announce the composition of the image by
       calling initRaster(), but we don't pass any useful parameters.
       ( How to map rect into something, that is useful to initialize a matrix
       of values in polar coordinates ? )
     */
    m_data->data->initRaster( QRectF(), QSize() );


#if !defined( QT_NO_QFUTURE )
    uint numThreads = renderThreadCount();

    if ( numThreads <= 0 )
        numThreads = QThread::idealThreadCount();

    if ( numThreads <= 0 )
        numThreads = 1;

    const int numRows = rect.height() / numThreads;


    QVector< TileInfo > tileInfos;
    for ( uint i = 0; i < numThreads; i++ )
    {
        QRect tile( rect.x(), rect.y() + i * numRows, rect.width(), numRows );
        if ( i == numThreads - 1 )
            tile.setHeight( rect.height() - i * numRows );

        TileInfo tileInfo;
        tileInfo.imagePos = rect.topLeft();
        tileInfo.rect = tile;
        tileInfo.image = &image;

        tileInfos += tileInfo;
    }

    QVector< QFuture< void > > futures;
    for ( int i = 0; i < tileInfos.size(); i++ )
    {
        if ( i == tileInfos.size() - 1 )
        {
            renderTileInfo( azimuthMap, radialMap, pole, &tileInfos[i] );
        }
        else
        {
            futures += QtConcurrent::run(
#if QT_VERSION >= 0x060000
                &QwtPolarSpectrogram::renderTileInfo, this,
#else
                this, &QwtPolarSpectrogram::renderTileInfo,
#endif
                azimuthMap, radialMap, pole, &tileInfos[i] );
        }
    }

    for ( int i = 0; i < futures.size(); i++ )
        futures[i].waitForFinished();

#else
    renderTile( azimuthMap, radialMap, pole, rect.topLeft(), rect, &image );
#endif

    m_data->data->discardRaster();

    return image;
}

void QwtPolarSpectrogram::renderTileInfo(
    const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
    const QPointF& pole, TileInfo* tileInfo ) const
{
    renderTile( azimuthMap, radialMap, pole,
        tileInfo->imagePos, tileInfo->rect, tileInfo->image );
}

/*!
   \brief Render a sub-rectangle of an image

   renderTile() is called by renderImage() to render different parts
   of the image by concurrent threads.

   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param imagePos Top/left position of the image in painter coordinates
   \param tile Sub-rectangle of the tile in painter coordinates
   \param image Image to be rendered

   \sa setRenderThreadCount()
   \note renderTile needs to be reentrant
 */
void QwtPolarSpectrogram::renderTile(
    const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
    const QPointF& pole, const QPoint& imagePos,
    const QRect& tile, QImage* image ) const
{
    const QwtInterval intensityRange = m_data->data->interval( Qt::ZAxis );
    if ( !intensityRange.isValid() )
        return;

    const bool doFastAtan = testPaintAttribute( ApproximatedAtan );

    const int y0 = imagePos.y();
    const int y1 = tile.top();
    const int y2 = tile.bottom();

    const int x0 = imagePos.x();
    const int x1 = tile.left();
    const int x2 = tile.right();

    if ( m_data->colorMap->format() == QwtColorMap::RGB )
    {
        for ( int y = y1; y <= y2; y++ )
        {
            const double dy = pole.y() - y;
            const double dy2 = qwtSqr( dy );

            QRgb* line = reinterpret_cast< QRgb* >( image->scanLine( y - y0 ) );
            line += x1 - x0;

            for ( int x = x1; x <= x2; x++ )
            {
                const double dx = x - pole.x();

                double a = doFastAtan ? qwtFastAtan2( dy, dx ) : qAtan2( dy, dx );

                if ( a < 0.0 )
                    a += 2 * M_PI;

                if ( a < azimuthMap.p1() )
                    a += 2 * M_PI;

                const double r = qSqrt( qwtSqr( dx ) + dy2 );

                const double azimuth = azimuthMap.invTransform( a );
                const double radius = radialMap.invTransform( r );

                const double value = m_data->data->value( azimuth, radius );
                if ( qIsNaN( value ) )
                {
                    *line++ = 0u;
                }
                else
                {
                    *line++ = m_data->colorMap->rgb( intensityRange, value );
                }
            }
        }
    }
    else if ( m_data->colorMap->format() == QwtColorMap::Indexed )
    {
        for ( int y = y1; y <= y2; y++ )
        {
            const double dy = pole.y() - y;
            const double dy2 = qwtSqr( dy );

            unsigned char* line = image->scanLine( y - y0 );
            line += x1 - x0;
            for ( int x = x1; x <= x2; x++ )
            {
                const double dx = x - pole.x();

                double a = doFastAtan ? qwtFastAtan2( dy, dx ) : qAtan2( dy, dx );
                if ( a < 0.0 )
                    a += 2 * M_PI;
                if ( a < azimuthMap.p1() )
                    a += 2 * M_PI;

                const double r = qSqrt( qwtSqr( dx ) + dy2 );

                const double azimuth = azimuthMap.invTransform( a );
                const double radius = radialMap.invTransform( r );

                const double value = m_data->data->value( azimuth, radius );

                const uint index = m_data->colorMap->colorIndex( 256, intensityRange, value );
                *line++ = static_cast< unsigned char >( index );
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval for a scale
 * @details This interval can be useful for operations like clipping or autoscaling.
 * @param[in] scaleId Scale index
 * @return Bounding interval ( == position )
 * @sa position()
 * \endif
 *
 * \if CHINESE
 * @brief 获取指定刻度的边界区间
 * @details 此区间可用于裁剪或自动缩放等操作。
 * @param[in] scaleId 刻度索引
 * @return 边界区间（== 位置）
 * @sa position()
 * \endif
 */
QwtInterval QwtPolarSpectrogram::boundingInterval( int scaleId ) const
{
    if ( scaleId == QwtPolar::ScaleRadius )
        return m_data->data->interval( Qt::YAxis );

    return QwtPolarItem::boundingInterval( scaleId );
}
