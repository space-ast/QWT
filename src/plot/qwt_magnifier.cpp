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

#include "qwt_magnifier.h"
#include "qwt_math.h"

#include <qevent.h>
#include <qwidget.h>

class QwtMagnifier::PrivateData
{
public:
    PrivateData()
        : isEnabled(false)
        , wheelFactor(0.9)
        , wheelModifiers(Qt::NoModifier)
        , mouseFactor(0.95)
        , mouseButton(Qt::RightButton)
        , mouseButtonModifiers(Qt::NoModifier)
        , keyFactor(0.9)
        , zoomInKey(Qt::Key_Plus)
        , zoomInKeyModifiers(Qt::NoModifier)
        , zoomOutKey(Qt::Key_Minus)
        , zoomOutKeyModifiers(Qt::NoModifier)
        , mousePressed(false)
        , hasMouseTracking(false)
    {
    }

    bool isEnabled;

    double wheelFactor;
    Qt::KeyboardModifiers wheelModifiers;

    double mouseFactor;

    Qt::MouseButton mouseButton;
    Qt::KeyboardModifiers mouseButtonModifiers;

    double keyFactor;

    int zoomInKey;
    Qt::KeyboardModifiers zoomInKeyModifiers;

    int zoomOutKey;
    Qt::KeyboardModifiers zoomOutKeyModifiers;

    bool mousePressed;
    bool hasMouseTracking;
    QPoint mousePos;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] parent Widget to be magnified
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param[in] parent 要放大的控件
 * \endif
 */
QwtMagnifier::QwtMagnifier(QWidget* parent) : QObject(parent)
{
    m_data = new PrivateData();

    if (parent) {
        if (parent->focusPolicy() == Qt::NoFocus)
            parent->setFocusPolicy(Qt::WheelFocus);
    }

    setEnabled(true);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtMagnifier::~QwtMagnifier()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief En/disable the magnifier
 * @details When enabled is true an event filter is installed for
 *          the observed widget, otherwise the event filter is removed.
 * @param[in] on true or false
 * @sa isEnabled(), eventFilter()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用放大器
 * @details 当 enabled 为 true 时，为观察的控件安装事件过滤器，
 *          否则移除事件过滤器。
 * @param[in] on true 或 false
 * @sa isEnabled(), eventFilter()
 * \endif
 */
void QwtMagnifier::setEnabled(bool on)
{
    if (m_data->isEnabled != on) {
        m_data->isEnabled = on;

        QObject* o = parent();
        if (o) {
            if (m_data->isEnabled)
                o->installEventFilter(this);
            else
                o->removeEventFilter(this);
        }
    }
}

/**
 * \if ENGLISH
 * @brief Return whether the magnifier is enabled
 * @return true when enabled, false otherwise
 * @sa setEnabled(), eventFilter()
 * \endif
 * \if CHINESE
 * @brief 返回放大器是否启用
 * @return 启用时返回 true，否则返回 false
 * @sa setEnabled(), eventFilter()
 * \endif
 */
bool QwtMagnifier::isEnabled() const
{
    return m_data->isEnabled;
}

/**
 * \if ENGLISH
 * @brief Change the wheel factor
 * @details The wheel factor defines the ratio between the current range
 *          on the parent widget and the zoomed range for each step of the wheel.
 *          Use values > 1 for magnification (i.e. 2.0) and values < 1 for
 *          scaling down (i.e. 1/2.0 = 0.5). You can use this feature for
 *          inverting the direction of the wheel.
 *          The default value is 0.9.
 * @param[in] factor Wheel factor
 * @sa wheelFactor(), setWheelButtonState(), setMouseFactor(), setKeyFactor()
 * \endif
 * \if CHINESE
 * @brief 更改滚轮因子
 * @details 滚轮因子定义了父控件当前范围与滚轮每步缩放范围之间的比率。
 *          使用 > 1 的值进行放大（如 2.0），使用 < 1 的值进行缩小（如 1/2.0 = 0.5）。
 *          您可以使用此功能反转滚轮方向。
 *          默认值为 0.9。
 * @param[in] factor 滚轮因子
 * @sa wheelFactor(), setWheelButtonState(), setMouseFactor(), setKeyFactor()
 * \endif
 */
void QwtMagnifier::setWheelFactor(double factor)
{
    m_data->wheelFactor = factor;
}

/**
 * \if ENGLISH
 * @brief Return the wheel factor
 * @return Wheel factor
 * @sa setWheelFactor()
 * \endif
 * \if CHINESE
 * @brief 返回滚轮因子
 * @return 滚轮因子
 * @sa setWheelFactor()
 * \endif
 */
double QwtMagnifier::wheelFactor() const
{
    return m_data->wheelFactor;
}

/**
 * \if ENGLISH
 * @brief Assign keyboard modifiers for zooming in/out using the wheel
 * @details The default modifiers are Qt::NoModifiers.
 * @param[in] modifiers Keyboard modifiers
 * @sa wheelModifiers()
 * \endif
 * \if CHINESE
 * @brief 设置使用滚轮放大/缩小时的键盘修饰键
 * @details 默认修饰键为 Qt::NoModifiers。
 * @param[in] modifiers 键盘修饰键
 * @sa wheelModifiers()
 * \endif
 */
void QwtMagnifier::setWheelModifiers(Qt::KeyboardModifiers modifiers)
{
    m_data->wheelModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Return the wheel modifiers
 * @return Wheel modifiers
 * @sa setWheelModifiers()
 * \endif
 * \if CHINESE
 * @brief 返回滚轮修饰键
 * @return 滚轮修饰键
 * @sa setWheelModifiers()
 * \endif
 */
Qt::KeyboardModifiers QwtMagnifier::wheelModifiers() const
{
    return m_data->wheelModifiers;
}

/**
 * \if ENGLISH
 * @brief Change the mouse factor
 * @details The mouse factor defines the ratio between the current range
 *          on the parent widget and the zoomed range for each vertical mouse movement.
 *          The default value is 0.95.
 * @param[in] factor Mouse factor
 * @sa mouseFactor(), setMouseButton(), setWheelFactor(), setKeyFactor()
 * \endif
 * \if CHINESE
 * @brief 更改鼠标因子
 * @details 鼠标因子定义了父控件当前范围与每次垂直鼠标移动缩放范围之间的比率。
 *          默认值为 0.95。
 * @param[in] factor 鼠标因子
 * @sa mouseFactor(), setMouseButton(), setWheelFactor(), setKeyFactor()
 * \endif
 */
void QwtMagnifier::setMouseFactor(double factor)
{
    m_data->mouseFactor = factor;
}

/**
 * \if ENGLISH
 * @brief Return the mouse factor
 * @return Mouse factor
 * @sa setMouseFactor()
 * \endif
 * \if CHINESE
 * @brief 返回鼠标因子
 * @return 鼠标因子
 * @sa setMouseFactor()
 * \endif
 */
double QwtMagnifier::mouseFactor() const
{
    return m_data->mouseFactor;
}

/**
 * \if ENGLISH
 * @brief Assign the mouse button that is used for zooming in/out
 * @details The default value is Qt::RightButton.
 * @param[in] button Button
 * @param[in] modifiers Keyboard modifiers
 * @sa getMouseButton()
 * \endif
 * \if CHINESE
 * @brief 设置用于放大/缩小的鼠标按钮
 * @details 默认值为 Qt::RightButton。
 * @param[in] button 鼠标按钮
 * @param[in] modifiers 键盘修饰键
 * @sa getMouseButton()
 * \endif
 */
void QwtMagnifier::setMouseButton(Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
    m_data->mouseButton          = button;
    m_data->mouseButtonModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Get the mouse button and modifiers used for zooming
 * @param[out] button Mouse button used for zooming
 * @param[out] modifiers Keyboard modifiers used for zooming
 * @sa setMouseButton()
 * \endif
 * \if CHINESE
 * @brief 获取用于放大/缩小的鼠标按钮和修饰键
 * @param[out] button 用于放大/缩小的鼠标按钮
 * @param[out] modifiers 用于放大/缩小的键盘修饰键
 * @sa setMouseButton()
 * \endif
 */
void QwtMagnifier::getMouseButton(Qt::MouseButton& button, Qt::KeyboardModifiers& modifiers) const
{
    button    = m_data->mouseButton;
    modifiers = m_data->mouseButtonModifiers;
}

/**
 * \if ENGLISH
 * @brief Change the key factor
 * @details The key factor defines the ratio between the current range
 *          on the parent widget and the zoomed range for each key press of
 *          the zoom in/out keys. The default value is 0.9.
 * @param[in] factor Key factor
 * @sa keyFactor(), setZoomInKey(), setZoomOutKey(), setWheelFactor(), setMouseFactor()
 * \endif
 * \if CHINESE
 * @brief 更改按键因子
 * @details 按键因子定义了父控件当前范围与每次按下放大/缩小键缩放范围之间的比率。
 *          默认值为 0.9。
 * @param[in] factor 按键因子
 * @sa keyFactor(), setZoomInKey(), setZoomOutKey(), setWheelFactor(), setMouseFactor()
 * \endif
 */
void QwtMagnifier::setKeyFactor(double factor)
{
    m_data->keyFactor = factor;
}

/**
 * \if ENGLISH
 * @brief Return the key factor
 * @return Key factor
 * @sa setKeyFactor()
 * \endif
 * \if CHINESE
 * @brief 返回按键因子
 * @return 按键因子
 * @sa setKeyFactor()
 * \endif
 */
double QwtMagnifier::keyFactor() const
{
    return m_data->keyFactor;
}

/**
 * \if ENGLISH
 * @brief Assign the key that is used for zooming in
 * @details The default combination is Qt::Key_Plus + Qt::NoModifier.
 * @param[in] key Key code
 * @param[in] modifiers Keyboard modifiers
 * @sa getZoomInKey(), setZoomOutKey()
 * \endif
 * \if CHINESE
 * @brief 设置用于放大的按键
 * @details 默认组合为 Qt::Key_Plus + Qt::NoModifier。
 * @param[in] key 按键代码
 * @param[in] modifiers 键盘修饰键
 * @sa getZoomInKey(), setZoomOutKey()
 * \endif
 */
void QwtMagnifier::setZoomInKey(int key, Qt::KeyboardModifiers modifiers)
{
    m_data->zoomInKey          = key;
    m_data->zoomInKeyModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Retrieve the settings of the zoom in key
 * @param[out] key Key code, see Qt::Key
 * @param[out] modifiers Keyboard modifiers
 * @sa setZoomInKey()
 * \endif
 * \if CHINESE
 * @brief 获取放大键的设置
 * @param[out] key 按键代码，参见 Qt::Key
 * @param[out] modifiers 键盘修饰键
 * @sa setZoomInKey()
 * \endif
 */
void QwtMagnifier::getZoomInKey(int& key, Qt::KeyboardModifiers& modifiers) const
{
    key       = m_data->zoomInKey;
    modifiers = m_data->zoomInKeyModifiers;
}

/**
 * \if ENGLISH
 * @brief Assign the key that is used for zooming out
 * @details The default combination is Qt::Key_Minus + Qt::NoModifier.
 * @param[in] key Key code
 * @param[in] modifiers Keyboard modifiers
 * @sa getZoomOutKey(), setZoomInKey()
 * \endif
 * \if CHINESE
 * @brief 设置用于缩小的按键
 * @details 默认组合为 Qt::Key_Minus + Qt::NoModifier。
 * @param[in] key 按键代码
 * @param[in] modifiers 键盘修饰键
 * @sa getZoomOutKey(), setZoomInKey()
 * \endif
 */
void QwtMagnifier::setZoomOutKey(int key, Qt::KeyboardModifiers modifiers)
{
    m_data->zoomOutKey          = key;
    m_data->zoomOutKeyModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Retrieve the settings of the zoom out key
 * @param[out] key Key code, see Qt::Key
 * @param[out] modifiers Keyboard modifiers
 * @sa setZoomOutKey()
 * \endif
 * \if CHINESE
 * @brief 获取缩小键的设置
 * @param[out] key 按键代码，参见 Qt::Key
 * @param[out] modifiers 键盘修饰键
 * @sa setZoomOutKey()
 * \endif
 */
void QwtMagnifier::getZoomOutKey(int& key, Qt::KeyboardModifiers& modifiers) const
{
    key       = m_data->zoomOutKey;
    modifiers = m_data->zoomOutKeyModifiers;
}

/**
 * \if ENGLISH
 * @brief Event filter
 * @details When isEnabled() is true, the mouse events of the
 *          observed widget are filtered.
 * @param[in] object Object to be filtered
 * @param[in] event Event
 * @return Forwarded to QObject::eventFilter()
 * @sa widgetMousePressEvent(), widgetMouseReleaseEvent(),
 *     widgetMouseMoveEvent(), widgetWheelEvent(), widgetKeyPressEvent(),
 *     widgetKeyReleaseEvent()
 * \endif
 * \if CHINESE
 * @brief 事件过滤器
 * @details 当 isEnabled() 为 true 时，观察控件的鼠标事件被过滤。
 * @param[in] object 要过滤的对象
 * @param[in] event 事件
 * @return 转发给 QObject::eventFilter()
 * @sa widgetMousePressEvent(), widgetMouseReleaseEvent(),
 *     widgetMouseMoveEvent(), widgetWheelEvent(), widgetKeyPressEvent(),
 *     widgetKeyReleaseEvent()
 * \endif
 */
bool QwtMagnifier::eventFilter(QObject* object, QEvent* event)
{
    if (object && object == parent()) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            widgetMousePressEvent(static_cast< QMouseEvent* >(event));
            break;
        }
        case QEvent::MouseMove: {
            widgetMouseMoveEvent(static_cast< QMouseEvent* >(event));
            break;
        }
        case QEvent::MouseButtonRelease: {
            widgetMouseReleaseEvent(static_cast< QMouseEvent* >(event));
            break;
        }
        case QEvent::Wheel: {
            widgetWheelEvent(static_cast< QWheelEvent* >(event));
            break;
        }
        case QEvent::KeyPress: {
            widgetKeyPressEvent(static_cast< QKeyEvent* >(event));
            break;
        }
        case QEvent::KeyRelease: {
            widgetKeyReleaseEvent(static_cast< QKeyEvent* >(event));
            break;
        }
        default:;
        }
    }
    return QObject::eventFilter(object, event);
}

/*!
   Handle a mouse press event for the observed widget.

   \param mouseEvent Mouse event
   \sa eventFilter(), widgetMouseReleaseEvent(), widgetMouseMoveEvent()
 */
void QwtMagnifier::widgetMousePressEvent(QMouseEvent* mouseEvent)
{
    if (parentWidget() == nullptr)
        return;

    if ((mouseEvent->button() != m_data->mouseButton) || (mouseEvent->modifiers() != m_data->mouseButtonModifiers)) {
        return;
    }

    m_data->hasMouseTracking = parentWidget()->hasMouseTracking();

    parentWidget()->setMouseTracking(true);
    m_data->mousePos     = mouseEvent->pos();
    m_data->mousePressed = true;
}

/*!
   Handle a mouse release event for the observed widget.

   \param mouseEvent Mouse event

   \sa eventFilter(), widgetMousePressEvent(), widgetMouseMoveEvent(),
 */
void QwtMagnifier::widgetMouseReleaseEvent(QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent);

    if (m_data->mousePressed && parentWidget()) {
        m_data->mousePressed = false;
        parentWidget()->setMouseTracking(m_data->hasMouseTracking);
    }
}

/*!
   Handle a mouse move event for the observed widget.

   \param mouseEvent Mouse event
   \sa eventFilter(), widgetMousePressEvent(), widgetMouseReleaseEvent(),
 */
void QwtMagnifier::widgetMouseMoveEvent(QMouseEvent* mouseEvent)
{
    if (!m_data->mousePressed)
        return;

    const int dy = mouseEvent->pos().y() - m_data->mousePos.y();
    if (dy != 0) {
        double f = m_data->mouseFactor;
        if (dy < 0)
            f = 1 / f;

        rescale(f);
    }

    m_data->mousePos = mouseEvent->pos();
}

/*!
   Handle a wheel event for the observed widget.

   \param wheelEvent Wheel event
   \sa eventFilter()
 */
void QwtMagnifier::widgetWheelEvent(QWheelEvent* wheelEvent)
{
    if (wheelEvent->modifiers() != m_data->wheelModifiers) {
        return;
    }

    if (m_data->wheelFactor != 0.0) {
#if QT_VERSION < 0x050000
        const int wheelDelta = wheelEvent->delta();
#else
        const QPoint delta   = wheelEvent->angleDelta();
        const int wheelDelta = (qAbs(delta.x()) > qAbs(delta.y())) ? delta.x() : delta.y();
#endif

        /*
            A positive delta indicates that the wheel was
            rotated forwards away from the user; a negative
            value indicates that the wheel was rotated
            backwards toward the user.
            Most mouse types work in steps of 15 degrees,
            in which case the delta value is a multiple
            of 120 (== 15 * 8).
         */
        double f = std::pow(m_data->wheelFactor, qAbs(wheelDelta / 120.0));

        if (wheelDelta > 0)
            f = 1 / f;

        rescale(f);
    }
}

/*!
   Handle a key press event for the observed widget.

   \param keyEvent Key event
   \sa eventFilter(), widgetKeyReleaseEvent()
 */
void QwtMagnifier::widgetKeyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->key() == m_data->zoomInKey && keyEvent->modifiers() == m_data->zoomInKeyModifiers) {
        rescale(m_data->keyFactor);
    } else if (keyEvent->key() == m_data->zoomOutKey && keyEvent->modifiers() == m_data->zoomOutKeyModifiers) {
        rescale(1.0 / m_data->keyFactor);
    }
}

/*!
   Handle a key release event for the observed widget.

   \param keyEvent Key event
   \sa eventFilter(), widgetKeyReleaseEvent()
 */
void QwtMagnifier::widgetKeyReleaseEvent(QKeyEvent* keyEvent)
{
    Q_UNUSED(keyEvent);
}

/**
 * \if ENGLISH
 * @brief Return the parent widget where the rescaling happens
 * @return Parent widget
 * \endif
 * \if CHINESE
 * @brief 返回发生重新缩放的父控件
 * @return 父控件
 * \endif
 */
QWidget* QwtMagnifier::parentWidget()
{
    return qobject_cast< QWidget* >(parent());
}

/**
 * \if ENGLISH
 * @brief Return the parent widget where the rescaling happens
 * @return Parent widget (const)
 * \endif
 * \if CHINESE
 * @brief 返回发生重新缩放的父控件
 * @return 父控件（常量版本）
 * \endif
 */
const QWidget* QwtMagnifier::parentWidget() const
{
    return qobject_cast< const QWidget* >(parent());
}
