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

#include "qwt_spline_parametrization.h"

/**
 * @brief Constructor
 *
 * Creates a spline parametrization with the specified type.
 *
 * @param[in] type Parametrization type
 * @sa type()
 */
QwtSplineParametrization::QwtSplineParametrization( int type )
    : m_type( type )
{
}

/**
 * @brief Destructor
 */
QwtSplineParametrization::~QwtSplineParametrization()
{
}

/**
 * @brief Calculate the parameter value increment for 2 points
 *
 * @param[in] point1 First point
 * @param[in] point2 Second point
 *
 * @return Value increment
 */
double QwtSplineParametrization::valueIncrement(
    const QPointF& point1, const QPointF& point2 ) const
{
    switch( m_type )
    {
        case QwtSplineParametrization::ParameterX:
        {
            return valueIncrementX( point1, point2 );
        }
        case QwtSplineParametrization::ParameterY:
        {
            return valueIncrementY( point1, point2 );
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            return valueIncrementCentripetal( point1, point2 );
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            return valueIncrementChordal( point1, point2 );
        }
        case QwtSplineParametrization::ParameterManhattan:
        {
            return valueIncrementManhattan( point1, point2 );
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            return valueIncrementUniform( point1, point2 );
        }
        default:
        {
            return 1;
        }
    }
}

/**
 * @brief Get the parametrization type
 *
 * @return Parametrization type
 */
int QwtSplineParametrization::type() const
{
    return m_type;
}
