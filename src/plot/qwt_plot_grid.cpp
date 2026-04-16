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

#include "qwt_plot_grid.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"

#include <qpainter.h>
#include <qpen.h>

static inline bool qwtFuzzyGreaterOrEqual( double d1, double d2 )
{
    return ( d1 >= d2 ) || qFuzzyCompare( d1, d2 );
}

class QwtPlotGrid::PrivateData
{
  public:
    PrivateData()
        : xEnabled( true )
        , yEnabled( true )
        , xMinEnabled( false )
        , yMinEnabled( false )
    {
    }

    bool xEnabled;
    bool yEnabled;
    bool xMinEnabled;
    bool yMinEnabled;

    QwtScaleDiv xScaleDiv;
    QwtScaleDiv yScaleDiv;

    QPen majorPen;
    QPen minorPen;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Enables major grid, disables minor grid
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 启用主要网格，禁用次要网格
 * \endif
 */
QwtPlotGrid::QwtPlotGrid()
    : QwtPlotItem( QwtText( "Grid" ) )
{
    m_data = new PrivateData;

    setItemInterest( QwtPlotItem::ScaleInterest, true );
    setZ( 10.0 );
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
QwtPlotGrid::~QwtPlotGrid()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotGrid
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotGrid
 * \endif
 */
int QwtPlotGrid::rtti() const
{
    return QwtPlotItem::Rtti_PlotGrid;
}

/**
 * \if ENGLISH
 * @brief Enable or disable vertical grid lines
 * @param[in] on Enable (true) or disable
 * @sa Minor grid lines can be enabled or disabled with enableXMin()
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用垂直网格线
 * @param[in] on 启用（true）或禁用
 * @sa 次要网格线可以通过 enableXMin() 启用或禁用
 * \endif
 */
void QwtPlotGrid::enableX( bool on )
{
    if ( m_data->xEnabled != on )
    {
        m_data->xEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Enable or disable horizontal grid lines
 * @param[in] on Enable (true) or disable
 * @sa Minor grid lines can be enabled or disabled with enableYMin()
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用水平网格线
 * @param[in] on 启用（true）或禁用
 * @sa 次要网格线可以通过 enableYMin() 启用或禁用
 * \endif
 */
void QwtPlotGrid::enableY( bool on )
{
    if ( m_data->yEnabled != on )
    {
        m_data->yEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Enable or disable minor vertical grid lines
 * @param[in] on Enable (true) or disable
 * @sa enableX()
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用次要垂直网格线
 * @param[in] on 启用（true）或禁用
 * @sa enableX()
 * \endif
 */
void QwtPlotGrid::enableXMin( bool on )
{
    if ( m_data->xMinEnabled != on )
    {
        m_data->xMinEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Enable or disable minor horizontal grid lines
 * @param[in] on Enable (true) or disable
 * @sa enableY()
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用次要水平网格线
 * @param[in] on 启用（true）或禁用
 * @sa enableY()
 * \endif
 */
void QwtPlotGrid::enableYMin( bool on )
{
    if ( m_data->yMinEnabled != on )
    {
        m_data->yMinEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Assign an x axis scale division
 * @param[in] scaleDiv Scale division
 * \endif
 *
 * \if CHINESE
 * @brief 分配x轴比例划分
 * @param[in] scaleDiv 比例划分
 * \endif
 */
void QwtPlotGrid::setXDiv( const QwtScaleDiv& scaleDiv )
{
    if ( m_data->xScaleDiv != scaleDiv )
    {
        m_data->xScaleDiv = scaleDiv;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Assign a y axis scale division
 * @param[in] scaleDiv Scale division
 * \endif
 *
 * \if CHINESE
 * @brief 分配y轴比例划分
 * @param[in] scaleDiv 比例划分
 * \endif
 */
void QwtPlotGrid::setYDiv( const QwtScaleDiv& scaleDiv )
{
    if ( m_data->yScaleDiv != scaleDiv )
    {
        m_data->yScaleDiv = scaleDiv;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen for both major and minor grid lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并分配主要和次要网格线的画笔
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），使其非装饰性
 *          （见 QPen::isCosmetic()）。此方法用于隐藏此不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotGrid::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Assign a pen for both major and minor grid lines
 * @param[in] pen Pen
 * @sa setMajorPen(), setMinorPen()
 * \endif
 *
 * \if CHINESE
 * @brief 分配主要和次要网格线的画笔
 * @param[in] pen 画笔
 * @sa setMajorPen(), setMinorPen()
 * \endif
 */
void QwtPlotGrid::setPen( const QPen& pen )
{
    if ( m_data->majorPen != pen || m_data->minorPen != pen )
    {
        m_data->majorPen = pen;
        m_data->minorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen for major grid lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并分配主要网格线的画笔
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），使其非装饰性
 *          （见 QPen::isCosmetic()）。此方法用于隐藏此不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotGrid::setMajorPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setMajorPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Assign a pen for the major grid lines
 * @param[in] pen Pen
 * @sa majorPen(), setMinorPen(), setPen()
 * \endif
 *
 * \if CHINESE
 * @brief 分配主要网格线的画笔
 * @param[in] pen 画笔
 * @sa majorPen(), setMinorPen(), setPen()
 * \endif
 */
void QwtPlotGrid::setMajorPen( const QPen& pen )
{
    if ( m_data->majorPen != pen )
    {
        m_data->majorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen for the minor grid lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并分配次要网格线的画笔
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），使其非装饰性
 *          （见 QPen::isCosmetic()）。此方法用于隐藏此不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotGrid::setMinorPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setMinorPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Assign a pen for the minor grid lines
 * @param[in] pen Pen
 * @sa minorPen(), setMajorPen(), setPen()
 * \endif
 *
 * \if CHINESE
 * @brief 分配次要网格线的画笔
 * @param[in] pen 画笔
 * @sa minorPen(), setMajorPen(), setPen()
 * \endif
 */
void QwtPlotGrid::setMinorPen( const QPen& pen )
{
    if ( m_data->minorPen != pen )
    {
        m_data->minorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Draw the grid
 * @details The grid is drawn into the bounding rectangle such that
 *          grid lines begin and end at the rectangle's borders. The X and Y
 *          maps are used to map the scale divisions into the drawing region screen.
 * @param[in] painter Painter
 * @param[in] xMap X axis map
 * @param[in] yMap Y axis
 * @param[in] canvasRect Contents rectangle of the plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 绘制网格
 * @details 网格绘制在边界矩形内，使网格线在矩形边界开始和结束。
 *          X 和 Y 映射用于将比例划分映射到绘图区域屏幕。
 * @param[in] painter 绘图器
 * @param[in] xMap X轴映射
 * @param[in] yMap Y轴映射
 * @param[in] canvasRect 绘图画布的内容矩形
 * \endif
 */
void QwtPlotGrid::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    //  draw minor grid lines
    QPen minorPen = m_data->minorPen;
    minorPen.setCapStyle( Qt::FlatCap );

    painter->setPen( minorPen );

    if ( m_data->xEnabled && m_data->xMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            m_data->xScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            m_data->xScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    if ( m_data->yEnabled && m_data->yMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            m_data->yScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            m_data->yScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    //  draw major grid lines
    QPen majorPen = m_data->majorPen;
    majorPen.setCapStyle( Qt::FlatCap );

    painter->setPen( majorPen );

    if ( m_data->xEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            m_data->xScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }

    if ( m_data->yEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            m_data->yScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }
}

void QwtPlotGrid::drawLines( QPainter* painter, const QRectF& canvasRect,
    Qt::Orientation orientation, const QwtScaleMap& scaleMap,
    const QList< double >& values ) const
{
    const double x1 = canvasRect.left();
    const double x2 = canvasRect.right() - 1.0;
    const double y1 = canvasRect.top();
    const double y2 = canvasRect.bottom() - 1.0;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < values.count(); i++ )
    {
        double value = scaleMap.transform( values[i] );
        if ( doAlign )
            value = qRound( value );

        if ( orientation == Qt::Horizontal )
        {
            if ( qwtFuzzyGreaterOrEqual( value, y1 ) &&
                qwtFuzzyGreaterOrEqual( y2, value ) )
            {
                QwtPainter::drawLine( painter, x1, value, x2, value );
            }
        }
        else
        {
            if ( qwtFuzzyGreaterOrEqual( value, x1 ) &&
                qwtFuzzyGreaterOrEqual( x2, value ) )
            {
                QwtPainter::drawLine( painter, value, y1, value, y2 );
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen for the major grid lines
 * @return Pen for major grid lines
 * @sa setMajorPen(), setMinorPen(), setPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取主要网格线的画笔
 * @return 主要网格线的画笔
 * @sa setMajorPen(), setMinorPen(), setPen()
 * \endif
 */
const QPen& QwtPlotGrid::majorPen() const
{
    return m_data->majorPen;
}

/**
 * \if ENGLISH
 * @brief Get the pen for the minor grid lines
 * @return Pen for minor grid lines
 * @sa setMinorPen(), setMajorPen(), setPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取次要网格线的画笔
 * @return 次要网格线的画笔
 * @sa setMinorPen(), setMajorPen(), setPen()
 * \endif
 */
const QPen& QwtPlotGrid::minorPen() const
{
    return m_data->minorPen;
}

/**
 * \if ENGLISH
 * @brief Check if vertical grid lines are enabled
 * @return true if vertical grid lines are enabled
 * @sa enableX()
 * \endif
 *
 * \if CHINESE
 * @brief 检查垂直网格线是否启用
 * @return 如果垂直网格线启用返回true
 * @sa enableX()
 * \endif
 */
bool QwtPlotGrid::xEnabled() const
{
    return m_data->xEnabled;
}

/**
 * \if ENGLISH
 * @brief Check if minor vertical grid lines are enabled
 * @return true if minor vertical grid lines are enabled
 * @sa enableXMin()
 * \endif
 *
 * \if CHINESE
 * @brief 检查次要垂直网格线是否启用
 * @return 如果次要垂直网格线启用返回true
 * @sa enableXMin()
 * \endif
 */
bool QwtPlotGrid::xMinEnabled() const
{
    return m_data->xMinEnabled;
}

/**
 * \if ENGLISH
 * @brief Check if horizontal grid lines are enabled
 * @return true if horizontal grid lines are enabled
 * @sa enableY()
 * \endif
 *
 * \if CHINESE
 * @brief 检查水平网格线是否启用
 * @return 如果水平网格线启用返回true
 * @sa enableY()
 * \endif
 */
bool QwtPlotGrid::yEnabled() const
{
    return m_data->yEnabled;
}

/**
 * \if ENGLISH
 * @brief Check if minor horizontal grid lines are enabled
 * @return true if minor horizontal grid lines are enabled
 * @sa enableYMin()
 * \endif
 *
 * \if CHINESE
 * @brief 检查次要水平网格线是否启用
 * @return 如果次要水平网格线启用返回true
 * @sa enableYMin()
 * \endif
 */
bool QwtPlotGrid::yMinEnabled() const
{
    return m_data->yMinEnabled;
}


/**
 * \if ENGLISH
 * @brief Get the scale division of the x axis
 * @return Scale division of the x axis
 * \endif
 *
 * \if CHINESE
 * @brief 获取x轴的比例划分
 * @return x轴的比例划分
 * \endif
 */
const QwtScaleDiv& QwtPlotGrid::xScaleDiv() const
{
    return m_data->xScaleDiv;
}

/**
 * \if ENGLISH
 * @brief Get the scale division of the y axis
 * @return Scale division of the y axis
 * \endif
 *
 * \if CHINESE
 * @brief 获取y轴的比例划分
 * @return y轴的比例划分
 * \endif
 */
const QwtScaleDiv& QwtPlotGrid::yScaleDiv() const
{
    return m_data->yScaleDiv;
}

/**
 * \if ENGLISH
 * @brief Update the grid to changes of the axes scale division
 * @param[in] xScaleDiv Scale division of the x-axis
 * @param[in] yScaleDiv Scale division of the y-axis
 * @sa QwtPlot::updateAxes()
 * \endif
 *
 * \if CHINESE
 * @brief 更新网格以响应坐标轴比例划分的变化
 * @param[in] xScaleDiv x轴的比例划分
 * @param[in] yScaleDiv y轴的比例划分
 * @sa QwtPlot::updateAxes()
 * \endif
 */
void QwtPlotGrid::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    setXDiv( xScaleDiv );
    setYDiv( yScaleDiv );
}
