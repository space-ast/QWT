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

#ifndef QWT_PLOT_AXIS_ZOOMER_H
#define QWT_PLOT_AXIS_ZOOMER_H

#include "qwt_global.h"
#include "qwt_plot_picker.h"

class QSizeF;
template< typename T >
class QStack;

/**
 * \if ENGLISH
 * @brief QwtPlotAxisZoomer provides stacked zooming for a plot widget
 * @details QwtPlotAxisZoomer selects rectangles from user inputs ( mouse or keyboard )
 *          translates them into plot coordinates and adjusts the axes to them.
 *          The selection is supported by a rubber band and optionally by displaying
 *          the coordinates of the current mouse position.
 * 
 *          Zooming can be repeated as often as possible, limited only by
 *          maxStackDepth() or minZoomSize().  Each rectangle is pushed on a stack.
 * 
 *          The default setting how to select rectangles is
 *          a QwtPickerDragRectMachine with the following bindings:
 * 
 *          - QwtEventPattern::MouseSelect1
 *           The first point of the zoom rectangle is selected by a mouse press,
 *           the second point from the position, where the mouse is released.
 * 
 *          - QwtEventPattern::KeySelect1
 *           The first key press selects the first, the second key press
 *           selects the second point.
 * 
 *          - QwtEventPattern::KeyAbort
 *           Discard the selection in the state, where the first point
 *           is selected.
 * 
 *          To traverse the zoom stack the following bindings are used:
 * 
 *          - QwtEventPattern::MouseSelect3, QwtEventPattern::KeyUndo
 *           Zoom out one position on the zoom stack
 * 
 *          - QwtEventPattern::MouseSelect6, QwtEventPattern::KeyRedo
 *           Zoom in one position on the zoom stack
 * 
 *          - QwtEventPattern::MouseSelect2, QwtEventPattern::KeyHome
 *           Zoom to the zoom base
 * 
 *          The setKeyPattern() and setMousePattern() functions can be used
 *          to configure the zoomer actions. The following example
 *          shows, how to configure the 'I' and 'O' keys for zooming in and out
 *          one position on the zoom stack. The "Home" key is used to
 *          "unzoom" the plot.
 * 
 *          @code
 *          zoomer = new QwtPlotAxisZoomer( plot );
 *          zoomer->setKeyPattern( QwtEventPattern::KeyRedo, Qt::Key_I, Qt::ShiftModifier );
 *          zoomer->setKeyPattern( QwtEventPattern::KeyUndo, Qt::Key_O, Qt::ShiftModifier );
 *          zoomer->setKeyPattern( QwtEventPattern::KeyHome, Qt::Key_Home );
 *          @endcode
 * 
 *          QwtPlotAxisZoomer is tailored for plots with one x and y axis, but it is
 *          allowed to attach a second QwtPlotAxisZoomer ( without rubber band and tracker )
 *          for the other axes.
 * 
 * @note The realtime example includes an derived zoomer class that adds
 *       scrollbars to the plot canvas.
 * @sa QwtPlotPanner, QwtPlotMagnifier
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotAxisZoomer 为绘图部件提供堆叠式缩放功能
 * @details QwtPlotAxisZoomer 根据用户输入（鼠标或键盘）选择矩形区域，
 *          将其转换为绘图坐标，并相应地调整坐标轴。
 *          选择操作由"橡皮筋"辅助，并可选择显示当前鼠标位置的坐标。
 * 
 *          缩放操作可以无限次重复，仅受 maxStackDepth() 或 minZoomSize() 的限制。
 *          每个矩形都会被压入一个堆栈中。
 * 
 *          默认的矩形选择方式是一个 QwtPickerDragRectMachine，
 *          其绑定方式如下：
 * 
 *          - QwtEventPattern::MouseSelect1
 *           缩放矩形的第一个点通过鼠标按下选择，第二个点则根据鼠标释放时的位置确定。
 * 
 *          - QwtEventPattern::KeySelect1
 *           第一次按键选择第一个点，第二次按键选择第二个点。
 * 
 *          - QwtEventPattern::KeyAbort
 *           在已选择第一个点的状态下，放弃当前选择。
 * 
 *          要遍历缩放堆栈，可使用以下绑定方式：
 * 
 *          - QwtEventPattern::MouseSelect3, QwtEventPattern::KeyUndo
 *           在缩放堆栈中后退一步（缩小）
 * 
 *          - QwtEventPattern::MouseSelect6, QwtEventPattern::KeyRedo
 *           在缩放堆栈中前进一步（放大）
 * 
 *          - QwtEventPattern::MouseSelect2, QwtEventPattern::KeyHome
 *           缩放到基准视图（完全缩小）
 * 
 *          可通过 setKeyPattern() 和 setMousePattern() 函数配置缩放器的行为。
 *          以下示例展示了如何将 'I' 和 'O' 键配置为在缩放堆栈中前进和后退一步，
 *          并使用 "Home" 键将绘图"取消缩放"到初始状态。
 * 
 *          @code
 *          zoomer = new QwtPlotAxisZoomer( plot );
 *          zoomer->setKeyPattern( QwtEventPattern::KeyRedo, Qt::Key_I, Qt::ShiftModifier );
 *          zoomer->setKeyPattern( QwtEventPattern::KeyUndo, Qt::Key_O, Qt::ShiftModifier );
 *          zoomer->setKeyPattern( QwtEventPattern::KeyHome, Qt::Key_Home );
 *          @endcode
 * 
 *          QwtPlotAxisZoomer 专为具有一个 x 轴和一个 y 轴的绘图设计，
 *          但也允许附加第二个 QwtPlotAxisZoomer（不带橡皮筋和追踪器）用于其他轴。
 * 
 * @note realtime 示例包含一个派生的缩放器类，为绘图画布添加了滚动条。
 * @sa QwtPlotPanner, QwtPlotMagnifier
 * \endif
 */
class QWT_EXPORT QwtPlotAxisZoomer : public QwtPlotPicker
{
    Q_OBJECT
public:
    /// Constructor
    explicit QwtPlotAxisZoomer(QWidget*, bool doReplot = true);
    /// Constructor with axes
    explicit QwtPlotAxisZoomer(QwtAxisId xAxis, QwtAxisId yAxis, QWidget*, bool doReplot = true);

    /// Destructor
    virtual ~QwtPlotAxisZoomer();

    /// Set zoom base
    virtual void setZoomBase(bool doReplot = true);
    /// Set zoom base from rectangle
    virtual void setZoomBase(const QRectF&);

    /// Get zoom base
    QRectF zoomBase() const;
    /// Get current zoom rectangle
    QRectF zoomRect() const;

    /// Set axes
    virtual void setAxes(QwtAxisId xAxis, QwtAxisId yAxis) override;

    /// Set maximum stack depth
    void setMaxStackDepth(int);
    /// Get maximum stack depth
    int maxStackDepth() const;

    /// Get zoom stack
    const QStack< QRectF >& zoomStack() const;
    /// Set zoom stack
    void setZoomStack(const QStack< QRectF >&, int zoomRectIndex = -1);

    /// Get current zoom rectangle index
    uint zoomRectIndex() const;

public Q_SLOTS:
    /// Move by offset
    void moveBy(double dx, double dy);
    /// Move to position
    virtual void moveTo(const QPointF&);

    /// Zoom to rectangle
    virtual void zoom(const QRectF&);
    /// Zoom by offset in stack
    virtual void zoom(int offset);

Q_SIGNALS:
    /**
     * \if ENGLISH
     * A signal emitting the zoomRect(), when the plot has been
     * zoomed in or out.
     * @param rect Current zoom rectangle.
     * \endif
     * 
     * \if CHINESE
     * 当绘图被放大或缩小时发出的信号，包含当前的缩放矩形。
     * @param rect 当前缩放矩形。
     * \endif
     */
    void zoomed(const QRectF& rect);

protected:
    /// Rescale the plot
    virtual void rescale();

    /// Get minimum zoom size
    virtual QSizeF minZoomSize() const;

    /// Handle mouse release event
    virtual void widgetMouseReleaseEvent(QMouseEvent*) override;
    /// Handle key press event
    virtual void widgetKeyPressEvent(QKeyEvent*) override;

    /// Begin selection
    virtual void begin() override;
    /// End selection
    virtual bool end(bool ok = true) override;
    /// Accept selection
    virtual bool accept(QPolygon&) const override;

private:
    /// Initialize the zoomer
    void init(bool doReplot);

    class PrivateData;
    PrivateData* m_data;
};

#endif
