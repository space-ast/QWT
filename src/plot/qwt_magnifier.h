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

#ifndef QWT_MAGNIFIER_H
#define QWT_MAGNIFIER_H

#include "qwt_global.h"
#include <qobject.h>

class QWidget;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

/**
 * \if ENGLISH
 * @brief QwtMagnifier provides zooming by magnifying in steps
 * @details Using QwtMagnifier a plot can be zoomed in/out in steps using
 *          keys, the mouse wheel or moving a mouse button in vertical direction.
 * \endif
 * \if CHINESE
 * @brief QwtMagnifier 提供逐步放大的缩放功能
 * @details 使用 QwtMagnifier 可以通过按键、鼠标滚轮或在垂直方向移动鼠标按钮
 *          来逐步放大/缩小绘图。
 * \endif
 */
class QWT_EXPORT QwtMagnifier : public QObject
{
    Q_OBJECT

  public:
    /// Constructor for QwtMagnifier (English only)
    explicit QwtMagnifier( QWidget* );
    
    /// Destructor for QwtMagnifier (English only)
    virtual ~QwtMagnifier();

    /// Return the parent widget (non-const version) (English only)
    QWidget* parentWidget();
    
    /// Return the parent widget (const version) (English only)
    const QWidget* parentWidget() const;

    /// Enable or disable the magnifier (English only)
    void setEnabled( bool );
    
    /// Return whether the magnifier is enabled (English only)
    bool isEnabled() const;

    // mouse
    
    /// Set the mouse factor for zooming (English only)
    void setMouseFactor( double );
    
    /// Return the mouse factor (English only)
    double mouseFactor() const;

    /// Set the mouse button for zooming (English only)
    void setMouseButton( Qt::MouseButton, Qt::KeyboardModifiers = Qt::NoModifier );
    
    /// Get the mouse button and modifiers (English only)
    void getMouseButton( Qt::MouseButton&, Qt::KeyboardModifiers& ) const;

    // mouse wheel
    
    /// Set the wheel factor for zooming (English only)
    void setWheelFactor( double );
    
    /// Return the wheel factor (English only)
    double wheelFactor() const;

    /// Set the wheel modifiers (English only)
    void setWheelModifiers( Qt::KeyboardModifiers );
    
    /// Return the wheel modifiers (English only)
    Qt::KeyboardModifiers wheelModifiers() const;

    // keyboard
    
    /// Set the key factor for zooming (English only)
    void setKeyFactor( double );
    
    /// Return the key factor (English only)
    double keyFactor() const;

    /// Set the zoom in key and modifiers (English only)
    void setZoomInKey( int key, Qt::KeyboardModifiers = Qt::NoModifier );
    
    /// Get the zoom in key and modifiers (English only)
    void getZoomInKey( int& key, Qt::KeyboardModifiers& ) const;

    /// Set the zoom out key and modifiers (English only)
    void setZoomOutKey( int key, Qt::KeyboardModifiers = Qt::NoModifier );
    
    /// Get the zoom out key and modifiers (English only)
    void getZoomOutKey( int& key, Qt::KeyboardModifiers& ) const;

    /// Event filter for mouse and keyboard events (English only)
    virtual bool eventFilter( QObject*, QEvent* ) override;

  protected:
    /**
     * \if ENGLISH
     * @brief Rescale the parent widget
     * @param factor Scale factor (>1 for zoom in, <1 for zoom out)
     * \endif
     * \if CHINESE
     * @brief 重新缩放父控件
     * @param factor 缩放因子（>1 放大，<1 缩小）
     * \endif
     */
    virtual void rescale( double factor ) = 0;

    /// Handle mouse press events for the widget (English only)
    virtual void widgetMousePressEvent( QMouseEvent* );
    
    /// Handle mouse release events for the widget (English only)
    virtual void widgetMouseReleaseEvent( QMouseEvent* );
    
    /// Handle mouse move events for the widget (English only)
    virtual void widgetMouseMoveEvent( QMouseEvent* );
    
    /// Handle wheel events for the widget (English only)
    virtual void widgetWheelEvent( QWheelEvent* );
    
    /// Handle key press events for the widget (English only)
    virtual void widgetKeyPressEvent( QKeyEvent* );
    
    /// Handle key release events for the widget (English only)
    virtual void widgetKeyReleaseEvent( QKeyEvent* );

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
