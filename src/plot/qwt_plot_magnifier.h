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

#ifndef QWT_PLOT_MAGNIFIER_H
#define QWT_PLOT_MAGNIFIER_H

#include "qwt_global.h"
#include "qwt_axis_id.h"
#include "qwt_magnifier.h"

class QwtPlot;

/**
 * \if ENGLISH
 * @brief QwtPlotMagnifier provides zooming, by magnifying in steps
 * @details Using QwtPlotMagnifier a plot can be zoomed in/out in steps using
 *          keys, the mouse wheel or moving a mouse button in vertical direction.
 * 
 *          Together with QwtPlotZoomer and QwtPlotPanner it is possible to implement
 *          individual and powerful navigation of the plot canvas.
 * 
 * @sa QwtPlotZoomer, QwtPlotPanner, QwtPlot
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotMagnifier 通过逐步放大提供缩放功能
 * @details 使用 QwtPlotMagnifier，可以使用按键、鼠标滚轮或在垂直方向移动鼠标按钮来逐步放大/缩小绘图。
 * 
 *          与 QwtPlotZoomer 和 QwtPlotPanner 一起，可以实现绘图画布的个性化和强大导航。
 * 
 * @sa QwtPlotZoomer, QwtPlotPanner, QwtPlot
 * \endif
 */
class QWT_EXPORT QwtPlotMagnifier : public QwtMagnifier
{
    Q_OBJECT

  public:
// Constructor
    explicit QwtPlotMagnifier( QWidget* );

    // Destructor
    virtual ~QwtPlotMagnifier();

    // Enable or disable an axis for magnification
    void setAxisEnabled( QwtAxisId, bool on );

    // Check if an axis is enabled for magnification
    bool isAxisEnabled( QwtAxisId ) const;

    // Return the observed plot canvas
    QWidget* canvas();

    // Return the observed plot canvas (const version)
    const QWidget* canvas() const;

    // Return the plot widget containing the observed canvas
    QwtPlot* plot();

    // Return the plot widget containing the observed canvas (const version)
    const QwtPlot* plot() const;

public Q_SLOTS:
    // Rescale the plot axes by the given factor
    virtual void rescale( double factor ) override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
