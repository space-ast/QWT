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

//! \if ENGLISH Smallest allowed value for logarithmic scales: 1.0e-150 \endif \if CHINESE 对数刻度允许的最小值：1.0e-150 \endif
const double QwtLogTransform::LogMin = 1.0e-150;

//! \if ENGLISH Largest allowed value for logarithmic scales: 1.0e150 \endif \if CHINESE 对数刻度允许的最大值：1.0e150 \endif
const double QwtLogTransform::LogMax = 1.0e150;

/**
 * \if ENGLISH
 * @brief Default constructor
 * \endif
 * \if CHINESE
 * @brief 默认构造函数
 * \endif
 */
QwtTransform::QwtTransform()
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtTransform::~QwtTransform()
{
}

/**
 * \if ENGLISH
 * @brief Bounded function - returns value unmodified
 * @param value Value to be bounded
 * @return Value unmodified
 * \endif
 * \if CHINESE
 * @brief 边界函数 - 返回未修改的值
 * @param value 要限制的值
 * @return 未修改的值
 * \endif
 */
double QwtTransform::bounded( double value ) const
{
    return value;
}

/**
 * \if ENGLISH
 * @brief Default constructor
 * \endif
 * \if CHINESE
 * @brief 默认构造函数
 * \endif
 */
QwtNullTransform::QwtNullTransform():
    QwtTransform()
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtNullTransform::~QwtNullTransform()
{
}

/**
 * \if ENGLISH
 * @brief Transform function - returns value unmodified
 * @param value Value to be transformed
 * @return Value unmodified
 * \endif
 * \if CHINESE
 * @brief 变换函数 - 返回未修改的值
 * @param value 要变换的值
 * @return 未修改的值
 * \endif
 */
double QwtNullTransform::transform( double value ) const
{
    return value;
}

/**
 * \if ENGLISH
 * @brief Inverse transform function - returns value unmodified
 * @param value Value to be transformed
 * @return Value unmodified
 * \endif
 * \if CHINESE
 * @brief 反变换函数 - 返回未修改的值
 * @param value 要变换的值
 * @return 未修改的值
 * \endif
 */
double QwtNullTransform::invTransform( double value ) const
{
    return value;
}

//! \if ENGLISH Clone of the transformation \endif \if CHINESE 变换的克隆 \endif
QwtTransform* QwtNullTransform::copy() const
{
    return new QwtNullTransform();
}

/**
 * \if ENGLISH
 * @brief Default constructor
 * \endif
 * \if CHINESE
 * @brief 默认构造函数
 * \endif
 */
QwtLogTransform::QwtLogTransform():
    QwtTransform()
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtLogTransform::~QwtLogTransform()
{
}

/**
 * \if ENGLISH
 * @brief Transform function
 * @param value Value to be transformed
 * @return log( value )
 * \endif
 *
 * \if CHINESE
 * @brief 变换函数
 * @param value 要变换的值
 * @return log( value )
 * \endif
 */
double QwtLogTransform::transform( double value ) const
{
    return std::log( value );
}

/**
 * \if ENGLISH
 * @brief Inverse transform function
 * @param value Value to be transformed
 * @return exp( value )
 * \endif
 *
 * \if CHINESE
 * @brief 反变换函数
 * @param value 要变换的值
 * @return exp( value )
 * \endif
 */
double QwtLogTransform::invTransform( double value ) const
{
    return std::exp( value );
}

/**
 * \if ENGLISH
 * @brief Bounded function
 * @param value Value to be bounded
 * @return qBound( LogMin, value, LogMax )
 * \endif
 *
 * \if CHINESE
 * @brief 边界函数
 * @param value 要限制的值
 * @return qBound( LogMin, value, LogMax )
 * \endif
 */
double QwtLogTransform::bounded( double value ) const
{
    return qBound( LogMin, value, LogMax );
}

/**
 * \if ENGLISH
 * @brief Clone of the transformation
 * @return New QwtLogTransform instance
 * \endif
 *
 * \if CHINESE
 * @brief 变换的克隆
 * @return 新的 QwtLogTransform 实例
 * \endif
 */
QwtTransform* QwtLogTransform::copy() const
{
    return new QwtLogTransform();
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param exponent Exponent
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param exponent 指数
 * \endif
 */
QwtPowerTransform::QwtPowerTransform( double exponent ):
    QwtTransform(),
    m_exponent( exponent )
{
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
QwtPowerTransform::~QwtPowerTransform()
{
}

/**
 * \if ENGLISH
 * @brief Transform function
 * @param value Value to be transformed
 * @return Exponentiation preserving the sign
 * \endif
 *
 * \if CHINESE
 * @brief 变换函数
 * @param value 要变换的值
 * @return 保持符号的幂运算
 * \endif
 */
double QwtPowerTransform::transform( double value ) const
{
    if ( value < 0.0 )
        return -std::pow( -value, 1.0 / m_exponent );
    else
        return std::pow( value, 1.0 / m_exponent );

}

/**
 * \if ENGLISH
 * @brief Inverse transform function
 * @param value Value to be transformed
 * @return Inverse exponentiation preserving the sign
 * \endif
 *
 * \if CHINESE
 * @brief 反变换函数
 * @param value 要变换的值
 * @return 保持符号的反幂运算
 * \endif
 */
double QwtPowerTransform::invTransform( double value ) const
{
    if ( value < 0.0 )
        return -std::pow( -value, m_exponent );
    else
        return std::pow( value, m_exponent );
}

/**
 * \if ENGLISH
 * @brief Clone of the transformation
 * @return New QwtPowerTransform instance
 * \endif
 *
 * \if CHINESE
 * @brief 变换的克隆
 * @return 新的 QwtPowerTransform 实例
 * \endif
 */
QwtTransform* QwtPowerTransform::copy() const
{
    return new QwtPowerTransform( m_exponent );
}
