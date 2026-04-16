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

#include "qwt_vectorfield_symbol.h"

#include <qpainter.h>
#include <qpainterpath.h>

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtVectorFieldSymbol::QwtVectorFieldSymbol()
{
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
QwtVectorFieldSymbol::~QwtVectorFieldSymbol()
{
}

class QwtVectorFieldArrow::PrivateData
{
  public:
    PrivateData( qreal headW, qreal tailW )
        : headWidth( headW )
        , tailWidth( tailW )
        , length( headW + 4.0 )
    {
        /*
            Arrow is drawn horizontally, pointing into positive x direction
            with tip at 0,0.
         */

        path.lineTo( -headWidth, headWidth );
        path.lineTo( -headWidth, tailWidth );
        path.lineTo( -length, tailWidth );
        path.lineTo( -length, -tailWidth );
        path.lineTo( -headWidth, -tailWidth );
        path.lineTo( -headWidth, -headWidth );

        path.closeSubpath();
    }

    void setLength( qreal l )
    {
        length = qMax( l, headWidth );

        path.setElementPositionAt( 3, -length, tailWidth );
        path.setElementPositionAt( 4, -length, -tailWidth );
    }

    const qreal headWidth;
    const qreal tailWidth;
    qreal length;

    QPainterPath path;

};

/*!
    \if ENGLISH
    @brief Constructor

    The length is initialized by headWidth + 4

    @param headWidth Width of the triangular head
    @param tailWidth Width of the arrow tail

    \sa setLength()
    \endif
    *
    \if CHINESE
    @brief 构造函数

    长度初始化为 headWidth + 4

    @param headWidth 三角形头部的宽度
    @param tailWidth 箭头尾部的宽度

    \sa setLength()
    \endif
 */
QwtVectorFieldArrow::QwtVectorFieldArrow( qreal headWidth, qreal tailWidth )
{
    m_data = new PrivateData( headWidth, tailWidth );
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
QwtVectorFieldArrow::~QwtVectorFieldArrow()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the length of the arrow
 * @param[in] length Length of the arrow in pixels
 * @sa length()
 * \endif
 * \if CHINESE
 * @brief 设置箭头的长度
 * @param[in] length 箭头的长度（像素）
 * @sa length()
 * \endif
 */
void QwtVectorFieldArrow::setLength( qreal length )
{
    m_data->setLength( length );
}

/**
 * \if ENGLISH
 * @brief Return the length of the arrow
 * @return Length of the arrow in pixels
 * @sa setLength()
 * \endif
 * \if CHINESE
 * @brief 返回箭头的长度
 * @return 箭头的长度（像素）
 * @sa setLength()
 * \endif
 */
qreal QwtVectorFieldArrow::length() const
{
    return m_data->length;
}

/**
 * \if ENGLISH
 * @brief Draw the arrow using QPainter
 * @param[in] painter Painter to use for drawing
 * \endif
 * \if CHINESE
 * @brief 使用 QPainter 绘制箭头
 * @param[in] painter 用于绘制的画笔
 * \endif
 */
void QwtVectorFieldArrow::paint( QPainter* painter ) const
{
    painter->drawPath( m_data->path );
}

class QwtVectorFieldThinArrow::PrivateData
{
  public:
    PrivateData( qreal headW )
        : headWidth( headW )
        , length( headW + 4.0 )
    {
        path.lineTo( -headWidth, headWidth * 0.6 );
        path.moveTo( 0, 0 );
        path.lineTo( -headWidth, -headWidth * 0.6 );
        path.moveTo( 0, 0 );
        path.lineTo( -length, 0 );
    }

    const qreal headWidth;
    qreal length;

    QPainterPath path;
};

/*!
    \if ENGLISH
    @brief Constructor

    The length is initialized by headWidth + 4

    @param headWidth Width of the triangular head
    \sa setLength()
    \endif
    *
    \if CHINESE
    @brief 构造函数

    长度初始化为 headWidth + 4

    @param headWidth 三角形头部的宽度
    \sa setLength()
    \endif
 */
QwtVectorFieldThinArrow::QwtVectorFieldThinArrow( qreal headWidth )
{
    m_data = new PrivateData( headWidth );
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
QwtVectorFieldThinArrow::~QwtVectorFieldThinArrow()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the length of the arrow
 * @details Adjusts the head width proportionally when length is very small
 * @param[in] length Length of the arrow in pixels
 * @sa length()
 * \endif
 * \if CHINESE
 * @brief 设置箭头的长度
 * @details 当长度非常小时，会按比例调整头部宽度
 * @param[in] length 箭头的长度（像素）
 * @sa length()
 * \endif
 */
void QwtVectorFieldThinArrow::setLength( qreal length )
{
    m_data->length = length;

    const qreal headWidth = qMin( m_data->headWidth, length / 3.0 );

    QPainterPath& path = m_data->path;

    path.setElementPositionAt( 1, -headWidth, headWidth * 0.6 );
    path.setElementPositionAt( 3, -headWidth, -headWidth * 0.6 );
    path.setElementPositionAt( 5, -length, 0 );
}

/**
 * \if ENGLISH
 * @brief Return the length of the arrow
 * @return Length of the arrow in pixels
 * @sa setLength()
 * \endif
 * \if CHINESE
 * @brief 返回箭头的长度
 * @return 箭头的长度（像素）
 * @sa setLength()
 * \endif
 */
qreal QwtVectorFieldThinArrow::length() const
{
    return m_data->length;
}

/**
 * \if ENGLISH
 * @brief Draw the arrow using QPainter
 * @param[in] p Painter to use for drawing
 * \endif
 * \if CHINESE
 * @brief 使用 QPainter 绘制箭头
 * @param[in] p 用于绘制的画笔
 * \endif
 */
void QwtVectorFieldThinArrow::paint(QPainter* p) const
{
    p->drawPath( m_data->path );
}
