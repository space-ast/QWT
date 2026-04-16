/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#ifndef QWT_PLOT_CANVAS_ZOOMER_H
#define QWT_PLOT_CANVAS_ZOOMER_H
#include "qwt_global.h"
#include "qwt_canvas_picker.h"
#include "qwt_axis_id.h"
#include "qwt_interval.h"
#include <QPointer>
// Qt
class QSizeF;
template< typename T >
class QStack;
// Qwt
class QwtPlot;

/**
 * \if ENGLISH
 * @brief Struct that stores zoom states for all four axes
 * \endif
 * 
 * \if CHINESE
 * @brief 存储所有四个坐标轴缩放状态的结构体
 * \endif
 */
struct QWT_EXPORT QwtPlotCanvasZoomState
{
public:
    // Default constructor
    QwtPlotCanvasZoomState();
    
    // Constructor with plot and axis intervals
    QwtPlotCanvasZoomState(QwtPlot* p,
                           const QwtInterval& yLeft,
                           const QwtInterval& yRight,
                           const QwtInterval& xBottom,
                           const QwtInterval& xTop);
    
    // Create zoom state from current plot axis ranges
    static QwtPlotCanvasZoomState fromPlot(QwtPlot* plot);

    // Apply this zoom state to the associated plot
    void apply() const;

    // Check if two zoom states are equal
    bool operator==(const QwtPlotCanvasZoomState& other) const;
    
    // Check if two zoom states are not equal
    bool operator!=(const QwtPlotCanvasZoomState& other) const;

    // Check if this zoom state is valid (has an associated plot)
    bool isValid() const;

public:
    QPointer< QwtPlot > plot;
    QwtInterval axisInterval[ QwtAxis::AxisPositions ];
};
Q_DECLARE_METATYPE(QwtPlotCanvasZoomState)

/**
 * \if ENGLISH
 * @brief QwtPlotCanvasZoomer provides zooming for all axes of a plot canvas
 * @details QwtPlotCanvasZoomer selects rectangles from user inputs (mouse or keyboard)
 *          and adjusts ALL axes of the plot simultaneously. Unlike QwtPlotZoomer which
 *          only works on two axes, this zoomer works on the entire canvas and maintains
 *          separate ranges for all four axes.
 * 
 *          The selection is supported by a rubber band and optionally by displaying
 *          the coordinates of the current mouse position.
 * 
 *          Zooming can be repeated as often as possible, limited only by
 *          maxStackDepth() or minZoomSize(). Each zoom state is pushed on a stack.
 * 
 *          Mouse shortcuts:
 *          - MouseSelect2: Reset to base
 *          - MouseSelect3: Zoom out (back)
 *          - MouseSelect6: Zoom in (forward)
 * 
 *          Keyboard shortcuts:
 *          - KeyUndo: Zoom out (back)
 *          - KeyRedo: Zoom in (forward)
 *          - KeyHome: Reset to base
 * 
 * @sa QwtPlotPanner, QwtPlotMagnifier
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotCanvasZoomer 为绘图画布的所有坐标轴提供缩放功能
 * @details QwtPlotCanvasZoomer 从用户输入（鼠标或键盘）中选择矩形
 *          并同时调整绘图的所有坐标轴。与只在两个坐标轴上工作的 QwtPlotZoomer 不同，
 *          此缩放器在整个画布上工作，并为所有四个坐标轴维护单独的范围。
 * 
 *          选择由橡皮筋支持，并可选地显示当前鼠标位置的坐标。
 * 
 *          缩放可以尽可能重复，仅受 maxStackDepth() 或 minZoomSize() 限制。
 *          每个缩放状态都被推送到堆栈上。
 * 
 *          支持如下鼠标快捷键设置：
 *          - MouseSelect2，重置回基础
 *          - MouseSelect3，缩放栈回退
 *          - MouseSelect6，缩放栈前进
 * 
 *          支持如下键盘快捷键设置：
 *          - KeyUndo，缩放栈回退
 *          - KeyRedo，缩放栈前进
 *          - KeyHome，重置回基础
 * 
 * @sa QwtPlotPanner, QwtPlotMagnifier
 * \endif
 */
class QWT_EXPORT QwtPlotCanvasZoomer : public QwtCanvasPicker
{
    Q_OBJECT
    QWT_DECLARE_PRIVATE(QwtPlotCanvasZoomer)
public:
    // Constructor
    explicit QwtPlotCanvasZoomer(QWidget* canvas, bool doReplot = true);
    
    // Destructor
    virtual ~QwtPlotCanvasZoomer();

    // Set zoom base to current axis ranges, min zoom size is calculated based on this
    virtual void setZoomBase(bool doReplot = true);

    // Enable or disable automatic replot after zoom operations
    void setAutoReplot(bool on = true);
    
    // Check if automatic replot is enabled
    bool isAutoReplot() const;
    
    // Get the base zoom state (initial unzoomed state)
    QList< QwtPlotCanvasZoomState > zoomBase() const;
    
    // Get the current zoom state
    QList< QwtPlotCanvasZoomState > zoomState() const;
    
    // Set maximum number of zoom levels in the stack, -1 for unlimited
    void setMaxStackDepth(int);
    
    // Get maximum stack depth
    int maxStackDepth() const;
    
    // Get the zoom stack containing all zoom states
    const QStack< QList< QwtPlotCanvasZoomState > >& zoomStack() const;

    // Get the current zoom state index in the stack
    uint zoomStateIndex() const;

public Q_SLOTS:
    // Navigate in the zoom stack by offset (negative = zoom out, positive = zoom in)
    virtual void zoom(int offset);
    
    // Append a new zoom state to the stack
    virtual void appendZoom(const QList< QwtPlotCanvasZoomState >& rect);
    
Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Signal emitted when the plot has been zoomed in or out
     * @param state Current zoom state containing all axis ranges
     * \endif
     * 
     * \if CHINESE
     * @brief 当绘图被放大或缩小时发出的信号
     * @param state 包含所有坐标轴范围的当前缩放状态
     * \endif
     */
    void zoomed(const QList< QwtPlotCanvasZoomState >& state);

protected:
    virtual void rescale();

    virtual void widgetMouseReleaseEvent(QMouseEvent*) override;
    virtual void widgetKeyPressEvent(QKeyEvent*) override;

    virtual void begin() override;
    virtual bool end(bool ok = true) override;
    virtual bool accept(QPolygon&) const override;

private:
    void init(bool doReplot);

    QList< QwtPlotCanvasZoomState > canvasRectToZoomStateList(const QRect& pixelRect) const;
    QwtPlotCanvasZoomState canvasRectToZoomState(QwtPlot* plt, const QRect& pixelRect) const;
    void moveCurrentState(double dx, double dy);
};

#endif  // QWTCANVASZOOMER_H
