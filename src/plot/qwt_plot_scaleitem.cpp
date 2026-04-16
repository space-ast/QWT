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

#include "qwt_plot_scaleitem.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_interval.h"
#include "qwt_text.h"

#include <qpalette.h>
#include <qpainter.h>

class QwtPlotScaleItem::PrivateData
{
  public:
    PrivateData()
        : position( 0.0 )
        , borderDistance( -1 )
        , scaleDivFromAxis( true )
        , scaleDraw( new QwtScaleDraw() )
    {
    }

    ~PrivateData()
    {
        delete scaleDraw;
    }

    QwtInterval scaleInterval( const QRectF&,
        const QwtScaleMap&, const QwtScaleMap& ) const;

    QPalette palette;
    QFont font;
    double position;
    int borderDistance;
    bool scaleDivFromAxis;
    QwtScaleDraw* scaleDraw;
};

QwtInterval QwtPlotScaleItem::PrivateData::scaleInterval( const QRectF& canvasRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap ) const
{
    QwtInterval interval;
    if ( scaleDraw->orientation() == Qt::Horizontal )
    {
        interval.setMinValue( xMap.invTransform( canvasRect.left() ) );
        interval.setMaxValue( xMap.invTransform( canvasRect.right() - 1 ) );
    }
    else
    {
        interval.setMinValue( yMap.invTransform( canvasRect.bottom() - 1 ) );
        interval.setMaxValue( yMap.invTransform( canvasRect.top() ) );
    }

    return interval;
}

/**
 * \if ENGLISH
 * @brief Constructor for scale item at the position pos
 * @param[in] alignment In case of QwtScaleDraw::BottomScale or QwtScaleDraw::TopScale,
 *                       the scale item is corresponding to the xAxis(),
 *                       otherwise it corresponds to the yAxis().
 * @param[in] pos x or y position, depending on the corresponding axis.
 * @sa setPosition(), setAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 在位置 pos 处的刻度项构造函数
 * @param[in] alignment 如果是 QwtScaleDraw::BottomScale 或 QwtScaleDraw::TopScale，
 *                       刻度项对应 xAxis()，否则对应 yAxis()。
 * @param[in] pos x 或 y 位置，取决于对应的轴。
 * @sa setPosition(), setAlignment()
 * \endif
 */
QwtPlotScaleItem::QwtPlotScaleItem(
        QwtScaleDraw::Alignment alignment, const double pos )
    : QwtPlotItem( QwtText( "Scale" ) )
{
    m_data = new PrivateData;
    m_data->position = pos;
    m_data->scaleDraw->setAlignment( alignment );

    setItemInterest( QwtPlotItem::ScaleInterest, true );
    setZ( 11.0 );
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
QwtPlotScaleItem::~QwtPlotScaleItem()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotScale
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotScale
 * \endif
 */
int QwtPlotScaleItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotScale;
}

/**
 * \if ENGLISH
 * @brief Assign a scale division
 * @param[in] scaleDiv Scale division
 * @details When assigning a scaleDiv the scale division won't be synchronized
 *          with the corresponding axis anymore.
 * @sa scaleDiv(), setScaleDivFromAxis(), isScaleDivFromAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 分配刻度划分
 * @param[in] scaleDiv 刻度划分
 * @details 分配刻度划分后，刻度划分将不再与对应的轴同步。
 * @sa scaleDiv(), setScaleDivFromAxis(), isScaleDivFromAxis()
 * \endif
 */
void QwtPlotScaleItem::setScaleDiv( const QwtScaleDiv& scaleDiv )
{
    m_data->scaleDivFromAxis = false;
    m_data->scaleDraw->setScaleDiv( scaleDiv );
}

/**
 * \if ENGLISH
 * @brief Get the scale division
 * @return Scale division
 * \endif
 *
 * \if CHINESE
 * @brief 获取刻度划分
 * @return 刻度划分
 * \endif
 */
const QwtScaleDiv& QwtPlotScaleItem::scaleDiv() const
{
    return m_data->scaleDraw->scaleDiv();
}

/**
 * \if ENGLISH
 * @brief Enable/Disable the synchronization of the scale division with the corresponding axis
 * @param[in] on true/false
 * @sa isScaleDivFromAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 启用/禁用刻度划分与对应轴的同步
 * @param[in] on true/false
 * @sa isScaleDivFromAxis()
 * \endif
 */
void QwtPlotScaleItem::setScaleDivFromAxis( bool on )
{
    if ( on != m_data->scaleDivFromAxis )
    {
        m_data->scaleDivFromAxis = on;
        if ( on )
        {
            const QwtPlot* plt = plot();
            if ( plt )
            {
                updateScaleDiv( plt->axisScaleDiv( xAxis() ),
                    plt->axisScaleDiv( yAxis() ) );
                itemChanged();
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Check if synchronization of the scale division with the corresponding axis is enabled
 * @return True, if the synchronization of the scale division with the corresponding axis is enabled.
 * @sa setScaleDiv(), setScaleDivFromAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 检查刻度划分与对应轴的同步是否启用
 * @return 如果刻度划分与对应轴的同步已启用，则返回 True。
 * @sa setScaleDiv(), setScaleDivFromAxis()
 * \endif
 */
bool QwtPlotScaleItem::isScaleDivFromAxis() const
{
    return m_data->scaleDivFromAxis;
}

/**
 * \if ENGLISH
 * @brief Set the palette
 * @param[in] palette Palette
 * @sa QwtAbstractScaleDraw::draw(), palette()
 * \endif
 *
 * \if CHINESE
 * @brief 设置调色板
 * @param[in] palette 调色板
 * @sa QwtAbstractScaleDraw::draw(), palette()
 * \endif
 */
void QwtPlotScaleItem::setPalette( const QPalette& palette )
{
    if ( palette != m_data->palette )
    {
        m_data->palette = palette;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the palette
 * @return Palette
 * @sa setPalette()
 * \endif
 *
 * \if CHINESE
 * @brief 获取调色板
 * @return 调色板
 * @sa setPalette()
 * \endif
 */
QPalette QwtPlotScaleItem::palette() const
{
    return m_data->palette;
}

/**
 * \if ENGLISH
 * @brief Change the tick label font
 * @param[in] font Font
 * @sa font()
 * \endif
 *
 * \if CHINESE
 * @brief 更改刻度标签字体
 * @param[in] font 字体
 * @sa font()
 * \endif
 */
void QwtPlotScaleItem::setFont( const QFont& font )
{
    if ( font != m_data->font )
    {
        m_data->font = font;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the tick label font
 * @return Font
 * @sa setFont()
 * \endif
 *
 * \if CHINESE
 * @brief 获取刻度标签字体
 * @return 字体
 * @sa setFont()
 * \endif
 */
QFont QwtPlotScaleItem::font() const
{
    return m_data->font;
}

/**
 * \if ENGLISH
 * @brief Set a scale draw
 * @param[in] scaleDraw Object responsible for drawing scales
 * @details The main use case for replacing the default QwtScaleDraw is
 *          to overload QwtAbstractScaleDraw::label, to replace or swallow tick labels.
 * @sa scaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 设置刻度绘制器
 * @param[in] scaleDraw 负责绘制刻度的对象
 * @details 替换默认 QwtScaleDraw 的主要用例是重载 QwtAbstractScaleDraw::label，
 *          以替换或吞掉刻度标签。
 * @sa scaleDraw()
 * \endif
 */
void QwtPlotScaleItem::setScaleDraw( QwtScaleDraw* scaleDraw )
{
    if ( scaleDraw == nullptr )
        return;

    if ( scaleDraw != m_data->scaleDraw )
        delete m_data->scaleDraw;

    m_data->scaleDraw = scaleDraw;

    const QwtPlot* plt = plot();
    if ( plt )
    {
        updateScaleDiv( plt->axisScaleDiv( xAxis() ),
            plt->axisScaleDiv( yAxis() ) );
    }

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the scale draw (const version)
 * @return Scale draw
 * @sa setScaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 获取刻度绘制器（const版本）
 * @return 刻度绘制器
 * @sa setScaleDraw()
 * \endif
 */
const QwtScaleDraw* QwtPlotScaleItem::scaleDraw() const
{
    return m_data->scaleDraw;
}

/**
 * \if ENGLISH
 * @brief Get the scale draw
 * @return Scale draw
 * @sa setScaleDraw()
 * \endif
 *
 * \if CHINESE
 * @brief 获取刻度绘制器
 * @return 刻度绘制器
 * @sa setScaleDraw()
 * \endif
 */
QwtScaleDraw* QwtPlotScaleItem::scaleDraw()
{
    return m_data->scaleDraw;
}

/**
 * \if ENGLISH
 * @brief Change the position of the scale
 * @param[in] pos New position
 * @details The position is interpreted as y value for horizontal axes
 *          and as x value for vertical axes. The border distance is set to -1.
 * @sa position(), setAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 更改刻度的位置
 * @param[in] pos 新位置
 * @details 对于水平轴，位置被解释为 y 值，对于垂直轴，位置被解释为 x 值。
 *          边界距离设置为 -1。
 * @sa position(), setAlignment()
 * \endif
 */
void QwtPlotScaleItem::setPosition( double pos )
{
    if ( m_data->position != pos )
    {
        m_data->position = pos;
        m_data->borderDistance = -1;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the position of the scale
 * @return Position of the scale
 * @sa setPosition(), setAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 获取刻度的位置
 * @return 刻度的位置
 * @sa setPosition(), setAlignment()
 * \endif
 */
double QwtPlotScaleItem::position() const
{
    return m_data->position;
}

/**
 * \if ENGLISH
 * @brief Align the scale to the canvas
 * @param[in] distance Number of pixels between the canvas border and the backbone of the scale.
 * @details If distance is >= 0 the scale will be aligned to a border of the contents rectangle
 *          of the canvas. If alignment() is QwtScaleDraw::LeftScale, the scale will be aligned
 *          to the right border, if it is QwtScaleDraw::TopScale it will be aligned to the bottom
 *          (and vice versa). If distance is < 0 the scale will be at the position().
 * @sa setPosition(), borderDistance()
 * \endif
 *
 * \if CHINESE
 * @brief 将刻度对齐到画布
 * @param[in] distance 画布边界与刻度骨干之间的像素数
 * @details 如果 distance >= 0，刻度将对齐到画布内容矩形的边界。
 *          如果 alignment() 是 QwtScaleDraw::LeftScale，刻度将右对齐，
 *          如果是 QwtScaleDraw::TopScale，刻度将下对齐（反之亦然）。
 *          如果 distance < 0，刻度将在 position() 处。
 * @sa setPosition(), borderDistance()
 * \endif
 */
void QwtPlotScaleItem::setBorderDistance( int distance )
{
    if ( distance < 0 )
        distance = -1;

    if ( distance != m_data->borderDistance )
    {
        m_data->borderDistance = distance;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the distance from a canvas border
 * @return Distance from a canvas border
 * @sa setBorderDistance(), setPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 获取从画布边界的距离
 * @return 从画布边界的距离
 * @sa setBorderDistance(), setPosition()
 * \endif
 */
int QwtPlotScaleItem::borderDistance() const
{
    return m_data->borderDistance;
}

/**
 * \if ENGLISH
 * @brief Change the alignment of the scale
 * @param[in] alignment Alignment
 * @details The alignment sets the orientation of the scale and the position of the ticks:
 *          - QwtScaleDraw::BottomScale: horizontal, ticks below
 *          - QwtScaleDraw::TopScale: horizontal, ticks above
 *          - QwtScaleDraw::LeftScale: vertical, ticks left
 *          - QwtScaleDraw::RightScale: vertical, ticks right
 *          For horizontal scales the position corresponds to QwtPlotItem::yAxis(),
 *          otherwise to QwtPlotItem::xAxis().
 * @sa scaleDraw(), QwtScaleDraw::alignment(), setPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 更改刻度的对齐方式
 * @param[in] alignment 对齐方式
 * @details 对齐设置刻度的方向和刻度位置：
 *          - QwtScaleDraw::BottomScale：水平，刻度在下
 *          - QwtScaleDraw::TopScale：水平，刻度在上
 *          - QwtScaleDraw::LeftScale：垂直，刻度在左
 *          - QwtScaleDraw::RightScale：垂直，刻度在右
 *          对于水平刻度，位置对应 QwtPlotItem::yAxis()，
 *          否则对应 QwtPlotItem::xAxis()。
 * @sa scaleDraw(), QwtScaleDraw::alignment(), setPosition()
 * \endif
 */
void QwtPlotScaleItem::setAlignment( QwtScaleDraw::Alignment alignment )
{
    QwtScaleDraw* sd = m_data->scaleDraw;
    if ( sd->alignment() != alignment )
    {
        sd->setAlignment( alignment );
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Draw the scale
 * @param[in] painter Painter
 * @param[in] xMap X Scale Map
 * @param[in] yMap Y Scale Map
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制刻度
 * @param[in] painter 绘制器
 * @param[in] xMap X 比例映射
 * @param[in] yMap Y 比例映射
 * @param[in] canvasRect 画布的内容矩形（绘制器坐标）
 * \endif
 */
void QwtPlotScaleItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    QwtScaleDraw* sd = m_data->scaleDraw;

    if ( m_data->scaleDivFromAxis )
    {
        const QwtInterval interval =
            m_data->scaleInterval( canvasRect, xMap, yMap );

        if ( interval != sd->scaleDiv().interval() )
        {
            QwtScaleDiv scaleDiv = sd->scaleDiv();
            scaleDiv.setInterval( interval );
            sd->setScaleDiv( scaleDiv );
        }
    }

    QPen pen = painter->pen();
    pen.setStyle( Qt::SolidLine );
    painter->setPen( pen );

    if ( sd->orientation() == Qt::Horizontal )
    {
        double y;
        if ( m_data->borderDistance >= 0 )
        {
            if ( sd->alignment() == QwtScaleDraw::BottomScale )
                y = canvasRect.top() + m_data->borderDistance;
            else
            {
                y = canvasRect.bottom() - m_data->borderDistance;
            }

        }
        else
        {
            y = yMap.transform( m_data->position );
        }

        if ( y < canvasRect.top() || y > canvasRect.bottom() )
            return;

        sd->move( canvasRect.left(), y );
        sd->setLength( canvasRect.width() - 1 );

        QwtTransform* transform = nullptr;
        if ( xMap.transformation() )
            transform = xMap.transformation()->copy();

        sd->setTransformation( transform );
    }
    else // == Qt::Vertical
    {
        double x;
        if ( m_data->borderDistance >= 0 )
        {
            if ( sd->alignment() == QwtScaleDraw::RightScale )
                x = canvasRect.left() + m_data->borderDistance;
            else
            {
                x = canvasRect.right() - m_data->borderDistance;
            }
        }
        else
        {
            x = xMap.transform( m_data->position );
        }
        if ( x < canvasRect.left() || x > canvasRect.right() )
            return;

        sd->move( x, canvasRect.top() );
        sd->setLength( canvasRect.height() - 1 );

        QwtTransform* transform = nullptr;
        if ( yMap.transformation() )
            transform = yMap.transformation()->copy();

        sd->setTransformation( transform );
    }

    painter->setFont( m_data->font );

    sd->draw( painter, m_data->palette );
}

/**
 * \if ENGLISH
 * @brief Update the item to changes of the axes scale division
 * @param[in] xScaleDiv Scale division of the x-axis
 * @param[in] yScaleDiv Scale division of the y-axis
 * @details In case of isScaleDivFromAxis(), the scale draw is synchronized to the correspond axis.
 * @sa QwtPlot::updateAxes()
 * \endif
 *
 * \if CHINESE
 * @brief 更新项目以响应轴刻度划分的更改
 * @param[in] xScaleDiv X轴的刻度划分
 * @param[in] yScaleDiv Y轴的刻度划分
 * @details 如果 isScaleDivFromAxis()，刻度绘制器将与对应轴同步。
 * @sa QwtPlot::updateAxes()
 * \endif
 */
void QwtPlotScaleItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    QwtScaleDraw* scaleDraw = m_data->scaleDraw;

    if ( m_data->scaleDivFromAxis && scaleDraw )
    {
        const QwtScaleDiv& scaleDiv =
            scaleDraw->orientation() == Qt::Horizontal ? xScaleDiv : yScaleDiv;

        const QwtPlot* plt = plot();
        if ( plt != nullptr )
        {
            const QRectF canvasRect = plt->canvas()->contentsRect();

            const QwtInterval interval = m_data->scaleInterval(
                canvasRect, plt->canvasMap( xAxis() ), plt->canvasMap( yAxis() ) );

            QwtScaleDiv sd = scaleDiv;
            sd.setInterval( interval );

            if ( sd != scaleDraw->scaleDiv() )
            {
                // the internal label cache of QwtScaleDraw
                // is cleared here, so better avoid pointless
                // assignments.

                scaleDraw->setScaleDiv( sd );
            }
        }
        else
        {
            scaleDraw->setScaleDiv( scaleDiv );
        }
    }
}
