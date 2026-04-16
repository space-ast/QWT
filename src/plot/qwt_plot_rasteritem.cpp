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

#include "qwt_plot_rasteritem.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_interval.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpaintengine.h>
#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>

#include <limits>

class QwtPlotRasterItem::PrivateData
{
  public:
    PrivateData()
        : alpha( -1 )
        , paintAttributes( QwtPlotRasterItem::PaintInDeviceResolution )
    {
        cache.policy = QwtPlotRasterItem::NoCache;
    }

    int alpha;

    QwtPlotRasterItem::PaintAttributes paintAttributes;

    struct ImageCache
    {
        QwtPlotRasterItem::CachePolicy policy;
        QRectF area;
        QSizeF size;
        QImage image;
    } cache;
};


static QRectF qwtAlignRect(const QRectF& rect)
{
    QRectF r;
    r.setLeft( qRound( rect.left() ) );
    r.setRight( qRound( rect.right() ) );
    r.setTop( qRound( rect.top() ) );
    r.setBottom( qRound( rect.bottom() ) );

    return r;
}

static QRectF qwtStripRect(const QRectF& rect, const QRectF& area,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtInterval& xInterval, const QwtInterval& yInterval)
{
    QRectF r = rect;
    if ( xInterval.borderFlags() & QwtInterval::ExcludeMinimum )
    {
        if ( area.left() <= xInterval.minValue() )
        {
            if ( xMap.isInverting() )
                r.adjust(0, 0, -1, 0);
            else
                r.adjust(1, 0, 0, 0);
        }
    }

    if ( xInterval.borderFlags() & QwtInterval::ExcludeMaximum )
    {
        if ( area.right() >= xInterval.maxValue() )
        {
            if ( xMap.isInverting() )
                r.adjust(1, 0, 0, 0);
            else
                r.adjust(0, 0, -1, 0);
        }
    }

    if ( yInterval.borderFlags() & QwtInterval::ExcludeMinimum )
    {
        if ( area.top() <= yInterval.minValue() )
        {
            if ( yMap.isInverting() )
                r.adjust(0, 0, 0, -1);
            else
                r.adjust(0, 1, 0, 0);
        }
    }

    if ( yInterval.borderFlags() & QwtInterval::ExcludeMaximum )
    {
        if ( area.bottom() >= yInterval.maxValue() )
        {
            if ( yMap.isInverting() )
                r.adjust(0, 1, 0, 0);
            else
                r.adjust(0, 0, 0, -1);
        }
    }

    return r;
}

static QImage qwtExpandImage(const QImage& image,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& area, const QRectF& area2, const QRectF& paintRect,
    const QwtInterval& xInterval, const QwtInterval& yInterval )
{
    const QRectF strippedRect = qwtStripRect(paintRect, area2,
        xMap, yMap, xInterval, yInterval);
    const QSize sz = strippedRect.toRect().size();

    const int w = image.width();
    const int h = image.height();

    const QRectF r = QwtScaleMap::transform(xMap, yMap, area).normalized();
    const double pw = ( r.width() - 1 ) / w;
    const double ph = ( r.height() - 1 ) / h;

    double px0, py0;
    if ( !xMap.isInverting() )
    {
        px0 = xMap.transform( area2.left() );
        px0 = qRound( px0 );
        px0 = px0 - xMap.transform( area.left() );
    }
    else
    {
        px0 = xMap.transform( area2.right() );
        px0 = qRound( px0 );
        px0 -= xMap.transform( area.right() );

        px0 -= 1.0;
    }
    px0 += strippedRect.left() - paintRect.left();

    if ( !yMap.isInverting() )
    {
        py0 = yMap.transform( area2.top() );
        py0 = qRound( py0 );
        py0 -= yMap.transform( area.top() );
    }
    else
    {
        py0 = yMap.transform( area2.bottom() );
        py0 = qRound( py0 );
        py0 -= yMap.transform( area.bottom() );

        py0 -= 1.0;
    }
    py0 += strippedRect.top() - paintRect.top();

    QImage expanded( sz, image.format() );
    if ( image.format() == QImage::Format_Indexed8 )
        expanded.setColorTable( image.colorTable() );

    switch( image.depth() )
    {
        case 32:
        {
            for ( int y1 = 0; y1 < h; y1++ )
            {
                int yy1;
                if ( y1 == 0 )
                {
                    yy1 = 0;
                }
                else
                {
                    yy1 = qRound( y1 * ph - py0 );
                    if ( yy1 < 0 )
                        yy1 = 0;
                }

                int yy2;
                if ( y1 == h - 1 )
                {
                    yy2 = sz.height();
                }
                else
                {
                    yy2 = qRound( ( y1 + 1 ) * ph - py0 );
                    if ( yy2 > sz.height() )
                        yy2 = sz.height();
                }

                const quint32* line1 =
                    reinterpret_cast< const quint32* >( image.scanLine( y1 ) );

                for ( int x1 = 0; x1 < w; x1++ )
                {
                    int xx1;
                    if ( x1 == 0 )
                    {
                        xx1 = 0;
                    }
                    else
                    {
                        xx1 = qRound( x1 * pw - px0 );
                        if ( xx1 < 0 )
                            xx1 = 0;
                    }

                    int xx2;
                    if ( x1 == w - 1 )
                    {
                        xx2 = sz.width();
                    }
                    else
                    {
                        xx2 = qRound( ( x1 + 1 ) * pw - px0 );
                        if ( xx2 > sz.width() )
                            xx2 = sz.width();
                    }

                    const quint32 rgb( line1[x1] );
                    for ( int y2 = yy1; y2 < yy2; y2++ )
                    {
                        quint32* line2 = reinterpret_cast< quint32* >(
                            expanded.scanLine( y2 ) );

                        for ( int x2 = xx1; x2 < xx2; x2++ )
                            line2[x2] = rgb;
                    }
                }
            }
            break;
        }
        case 8:
        {
            for ( int y1 = 0; y1 < h; y1++ )
            {
                int yy1;
                if ( y1 == 0 )
                {
                    yy1 = 0;
                }
                else
                {
                    yy1 = qRound( y1 * ph - py0 );
                    if ( yy1 < 0 )
                        yy1 = 0;
                }

                int yy2;
                if ( y1 == h - 1 )
                {
                    yy2 = sz.height();
                }
                else
                {
                    yy2 = qRound( ( y1 + 1 ) * ph - py0 );
                    if ( yy2 > sz.height() )
                        yy2 = sz.height();
                }

                const uchar* line1 = image.scanLine( y1 );

                for ( int x1 = 0; x1 < w; x1++ )
                {
                    int xx1;
                    if ( x1 == 0 )
                    {
                        xx1 = 0;
                    }
                    else
                    {
                        xx1 = qRound( x1 * pw - px0 );
                        if ( xx1 < 0 )
                            xx1 = 0;
                    }

                    int xx2;
                    if ( x1 == w - 1 )
                    {
                        xx2 = sz.width();
                    }
                    else
                    {
                        xx2 = qRound( ( x1 + 1 ) * pw - px0 );
                        if ( xx2 > sz.width() )
                            xx2 = sz.width();
                    }

                    for ( int y2 = yy1; y2 < yy2; y2++ )
                    {
                        uchar* line2 = expanded.scanLine( y2 );
                        memset( line2 + xx1, line1[x1], xx2 - xx1 );
                    }
                }
            }
            break;
        }
        default:
            expanded = image;
    }

    return expanded;
}

static QRectF qwtExpandToPixels(const QRectF& rect, const QRectF& pixelRect)
{
    const double pw = pixelRect.width();
    const double ph = pixelRect.height();

    const double dx1 = pixelRect.left() - rect.left();
    const double dx2 = pixelRect.right() - rect.right();
    const double dy1 = pixelRect.top() - rect.top();
    const double dy2 = pixelRect.bottom() - rect.bottom();

    QRectF r;
    r.setLeft( pixelRect.left() - qwtCeil( dx1 / pw ) * pw );
    r.setTop( pixelRect.top() - qwtCeil( dy1 / ph ) * ph );
    r.setRight( pixelRect.right() - qwtFloor( dx2 / pw ) * pw );
    r.setBottom( pixelRect.bottom() - qwtFloor( dy2 / ph ) * ph );

    return r;
}

static void qwtTransformMaps( const QTransform& tr,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    QwtScaleMap& xxMap, QwtScaleMap& yyMap )
{
    const QPointF p1 = tr.map( QPointF( xMap.p1(), yMap.p1() ) );
    const QPointF p2 = tr.map( QPointF( xMap.p2(), yMap.p2() ) );

    xxMap = xMap;
    xxMap.setPaintInterval( p1.x(), p2.x() );

    yyMap = yMap;
    yyMap.setPaintInterval( p1.y(), p2.y() );
}

static void qwtAdjustMaps( QwtScaleMap& xMap, QwtScaleMap& yMap,
    const QRectF& area, const QRectF& paintRect)
{
    double sx1 = area.left();
    double sx2 = area.right();
    if ( xMap.isInverting() )
        qSwap(sx1, sx2);

    double sy1 = area.top();
    double sy2 = area.bottom();

    if ( yMap.isInverting() )
        qSwap(sy1, sy2);

    xMap.setPaintInterval(paintRect.left(), paintRect.right() );
    xMap.setScaleInterval(sx1, sx2);

    yMap.setPaintInterval(paintRect.top(), paintRect.bottom() );
    yMap.setScaleInterval(sy1, sy2);
}

static bool qwtUseCache( QwtPlotRasterItem::CachePolicy policy,
    const QPainter* painter )
{
    bool doCache = false;

    if ( policy == QwtPlotRasterItem::PaintCache )
    {
        // Caching doesn't make sense, when the item is
        // not painted to screen

        switch ( painter->paintEngine()->type() )
        {
            case QPaintEngine::SVG:
            case QPaintEngine::Pdf:
#if QT_VERSION < 0x060000
            case QPaintEngine::PostScript:
#endif
            case QPaintEngine::MacPrinter:
            case QPaintEngine::Picture:
                break;
            default:;
                doCache = true;
        }
    }

    return doCache;
}

static void qwtToRgba( const QImage* from, QImage* to,
    const QRect& tile, int alpha )
{
    const QRgb mask1 = qRgba( 0, 0, 0, alpha );
    const QRgb mask2 = qRgba( 255, 255, 255, 0 );
    const QRgb mask3 = qRgba( 0, 0, 0, 255 );

    const int y0 = tile.top();
    const int y1 = tile.bottom();
    const int x0 = tile.left();
    const int x1 = tile.right();

    if ( from->depth() == 8 )
    {
        for ( int y = y0; y <= y1; y++ )
        {
            QRgb* alphaLine = reinterpret_cast< QRgb* >( to->scanLine( y ) );
            const unsigned char* line = from->scanLine( y );

            for ( int x = x0; x <= x1; x++ )
                *alphaLine++ = ( from->color( *line++ ) & mask2 ) | mask1;
        }
    }
    else if ( from->depth() == 32 )
    {
        for ( int y = y0; y <= y1; y++ )
        {
            QRgb* alphaLine = reinterpret_cast< QRgb* >( to->scanLine( y ) );
            const QRgb* line = reinterpret_cast< const QRgb* >( from->scanLine( y ) );

            for ( int x = x0; x <= x1; x++ )
            {
                const QRgb rgb = *line++;
                if ( rgb & mask3 ) // alpha != 0
                    *alphaLine++ = ( rgb & mask2 ) | mask1;
                else
                    *alphaLine++ = rgb;
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the raster item
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 栅格项的标题
 * \endif
 */
QwtPlotRasterItem::QwtPlotRasterItem( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor with title
 * @param[in] title Title of the raster item
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数（带标题）
 * @param[in] title 栅格项的标题
 * \endif
 */
QwtPlotRasterItem::QwtPlotRasterItem( const QwtText& title )
    : QwtPlotItem( title )
{
    init();
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
QwtPlotRasterItem::~QwtPlotRasterItem()
{
    delete m_data;
}

void QwtPlotRasterItem::init()
{
    m_data = new PrivateData();

    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 8.0 );
}

/**
 * \if ENGLISH
 * @brief Set a paint attribute
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa PaintAttribute, testPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 设置绘制属性
 * @param[in] attribute 绘制属性
 * @param[in] on 开/关
 * @sa PaintAttribute, testPaintAttribute()
 * \endif
 */
void QwtPlotRasterItem::setPaintAttribute( PaintAttribute attribute, bool on )
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
 * @return True, when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试绘制属性
 * @param[in] attribute 绘制属性
 * @return 如果属性启用则返回 true
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 */
bool QwtPlotRasterItem::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/**
 * \if ENGLISH
 * @brief Set an alpha value for the raster data
 * @details Often a plot has several types of raster data organized in layers.
 *          (e.g. a geographical map, with weather statistics).
 *          Using setAlpha() raster items can be stacked easily.
 *          The alpha value is a value [0, 255] to control the transparency of the image.
 *          0 represents a fully transparent color, while 255 represents a fully opaque color.
 *          - alpha >= 0: All alpha values of the pixels returned by renderImage() will be set to alpha,
 *            beside those with an alpha value of 0 (invalid pixels).
 *          - alpha < 0: The alpha values returned by renderImage() are not changed.
 *          The default alpha value is -1.
 * @param[in] alpha Alpha value
 * @sa alpha()
 * \endif
 *
 * \if CHINESE
 * @brief 设置栅格数据的 alpha 值
 * @details 通常，一个绘图会有几种类型的栅格数据组织在层中。
 *          （例如带有天气统计数据的地理地图）。
 *          使用 setAlpha() 可以轻松堆叠栅格项。
 *          alpha 值是 [0, 255] 的值，用于控制图像的透明度。
 *          0 表示完全透明的颜色，255 表示完全不透明的颜色。
 *          - alpha >= 0: renderImage() 返回的像素的所有 alpha 值都将设置为 alpha，
 *            除了 alpha 值为 0 的像素（无效像素）。
 *          - alpha < 0: renderImage() 返回的 alpha 值不会改变。
 *          默认 alpha 值为 -1。
 * @param[in] alpha Alpha 值
 * @sa alpha()
 * \endif
 */
void QwtPlotRasterItem::setAlpha( int alpha )
{
    if ( alpha < 0 )
        alpha = -1;

    if ( alpha > 255 )
        alpha = 255;

    if ( alpha != m_data->alpha )
    {
        m_data->alpha = alpha;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the alpha value of the raster item
 * @return Alpha value of the raster item
 * @sa setAlpha()
 * \endif
 *
 * \if CHINESE
 * @brief 获取栅格项的 alpha 值
 * @return 栅格项的 alpha 值
 * @sa setAlpha()
 * \endif
 */
int QwtPlotRasterItem::alpha() const
{
    return m_data->alpha;
}

/**
 * \if ENGLISH
 * @brief Change the cache policy
 * @details The default policy is NoCache
 * @param[in] policy Cache policy
 * @sa CachePolicy, cachePolicy()
 * \endif
 *
 * \if CHINESE
 * @brief 更改缓存策略
 * @details 默认策略是 NoCache
 * @param[in] policy 缓存策略
 * @sa CachePolicy, cachePolicy()
 * \endif
 */
void QwtPlotRasterItem::setCachePolicy(
    QwtPlotRasterItem::CachePolicy policy )
{
    if ( m_data->cache.policy != policy )
    {
        m_data->cache.policy = policy;

        invalidateCache();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the cache policy
 * @return Cache policy
 * @sa CachePolicy, setCachePolicy()
 * \endif
 *
 * \if CHINESE
 * @brief 获取缓存策略
 * @return 缓存策略
 * @sa CachePolicy, setCachePolicy()
 * \endif
 */
QwtPlotRasterItem::CachePolicy QwtPlotRasterItem::cachePolicy() const
{
    return m_data->cache.policy;
}

/**
 * \if ENGLISH
 * @brief Invalidate the paint cache
 * @sa setCachePolicy()
 * \endif
 *
 * \if CHINESE
 * @brief 使绘制缓存失效
 * @sa setCachePolicy()
 * \endif
 */
void QwtPlotRasterItem::invalidateCache()
{
    m_data->cache.image = QImage();
    m_data->cache.area = QRect();
    m_data->cache.size = QSize();
}

/**
 * \if ENGLISH
 * @brief Pixel hint
 * @details The geometry of a pixel is used to calculate the resolution and
 *          alignment of the rendered image.
 *          Width and height of the hint need to be the horizontal and vertical
 *          distances between 2 neighboring points. The center of the hint has to be
 *          the position of any point (it doesn't matter which one).
 *          Limiting the resolution of the image might significantly improve the performance
 *          and heavily reduce the amount of memory when rendering a QImage from the raster data.
 *          The default implementation returns an empty rectangle (QRectF()), meaning that
 *          the image will be rendered in target device (e.g. screen) resolution.
 * @param[in] area In most implementations the resolution of the data doesn't depend on the requested area
 * @return Bounding rectangle of a pixel
 * @sa render(), renderImage()
 * \endif
 *
 * \if CHINESE
 * @brief 像素提示
 * @details 像素的几何形状用于计算渲染图像的分辨率和对齐方式。
 *          提示的宽度和高度需要是两个相邻点之间的水平和垂直距离。
 *          提示的中心必须是任何点的位置（不重要哪个点）。
 *          限制图像的分辨率可能会显著提高性能，并大大减少从栅格数据渲染 QImage 时的内存使用量。
 *          默认实现返回空矩形（QRectF()），意味着图像将以目标设备（如屏幕）分辨率渲染。
 * @param[in] area 在大多数实现中，数据的分辨率不依赖于请求的区域
 * @return 像素的边界矩形
 * @sa render(), renderImage()
 * \endif
 */
QRectF QwtPlotRasterItem::pixelHint( const QRectF& area ) const
{
    Q_UNUSED( area );
    return QRectF();
}

/**
 * \if ENGLISH
 * @brief Draw the raster data
 * @param[in] painter Painter
 * @param[in] xMap X-Scale Map
 * @param[in] yMap Y-Scale Map
 * @param[in] canvasRect Contents rectangle of the plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 绘制栅格数据
 * @param[in] painter 画笔
 * @param[in] xMap X 比例尺映射
 * @param[in] yMap Y 比例尺映射
 * @param[in] canvasRect 绘图画布的内容矩形
 * \endif
 */
void QwtPlotRasterItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    if ( canvasRect.isEmpty() || m_data->alpha == 0 )
        return;

    const bool doCache = qwtUseCache( m_data->cache.policy, painter );

    const QwtInterval xInterval = interval( Qt::XAxis );
    const QwtInterval yInterval = interval( Qt::YAxis );

    /*
        Scaling an image always results in a loss of
        precision/quality. So we always render the image in
        paint device resolution.
     */

    QwtScaleMap xxMap, yyMap;
    qwtTransformMaps( painter->transform(), xMap, yMap, xxMap, yyMap );

    QRectF paintRect = painter->transform().mapRect( canvasRect );
    QRectF area = QwtScaleMap::invTransform( xxMap, yyMap, paintRect );

    const QRectF br = boundingRect();
    if ( br.isValid() && !br.contains( area ) )
    {
        area &= br;
        if ( !area.isValid() )
            return;

        paintRect = QwtScaleMap::transform( xxMap, yyMap, area );
    }

    QRectF imageRect;
    QImage image;

    QRectF pixelRect = pixelHint(area);
    if ( !pixelRect.isEmpty() )
    {
        // one pixel of the target device in plot coordinates
        const double dx = qAbs( xxMap.invTransform( 1 ) - xxMap.invTransform( 0 ) );
        const double dy = qAbs( yyMap.invTransform( 1 ) - yyMap.invTransform( 0 ) );

        if ( dx > pixelRect.width() && dy > pixelRect.height() )
        {
            /*
               When the resolution of the data pixels is higher than
               the resolution of the target device we render in
               target device resolution.
             */
            pixelRect = QRectF();
        }
        else
        {
            /*
               If only one dimension is of the data pixel is higher
               we expand the pixel rect to the resolution of the target device.
             */

            if ( dx > pixelRect.width() )
                pixelRect.setWidth( dx );

            if ( dy > pixelRect.height() )
                pixelRect.setHeight( dy );
        }
    }

    if ( pixelRect.isEmpty() )
    {
        if ( QwtPainter::roundingAlignment( painter ) )
        {
            // we want to have maps, where the boundaries of
            // the aligned paint rectangle exactly match the area

            paintRect = qwtAlignRect(paintRect);
            qwtAdjustMaps(xxMap, yyMap, area, paintRect);
        }

        // When we have no information about position and size of
        // data pixels we render in resolution of the paint device.

        image = compose(xxMap, yyMap,
            area, paintRect, paintRect.size().toSize(), doCache);
        if ( image.isNull() )
            return;

        // Remove pixels at the boundaries, when explicitly
        // excluded in the intervals

        imageRect = qwtStripRect(paintRect, area,
            xxMap, yyMap, xInterval, yInterval);

        if ( imageRect != paintRect )
        {
            const QRect r(
                qRound( imageRect.x() - paintRect.x() ),
                qRound( imageRect.y() - paintRect.y() ),
                qRound( imageRect.width() ),
                qRound( imageRect.height() ) );

            image = image.copy(r);
        }
    }
    else
    {
        if ( QwtPainter::roundingAlignment( painter ) )
            paintRect = qwtAlignRect(paintRect);

        // align the area to the data pixels
        QRectF imageArea = qwtExpandToPixels(area, pixelRect);

        if ( imageArea.right() == xInterval.maxValue() &&
            !( xInterval.borderFlags() & QwtInterval::ExcludeMaximum ) )
        {
            imageArea.adjust(0, 0, pixelRect.width(), 0);
        }
        if ( imageArea.bottom() == yInterval.maxValue() &&
            !( yInterval.borderFlags() & QwtInterval::ExcludeMaximum ) )
        {
            imageArea.adjust(0, 0, 0, pixelRect.height() );
        }

        QSize imageSize;
        imageSize.setWidth( qRound( imageArea.width() / pixelRect.width() ) );
        imageSize.setHeight( qRound( imageArea.height() / pixelRect.height() ) );

        image = compose(xxMap, yyMap,
            imageArea, paintRect, imageSize, doCache );

        if ( image.isNull() )
            return;

        imageRect = qwtStripRect(paintRect, area,
            xxMap, yyMap, xInterval, yInterval);

        if ( ( image.width() > 1 || image.height() > 1 ) &&
            testPaintAttribute( PaintInDeviceResolution ) )
        {
            // Because of rounding errors the pixels
            // need to be expanded manually to rectangles of
            // different sizes

            image = qwtExpandImage(image, xxMap, yyMap,
                imageArea, area, paintRect, xInterval, yInterval );
        }
    }

    painter->save();
    painter->setWorldTransform( QTransform() );

    QwtPainter::drawImage( painter, imageRect, image );

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval for an axis
 * @details This method is intended to be reimplemented by derived classes.
 *          The default implementation returns an invalid interval.
 * @param[in] axis X, Y, or Z axis
 * @return Bounding interval for an axis
 * \endif
 *
 * \if CHINESE
 * @brief 获取轴的边界区间
 * @details 此方法旨在由派生类重新实现。
 *          默认实现返回无效区间。
 * @param[in] axis X、Y 或 Z 轴
 * @return 轴的边界区间
 * \endif
 */
QwtInterval QwtPlotRasterItem::interval(Qt::Axis axis) const
{
    Q_UNUSED( axis );
    return QwtInterval();
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle of the data
 * @return Bounding rectangle of the data
 * @sa QwtPlotRasterItem::interval()
 * \endif
 *
 * \if CHINESE
 * @brief 获取数据的边界矩形
 * @return 数据的边界矩形
 * @sa QwtPlotRasterItem::interval()
 * \endif
 */
QRectF QwtPlotRasterItem::boundingRect() const
{
    const QwtInterval intervalX = interval( Qt::XAxis );
    const QwtInterval intervalY = interval( Qt::YAxis );

    if ( !intervalX.isValid() && !intervalY.isValid() )
        return QRectF(); // no bounding rect

    QRectF r;

    if ( intervalX.isValid() )
    {
        r.setLeft( intervalX.minValue() );
        r.setRight( intervalX.maxValue() );
    }
    else
    {
        const qreal max = std::numeric_limits< float >::max();

        r.setLeft( -0.5 * max );
        r.setWidth( max );
    }

    if ( intervalY.isValid() )
    {
        r.setTop( intervalY.minValue() );
        r.setBottom( intervalY.maxValue() );
    }
    else
    {
        const qreal max = std::numeric_limits< float >::max();

        r.setTop( -0.5 * max );
        r.setHeight( max );
    }

    return r.normalized();
}

QImage QwtPlotRasterItem::compose(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& imageArea, const QRectF& paintRect,
    const QSize& imageSize, bool doCache) const
{
    QImage image;
    if ( imageArea.isEmpty() || paintRect.isEmpty() || imageSize.isEmpty() )
        return image;

    if ( doCache )
    {
        if ( !m_data->cache.image.isNull()
            && m_data->cache.area == imageArea
            && m_data->cache.size == paintRect.size() )
        {
            image = m_data->cache.image;
        }
    }

    if ( image.isNull() )
    {
        double dx = 0.0;
        if ( paintRect.toRect().width() > imageSize.width() )
            dx = imageArea.width() / imageSize.width();

        const QwtScaleMap xxMap =
            imageMap(Qt::Horizontal, xMap, imageArea, imageSize, dx);

        double dy = 0.0;
        if ( paintRect.toRect().height() > imageSize.height() )
            dy = imageArea.height() / imageSize.height();

        const QwtScaleMap yyMap =
            imageMap(Qt::Vertical, yMap, imageArea, imageSize, dy);

        image = renderImage( xxMap, yyMap, imageArea, imageSize );

        if ( doCache )
        {
            m_data->cache.area = imageArea;
            m_data->cache.size = paintRect.size();
            m_data->cache.image = image;
        }
    }

    if ( m_data->alpha >= 0 && m_data->alpha < 255 )
    {
        QImage alphaImage( image.size(), QImage::Format_ARGB32 );

#if !defined( QT_NO_QFUTURE )
        uint numThreads = renderThreadCount();

        if ( numThreads <= 0 )
            numThreads = QThread::idealThreadCount();

        if ( numThreads <= 0 )
            numThreads = 1;

        const int numRows = image.height() / numThreads;

        QVector< QFuture< void > > futures;
        futures.reserve( numThreads - 1 );

        for ( uint i = 0; i < numThreads; i++ )
        {
            QRect tile( 0, i * numRows, image.width(), numRows );
            if ( i == numThreads - 1 )
            {
                tile.setHeight( image.height() - i * numRows );
                qwtToRgba( &image, &alphaImage, tile, m_data->alpha );
            }
            else
            {
                futures += QtConcurrent::run(
                    &qwtToRgba, &image, &alphaImage, tile, m_data->alpha );
            }
        }
        for ( int i = 0; i < futures.size(); i++ )
            futures[i].waitForFinished();
#else
        const QRect tile( 0, 0, image.width(), image.height() );
        qwtToRgba( &image, &alphaImage, tile, m_data->alpha );
#endif
        image = alphaImage;
    }

    return image;
}

/*!
   \brief Calculate a scale map for painting to an image

   \param orientation Orientation, Qt::Horizontal means a X axis
   \param map Scale map for rendering the plot item
   \param area Area to be painted on the image
   \param imageSize Image size
   \param pixelSize Width/Height of a data pixel

   \return Calculated scale map
 */
QwtScaleMap QwtPlotRasterItem::imageMap(
    Qt::Orientation orientation,
    const QwtScaleMap& map, const QRectF& area,
    const QSize& imageSize, double pixelSize) const
{
    double p1, p2, s1, s2;

    if ( orientation == Qt::Horizontal )
    {
        p1 = 0.0;
        p2 = imageSize.width();
        s1 = area.left();
        s2 = area.right();
    }
    else
    {
        p1 = 0.0;
        p2 = imageSize.height();
        s1 = area.top();
        s2 = area.bottom();
    }

    if ( pixelSize > 0.0 || p2 == 1.0 )
    {
        double off = 0.5 * pixelSize;
        if ( map.isInverting() )
            off = -off;

        s1 += off;
        s2 += off;
    }
    else
    {
        p2--;
    }

    if ( map.isInverting() && ( s1 < s2 ) )
        qSwap( s1, s2 );

    QwtScaleMap newMap = map;
    newMap.setPaintInterval( p1, p2 );
    newMap.setScaleInterval( s1, s2 );

    return newMap;
}
