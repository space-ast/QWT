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

#ifndef QWT_SPLINE_PLEASING_H
#define QWT_SPLINE_PLEASING_H

#include "qwt_spline.h"

/**
 * \if ENGLISH
 * @brief A spline with G1 continuity
 *
 * QwtSplinePleasing is some sort of cardinal spline, with
 * non C1 continuous extra rules for narrow angles. It has a locality of 2.
 *
 * @note The algorithm is the one offered by a popular office package.
 * \endif
 *
 * \if CHINESE
 * @brief 具有 G1 连续性的样条
 *
 * QwtSplinePleasing 是某种基数样条，对于窄角具有非 C1 连续的额外规则。
 * 它的局部性为 2。
 *
 * @note 该算法是由一个流行的办公软件包提供的。
 * \endif
 */
class QWT_EXPORT QwtSplinePleasing : public QwtSplineG1
{
  public:
    /// \if ENGLISH Constructor \endif \if CHINESE 构造函数 \endif
    QwtSplinePleasing();
    /// \if ENGLISH Destructor \endif \if CHINESE 析构函数 \endif
    virtual ~QwtSplinePleasing();

    /// \if ENGLISH Get locality (number of points used for calculation) \endif \if CHINESE 获取局部性（用于计算的点数） \endif
    virtual uint locality() const override;

    /// \if ENGLISH Get painter path from polygon \endif \if CHINESE 从多边形获取绘制路径 \endif
    virtual QPainterPath painterPath( const QPolygonF& ) const override;
    /// \if ENGLISH Get Bezier control lines \endif \if CHINESE 获取贝塞尔控制线 \endif
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const override;
};

#endif
