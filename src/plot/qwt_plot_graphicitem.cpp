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

#include "qwt_plot_graphicitem.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"

class QwtPlotGraphicItem::PrivateData
{
  public:
    QRectF boundingRect;
    QwtGraphic graphic;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 * @param[in] title Title
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 设置以下项目属性：
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 * @param[in] title 标题
 * \endif
 */
QwtPlotGraphicItem::QwtPlotGraphicItem( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 * @param[in] title Title
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 设置以下项目属性：
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 * @param[in] title 标题
 * \endif
 */
QwtPlotGraphicItem::QwtPlotGraphicItem( const QwtText& title )
    : QwtPlotItem( title )
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
QwtPlotGraphicItem::~QwtPlotGraphicItem()
{
    delete m_data;
}

void QwtPlotGraphicItem::init()
{
    m_data = new PrivateData();
    m_data->boundingRect = QwtPlotItem::boundingRect();

    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 8.0 );
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotGraphic
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotGraphic
 * \endif
 */
int QwtPlotGraphicItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotGraphic;
}

/**
 * \if ENGLISH
 * @brief Set the graphic to be displayed
 * @param[in] rect Rectangle in plot coordinates
 * @param[in] graphic Recorded sequence of painter commands
 * \endif
 *
 * \if CHINESE
 * @brief 设置要显示的图形
 * @param[in] rect 绘图坐标中的矩形
 * @param[in] graphic 记录的绘制命令序列
 * \endif
 */
void QwtPlotGraphicItem::setGraphic(
    const QRectF& rect, const QwtGraphic& graphic )
{
    m_data->boundingRect = rect;
    m_data->graphic = graphic;

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the recorded sequence of painter commands
 * @return Recorded sequence of painter commands
 * @sa setGraphic()
 * \endif
 *
 * \if CHINESE
 * @brief 获取记录的绘制命令序列
 * @return 记录的绘制命令序列
 * @sa setGraphic()
 * \endif
 */
QwtGraphic QwtPlotGraphicItem::graphic() const
{
    return m_data->graphic;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle of the item
 * @return Bounding rectangle of the item
 * \endif
 *
 * \if CHINESE
 * @brief 获取项目的边界矩形
 * @return 项目的边界矩形
 * \endif
 */
QRectF QwtPlotGraphicItem::boundingRect() const
{
    return m_data->boundingRect;
}

/**
 * \if ENGLISH
 * @brief Draw the item
 * @param[in] painter Painter
 * @param[in] xMap X-Scale Map
 * @param[in] yMap Y-Scale Map
 * @param[in] canvasRect Contents rect of the plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 绘制项目
 * @param[in] painter 画笔
 * @param[in] xMap X 比例尺映射
 * @param[in] yMap Y 比例尺映射
 * @param[in] canvasRect 绘图画布的内容矩形
 * \endif
 */
void QwtPlotGraphicItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    if ( m_data->graphic.isEmpty() )
        return;

    QRectF r = QwtScaleMap::transform( xMap, yMap, boundingRect() );

    if ( !r.intersects( canvasRect ) )
        return;

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        r.setLeft ( qRound( r.left() ) );
        r.setRight ( qRound( r.right() ) );
        r.setTop ( qRound( r.top() ) );
        r.setBottom ( qRound( r.bottom() ) );
    }

    m_data->graphic.render( painter, r );
}
