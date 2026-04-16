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

#ifndef QWT_PLOT_SVG_ITEM_H
#define QWT_PLOT_SVG_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_graphicitem.h"

class QByteArray;

/**
 * \if ENGLISH
 * @brief A plot item, which displays data in Scalable Vector Graphics (SVG) format
 * @details SVG images are often used to display maps.
 *          QwtPlotSvgItem is only a small convenience wrapper class for QwtPlotGraphicItem,
 *          that creates a QwtGraphic from SVG data.
 * \endif
 *
 * \if CHINESE
 * @brief 显示可缩放矢量图形（SVG）格式数据的绘图项
 * @details SVG 图像常用于显示地图。
 *          QwtPlotSvgItem 只是为 QwtPlotGraphicItem 提供的一个小型便利包装类，
 *          它从 SVG 数据创建 QwtGraphic。
 * \endif
 */
class QWT_EXPORT QwtPlotSvgItem : public QwtPlotGraphicItem
{
  public:
    // Constructor
    explicit QwtPlotSvgItem( const QString& title = QString() );
    // Constructor with QwtText title
    explicit QwtPlotSvgItem( const QwtText& title );
    // Destructor
    virtual ~QwtPlotSvgItem();

    // Load an SVG file
    bool loadFile( const QRectF&, const QString& fileName );
    // Load SVG data
    bool loadData( const QRectF&, const QByteArray& );
};

#endif
