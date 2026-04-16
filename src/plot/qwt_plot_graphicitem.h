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

#ifndef QWT_PLOT_GRAPHIC_ITEM_H
#define QWT_PLOT_GRAPHIC_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

#include <qstring.h>

/**
 * \if ENGLISH
 * @brief A plot item, which displays a recorded sequence of QPainter commands
 * @details QwtPlotGraphicItem renders a sequence of recorded painter commands
 *          into a specific plot area. Recording of painter commands can be
 *          done manually by QPainter or e.g. QSvgRenderer.
 * 
 * @sa QwtPlotShapeItem, QwtPlotSvgItem
 * \endif
 * 
 * \if CHINESE
 * @brief 显示记录的 QPainter 命令序列的绘图项
 * @details QwtPlotGraphicItem 将记录的绘制命令序列渲染到特定的绘图区域。
 *          绘制命令的记录可以通过 QPainter 或 QSvgRenderer 等手动完成。
 * 
 * @sa QwtPlotShapeItem, QwtPlotSvgItem
 * \endif
 */

class QWT_EXPORT QwtPlotGraphicItem : public QwtPlotItem
{
  public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     *
     * \if CHINESE
     * @brief 构造函数
     * \endif
     */
explicit QwtPlotGraphicItem( const QString& title = QString() );

    /**
     * \if ENGLISH
     * @brief Constructor with QwtText title
     * \endif
     *
     * \if CHINESE
     * @brief 构造函数（带 QwtText 标题）
     * \endif
     */
explicit QwtPlotGraphicItem( const QwtText& title );

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     *
     * \if CHINESE
     * @brief 析构函数
     * \endif
     */
virtual ~QwtPlotGraphicItem();

    /**
     * \if ENGLISH
     * @brief Set graphic with its bounding rectangle
     * \endif
     *
     * \if CHINESE
     * @brief 设置图形及其边界矩形
     * \endif
     */
void setGraphic( const QRectF& rect, const QwtGraphic& );

    /**
     * \if ENGLISH
     * @brief Get graphic
     * \endif
     *
     * \if CHINESE
     * @brief 获取图形
     * \endif
     */
QwtGraphic graphic() const;

    /**
     * \if ENGLISH
     * @brief Get the bounding rectangle
     * \endif
     *
     * \if CHINESE
     * @brief 获取边界矩形
     * \endif
     */
virtual QRectF boundingRect() const override;

    /**
     * \if ENGLISH
     * @brief Draw the graphic
     * \endif
     *
     * \if CHINESE
     * @brief 绘制图形
     * \endif
     */
virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const override;

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     *
     * \if CHINESE
     * @brief 获取运行时类型信息
     * \endif
     */
virtual int rtti() const override;

  private:
    /**
     * \if ENGLISH
     * @brief Initialize the item
     * \endif
     */
void init();

    class PrivateData;
    PrivateData* m_data;
};

#endif
