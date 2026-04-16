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

#include "qwt_plot_spectrocurve.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"

#include <qpainter.h>

class QwtPlotSpectroCurve::PrivateData
{
  public:
    PrivateData()
        : colorRange( 0.0, 1000.0 )
        , penWidth( 0.0 )
        , paintAttributes( QwtPlotSpectroCurve::ClipPoints )
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    QwtColorMap* colorMap;
    QwtInterval colorRange;
    QVector< QRgb > colorTable;
    double penWidth;
    QwtPlotSpectroCurve::PaintAttributes paintAttributes;
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
QwtPlotSpectroCurve::QwtPlotSpectroCurve( const QwtText& title )
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
QwtPlotSpectroCurve::QwtPlotSpectroCurve( const QString& title )
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
QwtPlotSpectroCurve::~QwtPlotSpectroCurve()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Initialize data members
 * \endif
 * 
 * \if CHINESE
 * @brief 初始化数据成员
 * \endif
 */
void QwtPlotSpectroCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    m_data = new PrivateData;
    setData( new QwtPoint3DSeriesData() );

    setZ( 20.0 );
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotSpectroCurve
 * \endif
 * 
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotSpectroCurve
 * \endif
 */
int QwtPlotSpectroCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectroCurve;
}

/**
 * \if ENGLISH
 * @brief Specify an attribute how to draw the curve
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa PaintAttribute, testPaintAttribute()
 * \endif
 * 
 * \if CHINESE
 * @brief 指定绘制曲线的属性
 * @param[in] attribute 绘制属性
 * @param[in] on 开启/关闭
 * @sa PaintAttribute, testPaintAttribute()
 * \endif
 */
void QwtPlotSpectroCurve::setPaintAttribute( PaintAttribute attribute, bool on )
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
bool QwtPlotSpectroCurve::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of points
 * \endif
 * 
 * \if CHINESE
 * @brief 使用样本数组初始化数据
 * @param[in] samples 点向量
 * \endif
 */
void QwtPlotSpectroCurve::setSamples( const QVector< QwtPoint3D >& samples )
{
    setData( new QwtPoint3DSeriesData( samples ) );
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
void QwtPlotSpectroCurve::setSamples(
    QwtSeriesData< QwtPoint3D >* data )
{
    setData( data );
}

/**
 * \if ENGLISH
 * @brief Change the color map
 * @details Often it is useful to display the mapping between intensities and
 *          colors as an additional plot axis, showing a color bar.
 * @param[in] colorMap Color map
 * @sa colorMap(), setColorRange(), QwtColorMap::color(),
 *     QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * \endif
 * 
 * \if CHINESE
 * @brief 更改颜色映射
 * @details 通常将强度和颜色之间的映射显示为额外的绘图轴（显示颜色条）是有用的。
 * @param[in] colorMap 颜色映射
 * @sa colorMap(), setColorRange(), QwtColorMap::color(),
 *     QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * \endif
 */
void QwtPlotSpectroCurve::setColorMap( QwtColorMap* colorMap )
{
    if ( colorMap != m_data->colorMap )
    {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color map used for mapping the intensity values to colors
 * @sa setColorMap(), setColorRange(), QwtColorMap::color()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取用于将强度值映射到颜色的颜色映射
 * @return 用于将强度值映射到颜色的颜色映射
 * @sa setColorMap(), setColorRange(), QwtColorMap::color()
 * \endif
 */
const QwtColorMap* QwtPlotSpectroCurve::colorMap() const
{
    return m_data->colorMap;
}

/**
 * \if ENGLISH
 * @brief Set the value interval that corresponds to the color map
 * @param[in] interval interval.minValue() corresponds to 0.0,
 *                     interval.maxValue() to 1.0 on the color map.
 * @sa colorRange(), setColorMap(), QwtColorMap::color()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置对应于颜色映射的值区间
 * @param[in] interval interval.minValue() 对应于颜色映射上的 0.0，
 *                     interval.maxValue() 对应于 1.0。
 * @sa colorRange(), setColorMap(), QwtColorMap::color()
 * \endif
 */
void QwtPlotSpectroCurve::setColorRange( const QwtInterval& interval )
{
    if ( interval != m_data->colorRange )
    {
        m_data->colorRange = interval;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the value interval that corresponds to the color map
 * @return Value interval that corresponds to the color map
 * @sa setColorRange(), setColorMap(), QwtColorMap::color()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取对应于颜色映射的值区间
 * @return 对应于颜色映射的值区间
 * @sa setColorRange(), setColorMap(), QwtColorMap::color()
 * \endif
 */
QwtInterval& QwtPlotSpectroCurve::colorRange() const
{
    return m_data->colorRange;
}

/**
 * \if ENGLISH
 * @brief Assign a pen width
 * @param[in] penWidth New pen width
 * @sa penWidth()
 * \endif
 * 
 * \if CHINESE
 * @brief 分配画笔宽度
 * @param[in] penWidth 新画笔宽度
 * @sa penWidth()
 * \endif
 */
void QwtPlotSpectroCurve::setPenWidth(double penWidth)
{
    if ( penWidth < 0.0 )
        penWidth = 0.0;

    if ( m_data->penWidth != penWidth )
    {
        m_data->penWidth = penWidth;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen width used to draw a dot
 * @return Pen width used to draw a dot
 * @sa setPenWidth()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取用于绘制点的画笔宽度
 * @return 用于绘制点的画笔宽度
 * @sa setPenWidth()
 * \endif
 */
double QwtPlotSpectroCurve::penWidth() const
{
    return m_data->penWidth;
}

/**
 * \if ENGLISH
 * @brief Draw a subset of the points
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first sample to be painted
 * @param[in] to Index of the last sample to be painted. If to < 0, the series will be painted to its last sample.
 * @sa drawDots()
 * \endif
 * 
 * \if CHINESE
 * @brief 绘制点的子集
 * @param[in] painter 绘制器
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 要绘制的第一个样本的索引
 * @param[in] to 要绘制的最后一个样本的索引。如果 to < 0，将绘制到序列的最后一个样本。
 * @sa drawDots()
 * \endif
 */
void QwtPlotSpectroCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    drawDots( painter, xMap, yMap, canvasRect, from, to );
}

/*!
   Draw a subset of the points

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

   \sa drawSeries()
 */
void QwtPlotSpectroCurve::drawDots( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( !m_data->colorRange.isValid() )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    const QwtColorMap::Format format = m_data->colorMap->format();
    if ( format == QwtColorMap::Indexed )
        m_data->colorTable = m_data->colorMap->colorTable256();

    const QwtSeriesData< QwtPoint3D >* series = data();

    for ( int i = from; i <= to; i++ )
    {
        const QwtPoint3D sample = series->sample( i );

        double xi = xMap.transform( sample.x() );
        double yi = yMap.transform( sample.y() );
        if ( doAlign )
        {
            xi = qRound( xi );
            yi = qRound( yi );
        }

        if ( m_data->paintAttributes & QwtPlotSpectroCurve::ClipPoints )
        {
            if ( !canvasRect.contains( xi, yi ) )
                continue;
        }

        if ( format == QwtColorMap::RGB )
        {
            const QRgb rgb = m_data->colorMap->rgb(
                m_data->colorRange, sample.z() );

            painter->setPen( QPen( QColor::fromRgba( rgb ), m_data->penWidth ) );
        }
        else
        {
            const unsigned char index = m_data->colorMap->colorIndex(
                256, m_data->colorRange, sample.z() );

            painter->setPen( QPen( QColor::fromRgba( m_data->colorTable[index] ),
                m_data->penWidth ) );
        }

        QwtPainter::drawPoint( painter, QPointF( xi, yi ) );
    }

    m_data->colorTable.clear();
}
