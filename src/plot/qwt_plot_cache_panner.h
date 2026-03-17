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

#ifndef QWT_PLOT_CACHE_PANNER_H
#define QWT_PLOT_CACHE_PANNER_H

#include "qwt_global.h"
#include "qwt_cache_panner.h"
#include "qwt_axis_id.h"

class QwtPlot;

/**
 * \if ENGLISH
 * @brief QwtPlotCachePanner provides panning of a plot canvas
 * @details QwtPlotCachePanner is a panner for a plot canvas, that
 *          adjusts the scales of the axes after dropping
 *          the canvas on its new position.
 * 
 *          Together with QwtPlotZoomer and QwtPlotMagnifier powerful ways
 *          of navigating on a QwtPlot widget can be implemented easily.
 * 
 * @note The axes are not updated, while dragging the canvas
 * @sa QwtPlotZoomer, QwtPlotMagnifier
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotCachePanner 提供绘图画布的平移功能
 * @details QwtPlotCachePanner 是绘图画布的平移器，它会在
 *          将画布拖放到新位置后调整坐标轴的比例尺。
 * 
 *          与 QwtPlotZoomer 和 QwtPlotMagnifier 一起使用，可以轻松实现
 *          在 QwtPlot 部件上导航的强大方法。
 * 
 * @note 拖动画布时，坐标轴不会更新
 * @sa QwtPlotZoomer, QwtPlotMagnifier
 * \endif
 */
class QWT_EXPORT QwtPlotCachePanner : public QwtCachePanner
{
    Q_OBJECT

public:
    /// Constructor
    explicit QwtPlotCachePanner(QWidget*);
    /// Destructor
    virtual ~QwtPlotCachePanner();

    /// Get the canvas
    QWidget* canvas();
    /// Get the canvas (const)
    const QWidget* canvas() const;

    /// Get the plot
    QwtPlot* plot();
    /// Get the plot (const)
    const QwtPlot* plot() const;

    /// Set axis enabled state
    void setAxisEnabled(QwtAxisId axisId, bool on);
    /// Check if axis is enabled
    bool isAxisEnabled(QwtAxisId) const;

public Q_SLOTS:
    /// Move the canvas
    virtual void moveCanvas(int dx, int dy);

protected:
    /// Get the contents mask
    virtual QBitmap contentsMask() const override;
    /// Grab the canvas
    virtual QPixmap grab() const override;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
