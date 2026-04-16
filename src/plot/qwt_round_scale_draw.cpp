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

#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_math.h"

#include <qpainter.h>

class QwtRoundScaleDraw::PrivateData
{
  public:
    PrivateData()
        : center( 50.0, 50.0 )
        , radius( 50.0 )
        , startAngle( -135.0 )
        , endAngle( 135.0 )
    {
    }

    QPointF center;
    double radius;

    double startAngle;
    double endAngle;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details The range of the scale is initialized to [0, 100], the center is set to (50, 50)
 *          with a radius of 50. The angle range is set to [-135, 135].
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @details 刻度范围初始化为 [0, 100]，中心设置在 (50, 50)，半径为 50。
 *          角度范围设置为 [-135, 135]。
 * \endif
 */
QwtRoundScaleDraw::QwtRoundScaleDraw()
{
    m_data = new QwtRoundScaleDraw::PrivateData;

    setRadius( 50 );
    scaleMap().setPaintInterval( m_data->startAngle, m_data->endAngle );
}

//! Destructor
QwtRoundScaleDraw::~QwtRoundScaleDraw()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Change the radius of the scale
 * @param radius New radius
 * @details Radius is the radius of the backbone without ticks and labels.
 * \sa moveCenter()
 * \endif
 * \if CHINESE
 * @brief 更改刻度的半径
 * @param radius 新的半径
 * @details 半径是不含刻度和标签的主干半径。
 * \sa moveCenter()
 * \endif
 */
void QwtRoundScaleDraw::setRadius( double radius )
{
    m_data->radius = radius;
}

/**
 * \if ENGLISH
 * @brief Get the radius
 * @return Radius of the scale
 * @details Radius is the radius of the backbone without ticks and labels.
 * \sa setRadius(), extent()
 * \endif
 * \if CHINESE
 * @brief 获取半径
 * @return 刻度的半径
 * @details 半径是不含刻度和标签的主干半径。
 * \sa setRadius(), extent()
 * \endif
 */
double QwtRoundScaleDraw::radius() const
{
    return m_data->radius;
}

/**
 * \if ENGLISH
 * @brief Move the center of the scale draw, leaving the radius unchanged
 * @param center New center
 * \sa setRadius()
 * \endif
 * \if CHINESE
 * @brief 移动刻度绘制的中心，保持半径不变
 * @param center 新的中心
 * \sa setRadius()
 * \endif
 */
void QwtRoundScaleDraw::moveCenter( const QPointF& center )
{
    m_data->center = center;
}

/**
 * \if ENGLISH
 * @brief Get the center of the scale
 * @return Center point of the scale
 * \sa moveCenter()
 * \endif
 * \if CHINESE
 * @brief 获取刻度的中心
 * @return 刻度的中心点
 * \sa moveCenter()
 * \endif
 */
QPointF QwtRoundScaleDraw::center() const
{
    return m_data->center;
}

/**
 * \if ENGLISH
 * @brief Adjust the baseline circle segment for round scales
 * @param angle1 First boundary of the angle interval in degrees
 * @param angle2 Second boundary of the angle interval in degrees
 * @details The baseline will be drawn from min(angle1,angle2) to max(angle1, angle2).
 *          The default setting is [ -135, 135 ]. An angle of 0 degrees corresponds to
 *          the 12 o'clock position, and positive angles count in a clockwise direction.
 *
 * @warning
 * - The angle range is limited to [-360, 360] degrees. Angles exceeding this range will be clipped.
 * - For angles more or equal than 360 degrees above or below min(angle1, angle2), scale marks will not be drawn.
 * - If you need a counterclockwise scale, use QwtScaleDiv::setInterval()
 * \endif
 * \if CHINESE
 * @brief 调整圆形刻度的基线圆弧段
 * @param angle1 角度区间的第一个边界（度）
 * @param angle2 角度区间的第二个边界（度）
 * @details 基线将从 min(angle1,angle2) 绘制到 max(angle1, angle2)。
 *          默认设置为 [ -135, 135 ]。0 度角对应于 12 点钟位置，正角度按顺时针方向计算。
 *
 * @warning
 * - 角度范围限制在 [-360, 360] 度。超出此范围的角度将被裁剪。
 * - 对于大于或等于 min(angle1, angle2) 上下 360 度的角度，将不绘制刻度标记。
 * - 如果需要逆时针刻度，请使用 QwtScaleDiv::setInterval()
 * \endif
 */
void QwtRoundScaleDraw::setAngleRange( double angle1, double angle2 )
{
#if 0
    angle1 = qBound( -360.0, angle1, 360.0 );
    angle2 = qBound( -360.0, angle2, 360.0 );
#endif

    m_data->startAngle = angle1;
    m_data->endAngle = angle2;

    if ( m_data->startAngle == m_data->endAngle )
    {
        m_data->startAngle -= 1;
        m_data->endAngle += 1;
    }

    scaleMap().setPaintInterval( m_data->startAngle, m_data->endAngle );
}

/**
 * \if ENGLISH
 * @brief Draws the label for a major scale tick
 * @param painter Painter
 * @param value Value
 * \sa drawTick(), drawBackbone()
 * \endif
 * \if CHINESE
 * @brief 绘制主刻度标签
 * @param painter 绘制器
 * @param value 值
 * \sa drawTick(), drawBackbone()
 * \endif
 */
void QwtRoundScaleDraw::drawLabel( QPainter* painter, double value ) const
{
    const double tval = scaleMap().transform( value );
    if ( ( tval >= m_data->startAngle + 360.0 )
        || ( tval <= m_data->startAngle - 360.0 ) )
    {
        return;
    }

    const QwtText label = tickLabel( painter->font(), value );
    if ( label.isEmpty() )
        return;

    double radius = m_data->radius;
    if ( hasComponent( QwtAbstractScaleDraw::Ticks ) ||
        hasComponent( QwtAbstractScaleDraw::Backbone ) )
    {
        radius += spacing();
    }

    if ( hasComponent( QwtAbstractScaleDraw::Ticks ) )
        radius += tickLength( QwtScaleDiv::MajorTick );

    const QSizeF sz = label.textSize( painter->font() );
    const double arc = qwtRadians( tval );

    const double x = m_data->center.x() +
        ( radius + sz.width() / 2.0 ) * std::sin( arc );
    const double y = m_data->center.y() -
        ( radius + sz.height() / 2.0 ) * std::cos( arc );

    const QRectF r( x - sz.width() / 2, y - sz.height() / 2,
        sz.width(), sz.height() );
    label.draw( painter, r );
}

/**
 * \if ENGLISH
 * @brief Draw a tick
 * @param painter Painter
 * @param value Value of the tick
 * @param len Length of the tick
 * \sa drawBackbone(), drawLabel()
 * \endif
 * \if CHINESE
 * @brief 绘制刻度线
 * @param painter 绘制器
 * @param value 刻度值
 * @param len 刻度线长度
 * \sa drawBackbone(), drawLabel()
 * \endif
 */
void QwtRoundScaleDraw::drawTick( QPainter* painter, double value, double len ) const
{
    if ( len <= 0 )
        return;

    const double tval = scaleMap().transform( value );

    const double cx = m_data->center.x();
    const double cy = m_data->center.y();
    const double radius = m_data->radius;

    if ( ( tval < m_data->startAngle + 360.0 )
        && ( tval > m_data->startAngle - 360.0 ) )
    {
        const double arc = qwtRadians( tval );

        const double sinArc = std::sin( arc );
        const double cosArc = std::cos( arc );

        const double x1 = cx + radius * sinArc;
        const double x2 = cx + ( radius + len ) * sinArc;
        const double y1 = cy - radius * cosArc;
        const double y2 = cy - ( radius + len ) * cosArc;

        QwtPainter::drawLine( painter, x1, y1, x2, y2 );
    }
}

/**
 * \if ENGLISH
 * @brief Draws the baseline of the scale
 * @param painter Painter
 * \sa drawTick(), drawLabel()
 * \endif
 * \if CHINESE
 * @brief 绘制刻度的基线
 * @param painter 绘制器
 * \sa drawTick(), drawLabel()
 * \endif
 */
void QwtRoundScaleDraw::drawBackbone( QPainter* painter ) const
{
    const double deg1 = scaleMap().p1();
    const double deg2 = scaleMap().p2();

    const int a1 = qRound( qwtMinF( deg1, deg2 ) - 90 );
    const int a2 = qRound( qwtMaxF( deg1, deg2 ) - 90 );

    const double radius = m_data->radius;
    const double x = m_data->center.x() - radius;
    const double y = m_data->center.y() - radius;

    painter->drawArc( QRectF( x, y, 2 * radius, 2 * radius ),
        -a2 * 16, ( a2 - a1 + 1 ) * 16 );          // counterclockwise
}

/**
 * \if ENGLISH
 * @brief Calculate the extent of the scale
 * @param font Font used for painting the labels
 * @return Calculated extent
 * @details The extent is the distance between the baseline to the outermost pixel of the scale draw.
 *          radius() + extent() is an upper limit for the radius of the bounding circle.
 *
 * @warning The implemented algorithm is not too smart and calculates only an upper limit,
 *          that might be a few pixels too large.
 * \sa setMinimumExtent(), minimumExtent()
 * \endif
 * \if CHINESE
 * @brief 计算刻度的范围
 * @param font 用于绘制标签的字体
 * @return 计算出的范围
 * @details 范围是基线到刻度绘制最外层像素的距离。
 *          radius() + extent() 是边界圆半径的上限。
 *
 * @warning 实现的算法不够智能，只计算上限，可能会大几个像素。
 * \sa setMinimumExtent(), minimumExtent()
 * \endif
 */
double QwtRoundScaleDraw::extent( const QFont& font ) const
{
    double d = 0.0;

    if ( hasComponent( QwtAbstractScaleDraw::Labels ) )
    {
        const QwtScaleDiv& sd = scaleDiv();
        const QList< double >& ticks = sd.ticks( QwtScaleDiv::MajorTick );
        for ( int i = 0; i < ticks.count(); i++ )
        {
            const double value = ticks[i];
            if ( !sd.contains( value ) )
                continue;

            const double tval = scaleMap().transform( value );
            if ( ( tval < m_data->startAngle + 360 )
                && ( tval > m_data->startAngle - 360 ) )
            {
                const QwtText label = tickLabel( font, value );
                if ( label.isEmpty() )
                    continue;

                const double arc = qwtRadians( tval );

                const QSizeF sz = label.textSize( font );
                const double off = qMax( sz.width(), sz.height() );

                double x = off * std::sin( arc );
                double y = off * std::cos( arc );

                const double dist = std::sqrt( x * x + y * y );
                if ( dist > d )
                    d = dist;
            }
        }
    }

    if ( hasComponent( QwtAbstractScaleDraw::Ticks ) )
    {
        d += maxTickLength();
    }

    if ( hasComponent( QwtAbstractScaleDraw::Backbone ) )
    {
        d += qwtMaxF( penWidthF(), 1.0 );
    }

    if ( hasComponent( QwtAbstractScaleDraw::Labels ) &&
        ( hasComponent( QwtAbstractScaleDraw::Ticks ) ||
        hasComponent( QwtAbstractScaleDraw::Backbone ) ) )
    {
        d += spacing();
    }

    d = qwtMaxF( d, minimumExtent() );

    return d;
}
