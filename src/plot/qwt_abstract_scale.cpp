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

#include "qwt_abstract_scale.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_interval.h"

#include <qcoreevent.h>

class QwtAbstractScale::PrivateData
{
public:
    PrivateData() : maxMajor(5), maxMinor(3), stepSize(0.0)
    {
        scaleEngine = new QwtLinearScaleEngine();
        scaleDraw   = new QwtScaleDraw();
    }

    ~PrivateData()
    {
        delete scaleEngine;
        delete scaleDraw;
    }

    QwtScaleEngine* scaleEngine;
    QwtAbstractScaleDraw* scaleDraw;

    int maxMajor;
    int maxMinor;
    double stepSize;
};

/**
 * \if ENGLISH
 * @brief Constructor for QwtAbstractScale
 * @param parent Parent widget
 * @details Creates a default QwtScaleDraw and a QwtLinearScaleEngine.
 *          The initial scale boundaries are set to [ 0.0, 100.0 ]
 *          The scaleStepSize() is initialized to 0.0, scaleMaxMajor() to 5
 *          and scaleMaxMinor to 3.
 * \endif
 * \if CHINESE
 * @brief QwtAbstractScale 构造函数
 * @param parent 父控件
 * @details 创建默认的 QwtScaleDraw 和 QwtLinearScaleEngine。
 *          初始刻度边界设置为 [ 0.0, 100.0 ]
 *          scaleStepSize() 初始化为 0.0，scaleMaxMajor() 为 5，
 *          scaleMaxMinor 为 3。
 * \endif
 */
QwtAbstractScale::QwtAbstractScale(QWidget* parent) : QWidget(parent)
{
    m_data = new PrivateData;
    rescale(0.0, 100.0, m_data->stepSize);
}

/**
 * \if ENGLISH
 * @brief Destructor for QwtAbstractScale
 * \endif
 * \if CHINESE
 * @brief QwtAbstractScale 析构函数
 * \endif
 */
QwtAbstractScale::~QwtAbstractScale()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the lower bound of the scale
 * @param value Lower bound value
 * \sa lowerBound(), setScale(), setUpperBound()
 * \note For inverted scales, the lower bound is greater than the upper bound
 * \endif
 * \if CHINESE
 * @brief 设置刻度的下界
 * @param value 下界值
 * \sa lowerBound(), setScale(), setUpperBound()
 * \note 对于反向刻度，下界大于上界
 * \endif
 */
void QwtAbstractScale::setLowerBound(double value)
{
    setScale(value, upperBound());
}

/**
 * \if ENGLISH
 * @brief Return the lower bound of the scale
 * @return Lower bound value
 * \sa setLowerBound(), setScale(), upperBound()
 * \endif
 * \if CHINESE
 * @brief 返回刻度的下界
 * @return 下界值
 * \sa setLowerBound(), setScale(), upperBound()
 * \endif
 */
double QwtAbstractScale::lowerBound() const
{
    return m_data->scaleDraw->scaleDiv().lowerBound();
}

/**
 * \if ENGLISH
 * @brief Set the upper bound of the scale
 * @param value Upper bound value
 * \sa upperBound(), setScale(), setLowerBound()
 * \note For inverted scales, the lower bound is greater than the upper bound
 * \endif
 * \if CHINESE
 * @brief 设置刻度的上界
 * @param value 上界值
 * \sa upperBound(), setScale(), setLowerBound()
 * \note 对于反向刻度，下界大于上界
 * \endif
 */
void QwtAbstractScale::setUpperBound(double value)
{
    setScale(lowerBound(), value);
}

/**
 * \if ENGLISH
 * @brief Return the upper bound of the scale
 * @return Upper bound value
 * \sa setUpperBound(), setScale(), lowerBound()
 * \endif
 * \if CHINESE
 * @brief 返回刻度的上界
 * @return 上界值
 * \sa setUpperBound(), setScale(), lowerBound()
 * \endif
 */
double QwtAbstractScale::upperBound() const
{
    return m_data->scaleDraw->scaleDiv().upperBound();
}

/**
 * \if ENGLISH
 * @brief Specify a scale by interval bounds
 * @details Define a scale by an interval.
 *          The ticks are calculated using scaleMaxMinor(),
 *          scaleMaxMajor() and scaleStepSize().
 * @param lowerBound Lower limit of the scale interval
 * @param upperBound Upper limit of the scale interval
 * \note For inverted scales, the lower bound is greater than the upper bound
 * \endif
 * \if CHINESE
 * @brief 通过区间边界指定刻度
 * @details 通过区间定义刻度。
 *          刻度使用 scaleMaxMinor()、scaleMaxMajor() 和 scaleStepSize() 计算。
 * @param lowerBound 刻度区间的下界
 * @param upperBound 刻度区间的上界
 * \note 对于反向刻度，下界大于上界
 * \endif
 */
void QwtAbstractScale::setScale(double lowerBound, double upperBound)
{
    rescale(lowerBound, upperBound, m_data->stepSize);
}

/**
 * \if ENGLISH
 * @brief Specify a scale by interval
 * @details Define a scale by an interval.
 *          The ticks are calculated using scaleMaxMinor(),
 *          scaleMaxMajor() and scaleStepSize().
 * @param interval Interval object
 * \endif
 * \if CHINESE
 * @brief 通过区间对象指定刻度
 * @details 通过区间定义刻度。
 *          刻度使用 scaleMaxMinor()、scaleMaxMajor() 和 scaleStepSize() 计算。
 * @param interval 区间对象
 * \endif
 */
void QwtAbstractScale::setScale(const QwtInterval& interval)
{
    setScale(interval.minValue(), interval.maxValue());
}

/**
 * \if ENGLISH
 * @brief Specify a scale by scale division
 * @details When using this method, scaleMaxMinor(), scaleMaxMajor() and
 *          scaleStepSize() have no effect.
 * @param scaleDiv Scale division object
 * \sa setAutoScale()
 * \endif
 * \if CHINESE
 * @brief 通过刻度划分指定刻度
 * @details 使用此方法时，scaleMaxMinor()、scaleMaxMajor() 和 scaleStepSize() 不起作用。
 * @param scaleDiv 刻度划分对象
 * \sa setAutoScale()
 * \endif
 */
void QwtAbstractScale::setScale(const QwtScaleDiv& scaleDiv)
{
    if (scaleDiv != m_data->scaleDraw->scaleDiv()) {
#if 1
        if (m_data->scaleEngine) {
            m_data->scaleDraw->setTransformation(m_data->scaleEngine->transformation());
        }
#endif

        m_data->scaleDraw->setScaleDiv(scaleDiv);

        scaleChange();
    }
}

/**
 * \if ENGLISH
 * @brief Set the maximum number of major tick intervals
 * @details The scale's major ticks are calculated automatically such that
 *          the number of major intervals does not exceed ticks.
 *          The default value is 5.
 * @param ticks Maximal number of major ticks
 * \sa scaleMaxMajor(), setScaleMaxMinor(), setScaleStepSize(), QwtScaleEngine::divideInterval()
 * \endif
 * \if CHINESE
 * @brief 设置主刻度间隔的最大数量
 * @details 刻度的主刻度会自动计算，使得主刻度间隔数量不超过 ticks。
 *          默认值为 5。
 * @param ticks 主刻度的最大数量
 * \sa scaleMaxMajor(), setScaleMaxMinor(), setScaleStepSize(), QwtScaleEngine::divideInterval()
 * \endif
 */
void QwtAbstractScale::setScaleMaxMajor(int ticks)
{
    if (ticks != m_data->maxMajor) {
        m_data->maxMajor = ticks;
        updateScaleDraw();
    }
}

/**
 * \if ENGLISH
 * @brief Return the maximum number of major tick intervals
 * @return Maximal number of major tick intervals
 * \sa setScaleMaxMajor(), scaleMaxMinor()
 * \endif
 * \if CHINESE
 * @brief 返回主刻度间隔的最大数量
 * @return 主刻度间隔的最大数量
 * \sa setScaleMaxMajor(), scaleMaxMinor()
 * \endif
 */
int QwtAbstractScale::scaleMaxMajor() const
{
    return m_data->maxMajor;
}

/**
 * \if ENGLISH
 * @brief Set the maximum number of minor tick intervals
 * @details The scale's minor ticks are calculated automatically such that
 *          the number of minor intervals does not exceed ticks.
 *          The default value is 3.
 * @param ticks Maximal number of minor ticks
 * \sa scaleMaxMajor(), setScaleMaxMinor(), setScaleStepSize(), QwtScaleEngine::divideInterval()
 * \endif
 * \if CHINESE
 * @brief 设置次刻度间隔的最大数量
 * @details 刻度的次刻度会自动计算，使得次刻度间隔数量不超过 ticks。
 *          默认值为 3。
 * @param ticks 次刻度的最大数量
 * \sa scaleMaxMajor(), setScaleMaxMinor(), setScaleStepSize(), QwtScaleEngine::divideInterval()
 * \endif
 */
void QwtAbstractScale::setScaleMaxMinor(int ticks)
{
    if (ticks != m_data->maxMinor) {
        m_data->maxMinor = ticks;
        updateScaleDraw();
    }
}

/**
 * \if ENGLISH
 * @brief Return the maximum number of minor tick intervals
 * @return Maximal number of minor tick intervals
 * \sa setScaleMaxMinor(), scaleMaxMajor()
 * \endif
 * \if CHINESE
 * @brief 返回次刻度间隔的最大数量
 * @return 次刻度间隔的最大数量
 * \sa setScaleMaxMinor(), scaleMaxMajor()
 * \endif
 */
int QwtAbstractScale::scaleMaxMinor() const
{
    return m_data->maxMinor;
}

/**
 * \if ENGLISH
 * @brief Set the step size used for calculating scale division
 * @details The step size is a hint for calculating the intervals for
 *          the major ticks of the scale. A value of 0.0 is interpreted
 *          as no hint.
 * @param stepSize Hint for the step size of the scale
 * \sa scaleStepSize(), QwtScaleEngine::divideScale()
 * \note Position and distance between major ticks also depends on scaleMaxMajor()
 * \endif
 * \if CHINESE
 * @brief 设置用于计算刻度划分的步长
 * @details 步长是计算刻度主刻度间隔的提示。值 0.0 表示无提示。
 * @param stepSize 刻度步长的提示值
 * \sa scaleStepSize(), QwtScaleEngine::divideScale()
 * \note 主刻度线的位置和距离还取决于 scaleMaxMajor()
 * \endif
 */
void QwtAbstractScale::setScaleStepSize(double stepSize)
{
    if (stepSize != m_data->stepSize) {
        m_data->stepSize = stepSize;
        updateScaleDraw();
    }
}

/**
 * \if ENGLISH
 * @brief Return the step size hint
 * @return Hint for the step size of the scale
 * \sa setScaleStepSize(), QwtScaleEngine::divideScale()
 * \endif
 * \if CHINESE
 * @brief 返回步长提示值
 * @return 刻度步长的提示值
 * \sa setScaleStepSize(), QwtScaleEngine::divideScale()
 * \endif
 */
double QwtAbstractScale::scaleStepSize() const
{
    return m_data->stepSize;
}

/**
 * \if ENGLISH
 * @brief Set the scale draw object
 * @details scaleDraw must be created with new and will be deleted in
 *          the destructor or the next call of setAbstractScaleDraw().
 * \sa abstractScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 设置刻度绘制对象
 * @details scaleDraw 必须使用 new 创建，并将在析构函数或下次调用
 *          setAbstractScaleDraw() 时删除。
 * \sa abstractScaleDraw()
 * \endif
 */
void QwtAbstractScale::setAbstractScaleDraw(QwtAbstractScaleDraw* scaleDraw)
{
    if (scaleDraw == nullptr || scaleDraw == m_data->scaleDraw)
        return;

    if (m_data->scaleDraw != nullptr)
        scaleDraw->setScaleDiv(m_data->scaleDraw->scaleDiv());

    delete m_data->scaleDraw;
    m_data->scaleDraw = scaleDraw;
}

/**
 * \if ENGLISH
 * @brief Return the scale draw object (non-const)
 * @return Scale draw object
 * \sa setAbstractScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回刻度绘制对象（非常量版本）
 * @return 刻度绘制对象
 * \sa setAbstractScaleDraw()
 * \endif
 */
QwtAbstractScaleDraw* QwtAbstractScale::abstractScaleDraw()
{
    return m_data->scaleDraw;
}

/**
 * \if ENGLISH
 * @brief Return the scale draw object (const)
 * @return Scale draw object
 * \sa setAbstractScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回刻度绘制对象（常量版本）
 * @return 刻度绘制对象
 * \sa setAbstractScaleDraw()
 * \endif
 */
const QwtAbstractScaleDraw* QwtAbstractScale::abstractScaleDraw() const
{
    return m_data->scaleDraw;
}

/**
 * \if ENGLISH
 * @brief Set the scale engine
 * @details The scale engine is responsible for calculating the scale division
 *          and provides a transformation between scale and widget coordinates.
 *          scaleEngine must be created with new and will be deleted in
 *          the destructor or the next call of setScaleEngine().
 * @param scaleEngine Scale engine object
 * \endif
 * \if CHINESE
 * @brief 设置刻度引擎
 * @details 刻度引擎负责计算刻度划分，并提供刻度和控件坐标之间的变换。
 *          scaleEngine 必须使用 new 创建，并将在析构函数或下次调用
 *          setScaleEngine() 时删除。
 * @param scaleEngine 刻度引擎对象
 * \endif
 */
void QwtAbstractScale::setScaleEngine(QwtScaleEngine* scaleEngine)
{
    if (scaleEngine != nullptr && scaleEngine != m_data->scaleEngine) {
        delete m_data->scaleEngine;
        m_data->scaleEngine = scaleEngine;
    }
}

/**
 * \if ENGLISH
 * @brief Return the scale engine (const version)
 * @return Scale engine object
 * \sa setScaleEngine()
 * \endif
 * \if CHINESE
 * @brief 返回刻度引擎（常量版本）
 * @return 刻度引擎对象
 * \sa setScaleEngine()
 * \endif
 */
const QwtScaleEngine* QwtAbstractScale::scaleEngine() const
{
    return m_data->scaleEngine;
}

/**
 * \if ENGLISH
 * @brief Return the scale engine (non-const version)
 * @return Scale engine object
 * \sa setScaleEngine()
 * \endif
 * \if CHINESE
 * @brief 返回刻度引擎（非常量版本）
 * @return 刻度引擎对象
 * \sa setScaleEngine()
 * \endif
 */
QwtScaleEngine* QwtAbstractScale::scaleEngine()
{
    return m_data->scaleEngine;
}

/**
 * \if ENGLISH
 * @brief Return the scale division
 * @return Scale boundaries and positions of the ticks
 * @details The scale division might have been assigned explicitly
 *          or calculated implicitly by rescale().
 * \endif
 * \if CHINESE
 * @brief 返回刻度划分
 * @return 刻度边界和刻度线位置
 * @details 刻度划分可能是显式分配的，也可能是通过 rescale() 隐式计算的。
 * \endif
 */
const QwtScaleDiv& QwtAbstractScale::scaleDiv() const
{
    return m_data->scaleDraw->scaleDiv();
}

/**
 * \if ENGLISH
 * @brief Return the scale map
 * @return Map to translate between scale and widget coordinates
 * \endif
 * \if CHINESE
 * @brief 返回刻度映射
 * @return 用于在刻度和控件坐标之间转换的映射
 * \endif
 */
const QwtScaleMap& QwtAbstractScale::scaleMap() const
{
    return m_data->scaleDraw->scaleMap();
}

/**
 * \if ENGLISH
 * @brief Transform a scale value to widget coordinates
 * @param value Scale value
 * @return Corresponding widget coordinate for value
 * \sa scaleMap(), invTransform()
 * \endif
 * \if CHINESE
 * @brief 将刻度值转换为控件坐标
 * @param value 刻度值
 * @return 对应的控件坐标
 * \sa scaleMap(), invTransform()
 * \endif
 */
int QwtAbstractScale::transform(double value) const
{
    return qRound(m_data->scaleDraw->scaleMap().transform(value));
}

/**
 * \if ENGLISH
 * @brief Transform a widget coordinate to scale value
 * @param value Widget coordinate
 * @return Corresponding scale coordinate for value
 * \sa scaleMap(), transform()
 * \endif
 * \if CHINESE
 * @brief 将控件坐标转换为刻度值
 * @param value 控件坐标
 * @return 对应的刻度坐标
 * \sa scaleMap(), transform()
 * \endif
 */
double QwtAbstractScale::invTransform(int value) const
{
    return m_data->scaleDraw->scaleMap().invTransform(value);
}

/**
 * \if ENGLISH
 * @brief Check if scale is inverted
 * @return True if scale is increasing in opposite direction to widget coordinates
 * \endif
 * \if CHINESE
 * @brief 检查刻度是否反向
 * @return 如果刻度增长方向与控件坐标方向相反则返回 true
 * \endif
 */
bool QwtAbstractScale::isInverted() const
{
    return m_data->scaleDraw->scaleMap().isInverting();
}

/**
 * \if ENGLISH
 * @brief Return the minimum boundary
 * @return The boundary with the smaller value
 * \sa maximum(), lowerBound(), upperBound()
 * \endif
 * \if CHINESE
 * @brief 返回最小边界
 * @return 较小的边界值
 * \sa maximum(), lowerBound(), upperBound()
 * \endif
 */
double QwtAbstractScale::minimum() const
{
    return qMin(m_data->scaleDraw->scaleDiv().lowerBound(), m_data->scaleDraw->scaleDiv().upperBound());
}

/**
 * \if ENGLISH
 * @brief Return the maximum boundary
 * @return The boundary with the larger value
 * \sa minimum(), lowerBound(), upperBound()
 * \endif
 * \if CHINESE
 * @brief 返回最大边界
 * @return 较大的边界值
 * \sa minimum(), lowerBound(), upperBound()
 * \endif
 */
double QwtAbstractScale::maximum() const
{
    return qMax(m_data->scaleDraw->scaleDiv().lowerBound(), m_data->scaleDraw->scaleDiv().upperBound());
}

/**
 * \if ENGLISH
 * @brief Notify about scale changes
 * @details This virtual function is called when the scale changes.
 *          Override this function in derived classes to handle scale changes.
 * \endif
 * \if CHINESE
 * @brief 通知刻度变化
 * @details 当刻度变化时调用此虚函数。
 *          在派生类中重写此函数以处理刻度变化。
 * \endif
 */
void QwtAbstractScale::scaleChange()
{
}

/**
 * \if ENGLISH
 * @brief Recalculate scale division and update scale
 * @param lowerBound Lower limit of the scale interval
 * @param upperBound Upper limit of the scale interval
 * @param stepSize Major step size
 * \sa scaleChange()
 * \endif
 * \if CHINESE
 * @brief 重新计算刻度划分并更新刻度
 * @param lowerBound 刻度区间的下界
 * @param upperBound 刻度区间的上界
 * @param stepSize 主刻度步长
 * \sa scaleChange()
 * \endif
 */
void QwtAbstractScale::rescale(double lowerBound, double upperBound, double stepSize)
{
    const QwtScaleDiv scaleDiv = m_data->scaleEngine->divideScale(lowerBound,
                                                                  upperBound,
                                                                  m_data->maxMajor,
                                                                  m_data->maxMinor,
                                                                  stepSize);

    if (scaleDiv != m_data->scaleDraw->scaleDiv()) {
#if 1
        m_data->scaleDraw->setTransformation(m_data->scaleEngine->transformation());
#endif

        m_data->scaleDraw->setScaleDiv(scaleDiv);
        scaleChange();
    }
}

/**
 * \if ENGLISH
 * @brief Handle change events
 * @param event Change event
 * @details Invalidates internal caches if necessary (e.g., on locale change).
 * \endif
 * \if CHINESE
 * @brief 处理变化事件
 * @param event 变化事件
 * @details 如有必要会失效内部缓存（例如，在区域设置变化时）。
 * \endif
 */
void QwtAbstractScale::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LocaleChange) {
        m_data->scaleDraw->invalidateCache();
    }

    QWidget::changeEvent(event);
}

/**
 * \if ENGLISH
 * @brief Recalculate ticks and scale boundaries
 * @details Updates the scale draw by recalculating ticks and boundaries
 *          based on the current scale division.
 * \endif
 * \if CHINESE
 * @brief 重新计算刻度线和刻度边界
 * @details 通过基于当前刻度划分重新计算刻度线和边界来更新刻度绘制。
 * \endif
 */
void QwtAbstractScale::updateScaleDraw()
{
    rescale(m_data->scaleDraw->scaleDiv().lowerBound(), m_data->scaleDraw->scaleDiv().upperBound(), m_data->stepSize);
}
