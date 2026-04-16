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

#include "qwt_null_paintdevice.h"
#include <qpaintengine.h>
#include <qpainterpath.h>

class QwtNullPaintDevice::PrivateData
{
  public:
    PrivateData():
        mode( QwtNullPaintDevice::NormalMode )
    {
    }

    QwtNullPaintDevice::Mode mode;
};

class QwtNullPaintDevice::PaintEngine final : public QPaintEngine
{
  public:
    PaintEngine();

    virtual bool begin( QPaintDevice* ) override;
    virtual bool end() override;

    virtual Type type () const override;
    virtual void updateState(const QPaintEngineState&) override;

    virtual void drawRects(const QRect*, int ) override;
    virtual void drawRects(const QRectF*, int ) override;

    virtual void drawLines(const QLine*, int ) override;
    virtual void drawLines(const QLineF*, int ) override;

    virtual void drawEllipse(const QRectF&) override;
    virtual void drawEllipse(const QRect&) override;

    virtual void drawPath(const QPainterPath&) override;

    virtual void drawPoints(const QPointF*, int ) override;
    virtual void drawPoints(const QPoint*, int ) override;

    virtual void drawPolygon(
        const QPointF*, int, PolygonDrawMode ) override;

    virtual void drawPolygon(
        const QPoint*, int, PolygonDrawMode ) override;

    virtual void drawPixmap(const QRectF&,
        const QPixmap&, const QRectF&) override;

    virtual void drawTextItem(
        const QPointF&, const QTextItem&) override;

    virtual void drawTiledPixmap(const QRectF&,
        const QPixmap&, const QPointF& s) override;

    virtual void drawImage(const QRectF&, const QImage&,
        const QRectF&, Qt::ImageConversionFlags ) override;

  private:
    QwtNullPaintDevice* nullDevice();
};

QwtNullPaintDevice::PaintEngine::PaintEngine()
    : QPaintEngine( QPaintEngine::AllFeatures )
{
}

bool QwtNullPaintDevice::PaintEngine::begin( QPaintDevice* )
{
    setActive( true );
    return true;
}

bool QwtNullPaintDevice::PaintEngine::end()
{
    setActive( false );
    return true;
}

QPaintEngine::Type QwtNullPaintDevice::PaintEngine::type() const
{
    /*
        How to avoid conflicts with other 3rd party pain engines ?
        At least we don't use QPaintEngine::User what is known to
        be the value of some print engines
     */
    return static_cast< QPaintEngine::Type >( QPaintEngine::MaxUser - 2 );
}

void QwtNullPaintDevice::PaintEngine::drawRects(
    const QRect* rects, int rectCount)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawRects( rects, rectCount );
        return;
    }

    device->drawRects( rects, rectCount );
}

void QwtNullPaintDevice::PaintEngine::drawRects(
    const QRectF* rects, int rectCount)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawRects( rects, rectCount );
        return;
    }

    device->drawRects( rects, rectCount );
}

void QwtNullPaintDevice::PaintEngine::drawLines(
    const QLine* lines, int lineCount)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawLines( lines, lineCount );
        return;
    }

    device->drawLines( lines, lineCount );
}

void QwtNullPaintDevice::PaintEngine::drawLines(
    const QLineF* lines, int lineCount)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawLines( lines, lineCount );
        return;
    }

    device->drawLines( lines, lineCount );
}

void QwtNullPaintDevice::PaintEngine::drawEllipse(
    const QRectF& rect)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawEllipse( rect );
        return;
    }

    device->drawEllipse( rect );
}

void QwtNullPaintDevice::PaintEngine::drawEllipse(
    const QRect& rect)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawEllipse( rect );
        return;
    }

    device->drawEllipse( rect );
}


void QwtNullPaintDevice::PaintEngine::drawPath(
    const QPainterPath& path)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    device->drawPath( path );
}

void QwtNullPaintDevice::PaintEngine::drawPoints(
    const QPointF* points, int pointCount)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawPoints( points, pointCount );
        return;
    }

    device->drawPoints( points, pointCount );
}

void QwtNullPaintDevice::PaintEngine::drawPoints(
    const QPoint* points, int pointCount)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawPoints( points, pointCount );
        return;
    }

    device->drawPoints( points, pointCount );
}

void QwtNullPaintDevice::PaintEngine::drawPolygon(
    const QPointF* points, int pointCount, PolygonDrawMode mode)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() == QwtNullPaintDevice::PathMode )
    {
        QPainterPath path;

        if ( pointCount > 0 )
        {
            path.moveTo( points[0] );
            for ( int i = 1; i < pointCount; i++ )
                path.lineTo( points[i] );

            if ( mode != PolylineMode )
                path.closeSubpath();
        }

        device->drawPath( path );
        return;
    }

    device->drawPolygon( points, pointCount, mode );
}

void QwtNullPaintDevice::PaintEngine::drawPolygon(
    const QPoint* points, int pointCount, PolygonDrawMode mode)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() == QwtNullPaintDevice::PathMode )
    {
        QPainterPath path;

        if ( pointCount > 0 )
        {
            path.moveTo( points[0] );
            for ( int i = 1; i < pointCount; i++ )
                path.lineTo( points[i] );

            if ( mode != PolylineMode )
                path.closeSubpath();
        }

        device->drawPath( path );
        return;
    }

    device->drawPolygon( points, pointCount, mode );
}

void QwtNullPaintDevice::PaintEngine::drawPixmap(
    const QRectF& rect, const QPixmap& pm, const QRectF& subRect )
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    device->drawPixmap( rect, pm, subRect );
}

void QwtNullPaintDevice::PaintEngine::drawTextItem(
    const QPointF& pos, const QTextItem& textItem)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawTextItem( pos, textItem );
        return;
    }

    device->drawTextItem( pos, textItem );
}

void QwtNullPaintDevice::PaintEngine::drawTiledPixmap(
    const QRectF& rect, const QPixmap& pixmap,
    const QPointF& subRect)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    if ( device->mode() != QwtNullPaintDevice::NormalMode )
    {
        QPaintEngine::drawTiledPixmap( rect, pixmap, subRect );
        return;
    }

    device->drawTiledPixmap( rect, pixmap, subRect );
}

void QwtNullPaintDevice::PaintEngine::drawImage(
    const QRectF& rect, const QImage& image,
    const QRectF& subRect, Qt::ImageConversionFlags flags)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    device->drawImage( rect, image, subRect, flags );
}

void QwtNullPaintDevice::PaintEngine::updateState(
    const QPaintEngineState& engineState)
{
    QwtNullPaintDevice* device = nullDevice();
    if ( device == nullptr )
        return;

    device->updateState( engineState );
}

inline QwtNullPaintDevice* QwtNullPaintDevice::PaintEngine::nullDevice()
{
    if ( !isActive() )
        return nullptr;

    return static_cast< QwtNullPaintDevice* >( paintDevice() );
}

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtNullPaintDevice::QwtNullPaintDevice():
    m_engine( nullptr )
{
    m_data = new PrivateData;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtNullPaintDevice::~QwtNullPaintDevice()
{
    delete m_engine;
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the render mode
 * @param[in] mode New mode for the paint device
 * \endif
 * \if CHINESE
 * @brief 设置渲染模式
 * @param[in] mode 绘制设备的新模式
 * \endif
 */
void QwtNullPaintDevice::setMode( Mode mode )
{
    m_data->mode = mode;
}

/**
 * \if ENGLISH
 * @brief Get the render mode
 * @return Current render mode
 * \endif
 * \if CHINESE
 * @brief 获取渲染模式
 * @return 当前渲染模式
 * \endif
 */
QwtNullPaintDevice::Mode QwtNullPaintDevice::mode() const
{
    return m_data->mode;
}

/**
 * \if ENGLISH
 * @brief Return the paint engine for this device
 * @return Pointer to the paint engine, created on first call
 * \endif
 * \if CHINESE
 * @brief 返回此设备的绘制引擎
 * @return 绘制引擎指针，在首次调用时创建
 * \endif
 */
QPaintEngine* QwtNullPaintDevice::paintEngine() const
{
    if ( m_engine == nullptr )
    {
        QwtNullPaintDevice* that =
            const_cast< QwtNullPaintDevice* >( this );

        that->m_engine = new PaintEngine();
    }

    return m_engine;
}

/**
 * \if ENGLISH
 * @brief Return metric information for the paint device
 * @details Returns information about the device metrics like width, height,
 *          DPI, etc. The actual size is determined by sizeMetrics().
 * @param[in] deviceMetric Type of metric to query
 * @return Metric value for the given paint device metric
 * \endif
 * \if CHINESE
 * @brief 返回绘制设备的度量信息
 * @details 返回关于设备度量的信息，如宽度、高度、DPI 等。
 *          实际大小由 sizeMetrics() 确定。
 * @param[in] deviceMetric 要查询的度量类型
 * @return 给定绘制设备度量的度量值
 * \endif
 */
int QwtNullPaintDevice::metric( PaintDeviceMetric deviceMetric ) const
{
    int value;

    switch ( deviceMetric )
    {
        case PdmWidth:
        {
            value = sizeMetrics().width();
            break;
        }
        case PdmHeight:
        {
            value = sizeMetrics().height();
            break;
        }
        case PdmNumColors:
        {
            value = 0xffffffff;
            break;
        }
        case PdmDepth:
        {
            value = 32;
            break;
        }
        case PdmPhysicalDpiX:
        case PdmPhysicalDpiY:
        case PdmDpiY:
        case PdmDpiX:
        {
            value = 72;
            break;
        }
        case PdmWidthMM:
        {
            value = qRound( metric( PdmWidth ) * 25.4 / metric( PdmDpiX ) );
            break;
        }
        case PdmHeightMM:
        {
            value = qRound( metric( PdmHeight ) * 25.4 / metric( PdmDpiY ) );
            break;
        }
#if QT_VERSION >= 0x050000
        case PdmDevicePixelRatio:
        {
            /*
             * Qt6 uses devicePixelRatioF() (→ metric(PdmDevicePixelRatioScaled))
             * to set up the painter's initial transform.  If we return 0 here,
             * QPainter builds a degenerate scale(0,0) transform which maps every
             * path to a zero-size bounding rect, leaving QwtGraphic::isEmpty()
             * permanently true and the legend icon blank.  Always return 1 for a
             * logical / virtual device.
             */
            value = 1;
            break;
        }
#endif
#if QT_VERSION >= 0x050400
        case PdmDevicePixelRatioScaled:
        {
            value = static_cast< int >( 1.0 * QPaintDevice::devicePixelRatioFScale() );
            break;
        }
#endif
        default:
            value = 0;
    }
    return value;

}

//! See QPaintEngine::drawRects()
void QwtNullPaintDevice::drawRects(
    const QRect* rects, int rectCount)
{
    Q_UNUSED(rects);
    Q_UNUSED(rectCount);
}

//! See QPaintEngine::drawRects()
void QwtNullPaintDevice::drawRects(
    const QRectF* rects, int rectCount)
{
    Q_UNUSED(rects);
    Q_UNUSED(rectCount);
}

//! See QPaintEngine::drawLines()
void QwtNullPaintDevice::drawLines(
    const QLine* lines, int lineCount)
{
    Q_UNUSED(lines);
    Q_UNUSED(lineCount);
}

//! See QPaintEngine::drawLines()
void QwtNullPaintDevice::drawLines(
    const QLineF* lines, int lineCount)
{
    Q_UNUSED(lines);
    Q_UNUSED(lineCount);
}

//! See QPaintEngine::drawEllipse()
void QwtNullPaintDevice::drawEllipse( const QRectF& rect )
{
    Q_UNUSED(rect);
}

//! See QPaintEngine::drawEllipse()
void QwtNullPaintDevice::drawEllipse( const QRect& rect )
{
    Q_UNUSED(rect);
}

//! See QPaintEngine::drawPath()
void QwtNullPaintDevice::drawPath( const QPainterPath& path )
{
    Q_UNUSED(path);
}

//! See QPaintEngine::drawPoints()
void QwtNullPaintDevice::drawPoints(
    const QPointF* points, int pointCount)
{
    Q_UNUSED(points);
    Q_UNUSED(pointCount);
}

//! See QPaintEngine::drawPoints()
void QwtNullPaintDevice::drawPoints(
    const QPoint* points, int pointCount)
{
    Q_UNUSED(points);
    Q_UNUSED(pointCount);
}

//! See QPaintEngine::drawPolygon()
void QwtNullPaintDevice::drawPolygon(
    const QPointF* points, int pointCount,
    QPaintEngine::PolygonDrawMode mode)
{
    Q_UNUSED(points);
    Q_UNUSED(pointCount);
    Q_UNUSED(mode);
}

//! See QPaintEngine::drawPolygon()
void QwtNullPaintDevice::drawPolygon(
    const QPoint* points, int pointCount,
    QPaintEngine::PolygonDrawMode mode)
{
    Q_UNUSED(points);
    Q_UNUSED(pointCount);
    Q_UNUSED(mode);
}

//! See QPaintEngine::drawPixmap()
void QwtNullPaintDevice::drawPixmap( const QRectF& rect,
    const QPixmap& pm, const QRectF& subRect )
{
    Q_UNUSED(rect);
    Q_UNUSED(pm);
    Q_UNUSED(subRect);
}

//! See QPaintEngine::drawTextItem()
void QwtNullPaintDevice::drawTextItem(
    const QPointF& pos, const QTextItem& textItem)
{
    Q_UNUSED(pos);
    Q_UNUSED(textItem);
}

//! See QPaintEngine::drawTiledPixmap()
void QwtNullPaintDevice::drawTiledPixmap(
    const QRectF& rect, const QPixmap& pixmap,
    const QPointF& subRect)
{
    Q_UNUSED(rect);
    Q_UNUSED(pixmap);
    Q_UNUSED(subRect);
}

//! See QPaintEngine::drawImage()
void QwtNullPaintDevice::drawImage(
    const QRectF& rect, const QImage& image,
    const QRectF& subRect, Qt::ImageConversionFlags flags)
{
    Q_UNUSED(rect);
    Q_UNUSED(image);
    Q_UNUSED(subRect);
    Q_UNUSED(flags);
}

//! See QPaintEngine::updateState()
void QwtNullPaintDevice::updateState(
    const QPaintEngineState& state )
{
    Q_UNUSED(state);
}
