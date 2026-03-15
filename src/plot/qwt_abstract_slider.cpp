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

#include "qwt_abstract_slider.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_math.h"

#include <qevent.h>

static double qwtAlignToScaleDiv(const QwtAbstractSlider* slider, double value)
{
    const QwtScaleDiv& sd = slider->scaleDiv();

    const int tValue = slider->transform(value);

    if (tValue == slider->transform(sd.lowerBound()))
        return sd.lowerBound();

    if (tValue == slider->transform(sd.upperBound()))
        return sd.upperBound();

    for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
        const QList< double > ticks = sd.ticks(i);
        for (int j = 0; j < ticks.size(); j++) {
            if (slider->transform(ticks[ j ]) == tValue)
                return ticks[ j ];
        }
    }

    return value;
}

class QwtAbstractSlider::PrivateData
{
public:
    PrivateData()
        : isScrolling(false)
        , isTracking(true)
        , pendingValueChanged(false)
        , readOnly(false)
        , totalSteps(100)
        , singleSteps(1)
        , pageSteps(10)
        , stepAlignment(true)
        , isValid(false)
        , value(0.0)
        , wrapping(false)
        , invertedControls(false)
    {
    }

    bool isScrolling;
    bool isTracking;
    bool pendingValueChanged;

    bool readOnly;

    uint totalSteps;
    uint singleSteps;
    uint pageSteps;
    bool stepAlignment;

    bool isValid;
    double value;

    bool wrapping;
    bool invertedControls;
};

/**
 * \if ENGLISH
 * @brief Constructor for QwtAbstractSlider
 * @details The scale is initialized to [0.0, 100.0], the
 *          number of steps is set to 100 with 1 and 10 as single
 *          and page step sizes. Step alignment is enabled.
 *          The initial value is invalid.
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief QwtAbstractSlider 构造函数
 * @details 刻度初始化为 [0.0, 100.0]，
 *          步数设置为 100，单步和页步大小分别为 1 和 10。
 *          步对齐已启用。
 *          初始值无效。
 * @param parent 父控件
 * \endif
 */
QwtAbstractSlider::QwtAbstractSlider(QWidget* parent) : QwtAbstractScale(parent)
{
    m_data = new QwtAbstractSlider::PrivateData;

    setScale(0.0, 100.0);
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * \if ENGLISH
 * @brief Destructor for QwtAbstractSlider
 * \endif
 * \if CHINESE
 * @brief QwtAbstractSlider 析构函数
 * \endif
 */
QwtAbstractSlider::~QwtAbstractSlider()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the value to be valid or invalid
 * @param on When true, the value is invalidated
 * \sa setValue()
 * \endif
 * \if CHINESE
 * @brief 设置值为有效或无效
 * @param on 如果为 true，值变为无效
 * \sa setValue()
 * \endif
 */
void QwtAbstractSlider::setValid(bool on)
{
    if (on != m_data->isValid) {
        m_data->isValid = on;
        sliderChange();

        Q_EMIT valueChanged(m_data->value);
    }
}

/**
 * \if ENGLISH
 * @brief Check if the value is valid
 * @return True if the value is invalid
 * \endif
 * \if CHINESE
 * @brief 检查值是否有效
 * @return 如果值无效则返回 true
 * \endif
 */
bool QwtAbstractSlider::isValid() const
{
    return m_data->isValid;
}

/**
 * \if ENGLISH
 * @brief Enable or disable read-only mode
 * @details In read-only mode the slider can't be controlled by mouse
 *          or keyboard.
 * @param on Enables read-only mode if true
 * \sa isReadOnly()
 * \warning The focus policy is set to Qt::StrongFocus or Qt::NoFocus
 * \endif
 * \if CHINESE
 * @brief 启用或禁用只读模式
 * @details 在只读模式下，滑块不能通过鼠标或键盘控制。
 * @param on 如果为 true 则启用只读模式
 * \sa isReadOnly()
 * \warning 焦点策略会被设置为 Qt::StrongFocus 或 Qt::NoFocus
 * \endif
 */
void QwtAbstractSlider::setReadOnly(bool on)
{
    if (m_data->readOnly != on) {
        m_data->readOnly = on;
        setFocusPolicy(on ? Qt::StrongFocus : Qt::NoFocus);

        update();
    }
}

/**
 * \if ENGLISH
 * @brief Check if read-only mode is enabled
 * @return True if read-only mode is enabled
 * \sa setReadOnly()
 * \endif
 * \if CHINESE
 * @brief 检查是否启用了只读模式
 * @return 如果启用了只读模式则返回 true
 * \sa setReadOnly()
 * \endif
 */
bool QwtAbstractSlider::isReadOnly() const
{
    return m_data->readOnly;
}

/**
 * \if ENGLISH
 * @brief Enable or disable tracking
 * @details If tracking is enabled, the slider emits the valueChanged()
 *          signal while the movable part of the slider is being dragged.
 *          If tracking is disabled, the slider emits the valueChanged() signal
 *          only when the user releases the slider.
 *          Tracking is enabled by default.
 * @param on True to enable tracking, false to disable
 * \sa isTracking(), sliderMoved()
 * \endif
 * \if CHINESE
 * @brief 启用或禁用跟踪
 * @details 如果启用跟踪，滑块在被拖动时会发出 valueChanged() 信号。
 *          如果禁用跟踪，滑块仅在用户释放时发出 valueChanged() 信号。
 *          跟踪默认启用。
 * @param on true 启用跟踪，false 禁用
 * \sa isTracking(), sliderMoved()
 * \endif
 */
void QwtAbstractSlider::setTracking(bool on)
{
    m_data->isTracking = on;
}

/**
 * \if ENGLISH
 * @brief Check if tracking is enabled
 * @return True if tracking is enabled
 * \sa setTracking()
 * \endif
 * \if CHINESE
 * @brief 检查是否启用了跟踪
 * @return 如果启用了跟踪则返回 true
 * \sa setTracking()
 * \endif
 */
bool QwtAbstractSlider::isTracking() const
{
    return m_data->isTracking;
}

/**
 * \if ENGLISH
 * @brief Handle mouse press events
 * @param event Mouse event
 * \details Initiates scrolling if the position is valid.
 * \sa mouseMoveEvent(), mouseReleaseEvent()
 * \endif
 * \if CHINESE
 * @brief 处理鼠标按下事件
 * @param event 鼠标事件
 * \details 如果位置有效则开始滚动。
 * \sa mouseMoveEvent(), mouseReleaseEvent()
 * \endif
 */
void QwtAbstractSlider::mousePressEvent(QMouseEvent* event)
{
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (!m_data->isValid || lowerBound() == upperBound())
        return;

    m_data->isScrolling = isScrollPosition(event->pos());

    if (m_data->isScrolling) {
        m_data->pendingValueChanged = false;

        Q_EMIT sliderPressed();
    }
}

/**
 * \if ENGLISH
 * @brief Handle mouse move events
 * @param event Mouse event
 * \details Updates the slider value while scrolling.
 * \sa mousePressEvent(), mouseReleaseEvent()
 * \endif
 * \if CHINESE
 * @brief 处理鼠标移动事件
 * @param event 鼠标事件
 * \details 滚动时更新滑块值。
 * \sa mousePressEvent(), mouseReleaseEvent()
 * \endif
 */
void QwtAbstractSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (m_data->isValid && m_data->isScrolling) {
        double value = scrolledTo(event->pos());
        if (value != m_data->value) {
            value = boundedValue(value);

            if (m_data->stepAlignment) {
                value = alignedValue(value);
            } else {
                value = qwtAlignToScaleDiv(this, value);
            }

            if (value != m_data->value) {
                m_data->value = value;

                sliderChange();

                Q_EMIT sliderMoved(m_data->value);

                if (m_data->isTracking)
                    Q_EMIT valueChanged(m_data->value);
                else
                    m_data->pendingValueChanged = true;
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Handle mouse release events
 * @param event Mouse event
 * \details Ends scrolling and emits valueChanged() if needed.
 * \sa mousePressEvent(), mouseMoveEvent()
 * \endif
 * \if CHINESE
 * @brief 处理鼠标释放事件
 * @param event 鼠标事件
 * \details 结束滚动，如有需要则发出 valueChanged() 信号。
 * \sa mousePressEvent(), mouseMoveEvent()
 * \endif
 */
void QwtAbstractSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (m_data->isScrolling && m_data->isValid) {
        m_data->isScrolling = false;

        if (m_data->pendingValueChanged)
            Q_EMIT valueChanged(m_data->value);

        Q_EMIT sliderReleased();
    }
}

/**
 * \if ENGLISH
 * @brief Handle wheel events
 * @details In/decreases the value by a number of steps. The direction
 *          depends on the invertedControls() property.
 *          When the control or shift modifier is pressed the wheel delta
 *          (divided by 120) is mapped to an increment according to
 *          pageSteps(). Otherwise it is mapped to singleSteps().
 * @param event Wheel event
 * \sa keyPressEvent()
 * \endif
 * \if CHINESE
 * @brief 处理滚轮事件
 * @details 按步数增加/减少值。方向取决于 invertedControls() 属性。
 *          当按下 Ctrl 或 Shift 修饰键时，滚轮增量（除以 120）根据
 *          pageSteps() 映射。否则根据 singleSteps() 映射。
 * @param event 滚轮事件
 * \sa keyPressEvent()
 * \endif
 */
void QwtAbstractSlider::wheelEvent(QWheelEvent* event)
{
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (!m_data->isValid || m_data->isScrolling)
        return;

#if QT_VERSION < 0x050000
    const int wheelDelta = event->delta();
#else
    const QPoint delta   = event->angleDelta();
    const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
#endif

    int numSteps = 0;

    if ((event->modifiers() & Qt::ControlModifier) || (event->modifiers() & Qt::ShiftModifier)) {
        // one page regardless of delta
        numSteps = m_data->pageSteps;
        if (wheelDelta < 0)
            numSteps = -numSteps;
    } else {
        const int numTurns = (wheelDelta / 120);
        numSteps           = numTurns * m_data->singleSteps;
    }

    if (m_data->invertedControls)
        numSteps = -numSteps;

    const double value = incrementedValue(m_data->value, numSteps);
    if (value != m_data->value) {
        m_data->value = value;
        sliderChange();

        Q_EMIT sliderMoved(m_data->value);
        Q_EMIT valueChanged(m_data->value);
    }
}

/**
 * \if ENGLISH
 * @brief Handle key press events
 * @details QwtAbstractSlider handles the following keys:
 *          - Qt::Key_Left: Add/Subtract singleSteps() in direction to lowerBound()
 *          - Qt::Key_Right: Add/Subtract singleSteps() in direction to upperBound()
 *          - Qt::Key_Down: Subtract singleSteps(), when invertedControls() is false
 *          - Qt::Key_Up: Add singleSteps(), when invertedControls() is false
 *          - Qt::Key_PageDown: Subtract pageSteps(), when invertedControls() is false
 *          - Qt::Key_PageUp: Add pageSteps(), when invertedControls() is false
 *          - Qt::Key_Home: Set the value to the minimum()
 *          - Qt::Key_End: Set the value to the maximum()
 * @param event Key event
 * \sa isReadOnly(), wheelEvent()
 * \endif
 * \if CHINESE
 * @brief 处理键盘按下事件
 * @details QwtAbstractSlider 处理以下按键：
 *          - Qt::Key_Left: 朝下界方向增加/减少 singleSteps()
 *          - Qt::Key_Right: 朝上界方向增加/减少 singleSteps()
 *          - Qt::Key_Down: 当 invertedControls() 为 false 时减少 singleSteps()
 *          - Qt::Key_Up: 当 invertedControls() 为 false 时增加 singleSteps()
 *          - Qt::Key_PageDown: 当 invertedControls() 为 false 时减少 pageSteps()
 *          - Qt::Key_PageUp: 当 invertedControls() 为 false 时增加 pageSteps()
 *          - Qt::Key_Home: 将值设置为 minimum()
 *          - Qt::Key_End: 将值设置为 maximum()
 * @param event 键盘事件
 * \sa isReadOnly(), wheelEvent()
 * \endif
 */
void QwtAbstractSlider::keyPressEvent(QKeyEvent* event)
{
    if (isReadOnly()) {
        event->ignore();
        return;
    }

    if (!m_data->isValid || m_data->isScrolling)
        return;

    int numSteps = 0;
    double value = m_data->value;

    switch (event->key()) {
    case Qt::Key_Left: {
        numSteps = -static_cast< int >(m_data->singleSteps);
        if (isInverted())
            numSteps = -numSteps;

        break;
    }
    case Qt::Key_Right: {
        numSteps = m_data->singleSteps;
        if (isInverted())
            numSteps = -numSteps;

        break;
    }
    case Qt::Key_Down: {
        numSteps = -static_cast< int >(m_data->singleSteps);
        if (m_data->invertedControls)
            numSteps = -numSteps;
        break;
    }
    case Qt::Key_Up: {
        numSteps = m_data->singleSteps;
        if (m_data->invertedControls)
            numSteps = -numSteps;

        break;
    }
    case Qt::Key_PageUp: {
        numSteps = m_data->pageSteps;
        if (m_data->invertedControls)
            numSteps = -numSteps;
        break;
    }
    case Qt::Key_PageDown: {
        numSteps = -static_cast< int >(m_data->pageSteps);
        if (m_data->invertedControls)
            numSteps = -numSteps;
        break;
    }
    case Qt::Key_Home: {
        value = minimum();
        break;
    }
    case Qt::Key_End: {
        value = maximum();
        break;
    }
    default: {
        event->ignore();
    }
    }

    if (numSteps != 0) {
        value = incrementedValue(m_data->value, numSteps);
    }

    if (value != m_data->value) {
        m_data->value = value;
        sliderChange();

        Q_EMIT sliderMoved(m_data->value);
        Q_EMIT valueChanged(m_data->value);
    }
}

/*!
   \brief Set the number of steps

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.

   The default setting is 100.

   \param stepCount Number of steps

   \sa totalSteps(), setSingleSteps(), setPageSteps()
 */
void QwtAbstractSlider::setTotalSteps(uint stepCount)
{
    m_data->totalSteps = stepCount;
}

/*!
   \return Number of steps
   \sa setTotalSteps(), singleSteps(), pageSteps()
 */
uint QwtAbstractSlider::totalSteps() const
{
    return m_data->totalSteps;
}

/*!
   \brief Set the number of steps for a single increment

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.

   \param stepCount Number of steps

   \sa singleSteps(), setTotalSteps(), setPageSteps()
 */

void QwtAbstractSlider::setSingleSteps(uint stepCount)
{
    m_data->singleSteps = stepCount;
}

/*!
   \return Number of steps
   \sa setSingleSteps(), totalSteps(), pageSteps()
 */
uint QwtAbstractSlider::singleSteps() const
{
    return m_data->singleSteps;
}

/*!
   \brief Set the number of steps for a page increment

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.

   \param stepCount Number of steps

   \sa pageSteps(), setTotalSteps(), setSingleSteps()
 */

void QwtAbstractSlider::setPageSteps(uint stepCount)
{
    m_data->pageSteps = stepCount;
}

/*!
   \return Number of steps
   \sa setPageSteps(), totalSteps(), singleSteps()
 */
uint QwtAbstractSlider::pageSteps() const
{
    return m_data->pageSteps;
}

/*!
   \brief Enable step alignment

   When step alignment is enabled values resulting from slider
   movements are aligned to the step size.

   \param on Enable step alignment when true
   \sa stepAlignment()
 */
void QwtAbstractSlider::setStepAlignment(bool on)
{
    if (on != m_data->stepAlignment) {
        m_data->stepAlignment = on;
    }
}

/*!
   \return True, when step alignment is enabled
   \sa setStepAlignment()
 */
bool QwtAbstractSlider::stepAlignment() const
{
    return m_data->stepAlignment;
}

/*!
   Set the slider to the specified value

   \param value New value
   \sa setValid(), sliderChange(), valueChanged()
 */
void QwtAbstractSlider::setValue(double value)
{
    value = qBound(minimum(), value, maximum());

    const bool changed = (m_data->value != value) || !m_data->isValid;

    m_data->value   = value;
    m_data->isValid = true;

    if (changed) {
        sliderChange();
        Q_EMIT valueChanged(m_data->value);
    }
}

//! Returns the current value.
double QwtAbstractSlider::value() const
{
    return m_data->value;
}

/*!
   If wrapping is true stepping up from upperBound() value will
   take you to the minimum() value and vice versa.

   \param on En/Disable wrapping
   \sa wrapping()
 */
void QwtAbstractSlider::setWrapping(bool on)
{
    m_data->wrapping = on;
}

/*!
   \return True, when wrapping is set
   \sa setWrapping()
 */
bool QwtAbstractSlider::wrapping() const
{
    return m_data->wrapping;
}

/*!
   Invert wheel and key events

   Usually scrolling the mouse wheel "up" and using keys like page
   up will increase the slider's value towards its maximum.
   When invertedControls() is enabled the value is scrolled
   towards its minimum.

   Inverting the controls might be f.e. useful for a vertical slider
   with an inverted scale ( decreasing from top to bottom ).

   \param on Invert controls, when true

   \sa invertedControls(), keyEvent(), wheelEvent()
 */
void QwtAbstractSlider::setInvertedControls(bool on)
{
    m_data->invertedControls = on;
}

/*!
   \return True, when the controls are inverted
   \sa setInvertedControls()
 */
bool QwtAbstractSlider::invertedControls() const
{
    return m_data->invertedControls;
}

/*!
   Increment the slider

   The step size depends on the number of totalSteps()

   \param stepCount Number of steps
   \sa setTotalSteps(), incrementedValue()
 */
void QwtAbstractSlider::incrementValue(int stepCount)
{
    const double value = incrementedValue(m_data->value, stepCount);

    if (value != m_data->value) {
        m_data->value = value;
        sliderChange();
    }
}

/*!
   Increment a value

   \param value Value
   \param stepCount Number of steps

   \return Incremented value
 */
double QwtAbstractSlider::incrementedValue(double value, int stepCount) const
{
    if (m_data->totalSteps == 0)
        return value;

    const QwtTransform* transformation = scaleMap().transformation();

    if (transformation == nullptr) {
        const double range = maximum() - minimum();
        value += stepCount * range / m_data->totalSteps;
    } else {
        QwtScaleMap map = scaleMap();
        map.setPaintInterval(0, m_data->totalSteps);

        // we need equidistant steps according to
        // paint device coordinates
        const double range = transformation->transform(maximum()) - transformation->transform(minimum());

        const double stepSize = range / m_data->totalSteps;

        double v = transformation->transform(value);

        v = qRound(v / stepSize) * stepSize;
        v += stepCount * range / m_data->totalSteps;

        value = transformation->invTransform(v);
    }

    value = boundedValue(value);

    if (m_data->stepAlignment)
        value = alignedValue(value);

    return value;
}

double QwtAbstractSlider::boundedValue(double value) const
{
    const double vmin = minimum();
    const double vmax = maximum();

    if (m_data->wrapping && vmin != vmax) {
        if (qFuzzyCompare(scaleMap().pDist(), 360.0)) {
            // full circle scales: min and max are the same

            if (qFuzzyCompare(value, vmax)) {
                value = vmin;
            } else {
                const double range = vmax - vmin;

                if (value < vmin) {
                    value += std::ceil((vmin - value) / range) * range;
                } else if (value > vmax) {
                    value -= std::ceil((value - vmax) / range) * range;
                }
            }
        } else {
            if (value < vmin)
                value = vmax;
            else if (value > vmax)
                value = vmin;
        }
    } else {
        value = qBound(vmin, value, vmax);
    }

    return value;
}

double QwtAbstractSlider::alignedValue(double value) const
{
    if (m_data->totalSteps == 0)
        return value;

    double stepSize;

    if (scaleMap().transformation() == nullptr) {
        stepSize = (maximum() - minimum()) / m_data->totalSteps;
        if (stepSize > 0.0) {
            value = lowerBound() + qRound((value - lowerBound()) / stepSize) * stepSize;
        }
    } else {
        stepSize = (scaleMap().p2() - scaleMap().p1()) / m_data->totalSteps;

        if (stepSize > 0.0) {
            double v = scaleMap().transform(value);

            v = scaleMap().p1() + qRound((v - scaleMap().p1()) / stepSize) * stepSize;

            value = scaleMap().invTransform(v);
        }
    }

    if (qAbs(stepSize) > 1e-12) {
        if (qFuzzyCompare(value + 1.0, 1.0)) {
            // correct rounding error if value = 0
            value = 0.0;
        } else {
            // correct rounding error at the border
            if (qFuzzyCompare(value, upperBound()))
                value = upperBound();
            else if (qFuzzyCompare(value, lowerBound()))
                value = lowerBound();
        }
    }

    return value;
}

/*!
   Update the slider according to modifications of the scale
 */
void QwtAbstractSlider::scaleChange()
{
    const double value = qBound(minimum(), m_data->value, maximum());

    const bool changed = (value != m_data->value);
    if (changed) {
        m_data->value = value;
    }

    if (m_data->isValid || changed)
        Q_EMIT valueChanged(m_data->value);

    updateGeometry();
    update();
}

//! Calling update()
void QwtAbstractSlider::sliderChange()
{
    update();
}
