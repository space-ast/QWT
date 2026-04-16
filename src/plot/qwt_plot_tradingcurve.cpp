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

#include "qwt_plot_tradingcurve.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>

static inline bool qwtIsSampleInside( const QwtOHLCSample& sample,
    double tMin, double tMax, double vMin, double vMax )
{
    const double t = sample.time;
    const QwtInterval interval = sample.boundingInterval();

    const bool isOffScreen = ( t < tMin ) || ( t > tMax )
        || ( interval.maxValue() < vMin ) || ( interval.minValue() > vMax );

    return !isOffScreen;
}

class QwtPlotTradingCurve::PrivateData
{
  public:
    PrivateData()
        : symbolStyle( QwtPlotTradingCurve::CandleStick )
        , symbolExtent( 0.6 )
        , minSymbolWidth( 2.0 )
        , maxSymbolWidth( -1.0 )
        , paintAttributes( QwtPlotTradingCurve::ClipSymbols )
    {
        symbolBrush[0] = QBrush( Qt::white );
        symbolBrush[1] = QBrush( Qt::black );
    }

    QwtPlotTradingCurve::SymbolStyle symbolStyle;
    double symbolExtent;
    double minSymbolWidth;
    double maxSymbolWidth;

    QPen symbolPen;
    QBrush symbolBrush[2]; // Increasing/Decreasing

    QwtPlotTradingCurve::PaintAttributes paintAttributes;
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
QwtPlotTradingCurve::QwtPlotTradingCurve( const QwtText& title )
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
QwtPlotTradingCurve::QwtPlotTradingCurve( const QString& title )
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
QwtPlotTradingCurve::~QwtPlotTradingCurve()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Initialize internal members
 * @details Sets default attributes and creates internal data storage.
 * \endif
 *
 * \if CHINESE
 * @brief 初始化内部成员
 * @details 设置默认属性并创建内部数据存储。
 * \endif
 */
void QwtPlotTradingCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );

    m_data = new PrivateData;
    setData( new QwtTradingChartData() );

    setZ( 19.0 );
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotTradingCurve
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotTradingCurve
 * \endif
 */
int QwtPlotTradingCurve::rtti() const
{
    return QwtPlotTradingCurve::Rtti_PlotTradingCurve;
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
void QwtPlotTradingCurve::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/**
 * \if ENGLISH
 * @brief Test a paint attribute
 * @return True when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试绘制属性
 * @return 当属性启用时返回 true
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 */
bool QwtPlotTradingCurve::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of samples
 * @sa QwtPlotSeriesItem::setData()
 * \endif
 *
 * \if CHINESE
 * @brief 用样本数组初始化数据
 * @param[in] samples 样本向量
 * @sa QwtPlotSeriesItem::setData()
 * \endif
 */
void QwtPlotTradingCurve::setSamples(
    const QVector< QwtOHLCSample >& samples )
{
    setData( new QwtTradingChartData( samples ) );
}

/**
 * \if ENGLISH
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting it when its not used anymore.
 * \endif
 *
 * \if CHINESE
 * @brief 分配一系列样本
 * @details setSamples() 只是 setData() 的包装器，没有任何额外价值——除了让开发者更容易找到。
 * @param[in] data 数据
 * @warning 该项获取数据对象的所有权，当不再使用时会删除它。
 * \endif
 */
void QwtPlotTradingCurve::setSamples(
    QwtSeriesData< QwtOHLCSample >* data )
{
    setData( data );
}

/**
 * \if ENGLISH
 * @brief Set the symbol style
 * @param[in] style Symbol style
 * @sa symbolStyle(), setSymbolExtent(), setSymbolPen(), setSymbolBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号样式
 * @param[in] style 符号样式
 * @sa symbolStyle(), setSymbolExtent(), setSymbolPen(), setSymbolBrush()
 * \endif
 */
void QwtPlotTradingCurve::setSymbolStyle( SymbolStyle style )
{
    if ( style != m_data->symbolStyle )
    {
        m_data->symbolStyle = style;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the symbol style
 * @return Symbol style
 * @sa setSymbolStyle(), symbolExtent(), symbolPen(), symbolBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号样式
 * @return 符号样式
 * @sa setSymbolStyle(), symbolExtent(), symbolPen(), symbolBrush()
 * \endif
 */
QwtPlotTradingCurve::SymbolStyle QwtPlotTradingCurve::symbolStyle() const
{
    return m_data->symbolStyle;
}

/**
 * \if ENGLISH
 * @brief Build and assign the symbol pen
 * @details In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
 *          non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并设置符号画笔
 * @details 在 Qt5 中默认画笔宽度是 1.0（Qt4 中是 0.0），这使其成为非装饰性画笔（参见 QPen::isCosmetic()）。
 *          此方法已引入以隐藏此不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotTradingCurve::setSymbolPen(
    const QColor& color, qreal width, Qt::PenStyle style )
{
    setSymbolPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Set the symbol pen
 * @details The symbol pen is used for rendering the lines of the bar or candlestick symbols
 * @param[in] pen Pen
 * @sa symbolPen(), setSymbolBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号画笔
 * @details 符号画笔用于渲染柱状图或烛台符号的线条
 * @param[in] pen 画笔
 * @sa symbolPen(), setSymbolBrush()
 * \endif
 */
void QwtPlotTradingCurve::setSymbolPen( const QPen& pen )
{
    if ( pen != m_data->symbolPen )
    {
        m_data->symbolPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the symbol pen
 * @return Symbol pen
 * @sa setSymbolPen(), symbolBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号画笔
 * @return 符号画笔
 * @sa setSymbolPen(), symbolBrush()
 * \endif
 */
QPen QwtPlotTradingCurve::symbolPen() const
{
    return m_data->symbolPen;
}

/**
 * \if ENGLISH
 * @brief Set the symbol brush
 * @param[in] direction Direction type
 * @param[in] brush Brush used to fill the body of all candlestick symbols with the direction
 * @sa symbolBrush(), setSymbolPen()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号画刷
 * @param[in] direction 方向类型
 * @param[in] brush 用于填充具有该方向的所有烛台符号主体的画刷
 * @sa symbolBrush(), setSymbolPen()
 * \endif
 */
void QwtPlotTradingCurve::setSymbolBrush(
    Direction direction, const QBrush& brush )
{
    // silencing -Wtautological-constant-out-of-range-compare
    const int index = static_cast< int >( direction );
    if ( index < 0 || index >= 2 )
        return;

    if ( brush != m_data->symbolBrush[ index ] )
    {
        m_data->symbolBrush[ index ] = brush;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the symbol brush
 * @param[in] direction Direction type
 * @return Brush used to fill the body of all candlestick symbols with the direction
 * @sa setSymbolPen(), symbolBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号画刷
 * @param[in] direction 方向类型
 * @return 用于填充具有该方向的所有烛台符号主体的画刷
 * @sa setSymbolPen(), symbolBrush()
 * \endif
 */
QBrush QwtPlotTradingCurve::symbolBrush( Direction direction ) const
{
    const int index = static_cast< int >( direction );
    if ( index < 0 || index >= 2 )
        return QBrush();

    return m_data->symbolBrush[ index ];
}

/**
 * \if ENGLISH
 * @brief Set the extent of the symbol
 * @details The width of the symbol is given in scale coordinates. When painting
 *          a symbol the width is scaled into paint device coordinates
 *          by scaledSymbolWidth(). The scaled width is bounded by
 *          minSymbolWidth(), maxSymbolWidth()
 * @param[in] extent Symbol width in scale coordinates
 * @sa symbolExtent(), scaledSymbolWidth(), setMinSymbolWidth(), setMaxSymbolWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号的范围
 * @details 符号的宽度以缩放坐标给出。绘制符号时，宽度由 scaledSymbolWidth() 缩放为绘图设备坐标。
 *          缩放后的宽度受 minSymbolWidth() 和 maxSymbolWidth() 限制。
 * @param[in] extent 缩放坐标中的符号宽度
 * @sa symbolExtent(), scaledSymbolWidth(), setMinSymbolWidth(), setMaxSymbolWidth()
 * \endif
 */
void QwtPlotTradingCurve::setSymbolExtent( double extent )
{
    extent = qwtMaxF( 0.0, extent );
    if ( extent != m_data->symbolExtent )
    {
        m_data->symbolExtent = extent;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the extent of the symbol
 * @return Extent of a symbol in scale coordinates
 * @sa setSymbolExtent(), scaledSymbolWidth(), minSymbolWidth(), maxSymbolWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号的范围
 * @return 缩放坐标中的符号范围
 * @sa setSymbolExtent(), scaledSymbolWidth(), minSymbolWidth(), maxSymbolWidth()
 * \endif
 */
double QwtPlotTradingCurve::symbolExtent() const
{
    return m_data->symbolExtent;
}

/**
 * \if ENGLISH
 * @brief Set a minimum for the symbol width
 * @param[in] width Width in paint device coordinates
 * @sa minSymbolWidth(), setMaxSymbolWidth(), setSymbolExtent()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号宽度的最小值
 * @param[in] width 绘图设备坐标中的宽度
 * @sa minSymbolWidth(), setMaxSymbolWidth(), setSymbolExtent()
 * \endif
 */
void QwtPlotTradingCurve::setMinSymbolWidth( double width )
{
    width = qwtMaxF( width, 0.0 );
    if ( width != m_data->minSymbolWidth )
    {
        m_data->minSymbolWidth = width;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the minimum for the symbol width
 * @return Minimum for the symbol width
 * @sa setMinSymbolWidth(), maxSymbolWidth(), symbolExtent()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号宽度的最小值
 * @return 符号宽度的最小值
 * @sa setMinSymbolWidth(), maxSymbolWidth(), symbolExtent()
 * \endif
 */
double QwtPlotTradingCurve::minSymbolWidth() const
{
    return m_data->minSymbolWidth;
}

/**
 * \if ENGLISH
 * @brief Set a maximum for the symbol width
 * @details A value <= 0.0 means an unlimited width
 * @param[in] width Width in paint device coordinates
 * @sa maxSymbolWidth(), setMinSymbolWidth(), setSymbolExtent()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号宽度的最大值
 * @details 值 <= 0.0 表示无限制宽度
 * @param[in] width 绘图设备坐标中的宽度
 * @sa maxSymbolWidth(), setMinSymbolWidth(), setSymbolExtent()
 * \endif
 */
void QwtPlotTradingCurve::setMaxSymbolWidth( double width )
{
    if ( width != m_data->maxSymbolWidth )
    {
        m_data->maxSymbolWidth = width;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum for the symbol width
 * @return Maximum for the symbol width
 * @sa setMaxSymbolWidth(), minSymbolWidth(), symbolExtent()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号宽度的最大值
 * @return 符号宽度的最大值
 * @sa setMaxSymbolWidth(), minSymbolWidth(), symbolExtent()
 * \endif
 */
double QwtPlotTradingCurve::maxSymbolWidth() const
{
    return m_data->maxSymbolWidth;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle of all samples
 * @return Bounding rectangle of all samples. For an empty series the rectangle is invalid.
 * \endif
 *
 * \if CHINESE
 * @brief 获取所有样本的边界矩形
 * @return 所有样本的边界矩形。对于空系列，矩形无效。
 * \endif
 */
QRectF QwtPlotTradingCurve::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if ( orientation() == Qt::Vertical )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/**
 * \if ENGLISH
 * @brief Draw an interval of the curve
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the curve will be painted to its last point.
 * @sa drawSymbols()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制曲线的一个区间
 * @param[in] painter 绘图器
 * @param[in] xMap 将 x 值映射到像素坐标。
 * @param[in] yMap 将 y 值映射到像素坐标。
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 要绘制的第一个点的索引
 * @param[in] to 要绘制的最后一个点的索引。如果 to < 0，则绘制到曲线的最后一个点。
 * @sa drawSymbols()
 * \endif
 */
void QwtPlotTradingCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    painter->save();

    if ( m_data->symbolStyle != QwtPlotTradingCurve::NoSymbol )
        drawSymbols( painter, xMap, yMap, canvasRect, from, to );

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Draw symbols
 * @param[in] painter Painter
 * @param[in] xMap x map
 * @param[in] yMap y map
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted
 * @sa drawSeries()
 * \endif
 *
 * \if CHINESE
 * @brief 绘制符号
 * @param[in] painter 绘图器
 * @param[in] xMap x 映射
 * @param[in] yMap y 映射
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 要绘制的第一个点的索引
 * @param[in] to 要绘制的最后一个点的索引
 * @sa drawSeries()
 * \endif
 */
void QwtPlotTradingCurve::drawSymbols( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const QRectF tr = QwtScaleMap::invTransform( xMap, yMap, canvasRect );

    const QwtScaleMap* timeMap, * valueMap;
    double tMin, tMax, vMin, vMax;

    const Qt::Orientation orient = orientation();
    if ( orient == Qt::Vertical )
    {
        timeMap = &xMap;
        valueMap = &yMap;

        tMin = tr.left();
        tMax = tr.right();
        vMin = tr.top();
        vMax = tr.bottom();
    }
    else
    {
        timeMap = &yMap;
        valueMap = &xMap;

        vMin = tr.left();
        vMax = tr.right();
        tMin = tr.top();
        tMax = tr.bottom();
    }

    const bool inverted = timeMap->isInverting();
    const bool doClip = m_data->paintAttributes & ClipSymbols;
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double symbolWidth = scaledSymbolWidth( xMap, yMap, canvasRect );
    if ( doAlign )
        symbolWidth = std::floor( 0.5 * symbolWidth ) * 2.0;

    QPen pen = m_data->symbolPen;
    pen.setCapStyle( Qt::FlatCap );

    painter->setPen( pen );

    for ( int i = from; i <= to; i++ )
    {
        const QwtOHLCSample s = sample( i );

        if ( !doClip || qwtIsSampleInside( s, tMin, tMax, vMin, vMax ) )
        {
            QwtOHLCSample translatedSample;

            translatedSample.time = timeMap->transform( s.time );
            translatedSample.open = valueMap->transform( s.open );
            translatedSample.high = valueMap->transform( s.high );
            translatedSample.low = valueMap->transform( s.low );
            translatedSample.close = valueMap->transform( s.close );

            const int brushIndex = ( s.open < s.close )
                ? QwtPlotTradingCurve::Increasing
                : QwtPlotTradingCurve::Decreasing;

            if ( doAlign )
            {
                translatedSample.time = qRound( translatedSample.time );
                translatedSample.open = qRound( translatedSample.open );
                translatedSample.high = qRound( translatedSample.high );
                translatedSample.low = qRound( translatedSample.low );
                translatedSample.close = qRound( translatedSample.close );
            }

            switch( m_data->symbolStyle )
            {
                case Bar:
                {
                    drawBar( painter, translatedSample,
                        orient, inverted, symbolWidth );
                    break;
                }
                case CandleStick:
                {
                    painter->setBrush( m_data->symbolBrush[ brushIndex ] );
                    drawCandleStick( painter, translatedSample,
                        orient, symbolWidth );
                    break;
                }
                default:
                {
                    if ( m_data->symbolStyle >= UserSymbol )
                    {
                        painter->setBrush( m_data->symbolBrush[ brushIndex ] );
                        drawUserSymbol( painter, m_data->symbolStyle,
                            translatedSample, orient, inverted, symbolWidth );
                    }
                }
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Draw a symbol for a symbol style >= UserSymbol
 * @details The implementation does nothing and is intended to be overloaded
 * @param[in] painter Qt painter, initialized with pen/brush
 * @param[in] symbolStyle Symbol style
 * @param[in] sample Samples already translated into paint device coordinates
 * @param[in] orientation Vertical or horizontal
 * @param[in] inverted True, when the opposite scale ( Qt::Vertical: x, Qt::Horizontal: y ) is increasing
 *                      in the opposite direction as QPainter coordinates.
 * @param[in] symbolWidth Width of the symbol in paint device coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 为符号样式 >= UserSymbol 绘制符号
 * @details 该实现不执行任何操作，旨在被重载
 * @param[in] painter Qt 绘图器，已用画笔/画刷初始化
 * @param[in] symbolStyle 符号样式
 * @param[in] sample 已转换为绘图设备坐标的样本
 * @param[in] orientation 垂直或水平
 * @param[in] inverted 为真时，相反比例尺（Qt::Vertical: x, Qt::Horizontal: y）
 *                      以与 QPainter 坐标相反的方向增加。
 * @param[in] symbolWidth 绘图设备坐标中的符号宽度
 * \endif
 */
void QwtPlotTradingCurve::drawUserSymbol( QPainter* painter,
    SymbolStyle symbolStyle, const QwtOHLCSample& sample,
    Qt::Orientation orientation, bool inverted, double symbolWidth ) const
{
    Q_UNUSED( painter )
    Q_UNUSED( symbolStyle )
    Q_UNUSED( orientation )
    Q_UNUSED( inverted )
    Q_UNUSED( symbolWidth )
    Q_UNUSED( sample )
}

/**
 * \if ENGLISH
 * @brief Draw a bar
 * @param[in] painter Qt painter, initialized with pen/brush
 * @param[in] sample Sample, already translated into paint device coordinates
 * @param[in] orientation Vertical or horizontal
 * @param[in] inverted When inverted is false the open tick is painted to the left/top,
 *                      otherwise it is painted right/bottom. The close tick is painted
 *                      in the opposite direction of the open tick.
 * @param[in] width Width or height of the candle, depending on the orientation
 * @sa Bar
 * \endif
 *
 * \if CHINESE
 * @brief 绘制柱状图
 * @param[in] painter Qt 绘图器，已用画笔/画刷初始化
 * @param[in] sample 已转换为绘图设备坐标的样本
 * @param[in] orientation 垂直或水平
 * @param[in] inverted 当 inverted 为假时，开盘刻度绘制在左侧/顶部，
 *                      否则绘制在右侧/底部。收盘刻度以与开盘刻度相反的方向绘制。
 * @param[in] width 柱的宽度或高度，取决于方向
 * @sa Bar
 * \endif
 */
void QwtPlotTradingCurve::drawBar( QPainter* painter,
    const QwtOHLCSample& sample, Qt::Orientation orientation,
    bool inverted, double width ) const
{
    double w2 = 0.5 * width;
    if ( inverted )
        w2 *= -1;

    if ( orientation == Qt::Vertical )
    {
        QwtPainter::drawLine( painter,
            sample.time, sample.low, sample.time, sample.high );

        QwtPainter::drawLine( painter,
            sample.time - w2, sample.open, sample.time, sample.open );
        QwtPainter::drawLine( painter,
            sample.time + w2, sample.close, sample.time, sample.close );
    }
    else
    {
        QwtPainter::drawLine( painter, sample.low, sample.time,
            sample.high, sample.time );
        QwtPainter::drawLine( painter,
            sample.open, sample.time - w2, sample.open, sample.time );
        QwtPainter::drawLine( painter,
            sample.close, sample.time + w2, sample.close, sample.time );
    }
}

/**
 * \if ENGLISH
 * @brief Draw a candle stick
 * @param[in] painter Qt painter, initialized with pen/brush
 * @param[in] sample Samples already translated into paint device coordinates
 * @param[in] orientation Vertical or horizontal
 * @param[in] width Width or height of the candle, depending on the orientation
 * @sa CandleStick
 * \endif
 *
 * \if CHINESE
 * @brief 绘制烛台图
 * @param[in] painter Qt 绘图器，已用画笔/画刷初始化
 * @param[in] sample 已转换为绘图设备坐标的样本
 * @param[in] orientation 垂直或水平
 * @param[in] width 柱的宽度或高度，取决于方向
 * @sa CandleStick
 * \endif
 */
void QwtPlotTradingCurve::drawCandleStick( QPainter* painter,
    const QwtOHLCSample& sample, Qt::Orientation orientation,
    double width ) const
{
    const double t = sample.time;
    const double v1 = qwtMinF( sample.low, sample.high );
    const double v2 = qwtMinF( sample.open, sample.close );
    const double v3 = qwtMaxF( sample.low, sample.high );
    const double v4 = qwtMaxF( sample.open, sample.close );

    if ( orientation == Qt::Vertical )
    {
        QwtPainter::drawLine( painter, t, v1, t, v2 );
        QwtPainter::drawLine( painter, t, v3, t, v4 );

        QRectF rect( t - 0.5 * width, sample.open,
            width, sample.close - sample.open );

        QwtPainter::drawRect( painter, rect );
    }
    else
    {
        QwtPainter::drawLine( painter, v1, t, v2, t );
        QwtPainter::drawLine( painter, v3, t, v4, t );

        const QRectF rect( sample.open, t - 0.5 * width,
            sample.close - sample.open, width );

        QwtPainter::drawRect( painter, rect );
    }
}

/**
 * \if ENGLISH
 * @brief Get a legend icon
 * @details Returns a rectangle filled with the color of the symbol pen
 * @param[in] index Index of the legend entry ( usually there is only one )
 * @param[in] size Icon size
 * @return Legend icon
 * @sa setLegendIconSize(), legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例图标
 * @details 返回填充有符号画笔颜色的矩形
 * @param[in] index 图例条目的索引（通常只有一个）
 * @param[in] size 图标大小
 * @return 图例图标
 * @sa setLegendIconSize(), legendData()
 * \endif
 */
QwtGraphic QwtPlotTradingCurve::legendIcon( int index,
    const QSizeF& size ) const
{
    Q_UNUSED( index );
    return defaultIcon( m_data->symbolPen.color(), size );
}

/**
 * \if ENGLISH
 * @brief Calculate the symbol width in paint coordinates
 * @details The width is calculated by scaling the symbol extent into
 *          paint device coordinates bounded by the minimum/maximum symbol width.
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * @return Symbol width in paint coordinates
 * @sa symbolExtent(), minSymbolWidth(), maxSymbolWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 计算绘图坐标中的符号宽度
 * @details 宽度通过将符号范围缩放到绘图设备坐标来计算，受最小/最大符号宽度限制。
 * @param[in] xMap 将 x 值映射到像素坐标。
 * @param[in] yMap 将 y 值映射到像素坐标。
 * @param[in] canvasRect 画布的内容矩形
 * @return 绘图坐标中的符号宽度
 * @sa symbolExtent(), minSymbolWidth(), maxSymbolWidth()
 * \endif
 */
double QwtPlotTradingCurve::scaledSymbolWidth(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    Q_UNUSED( canvasRect );

    if ( m_data->maxSymbolWidth > 0.0 &&
        m_data->minSymbolWidth >= m_data->maxSymbolWidth )
    {
        return m_data->minSymbolWidth;
    }

    const QwtScaleMap* map =
        ( orientation() == Qt::Vertical ) ? &xMap : &yMap;

    const double pos = map->transform( map->s1() + m_data->symbolExtent );

    double width = qAbs( pos - map->p1() );

    width = qwtMaxF( width,  m_data->minSymbolWidth );
    if ( m_data->maxSymbolWidth > 0.0 )
        width = qwtMinF( width, m_data->maxSymbolWidth );

    return width;
}
