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

#include "qwt_spline_curve_fitter.h"
#include "qwt_spline_local.h"
#include "qwt_spline_parametrization.h"

#include <qpolygon.h>
#include <qpainterpath.h>

/**
 * \if ENGLISH
 * @brief Constructor
 *
 * Creates a spline curve fitter with a cardinal spline using uniform parametrization.
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 *
 * 创建一个使用均匀参数化的基数样条的样条曲线拟合器。
 * \endif
 */
QwtSplineCurveFitter::QwtSplineCurveFitter() : QwtCurveFitter(QwtCurveFitter::Path)
{
    m_spline = new QwtSplineLocal(QwtSplineLocal::Cardinal);
    m_spline->setParametrization(QwtSplineParametrization::ParameterUniform);
}

/**
 * \if ENGLISH
 * @brief Destructor
 *
 * Deletes the internal spline object.
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 *
 * 删除内部样条对象。
 * \endif
 */
QwtSplineCurveFitter::~QwtSplineCurveFitter()
{
    delete m_spline;
}

/**
 * \if ENGLISH
 * @brief Assign a spline
 *
 * The spline needs to be allocated by new and will be deleted
 * in the destructor of the fitter.
 *
 * @param spline Spline
 * @sa spline()
 * \endif
 *
 * \if CHINESE
 * @brief 分配样条
 *
 * 样条需要使用 new 分配，并将在拟合器的析构函数中被删除。
 *
 * @param spline 样条
 * @sa spline()
 * \endif
 */
void QwtSplineCurveFitter::setSpline(QwtSpline* spline)
{
    if (m_spline == spline)
        return;

    delete m_spline;
    m_spline = spline;
}

/**
 * \if ENGLISH
 * @brief Get the spline (const version)
 *
 * @return Spline
 * @sa setSpline()
 * \endif
 *
 * \if CHINESE
 * @brief 获取样条（const 版本）
 *
 * @return 样条
 * @sa setSpline()
 * \endif
 */
const QwtSpline* QwtSplineCurveFitter::spline() const
{
    return m_spline;
}

/**
 * \if ENGLISH
 * @brief Get the spline (non-const version)
 *
 * @return Spline
 * @sa setSpline()
 * \endif
 *
 * \if CHINESE
 * @brief 获取样条（非 const 版本）
 *
 * @return 样条
 * @sa setSpline()
 * \endif
 */
QwtSpline* QwtSplineCurveFitter::spline()
{
    return m_spline;
}

/**
 * \if ENGLISH
 * @brief Find a curve which has the best fit to a series of data points
 *
 * @param points Series of data points
 * @return Fitted Curve
 *
 * @sa fitCurvePath()
 * \endif
 *
 * \if CHINESE
 * @brief 找到最适合一系列数据点的曲线
 *
 * @param points 数据点系列
 * @return 拟合曲线
 *
 * @sa fitCurvePath()
 * \endif
 */
QPolygonF QwtSplineCurveFitter::fitCurve(const QPolygonF& points) const
{
    const QPainterPath path = fitCurvePath(points);

    const QList< QPolygonF > subPaths = path.toSubpathPolygons();
    if (subPaths.size() == 1)
        subPaths.first();

    return QPolygonF();
}

/**
 * \if ENGLISH
 * @brief Find a curve path which has the best fit to a series of data points
 *
 * @param points Series of data points
 * @return Fitted Curve
 *
 * @sa fitCurve()
 * \endif
 *
 * \if CHINESE
 * @brief 找到最适合一系列数据点的曲线路径
 *
 * @param points 数据点系列
 * @return 拟合曲线
 *
 * @sa fitCurve()
 * \endif
 */
QPainterPath QwtSplineCurveFitter::fitCurvePath(const QPolygonF& points) const
{
    QPainterPath path;

    if (m_spline)
        path = m_spline->painterPath(points);

    return path;
}
