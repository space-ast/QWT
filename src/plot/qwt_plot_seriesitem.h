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

#ifndef QWT_PLOT_SERIES_ITEM_H
#define QWT_PLOT_SERIES_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"
#include "qwt_series_store.h"

#include <qstring.h>

class QwtScaleDiv;

/**
 * \if ENGLISH
 * @brief Base class for plot items representing a series of samples
 * @details QwtPlotSeriesItem is the base class for plot items that represent a series of samples,
 *          such as curves, bars, and other data visualization elements.
 * \endif
 * 
 * \if CHINESE
 * @brief 表示一系列样本的绘图项的基类
 * @details QwtPlotSeriesItem 是表示一系列样本的绘图项的基类，
 *          例如曲线、条形图和其他数据可视化元素。
 * \endif
 */
class QWT_EXPORT QwtPlotSeriesItem : public QwtPlotItem,
    public virtual QwtAbstractSeriesStore
{
  public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    explicit QwtPlotSeriesItem( const QString& title = QString() );
    /**
     * \if ENGLISH
     * @brief Constructor with title
     * \endif
     */
    explicit QwtPlotSeriesItem( const QwtText& title );

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotSeriesItem();

    /**
     * \if ENGLISH
     * @brief Set the orientation
     * \endif
     */
    void setOrientation( Qt::Orientation );
    /**
     * \if ENGLISH
     * @brief Get the orientation
     * \endif
     */
    Qt::Orientation orientation() const;

    /**
     * \if ENGLISH
     * @brief Draw the series item
     * \endif
     */
    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const override;

    /**
     * \if ENGLISH
     * @brief Draw a subset of the samples
     * @param painter Painter
     * @param xMap Maps x-values into pixel coordinates.
     * @param yMap Maps y-values into pixel coordinates.
     * @param canvasRect Contents rectangle of the canvas
     * @param from Index of the first point to be painted
     * @param to Index of the last point to be painted. If to < 0 the
     *           curve will be painted to its last point.
     * \endif
     * 
     * \if CHINESE
     * @brief 绘制样本的子集
     * @param painter 绘图器
     * @param xMap 将 x 值映射到像素坐标。
     * @param yMap 将 y 值映射到像素坐标。
     * @param canvasRect 画布的内容矩形
     * @param from 要绘制的第一个点的索引
     * @param to 要绘制的最后一个点的索引。如果 to < 0，则曲线将绘制到其最后一个点。
     * \endif
     */
    virtual void drawSeries( QPainter* painter,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const = 0;

    /**
     * \if ENGLISH
     * @brief Get the bounding rectangle
     * \endif
     */
    virtual QRectF boundingRect() const override;

    /**
     * \if ENGLISH
     * @brief Update the scale divisions
     * \endif
     */
    virtual void updateScaleDiv(
        const QwtScaleDiv&, const QwtScaleDiv& ) override;

  protected:
    /**
     * \if ENGLISH
     * @brief Called when the data has changed
     * \endif
     */
    virtual void dataChanged() override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
