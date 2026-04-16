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

#include "qwt_plot_directpainter.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_seriesitem.h"

#include <qpainter.h>
#include <qevent.h>
#include <qpixmap.h>

static inline void qwtRenderItem(
    QPainter* painter, const QRect& canvasRect,
    QwtPlotSeriesItem* seriesItem, int from, int to )
{
    // A minor performance improvement is possible
    // with caching the maps. TODO ...

    QwtPlot* plot = seriesItem->plot();
    const QwtScaleMap xMap = plot->canvasMap( seriesItem->xAxis() );
    const QwtScaleMap yMap = plot->canvasMap( seriesItem->yAxis() );

    painter->setRenderHint( QPainter::Antialiasing,
        seriesItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );
    seriesItem->drawSeries( painter, xMap, yMap, canvasRect, from, to );
}

static inline bool qwtHasBackingStore( const QwtPlotCanvas* canvas )
{
    return canvas->testPaintAttribute( QwtPlotCanvas::BackingStore )
           && canvas->backingStore() && !canvas->backingStore()->isNull();
}

class QwtPlotDirectPainter::PrivateData
{
  public:
    PrivateData()
        : hasClipping( false )
        , seriesItem( nullptr )
        , from( 0 )
        , to( 0 )
    {
    }

    QwtPlotDirectPainter::Attributes attributes;

    bool hasClipping;
    QRegion clipRegion;

    QPainter painter;

    QwtPlotSeriesItem* seriesItem;
    int from;
    int to;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] parent Parent object
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] parent 父对象
 * \endif
 */
QwtPlotDirectPainter::QwtPlotDirectPainter( QObject* parent )
    : QObject( parent )
{
    m_data = new PrivateData;
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
QwtPlotDirectPainter::~QwtPlotDirectPainter()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Change an attribute
 * @param[in] attribute Attribute to change
 * @param[in] on On/Off
 * @sa Attribute, testAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 更改属性
 * @param[in] attribute 要更改的属性
 * @param[in] on 开/关
 * @sa Attribute, testAttribute()
 * \endif
 */
void QwtPlotDirectPainter::setAttribute( Attribute attribute, bool on )
{
    if ( bool( m_data->attributes & attribute ) != on )
    {
        if ( on )
            m_data->attributes |= attribute;
        else
            m_data->attributes &= ~attribute;

        if ( ( attribute == AtomicPainter ) && on )
            reset();
    }
}

/**
 * \if ENGLISH
 * @brief Test an attribute
 * @param[in] attribute Attribute to be tested
 * @return True, when attribute is enabled
 * @sa Attribute, setAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试属性
 * @param[in] attribute 要测试的属性
 * @return 如果属性启用则返回 true
 * @sa Attribute, setAttribute()
 * \endif
 */
bool QwtPlotDirectPainter::testAttribute( Attribute attribute ) const
{
    return m_data->attributes & attribute;
}

/**
 * \if ENGLISH
 * @brief Enable or disable clipping
 * @param[in] enable Enables clipping if true, disables it otherwise
 * @sa hasClipping(), clipRegion(), setClipRegion()
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用裁剪
 * @param[in] enable 如果为 true 则启用裁剪，否则禁用
 * @sa hasClipping(), clipRegion(), setClipRegion()
 * \endif
 */
void QwtPlotDirectPainter::setClipping( bool enable )
{
    m_data->hasClipping = enable;
}

/**
 * \if ENGLISH
 * @brief Check if clipping is enabled
 * @return true, when clipping is enabled
 * @sa setClipping(), clipRegion(), setClipRegion()
 * \endif
 *
 * \if CHINESE
 * @brief 检查是否启用了裁剪
 * @return 如果裁剪启用则返回 true
 * @sa setClipping(), clipRegion(), setClipRegion()
 * \endif
 */
bool QwtPlotDirectPainter::hasClipping() const
{
    return m_data->hasClipping;
}

/**
 * \if ENGLISH
 * @brief Assign a clip region and enable clipping
 * @details Depending on the environment setting a proper clip region might improve
 *          the performance heavily. E.g. on Qt embedded only the clipped part of
 *          the backing store will be copied to a (maybe unaccelerated) frame buffer device.
 * @param[in] region Clip region
 * @sa clipRegion(), hasClipping(), setClipping()
 * \endif
 *
 * \if CHINESE
 * @brief 分配裁剪区域并启用裁剪
 * @details 根据环境，设置适当的裁剪区域可能会大大提高性能。
 *          例如，在 Qt Embedded 上，只有裁剪部分的后备存储会被复制到
 *          （可能未加速的）帧缓冲设备。
 * @param[in] region 裁剪区域
 * @sa clipRegion(), hasClipping(), setClipping()
 * \endif
 */
void QwtPlotDirectPainter::setClipRegion( const QRegion& region )
{
    m_data->clipRegion = region;
    m_data->hasClipping = true;
}

/**
 * \if ENGLISH
 * @brief Get the currently set clip region
 * @return Currently set clip region
 * @sa setClipRegion(), setClipping(), hasClipping()
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前设置的裁剪区域
 * @return 当前设置的裁剪区域
 * @sa setClipRegion(), setClipping(), hasClipping()
 * \endif
 */
QRegion QwtPlotDirectPainter::clipRegion() const
{
    return m_data->clipRegion;
}

/**
 * \if ENGLISH
 * @brief Draw a set of points of a seriesItem
 * @details When observing a measurement while it is running, new points have to be
 *          added to an existing seriesItem. drawSeries() can be used to display them
 *          avoiding a complete redraw of the canvas.
 *          Setting plot()->canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
 *          will result in faster painting, if the paint engine of the canvas widget
 *          supports this feature.
 * @param[in] seriesItem Item to be painted
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the series will be painted to its last point.
 * \endif
 *
 * \if CHINESE
 * @brief 绘制系列项的一组点
 * @details 在观察正在运行的测量时，必须将新点添加到现有的系列项。
 *          drawSeries() 可以用于显示它们，避免完全重绘画布。
 *          设置 plot()->canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
 *          如果画布部件的绘制引擎支持此功能，将导致更快的绘制。
 * @param[in] seriesItem 要绘制的项
 * @param[in] from 要绘制的第一个点的索引
 * @param[in] to 要绘制的最后一个点的索引。如果 to < 0，系列将绘制到最后一个点。
 * \endif
 */
void QwtPlotDirectPainter::drawSeries(
    QwtPlotSeriesItem* seriesItem, int from, int to )
{
    if ( seriesItem == nullptr || seriesItem->plot() == nullptr )
        return;

    QWidget* canvas = seriesItem->plot()->canvas();
    const QRect canvasRect = canvas->contentsRect();

    QwtPlotCanvas* plotCanvas = qobject_cast< QwtPlotCanvas* >( canvas );

    if ( plotCanvas && qwtHasBackingStore( plotCanvas ) )
    {
        QPainter painter( const_cast< QPixmap* >( plotCanvas->backingStore() ) );

        if ( m_data->hasClipping )
            painter.setClipRegion( m_data->clipRegion );

        qwtRenderItem( &painter, canvasRect, seriesItem, from, to );

        painter.end();

        if ( testAttribute( QwtPlotDirectPainter::FullRepaint ) )
        {
            plotCanvas->repaint();
            return;
        }
    }

    bool immediatePaint = true;
    if ( !canvas->testAttribute( Qt::WA_WState_InPaintEvent ) )
    {
#if QT_VERSION < 0x050000
        if ( !canvas->testAttribute( Qt::WA_PaintOutsidePaintEvent ) )
#endif
        immediatePaint = false;
    }

    if ( immediatePaint )
    {
        if ( !m_data->painter.isActive() )
        {
            reset();

            m_data->painter.begin( canvas );
            canvas->installEventFilter( this );
        }

        if ( m_data->hasClipping )
        {
            m_data->painter.setClipRegion(
                QRegion( canvasRect ) & m_data->clipRegion );
        }
        else
        {
            if ( !m_data->painter.hasClipping() )
                m_data->painter.setClipRect( canvasRect );
        }

        qwtRenderItem( &m_data->painter, canvasRect, seriesItem, from, to );

        if ( m_data->attributes & QwtPlotDirectPainter::AtomicPainter )
        {
            reset();
        }
        else
        {
            if ( m_data->hasClipping )
                m_data->painter.setClipping( false );
        }
    }
    else
    {
        reset();

        m_data->seriesItem = seriesItem;
        m_data->from = from;
        m_data->to = to;

        QRegion clipRegion = canvasRect;
        if ( m_data->hasClipping )
            clipRegion &= m_data->clipRegion;

        canvas->installEventFilter( this );
        canvas->repaint(clipRegion);
        canvas->removeEventFilter( this );

        m_data->seriesItem = nullptr;
    }
}

/**
 * \if ENGLISH
 * @brief Close the internal QPainter
 * \endif
 *
 * \if CHINESE
 * @brief 关闭内部 QPainter
 * \endif
 */
void QwtPlotDirectPainter::reset()
{
    if ( m_data->painter.isActive() )
    {
        QWidget* w = static_cast< QWidget* >( m_data->painter.device() );
        if ( w )
            w->removeEventFilter( this );

        m_data->painter.end();
    }
}

/**
 * \if ENGLISH
 * @brief Event filter
 * @param[in] object Object
 * @param[in] event Event
 * @return True if the event was handled
 * \endif
 *
 * \if CHINESE
 * @brief 事件过滤器
 * @param[in] object 对象
 * @param[in] event 事件
 * @return 如果事件被处理则返回 true
 * \endif
 */
bool QwtPlotDirectPainter::eventFilter( QObject*, QEvent* event )
{
    if ( event->type() == QEvent::Paint )
    {
        reset();

        if ( m_data->seriesItem )
        {
            const QPaintEvent* pe = static_cast< QPaintEvent* >( event );

            QWidget* canvas = m_data->seriesItem->plot()->canvas();

            QPainter painter( canvas );
            painter.setClipRegion( pe->region() );

            bool doCopyCache = testAttribute( CopyBackingStore );

            if ( doCopyCache )
            {
                QwtPlotCanvas* plotCanvas =
                    qobject_cast< QwtPlotCanvas* >( canvas );
                if ( plotCanvas )
                {
                    doCopyCache = qwtHasBackingStore( plotCanvas );
                    if ( doCopyCache )
                    {
                        painter.drawPixmap( plotCanvas->rect().topLeft(),
                            *plotCanvas->backingStore() );
                    }
                }
            }

            if ( !doCopyCache )
            {
                qwtRenderItem( &painter, canvas->contentsRect(),
                    m_data->seriesItem, m_data->from, m_data->to );
            }

            return true; // don't call QwtPlotCanvas::paintEvent()
        }
    }

    return false;
}
