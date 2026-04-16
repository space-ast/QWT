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

#include "qwt_plot_multi_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_column_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_legend_data.h"
#include "qwt_math.h"

#include <qmap.h>

inline static bool qwtIsIncreasing(
    const QwtScaleMap& map, const QVector< double >& values )
{
    bool isInverting = map.isInverting();

    for ( int i = 0; i < values.size(); i++ )
    {
        const double y = values[ i ];
        if ( y != 0.0 )
            return ( map.isInverting() != ( y > 0.0 ) );
    }

    return !isInverting;
}

class QwtPlotMultiBarChart::PrivateData
{
  public:
    PrivateData()
        : style( QwtPlotMultiBarChart::Grouped )
    {
    }

    QwtPlotMultiBarChart::ChartStyle style;
    QList< QwtText > barTitles;
    QMap< int, QwtColumnSymbol* > symbolMap;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the chart
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 图表标题
 * \endif
 */
QwtPlotMultiBarChart::QwtPlotMultiBarChart( const QwtText& title )
    : QwtPlotAbstractBarChart( title )
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the chart
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 图表标题
 * \endif
 */
QwtPlotMultiBarChart::QwtPlotMultiBarChart( const QString& title )
    : QwtPlotAbstractBarChart( QwtText( title ) )
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
QwtPlotMultiBarChart::~QwtPlotMultiBarChart()
{
    resetSymbolMap();
    delete m_data;
}

void QwtPlotMultiBarChart::init()
{
    m_data = new PrivateData;
    setData( new QwtSetSeriesData() );
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotMultiBarChart
 * \endif
 * 
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotMultiBarChart
 * \endif
 */
int QwtPlotMultiBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotMultiBarChart;
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
void QwtPlotMultiBarChart::setSamples(
    const QVector< QwtSetSample >& samples )
{
    setData( new QwtSetSeriesData( samples ) );
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of vectors containing values
 * \endif
 * 
 * \if CHINESE
 * @brief 使用向量数组初始化数据
 * @param[in] samples 包含值的向量数组
 * \endif
 */
void QwtPlotMultiBarChart::setSamples(
    const QVector< QVector< double > >& samples )
{
    QVector< QwtSetSample > s;
    s.reserve( samples.size() );

    for ( int i = 0; i < samples.size(); i++ )
        s += QwtSetSample( i, samples[ i ] );

    setData( new QwtSetSeriesData( s ) );
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
void QwtPlotMultiBarChart::setSamples(
    QwtSeriesData< QwtSetSample >* data )
{
    setData( data );
}

/**
 * \if ENGLISH
 * @brief Set the titles for the bars
 * @details The titles are used for the legend.
 * @param[in] titles Bar titles
 * @sa barTitles(), legendData()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置条形的标题
 * @details 标题用于图例。
 * @param[in] titles 条形标题
 * @sa barTitles(), legendData()
 * \endif
 */
void QwtPlotMultiBarChart::setBarTitles( const QList< QwtText >& titles )
{
    m_data->barTitles = titles;
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the bar titles
 * @return Bar titles
 * @sa setBarTitles(), legendData()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取条形标题
 * @return 条形标题
 * @sa setBarTitles(), legendData()
 * \endif
 */
QList< QwtText > QwtPlotMultiBarChart::barTitles() const
{
    return m_data->barTitles;
}

/**
 * \if ENGLISH
 * @brief Add a symbol to the symbol map
 * @details Assign a default symbol for drawing the bar representing all values
 *          with the same index in a set.
 * @param[in] valueIndex Index of a value in a set
 * @param[in] symbol Symbol used for drawing a bar
 * @sa symbol(), resetSymbolMap(), specialSymbol()
 * \endif
 * 
 * \if CHINESE
 * @brief 将符号添加到符号映射
 * @details 分配用于绘制表示集合中具有相同索引的所有值的条形的默认符号。
 * @param[in] valueIndex 集合中值的索引
 * @param[in] symbol 用于绘制条形的符号
 * @sa symbol(), resetSymbolMap(), specialSymbol()
 * \endif
 */
void QwtPlotMultiBarChart::setSymbol( int valueIndex, QwtColumnSymbol* symbol )
{
    if ( valueIndex < 0 )
        return;

    QMap< int, QwtColumnSymbol* >::iterator it =
        m_data->symbolMap.find(valueIndex);
    if ( it == m_data->symbolMap.end() )
    {
        if ( symbol != nullptr )
        {
            m_data->symbolMap.insert( valueIndex, symbol );

            legendChanged();
            itemChanged();
        }
    }
    else
    {
        if ( symbol != it.value() )
        {
            delete it.value();

            if ( symbol == nullptr )
            {
                m_data->symbolMap.remove( valueIndex );
            }
            else
            {
                it.value() = symbol;
            }

            legendChanged();
            itemChanged();
        }
    }
}

/**
 * \if ENGLISH
 * @brief Find a symbol in the symbol map
 * @param[in] valueIndex Index of a value in a set
 * @return The symbol that had been set by setSymbol() or nullptr
 * @sa setSymbol(), specialSymbol(), drawBar()
 * \endif
 * 
 * \if CHINESE
 * @brief 在符号映射中查找符号
 * @param[in] valueIndex 集合中值的索引
 * @return 通过 setSymbol() 设置的符号，如果未设置则返回 nullptr
 * @sa setSymbol(), specialSymbol(), drawBar()
 * \endif
 */
const QwtColumnSymbol* QwtPlotMultiBarChart::symbol( int valueIndex ) const
{
    QMap< int, QwtColumnSymbol* >::const_iterator it =
        m_data->symbolMap.constFind( valueIndex );

    return ( it == m_data->symbolMap.constEnd() ) ? nullptr : it.value();
}

/*!
   Find a symbol in the symbol map

   \param valueIndex Index of a value in a set
   \return The symbol, that had been set by setSymbol() or nullptr.

   \sa setSymbol(), specialSymbol(), drawBar()
 */
QwtColumnSymbol* QwtPlotMultiBarChart::symbol( int valueIndex )
{
    QMap< int, QwtColumnSymbol* >::const_iterator it =
        m_data->symbolMap.constFind( valueIndex );

    return ( it == m_data->symbolMap.constEnd() ) ? nullptr : it.value();
}

/**
 * \if ENGLISH
 * @brief Remove all symbols from the symbol map
 * \endif
 * 
 * \if CHINESE
 * @brief 从符号映射中删除所有符号
 * \endif
 */
void QwtPlotMultiBarChart::resetSymbolMap()
{
    qDeleteAll( m_data->symbolMap );
    m_data->symbolMap.clear();
}

/*!
   \brief Create a symbol for special values

   Usually the symbols for displaying a bar are set by setSymbols() and
   common for all sets. By overloading specialSymbol() it is possible to
   create a temporary symbol() for displaying a special value.

   The symbol has to be created by new each time specialSymbol() is
   called. As soon as the symbol is painted this symbol gets deleted.

   When no symbol ( nullptr ) is returned, the value will be displayed
   with the standard symbol that is used for all symbols with the same
   valueIndex.

   \param sampleIndex Index of the sample
   \param valueIndex Index of the value in the set

   \return nullptr, meaning that the value is not special

 */
QwtColumnSymbol* QwtPlotMultiBarChart::specialSymbol(
    int sampleIndex, int valueIndex ) const
{
    Q_UNUSED( sampleIndex );
    Q_UNUSED( valueIndex );

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Set the style of the chart
 * @param[in] style Chart style
 * @sa style()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置图表样式
 * @param[in] style 图表样式
 * @sa style()
 * \endif
 */
void QwtPlotMultiBarChart::setStyle( ChartStyle style )
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
 * @brief Get the style of the chart
 * @return Style of the chart
 * @sa setStyle()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取图表样式
 * @return 图表样式
 * @sa setStyle()
 * \endif
 */
QwtPlotMultiBarChart::ChartStyle QwtPlotMultiBarChart::style() const
{
    return m_data->style;
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
QRectF QwtPlotMultiBarChart::boundingRect() const
{
    const size_t numSamples = dataSize();

    if ( numSamples == 0 )
        return QwtPlotSeriesItem::boundingRect();

    const double baseLine = baseline();

    QRectF rect;

    if ( m_data->style != QwtPlotMultiBarChart::Stacked )
    {
        rect = QwtPlotSeriesItem::boundingRect();

        if ( rect.height() >= 0 )
        {
            if ( rect.bottom() < baseLine )
                rect.setBottom( baseLine );
            if ( rect.top() > baseLine )
                rect.setTop( baseLine );
        }
    }
    else
    {
        double xMin, xMax, yMin, yMax;

        xMin = xMax = 0.0;
        yMin = yMax = baseLine;

        const QwtSeriesData< QwtSetSample >* series = data();

        for ( size_t i = 0; i < numSamples; i++ )
        {
            const QwtSetSample sample = series->sample( i );
            if ( i == 0 )
            {
                xMin = xMax = sample.value;
            }
            else
            {
                xMin = qwtMinF( xMin, sample.value );
                xMax = qwtMaxF( xMax, sample.value );
            }

            const double y = baseLine + sample.added();

            yMin = qwtMinF( yMin, y );
            yMax = qwtMaxF( yMax, y );
        }
        rect.setRect( xMin, yMin, xMax - xMin, yMax - yMin );
    }

    if ( orientation() == Qt::Horizontal )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/**
 * \if ENGLISH
 * @brief Draw an interval of the bar chart
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0, the curve will be painted to its last point.
 * @sa drawSample()
 * \endif
 * 
 * \if CHINESE
 * @brief 绘制柱状图的区间
 * @param[in] painter 绘制器
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 要绘制的第一个点的索引
 * @param[in] to 要绘制的最后一个点的索引。如果 to < 0，将绘制到曲线的最后一个点。
 * @sa drawSample()
 * \endif
 */
void QwtPlotMultiBarChart::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;


    const QRectF br = data()->boundingRect();
    const QwtInterval interval( br.left(), br.right() );

    painter->save();

    for ( int i = from; i <= to; i++ )
    {
        drawSample( painter, xMap, yMap,
            canvasRect, interval, i, sample( i ) );
    }

    painter->restore();
}

/*!
   Draw a sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param boundingInterval Bounding interval of sample values
   \param index Index of the sample to be painted
   \param sample Sample value

   \sa drawSeries()
 */
void QwtPlotMultiBarChart::drawSample( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, const QwtInterval& boundingInterval,
    int index, const QwtSetSample& sample ) const
{
    if ( sample.set.size() <= 0 )
        return;

    double sampleW;

    if ( orientation() == Qt::Horizontal )
    {
        sampleW = sampleWidth( yMap, canvasRect.height(),
            boundingInterval.width(), sample.value );
    }
    else
    {
        sampleW = sampleWidth( xMap, canvasRect.width(),
            boundingInterval.width(), sample.value );
    }

    if ( m_data->style == Stacked )
    {
        drawStackedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
    else
    {
        drawGroupedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
}

/*!
   Draw a grouped sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param index Index of the sample to be painted
   \param sampleWidth Bounding width for all bars of the sample
   \param sample Sample

   \sa drawSeries(), sampleWidth()
 */
void QwtPlotMultiBarChart::drawGroupedBars( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int index, double sampleWidth,
    const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect );

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    if ( orientation() == Qt::Vertical )
    {
        const double barWidth = sampleWidth / numBars;

        const double y1 = yMap.transform( baseline() );
        const double x0 = xMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            const double x1 = x0 + i * barWidth;
            const double x2 = x1 + barWidth;

            const double y2 = yMap.transform( sample.set[i] );

            QwtColumnRect barRect;
            barRect.direction = ( y1 < y2 ) ?
                QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

            barRect.hInterval = QwtInterval( x1, x2 ).normalized();
            if ( i != 0 )
                barRect.hInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            barRect.vInterval = QwtInterval( y1, y2 ).normalized();

            drawBar( painter, index, i, barRect );
        }
    }
    else
    {
        const double barHeight = sampleWidth / numBars;

        const double x1 = xMap.transform( baseline() );
        const double y0 = yMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            double y1 = y0 + i * barHeight;
            double y2 = y1 + barHeight;

            double x2 = xMap.transform( sample.set[i] );

            QwtColumnRect barRect;
            barRect.direction = x1 < x2 ?
                QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

            barRect.hInterval = QwtInterval( x1, x2 ).normalized();

            barRect.vInterval = QwtInterval( y1, y2 );
            if ( i != 0 )
                barRect.vInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            drawBar( painter, index, i, barRect );
        }
    }
}

/*!
   Draw a stacked sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param index Index of the sample to be painted
   \param sampleWidth Width of the bars
   \param sample Sample

   \sa drawSeries(), sampleWidth()
 */
void QwtPlotMultiBarChart::drawStackedBars( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int index,
    double sampleWidth, const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect ); // clipping the bars ?

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    QwtInterval::BorderFlag borderFlags = QwtInterval::IncludeBorders;

    if ( orientation() == Qt::Vertical )
    {
        const double x1 = xMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double x2 = x1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( yMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

        bar.hInterval = QwtInterval( x1, x2 ).normalized();

        double sum = baseline();

        for ( int i = 0; i < numBars; i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double y1 = yMap.transform( sum );
            const double y2 = yMap.transform( sum + si );

            if ( ( y2 > y1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.vInterval = QwtInterval( y1, y2 ).normalized();
            bar.vInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
    else
    {
        const double y1 = yMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double y2 = y1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( xMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;
        bar.vInterval = QwtInterval( y1, y2 ).normalized();

        double sum = baseline();

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double x1 = xMap.transform( sum );
            const double x2 = xMap.transform( sum + si );

            if ( ( x2 > x1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            bar.hInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
}

/*!
   Draw a bar

   \param painter Painter
   \param sampleIndex Index of the sample - might be -1 when the
                     bar is painted for the legend
   \param valueIndex Index of a value in a set
   \param rect Directed target rectangle for the bar

   \sa drawSeries()
 */
void QwtPlotMultiBarChart::drawBar( QPainter* painter,
    int sampleIndex, int valueIndex, const QwtColumnRect& rect ) const
{
    const QwtColumnSymbol* specialSym = nullptr;
    if ( sampleIndex >= 0 )
        specialSym = specialSymbol( sampleIndex, valueIndex );

    const QwtColumnSymbol* sym = specialSym;
    if ( sym == nullptr )
        sym = symbol( valueIndex );

    if ( sym )
    {
        sym->draw( painter, rect );
    }
    else
    {
        // we build a temporary default symbol
        QwtColumnSymbol columnSymbol( QwtColumnSymbol::Box );
        columnSymbol.setLineWidth( 1 );
        columnSymbol.setFrameStyle( QwtColumnSymbol::Plain );
        columnSymbol.draw( painter, rect );
    }

    delete specialSym;
}

/**
 * \if ENGLISH
 * @brief Get information to be displayed on the legend
 * @details The chart is represented by a list of entries - one for each bar title.
 *          Each element contains a bar title and an icon showing its corresponding bar.
 * @return List of legend data entries
 * @sa barTitles(), legendIcon(), legendIconSize()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取要在图例上显示的信息
 * @details 图表由条目列表表示 - 每个条形标题一个条目。
 *          每个元素包含一个条形标题和一个显示其对应条形的图标。
 * @return 图例数据条目列表
 * @sa barTitles(), legendIcon(), legendIconSize()
 * \endif
 */
QList< QwtLegendData > QwtPlotMultiBarChart::legendData() const
{
    QList< QwtLegendData > list;
    list.reserve( m_data->barTitles.size() );

    for ( int i = 0; i < m_data->barTitles.size(); i++ )
    {
        QwtLegendData data;

        data.setValue( QwtLegendData::TitleRole,
            QVariant::fromValue( m_data->barTitles[i] ) );

        if ( !legendIconSize().isEmpty() )
        {
            data.setValue( QwtLegendData::IconRole,
                QVariant::fromValue( legendIcon( i, legendIconSize() ) ) );
        }

        list += data;
    }

    return list;
}

/**
 * \if ENGLISH
 * @brief Get icon for representing a bar on the legend
 * @param[in] index Index of the bar
 * @param[in] size Icon size
 * @return An icon showing a bar
 * @sa drawBar(), legendData()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取用于在图例上表示条形的图标
 * @param[in] index 条形的索引
 * @param[in] size 图标大小
 * @return 显示条形的图标
 * @sa drawBar(), legendData()
 * \endif
 */
QwtGraphic QwtPlotMultiBarChart::legendIcon( int index,
    const QSizeF& size ) const
{
    QwtColumnRect column;
    column.hInterval = QwtInterval( 0.0, size.width() - 1.0 );
    column.vInterval = QwtInterval( 0.0, size.height() - 1.0 );

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    drawBar( &painter, -1, index, column );

    return icon;
}

