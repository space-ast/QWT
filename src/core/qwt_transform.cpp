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

#include "qwt_transform.h"
#include "qwt_math.h"

//! Smallest allowed value for logarithmic scales: 1.0e-150
const double QwtLogTransform::LogMin = 1.0e-150;

//! Largest allowed value for logarithmic scales: 1.0e150
const double QwtLogTransform::LogMax = 1.0e150;

/**
 * @brief Default constructor
 */
QwtTransform::QwtTransform()
{
}

/**
 * @brief Destructor
 */
QwtTransform::~QwtTransform()
{
}

/**
 * @brief Bounded function - returns value unmodified
 * @param value Value to be bounded
 * @return Value unmodified
 */
double QwtTransform::bounded(double value) const
{
    return value;
}

/**
 * @brief Default constructor
 */
QwtNullTransform::QwtNullTransform() : QwtTransform()
{
}

/**
 * @brief Destructor
 */
QwtNullTransform::~QwtNullTransform()
{
}

/**
 * @brief Transform function - returns value unmodified
 * @param value Value to be transformed
 * @return Value unmodified
 */
double QwtNullTransform::transform(double value) const
{
    return value;
}

/**
 * @brief Inverse transform function - returns value unmodified
 * @param value Value to be transformed
 * @return Value unmodified
 */
double QwtNullTransform::invTransform(double value) const
{
    return value;
}

/**
 * @brief Clone of the transformation
 * @return New QwtNullTransform instance
 */
QwtTransform* QwtNullTransform::copy() const
{
    return new QwtNullTransform();
}

/**
 * @brief Default constructor
 */
QwtLogTransform::QwtLogTransform() : QwtTransform()
{
}

/**
 * @brief Destructor
 */
QwtLogTransform::~QwtLogTransform()
{
}

/**
 * @brief Transform function
 * @param value Value to be transformed
 * @return log( value )
 *
 */
double QwtLogTransform::transform(double value) const
{
    return std::log(value);
}

/**
 * @brief Inverse transform function
 * @param value Value to be transformed
 * @return exp( value )
 *
 */
double QwtLogTransform::invTransform(double value) const
{
    return std::exp(value);
}

/**
 * @brief Bounded function
 * @param value Value to be bounded
 * @return qBound( LogMin, value, LogMax )
 *
 */
double QwtLogTransform::bounded(double value) const
{
    return qBound(LogMin, value, LogMax);
}

/**
 * @brief Clone of the transformation
 * @return New QwtLogTransform instance
 *
 */
QwtTransform* QwtLogTransform::copy() const
{
    return new QwtLogTransform();
}

/**
 * @brief Constructor
 * @param exponent Exponent
 *
 */
QwtPowerTransform::QwtPowerTransform(double exponent) : QwtTransform(), m_exponent(exponent)
{
}

/**
 * @brief Destructor
 *
 */
QwtPowerTransform::~QwtPowerTransform()
{
}

/**
 * @brief Transform function
 * @param value Value to be transformed
 * @return Exponentiation preserving the sign
 *
 */
double QwtPowerTransform::transform(double value) const
{
    if (value < 0.0)
        return -std::pow(-value, 1.0 / m_exponent);
    else
        return std::pow(value, 1.0 / m_exponent);
}

/**
 * @brief Inverse transform function
 * @param value Value to be transformed
 * @return Inverse exponentiation preserving the sign
 *
 */
double QwtPowerTransform::invTransform(double value) const
{
    if (value < 0.0)
        return -std::pow(-value, m_exponent);
    else
        return std::pow(value, m_exponent);
}

/**
 * @brief Clone of the transformation
 * @return New QwtPowerTransform instance
 *
 */
QwtTransform* QwtPowerTransform::copy() const
{
    return new QwtPowerTransform(m_exponent);
}
