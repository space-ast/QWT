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
 *        - QwtPanner -> QwtCachePanner (pixmap-cache version)
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
    QWT_DECLARE_PUBLIC(QwtCachePanner)
public:
    PrivateData(QwtCachePanner* p)
        : q_ptr(p)
        , button(Qt::LeftButton)
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
 * @brief Creates a panner that is enabled for the left mouse button
 * @param[in] parent Parent widget to be panned
 */
QwtCachePanner::QwtCachePanner(QWidget* parent) : QWidget(parent), QWT_PIMPL_CONSTRUCT
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::NoFocus);
    hide();

    setEnabled(true);
}

/**
 * @brief Destructor
 */
QwtCachePanner::~QwtCachePanner()
{
}

/**
 * @brief Change the mouse button and modifiers used for panning
 * @details The defaults are Qt::LeftButton and Qt::NoModifier
 * @param[in] button Mouse button for panning
 * @param[in] modifiers Keyboard modifiers
 */
void QwtCachePanner::setMouseButton(Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
    QWT_D(d);
    d->button          = button;
    d->buttonModifiers = modifiers;
}

/**
 * @brief Get mouse button and modifiers used for panning
 * @param[out] button Mouse button used for panning
 * @param[out] modifiers Keyboard modifiers used for panning
 */
void QwtCachePanner::getMouseButton(Qt::MouseButton& button, Qt::KeyboardModifiers& modifiers) const
{
    QWT_DC(d);
    button    = d->button;
    modifiers = d->buttonModifiers;
}

/**
 * @brief Change the abort key
 * @details The defaults are Qt::Key_Escape and Qt::NoModifiers
 * @param[in] key Key code (See Qt::Keycode)
 * @param[in] modifiers Keyboard modifiers
 */
void QwtCachePanner::setAbortKey(int key, Qt::KeyboardModifiers modifiers)
{
    QWT_D(d);
    d->abortKey          = key;
    d->abortKeyModifiers = modifiers;
}

/**
 * @brief Get the abort key and modifiers
 * @param[out] key Key code used for abort
 * @param[out] modifiers Keyboard modifiers used for abort
 */
void QwtCachePanner::getAbortKey(int& key, Qt::KeyboardModifiers& modifiers) const
{
    QWT_DC(d);
    key       = d->abortKey;
    modifiers = d->abortKeyModifiers;
}

/**
 * @brief Change the cursor that is active while panning
 * @details The default is the cursor of the parent widget.
 * @param[in] cursor New cursor
 * @sa setCursor()
 */
#ifndef QT_NO_CURSOR
void QwtCachePanner::setCursor(const QCursor& cursor)
{
    QWT_D(d);
    d->cursor = new QCursor(cursor);
}
#endif

/**
 * @brief Return the cursor that is active while panning
 * @return Cursor active while panning
 * @sa setCursor()
 */
#ifndef QT_NO_CURSOR
const QCursor QwtCachePanner::cursor() const
{
    QWT_DC(d);
    if (d->cursor)
        return *d->cursor;

    if (parentWidget())
        return parentWidget()->cursor();

    return QCursor();
}
#endif

/**
 * @brief En/disable the panner
 * @details When enabled is true an event filter is installed for
 *          the observed widget, otherwise the event filter is removed.
 * @param[in] on true or false
 * @sa isEnabled(), eventFilter()
 */
void QwtCachePanner::setEnabled(bool on)
{
    QWT_D(d);
    if (d->isEnabled != on) {
        d->isEnabled = on;

        QWidget* w = parentWidget();
        if (w) {
            if (d->isEnabled) {
                w->installEventFilter(this);
            } else {
                w->removeEventFilter(this);
                hide();
            }
        }
    }
}

/**
 * @brief Set the orientations where panning is enabled
 * @details The default value is in both directions: Qt::Horizontal | Qt::Vertical
 * @param[in] o Orientation flags
 */
void QwtCachePanner::setOrientations(Qt::Orientations o)
{
    QWT_D(d);
    d->orientations = o;
}

/**
 * @brief Return the orientation where panning is enabled
 * @return Orientation flags
 */
Qt::Orientations QwtCachePanner::orientations() const
{
    QWT_DC(d);
    return d->orientations;
}

/**
 * @brief Check if an orientation is enabled
 * @param[in] o Orientation to check
 * @return True if the orientation is enabled
 * @sa orientations(), setOrientations()
 */
bool QwtCachePanner::isOrientationEnabled(Qt::Orientation o) const
{
    QWT_DC(d);
    return d->orientations & o;
}

/**
 * @brief Return whether the panner is enabled
 * @return true when enabled, false otherwise
 * @sa setEnabled(), eventFilter()
 */
bool QwtCachePanner::isEnabled() const
{
    QWT_DC(d);
    return d->isEnabled;
}

/**
 * @brief Paint event
 * @details Repaint the grabbed pixmap on its current position and
 *          fill the empty spaces by the background of the parent widget.
 * @param[in] event Paint event
 */
void QwtCachePanner::paintEvent(QPaintEvent* event)
{
    QWT_D(d);
    int dx = d->pos.x() - d->initialPos.x();
    int dy = d->pos.y() - d->initialPos.y();

    QRectF r;
    r.setSize(d->pixmap.size() / QwtPainter::devicePixelRatio(&d->pixmap));
    r.moveCenter(QPointF(r.center().x() + dx, r.center().y() + dy));

    QPixmap pm = QwtPainter::backingStore(this, size());
    QwtPainter::fillPixmap(parentWidget(), pm);

    QPainter painter(&pm);

    if (!d->contentsMask.isNull()) {
        QPixmap masked = d->pixmap;
        masked.setMask(d->contentsMask);
        painter.drawPixmap(r.toRect(), masked);
    } else {
        painter.drawPixmap(r.toRect(), d->pixmap);
    }

    painter.end();

    if (!d->contentsMask.isNull())
        pm.setMask(d->contentsMask);

    painter.begin(this);
    painter.setClipRegion(event->region());
    painter.drawPixmap(0, 0, pm);
}

/**
 * @brief Calculate a mask for the contents of the panned widget
 * @details Sometimes only parts of the contents of a widget should be
 *          panned. F.e. for a widget with a styled background with rounded borders
 *          only the area inside of the border should be panned.
 * @return An empty bitmap, indicating no mask
 */
QBitmap QwtCachePanner::contentsMask() const
{
    return QBitmap();
}

/**
 * @brief Grab the widget into a pixmap
 * @return Grabbed pixmap
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
 * @brief Event filter
 * @details When isEnabled() is true mouse events of the
 *          observed widget are filtered.
 * @param[in] object Object to be filtered
 * @param[in] event Event
 * @return Always false, beside for paint events for the parent widget
 * @sa widgetMousePressEvent(), widgetMouseReleaseEvent(), widgetMouseMoveEvent()
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

   @param mouseEvent Mouse event
   @sa eventFilter(), widgetMouseReleaseEvent(),
      widgetMouseMoveEvent(),
 */
void QwtCachePanner::widgetMousePressEvent(QMouseEvent* mouseEvent)
{
    QWT_D(d);
    if ((mouseEvent->button() != d->button) || (mouseEvent->modifiers() != d->buttonModifiers)) {
        return;
    }

    QWidget* w = parentWidget();
    if (w == nullptr)
        return;

#ifndef QT_NO_CURSOR
    showCursor(true);
#endif

    d->initialPos = d->pos = mouseEvent->pos();

    setGeometry(parentWidget()->rect());

    // We don't want to grab the picker !
    QVector< QwtPicker* > pickers = qwtActivePickers(parentWidget());
    for (int i = 0; i < pickers.size(); i++)
        pickers[ i ]->setEnabled(false);

    d->pixmap       = grab();
    d->contentsMask = contentsMask();

    for (int i = 0; i < pickers.size(); i++)
        pickers[ i ]->setEnabled(true);

    show();
}

/*!
   Handle a mouse move event for the observed widget.

   @param mouseEvent Mouse event
   @sa eventFilter(), widgetMousePressEvent(), widgetMouseReleaseEvent()
 */
void QwtCachePanner::widgetMouseMoveEvent(QMouseEvent* mouseEvent)
{
    QWT_D(d);
    if (!isVisible())
        return;

    QPoint pos = mouseEvent->pos();
    if (!isOrientationEnabled(Qt::Horizontal))
        pos.setX(d->initialPos.x());
    if (!isOrientationEnabled(Qt::Vertical))
        pos.setY(d->initialPos.y());

    if (pos != d->pos && rect().contains(pos)) {
        d->pos = pos;
        update();

        Q_EMIT moved(d->pos.x() - d->initialPos.x(), d->pos.y() - d->initialPos.y());
    }
}

/*!
   Handle a mouse release event for the observed widget.

   @param mouseEvent Mouse event
   @sa eventFilter(), widgetMousePressEvent(),
      widgetMouseMoveEvent(),
 */
void QwtCachePanner::widgetMouseReleaseEvent(QMouseEvent* mouseEvent)
{
    QWT_D(d);
    if (isVisible()) {
        hide();
#ifndef QT_NO_CURSOR
        showCursor(false);
#endif

        QPoint pos = mouseEvent->pos();
        if (!isOrientationEnabled(Qt::Horizontal))
            pos.setX(d->initialPos.x());
        if (!isOrientationEnabled(Qt::Vertical))
            pos.setY(d->initialPos.y());

        d->pixmap       = QPixmap();
        d->contentsMask = QBitmap();
        d->pos          = pos;

        if (d->pos != d->initialPos) {
            Q_EMIT panned(d->pos.x() - d->initialPos.x(), d->pos.y() - d->initialPos.y());
        }
    }
}

/*!
   Handle a key press event for the observed widget.

   @param keyEvent Key event
   @sa eventFilter(), widgetKeyReleaseEvent()
 */
void QwtCachePanner::widgetKeyPressEvent(QKeyEvent* keyEvent)
{
    QWT_D(d);
    if ((keyEvent->key() == d->abortKey) && (keyEvent->modifiers() == d->abortKeyModifiers)) {
        hide();

#ifndef QT_NO_CURSOR
        showCursor(false);
#endif
        d->pixmap = QPixmap();
    }
}

/*!
   Handle a key release event for the observed widget.

   @param keyEvent Key event
   @sa eventFilter(), widgetKeyReleaseEvent()
 */
void QwtCachePanner::widgetKeyReleaseEvent(QKeyEvent* keyEvent)
{
    Q_UNUSED(keyEvent);
}

#ifndef QT_NO_CURSOR
void QwtCachePanner::showCursor(bool on)
{
    QWT_D(d);
    if (on == d->hasCursor)
        return;

    QWidget* w = parentWidget();
    if (w == nullptr || d->cursor == nullptr)
        return;

    d->hasCursor = on;

    if (on) {
        if (w->testAttribute(Qt::WA_SetCursor)) {
            delete d->restoreCursor;
            d->restoreCursor = new QCursor(w->cursor());
        }
        w->setCursor(*d->cursor);
    } else {
        if (d->restoreCursor) {
            w->setCursor(*d->restoreCursor);
            delete d->restoreCursor;
            d->restoreCursor = nullptr;
        } else
            w->unsetCursor();
    }
}
#endif
