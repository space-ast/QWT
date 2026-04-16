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

#ifndef QWT_CURVE_FITTER_H
#define QWT_CURVE_FITTER_H

#include "qwt_global.h"

class QPainterPath;
class QPolygonF;

/**
 * \if ENGLISH
 * @brief Abstract base class for curve fitting algorithms
 * \endif
 * \if CHINESE
 * @brief 曲线拟合算法的抽象基类
 * \endif
 */
class QWT_EXPORT QwtCurveFitter
{
  public:
    /*!
       \if ENGLISH
       \brief Preferred mode of the fitting algorithm
       \details Even if a QPainterPath can always be created from a QPolygonF,
                the overhead of the conversion can be avoided by indicating
                the preference of the implementation to the application code.
       \endif
       \if CHINESE
       \brief 拟合算法的首选模式
       \details 即使总是可以从 QPolygonF 创建 QPainterPath，
                但通过向应用程序代码指示实现的首选项，
                可以避免转换的开销。
       \endif
     */
    enum Mode
    {
        /*!
           \if ENGLISH
           The fitting algorithm creates a polygon - the implementation
           of fitCurvePath() simply wraps the polygon into a path.
           \sa QwtWeedingCurveFitter
           \endif
           \if CHINESE
           拟合算法创建多边形 - fitCurvePath() 的实现
           只是将多边形包装到路径中。
           \sa QwtWeedingCurveFitter
           \endif
         */
        Polygon,

        /*!
           \if ENGLISH
           The fitting algorithm creates a painter path - the implementation
           of fitCurve() extracts a polygon from the path.
           \sa QwtSplineCurveFitter
           \endif
           \if CHINESE
           拟合算法创建绘制器路径 - fitCurve() 的实现
           从路径中提取多边形。
           \sa QwtSplineCurveFitter
           \endif
         */
        Path
    };

    virtual ~QwtCurveFitter();

    // Get the preferred fitting mode
    Mode mode() const;

    // Find a curve which has the best fit to a series of data points
    virtual QPolygonF fitCurve( const QPolygonF& polygon ) const = 0;

    // Find a curve path which has the best fit to a series of data points
    virtual QPainterPath fitCurvePath( const QPolygonF& polygon ) const = 0;

  protected:
    explicit QwtCurveFitter( Mode mode );

  private:
    Q_DISABLE_COPY(QwtCurveFitter)

    const Mode m_mode;
};

#endif
