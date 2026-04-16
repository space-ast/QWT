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

#include "qwt_plot_intervalcurve.h"
#include "qwt_interval_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include "qwt_graphic.h"
#include "qwt_text.h"

#include <qpainter.h>
#include <cstring>

static inline bool qwtIsHSampleInside( const QwtIntervalSample& sample,
    double xMin, double xMax, double yMin, double yMax )
{
    const double y = sample.value;
    const double x1 = sample.interval.minValue();
    const double x2 = sample.interval.maxValue();

    const bool isOffScreen = ( y < yMin ) || ( y > yMax )
        || ( x1 < xMin && x2 < xMin ) || ( x1 > xMax && x2 > xMax );

    return !isOffScreen;
}

static inline bool qwtIsVSampleInside( const QwtIntervalSample& sample,
    double xMin, double xMax, double yMin, double yMax )
{
    const double x = sample.value;
    const double y1 = sample.interval.minValue();
    const double y2 = sample.interval.maxValue();

    const bool isOffScreen = ( x < xMin ) || ( x > xMax )
        || ( y1 < yMin && y2 < yMin ) || ( y1 > yMax && y2 > yMax );

    return !isOffScreen;
}

class QwtPlotIntervalCurve::PrivateData
{
  public:
    PrivateData():
        style( QwtPlotIntervalCurve::Tube ),
        symbol( nullptr ),
        pen( Qt::black ),
        brush( Qt::white )
    {
        paintAttributes = QwtPlotIntervalCurve::ClipPolygons;
        paintAttributes |= QwtPlotIntervalCurve::ClipSymbol;

        pen.setCapStyle( Qt::FlatCap );
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtPlotIntervalCurve::CurveStyle style;
    const QwtIntervalSymbol* symbol;

    QPen pen;
    QBrush brush;

    QwtPlotIntervalCurve::PaintAttributes paintAttributes;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the curve
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPlotIntervalCurve::QwtPlotIntervalCurve( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the curve
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPlotIntervalCurve::QwtPlotIntervalCurve( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
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
QwtPlotIntervalCurve::~QwtPlotIntervalCurve()
{
    delete m_data;
}

//! Initialize internal members
void QwtPlotIntervalCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );

    m_data = new PrivateData;
    setData( new QwtIntervalSeriesData() );

    setZ( 19.0 );
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotIntervalCurve
 * \endif
 * 
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotIntervalCurve
 * \endif
 */
int QwtPlotIntervalCurve::rtti() const
{
    return QwtPlotIntervalCurve::Rtti_PlotIntervalCurve;
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
void QwtPlotIntervalCurve::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/**
 * \if ENGLISH
 * @brief Test if a paint attribute is enabled
 * @return True when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 * 
 * \if CHINESE
 * @brief 测试绘制属性是否启用
 * @return 属性启用时返回 true
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 */
bool QwtPlotIntervalCurve::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of samples
 * \endif
 * 
 * \if CHINESE
 * @brief 使用样本数组初始化数据
 * @param[in] samples 样本向量
 * \endif
 */
void QwtPlotIntervalCurve::setSamples(
    const QVector< QwtIntervalSample >& samples )
{
    setData( new QwtIntervalSeriesData( samples ) );
}

/**
 * \if ENGLISH
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting
 *          it when its not used anymore.
 * \endif
 * 
 * \if CHINESE
 * @brief 分配样本序列
 * @details setSamples() 只是对 setData() 的封装，没有额外的价值，
 *          除了对开发者来说更容易找到。
 * @param[in] data 数据
 * @warning 该项取得数据对象的所有权，当不再使用时会删除它。
 * \endif
 */
void QwtPlotIntervalCurve::setSamples(
    QwtSeriesData< QwtIntervalSample >* data )
{
    setData( data );
}

/**
 * \if ENGLISH
 * @brief Set the curve's drawing style
 * @param[in] style Curve style
 * @sa CurveStyle, style()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置曲线的绘制样式
 * @param[in] style 曲线样式
 * @sa CurveStyle, style()
 * \endif
 */
void QwtPlotIntervalCurve::setStyle( CurveStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the curve's drawing style
 * @return Style of the curve
 * @sa setStyle()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取曲线的绘制样式
 * @return 曲线样式
 * @sa setStyle()
 * \endif
 */
QwtPlotIntervalCurve::CurveStyle QwtPlotIntervalCurve::style() const
{
    return m_data->style;
}

/**
 * \if ENGLISH
 * @brief Assign a symbol
 * @param[in] symbol Symbol
 * @sa symbol()
 * \endif
 * 
 * \if CHINESE
 * @brief 分配符号
 * @param[in] symbol 符号
 * @sa symbol()
 * \endif
 */
void QwtPlotIntervalCurve::setSymbol( const QwtIntervalSymbol* symbol )
{
    if ( symbol != m_data->symbol )
    {
        delete m_data->symbol;
        m_data->symbol = symbol;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the current symbol
 * @return Current symbol or nullptr when no symbol has been assigned
 * @sa setSymbol()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取当前符号
 * @return 当前符号，如果没有分配符号则返回 nullptr
 * @sa setSymbol()
 * \endif
 */
const QwtIntervalSymbol* QwtPlotIntervalCurve::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) what makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 * 
 * \if CHINESE
 * @brief 构建并分配画笔
 * @details 在 Qt5 中，默认画笔宽度是 1.0（Qt4 中是 0.0），这使其
 *          成为非装饰性画笔（参见 QPen::isCosmetic()）。此方法被引入
 *          以隐藏这种不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotIntervalCurve::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Assign a pen
 * @param[in] pen New pen
 * @sa pen(), brush()
 * \endif
 * 
 * \if CHINESE
 * @brief 分配画笔
 * @param[in] pen 新画笔
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotIntervalCurve::setPen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen used to draw the lines
 * @return Pen used to draw the lines
 * @sa setPen(), brush()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取用于绘制线条的画笔
 * @return 用于绘制线条的画笔
 * @sa setPen(), brush()
 * \endif
 */
const QPen& QwtPlotIntervalCurve::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Assign a brush
 * @details The brush is used to fill the area in Tube style().
 * @param[in] brush Brush
 * @sa brush(), pen(), setStyle(), CurveStyle
 * \endif
 * 
 * \if CHINESE
 * @brief 分配画刷
 * @details 画刷用于在 Tube 样式()中填充区域。
 * @param[in] brush 画刷
 * @sa brush(), pen(), setStyle(), CurveStyle
 * \endif
 */
void QwtPlotIntervalCurve::setBrush( const QBrush& brush )
{
    if ( brush != m_data->brush )
    {
        m_data->brush = brush;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the brush used to fill the area in Tube style()
 * @return Brush used to fill the area in Tube style()
 * @sa setBrush(), setStyle(), CurveStyle
 * \endif
 * 
 * \if CHINESE
 * @brief 获取用于在 Tube 样式()中填充区域的画刷
 * @return 用于在 Tube 样式()中填充区域的画刷
 * @sa setBrush(), setStyle(), CurveStyle
 * \endif
 */
const QBrush& QwtPlotIntervalCurve::brush() const
{
    return m_data->brush;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle of all samples
 * @return Bounding rectangle of all samples. For an empty series the rectangle is invalid.
 * \endif
 * 
 * \if CHINESE
 * @brief 获取所有样本的边界矩形
 * @return 所有样本的边界矩形。对于空序列，矩形无效。
 * \endif
 */
QRectF QwtPlotIntervalCurve::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if ( orientation() == Qt::Vertical )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/**
 * \if ENGLISH
 * @brief Draw a subset of the samples
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first sample to be painted
 * @param[in] to Index of the last sample to be painted. If to < 0, the series will be painted to its last sample.
 * @sa drawTube(), drawSymbols()
 * \endif
 * 
 * \if CHINESE
 * @brief 绘制样本的子集
 * @param[in] painter 绘制器
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 要绘制的第一个样本的索引
 * @param[in] to 要绘制的最后一个样本的索引。如果 to < 0，将绘制到序列的最后一个样本。
 * @sa drawTube(), drawSymbols()
 * \endif
 */
void QwtPlotIntervalCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    switch ( m_data->style )
    {
        case Tube:
            drawTube( painter, xMap, yMap, canvasRect, from, to );
            break;

        case NoCurve:
        default:
            break;
    }

    if ( m_data->symbol &&
        ( m_data->symbol->style() != QwtIntervalSymbol::NoSymbol ) )
    {
        drawSymbols( painter, *m_data->symbol,
            xMap, yMap, canvasRect, from, to );
    }
}

/*!
   Draw a tube

   Builds 2 curves from the upper and lower limits of the intervals
   and draws them with the pen(). The area between the curves is
   filled with the brush().

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

   \sa drawSeries(), drawSymbols()
 */
void QwtPlotIntervalCurve::drawTube( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    painter->save();

    const size_t size = to - from + 1;
    QPolygonF polygon( 2 * size );
    QPointF* points = polygon.data();

    for ( uint i = 0; i < size; i++ )
    {
        QPointF& minValue = points[i];
        QPointF& maxValue = points[2 * size - 1 - i];

        const QwtIntervalSample intervalSample = sample( from + i );
        if ( orientation() == Qt::Vertical )
        {
            double x = xMap.transform( intervalSample.value );
            double y1 = yMap.transform( intervalSample.interval.minValue() );
            double y2 = yMap.transform( intervalSample.interval.maxValue() );
            if ( doAlign )
            {
                x = qRound( x );
                y1 = qRound( y1 );
                y2 = qRound( y2 );
            }

            minValue.rx() = x;
            minValue.ry() = y1;
            maxValue.rx() = x;
            maxValue.ry() = y2;
        }
        else
        {
            double y = yMap.transform( intervalSample.value );
            double x1 = xMap.transform( intervalSample.interval.minValue() );
            double x2 = xMap.transform( intervalSample.interval.maxValue() );
            if ( doAlign )
            {
                y = qRound( y );
                x1 = qRound( x1 );
                x2 = qRound( x2 );
            }

            minValue.rx() = x1;
            minValue.ry() = y;
            maxValue.rx() = x2;
            maxValue.ry() = y;
        }
    }

    if ( m_data->brush.style() != Qt::NoBrush )
    {
        painter->setPen( QPen( Qt::NoPen ) );
        painter->setBrush( m_data->brush );

        if ( m_data->paintAttributes & ClipPolygons )
        {
            const qreal m = 1.0;
            const QPolygonF p = QwtClipper::clippedPolygonF(
                canvasRect.adjusted( -m, -m, m, m ), polygon, true );

            QwtPainter::drawPolygon( painter, p );
        }
        else
        {
            QwtPainter::drawPolygon( painter, polygon );
        }
    }

    if ( m_data->pen.style() != Qt::NoPen )
    {
        painter->setPen( m_data->pen );
        painter->setBrush( Qt::NoBrush );

        if ( m_data->paintAttributes & ClipPolygons )
        {
            qreal pw = QwtPainter::effectivePenWidth( painter->pen() );
            const QRectF clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );

            QPolygonF p( size );

            std::memcpy( p.data(), points, size * sizeof( QPointF ) );
            QwtPainter::drawPolyline( painter,
                QwtClipper::clippedPolygonF( clipRect, p ) );

            std::memcpy( p.data(), points + size, size * sizeof( QPointF ) );
            QwtPainter::drawPolyline( painter,
                QwtClipper::clippedPolygonF( clipRect, p ) );
        }
        else
        {
            QwtPainter::drawPolyline( painter, points, size );
            QwtPainter::drawPolyline( painter, points + size, size );
        }
    }

    painter->restore();
}

/*!
   Draw symbols for a subset of the samples

   \param painter Painter
   \param symbol Interval symbol
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted

   \sa setSymbol(), drawSeries(), drawTube()
 */
void QwtPlotIntervalCurve::drawSymbols(
    QPainter* painter, const QwtIntervalSymbol& symbol,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    painter->save();

    QPen pen = symbol.pen();
    pen.setCapStyle( Qt::FlatCap );

    painter->setPen( pen );
    painter->setBrush( symbol.brush() );

    const QRectF tr = QwtScaleMap::invTransform( xMap, yMap, canvasRect );

    const double xMin = tr.left();
    const double xMax = tr.right();
    const double yMin = tr.top();
    const double yMax = tr.bottom();

    const bool doClip = m_data->paintAttributes & ClipSymbol;

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample s = sample( i );

        if ( orientation() == Qt::Vertical )
        {
            if ( !doClip || qwtIsVSampleInside( s, xMin, xMax, yMin, yMax ) )
            {
                const double x = xMap.transform( s.value );
                const double y1 = yMap.transform( s.interval.minValue() );
                const double y2 = yMap.transform( s.interval.maxValue() );

                symbol.draw( painter, orientation(),
                    QPointF( x, y1 ), QPointF( x, y2 ) );
            }
        }
        else
        {
            if ( !doClip || qwtIsHSampleInside( s, xMin, xMax, yMin, yMax ) )
            {
                const double y = yMap.transform( s.value );
                const double x1 = xMap.transform( s.interval.minValue() );
                const double x2 = xMap.transform( s.interval.maxValue() );

                symbol.draw( painter, orientation(),
                    QPointF( x1, y ), QPointF( x2, y ) );
            }
        }
    }

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Get the icon for the legend
 * @details In case of Tube style() the icon is a plain rectangle filled with the brush().
 *          If a symbol is assigned it is scaled to size.
 * @param[in] index Index of the legend entry (ignored as there is only one)
 * @param[in] size Icon size
 * @return Icon for the legend
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取图例图标
 * @details 在 Tube 样式()的情况下，图标是用 brush() 填充的纯矩形。
 *          如果分配了符号，它会缩放到指定大小。
 * @param[in] index 图例条目的索引（忽略，因为只有一个）
 * @param[in] size 图标大小
 * @return 图例图标
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 * \endif
 */
QwtGraphic QwtPlotIntervalCurve::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    if ( size.isEmpty() )
        return QwtGraphic();

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    if ( m_data->style == Tube )
    {
        QRectF r( 0, 0, size.width(), size.height() );
        painter.fillRect( r, m_data->brush );
    }

    if ( m_data->symbol &&
        ( m_data->symbol->style() != QwtIntervalSymbol::NoSymbol ) )
    {
        QPen pen = m_data->symbol->pen();
        pen.setWidthF( pen.widthF() );
        pen.setCapStyle( Qt::FlatCap );

        painter.setPen( pen );
        painter.setBrush( m_data->symbol->brush() );

        if ( orientation() == Qt::Vertical )
        {
            const double x = 0.5 * size.width();

            m_data->symbol->draw( &painter, orientation(),
                QPointF( x, 0 ), QPointF( x, size.height() - 1.0 ) );
        }
        else
        {
            const double y = 0.5 * size.height();

            m_data->symbol->draw( &painter, orientation(),
                QPointF( 0.0, y ), QPointF( size.width() - 1.0, y ) );
        }
    }

    return icon;
}
