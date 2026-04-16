/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_cache_panner.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_point_polar.h"

/**
 * \if ENGLISH
 * @brief Create a plot panner for a polar plot canvas
 * @param[in] canvas Plot canvas to be panned
 * \endif
 *
 * \if CHINESE
 * @brief 为极坐标绘图画布创建绘图平移器
 * @param[in] canvas 要平移的绘图画布
 * \endif
 */
QwtPolarCachePanner::QwtPolarCachePanner(QwtPolarCanvas* canvas) : QwtCachePanner(canvas)
{
    connect(this, SIGNAL(panned(int, int)), SLOT(movePlot(int, int)));
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
QwtPolarCachePanner::~QwtPolarCachePanner()
{
}

/**
 * \if ENGLISH
 * @brief Get the observed plot canvas
 * @return Observed plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图画布
 * @return 观察的绘图画布
 * \endif
 */
QwtPolarCanvas* QwtPolarCachePanner::canvas()
{
    return qobject_cast< QwtPolarCanvas* >(parent());
}

/**
 * \if ENGLISH
 * @brief Get the observed plot canvas (const version)
 * @return Observed plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图画布（常量版本）
 * @return 观察的绘图画布
 * \endif
 */
const QwtPolarCanvas* QwtPolarCachePanner::canvas() const
{
    return qobject_cast< const QwtPolarCanvas* >(parent());
}

/**
 * \if ENGLISH
 * @brief Get the observed plot
 * @return Observed plot
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图
 * @return 观察的绘图
 * \endif
 */
QwtPolarPlot* QwtPolarCachePanner::plot()
{
    QwtPolarCanvas* c = canvas();
    if (c)
        return c->plot();

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Get the observed plot (const version)
 * @return Observed plot
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图（常量版本）
 * @return 观察的绘图
 * \endif
 */
const QwtPolarPlot* QwtPolarCachePanner::plot() const
{
    const QwtPolarCanvas* c = canvas();
    if (c)
        return c->plot();

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Adjust the zoomed area according to dx/dy
 * @param[in] dx Pixel offset in x direction
 * @param[in] dy Pixel offset in y direction
 * @sa QwtPanner::panned(), QwtPolarPlot::zoom()
 * \endif
 *
 * \if CHINESE
 * @brief 根据 dx/dy 调整缩放区域
 * @param[in] dx X方向的像素偏移
 * @param[in] dy Y方向的像素偏移
 * @sa QwtPanner::panned(), QwtPolarPlot::zoom()
 * \endif
 */
void QwtPolarCachePanner::movePlot(int dx, int dy)
{
    QwtPolarPlot* plot = QwtPolarCachePanner::plot();
    if (plot == nullptr || (dx == 0 && dy == 0))
        return;

    const QwtScaleMap map = plot->scaleMap(QwtPolar::Radius);

    QwtPointPolar pos = plot->zoomPos();
    if (map.s1() <= map.s2()) {
        pos.setRadius(map.transform(map.s1() + pos.radius()) - map.p1());
        pos.setPoint(pos.toPoint() - QPointF(dx, -dy));
        pos.setRadius(map.invTransform(map.p1() + pos.radius()) - map.s1());
    } else {
        pos.setRadius(map.transform(map.s1() - pos.radius()) - map.p1());
        pos.setPoint(pos.toPoint() - QPointF(dx, -dy));
        pos.setRadius(map.s1() - map.invTransform(map.p1() + pos.radius()));
    }

    const bool doAutoReplot = plot->autoReplot();
    plot->setAutoReplot(false);

    plot->zoom(pos, plot->zoomFactor());

    plot->setAutoReplot(doAutoReplot);
    plot->replot();
}

/*!
   Block panning when the plot zoom factor is >= 1.0.

   \param event Mouse event
 */
void QwtPolarCachePanner::widgetMousePressEvent(QMouseEvent* event)
{
    const QwtPolarPlot* plot = QwtPolarCachePanner::plot();
    if (plot) {
        if (plot->zoomFactor() < 1.0)
            QwtCachePanner::widgetMousePressEvent(event);
    }
}
