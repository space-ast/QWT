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

#ifndef QWT_WIDGET_OVERLAY_H
#define QWT_WIDGET_OVERLAY_H

#include "qwt_global.h"
#include <qwidget.h>

class QPainter;
class QRegion;

/**
 * \if ENGLISH
 * @brief An overlay for a widget
 *
 * The main use case of an widget overlay is to avoid
 * heavy repaint operation of the widget below.
 *
 * F.e. in combination with the plot canvas an overlay
 * avoid replots as the content of the canvas can be restored from
 * its backing store.
 *
 * QwtWidgetOverlay is an abstract base class. Deriving classes are
 * supposed to reimplement the following methods:
 *
 * - drawOverlay()
 * - maskHint()
 *
 * Internally QwtPlotPicker uses overlays for displaying
 * the rubber band and the tracker text.
 *
 * \sa QwtPlotCanvas::BackingStore
 * \endif
 *
 * \if CHINESE
 * @brief 控件的覆盖层
 *
 * 控件覆盖层的主要用途是避免下方控件的重绘操作。
 *
 * 例如，与绘图画布结合使用时，覆盖层可以避免重绘，
 * 因为画布的内容可以从其后备存储中恢复。
 *
 * QwtWidgetOverlay 是一个抽象基类。派生类应该
 * 重新实现以下方法：
 *
 * - drawOverlay()
 * - maskHint()
 *
 * 内部 QwtPlotPicker 使用覆盖层来显示
 * 橡皮筋和跟踪器文本。
 *
 * \sa QwtPlotCanvas::BackingStore
 * \endif
 */
class QWT_EXPORT QwtWidgetOverlay : public QWidget
{
    Q_OBJECT
public:
    /**
     * \if ENGLISH
     * @brief Mask mode
     *
     * When using masks the widget below gets paint events for
     * the masked regions of the overlay only. Otherwise
     * Qt triggers full repaints. On less powerful hardware
     * ( f.e embedded systems ) - or when using the raster paint
     * engine on a remote desktop - bit blitting is a noticeable
     * operation, that needs to be avoided.
     *
     * If and how to mask depends on how expensive the calculation
     * of the mask is and how many pixels can be excluded by the mask.
     *
     * The default setting is MaskHint.
     *
     * \sa setMaskMode(), maskMode()
     * \endif
     *
     * \if CHINESE
     * @brief 掩码模式
     *
     * 使用掩码时，下方的控件只会为覆盖层的掩码区域
     * 接收绘制事件。否则 Qt 会触发完全重绘。在性能较弱的
     * 硬件上（例如嵌入式系统）——或者在远程桌面上使用光栅
     * 绘制引擎时——位块传输是一个需要注意的操作，应该避免。
     *
     * 是否使用掩码以及如何使用取决于掩码计算的开销
     * 以及有多少像素可以被排除。
     *
     * 默认设置为 MaskHint。
     *
     * \sa setMaskMode(), maskMode()
     * \endif
     */
    enum MaskMode
    {
        /**
         * \if ENGLISH
         * @brief Don't use a mask.
         * \endif
         *
         * \if CHINESE
         * @brief 不使用掩码。
         * \endif
         */
        NoMask,

        /*!
           \if ENGLISH
           @brief Use maskHint() as mask

           For many situations a fast approximation is good enough
           and it is not necessary to build a more detailed mask
           ( f.e the bounding rectangle of a text ).
           \endif
           *
           \if CHINESE
           @brief 使用 maskHint() 作为掩码

           在许多情况下，快速近似已经足够，
           不需要构建更详细的掩码（例如文本的边界矩形）。
           \endif
         */
        MaskHint,

        /*!
           \if ENGLISH
           @brief Calculate a mask by checking the alpha values

           Sometimes it is not possible to give a fast approximation
           and the mask needs to be calculated by drawing the overlay
           and testing the result.

           When a valid maskHint() is available
           only pixels inside this approximation are checked.
           \endif
           *
           \if CHINESE
           @brief 通过检查 alpha 值计算掩码

           有时无法提供快速近似，
           需要通过绘制覆盖层并测试结果来计算掩码。

           当有有效的 maskHint() 时，
           只检查此近似内的像素。
           \endif
         */
        AlphaMask
    };

    /*!
       \if ENGLISH
       @brief Render mode

       For calculating the alpha mask the overlay has already
       been painted to a temporary QImage. Instead of rendering
       the overlay twice this buffer can be copied for drawing
       the overlay.

       On graphic systems using the raster paint engine ( QWS, Windows )
       it means usually copying some memory only. On X11 it results in an
       expensive operation building a pixmap and for simple overlays
       it might not be recommended.

       \note The render mode has no effect, when maskMode() != AlphaMask.
       \endif
       *
       \if CHINESE
       @brief 渲染模式

       为了计算 alpha 掩码，覆盖层已经被绘制到临时的 QImage。
       与其将覆盖层渲染两次，不如复制此缓冲区用于绘制覆盖层。

       在使用光栅绘制引擎的图形系统上（QWS, Windows），
       这通常只意味着复制一些内存。在 X11 上，这会导致
       构建 pixmap 的昂贵操作，对于简单的覆盖层可能不推荐。

       \note 当 maskMode() != AlphaMask 时，渲染模式无效。
       \endif
     */
    enum RenderMode
    {
        /**
         * \if ENGLISH
         * @brief Copy the buffer, when using the raster paint engine.
         * \endif
         *
         * \if CHINESE
         * @brief 使用光栅绘制引擎时复制缓冲区。
         * \endif
         */
        AutoRenderMode,

        /**
         * \if ENGLISH
         * @brief Always copy the buffer
         * \endif
         *
         * \if CHINESE
         * @brief 始终复制缓冲区
         * \endif
         */
        CopyAlphaMask,

        /**
         * \if ENGLISH
         * @brief Never copy the buffer
         * \endif
         *
         * \if CHINESE
         * @brief 从不复制缓冲区
         * \endif
         */
        DrawOverlay
    };

    explicit QwtWidgetOverlay(QWidget*);
    virtual ~QwtWidgetOverlay();

    void setMaskMode(MaskMode);
    MaskMode maskMode() const;

    void setRenderMode(RenderMode);
    RenderMode renderMode() const;

    virtual bool eventFilter(QObject*, QEvent*) override;

public Q_SLOTS:
    void updateOverlay();

protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent*) override;

    virtual QRegion maskHint() const;

    /*!
       \if ENGLISH
       Draw the widget overlay
       \param painter Painter
       \endif
       *
       \if CHINESE
       绘制控件覆盖层
       \param painter 绘制器
       \endif
     */
    virtual void drawOverlay(QPainter* painter) const = 0;

private:
    void updateMask();
    void draw(QPainter*) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
