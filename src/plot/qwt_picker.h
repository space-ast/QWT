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

#ifndef QWT_PICKER
#define QWT_PICKER

#include "qwt_global.h"
#include "qwt_event_pattern.h"

#include <qobject.h>
#include <QPen>
#include <QFont>
#include <QPainterPath>
#include <QPoint>
#include <QPolygon>
#include <QSize>
#include <QRect>
#include <QRegion>
class QwtPickerMachine;
class QwtWidgetOverlay;
class QwtText;
class QWidget;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QPainter;

/**
 * \if ENGLISH
 * @brief QwtPicker provides selections on a widget
 * @details QwtPicker filters all enter, leave, mouse and keyboard events of a widget
 *          and translates them into an array of selected points.
 *
 *          The way how the points are collected depends on type of state machine
 *          that is connected to the picker. Qwt offers a couple of predefined
 *          state machines for selecting:
 *
 *          - Nothing: QwtPickerTrackerMachine
 *          - Single points: QwtPickerClickPointMachine, QwtPickerDragPointMachine
 *          - Rectangles: QwtPickerClickRectMachine, QwtPickerDragRectMachine
 *          - Polygons: QwtPickerPolygonMachine
 *
 *          While these state machines cover the most common ways to collect points
 *          it is also possible to implement individual machines as well.
 *
 *          QwtPicker translates the picked points into a selection using the
 *          adjustedPoints() method. adjustedPoints() is intended to be reimplemented
 *          to fix up the selection according to application specific requirements.
 *          (F.e. when an application accepts rectangles of a fixed aspect ratio only.)
 *
 *          Optionally QwtPicker support the process of collecting points by a
 *          rubber band and tracker displaying a text for the current mouse
 *          position.
 *
 *          The state machine triggers the following commands:
 *          - begin(): Activate/Initialize the selection.
 *          - append(): Add a new point.
 *          - move(): Change the position of the last point.
 *          - remove(): Remove the last point.
 *          - end(): Terminate the selection and call accept to validate the picked points.
 *
 *          The picker is active (isActive()), between begin() and end().
 *          In active state the rubber band is displayed, and the tracker is visible
 *          in case of trackerMode is ActiveOnly or AlwaysOn.
 *
 *          The cursor can be moved using the arrow keys. All selections can be aborted
 *          using the abort key. (QwtEventPattern::KeyPatternCode)
 *
 * @warning In case of QWidget::NoFocus the focus policy of the observed
 *          widget is set to QWidget::WheelFocus and mouse tracking
 *          will be manipulated while the picker is active,
 *          or if trackerMode() is AlwaysOn.
 *
 * @par Example
 * @code
 * #include <qwt_picker.h>
 * #include <qwt_picker_machine.h>
 *
 * QwtPicker *picker = new QwtPicker(widget);
 * picker->setStateMachine(new QwtPickerDragRectMachine);
 * picker->setTrackerMode(QwtPicker::ActiveOnly);
 * picker->setRubberBand(QwtPicker::RectRubberBand);
 * @endcode
 * \endif
 * \if CHINESE
 * @brief QwtPicker 在一个部件上提供选择功能
 * @details QwtPicker 会过滤一个部件的所有进入、离开、鼠标和键盘事件，
 *          并将它们转换为一个选定坐标点的数组。
 *
 *          收集点的方式取决于连接到选择器的状态机类型。Qwt 提供了几个预定义的选择状态机：
 *
 *          - 无：QwtPickerTrackerMachine
 *          - 单个点：QwtPickerClickPointMachine, QwtPickerDragPointMachine
 *          - 矩形：QwtPickerClickRectMachine, QwtPickerDragRectMachine
 *          - 多边形：QwtPickerPolygonMachine
 *
 *          虽然这些状态机涵盖了最常见的点收集方式，但也可以实现自定义的状态机。
 *
 *          QwtPicker 使用 adjustedPoints() 方法将拾取的点转换为一个选择区域/集合。
 *          adjustedPoints() 方法旨在被重写，以便根据应用程序的特定要求修正选择结果。
 *          （例如：当应用程序只接受固定宽高比的矩形时。）
 *
 *          QwtPicker 可以选择性地通过一个橡皮筋和一个显示当前鼠标位置文本的追踪器来辅助点的收集过程。
 *
 *          状态机会触发以下命令：
 *          - begin()：激活/初始化选择。
 *          - append()：添加一个新点。
 *          - move()：改变最后一个点的位置。
 *          - remove()：移除最后一个点。
 *          - end()：终止选择，并调用 accept 来验证拾取的点。
 *
 *          在 begin() 和 end() 之间，选择器处于活动状态（isActive()）。
 *          在活动状态下，橡皮筋会显示。如果追踪器模式是 ActiveOnly 或 AlwaysOn，追踪器也会可见。
 *
 *          可以使用方向键移动光标。所有选择都可以使用取消键来中止。(QwtEventPattern::KeyPatternCode)
 *
 * @warning 如果观察的部件的焦点策略是 QWidget::NoFocus，
 *          那么在选择器处于活动状态时，或者如果 trackerMode() 是 AlwaysOn 时，
 *          该部件的焦点策略会被设置为 QWidget::WheelFocus，并且鼠标追踪会被操控。
 *
 * @par 示例
 * @code
 * #include <qwt_picker.h>
 * #include <qwt_picker_machine.h>
 *
 * QwtPicker *picker = new QwtPicker(widget);
 * picker->setStateMachine(new QwtPickerDragRectMachine);
 * picker->setTrackerMode(QwtPicker::ActiveOnly);
 * picker->setRubberBand(QwtPicker::RectRubberBand);
 * @endcode
 * \endif
 */
class QWT_EXPORT QwtPicker : public QObject, public QwtEventPattern
{
    Q_OBJECT

    Q_ENUMS(RubberBand DisplayMode ResizeMode)

    Q_PROPERTY(bool isEnabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)

    Q_PROPERTY(DisplayMode trackerMode READ trackerMode WRITE setTrackerMode)
    Q_PROPERTY(QPen trackerPen READ trackerPen WRITE setTrackerPen)
    Q_PROPERTY(QFont trackerFont READ trackerFont WRITE setTrackerFont)

    Q_PROPERTY(RubberBand rubberBand READ rubberBand WRITE setRubberBand)
    Q_PROPERTY(QPen rubberBandPen READ rubberBandPen WRITE setRubberBandPen)
    QWT_DECLARE_PRIVATE(QwtPicker)
public:
/**
     * \if ENGLISH
     * @brief Rubber band style
     * @details The default value is QwtPicker::NoRubberBand.
     * \sa setRubberBand(), rubberBand()
     * \endif
     * \if CHINESE
     * @brief 橡皮筋样式
     * @details 默认值为 QwtPicker::NoRubberBand。
     * \sa setRubberBand(), rubberBand()
     * \endif
     */
    enum RubberBand
    {
        //! No rubberband.
        NoRubberBand = 0,

        //! A horizontal line ( only for QwtPickerMachine::PointSelection )
        HLineRubberBand,

        //! A vertical line ( only for QwtPickerMachine::PointSelection )
        VLineRubberBand,

        //! A crosshair ( only for QwtPickerMachine::PointSelection )
        CrossRubberBand,

        //! A rectangle ( only for QwtPickerMachine::RectSelection )
        RectRubberBand,

        //! An ellipse ( only for QwtPickerMachine::RectSelection )
        EllipseRubberBand,

        //! A polygon ( only for QwtPickerMachine::PolygonSelection )
        PolygonRubberBand,

        /**
         * \if ENGLISH
         * @brief Values >= UserRubberBand can be used to define additional rubber bands
         * \endif
         * \if CHINESE
         * @brief 值 >= UserRubberBand 可用于定义额外的橡皮筋
         * \endif
         */
        UserRubberBand = 100
    };

/**
     * \if ENGLISH
     * @brief Display mode
     * \sa setTrackerMode(), trackerMode(), isActive()
     * \endif
     * \if CHINESE
     * @brief 显示模式
     * \sa setTrackerMode(), trackerMode(), isActive()
     * \endif
     */
    enum DisplayMode
    {
        //! Display never
        AlwaysOff,

        //! Display always
        AlwaysOn,

        //! Display only when the selection is active
        ActiveOnly
    };

/**
     * \if ENGLISH
     * @brief Controls what to do with the selected points when the observed widget is resized
     * @details The default value is QwtPicker::Stretch.
     * \sa setResizeMode()
     * \endif
     * \if CHINESE
     * @brief 控制当观察部件调整大小时对选定点的处理方式
     * @details 默认值为 QwtPicker::Stretch。
     * \sa setResizeMode()
     * \endif
     */
    enum ResizeMode
    {
        //! All points are scaled according to the new size,
        Stretch,

        //! All points remain unchanged.
        KeepSize
    };

    // Constructor
    explicit QwtPicker(QWidget* parent);
    // Constructor with rubber band and tracker mode
    explicit QwtPicker(RubberBand rubberBand, DisplayMode trackerMode, QWidget*);

    // Destructor
    virtual ~QwtPicker();

    // Set the state machine
    void setStateMachine(QwtPickerMachine*);
    // Return the state machine (const)
    const QwtPickerMachine* stateMachine() const;
    // Return the state machine
    QwtPickerMachine* stateMachine();

    // Set the rubber band style
    void setRubberBand(RubberBand);
    // Return the rubber band style
    RubberBand rubberBand() const;

    // Set the tracker display mode
    void setTrackerMode(DisplayMode);
    // Return the tracker display mode
    DisplayMode trackerMode() const;

    // Set the resize mode
    void setResizeMode(ResizeMode);
    // Return the resize mode
    ResizeMode resizeMode() const;

    // Set the rubber band pen
    void setRubberBandPen(const QPen&);
    // Return the rubber band pen
    QPen rubberBandPen() const;

    // Set the tracker pen
    void setTrackerPen(const QPen&);
    // Return the tracker pen
    QPen trackerPen() const;

    // Set the tracker font
    void setTrackerFont(const QFont&);
    // Return the tracker font
    QFont trackerFont() const;

    // Return true when enabled
    bool isEnabled() const;
    // Return true if the selection is active
    bool isActive() const;

    // Event filter for handling events
    virtual bool eventFilter(QObject*, QEvent*) override;

    // Return the parent widget
    QWidget* parentWidget();
    // Return the parent widget (const)
    const QWidget* parentWidget() const;

    // Return the pick area
    virtual QPainterPath pickArea() const;

    // Draw the rubber band
    virtual void drawRubberBand(QPainter*) const;
    // Draw the tracker
    virtual void drawTracker(QPainter*) const;

    // Return the tracker mask
    virtual QRegion trackerMask() const;
    // Return the rubber band mask
    virtual QRegion rubberBandMask() const;

    // Return the tracker text for a position
    virtual QwtText trackerText(const QPoint& pos) const;
    // Return the tracker rectangle
    virtual QRect trackerRect(const QFont&) const;
    // Set the tracker position manually (for displaying picker without mouse)
    virtual void setTrackerPosition(const QPoint& pos);
    // Return the tracker position
    QPoint trackerPosition() const;
    // Return the selected points
    QPolygon selection() const;
    // Update the display
    void update();
    // Set the picker active manually
    void setActive(bool on);
public Q_SLOTS:
    // Enable or disable the picker
    void setEnabled(bool);

Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Signal indicating when the picker has been activated
     * @details Together with setEnabled() it can be used to implement
     *          selections with more than one picker.
     * @param on True, when the picker has been activated
     * \endif
     * \if CHINESE
     * @brief 当选择器被激活时发出的信号
     * @details 与 setEnabled() 配合使用可以实现多个选择器的选择。
     * @param on 当选择器被激活时为 true
     * \endif
     */
    void activated(bool on);

    /**
     * \if ENGLISH
     * @brief Signal emitting the selected points at the end of a selection
     * @param polygon Selected points
     * \endif
     * \if CHINESE
     * @brief 在选择结束时发出选定点的信号
     * @param polygon 选定的点
     * \endif
     */
    void selected(const QPolygon& polygon);

    /**
     * \if ENGLISH
     * @brief Signal emitted when a point has been appended to the selection
     * @param pos Position of the appended point
     * \sa append(), moved()
     * \endif
     * \if CHINESE
     * @brief 当一个点被添加到选择时发出的信号
     * @param pos 添加点的位置
     * \sa append(), moved()
     * \endif
     */
    void appended(const QPoint& pos);

    /**
     * \if ENGLISH
     * @brief Signal emitted whenever the last appended point of the selection has been moved
     * @param pos Position of the moved last point of the selection
     * \sa move(), appended()
     * \endif
     * \if CHINESE
     * @brief 当选择的最后一个添加点被移动时发出的信号
     * @param pos 移动的最后一个点的位置
     * \sa move(), appended()
     * \endif
     */
    void moved(const QPoint& pos);

    /**
     * \if ENGLISH
     * @brief Signal emitted whenever the last appended point of the selection has been removed
     * @param pos Position of the point that has been removed
     * \sa remove(), appended()
     * \endif
     * \if CHINESE
     * @brief 当选择的最后一个添加点被移除时发出的信号
     * @param pos 被移除点的位置
     * \sa remove(), appended()
     * \endif
     */
    void removed(const QPoint& pos);

    /**
     * \if ENGLISH
     * @brief Signal emitted when the active selection has been changed
     * @details This might happen when the observed widget is resized.
     * @param selection Changed selection
     * \sa stretchSelection()
     * \endif
     * \if CHINESE
     * @brief 当活动选择被更改时发出的信号
     * @details 这可能在观察部件调整大小时发生。
     * @param selection 更改的选择
     * \sa stretchSelection()
     * \endif
     */
    void changed(const QPolygon& selection);

protected:
    virtual QPolygon adjustedPoints(const QPolygon&) const;

    virtual void transition(const QEvent*);

    virtual void begin();
    virtual void append(const QPoint&);
    virtual void move(const QPoint&);
    virtual void remove();
    virtual bool end(bool ok = true);

    virtual bool accept(QPolygon&) const;
    virtual void reset();

    virtual void widgetMousePressEvent(QMouseEvent*);
    virtual void widgetMouseReleaseEvent(QMouseEvent*);
    virtual void widgetMouseDoubleClickEvent(QMouseEvent*);
    virtual void widgetMouseMoveEvent(QMouseEvent*);
    virtual void widgetWheelEvent(QWheelEvent*);
    virtual void widgetKeyPressEvent(QKeyEvent*);
    virtual void widgetKeyReleaseEvent(QKeyEvent*);
    virtual void widgetEnterEvent(QEvent*);
    virtual void widgetLeaveEvent(QEvent*);

    virtual void stretchSelection(const QSize& oldSize, const QSize& newSize);

    virtual void updateDisplay();

    const QwtWidgetOverlay* rubberBandOverlay() const;
    const QwtWidgetOverlay* trackerOverlay() const;

    const QPolygon& pickedPoints() const;

private:
    void init(QWidget*, RubberBand rubberBand, DisplayMode trackerMode);

    void setMouseTracking(bool);
};

#endif
