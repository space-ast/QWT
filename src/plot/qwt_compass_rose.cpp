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

#include "qwt_compass_rose.h"
#include "qwt_point_polar.h"

#include <qpainter.h>
#include <qpainterpath.h>

static QPointF qwtIntersection(
    QPointF p11, QPointF p12, QPointF p21, QPointF p22 )
{
    const QLineF line1( p11, p12 );
    const QLineF line2( p21, p22 );

    QPointF pos;
#if QT_VERSION >= 0x050e00
    if ( line1.intersects( line2, &pos ) == QLineF::NoIntersection )
#else
    if ( line1.intersect( line2, &pos ) == QLineF::NoIntersection )
#endif
        return QPointF();

    return pos;
}

/**
 * \if ENGLISH
 *   \brief Constructor
 * \endif
 * \if CHINESE
 *   \brief 构造函数
 * \endif
 */
QwtCompassRose::QwtCompassRose()
{
}

/**
 * \if ENGLISH
 *   \brief Destructor
 * \endif
 * \if CHINESE
 *   \brief 析构函数
 * \endif
 */
QwtCompassRose::~QwtCompassRose()
{
}

/**
 * \if ENGLISH
 *   \brief Assign a palette
 *   \param[in] p Palette to assign
 * \endif
 * \if CHINESE
 *   \brief 设置调色板
 *   \param[in] p 要设置的调色板
 * \endif
 */
void QwtCompassRose::setPalette( const QPalette& p )
{
    m_palette = p;
}

/**
 * \if ENGLISH
 *   \brief Get the current palette
 *   \return Current palette
 * \endif
 * \if CHINESE
 *   \brief 获取当前调色板
 *   \return 当前调色板
 * \endif
 */
const QPalette& QwtCompassRose::palette() const
{
    return m_palette;
}

class QwtSimpleCompassRose::PrivateData
{
  public:
    PrivateData()
        : width( 0.2 )
        , numThorns( 8 )
        , numThornLevels( -1 )
        , shrinkFactor( 0.9 )
    {
    }

    double width;
    int numThorns;
    int numThornLevels;
    double shrinkFactor;
};

/**
 * \if ENGLISH
 *   \brief Constructor
 *   \param[in] numThorns Number of thorns
 *   \param[in] numThornLevels Number of thorn levels (-1 means auto)
 * \endif
 * \if CHINESE
 *   \brief 构造函数
 *   \param[in] numThorns 刺的数量
 *   \param[in] numThornLevels 刺的层级数（-1表示自动）
 * \endif
 */
QwtSimpleCompassRose::QwtSimpleCompassRose(
    int numThorns, int numThornLevels )
{
    m_data = new PrivateData();
    m_data->numThorns = numThorns;
    m_data->numThornLevels = numThornLevels;

    const QColor dark( 128, 128, 255 );
    const QColor light( 192, 255, 255 );

    QPalette palette;
    palette.setColor( QPalette::Dark, dark );
    palette.setColor( QPalette::Light, light );

    setPalette( palette );
}

/**
 * \if ENGLISH
 *   \brief Destructor
 * \endif
 * \if CHINESE
 *   \brief 析构函数
 * \endif
 */
QwtSimpleCompassRose::~QwtSimpleCompassRose()
{
    delete m_data;
}

/**
 * \if ENGLISH
 *   \brief Set the shrink factor for thorns with each level
 *   \param[in] factor Shrink factor (default: 0.9)
 *   \sa shrinkFactor()
 * \endif
 * \if CHINESE
 *   \brief 设置每个层级刺的收缩因子
 *   \param[in] factor 收缩因子（默认值：0.9）
 *   \sa shrinkFactor()
 * \endif
 */
void QwtSimpleCompassRose::setShrinkFactor( double factor )
{
    m_data->shrinkFactor = factor;
}

/**
 * \if ENGLISH
 *   \brief Get the shrink factor for thorns with each level
 *   \return Shrink factor
 *   \sa setShrinkFactor()
 * \endif
 * \if CHINESE
 *   \brief 获取每个层级刺的收缩因子
 *   \return 收缩因子
 *   \sa setShrinkFactor()
 * \endif
 */
double QwtSimpleCompassRose::shrinkFactor() const
{
    return m_data->shrinkFactor;
}

/**
 * \if ENGLISH
 *   \brief Draw the rose
 *   \param[in] painter Painter
 *   \param[in] center Center point
 *   \param[in] radius Radius of the rose
 *   \param[in] north Position pointing north
 *   \param[in] cg Color group
 * \endif
 * \if CHINESE
 *   \brief 绘制罗盘花
 *   \param[in] painter 绘图设备
 *   \param[in] center 中心点
 *   \param[in] radius 罗盘花的半径
 *   \param[in] north 指向北方的位置
 *   \param[in] cg 颜色组
 * \endif
 */
void QwtSimpleCompassRose::draw( QPainter* painter, const QPointF& center,
    double radius, double north, QPalette::ColorGroup cg ) const
{
    QPalette pal = palette();
    pal.setCurrentColorGroup( cg );

    drawRose( painter, pal, center, radius, north, m_data->width,
        m_data->numThorns, m_data->numThornLevels, m_data->shrinkFactor );
}

/**
 * \if ENGLISH
 *   \brief Static helper to draw a rose with specified parameters
 *   \param[in] painter Painter
 *   \param[in] palette Palette for drawing
 *   \param[in] center Center of the rose
 *   \param[in] radius Radius of the rose
 *   \param[in] north Position pointing to north
 *   \param[in] width Width of the rose heads
 *   \param[in] numThorns Number of thorns
 *   \param[in] numThornLevels Number of thorn levels
 *   \param[in] shrinkFactor Factor to shrink the thorns with each level
 * \endif
 * \if CHINESE
 *   \brief 静态辅助函数：使用指定参数绘制罗盘花
 *   \param[in] painter 绘图设备
 *   \param[in] palette 绘制用的调色板
 *   \param[in] center 罗盘花的中心
 *   \param[in] radius 罗盘花的半径
 *   \param[in] north 指向北方的位置
 *   \param[in] width 罗盘花头的宽度
 *   \param[in] numThorns 刺的数量
 *   \param[in] numThornLevels 刺的层级数
 *   \param[in] shrinkFactor 每个层级刺的收缩因子
 * \endif
 */
void QwtSimpleCompassRose::drawRose(
    QPainter* painter,
    const QPalette& palette,
    const QPointF& center, double radius, double north, double width,
    int numThorns, int numThornLevels, double shrinkFactor )
{
    if ( numThorns < 4 )
        numThorns = 4;

    if ( numThorns % 4 )
        numThorns += 4 - numThorns % 4;

    if ( numThornLevels <= 0 )
        numThornLevels = numThorns / 4;

    if ( shrinkFactor >= 1.0 )
        shrinkFactor = 1.0;

    if ( shrinkFactor <= 0.5 )
        shrinkFactor = 0.5;

    painter->save();

    painter->setPen( Qt::NoPen );

    for ( int j = 1; j <= numThornLevels; j++ )
    {
        double step = std::pow( 2.0, j ) * M_PI / numThorns;
        if ( step > M_PI_2 )
            break;

        double r = radius;
        for ( int k = 0; k < 3; k++ )
        {
            if ( j + k < numThornLevels )
                r *= shrinkFactor;
        }

        double leafWidth = r * width;
        if ( 2.0 * M_PI / step > 32 )
            leafWidth = 16;

        const double origin = qwtRadians( north );
        for ( double angle = origin;
            angle < 2.0 * M_PI + origin; angle += step )
        {
            const QPointF p = qwtPolar2Pos( center, r, angle );
            const QPointF p1 = qwtPolar2Pos( center, leafWidth, angle + M_PI_2 );
            const QPointF p2 = qwtPolar2Pos( center, leafWidth, angle - M_PI_2 );
            const QPointF p3 = qwtPolar2Pos( center, r, angle + step / 2.0 );
            const QPointF p4 = qwtPolar2Pos( center, r, angle - step / 2.0 );

            QPainterPath darkPath;
            darkPath.moveTo( center );
            darkPath.lineTo( p );
            darkPath.lineTo( qwtIntersection( center, p3, p1, p ) );

            painter->setBrush( palette.brush( QPalette::Dark ) );
            painter->drawPath( darkPath );

            QPainterPath lightPath;
            lightPath.moveTo( center );
            lightPath.lineTo( p );
            lightPath.lineTo( qwtIntersection( center, p4, p2, p ) );

            painter->setBrush( palette.brush( QPalette::Light ) );
            painter->drawPath( lightPath );
        }
    }
    painter->restore();
}

/**
 * \if ENGLISH
 *   \brief Set the width of the rose heads
 *   \param[in] width Width (range: 0.03 to 0.4, lower values make thinner heads)
 * \endif
 * \if CHINESE
 *   \brief 设置罗盘花头的宽度
 *   \param[in] width 宽度（范围：0.03到0.4，较低的值会产生更细的花头）
 * \endif
 */
void QwtSimpleCompassRose::setWidth( double width )
{
    m_data->width = width;
    if ( m_data->width < 0.03 )
        m_data->width = 0.03;

    if ( m_data->width > 0.4 )
        m_data->width = 0.4;
}

/**
 * \if ENGLISH
 *   \brief Get the width of the rose heads
 *   \return Width of the rose heads
 *   \sa setWidth()
 * \endif
 * \if CHINESE
 *   \brief 获取罗盘花头的宽度
 *   \return 罗盘花头的宽度
 *   \sa setWidth()
 * \endif
 */
double QwtSimpleCompassRose::width() const
{
    return m_data->width;
}

/**
 * \if ENGLISH
 *   \brief Set the number of thorns on one level
 *   \param[in] numThorns Number of thorns (aligned to multiple of 4, minimum 4)
 *   \sa numThorns(), setNumThornLevels()
 * \endif
 * \if CHINESE
 *   \brief 设置一个层级上的刺数
 *   \param[in] numThorns 刺的数量（对齐到4的倍数，最小为4）
 *   \sa numThorns(), setNumThornLevels()
 * \endif
 */
void QwtSimpleCompassRose::setNumThorns( int numThorns )
{
    if ( numThorns < 4 )
        numThorns = 4;

    if ( numThorns % 4 )
        numThorns += 4 - numThorns % 4;

    m_data->numThorns = numThorns;
}

/**
 * \if ENGLISH
 *   \brief Get the number of thorns
 *   \return Number of thorns
 *   \sa setNumThorns(), setNumThornLevels()
 * \endif
 * \if CHINESE
 *   \brief 获取刺的数量
 *   \return 刺的数量
 *   \sa setNumThorns(), setNumThornLevels()
 * \endif
 */
int QwtSimpleCompassRose::numThorns() const
{
    return m_data->numThorns;
}

/**
 * \if ENGLISH
 *   \brief Set the number of thorn levels
 *   \param[in] numThornLevels Number of thorn levels
 *   \sa setNumThorns(), numThornLevels()
 * \endif
 * \if CHINESE
 *   \brief 设置刺的层级数
 *   \param[in] numThornLevels 刺的层级数
 *   \sa setNumThorns(), numThornLevels()
 * \endif
 */
void QwtSimpleCompassRose::setNumThornLevels( int numThornLevels )
{
    m_data->numThornLevels = numThornLevels;
}

/**
 * \if ENGLISH
 *   \brief Get the number of thorn levels
 *   \return Number of thorn levels
 *   \sa setNumThorns(), setNumThornLevels()
 * \endif
 * \if CHINESE
 *   \brief 获取刺的层级数
 *   \return 刺的层级数
 *   \sa setNumThorns(), setNumThornLevels()
 * \endif
 */
int QwtSimpleCompassRose::numThornLevels() const
{
    return m_data->numThornLevels;
}
