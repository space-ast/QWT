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

#include "qwt_scale_engine.h"
#include "qwt_math.h"
#include "qwt_interval.h"
#include "qwt_transform.h"

#include <qdebug.h>

#include <limits>

static inline double qwtLog(double base, double value)
{
    return std::log(value) / std::log(base);
}

static inline QwtInterval qwtLogInterval(double base, const QwtInterval& interval)
{
    return QwtInterval(qwtLog(base, interval.minValue()), qwtLog(base, interval.maxValue()));
}

static inline QwtInterval qwtPowInterval(double base, const QwtInterval& interval)
{
    return QwtInterval(std::pow(base, interval.minValue()), std::pow(base, interval.maxValue()));
}

#if 1

// this version often doesn't find the best ticks: f.e for 15: 5, 10
static double qwtStepSize(double intervalSize, int maxSteps, uint base)
{
    const double minStep = QwtScaleArithmetic::divideInterval(intervalSize, maxSteps, base);

    if (minStep != 0.0) {
        // # ticks per interval
        const int numTicks = qwtCeil(qAbs(intervalSize / minStep)) - 1;

        // Do the minor steps fit into the interval?
        if (qwtFuzzyCompare((numTicks + 1) * qAbs(minStep), qAbs(intervalSize), intervalSize) > 0) {
            // The minor steps doesn't fit into the interval
            return 0.5 * intervalSize;
        }
    }

    return minStep;
}

#else

static double qwtStepSize(double intervalSize, int maxSteps, uint base)
{
    if (maxSteps <= 0)
        return 0.0;

    if (maxSteps > 2) {
        for (int numSteps = maxSteps; numSteps > 1; numSteps--) {
            const double stepSize = intervalSize / numSteps;

            const double p        = std::floor(std::log(stepSize) / std::log(base));
            const double fraction = std::pow(base, p);

            for (uint n = base; n > 1; n /= 2) {
                if (qFuzzyCompare(stepSize, n * fraction))
                    return stepSize;

                if (n == 3 && (base % 2) == 0) {
                    if (qFuzzyCompare(stepSize, 2 * fraction))
                        return stepSize;
                }
            }
        }
    }

    return intervalSize * 0.5;
}

#endif

static const double cs_eps_ = 1.0e-6;

/**
 * \if ENGLISH
 * @brief Ceil a value, relative to an interval
 * @param value Value to be ceiled
 * @param intervalSize Interval size
 * @return Rounded value
 * \sa floorEps()
 * \endif
 * \if CHINESE
 * @brief 相对于区间对值向上取整
 * @param value 要向上取整的值
 * @param intervalSize 区间大小
 * @return 舍入后的值
 * \sa floorEps()
 * \endif
 */
double QwtScaleArithmetic::ceilEps(double value, double intervalSize)
{
    const double eps = cs_eps_ * intervalSize;

    value = (value - eps) / intervalSize;
    return std::ceil(value) * intervalSize;
}

/**
 * \if ENGLISH
 * @brief Floor a value, relative to an interval
 * @param value Value to be floored
 * @param intervalSize Interval size
 * @return Rounded value
 * \sa ceilEps()
 * \endif
 * \if CHINESE
 * @brief 相对于区间对值向下取整
 * @param value 要向下取整的值
 * @param intervalSize 区间大小
 * @return 舍入后的值
 * \sa ceilEps()
 * \endif
 */
double QwtScaleArithmetic::floorEps(double value, double intervalSize)
{
    const double eps = cs_eps_ * intervalSize;

    value = (value + eps) / intervalSize;
    return std::floor(value) * intervalSize;
}

/**
 * \if ENGLISH
 * @brief Divide an interval into steps
 * @details Formula: \f$stepSize = (intervalSize - intervalSize * 10e^{-6}) / numSteps\f$
 * @param intervalSize Interval size
 * @param numSteps Number of steps
 * @return Step size
 * \endif
 * \if CHINESE
 * @brief 将区间划分为步长
 * @details 公式：\f$stepSize = (intervalSize - intervalSize * 10e^{-6}) / numSteps\f$
 * @param intervalSize 区间大小
 * @param numSteps 步数
 * @return 步长
 * \endif
 */
double QwtScaleArithmetic::divideEps(double intervalSize, double numSteps)
{
    if (numSteps == 0.0 || intervalSize == 0.0)
        return 0.0;

    return (intervalSize - (cs_eps_ * intervalSize)) / numSteps;
}

/**
 * \if ENGLISH
 * @brief Calculate a step size for a given interval
 * @param intervalSize Interval size
 * @param numSteps Number of steps
 * @param base Base for the division (usually 10)
 * @return Calculated step size
 * \endif
 * \if CHINESE
 * @brief 计算给定区间的步长
 * @param intervalSize 区间大小
 * @param numSteps 步数
 * @param base 除法基数（通常为 10）
 * @return 计算出的步长
 * \endif
 */
double QwtScaleArithmetic::divideInterval(double intervalSize, int numSteps, uint base)
{
    if (numSteps <= 0)
        return 0.0;

    const double v = QwtScaleArithmetic::divideEps(intervalSize, numSteps);
    if (v == 0.0)
        return 0.0;

    const double lx = qwtLog(base, std::fabs(v));
    const double p  = std::floor(lx);

    const double fraction = std::pow(base, lx - p);

    uint n = base;
    while ((n > 1) && (fraction <= n / 2))
        n /= 2;

    double stepSize = n * std::pow(base, p);
    if (v < 0)
        stepSize = -stepSize;

    return stepSize;
}

class QwtScaleEngine::PrivateData
{
public:
    PrivateData()
        : attributes(QwtScaleEngine::NoAttribute)
        , lowerMargin(0.0)
        , upperMargin(0.0)
        , referenceValue(0.0)
        , base(10)
        , transform(nullptr)
    {
    }

    ~PrivateData()
    {
        delete transform;
    }

    QwtScaleEngine::Attributes attributes;

    double lowerMargin;
    double upperMargin;

    double referenceValue;

    uint base;

    QwtTransform* transform;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param base Base of the scale engine
 * \sa setBase()
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param base 刻度引擎的基数
 * \sa setBase()
 * \endif
 */
QwtScaleEngine::QwtScaleEngine(uint base)
{
    m_data = new PrivateData;
    setBase(base);
}

//! Destructor
QwtScaleEngine::~QwtScaleEngine()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Assign a transformation
 * @param transform Transformation
 * @details The transformation object is used as factory for clones that are returned by transformation().
 *          The scale engine takes ownership of the transformation.
 * \sa QwtTransform::copy(), transformation()
 * \endif
 * \if CHINESE
 * @brief 设置变换
 * @param transform 变换对象
 * @details 变换对象用作工厂，用于生成 transformation() 返回的克隆。
 *          刻度引擎拥有变换对象的所有权。
 * \sa QwtTransform::copy(), transformation()
 * \endif
 */
void QwtScaleEngine::setTransformation(QwtTransform* transform)
{
    if (transform != m_data->transform) {
        delete m_data->transform;
        m_data->transform = transform;
    }
}

/**
 * \if ENGLISH
 * @brief Create and return a clone of the transformation of the engine
 * @details When the engine has no special transformation nullptr is returned, indicating no transformation.
 * @return A clone of the transformation
 * \sa setTransformation()
 * \endif
 * \if CHINESE
 * @brief 创建并返回引擎变换的克隆
 * @details 当引擎没有特殊变换时，返回 nullptr，表示无变换。
 * @return 变换的克隆
 * \sa setTransformation()
 * \endif
 */
QwtTransform* QwtScaleEngine::transformation() const
{
    QwtTransform* transform = nullptr;
    if (m_data->transform)
        transform = m_data->transform->copy();

    return transform;
}

/**
 * \if ENGLISH
 * @brief Return the margin at the lower end of the scale
 * @return The margin at the lower end
 * @details The default margin is 0.
 * \sa setMargins()
 * \endif
 * \if CHINESE
 * @brief 返回刻度下端的边距
 * @return 下端的边距
 * @details 默认边距为 0。
 * \sa setMargins()
 * \endif
 */
double QwtScaleEngine::lowerMargin() const
{
    return m_data->lowerMargin;
}

/**
 * \if ENGLISH
 * @brief Return the margin at the upper end of the scale
 * @return The margin at the upper end
 * @details The default margin is 0.
 * \sa setMargins()
 * \endif
 * \if CHINESE
 * @brief 返回刻度上端的边距
 * @return 上端的边距
 * @details 默认边距为 0。
 * \sa setMargins()
 * \endif
 */
double QwtScaleEngine::upperMargin() const
{
    return m_data->upperMargin;
}

/**
 * \if ENGLISH
 * @brief Specify margins at the scale's endpoints
 * @param lower Minimum distance between the scale's lower boundary and the smallest enclosed value
 * @param upper Minimum distance between the scale's upper boundary and the greatest enclosed value
 * @details Margins can be used to leave a minimum amount of space between the enclosed intervals and the boundaries of the scale.
 * @warning QwtLogScaleEngine measures the margins in decades.
 * \sa upperMargin(), lowerMargin()
 * \endif
 * \if CHINESE
 * @brief 指定刻度端点的边距
 * @param lower 刻度下边界与最小包含值之间的最小距离
 * @param upper 刻度上边界与最大包含值之间的最小距离
 * @details 边距可用于在包含的区间和刻度边界之间留出最小空间。
 * @warning QwtLogScaleEngine 以十倍为单位测量边距。
 * \sa upperMargin(), lowerMargin()
 * \endif
 */
void QwtScaleEngine::setMargins(double lower, double upper)
{
    m_data->lowerMargin = qwtMaxF(lower, 0.0);
    m_data->upperMargin = qwtMaxF(upper, 0.0);
}

/**
 * \if ENGLISH
 * @brief Calculate a step size for an interval size
 * @param intervalSize Interval size
 * @param numSteps Number of steps
 * @return Step size
 * \endif
 * \if CHINESE
 * @brief 计算区间大小的步长
 * @param intervalSize 区间大小
 * @param numSteps 步数
 * @return 步长
 * \endif
 */
double QwtScaleEngine::divideInterval(double intervalSize, int numSteps) const
{
    return QwtScaleArithmetic::divideInterval(intervalSize, numSteps, m_data->base);
}

/**
 * \if ENGLISH
 * @brief Check if an interval "contains" a value
 * @param interval Interval
 * @param value Value
 * @return True, when the value is inside the interval
 * \endif
 * \if CHINESE
 * @brief 检查区间是否"包含"一个值
 * @param interval 区间
 * @param value 值
 * @return 当值在区间内时返回 true
 * \endif
 */
bool QwtScaleEngine::contains(const QwtInterval& interval, double value) const
{
    if (!interval.isValid())
        return false;

    if (qwtFuzzyCompare(value, interval.minValue(), interval.width()) < 0)
        return false;

    if (qwtFuzzyCompare(value, interval.maxValue(), interval.width()) > 0)
        return false;

    return true;
}

/**
 * \if ENGLISH
 * @brief Remove ticks from a list, that are not inside an interval
 * @param ticks Tick list
 * @param interval Interval
 * @return Stripped tick list
 * \endif
 * \if CHINESE
 * @brief 从列表中移除不在区间内的刻度
 * @param ticks 刻度列表
 * @param interval 区间
 * @return 过滤后的刻度列表
 * \endif
 */
QList< double > QwtScaleEngine::strip(const QList< double >& ticks, const QwtInterval& interval) const
{
    if (!interval.isValid() || ticks.count() == 0)
        return QList< double >();

    if (contains(interval, ticks.first()) && contains(interval, ticks.last())) {
        return ticks;
    }

    QList< double > strippedTicks;
    for (int i = 0; i < ticks.count(); i++) {
        if (contains(interval, ticks[ i ]))
            strippedTicks += ticks[ i ];
    }
    return strippedTicks;
}

/**
 * \if ENGLISH
 * @brief Build an interval around a value
 * @details In case of v == 0.0 the interval is [-0.5, 0.5], otherwise it is [0.5 * v, 1.5 * v]
 * @param value Initial value
 * @return Calculated interval
 * \endif
 * \if CHINESE
 * @brief 围绕值构建区间
 * @details 当 v == 0.0 时，区间为 [-0.5, 0.5]，否则为 [0.5 * v, 1.5 * v]
 * @param value 初始值
 * @return 计算出的区间
 * \endif
 */
QwtInterval QwtScaleEngine::buildInterval(double value) const
{
    const double delta = (value == 0.0) ? 0.5 : qAbs(0.5 * value);
    const double max   = std::numeric_limits< double >::max();

    if (max - delta < value)
        return QwtInterval(max - delta, max);

    if (-max + delta > value)
        return QwtInterval(-max, -max + delta);

    return QwtInterval(value - delta, value + delta);
}

/**
 * \if ENGLISH
 * @brief Change a scale attribute
 * @param attribute Attribute to change
 * @param on On/Off
 * \sa Attribute, testAttribute()
 * \endif
 * \if CHINESE
 * @brief 更改刻度属性
 * @param attribute 要更改的属性
 * @param on 开/关
 * \sa Attribute, testAttribute()
 * \endif
 */
void QwtScaleEngine::setAttribute(Attribute attribute, bool on)
{
    if (on)
        m_data->attributes |= attribute;
    else
        m_data->attributes &= ~attribute;
}

/**
 * \if ENGLISH
 * @brief Test if an attribute is enabled
 * @param attribute Attribute to be tested
 * @return True, if attribute is enabled
 * \sa Attribute, setAttribute()
 * \endif
 * \if CHINESE
 * @brief 测试属性是否启用
 * @param attribute 要测试的属性
 * @return 如果属性已启用则返回 true
 * \sa Attribute, setAttribute()
 * \endif
 */
bool QwtScaleEngine::testAttribute(Attribute attribute) const
{
    return (m_data->attributes & attribute);
}

/**
 * \if ENGLISH
 * @brief Change the scale attribute
 * @param attributes Set scale attributes
 * \sa Attribute, attributes()
 * \endif
 * \if CHINESE
 * @brief 更改刻度属性
 * @param attributes 设置刻度属性
 * \sa Attribute, attributes()
 * \endif
 */
void QwtScaleEngine::setAttributes(Attributes attributes)
{
    m_data->attributes = attributes;
}

/**
 * \if ENGLISH
 * @brief Return scale attributes
 * @return Scale attributes
 * \sa Attribute, setAttributes(), testAttribute()
 * \endif
 * \if CHINESE
 * @brief 返回刻度属性
 * @return 刻度属性
 * \sa Attribute, setAttributes(), testAttribute()
 * \endif
 */
QwtScaleEngine::Attributes QwtScaleEngine::attributes() const
{
    return m_data->attributes;
}

/**
 * \if ENGLISH
 * @brief Specify a reference point
 * @param reference New reference value
 * @details The reference point is needed if options IncludeReference or Symmetric are active. Its default value is 0.0.
 * \sa Attribute
 * \endif
 * \if CHINESE
 * @brief 指定参考点
 * @param reference 新的参考值
 * @details 如果启用了 IncludeReference 或 Symmetric 选项，则需要参考点。默认值为 0.0。
 * \sa Attribute
 * \endif
 */
void QwtScaleEngine::setReference(double reference)
{
    m_data->referenceValue = reference;
}

/**
 * \if ENGLISH
 * @brief Return the reference value
 * @return The reference value
 * \sa setReference(), setAttribute()
 * \endif
 * \if CHINESE
 * @brief 返回参考值
 * @return 参考值
 * \sa setReference(), setAttribute()
 * \endif
 */
double QwtScaleEngine::reference() const
{
    return m_data->referenceValue;
}

/**
 * \if ENGLISH
 * @brief Set the base of the scale engine
 * @param base Base of the engine
 * @details While a base of 10 is what 99.9% of all applications need, certain scales might need a different base: f.e 2.
 *          The default setting is 10.
 * \sa base()
 * \endif
 * \if CHINESE
 * @brief 设置刻度引擎的基数
 * @param base 引擎的基数
 * @details 虽然 99.9% 的应用都需要基数 10，但某些刻度可能需要不同的基数：例如 2。
 *          默认设置为 10。
 * \sa base()
 * \endif
 */
void QwtScaleEngine::setBase(uint base)
{
    m_data->base = qMax(base, 2U);
}

/**
 * \if ENGLISH
 * @brief Return base of the scale engine
 * @return Base of the scale engine
 * \sa setBase()
 * \endif
 * \if CHINESE
 * @brief 返回刻度引擎的基数
 * @return 刻度引擎的基数
 * \sa setBase()
 * \endif
 */
uint QwtScaleEngine::base() const
{
    return m_data->base;
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param base Base of the scale engine
 * \sa setBase()
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param base 刻度引擎的基数
 * \sa setBase()
 * \endif
 */
QwtLinearScaleEngine::QwtLinearScaleEngine(uint base) : QwtScaleEngine(base)
{
}

//! Destructor
QwtLinearScaleEngine::~QwtLinearScaleEngine()
{
}

/**
 * \if ENGLISH
 * @brief Align and divide an interval
 * @param maxNumSteps Max. number of steps
 * @param x1 First limit of the interval (In/Out)
 * @param x2 Second limit of the interval (In/Out)
 * @param stepSize Step size (Out)
 * \sa setAttribute()
 * \endif
 * \if CHINESE
 * @brief 对齐并划分区间
 * @param maxNumSteps 最大步数
 * @param x1 区间的第一个限制（输入/输出）
 * @param x2 区间的第二个限制（输入/输出）
 * @param stepSize 步长（输出）
 * \sa setAttribute()
 * \endif
 */
void QwtLinearScaleEngine::autoScale(int maxNumSteps, double& x1, double& x2, double& stepSize) const
{
    QwtInterval interval(x1, x2);
    interval = interval.normalized();

    interval.setMinValue(interval.minValue() - lowerMargin());
    interval.setMaxValue(interval.maxValue() + upperMargin());

    if (testAttribute(QwtScaleEngine::Symmetric))
        interval = interval.symmetrize(reference());

    if (testAttribute(QwtScaleEngine::IncludeReference))
        interval = interval.extend(reference());

    if (interval.width() == 0.0)
        interval = buildInterval(interval.minValue());

    stepSize = QwtScaleArithmetic::divideInterval(interval.width(), qMax(maxNumSteps, 1), base());

    if (!testAttribute(QwtScaleEngine::Floating))
        interval = align(interval, stepSize);

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if (testAttribute(QwtScaleEngine::Inverted)) {
        qSwap(x1, x2);
        stepSize = -stepSize;
    }
}

/**
 * \if ENGLISH
 * @brief Calculate a scale division for an interval
 * @param x1 First interval limit
 * @param x2 Second interval limit
 * @param maxMajorSteps Maximum for the number of major steps
 * @param maxMinorSteps Maximum number of minor steps
 * @param stepSize Step size. If stepSize == 0, the engine calculates one.
 * @return Calculated scale division
 * \endif
 * \if CHINESE
 * @brief 计算区间的刻度划分
 * @param x1 第一个区间限制
 * @param x2 第二个区间限制
 * @param maxMajorSteps 主刻度的最大步数
 * @param maxMinorSteps 次刻度的最大步数
 * @param stepSize 步长。如果 stepSize == 0，引擎会自动计算一个。
 * @return 计算出的刻度划分
 * \endif
 */
QwtScaleDiv QwtLinearScaleEngine::divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize) const
{
    QwtInterval interval = QwtInterval(x1, x2).normalized();

    if (interval.widthL() > std::numeric_limits< double >::max()) {
        qWarning() << "QwtLinearScaleEngine::divideScale: overflow";
        return QwtScaleDiv();
    }

    if (interval.width() <= 0)
        return QwtScaleDiv();

    stepSize = qAbs(stepSize);
    if (stepSize == 0.0) {
        if (maxMajorSteps < 1)
            maxMajorSteps = 1;

        stepSize = QwtScaleArithmetic::divideInterval(interval.width(), maxMajorSteps, base());
    }

    QwtScaleDiv scaleDiv;

    if (stepSize != 0.0) {
        QList< double > ticks[ QwtScaleDiv::NTickTypes ];
        buildTicks(interval, stepSize, maxMinorSteps, ticks);

        scaleDiv = QwtScaleDiv(interval, ticks);
    }

    if (x1 > x2)
        scaleDiv.invert();

    return scaleDiv;
}

/**
 * \if ENGLISH
 * @brief Calculate ticks for an interval
 * @param interval Interval
 * @param stepSize Step size
 * @param maxMinorSteps Maximum number of minor steps
 * @param ticks Arrays to be filled with the calculated ticks
 * \sa buildMajorTicks(), buildMinorTicks()
 * \endif
 * \if CHINESE
 * @brief 计算区间的刻度
 * @param interval 区间
 * @param stepSize 步长
 * @param maxMinorSteps 次刻度的最大步数
 * @param ticks 用于填充计算出的刻度的数组
 * \sa buildMajorTicks(), buildMinorTicks()
 * \endif
 */
void QwtLinearScaleEngine::buildTicks(const QwtInterval& interval,
                                      double stepSize,
                                      int maxMinorSteps,
                                      QList< double > ticks[ QwtScaleDiv::NTickTypes ]) const
{
    const QwtInterval boundingInterval = align(interval, stepSize);

    ticks[ QwtScaleDiv::MajorTick ] = buildMajorTicks(boundingInterval, stepSize);

    if (maxMinorSteps > 0) {
        buildMinorTicks(ticks[ QwtScaleDiv::MajorTick ],
                        maxMinorSteps,
                        stepSize,
                        ticks[ QwtScaleDiv::MinorTick ],
                        ticks[ QwtScaleDiv::MediumTick ]);
    }

    for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
        ticks[ i ] = strip(ticks[ i ], interval);

        // ticks very close to 0.0 are explicitly set to 0.0

        for (int j = 0; j < ticks[ i ].count(); j++) {
            if (qwtFuzzyCompare(ticks[ i ][ j ], 0.0, stepSize) == 0)
                ticks[ i ][ j ] = 0.0;
        }
    }
}

/**
 * \if ENGLISH
 * @brief Calculate major ticks for an interval
 * @param interval Interval
 * @param stepSize Step size
 * @return Calculated ticks
 * \endif
 * \if CHINESE
 * @brief 计算区间的主刻度
 * @param interval 区间
 * @param stepSize 步长
 * @return 计算出的刻度
 * \endif
 */
QList< double > QwtLinearScaleEngine::buildMajorTicks(const QwtInterval& interval, double stepSize) const
{
    int numTicks = qRound(interval.width() / stepSize) + 1;
    if (numTicks > 10000)
        numTicks = 10000;

    QList< double > ticks;
    ticks.reserve(numTicks);

    ticks += interval.minValue();
    for (int i = 1; i < numTicks - 1; i++)
        ticks += interval.minValue() + i * stepSize;
    ticks += interval.maxValue();

    return ticks;
}

/**
 * \if ENGLISH
 * @brief Calculate minor/medium ticks for major ticks
 * @param majorTicks Major ticks
 * @param maxMinorSteps Maximum number of minor steps
 * @param stepSize Step size
 * @param minorTicks Array to be filled with the calculated minor ticks
 * @param mediumTicks Array to be filled with the calculated medium ticks
 * \endif
 * \if CHINESE
 * @brief 计算主刻度的次刻度/中刻度
 * @param majorTicks 主刻度
 * @param maxMinorSteps 次刻度的最大步数
 * @param stepSize 步长
 * @param minorTicks 用于填充计算出的次刻度的数组
 * @param mediumTicks 用于填充计算出的中刻度的数组
 * \endif
 */
void QwtLinearScaleEngine::buildMinorTicks(const QList< double >& majorTicks,
                                           int maxMinorSteps,
                                           double stepSize,
                                           QList< double >& minorTicks,
                                           QList< double >& mediumTicks) const
{
    double minStep = qwtStepSize(stepSize, maxMinorSteps, base());
    if (minStep == 0.0)
        return;

    // # ticks per interval
    const int numTicks = qwtCeil(qAbs(stepSize / minStep)) - 1;

    int medIndex = -1;
    if (numTicks % 2)
        medIndex = numTicks / 2;

    // calculate minor ticks

    for (int i = 0; i < majorTicks.count(); i++) {
        double val = majorTicks[ i ];
        for (int k = 0; k < numTicks; k++) {
            val += minStep;

            double alignedValue = val;
            if (qwtFuzzyCompare(val, 0.0, stepSize) == 0)
                alignedValue = 0.0;

            if (k == medIndex)
                mediumTicks += alignedValue;
            else
                minorTicks += alignedValue;
        }
    }
}

/**
 * \if ENGLISH
 * @brief Align an interval to a step size
 * @details The limits of an interval are aligned that both are integer multiples of the step size.
 * @param interval Interval
 * @param stepSize Step size
 * @return Aligned interval
 * \endif
 * \if CHINESE
 * @brief 将区间对齐到步长
 * @details 区间的限制都对齐为步长的整数倍。
 * @param interval 区间
 * @param stepSize 步长
 * @return 对齐后的区间
 * \endif
 */
QwtInterval QwtLinearScaleEngine::align(const QwtInterval& interval, double stepSize) const
{
    double x1 = interval.minValue();
    double x2 = interval.maxValue();

    // when there is no rounding beside some effect, when
    // calculating with doubles, we keep the original value

    const double eps = 0.000000000001;  // since Qt 4.8: qFuzzyIsNull
    const double max = std::numeric_limits< double >::max();

    if (-max + stepSize <= x1) {
        const double x = QwtScaleArithmetic::floorEps(x1, stepSize);
        if (qAbs(x) <= eps || !qFuzzyCompare(x1, x))
            x1 = x;
    }

    if (max - stepSize >= x2) {
        const double x = QwtScaleArithmetic::ceilEps(x2, stepSize);
        if (qAbs(x) <= eps || !qFuzzyCompare(x2, x))
            x2 = x;
    }

    return QwtInterval(x1, x2);
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param base Base of the scale engine
 * \sa setBase()
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param base 刻度引擎的基数
 * \sa setBase()
 * \endif
 */
QwtLogScaleEngine::QwtLogScaleEngine(uint base) : QwtScaleEngine(base)
{
    setTransformation(new QwtLogTransform());
}

//! Destructor
QwtLogScaleEngine::~QwtLogScaleEngine()
{
}

/**
 * \if ENGLISH
 * @brief Align and divide an interval
 * @param maxNumSteps Max. number of steps
 * @param x1 First limit of the interval (In/Out)
 * @param x2 Second limit of the interval (In/Out)
 * @param stepSize Step size (Out)
 * @sa QwtScaleEngine::setAttribute()
 * \endif
 * \if CHINESE
 * @brief 对齐并划分区间
 * @param maxNumSteps 最大步数
 * @param x1 区间的第一个限制（输入/输出）
 * @param x2 区间的第二个限制（输入/输出）
 * @param stepSize 步长（输出）
 * @sa QwtScaleEngine::setAttribute()
 * \endif
 */
void QwtLogScaleEngine::autoScale(int maxNumSteps, double& x1, double& x2, double& stepSize) const
{
    if (x1 > x2)
        qSwap(x1, x2);

    const double logBase = base();

    QwtInterval interval(x1 / std::pow(logBase, lowerMargin()), x2 * std::pow(logBase, upperMargin()));

    if (interval.maxValue() / interval.minValue() < logBase) {
        // scale width is less than one step -> try to build a linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes(attributes());
        linearScaler.setReference(reference());
        linearScaler.setMargins(lowerMargin(), upperMargin());

        linearScaler.autoScale(maxNumSteps, x1, x2, stepSize);

        QwtInterval linearInterval = QwtInterval(x1, x2).normalized();
        linearInterval             = linearInterval.limited(QwtLogTransform::LogMin, QwtLogTransform::LogMax);

        if (linearInterval.maxValue() / linearInterval.minValue() < logBase) {
            stepSize = 0.0;
            return;
        }
    }

    double logRef = 1.0;
    if (reference() > QwtLogTransform::LogMin / 2)
        logRef = qwtMinF(reference(), QwtLogTransform::LogMax / 2);

    if (testAttribute(QwtScaleEngine::Symmetric)) {
        const double delta = qwtMaxF(interval.maxValue() / logRef, logRef / interval.minValue());
        interval.setInterval(logRef / delta, logRef * delta);
    }

    if (testAttribute(QwtScaleEngine::IncludeReference))
        interval = interval.extend(logRef);

    interval = interval.limited(QwtLogTransform::LogMin, QwtLogTransform::LogMax);

    if (interval.width() == 0.0)
        interval = buildInterval(interval.minValue());

    stepSize = divideInterval(qwtLogInterval(logBase, interval).width(), qMax(maxNumSteps, 1));
    if (stepSize < 1.0)
        stepSize = 1.0;

    if (!testAttribute(QwtScaleEngine::Floating))
        interval = align(interval, stepSize);

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if (testAttribute(QwtScaleEngine::Inverted)) {
        qSwap(x1, x2);
        stepSize = -stepSize;
    }
}

/**
 * \if ENGLISH
 * @brief Calculate a scale division for an interval
 * @param x1 First interval limit
 * @param x2 Second interval limit
 * @param maxMajorSteps Maximum for the number of major steps
 * @param maxMinorSteps Maximum number of minor steps
 * @param stepSize Step size. If stepSize == 0, the engine calculates one.
 * @return Calculated scale division
 * \endif
 * \if CHINESE
 * @brief 计算区间的刻度划分
 * @param x1 第一个区间限制
 * @param x2 第二个区间限制
 * @param maxMajorSteps 主刻度的最大步数
 * @param maxMinorSteps 次刻度的最大步数
 * @param stepSize 步长。如果 stepSize == 0，引擎会自动计算一个。
 * @return 计算出的刻度划分
 * \endif
 */
QwtScaleDiv QwtLogScaleEngine::divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps, double stepSize) const
{
    QwtInterval interval = QwtInterval(x1, x2).normalized();
    interval             = interval.limited(QwtLogTransform::LogMin, QwtLogTransform::LogMax);

    if (interval.width() <= 0)
        return QwtScaleDiv();

    const double logBase = base();

    if (interval.maxValue() / interval.minValue() < logBase) {
        // scale width is less than one decade -> build linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes(attributes());
        linearScaler.setReference(reference());
        linearScaler.setMargins(lowerMargin(), upperMargin());

        return linearScaler.divideScale(x1, x2, maxMajorSteps, maxMinorSteps, 0.0);
    }

    stepSize = qAbs(stepSize);
    if (stepSize == 0.0) {
        if (maxMajorSteps < 1)
            maxMajorSteps = 1;

        stepSize = divideInterval(qwtLogInterval(logBase, interval).width(), maxMajorSteps);
        if (stepSize < 1.0)
            stepSize = 1.0;  // major step must be >= 1 decade
    }

    QwtScaleDiv scaleDiv;
    if (stepSize != 0.0) {
        QList< double > ticks[ QwtScaleDiv::NTickTypes ];
        buildTicks(interval, stepSize, maxMinorSteps, ticks);

        scaleDiv = QwtScaleDiv(interval, ticks);
    }

    if (x1 > x2)
        scaleDiv.invert();

    return scaleDiv;
}

/**
 * \if ENGLISH
 * @brief Calculate ticks for an interval
 * @param interval Interval
 * @param stepSize Step size
 * @param maxMinorSteps Maximum number of minor steps
 * @param ticks Arrays to be filled with the calculated ticks
 * \sa buildMajorTicks(), buildMinorTicks()
 * \endif
 * \if CHINESE
 * @brief 计算区间的刻度
 * @param interval 区间
 * @param stepSize 步长
 * @param maxMinorSteps 次刻度的最大步数
 * @param ticks 用于填充计算出的刻度的数组
 * \sa buildMajorTicks(), buildMinorTicks()
 * \endif
 */
void QwtLogScaleEngine::buildTicks(const QwtInterval& interval,
                                   double stepSize,
                                   int maxMinorSteps,
                                   QList< double > ticks[ QwtScaleDiv::NTickTypes ]) const
{
    const QwtInterval boundingInterval = align(interval, stepSize);

    ticks[ QwtScaleDiv::MajorTick ] = buildMajorTicks(boundingInterval, stepSize);

    if (maxMinorSteps > 0) {
        buildMinorTicks(ticks[ QwtScaleDiv::MajorTick ],
                        maxMinorSteps,
                        stepSize,
                        ticks[ QwtScaleDiv::MinorTick ],
                        ticks[ QwtScaleDiv::MediumTick ]);
    }

    for (int i = 0; i < QwtScaleDiv::NTickTypes; i++)
        ticks[ i ] = strip(ticks[ i ], interval);
}

/**
 * \if ENGLISH
 * @brief Calculate major ticks for an interval
 * @param interval Interval
 * @param stepSize Step size
 * @return Calculated ticks
 * \endif
 * \if CHINESE
 * @brief 计算区间的主刻度
 * @param interval 区间
 * @param stepSize 步长
 * @return 计算出的刻度
 * \endif
 */
QList< double > QwtLogScaleEngine::buildMajorTicks(const QwtInterval& interval, double stepSize) const
{
    double width = qwtLogInterval(base(), interval).width();

    int numTicks = qRound(width / stepSize) + 1;
    if (numTicks > 10000)
        numTicks = 10000;

    const double lxmin = std::log(interval.minValue());
    const double lxmax = std::log(interval.maxValue());
    const double lstep = (lxmax - lxmin) / double(numTicks - 1);

    QList< double > ticks;
    ticks.reserve(numTicks);

    ticks += interval.minValue();

    for (int i = 1; i < numTicks - 1; i++)
        ticks += std::exp(lxmin + double(i) * lstep);

    ticks += interval.maxValue();

    return ticks;
}

/**
 * \if ENGLISH
 * @brief Calculate minor/medium ticks for major ticks
 * @param majorTicks Major ticks
 * @param maxMinorSteps Maximum number of minor steps
 * @param stepSize Step size
 * @param minorTicks Array to be filled with the calculated minor ticks
 * @param mediumTicks Array to be filled with the calculated medium ticks
 * \endif
 * \if CHINESE
 * @brief 计算主刻度的次刻度/中刻度
 * @param majorTicks 主刻度
 * @param maxMinorSteps 次刻度的最大步数
 * @param stepSize 步长
 * @param minorTicks 用于填充计算出的次刻度的数组
 * @param mediumTicks 用于填充计算出的中刻度的数组
 * \endif
 */
void QwtLogScaleEngine::buildMinorTicks(const QList< double >& majorTicks,
                                        int maxMinorSteps,
                                        double stepSize,
                                        QList< double >& minorTicks,
                                        QList< double >& mediumTicks) const
{
    const double logBase = base();

    if (stepSize < 1.1)  // major step width is one base
    {
        double minStep = divideInterval(stepSize, maxMinorSteps + 1);
        if (minStep == 0.0)
            return;

        const int numSteps = qRound(stepSize / minStep);

        int mediumTickIndex = -1;
        if ((numSteps > 2) && (numSteps % 2 == 0))
            mediumTickIndex = numSteps / 2;

        for (int i = 0; i < majorTicks.count() - 1; i++) {
            const double v = majorTicks[ i ];
            const double s = logBase / numSteps;

            if (s >= 1.0) {
                if (!qFuzzyCompare(s, 1.0))
                    minorTicks += v * s;

                for (int j = 2; j < numSteps; j++) {
                    minorTicks += v * j * s;
                }
            } else {
                for (int j = 1; j < numSteps; j++) {
                    const double tick = v + j * v * (logBase - 1) / numSteps;
                    if (j == mediumTickIndex)
                        mediumTicks += tick;
                    else
                        minorTicks += tick;
                }
            }
        }
    } else {
        double minStep = divideInterval(stepSize, maxMinorSteps);
        if (minStep == 0.0)
            return;

        if (minStep < 1.0)
            minStep = 1.0;

        // # subticks per interval
        int numTicks = qRound(stepSize / minStep) - 1;

        // Do the minor steps fit into the interval?
        if (qwtFuzzyCompare((numTicks + 1) * minStep, stepSize, stepSize) > 0) {
            numTicks = 0;
        }

        if (numTicks < 1)
            return;

        int mediumTickIndex = -1;
        if ((numTicks > 2) && (numTicks % 2))
            mediumTickIndex = numTicks / 2;

        // substep factor = base^substeps
        const qreal minFactor = qwtMaxF(std::pow(logBase, minStep), logBase);

        for (int i = 0; i < majorTicks.count(); i++) {
            double tick = majorTicks[ i ];
            for (int j = 0; j < numTicks; j++) {
                tick *= minFactor;

                if (j == mediumTickIndex)
                    mediumTicks += tick;
                else
                    minorTicks += tick;
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Align an interval to a step size
 * @param interval Interval
 * @param stepSize Step size
 * @return Aligned interval
 * \endif
 * \if CHINESE
 * @brief 将区间对齐到步长
 * @param interval 区间
 * @param stepSize 步长
 * @return 对齐后的区间
 * \endif
 */
QwtInterval QwtLogScaleEngine::align(const QwtInterval& interval, double stepSize) const
{
    const QwtInterval intv = qwtLogInterval(base(), interval);

    double x1 = QwtScaleArithmetic::floorEps(intv.minValue(), stepSize);
    if (qwtFuzzyCompare(interval.minValue(), x1, stepSize) == 0)
        x1 = interval.minValue();

    double x2 = QwtScaleArithmetic::ceilEps(intv.maxValue(), stepSize);
    if (qwtFuzzyCompare(interval.maxValue(), x2, stepSize) == 0)
        x2 = interval.maxValue();

    return qwtPowInterval(base(), QwtInterval(x1, x2));
}
