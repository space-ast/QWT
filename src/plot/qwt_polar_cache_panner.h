/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_PANNER_H
#define QWT_POLAR_PANNER_H

#include "qwt_global.h"
#include "qwt_cache_panner.h"
class QwtPolarPlot;
class QwtPolarCanvas;
/**
 * \if ENGLISH
 * @brief QwtPolarCachePanner provides panning of a polar plot canvas
 * @details QwtPolarPanner is a panner for a QwtPolarCanvas, that
 *          adjusts the visible area after dropping
 *          the canvas on its new position.
 * 
 *          Together with QwtPolarMagnifier individual ways
 *          of navigating on a QwtPolarPlot widget can be implemented easily.
 * 
 * @sa QwtPolarMagnifier
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPolarCachePanner 提供极坐标绘图画布的平移功能
 * @details QwtPolarPanner 是 QwtPolarCanvas 的平移器，
 *          在将画布放在新位置后调整可见区域。
 * 
 *          与 QwtPolarMagnifier 一起，可以轻松实现
 *          在 QwtPolarPlot 控件上导航的个性化方式。
 * 
 * @sa QwtPolarMagnifier
 * \endif
 */
class QWT_EXPORT QwtPolarCachePanner : public QwtCachePanner
{
    Q_OBJECT

public:
    /// Constructor
    explicit QwtPolarCachePanner(QwtPolarCanvas*);
    /// Destructor
    virtual ~QwtPolarCachePanner();

    /// Get the plot
    QwtPolarPlot* plot();
    /// Get the plot (const version)
    const QwtPolarPlot* plot() const;

    /// Get the canvas
    QwtPolarCanvas* canvas();
    /// Get the canvas (const version)
    const QwtPolarCanvas* canvas() const;

public Q_SLOTS:
    /// Move the plot
    virtual void movePlot(int dx, int dy);

protected:
    /// Handle mouse press events
    virtual void widgetMousePressEvent(QMouseEvent*) override;
};

#endif
