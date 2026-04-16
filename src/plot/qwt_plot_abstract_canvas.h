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

#ifndef QWT_PLOT_ABSTRACT_CANVAS_H
#define QWT_PLOT_ABSTRACT_CANVAS_H

#include "qwt_global.h"
#include <qframe.h>

class QwtPlot;

/**
 * \if ENGLISH
 * @brief Base class for all type of plot canvases
 * \endif
 *
 * \if CHINESE
 * @brief 所有类型绘图画布的基类
 * \endif
 */
class QWT_EXPORT QwtPlotAbstractCanvas
{
  public:
    /**
     * \if ENGLISH
     * @brief Focus indicator
     * @details The default setting is NoFocusIndicator
     * @sa setFocusIndicator(), focusIndicator(), drawFocusIndicator()
     * \endif
     *
     * \if CHINESE
     * @brief 焦点指示器
     * @details 默认设置是 NoFocusIndicator
     * @sa setFocusIndicator(), focusIndicator(), drawFocusIndicator()
     * \endif
     */

    enum FocusIndicator
    {
        /**
         * \if ENGLISH
         * Don't paint a focus indicator
         * \endif
         *
         * \if CHINESE
         * 不绘制焦点指示器
         * \endif
         */
        NoFocusIndicator,

        /**
         * \if ENGLISH
         * The focus is related to the complete canvas.
         * Paint the focus indicator using drawFocusIndicator()
         * \endif
         *
         * \if CHINESE
         * 焦点与整个画布相关。
         * 使用 drawFocusIndicator() 绘制焦点指示器
         * \endif
         */
        CanvasFocusIndicator,

        /**
         * \if ENGLISH
         * The focus is related to an item (curve, point, ...) on
         * the canvas. It is up to the application to display a
         * focus indication using e.g. highlighting.
         * \endif
         *
         * \if CHINESE
         * 焦点与画布上的项目（曲线、点等）相关。
         * 由应用程序决定如何显示焦点指示，例如通过高亮。
         * \endif
         */
        ItemFocusIndicator
    };

    // Constructor
    explicit QwtPlotAbstractCanvas( QWidget* canvasWidget );
    // Destructor
    virtual ~QwtPlotAbstractCanvas();

    // Get the parent plot widget
    QwtPlot* plot();
    // Get the parent plot widget
    const QwtPlot* plot() const;

    // Set the focus indicator
    void setFocusIndicator( FocusIndicator );
    // Get the focus indicator
    FocusIndicator focusIndicator() const;

    // Set the border radius
    void setBorderRadius( double );
    // Get the border radius
    double borderRadius() const;

  protected:
    QWidget* canvasWidget();
    const QWidget* canvasWidget() const;

    virtual void drawFocusIndicator( QPainter* );
    virtual void drawBorder( QPainter* );
    virtual void drawBackground( QPainter* );

    void fillBackground( QPainter* );
    void drawCanvas( QPainter* );
    void drawStyled( QPainter*, bool );
    void drawUnstyled( QPainter* );

    QPainterPath canvasBorderPath( const QRect& rect ) const;
    void updateStyleSheetInfo();

  private:
    Q_DISABLE_COPY(QwtPlotAbstractCanvas)

    class PrivateData;
    PrivateData* m_data;
};

/**
 * \if ENGLISH
 * @brief Base class of QwtPlotOpenGLCanvas and QwtPlotGLCanvas
 * \endif
 *
 * \if CHINESE
 * @brief QwtPlotOpenGLCanvas 和 QwtPlotGLCanvas 的基类
 * \endif
 */
class QWT_EXPORT QwtPlotAbstractGLCanvas : public QwtPlotAbstractCanvas
{
  public:
    /**
     * \if ENGLISH
     * @brief Paint attributes
     * @details The default setting enables BackingStore and Opaque.
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     *
     * \if CHINESE
     * @brief 绘制属性
     * @details 默认设置启用 BackingStore 和 Opaque。
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * @brief Paint double buffered reusing the content of the pixmap buffer when possible
         * @details Using a backing store might improve the performance significantly,
         *          when working with widget overlays (like rubber bands).
         *          Disabling the cache might improve the performance for incremental paints
         *          (using QwtPlotDirectPainter).
         * @sa backingStore(), invalidateBackingStore()
         * \endif
         *
         * \if CHINESE
         * @brief 尽可能重用像素映射缓冲区内容进行双缓冲绘制
         * @details 使用后备存储可能会显著提高性能，
         *          当使用部件覆盖（如橡皮筋）时。
         *          禁用缓存可能会提高增量绘制的性能（使用 QwtPlotDirectPainter）。
         * @sa backingStore(), invalidateBackingStore()
         * \endif
         */
        BackingStore = 1,

        /**
         * \if ENGLISH
         * @brief When ImmediatePaint is set replot() calls repaint() instead of update()
         * @sa replot(), QWidget::repaint(), QWidget::update()
         * \endif
         *
         * \if CHINESE
         * @brief 当设置 ImmediatePaint 时，replot() 调用 repaint() 而不是 update()
         * @sa replot(), QWidget::repaint(), QWidget::update()
         * \endif
         */
        ImmediatePaint = 8,
    };

    //! Paint attributes
    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    // Constructor
    explicit QwtPlotAbstractGLCanvas( QWidget* canvasWidget );
    // Destructor
    virtual ~QwtPlotAbstractGLCanvas();

    // Set a paint attribute
    void setPaintAttribute( PaintAttribute, bool on = true );
    // Test a paint attribute
    bool testPaintAttribute( PaintAttribute ) const;

    // Set the frame style
    void setFrameStyle( int style );
    // Get the frame style
    int frameStyle() const;

    // Set the frame shadow
    void setFrameShadow( QFrame::Shadow );
    // Get the frame shadow
    QFrame::Shadow frameShadow() const;

    // Set the frame shape
    void setFrameShape( QFrame::Shape );
    // Get the frame shape
    QFrame::Shape frameShape() const;

    // Set the line width
    void setLineWidth( int );
    // Get the line width
    int lineWidth() const;

    // Set the mid line width
    void setMidLineWidth( int );
    // Get the mid line width
    int midLineWidth() const;

    // Get the frame width
    int frameWidth() const;
    // Get the frame rect
    QRect frameRect() const;

    //! Invalidate the internal backing store
    virtual void invalidateBackingStore() = 0;

  protected:
    void replot();
    void draw( QPainter* );

  private:
    virtual void clearBackingStore() = 0;

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotAbstractGLCanvas::PaintAttributes )

#endif
