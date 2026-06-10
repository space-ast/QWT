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
    QWT_DECLARE_PUBLIC(QwtCounter)
public:
    PrivateData(QwtCounter* p) : q_ptr(p), minimum(0.0), maximum(0.0), singleStep(1.0), isValid(false), value(0.0), wrapping(false)
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
 * @brief Constructor
 * @details The counter is initialized with a range set to [0.0, 1.0] with
 *          0.01 as single step size. The value is invalid.
 *          The default number of buttons is set to 2. The default increments are:
 *          - Button 1: 1 step
 *          - Button 2: 10 steps
 *          - Button 3: 100 steps
 * @param parent Parent widget
 */
QwtCounter::QwtCounter(QWidget* parent) : QWidget(parent)
{
    initCounter();
}

void QwtCounter::initCounter()
{
    QWT_PIMPL_CONSTRUCT_INIT();
    QWT_D(d);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());

    for (int i = ButtonCnt - 1; i >= 0; i--) {
        QwtArrowButton* btn = new QwtArrowButton(i + 1, Qt::DownArrow, this);
        btn->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(btn);

        connect(btn, SIGNAL(released()), SLOT(btnReleased()));
        connect(btn, SIGNAL(clicked()), SLOT(btnClicked()));

        d->buttonDown[ i ] = btn;
    }

    d->valueEdit = new QLineEdit(this);
    d->valueEdit->setReadOnly(false);
    d->valueEdit->setValidator(new QDoubleValidator(d->valueEdit));
    layout->addWidget(d->valueEdit);

    connect(d->valueEdit, SIGNAL(editingFinished()), SLOT(textChanged()));

    layout->setStretchFactor(d->valueEdit, 10);

    for (int i = 0; i < ButtonCnt; i++) {
        QwtArrowButton* btn = new QwtArrowButton(i + 1, Qt::UpArrow, this);
        btn->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(btn);

        connect(btn, SIGNAL(released()), SLOT(btnReleased()));
        connect(btn, SIGNAL(clicked()), SLOT(btnClicked()));

        d->buttonUp[ i ] = btn;
    }

    setNumButtons(2);
    setRange(0.0, 1.0);
    setSingleStep(0.001);
    setValue(0.0);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    setFocusProxy(d->valueEdit);
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * @brief Destructor
 */
QwtCounter::~QwtCounter()
{
}

/**
 * @brief Set the counter to be in valid/invalid state
 * @details When the counter is set to invalid, no numbers are displayed and
 *          the buttons are disabled.
 * @param on If true the counter will be set as valid
 * @sa setValue(), isValid()
 */
void QwtCounter::setValid(bool on)
{
    QWT_D(d);
    if (on != d->isValid) {
        d->isValid = on;

        updateButtons();

        if (d->isValid) {
            showNumber(value());
            Q_EMIT valueChanged(value());
        } else {
            d->valueEdit->setText(QString());
        }
    }
}

/**
 * @brief Return true if the value is valid
 * @sa setValid(), setValue()
 */
bool QwtCounter::isValid() const
{
    QWT_DC(d);
    return d->isValid;
}

/**
 * @brief Allow/disallow the user to manually edit the value
 * @param on True disable editing
 * @sa isReadOnly()
 */
void QwtCounter::setReadOnly(bool on)
{
    QWT_D(d);
    d->valueEdit->setReadOnly(on);
}

/**
 * @brief Return true when the line edit is read only
 * @sa setReadOnly()
 */
bool QwtCounter::isReadOnly() const
{
    QWT_DC(d);
    return d->valueEdit->isReadOnly();
}

/**
 * @brief Set a new value without adjusting to the step raster
 * @details The state of the counter is set to be valid.
 * @param value New value
 * @sa isValid(), value(), valueChanged()
 * @warning The value is clipped when it lies outside the range.
 */

void QwtCounter::setValue(double value)
{
    QWT_D(d);
    const double vmin = qwtMinF(d->minimum, d->maximum);
    const double vmax = qwtMaxF(d->minimum, d->maximum);

    value = qBound(vmin, value, vmax);

    if (!d->isValid || value != d->value) {
        d->isValid = true;
        d->value   = value;

        showNumber(value);
        updateButtons();

        Q_EMIT valueChanged(value);
    }
}

/**
 * @brief Return current value of the counter
 * @sa setValue(), valueChanged()
 */
double QwtCounter::value() const
{
    QWT_DC(d);
    return d->value;
}

/**
 * @brief Set the minimum and maximum values
 * @details The maximum is adjusted if necessary to ensure that the range remains valid.
 *          The value might be modified to be inside of the range.
 * @param[in] min Minimum value
 * @param[in] max Maximum value
 * @sa minimum(), maximum()
 */
void QwtCounter::setRange(double min, double max)
{
    QWT_D(d);
    max = qwtMaxF(min, max);

    if (d->maximum == max && d->minimum == min)
        return;

    d->minimum = min;
    d->maximum = max;

    setSingleStep(singleStep());

    const double value = qBound(min, d->value, max);

    if (value != d->value) {
        d->value = value;

        if (d->isValid) {
            showNumber(value);
            Q_EMIT valueChanged(value);
        }
    }

    updateButtons();
}

/**
 * @brief Set the minimum value of the range
 * @param value Minimum value
 * @sa setRange(), setMaximum(), minimum()
 * @note The maximum is adjusted if necessary to ensure that the range remains valid.
 */
void QwtCounter::setMinimum(double value)
{
    setRange(value, maximum());
}

/**
 * @brief Return the minimum of the range
 * @sa setRange(), setMinimum(), maximum()
 */
double QwtCounter::minimum() const
{
    QWT_DC(d);
    return d->minimum;
}

/**
 * @brief Set the maximum value of the range
 * @param value Maximum value
 * @sa setRange(), setMinimum(), maximum()
 */
void QwtCounter::setMaximum(double value)
{
    setRange(minimum(), value);
}

/**
 * @brief Return the maximum of the range
 * @sa setRange(), setMaximum(), minimum()
 */
double QwtCounter::maximum() const
{
    QWT_DC(d);
    return d->maximum;
}

/**
 * @brief Set the step size of the counter
 * @details A value <= 0.0 disables stepping
 * @param stepSize Single step size
 * @sa singleStep()
 */
void QwtCounter::setSingleStep(double stepSize)
{
    QWT_D(d);
    d->singleStep = qwtMaxF(stepSize, 0.0);
}

/**
 * @brief Return single step size
 * @sa setSingleStep()
 */
double QwtCounter::singleStep() const
{
    QWT_DC(d);
    return d->singleStep;
}

/**
 * @brief Enable/disable wrapping
 * @details If wrapping is true stepping up from maximum() value will take
 *          you to the minimum() value and vice versa.
 * @param on Enable/disable wrapping
 * @sa wrapping()
 */
void QwtCounter::setWrapping(bool on)
{
    QWT_D(d);
    d->wrapping = on;
}

/**
 * @brief Return true when wrapping is set
 * @sa setWrapping()
 */
bool QwtCounter::wrapping() const
{
    QWT_DC(d);
    return d->wrapping;
}

/**
 * @brief Specify the number of buttons on each side of the label
 * @param numButtons Number of buttons
 * @sa numButtons()
 */
void QwtCounter::setNumButtons(int numButtons)
{
    QWT_D(d);
    if (numButtons < 0 || numButtons > QwtCounter::ButtonCnt)
        return;

    for (int i = 0; i < QwtCounter::ButtonCnt; i++) {
        if (i < numButtons) {
            d->buttonDown[ i ]->show();
            d->buttonUp[ i ]->show();
        } else {
            d->buttonDown[ i ]->hide();
            d->buttonUp[ i ]->hide();
        }
    }

    d->numButtons = numButtons;
}

/**
 * @brief Return the number of buttons on each side of the widget
 * @sa setNumButtons()
 */
int QwtCounter::numButtons() const
{
    QWT_DC(d);
    return d->numButtons;
}

/**
 * @brief Specify the number of steps by which the value is incremented or decremented when a specified button is pushed
 * @param[in] button Button index
 * @param[in] numSteps Number of steps
 * @sa incSteps()
 */
void QwtCounter::setIncSteps(QwtCounter::Button button, int numSteps)
{
    QWT_D(d);
    if (button >= 0 && button < QwtCounter::ButtonCnt)
        d->increment[ button ] = numSteps;
}

/**
 * @brief Return the number of steps by which a specified button increments the value, or 0 if the button is invalid
 * @param button Button index
 * @return Number of increment steps
 * @sa setIncSteps()
 */
int QwtCounter::incSteps(QwtCounter::Button button) const
{
    QWT_DC(d);
    if (button >= 0 && button < QwtCounter::ButtonCnt)
        return d->increment[ button ];

    return 0;
}

/**
 * @brief Set the number of increment steps for button 1
 * @param nSteps Number of steps
 * @sa stepButton1()
 */
void QwtCounter::setStepButton1(int nSteps)
{
    setIncSteps(QwtCounter::Button1, nSteps);
}

/**
 * @brief Return the number of increment steps for button 1
 * @sa setStepButton1()
 */
int QwtCounter::stepButton1() const
{
    return incSteps(QwtCounter::Button1);
}

/**
 * @brief Set the number of increment steps for button 2
 * @param nSteps Number of steps
 * @sa stepButton2()
 */
void QwtCounter::setStepButton2(int nSteps)
{
    setIncSteps(QwtCounter::Button2, nSteps);
}

/**
 * @brief Return the number of increment steps for button 2
 * @sa setStepButton2()
 */
int QwtCounter::stepButton2() const
{
    return incSteps(QwtCounter::Button2);
}

/**
 * @brief Set the number of increment steps for button 3
 * @param nSteps Number of steps
 * @sa stepButton3()
 */
void QwtCounter::setStepButton3(int nSteps)
{
    setIncSteps(QwtCounter::Button3, nSteps);
}

/**
 * @brief Return the number of increment steps for button 3
 * @sa setStepButton3()
 */
int QwtCounter::stepButton3() const
{
    return incSteps(QwtCounter::Button3);
}

//! Set from lineedit
void QwtCounter::textChanged()
{
    QWT_D(d);
    bool converted = false;

    const double value = d->valueEdit->text().toDouble(&converted);
    if (converted)
        setValue(value);
}

/*!
   Handle QEvent::PolishRequest events
   @param event Event
   @return see QWidget::event()
 */
bool QwtCounter::event(QEvent* event)
{
    QWT_D(d);
    if (event->type() == QEvent::PolishRequest) {
        const QFontMetrics fm = d->valueEdit->fontMetrics();

        const int w = QwtPainter::horizontalAdvance(fm, "W") + 8;
        for (int i = 0; i < ButtonCnt; i++) {
            d->buttonDown[ i ]->setMinimumWidth(w);
            d->buttonUp[ i ]->setMinimumWidth(w);
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

   @param event Key event
 */
void QwtCounter::keyPressEvent(QKeyEvent* event)
{
    QWT_D(d);
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
        incrementValue(d->increment[ 0 ]);
        break;
    }
    case Qt::Key_Down: {
        incrementValue(-d->increment[ 0 ]);
        break;
    }
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        int increment = d->increment[ 0 ];
        if (d->numButtons >= 2)
            increment = d->increment[ 1 ];
        if (d->numButtons >= 3) {
            if (event->modifiers() & Qt::ShiftModifier)
                increment = d->increment[ 2 ];
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
   @param event Wheel event
 */
void QwtCounter::wheelEvent(QWheelEvent* event)
{
    QWT_D(d);
    event->accept();

    if (d->numButtons <= 0)
        return;

    int increment = d->increment[ 0 ];
    if (d->numButtons >= 2) {
        if (event->modifiers() & Qt::ControlModifier)
            increment = d->increment[ 1 ];
    }
    if (d->numButtons >= 3) {
        if (event->modifiers() & Qt::ShiftModifier)
            increment = d->increment[ 2 ];
    }

#if QT_VERSION < 0x050e00
    const QPoint wheelPos = event->pos();
    const int wheelDelta  = event->delta();
#else
    const QPoint wheelPos = event->position().toPoint();

    const QPoint delta   = event->angleDelta();
    const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
#endif

    for (int i = 0; i < d->numButtons; i++) {
        if (d->buttonDown[ i ]->geometry().contains(wheelPos) || d->buttonUp[ i ]->geometry().contains(wheelPos)) {
            increment = d->increment[ i ];
        }
    }

    incrementValue(wheelDelta / 120 * increment);
}

void QwtCounter::incrementValue(int numSteps)
{
    QWT_D(d);
    const double min = d->minimum;
    const double max = d->maximum;
    double stepSize  = d->singleStep;

    if (!d->isValid || min >= max || stepSize <= 0.0)
        return;

#if 1
    stepSize = qwtMaxF(stepSize, 1.0e-10 * (max - min));
#endif

    double value = d->value + numSteps * stepSize;

    if (d->wrapping) {
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

    if (value != d->value) {
        d->value = value;
        showNumber(d->value);
        updateButtons();

        Q_EMIT valueChanged(d->value);
    }
}

/*!
   @brief Update buttons according to the current value

   When the QwtCounter under- or over-flows, the focus is set to the smallest
   up- or down-button and counting is disabled.

   Counting is re-enabled on a button release event (mouse or space bar).
 */
void QwtCounter::updateButtons()
{
    QWT_D(d);
    if (d->isValid) {
        // 1. save enabled state of the smallest down- and up-button
        // 2. change enabled state on under- or over-flow

        for (int i = 0; i < QwtCounter::ButtonCnt; i++) {
            d->buttonDown[ i ]->setEnabled(value() > minimum());
            d->buttonUp[ i ]->setEnabled(value() < maximum());
        }
    } else {
        for (int i = 0; i < QwtCounter::ButtonCnt; i++) {
            d->buttonDown[ i ]->setEnabled(false);
            d->buttonUp[ i ]->setEnabled(false);
        }
    }
}
/*!
   Display number string

   @param number Number
 */
void QwtCounter::showNumber(double number)
{
    QWT_D(d);
    QString text;
    text.setNum(number);

    const int cursorPos = d->valueEdit->cursorPosition();
    d->valueEdit->setText(text);
    d->valueEdit->setCursorPosition(cursorPos);
}

//!  Button clicked
void QwtCounter::btnClicked()
{
    QWT_D(d);
    for (int i = 0; i < ButtonCnt; i++) {
        if (d->buttonUp[ i ] == sender())
            incrementValue(d->increment[ i ]);

        if (d->buttonDown[ i ] == sender())
            incrementValue(-d->increment[ i ]);
    }
}

//!  Button released
void QwtCounter::btnReleased()
{
    Q_EMIT buttonReleased(value());
}

/**
 * @brief Return a size hint
 */
QSize QwtCounter::sizeHint() const
{
    QWT_DC(d);
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

    w = QwtPainter::horizontalAdvance(d->valueEdit->fontMetrics(), tmp) + 2;

    if (d->valueEdit->hasFrame())
        w += 2 * style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    // Now we replace default sizeHint contribution of d->valueEdit by
    // what we really need.

    w += QWidget::sizeHint().width() - d->valueEdit->sizeHint().width();

    const int h = qMin(QWidget::sizeHint().height(), d->valueEdit->minimumSizeHint().height());

    return QSize(w, h);
}
