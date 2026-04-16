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

#include "qwt_plot_seriesitem.h"
#include "qwt_scale_div.h"
#include "qwt_text.h"

class QwtPlotSeriesItem::PrivateData
{
  public:
    PrivateData()
        : orientation( Qt::Vertical )
    {
    }

    Qt::Orientation orientation;
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
QwtPlotSeriesItem::QwtPlotSeriesItem( const QwtText& title )
    : QwtPlotItem( title )
{
    m_data = new PrivateData();
    setItemInterest( QwtPlotItem::ScaleInterest, true );
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
QwtPlotSeriesItem::QwtPlotSeriesItem( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    m_data = new PrivateData();
    setItemInterest( QwtPlotItem::ScaleInterest, true );
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
QwtPlotSeriesItem::~QwtPlotSeriesItem()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the orientation of the item
 * @details The orientation() might be used in specific way by a plot item.
 *          F.e. a QwtPlotCurve uses it to identify how to display the curve
 *          in QwtPlotCurve::Steps or QwtPlotCurve::Sticks style.
 * @param[in] orientation Orientation
 * @sa orientation()
 * \endif
 *
 * \if CHINESE
 * @brief 设置绘图项的方向
 * @details orientation() 可能被绘图项以特定方式使用。
 *          例如，QwtPlotCurve 使用它来识别如何在 QwtPlotCurve::Steps 或 QwtPlotCurve::Sticks 样式下显示曲线。
 * @param[in] orientation 方向
 * @sa orientation()
 * \endif
 */
void QwtPlotSeriesItem::setOrientation( Qt::Orientation orientation )
{
    if ( m_data->orientation != orientation )
    {
        m_data->orientation = orientation;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the orientation of the plot item
 * @return Orientation of the plot item
 * @sa setOrientation()
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图项的方向
 * @return 绘图项的方向
 * @sa setOrientation()
 * \endif
 */
Qt::Orientation QwtPlotSeriesItem::orientation() const
{
    return m_data->orientation;
}

/**
 * \if ENGLISH
 * @brief Draw the complete series
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * \endif
 *
 * \if CHINESE
 * @brief 绘制完整系列
 * @param[in] painter 绘图器
 * @param[in] xMap 将 x 值映射到像素坐标。
 * @param[in] yMap 将 y 值映射到像素坐标。
 * @param[in] canvasRect 画布的内容矩形
 * \endif
 */
void QwtPlotSeriesItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    drawSeries( painter, xMap, yMap, canvasRect, 0, -1 );
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle
 * @return Bounding rectangle
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界矩形
 * @return 边界矩形
 * \endif
 */
QRectF QwtPlotSeriesItem::boundingRect() const
{
    return dataRect();
}

/**
 * \if ENGLISH
 * @brief Update the scale divisions
 * @param[in] xScaleDiv X scale division
 * @param[in] yScaleDiv Y scale division
 * \endif
 *
 * \if CHINESE
 * @brief 更新刻度划分
 * @param[in] xScaleDiv X 刻度划分
 * @param[in] yScaleDiv Y 刻度划分
 * \endif
 */
void QwtPlotSeriesItem::updateScaleDiv(
    const QwtScaleDiv& xScaleDiv, const QwtScaleDiv& yScaleDiv )
{
    const QRectF rect = QRectF(
        xScaleDiv.lowerBound(), yScaleDiv.lowerBound(),
        xScaleDiv.range(), yScaleDiv.range() );

    setRectOfInterest( rect );
}

void QwtPlotSeriesItem::dataChanged()
{
    itemChanged();
}
