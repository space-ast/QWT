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

#ifndef QWT_CACHE_PANNER_H
#define QWT_CACHE_PANNER_H

#include "qwt_global.h"
#include <qwidget.h>

class QCursor;
class QPixmap;

/**
 * \if ENGLISH
 * @brief QwtCachePanner provides panning of a widget
 * @details QwtCachePanner grabs the contents of a widget, that can be dragged
 *          in all directions. The offset between the start and the end position
 *          is emitted by the panned signal.
 *          QwtCachePanner grabs the content of the widget into a pixmap and moves
 *          the pixmap around, without initiating any repaint events for the widget.
 *          Areas, that are not part of content are not painted while panning.
 *          This makes panning fast enough for widgets, where repaints are too slow
 *          for mouse movements.
 *          For widgets, where repaints are very fast it might be better to
 *          implement panning manually by mapping mouse events into paint events.
 * \endif
 * \if CHINESE
 * @brief QwtCachePanner 提供控件的拖动平移功能
 * @details QwtCachePanner 捕获控件的内容，可以在所有方向上拖动。
 *          起始位置和结束位置之间的偏移通过 panned 信号发出。
 *          QwtCachePanner 将控件内容捕获到 pixmap 中并移动 pixmap，
 *          而不会为控件触发任何重绘事件。
 *          不属于内容的部分在平移时不会被绘制。
 *          这使得平移速度足够快，适用于重绘太慢而无法跟随鼠标移动的控件。
 *          对于重绘非常快的控件，最好手动实现平移，
 *          将鼠标事件映射为绘制事件。
 * \endif
 */
class QWT_EXPORT QwtCachePanner : public QWidget
{
    Q_OBJECT

public:
    // Constructor with parent widget
    explicit QwtCachePanner(QWidget* parent);
    // Destructor
    virtual ~QwtCachePanner();

    // Enable or disable the panner
    void setEnabled(bool);
    // Return whether the panner is enabled
    bool isEnabled() const;

    // Set the mouse button and modifiers for panning
    void setMouseButton(Qt::MouseButton, Qt::KeyboardModifiers = Qt::NoModifier);
    // Get the mouse button and modifiers used for panning
    void getMouseButton(Qt::MouseButton& button, Qt::KeyboardModifiers&) const;

    // Set the abort key and modifiers
    void setAbortKey(int key, Qt::KeyboardModifiers = Qt::NoModifier);
    // Get the abort key and modifiers
    void getAbortKey(int& key, Qt::KeyboardModifiers&) const;

    // Set the cursor active while panning
    void setCursor(const QCursor&);
    // Return the cursor active while panning
    const QCursor cursor() const;

    // Set orientations where panning is enabled
    void setOrientations(Qt::Orientations);
    // Return orientations where panning is enabled
    Qt::Orientations orientations() const;

    // Check if an orientation is enabled for panning
    bool isOrientationEnabled(Qt::Orientation) const;

    // Event filter for the parent widget
    virtual bool eventFilter(QObject*, QEvent*) override;

Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Signal emitted when panning is done
     * @param[in] dx Offset in horizontal direction
     * @param[in] dy Offset in vertical direction
     * \endif
     * \if CHINESE
     * @brief 平移完成时发出的信号
     * @param[in] dx 水平方向的偏移量
     * @param[in] dy 垂直方向的偏移量
     * \endif
     */
    void panned(int dx, int dy);

    /**
     * \if ENGLISH
     * @brief Signal emitted while the widget moved, but panning is not finished
     * @param[in] dx Offset in horizontal direction
     * @param[in] dy Offset in vertical direction
     * \endif
     * \if CHINESE
     * @brief 控件移动但平移未完成时发出的信号
     * @param[in] dx 水平方向的偏移量
     * @param[in] dy 垂直方向的偏移量
     * \endif
     */
    void moved(int dx, int dy);

protected:
    virtual void widgetMousePressEvent(QMouseEvent*);
    virtual void widgetMouseReleaseEvent(QMouseEvent*);
    virtual void widgetMouseMoveEvent(QMouseEvent*);
    virtual void widgetKeyPressEvent(QKeyEvent*);
    virtual void widgetKeyReleaseEvent(QKeyEvent*);

    virtual void paintEvent(QPaintEvent*) override;

    virtual QBitmap contentsMask() const;
    virtual QPixmap grab() const;

private:
#ifndef QT_NO_CURSOR
    void showCursor(bool);
#endif

    class PrivateData;
    PrivateData* m_data;
};

#endif
