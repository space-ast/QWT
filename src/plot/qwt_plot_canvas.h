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

#ifndef QWT_PLOT_CANVAS_H
#define QWT_PLOT_CANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"

#include <qframe.h>

class QwtPlot;
class QPixmap;
class QPainterPath;

/**
 * \if ENGLISH
 * @brief Canvas of a QwtPlot
 * @details Canvas is the widget where all plot items are displayed
 * @sa QwtPlot::setCanvas(), QwtPlotGLCanvas, QwtPlotOpenGLCanvas
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlot 的画布
 * @details 画布是显示所有绘图项的部件
 * @sa QwtPlot::setCanvas(), QwtPlotGLCanvas, QwtPlotOpenGLCanvas
 * \endif
 */
class QWT_EXPORT QwtPlotCanvas : public QFrame, public QwtPlotAbstractCanvas
{
    Q_OBJECT

    Q_PROPERTY(double borderRadius READ borderRadius WRITE setBorderRadius)

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
     * @details 默认设置启用 BackingStore 与 Opaque。
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * @brief BackingStore
         * @details Paint double buffered reusing the content of the pixmap buffer when possible.
         *          Using a backing store might improve the performance significantly,
         *          when working with widget overlays (like rubber bands).
         *          Disabling the cache might improve the performance for incremental paints
         *          (using QwtPlotDirectPainter).
         * @sa backingStore(), invalidateBackingStore()
         * \endif
         * 
         * \if CHINESE
         * @brief 后备存储
         * @details 双缓冲绘制，尽可能复用 pixmap 缓存的内容。
         *          使用后备存储可显著提升性能，尤其当存在部件叠加（如 rubber band）时。
         *          禁用缓存可能提升增量绘制（如使用 QwtPlotDirectPainter）的性能。
         * @sa backingStore(), invalidateBackingStore()
         * \endif
         */
        BackingStore = 1,

        /**
         * \if ENGLISH
         * @brief Opaque
         * @details Try to fill the complete contents rectangle of the plot canvas.
         *          When using styled backgrounds Qt assumes that the canvas doesn't fill its area
         *          completely (e.g. because of rounded borders) and fills the area below the canvas.
         *          When this is done with gradients it might result in a serious performance bottleneck
         *          — depending on the size.
         *          When the Opaque attribute is enabled the canvas tries to identify the gaps
         *          with some heuristics and to fill those only.
         * @warning Will not work for semitransparent backgrounds.
         * \endif
         * 
         * \if CHINESE
         * @brief 不透明
         * @details 尝试填充绘图画布的全部内容矩形。
         *          使用样式背景时，Qt 假定画布并未完全填充其区域（例如圆角边框），
         *          因而会在画布下方继续填充。若使用渐变填充，这可能成为严重性能瓶颈。
         *          启用 Opaque 后，画布会通过启发式算法识别空隙并仅填充这些区域。
         * @warning 对半透明背景无效。
         * \endif
         */
        Opaque = 2,

        /**
         * \if ENGLISH
         * @brief HackStyledBackground
         * @details Try to improve painting of styled backgrounds.
         *          QwtPlotCanvas supports the box model attributes for customizing the layout
         *          with style sheets. Unfortunately the design of Qt style sheets has no concept
         *          how to handle backgrounds with rounded corners — beside of padding.
         *          When HackStyledBackground is enabled the plot canvas tries to separate
         *          the background from the background border by reverse engineering to paint
         *          the background before and the border after the plot items.
         *          In this order the border gets perfectly antialiased and you can avoid
         *          some pixel artifacts in the corners.
         * \endif
         * 
         * \if CHINESE
         * @brief 样式背景黑客
         * @details 尝试优化样式背景的绘制。
         *          QwtPlotCanvas 支持盒模型属性，以便通过样式表自定义布局。
         *          遗憾的是，Qt 样式表的设计并未提供处理圆角背景的方案，除 padding 外。
         *          启用 HackStyledBackground 后，画布会通过逆向工程将背景与边框分离：
         *          先绘制背景，再绘制边框，使边框获得完美抗锯齿并避免角落像素伪影。
         * \endif
         */
        HackStyledBackground = 4,

        /**
         * \if ENGLISH
         * @brief ImmediatePaint
         * @details When ImmediatePaint is set replot() calls repaint() instead of update().
         * @sa replot(), QWidget::repaint(), QWidget::update()
         * \endif
         * 
         * \if CHINESE
         * @brief 立即绘制
         * @details 当 ImmediatePaint 被设置时，replot() 将调用 repaint() 而非 update()。
         * @sa replot(), QWidget::repaint(), QWidget::update()
         * \endif
         */
        ImmediatePaint = 8
    };

    Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)

    /// Constructor
    explicit QwtPlotCanvas(QwtPlot* = nullptr);
    /// Destructor
    virtual ~QwtPlotCanvas();

    /// Set paint attribute
    void setPaintAttribute(PaintAttribute, bool on = true);
    /// Test paint attribute
    bool testPaintAttribute(PaintAttribute) const;

    /// Get backing store
    const QPixmap* backingStore() const;
    /// Invalidate backing store
    Q_INVOKABLE void invalidateBackingStore();

    /// Handle events
    virtual bool event(QEvent*) override;

    /// Get border path
    Q_INVOKABLE QPainterPath borderPath(const QRect&) const;

public Q_SLOTS:
    /// Replot the canvas
    void replot();

protected:
    /// Paint event handler
    virtual void paintEvent(QPaintEvent*) override;
    /// Resize event handler
    virtual void resizeEvent(QResizeEvent*) override;

    /// Draw border
    virtual void drawBorder(QPainter*) override;

private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotCanvas::PaintAttributes)

#endif
