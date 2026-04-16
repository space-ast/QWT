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

#ifndef QWT_TRANSFORM_H
#define QWT_TRANSFORM_H

#include "qwt_global.h"

/**
 * \if ENGLISH
 * @brief A transformation between coordinate systems
 * @details QwtTransform manipulates values when being mapped between
 *          the scale and the paint device coordinate system.
 *          A transformation consists of 2 methods:
 *          - transform
 *          - invTransform
 *          where one is the inverse function of the other.
 *          When p1, p2 are the boundaries of the paint device coordinates
 *          and s1, s2 the boundaries of the scale, QwtScaleMap uses the
 *          following calculations:
 *          - p = p1 + ( p2 - p1 ) * ( T( s ) - T( s1 ) / ( T( s2 ) - T( s1 ) );
 *          - s = invT ( T( s1 ) + ( T( s2 ) - T( s1 ) ) * ( p - p1 ) / ( p2 - p1 ) );
 * \endif
 * \if CHINESE
 * @brief 坐标系之间的变换
 * @details QwtTransform 在值和绘制设备坐标系之间映射时操作值。
 *          变换由 2 个方法组成:
 *          - transform
 *          - invTransform
 *          其中一个是另一个的反函数。
 *          当 p1, p2 是绘制设备坐标的边界，s1, s2 是刻度的边界时，
 *          QwtScaleMap 使用以下计算:
 *          - p = p1 + ( p2 - p1 ) * ( T( s ) - T( s1 ) / ( T( s2 ) - T( s1 ) );
 *          - s = invT ( T( s1 ) + ( T( s2 ) - T( s1 ) ) * ( p - p1 ) / ( p2 - p1 ) );
 * \endif
 */
class QWT_EXPORT QwtTransform
{
public:
    //! Constructor
    QwtTransform();
    //! Destructor
    virtual ~QwtTransform();

    //! Modify value to be a valid value for the transformation
    virtual double bounded(double value) const;
    //! Transformation function
    virtual double transform(double value) const = 0;
    //! Inverse transformation function
    virtual double invTransform(double value) const = 0;
    //! Virtualized copy operation
    virtual QwtTransform* copy() const = 0;

private:
    Q_DISABLE_COPY(QwtTransform)
};

/**
 * \if ENGLISH
 * @brief Null transformation
 * @details QwtNullTransform returns the values unmodified.
 * \endif
 * \if CHINESE
 * @brief 空变换
 * @details QwtNullTransform 返回未修改的值。
 * \endif
 */
class QWT_EXPORT QwtNullTransform : public QwtTransform
{
public:
    //! Constructor
    QwtNullTransform();
    //! Destructor
    virtual ~QwtNullTransform();

    //! Transformation function - returns value unmodified
    virtual double transform(double value) const override;
    //! Inverse transformation function - returns value unmodified
    virtual double invTransform(double value) const override;
    //! Clone of the transformation
    virtual QwtTransform* copy() const override;
};
/**
 * \if ENGLISH
 * @brief Logarithmic transformation
 * @details QwtLogTransform modifies the values using log() and exp().
 * \note In the calculations of QwtScaleMap the base of the log function
 *       has no effect on the mapping. So QwtLogTransform can be used
 *       for log2(), log10() or any other logarithmic scale.
 * \endif
 * \if CHINESE
 * @brief 对数变换
 * @details QwtLogTransform 使用 log() 和 exp() 修改值。
 * \note 在 QwtScaleMap 的计算中，对数函数的底数对映射没有影响。
 *       因此 QwtLogTransform 可以用于 log2()、log10() 或任何其他对数刻度。
 * \endif
 */
class QWT_EXPORT QwtLogTransform : public QwtTransform
{
public:
    //! Constructor
    QwtLogTransform();
    //! Destructor
    virtual ~QwtLogTransform();

    //! Transformation function - log(value)
    virtual double transform(double value) const override;
    //! Inverse transformation function - exp(value)
    virtual double invTransform(double value) const override;
    //! Bounded function - qBound(LogMin, value, LogMax)
    virtual double bounded(double value) const override;
    //! Clone of the transformation
    virtual QwtTransform* copy() const override;

    //! Smallest allowed value for logarithmic scales: 1.0e-150
    static const double LogMin;
    //! Largest allowed value for logarithmic scales: 1.0e150
    static const double LogMax;
};

/**
 * \if ENGLISH
 * @brief A transformation using pow()
 * @details QwtPowerTransform preserves the sign of a value.
 *          F.e. a transformation with a factor of 2
 *          transforms a value of -3 to -9 and v.v. Thus QwtPowerTransform
 *          can be used for scales including negative values.
 * \endif
 * \if CHINESE
 * @brief 使用 pow() 的变换
 * @details QwtPowerTransform 保持值的符号。
 *          例如，因子为 2 的变换将 -3 变换为 -9，反之亦然。
 *          因此 QwtPowerTransform 可以用于包含负值的刻度。
 * \endif
 */
class QWT_EXPORT QwtPowerTransform : public QwtTransform
{
public:
    //! Constructor with exponent parameter
    explicit QwtPowerTransform(double exponent);
    //! Destructor
    virtual ~QwtPowerTransform();

    //! Transformation function - pow() preserving sign
    virtual double transform(double value) const override;
    //! Inverse transformation function - inverse pow() preserving sign
    virtual double invTransform(double value) const override;
    //! Clone of the transformation
    virtual QwtTransform* copy() const override;

private:
    const double m_exponent;
};

#endif
