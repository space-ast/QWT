/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by czy in 2025 <czy.t@163.com>
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

#include "qwt_cache_panner.h"
#include "qwt_picker.h"
#include "qwt_painter.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qcursor.h>
#include <qbitmap.h>

static QVector< QwtPicker* > qwtActivePickers(QWidget* w)
{
    QVector< QwtPicker* > pickers;

    QObjectList children = w->children();
    for (int i = 0; i < children.size(); i++) {
        QwtPicker* picker = qobject_cast< QwtPicker* >(children[ i ]);
        if (picker && picker->isEnabled())
            pickers += picker;
    }

    return pickers;
}

class QwtCachePanner::PrivateData
{
public:
    PrivateData()
        : button(Qt::LeftButton)
        , buttonModifiers(Qt::NoModifier)
        , abortKey(Qt::Key_Escape)
        , abortKeyModifiers(Qt::NoModifier)
#ifndef QT_NO_CURSOR
        , cursor(nullptr)
        , restoreCursor(nullptr)
        , hasCursor(false)
#endif
        , isEnabled(false)
        , orientations(Qt::Vertical | Qt::Horizontal)
    {
    }

    ~PrivateData()
    {
#ifndef QT_NO_CURSOR
        delete cursor;
        delete restoreCursor;
#endif
    }

    Qt::MouseButton button;
    Qt::KeyboardModifiers buttonModifiers;

    int abortKey;
    Qt::KeyboardModifiers abortKeyModifiers;

    QPoint initialPos;
    QPoint pos;

    QPixmap pixmap;
    QBitmap contentsMask;

#ifndef QT_NO_CURSOR
    QCursor* cursor;
    QCursor* restoreCursor;
    bool hasCursor;
#endif
    bool isEnabled;
    Qt::Orientations orientations;
};

/**
 * \if ENGLISH
 * @brief Creates a panner that is enabled for the left mouse button
 * @param[in] parent Parent widget to be panned
 * \endif
 * \if CHINESE
 * @brief 创建一个启用左键鼠标按钮的平移器
 * @param[in] parent 要平移的父控件
 * \endif
 */
QwtCachePanner::QwtCachePanner(QWidget* parent) : QWidget(parent)
{
    m_data = new PrivateData();

    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::NoFocus);
    hide();

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
QwtCachePanner::~QwtCachePanner()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Change the mouse button and modifiers used for panning
 * @details The defaults are Qt::LeftButton and Qt::NoModifier
 * @param[in] button Mouse button for panning
 * @param[in] modifiers Keyboard modifiers
 * \endif
 * \if CHINESE
 * @brief 更改用于平移的鼠标按钮和修饰键
 * @details 默认值为 Qt::LeftButton 和 Qt::NoModifier
 * @param[in] button 用于平移的鼠标按钮
 * @param[in] modifiers 键盘修饰键
 * \endif
 */
void QwtCachePanner::setMouseButton(Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
    m_data->button          = button;
    m_data->buttonModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Get mouse button and modifiers used for panning
 * @param[out] button Mouse button used for panning
 * @param[out] modifiers Keyboard modifiers used for panning
 * \endif
 * \if CHINESE
 * @brief 获取用于平移的鼠标按钮和修饰键
 * @param[out] button 用于平移的鼠标按钮
 * @param[out] modifiers 用于平移的键盘修饰键
 * \endif
 */
void QwtCachePanner::getMouseButton(Qt::MouseButton& button, Qt::KeyboardModifiers& modifiers) const
{
    button    = m_data->button;
    modifiers = m_data->buttonModifiers;
}

/**
 * \if ENGLISH
 * @brief Change the abort key
 * @details The defaults are Qt::Key_Escape and Qt::NoModifiers
 * @param[in] key Key code (See Qt::Keycode)
 * @param[in] modifiers Keyboard modifiers
 * \endif
 * \if CHINESE
 * @brief 更改中止键
 * @details 默认值为 Qt::Key_Escape 和 Qt::NoModifiers
 * @param[in] key 键代码（参见 Qt::Keycode）
 * @param[in] modifiers 键盘修饰键
 * \endif
 */
void QwtCachePanner::setAbortKey(int key, Qt::KeyboardModifiers modifiers)
{
    m_data->abortKey          = key;
    m_data->abortKeyModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Get the abort key and modifiers
 * @param[out] key Key code used for abort
 * @param[out] modifiers Keyboard modifiers used for abort
 * \endif
 * \if CHINESE
 * @brief 获取中止键和修饰键
 * @param[out] key 用于中止的键代码
 * @param[out] modifiers 用于中止的键盘修饰键
 * \endif
 */
void QwtCachePanner::getAbortKey(int& key, Qt::KeyboardModifiers& modifiers) const
{
    key       = m_data->abortKey;
    modifiers = m_data->abortKeyModifiers;
}

/**
 * \if ENGLISH
 * @brief Change the cursor that is active while panning
 * @details The default is the cursor of the parent widget.
 * @param[in] cursor New cursor
 * @sa setCursor()
 * \endif
 * \if CHINESE
 * @brief 更改平移时激活的光标
 * @details 默认值是父控件的光标。
 * @param[in] cursor 新的光标
 * @sa setCursor()
 * \endif
 */
#ifndef QT_NO_CURSOR
void QwtCachePanner::setCursor(const QCursor& cursor)
{
    m_data->cursor = new QCursor(cursor);
}
#endif

/**
 * \if ENGLISH
 * @brief Return the cursor that is active while panning
 * @return Cursor active while panning
 * @sa setCursor()
 * \endif
 * \if CHINESE
 * @brief 返回平移时激活的光标
 * @return 平移时激活的光标
 * @sa setCursor()
 * \endif
 */
#ifndef QT_NO_CURSOR
const QCursor QwtCachePanner::cursor() const
{
    if (m_data->cursor)
        return *m_data->cursor;

    if (parentWidget())
        return parentWidget()->cursor();

    return QCursor();
}
#endif

/**
 * \if ENGLISH
 * @brief En/disable the panner
 * @details When enabled is true an event filter is installed for
 *          the observed widget, otherwise the event filter is removed.
 * @param[in] on true or false
 * @sa isEnabled(), eventFilter()
 * \endif
 * \if CHINESE
 * @brief 启用/禁用平移器
 * @details 当 enabled 为 true 时，为观察的控件安装事件过滤器，
 *          否则移除事件过滤器。
 * @param[in] on true 或 false
 * @sa isEnabled(), eventFilter()
 * \endif
 */
void QwtCachePanner::setEnabled(bool on)
{
    if (m_data->isEnabled != on) {
        m_data->isEnabled = on;

        QWidget* w = parentWidget();
        if (w) {
            if (m_data->isEnabled) {
                w->installEventFilter(this);
            } else {
                w->removeEventFilter(this);
                hide();
            }
        }
    }
}

/**
 * \if ENGLISH
 * @brief Set the orientations where panning is enabled
 * @details The default value is in both directions: Qt::Horizontal | Qt::Vertical
 * @param[in] o Orientation flags
 * \endif
 * \if CHINESE
 * @brief 设置启用平移的方向
 * @details 默认值为双向：Qt::Horizontal | Qt::Vertical
 * @param[in] o 方向标志
 * \endif
 */
void QwtCachePanner::setOrientations(Qt::Orientations o)
{
    m_data->orientations = o;
}

/**
 * \if ENGLISH
 * @brief Return the orientation where panning is enabled
 * @return Orientation flags
 * \endif
 * \if CHINESE
 * @brief 返回启用平移的方向
 * @return 方向标志
 * \endif
 */
Qt::Orientations QwtCachePanner::orientations() const
{
    return m_data->orientations;
}

/**
 * \if ENGLISH
 * @brief Check if an orientation is enabled
 * @param[in] o Orientation to check
 * @return True if the orientation is enabled
 * @sa orientations(), setOrientations()
 * \endif
 * \if CHINESE
 * @brief 检查某个方向是否启用
 * @param[in] o 要检查的方向
 * @return 如果方向启用则返回 true
 * @sa orientations(), setOrientations()
 * \endif
 */
bool QwtCachePanner::isOrientationEnabled(Qt::Orientation o) const
{
    return m_data->orientations & o;
}

/**
 * \if ENGLISH
 * @brief Return whether the panner is enabled
 * @return true when enabled, false otherwise
 * @sa setEnabled(), eventFilter()
 * \endif
 * \if CHINESE
 * @brief 返回平移器是否启用
 * @return 启用时返回 true，否则返回 false
 * @sa setEnabled(), eventFilter()
 * \endif
 */
bool QwtCachePanner::isEnabled() const
{
    return m_data->isEnabled;
}

/**
 * \if ENGLISH
 * @brief Paint event
 * @details Repaint the grabbed pixmap on its current position and
 *          fill the empty spaces by the background of the parent widget.
 * @param[in] event Paint event
 * \endif
 * \if CHINESE
 * @brief 绘制事件
 * @details 在当前位置重绘捕获的 pixmap，并用父控件的背景填充空白区域。
 * @param[in] event 绘制事件
 * \endif
 */
void QwtCachePanner::paintEvent(QPaintEvent* event)
{
    int dx = m_data->pos.x() - m_data->initialPos.x();
    int dy = m_data->pos.y() - m_data->initialPos.y();

    QRectF r;
    r.setSize(m_data->pixmap.size() / QwtPainter::devicePixelRatio(&m_data->pixmap));
    r.moveCenter(QPointF(r.center().x() + dx, r.center().y() + dy));

    QPixmap pm = QwtPainter::backingStore(this, size());
    QwtPainter::fillPixmap(parentWidget(), pm);

    QPainter painter(&pm);

    if (!m_data->contentsMask.isNull()) {
        QPixmap masked = m_data->pixmap;
        masked.setMask(m_data->contentsMask);
        painter.drawPixmap(r.toRect(), masked);
    } else {
        painter.drawPixmap(r.toRect(), m_data->pixmap);
    }

    painter.end();

    if (!m_data->contentsMask.isNull())
        pm.setMask(m_data->contentsMask);

    painter.begin(this);
    painter.setClipRegion(event->region());
    painter.drawPixmap(0, 0, pm);
}

/**
 * \if ENGLISH
 * @brief Calculate a mask for the contents of the panned widget
 * @details Sometimes only parts of the contents of a widget should be
 *          panned. F.e. for a widget with a styled background with rounded borders
 *          only the area inside of the border should be panned.
 * @return An empty bitmap, indicating no mask
 * \endif
 * \if CHINESE
 * @brief 计算平移控件内容的遮罩
 * @details 有时只应平移控件内容的某些部分。例如，对于带有圆角边框样式背景的控件，
 *          只应平移边框内部的区域。
 * @return 空位图，表示无遮罩
 * \endif
 */
QBitmap QwtCachePanner::contentsMask() const
{
    return QBitmap();
}

/**
 * \if ENGLISH
 * @brief Grab the widget into a pixmap
 * @return Grabbed pixmap
 * \endif
 * \if CHINESE
 * @brief 将控件捕获到 pixmap 中
 * @return 捕获的 pixmap
 * \endif
 */
QPixmap QwtCachePanner::grab() const
{
#if QT_VERSION >= 0x050000
    return parentWidget()->grab(parentWidget()->rect());
#else
    return QPixmap::grabWidget(parentWidget());
#endif
}

/**
 * \if ENGLISH
 * @brief Event filter
 * @details When isEnabled() is true mouse events of the
 *          observed widget are filtered.
 * @param[in] object Object to be filtered
 * @param[in] event Event
 * @return Always false, beside for paint events for the parent widget
 * @sa widgetMousePressEvent(), widgetMouseReleaseEvent(), widgetMouseMoveEvent()
 * \endif
 * \if CHINESE
 * @brief 事件过滤器
 * @details 当 isEnabled() 为 true 时，观察控件的鼠标事件被过滤。
 * @param[in] object 要过滤的对象
 * @param[in] event 事件
 * @return 总是返回 false，除了父控件的绘制事件
 * @sa widgetMousePressEvent(), widgetMouseReleaseEvent(), widgetMouseMoveEvent()
 * \endif
 */
bool QwtCachePanner::eventFilter(QObject* object, QEvent* event)
{
    if (object == nullptr || object != parentWidget())
        return false;

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
    case QEvent::KeyPress: {
        widgetKeyPressEvent(static_cast< QKeyEvent* >(event));
        break;
    }
    case QEvent::KeyRelease: {
        widgetKeyReleaseEvent(static_cast< QKeyEvent* >(event));
        break;
    }
    case QEvent::Paint: {
        if (isVisible())
            return true;
        break;
    }
    default:;
    }

    return false;
}

/*!
   Handle a mouse press event for the observed widget.

   \param mouseEvent Mouse event
   \sa eventFilter(), widgetMouseReleaseEvent(),
      widgetMouseMoveEvent(),
 */
void QwtCachePanner::widgetMousePressEvent(QMouseEvent* mouseEvent)
{
    if ((mouseEvent->button() != m_data->button) || (mouseEvent->modifiers() != m_data->buttonModifiers)) {
        return;
    }

    QWidget* w = parentWidget();
    if (w == nullptr)
        return;

#ifndef QT_NO_CURSOR
    showCursor(true);
#endif

    m_data->initialPos = m_data->pos = mouseEvent->pos();

    setGeometry(parentWidget()->rect());

    // We don't want to grab the picker !
    QVector< QwtPicker* > pickers = qwtActivePickers(parentWidget());
    for (int i = 0; i < pickers.size(); i++)
        pickers[ i ]->setEnabled(false);

    m_data->pixmap       = grab();
    m_data->contentsMask = contentsMask();

    for (int i = 0; i < pickers.size(); i++)
        pickers[ i ]->setEnabled(true);

    show();
}

/*!
   Handle a mouse move event for the observed widget.

   \param mouseEvent Mouse event
   \sa eventFilter(), widgetMousePressEvent(), widgetMouseReleaseEvent()
 */
void QwtCachePanner::widgetMouseMoveEvent(QMouseEvent* mouseEvent)
{
    if (!isVisible())
        return;

    QPoint pos = mouseEvent->pos();
    if (!isOrientationEnabled(Qt::Horizontal))
        pos.setX(m_data->initialPos.x());
    if (!isOrientationEnabled(Qt::Vertical))
        pos.setY(m_data->initialPos.y());

    if (pos != m_data->pos && rect().contains(pos)) {
        m_data->pos = pos;
        update();

        Q_EMIT moved(m_data->pos.x() - m_data->initialPos.x(), m_data->pos.y() - m_data->initialPos.y());
    }
}

/*!
   Handle a mouse release event for the observed widget.

   \param mouseEvent Mouse event
   \sa eventFilter(), widgetMousePressEvent(),
      widgetMouseMoveEvent(),
 */
void QwtCachePanner::widgetMouseReleaseEvent(QMouseEvent* mouseEvent)
{
    if (isVisible()) {
        hide();
#ifndef QT_NO_CURSOR
        showCursor(false);
#endif

        QPoint pos = mouseEvent->pos();
        if (!isOrientationEnabled(Qt::Horizontal))
            pos.setX(m_data->initialPos.x());
        if (!isOrientationEnabled(Qt::Vertical))
            pos.setY(m_data->initialPos.y());

        m_data->pixmap       = QPixmap();
        m_data->contentsMask = QBitmap();
        m_data->pos          = pos;

        if (m_data->pos != m_data->initialPos) {
            Q_EMIT panned(m_data->pos.x() - m_data->initialPos.x(), m_data->pos.y() - m_data->initialPos.y());
        }
    }
}

/*!
   Handle a key press event for the observed widget.

   \param keyEvent Key event
   \sa eventFilter(), widgetKeyReleaseEvent()
 */
void QwtCachePanner::widgetKeyPressEvent(QKeyEvent* keyEvent)
{
    if ((keyEvent->key() == m_data->abortKey) && (keyEvent->modifiers() == m_data->abortKeyModifiers)) {
        hide();

#ifndef QT_NO_CURSOR
        showCursor(false);
#endif
        m_data->pixmap = QPixmap();
    }
}

/*!
   Handle a key release event for the observed widget.

   \param keyEvent Key event
   \sa eventFilter(), widgetKeyReleaseEvent()
 */
void QwtCachePanner::widgetKeyReleaseEvent(QKeyEvent* keyEvent)
{
    Q_UNUSED(keyEvent);
}

#ifndef QT_NO_CURSOR
void QwtCachePanner::showCursor(bool on)
{
    if (on == m_data->hasCursor)
        return;

    QWidget* w = parentWidget();
    if (w == nullptr || m_data->cursor == nullptr)
        return;

    m_data->hasCursor = on;

    if (on) {
        if (w->testAttribute(Qt::WA_SetCursor)) {
            delete m_data->restoreCursor;
            m_data->restoreCursor = new QCursor(w->cursor());
        }
        w->setCursor(*m_data->cursor);
    } else {
        if (m_data->restoreCursor) {
            w->setCursor(*m_data->restoreCursor);
            delete m_data->restoreCursor;
            m_data->restoreCursor = nullptr;
        } else
            w->unsetCursor();
    }
}
#endif
