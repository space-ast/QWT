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

#include "qwt_arrow_button.h"
#include "qwt_counter.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qevent.h>
#include <qstyle.h>

class QwtCounter::PrivateData
{
public:
    PrivateData() : minimum(0.0), maximum(0.0), singleStep(1.0), isValid(false), value(0.0), wrapping(false)
    {
        increment[ Button1 ] = 1;
        increment[ Button2 ] = 10;
        increment[ Button3 ] = 100;
    }

    QwtArrowButton* buttonDown[ ButtonCnt ];
    QwtArrowButton* buttonUp[ ButtonCnt ];
    QLineEdit* valueEdit;

    int increment[ ButtonCnt ];
    int numButtons;

    double minimum;
    double maximum;
    double singleStep;

    bool isValid;
    double value;

    bool wrapping;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details The counter is initialized with a range set to [0.0, 1.0] with
 *          0.01 as single step size. The value is invalid.
 *          The default number of buttons is set to 2. The default increments are:
 *          - Button 1: 1 step
 *          - Button 2: 10 steps
 *          - Button 3: 100 steps
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @details 计数器初始化时范围设置为 [0.0, 1.0]，单步大小为 0.01。
 *          值无效。默认按钮数量为 2。默认增量为：
 *          - 按钮 1：1 步
 *          - 按钮 2：10 步
 *          - 按钮 3：100 步
 * @param parent 父控件
 * \endif
 */
QwtCounter::QwtCounter(QWidget* parent) : QWidget(parent)
{
    initCounter();
}

void QwtCounter::initCounter()
{
    m_data = new PrivateData;

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());

    for (int i = ButtonCnt - 1; i >= 0; i--) {
        QwtArrowButton* btn = new QwtArrowButton(i + 1, Qt::DownArrow, this);
        btn->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(btn);

        connect(btn, SIGNAL(released()), SLOT(btnReleased()));
        connect(btn, SIGNAL(clicked()), SLOT(btnClicked()));

        m_data->buttonDown[ i ] = btn;
    }

    m_data->valueEdit = new QLineEdit(this);
    m_data->valueEdit->setReadOnly(false);
    m_data->valueEdit->setValidator(new QDoubleValidator(m_data->valueEdit));
    layout->addWidget(m_data->valueEdit);

    connect(m_data->valueEdit, SIGNAL(editingFinished()), SLOT(textChanged()));

    layout->setStretchFactor(m_data->valueEdit, 10);

    for (int i = 0; i < ButtonCnt; i++) {
        QwtArrowButton* btn = new QwtArrowButton(i + 1, Qt::UpArrow, this);
        btn->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(btn);

        connect(btn, SIGNAL(released()), SLOT(btnReleased()));
        connect(btn, SIGNAL(clicked()), SLOT(btnClicked()));

        m_data->buttonUp[ i ] = btn;
    }

    setNumButtons(2);
    setRange(0.0, 1.0);
    setSingleStep(0.001);
    setValue(0.0);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    setFocusProxy(m_data->valueEdit);
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtCounter::~QwtCounter()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the counter to be in valid/invalid state
 * @details When the counter is set to invalid, no numbers are displayed and
 *          the buttons are disabled.
 * @param on If true the counter will be set as valid
 * \sa setValue(), isValid()
 * \endif
 * \if CHINESE
 * @brief 设置计数器的有效/无效状态
 * @details 当计数器设置为无效时，不显示数字，按钮被禁用。
 * @param on 如果为 true，计数器将被设置为有效
 * \sa setValue(), isValid()
 * \endif
 */
void QwtCounter::setValid(bool on)
{
    if (on != m_data->isValid) {
        m_data->isValid = on;

        updateButtons();

        if (m_data->isValid) {
            showNumber(value());
            Q_EMIT valueChanged(value());
        } else {
            m_data->valueEdit->setText(QString());
        }
    }
}

/**
 * \if ENGLISH
 * @brief Return true if the value is valid
 * \sa setValid(), setValue()
 * \endif
 * \if CHINESE
 * @brief 如果值有效则返回 true
 * \sa setValid(), setValue()
 * \endif
 */
bool QwtCounter::isValid() const
{
    return m_data->isValid;
}

/**
 * \if ENGLISH
 * @brief Allow/disallow the user to manually edit the value
 * @param on True disable editing
 * \sa isReadOnly()
 * \endif
 * \if CHINESE
 * @brief 允许/禁止用户手动编辑值
 * @param on true 禁用编辑
 * \sa isReadOnly()
 * \endif
 */
void QwtCounter::setReadOnly(bool on)
{
    m_data->valueEdit->setReadOnly(on);
}

/**
 * \if ENGLISH
 * @brief Return true when the line edit is read only
 * \sa setReadOnly()
 * \endif
 * \if CHINESE
 * @brief 当行编辑器为只读时返回 true
 * \sa setReadOnly()
 * \endif
 */
bool QwtCounter::isReadOnly() const
{
    return m_data->valueEdit->isReadOnly();
}

/**
 * \if ENGLISH
 * @brief Set a new value without adjusting to the step raster
 * @details The state of the counter is set to be valid.
 * @param value New value
 * \sa isValid(), value(), valueChanged()
 * @warning The value is clipped when it lies outside the range.
 * \endif
 * \if CHINESE
 * @brief 设置新值，不调整到步长网格
 * @details 计数器的状态设置为有效。
 * @param value 新值
 * \sa isValid(), value(), valueChanged()
 * @warning 当值超出范围时会被裁剪。
 * \endif
 */

void QwtCounter::setValue(double value)
{
    const double vmin = qwtMinF(m_data->minimum, m_data->maximum);
    const double vmax = qwtMaxF(m_data->minimum, m_data->maximum);

    value = qBound(vmin, value, vmax);

    if (!m_data->isValid || value != m_data->value) {
        m_data->isValid = true;
        m_data->value   = value;

        showNumber(value);
        updateButtons();

        Q_EMIT valueChanged(value);
    }
}

/**
 * \if ENGLISH
 * @brief Return current value of the counter
 * \sa setValue(), valueChanged()
 * \endif
 * \if CHINESE
 * @brief 返回计数器的当前值
 * \sa setValue(), valueChanged()
 * \endif
 */
double QwtCounter::value() const
{
    return m_data->value;
}

/**
 * \if ENGLISH
 * @brief Set the minimum and maximum values
 * @details The maximum is adjusted if necessary to ensure that the range remains valid.
 *          The value might be modified to be inside of the range.
 * @param[in] min Minimum value
 * @param[in] max Maximum value
 * \sa minimum(), maximum()
 * \endif
 * \if CHINESE
 * @brief 设置最小值和最大值
 * @details 如果需要，会调整最大值以确保范围有效。
 *          值可能会被修改到范围内。
 * @param[in] min 最小值
 * @param[in] max 最大值
 * \sa minimum(), maximum()
 * \endif
 */
void QwtCounter::setRange(double min, double max)
{
    max = qwtMaxF(min, max);

    if (m_data->maximum == max && m_data->minimum == min)
        return;

    m_data->minimum = min;
    m_data->maximum = max;

    setSingleStep(singleStep());

    const double value = qBound(min, m_data->value, max);

    if (value != m_data->value) {
        m_data->value = value;

        if (m_data->isValid) {
            showNumber(value);
            Q_EMIT valueChanged(value);
        }
    }

    updateButtons();
}

/**
 * \if ENGLISH
 * @brief Set the minimum value of the range
 * @param value Minimum value
 * \sa setRange(), setMaximum(), minimum()
 * @note The maximum is adjusted if necessary to ensure that the range remains valid.
 * \endif
 * \if CHINESE
 * @brief 设置范围的最小值
 * @param value 最小值
 * \sa setRange(), setMaximum(), minimum()
 * @note 如果需要，会调整最大值以确保范围有效。
 * \endif
 */
void QwtCounter::setMinimum(double value)
{
    setRange(value, maximum());
}

/**
 * \if ENGLISH
 * @brief Return the minimum of the range
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 * \if CHINESE
 * @brief 返回范围的最小值
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 */
double QwtCounter::minimum() const
{
    return m_data->minimum;
}

/**
 * \if ENGLISH
 * @brief Set the maximum value of the range
 * @param value Maximum value
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 * \if CHINESE
 * @brief 设置范围的最大值
 * @param value 最大值
 * \sa setRange(), setMinimum(), maximum()
 * \endif
 */
void QwtCounter::setMaximum(double value)
{
    setRange(minimum(), value);
}

/**
 * \if ENGLISH
 * @brief Return the maximum of the range
 * \sa setRange(), setMaximum(), minimum()
 * \endif
 * \if CHINESE
 * @brief 返回范围的最大值
 * \sa setRange(), setMaximum(), minimum()
 * \endif
 */
double QwtCounter::maximum() const
{
    return m_data->maximum;
}

/**
 * \if ENGLISH
 * @brief Set the step size of the counter
 * @details A value <= 0.0 disables stepping
 * @param stepSize Single step size
 * \sa singleStep()
 * \endif
 * \if CHINESE
 * @brief 设置计数器的步长
 * @details 值 <= 0.0 时禁用步进
 * @param stepSize 单步大小
 * \sa singleStep()
 * \endif
 */
void QwtCounter::setSingleStep(double stepSize)
{
    m_data->singleStep = qwtMaxF(stepSize, 0.0);
}

/**
 * \if ENGLISH
 * @brief Return single step size
 * \sa setSingleStep()
 * \endif
 * \if CHINESE
 * @brief 返回单步大小
 * \sa setSingleStep()
 * \endif
 */
double QwtCounter::singleStep() const
{
    return m_data->singleStep;
}

/**
 * \if ENGLISH
 * @brief Enable/disable wrapping
 * @details If wrapping is true stepping up from maximum() value will take
 *          you to the minimum() value and vice versa.
 * @param on Enable/disable wrapping
 * \sa wrapping()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用循环
 * @details 如果 wrapping 为 true，从 maximum() 值向上步进将到达 minimum() 值，反之亦然。
 * @param on 启用/禁用循环
 * \sa wrapping()
 * \endif
 */
void QwtCounter::setWrapping(bool on)
{
    m_data->wrapping = on;
}

/**
 * \if ENGLISH
 * @brief Return true when wrapping is set
 * \sa setWrapping()
 * \endif
 * \if CHINESE
 * @brief 当设置了循环时返回 true
 * \sa setWrapping()
 * \endif
 */
bool QwtCounter::wrapping() const
{
    return m_data->wrapping;
}

/**
 * \if ENGLISH
 * @brief Specify the number of buttons on each side of the label
 * @param numButtons Number of buttons
 * \sa numButtons()
 * \endif
 * \if CHINESE
 * @brief 指定标签两侧的按钮数量
 * @param numButtons 钮数量
 * \sa numButtons()
 * \endif
 */
void QwtCounter::setNumButtons(int numButtons)
{
    if (numButtons < 0 || numButtons > QwtCounter::ButtonCnt)
        return;

    for (int i = 0; i < QwtCounter::ButtonCnt; i++) {
        if (i < numButtons) {
            m_data->buttonDown[ i ]->show();
            m_data->buttonUp[ i ]->show();
        } else {
            m_data->buttonDown[ i ]->hide();
            m_data->buttonUp[ i ]->hide();
        }
    }

    m_data->numButtons = numButtons;
}

/**
 * \if ENGLISH
 * @brief Return the number of buttons on each side of the widget
 * \sa setNumButtons()
 * \endif
 * \if CHINESE
 * @brief 返回控件两侧的按钮数量
 * \sa setNumButtons()
 * \endif
 */
int QwtCounter::numButtons() const
{
    return m_data->numButtons;
}

/**
 * \if ENGLISH
 * @brief Specify the number of steps by which the value is incremented or decremented when a specified button is pushed
 * @param[in] button Button index
 * @param[in] numSteps Number of steps
 * \sa incSteps()
 * \endif
 * \if CHINESE
 * @brief 指定按下特定按钮时值增加或减少的步数
 * @param[in] button 按钮索引
 * @param[in] numSteps 步数
 * \sa incSteps()
 * \endif
 */
void QwtCounter::setIncSteps(QwtCounter::Button button, int numSteps)
{
    if (button >= 0 && button < QwtCounter::ButtonCnt)
        m_data->increment[ button ] = numSteps;
}

/**
 * \if ENGLISH
 * @brief Return the number of steps by which a specified button increments the value, or 0 if the button is invalid
 * @param button Button index
 * \return Number of increment steps
 * \sa setIncSteps()
 * \endif
 * \if CHINESE
 * @brief 返回指定按钮增加值的步数，如果按钮无效则返回 0
 * @param button 按钮索引
 * \return 增加步数
 * \sa setIncSteps()
 * \endif
 */
int QwtCounter::incSteps(QwtCounter::Button button) const
{
    if (button >= 0 && button < QwtCounter::ButtonCnt)
        return m_data->increment[ button ];

    return 0;
}

/**
 * \if ENGLISH
 * @brief Set the number of increment steps for button 1
 * @param nSteps Number of steps
 * \sa stepButton1()
 * \endif
 * \if CHINESE
 * @brief 设置按钮 1 的增加步数
 * @param nSteps 步数
 * \sa stepButton1()
 * \endif
 */
void QwtCounter::setStepButton1(int nSteps)
{
    setIncSteps(QwtCounter::Button1, nSteps);
}

/**
 * \if ENGLISH
 * @brief Return the number of increment steps for button 1
 * \sa setStepButton1()
 * \endif
 * \if CHINESE
 * @brief 返回按钮 1 的增加步数
 * \sa setStepButton1()
 * \endif
 */
int QwtCounter::stepButton1() const
{
    return incSteps(QwtCounter::Button1);
}

/**
 * \if ENGLISH
 * @brief Set the number of increment steps for button 2
 * @param nSteps Number of steps
 * \sa stepButton2()
 * \endif
 * \if CHINESE
 * @brief 设置按钮 2 的增加步数
 * @param nSteps 步数
 * \sa stepButton2()
 * \endif
 */
void QwtCounter::setStepButton2(int nSteps)
{
    setIncSteps(QwtCounter::Button2, nSteps);
}

/**
 * \if ENGLISH
 * @brief Return the number of increment steps for button 2
 * \sa setStepButton2()
 * \endif
 * \if CHINESE
 * @brief 返回按钮 2 的增加步数
 * \sa setStepButton2()
 * \endif
 */
int QwtCounter::stepButton2() const
{
    return incSteps(QwtCounter::Button2);
}

/**
 * \if ENGLISH
 * @brief Set the number of increment steps for button 3
 * @param nSteps Number of steps
 * \sa stepButton3()
 * \endif
 * \if CHINESE
 * @brief 设置按钮 3 的增加步数
 * @param nSteps 步数
 * \sa stepButton3()
 * \endif
 */
void QwtCounter::setStepButton3(int nSteps)
{
    setIncSteps(QwtCounter::Button3, nSteps);
}

/**
 * \if ENGLISH
 * @brief Return the number of increment steps for button 3
 * \sa setStepButton3()
 * \endif
 * \if CHINESE
 * @brief 返回按钮 3 的增加步数
 * \sa setStepButton3()
 * \endif
 */
int QwtCounter::stepButton3() const
{
    return incSteps(QwtCounter::Button3);
}

//! Set from lineedit
void QwtCounter::textChanged()
{
    bool converted = false;

    const double value = m_data->valueEdit->text().toDouble(&converted);
    if (converted)
        setValue(value);
}

/*!
   Handle QEvent::PolishRequest events
   \param event Event
   \return see QWidget::event()
 */
bool QwtCounter::event(QEvent* event)
{
    if (event->type() == QEvent::PolishRequest) {
        const QFontMetrics fm = m_data->valueEdit->fontMetrics();

        const int w = QwtPainter::horizontalAdvance(fm, "W") + 8;
        for (int i = 0; i < ButtonCnt; i++) {
            m_data->buttonDown[ i ]->setMinimumWidth(w);
            m_data->buttonUp[ i ]->setMinimumWidth(w);
        }
    }

    return QWidget::event(event);
}

/*!
   Handle key events

   - Ctrl + Qt::Key_Home\n
    Step to minimum()
   - Ctrl + Qt::Key_End\n
    Step to maximum()
   - Qt::Key_Up\n
    Increment by incSteps(QwtCounter::Button1)
   - Qt::Key_Down\n
    Decrement by incSteps(QwtCounter::Button1)
   - Qt::Key_PageUp\n
    Increment by incSteps(QwtCounter::Button2)
   - Qt::Key_PageDown\n
    Decrement by incSteps(QwtCounter::Button2)
   - Shift + Qt::Key_PageUp\n
    Increment by incSteps(QwtCounter::Button3)
   - Shift + Qt::Key_PageDown\n
    Decrement by incSteps(QwtCounter::Button3)

   \param event Key event
 */
void QwtCounter::keyPressEvent(QKeyEvent* event)
{
    bool accepted = true;

    switch (event->key()) {
    case Qt::Key_Home: {
        if (event->modifiers() & Qt::ControlModifier)
            setValue(minimum());
        else
            accepted = false;
        break;
    }
    case Qt::Key_End: {
        if (event->modifiers() & Qt::ControlModifier)
            setValue(maximum());
        else
            accepted = false;
        break;
    }
    case Qt::Key_Up: {
        incrementValue(m_data->increment[ 0 ]);
        break;
    }
    case Qt::Key_Down: {
        incrementValue(-m_data->increment[ 0 ]);
        break;
    }
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        int increment = m_data->increment[ 0 ];
        if (m_data->numButtons >= 2)
            increment = m_data->increment[ 1 ];
        if (m_data->numButtons >= 3) {
            if (event->modifiers() & Qt::ShiftModifier)
                increment = m_data->increment[ 2 ];
        }
        if (event->key() == Qt::Key_PageDown)
            increment = -increment;
        incrementValue(increment);
        break;
    }
    default: {
        accepted = false;
    }
    }

    if (accepted) {
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

/*!
   Handle wheel events
   \param event Wheel event
 */
void QwtCounter::wheelEvent(QWheelEvent* event)
{
    event->accept();

    if (m_data->numButtons <= 0)
        return;

    int increment = m_data->increment[ 0 ];
    if (m_data->numButtons >= 2) {
        if (event->modifiers() & Qt::ControlModifier)
            increment = m_data->increment[ 1 ];
    }
    if (m_data->numButtons >= 3) {
        if (event->modifiers() & Qt::ShiftModifier)
            increment = m_data->increment[ 2 ];
    }

#if QT_VERSION < 0x050e00
    const QPoint wheelPos = event->pos();
    const int wheelDelta  = event->delta();
#else
    const QPoint wheelPos = event->position().toPoint();

    const QPoint delta   = event->angleDelta();
    const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
#endif

    for (int i = 0; i < m_data->numButtons; i++) {
        if (m_data->buttonDown[ i ]->geometry().contains(wheelPos) || m_data->buttonUp[ i ]->geometry().contains(wheelPos)) {
            increment = m_data->increment[ i ];
        }
    }

    incrementValue(wheelDelta / 120 * increment);
}

void QwtCounter::incrementValue(int numSteps)
{
    const double min = m_data->minimum;
    const double max = m_data->maximum;
    double stepSize  = m_data->singleStep;

    if (!m_data->isValid || min >= max || stepSize <= 0.0)
        return;

#if 1
    stepSize = qwtMaxF(stepSize, 1.0e-10 * (max - min));
#endif

    double value = m_data->value + numSteps * stepSize;

    if (m_data->wrapping) {
        const double range = max - min;

        if (value < min) {
            value += std::ceil((min - value) / range) * range;
        } else if (value > max) {
            value -= std::ceil((value - max) / range) * range;
        }
    } else {
        value = qBound(min, value, max);
    }

    value = min + qRound((value - min) / stepSize) * stepSize;

    if (stepSize > 1e-12) {
        if (qFuzzyCompare(value + 1.0, 1.0)) {
            // correct rounding error if value = 0
            value = 0.0;
        } else if (qFuzzyCompare(value, max)) {
            // correct rounding error at the border
            value = max;
        }
    }

    if (value != m_data->value) {
        m_data->value = value;
        showNumber(m_data->value);
        updateButtons();

        Q_EMIT valueChanged(m_data->value);
    }
}

/*!
   \brief Update buttons according to the current value

   When the QwtCounter under- or over-flows, the focus is set to the smallest
   up- or down-button and counting is disabled.

   Counting is re-enabled on a button release event (mouse or space bar).
 */
void QwtCounter::updateButtons()
{
    if (m_data->isValid) {
        // 1. save enabled state of the smallest down- and up-button
        // 2. change enabled state on under- or over-flow

        for (int i = 0; i < QwtCounter::ButtonCnt; i++) {
            m_data->buttonDown[ i ]->setEnabled(value() > minimum());
            m_data->buttonUp[ i ]->setEnabled(value() < maximum());
        }
    } else {
        for (int i = 0; i < QwtCounter::ButtonCnt; i++) {
            m_data->buttonDown[ i ]->setEnabled(false);
            m_data->buttonUp[ i ]->setEnabled(false);
        }
    }
}
/*!
   Display number string

   \param number Number
 */
void QwtCounter::showNumber(double number)
{
    QString text;
    text.setNum(number);

    const int cursorPos = m_data->valueEdit->cursorPosition();
    m_data->valueEdit->setText(text);
    m_data->valueEdit->setCursorPosition(cursorPos);
}

//!  Button clicked
void QwtCounter::btnClicked()
{
    for (int i = 0; i < ButtonCnt; i++) {
        if (m_data->buttonUp[ i ] == sender())
            incrementValue(m_data->increment[ i ]);

        if (m_data->buttonDown[ i ] == sender())
            incrementValue(-m_data->increment[ i ]);
    }
}

//!  Button released
void QwtCounter::btnReleased()
{
    Q_EMIT buttonReleased(value());
}

/**
 * \if ENGLISH
 * @brief Return a size hint
 * \endif
 * \if CHINESE
 * @brief 返回尺寸提示
 * \endif
 */
QSize QwtCounter::sizeHint() const
{
    QString tmp;

    int w  = tmp.setNum(minimum()).length();
    int w1 = tmp.setNum(maximum()).length();
    if (w1 > w)
        w = w1;
    w1 = tmp.setNum(minimum() + singleStep()).length();
    if (w1 > w)
        w = w1;
    w1 = tmp.setNum(maximum() - singleStep()).length();
    if (w1 > w)
        w = w1;

    tmp.fill('9', w);

    w = QwtPainter::horizontalAdvance(m_data->valueEdit->fontMetrics(), tmp) + 2;

    if (m_data->valueEdit->hasFrame())
        w += 2 * style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    // Now we replace default sizeHint contribution of m_data->valueEdit by
    // what we really need.

    w += QWidget::sizeHint().width() - m_data->valueEdit->sizeHint().width();

    const int h = qMin(QWidget::sizeHint().height(), m_data->valueEdit->minimumSizeHint().height());

    return QSize(w, h);
}
