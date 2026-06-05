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

#include "qwt_event_pattern.h"
#include <qevent.h>

/**
 * @brief Constructor
 * @details Initializes mouse and key patterns with default settings.
 * @sa MousePatternCode, KeyPatternCode
 */
QwtEventPattern::QwtEventPattern()
    : m_mousePattern( MousePatternCount )
    , m_keyPattern( KeyPatternCount )
{
    initKeyPattern();
    initMousePattern( 3 );
}

/**
 * @brief Destructor
 */
QwtEventPattern::~QwtEventPattern()
{
}

/**
 * @brief Set default mouse patterns depending on the number of mouse buttons
 * @param[in] numButtons Number of mouse buttons (<= 3)
 * @sa MousePatternCode
 */
void QwtEventPattern::initMousePattern( int numButtons )
{
    m_mousePattern.resize( MousePatternCount );

    switch ( numButtons )
    {
        case 1:
        {
            setMousePattern( MouseSelect1, Qt::LeftButton );
            setMousePattern( MouseSelect2, Qt::LeftButton, Qt::ControlModifier );
            setMousePattern( MouseSelect3, Qt::LeftButton, Qt::AltModifier );
            break;
        }
        case 2:
        {
            setMousePattern( MouseSelect1, Qt::LeftButton );
            setMousePattern( MouseSelect2, Qt::RightButton );
            setMousePattern( MouseSelect3, Qt::LeftButton, Qt::AltModifier );
            break;
        }
        default:
        {
            setMousePattern( MouseSelect1, Qt::LeftButton );
            setMousePattern( MouseSelect2, Qt::RightButton );
            setMousePattern( MouseSelect3, Qt::MiddleButton );
        }
    }

    setMousePattern( MouseSelect4, m_mousePattern[MouseSelect1].button,
        m_mousePattern[MouseSelect1].modifiers | Qt::ShiftModifier );

    setMousePattern( MouseSelect5, m_mousePattern[MouseSelect2].button,
        m_mousePattern[MouseSelect2].modifiers | Qt::ShiftModifier );

    setMousePattern( MouseSelect6, m_mousePattern[MouseSelect3].button,
        m_mousePattern[MouseSelect3].modifiers | Qt::ShiftModifier );
}

/**
 * @brief Set default key patterns
 * @sa KeyPatternCode
 */
void QwtEventPattern::initKeyPattern()
{
    m_keyPattern.resize( KeyPatternCount );

    setKeyPattern( KeySelect1, Qt::Key_Return );
    setKeyPattern( KeySelect2, Qt::Key_Space );
    setKeyPattern( KeyAbort, Qt::Key_Escape );

    setKeyPattern( KeyLeft, Qt::Key_Left );
    setKeyPattern( KeyRight, Qt::Key_Right );
    setKeyPattern( KeyUp, Qt::Key_Up );
    setKeyPattern( KeyDown, Qt::Key_Down );

    setKeyPattern( KeyRedo, Qt::Key_Plus );
    setKeyPattern( KeyUndo, Qt::Key_Minus );
    setKeyPattern( KeyHome, Qt::Key_Escape );
}

/**
 * @brief Change one mouse pattern
 * @param[in] pattern Index of the pattern
 * @param[in] button Mouse button
 * @param[in] modifiers Keyboard modifiers
 * @sa QMouseEvent
 */
void QwtEventPattern::setMousePattern( MousePatternCode pattern,
    Qt::MouseButton button, Qt::KeyboardModifiers modifiers )
{
    if ( pattern >= 0 && pattern < MousePatternCount )
    {
        m_mousePattern[ pattern ].button = button;
        m_mousePattern[ pattern ].modifiers = modifiers;
    }
}

/**
 * @brief Change one key pattern
 * @param[in] pattern Index of the pattern
 * @param[in] key Key code
 * @param[in] modifiers Keyboard modifiers
 * @sa QKeyEvent
 */
void QwtEventPattern::setKeyPattern( KeyPatternCode pattern,
    int key, Qt::KeyboardModifiers modifiers )
{
    if ( pattern >= 0 && pattern < KeyPatternCount )
    {
        m_keyPattern[ pattern ].key = key;
        m_keyPattern[ pattern ].modifiers = modifiers;
    }
}

/**
 * @brief Change the mouse event patterns
 * @param[in] pattern Vector of mouse patterns
 */
void QwtEventPattern::setMousePattern( const QVector< MousePattern >& pattern )
{
    m_mousePattern = pattern;
}

/**
 * @brief Change the key event patterns
 * @param[in] pattern Vector of key patterns
 */
void QwtEventPattern::setKeyPattern( const QVector< KeyPattern >& pattern )
{
    m_keyPattern = pattern;
}

/**
 * @brief Return the mouse pattern vector
 * @return Mouse pattern vector (const)
 */
const QVector< QwtEventPattern::MousePattern >&
QwtEventPattern::mousePattern() const
{
    return m_mousePattern;
}

/**
 * @brief Return the key pattern vector
 * @return Key pattern vector (const)
 */
const QVector< QwtEventPattern::KeyPattern >&
QwtEventPattern::keyPattern() const
{
    return m_keyPattern;
}

/**
 * @brief Return the mouse pattern vector
 * @return Mouse pattern vector (mutable)
 */
QVector< QwtEventPattern::MousePattern >& QwtEventPattern::mousePattern()
{
    return m_mousePattern;
}

/**
 * @brief Return the key pattern vector
 * @return Key pattern vector (mutable)
 */
QVector< QwtEventPattern::KeyPattern >& QwtEventPattern::keyPattern()
{
    return m_keyPattern;
}

/**
 * @brief Compare a mouse event with an event pattern
 * @details A mouse event matches the pattern when both have the same button
 *          value and in the state value the same key flags(Qt::KeyButtonMask)
 *          are set.
 * @param[in] code Index of the event pattern
 * @param[in] event Mouse event
 * @return true if matches, false otherwise
 * @sa keyMatch()
 */
bool QwtEventPattern::mouseMatch( MousePatternCode code,
    const QMouseEvent* event ) const
{
    if ( code >= 0 && code < MousePatternCount )
        return mouseMatch( m_mousePattern[ code ], event );

    return false;
}

/*!
   \brief Compare a mouse event with an event pattern.

   A mouse event matches the pattern when both have the same button
   value and in the state value the same key flags(Qt::KeyButtonMask)
   are set.

   \param pattern Mouse event pattern
   \param event Mouse event
   \return true if matches

   \sa keyMatch()
 */

bool QwtEventPattern::mouseMatch( const MousePattern& pattern,
    const QMouseEvent* event ) const
{
    if ( event == nullptr )
        return false;

    const MousePattern mousePattern( event->button(), event->modifiers() );
    return mousePattern == pattern;
}

/**
 * @brief Compare a key event with an event pattern
 * @details A key event matches the pattern when both have the same key
 *          value and in the state value the same key flags (Qt::KeyButtonMask)
 *          are set.
 * @param[in] code Index of the event pattern
 * @param[in] event Key event
 * @return true if matches, false otherwise
 * @sa mouseMatch()
 */
bool QwtEventPattern::keyMatch( KeyPatternCode code,
    const QKeyEvent* event ) const
{
    if ( code >= 0 && code < KeyPatternCount )
        return keyMatch( m_keyPattern[ code ], event );

    return false;
}

/*!
   \brief Compare a key event with an event pattern.

   A key event matches the pattern when both have the same key
   value and in the state value the same key flags (Qt::KeyButtonMask)
   are set.

   \param pattern Key event pattern
   \param event Key event
   \return true if matches

   \sa mouseMatch()
 */

bool QwtEventPattern::keyMatch(
    const KeyPattern& pattern, const QKeyEvent* event ) const
{
    if ( event == nullptr )
        return false;

    const KeyPattern keyPattern( event->key(), event->modifiers() );
    return keyPattern == pattern;
}
