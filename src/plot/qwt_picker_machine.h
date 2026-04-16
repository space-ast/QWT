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

#ifndef QWT_PICKER_MACHINE
#define QWT_PICKER_MACHINE

#include "qwt_global.h"

class QwtEventPattern;
class QEvent;
template< typename T > class QList;

/**
 * \if ENGLISH
 * @brief A state machine for QwtPicker selections
 * @details QwtPickerMachine accepts key and mouse events and translates them
 *          into selection commands.
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 * \if CHINESE
 * @brief QwtPicker 选择的状态机
 * @details QwtPickerMachine 接收键盘和鼠标事件，并将其转换为选择命令。
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 */

class QWT_EXPORT QwtPickerMachine
{
  public:
    /*!
       Type of a selection.
       \sa selectionType()
     */
    enum SelectionType
    {
        //! The state machine not usable for any type of selection.
        NoSelection = -1,

        //! The state machine is for selecting a single point.
        PointSelection,

        //! The state machine is for selecting a rectangle (2 points).
        RectSelection,

        //! The state machine is for selecting a polygon (many points).
        PolygonSelection
    };

    //! Commands - the output of a state machine
    enum Command
    {
        Begin,
        Append,
        Move,
        Remove,
        End
    };

    // Constructor with selection type
    explicit QwtPickerMachine( SelectionType );
    // Destructor
    virtual ~QwtPickerMachine();

    // Transition function that processes events and returns commands
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) = 0;
    // Reset the state machine to initial state
    void reset();

    // Return the current state
    int state() const;
    // Set the current state
    void setState( int );

    // Return the selection type
    SelectionType selectionType() const;

  private:
    const SelectionType m_selectionType;
    int m_state;
};

/**
 * \if ENGLISH
 * @brief A state machine for indicating mouse movements
 * @details QwtPickerTrackerMachine supports displaying information
 *          corresponding to mouse movements, but is not intended for
 *          selecting anything. Begin/End are related to Enter/Leave events.
 * \endif
 * \if CHINESE
 * @brief 用于指示鼠标移动的状态机
 * @details QwtPickerTrackerMachine 支持显示与鼠标移动相关的信息，
 *          但不用于选择任何内容。Begin/End 与 Enter/Leave 事件相关。
 * \endif
 */
class QWT_EXPORT QwtPickerTrackerMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerTrackerMachine();

    // Transition function for tracking mouse movements
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

/**
 * \if ENGLISH
 * @brief A state machine for point selections
 * @details Pressing QwtEventPattern::MouseSelect1 or
 *          QwtEventPattern::KeySelect1 selects a point.
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 * \if CHINESE
 * @brief 用于点选择的状态机
 * @details 按下 QwtEventPattern::MouseSelect1 或 QwtEventPattern::KeySelect1
 *          选择一个点。
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 */
class QWT_EXPORT QwtPickerClickPointMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerClickPointMachine();

    // Transition function for click point selection
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

/**
 * \if ENGLISH
 * @brief A state machine for point selections
 * @details Pressing QwtEventPattern::MouseSelect1 or QwtEventPattern::KeySelect1
 *          starts the selection, releasing QwtEventPattern::MouseSelect1 or
 *          a second press of QwtEventPattern::KeySelect1 terminates it.
 * \endif
 * \if CHINESE
 * @brief 用于点选择的状态机
 * @details 按下 QwtEventPattern::MouseSelect1 或 QwtEventPattern::KeySelect1
 *          开始选择，释放 QwtEventPattern::MouseSelect1 或再次按下
 *          QwtEventPattern::KeySelect1 结束选择。
 * \endif
 */
class QWT_EXPORT QwtPickerDragPointMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerDragPointMachine();

    // Transition function for drag point selection
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

/**
 * \if ENGLISH
 * @brief A state machine for rectangle selections
 * @details Pressing QwtEventPattern::MouseSelect1 starts the selection,
 *          releasing it selects the first point. Pressing it again selects
 *          the second point and terminates the selection.
 *          Pressing QwtEventPattern::KeySelect1 also starts the selection,
 *          a second press selects the first point. A third one selects
 *          the second point and terminates the selection.
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 * \if CHINESE
 * @brief 用于矩形选择的状态机
 * @details 按下 QwtEventPattern::MouseSelect1 开始选择，释放它选择第一个点。
 *          再次按下选择第二个点并结束选择。按下 QwtEventPattern::KeySelect1
 *          也开始选择，第二次按下选择第一个点，第三次按下选择第二个点并结束选择。
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 */

class QWT_EXPORT QwtPickerClickRectMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerClickRectMachine();

    // Transition function for click rectangle selection
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

/**
 * \if ENGLISH
 * @brief A state machine for rectangle selections
 * @details Pressing QwtEventPattern::MouseSelect1 selects the first point,
 *          releasing it the second point.
 *          Pressing QwtEventPattern::KeySelect1 also selects the first point,
 *          a second press selects the second point and terminates the selection.
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 * \if CHINESE
 * @brief 用于矩形选择的状态机
 * @details 按下 QwtEventPattern::MouseSelect1 选择第一个点，释放它选择第二个点。
 *          按下 QwtEventPattern::KeySelect1 也选择第一个点，第二次按下选择第二个点并结束选择。
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 */

class QWT_EXPORT QwtPickerDragRectMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerDragRectMachine();

    // Transition function for drag rectangle selection
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

/**
 * \if ENGLISH
 * @brief A state machine for line selections
 * @details Pressing QwtEventPattern::MouseSelect1 selects the first point,
 *          releasing it the second point.
 *          Pressing QwtEventPattern::KeySelect1 also selects the first point,
 *          a second press selects the second point and terminates the selection.
 *          A common use case of QwtPickerDragLineMachine are pickers for
 *          distance measurements.
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 * \if CHINESE
 * @brief 用于线段选择的状态机
 * @details 按下 QwtEventPattern::MouseSelect1 选择第一个点，释放它选择第二个点。
 *          按下 QwtEventPattern::KeySelect1 也选择第一个点，第二次按下选择第二个点并结束选择。
 *          QwtPickerDragLineMachine 的常见用途是距离测量拾取器。
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 */

class QWT_EXPORT QwtPickerDragLineMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerDragLineMachine();

    // Transition function for drag line selection
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

/**
 * \if ENGLISH
 * @brief A state machine for polygon selections
 * @details Pressing QwtEventPattern::MouseSelect1 or QwtEventPattern::KeySelect1
 *          starts the selection and selects the first point, or appends a point.
 *          Pressing QwtEventPattern::MouseSelect2 or QwtEventPattern::KeySelect2
 *          appends the last point and terminates the selection.
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 * \if CHINESE
 * @brief 用于多边形选择的状态机
 * @details 按下 QwtEventPattern::MouseSelect1 或 QwtEventPattern::KeySelect1
 *          开始选择并选择第一个点，或追加一个点。
 *          按下 QwtEventPattern::MouseSelect2 或 QwtEventPattern::KeySelect2
 *          追加最后一个点并结束选择。
 * @sa QwtEventPattern::MousePatternCode, QwtEventPattern::KeyPatternCode
 * \endif
 */

class QWT_EXPORT QwtPickerPolygonMachine : public QwtPickerMachine
{
  public:
    // Constructor
    QwtPickerPolygonMachine();

    // Transition function for polygon selection
    virtual QList< Command > transition(
        const QwtEventPattern&, const QEvent* ) override;
};

#endif
