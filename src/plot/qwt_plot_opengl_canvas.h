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

#ifndef QWT_PLOT_OPENGL_CANVAS_H
#define QWT_PLOT_OPENGL_CANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtOpenGLWidgets/QOpenGLWidget>
#else
#include <QOpenGLWidget>
#endif
#include <QSurfaceFormat>

class QwtPlot;

/**
 * \if ENGLISH
 * @brief An alternative canvas for a QwtPlot derived from QOpenGLWidget
 * @details Even if QwtPlotOpenGLCanvas is not derived from QFrame it imitates
 *          its API. When using style sheets it supports the box model - beside
 *          backgrounds with rounded borders.
 * 
 * @sa QwtPlot::setCanvas(), QwtPlotCanvas, QwtPlotCanvas::OpenGLBuffer
 * 
 * @note Another way for getting hardware accelerated graphics is using
 *       an OpenGL offscreen buffer ( QwtPlotCanvas::OpenGLBuffer ) with QwtPlotCanvas.
 *       Performance is worse, than rendering straight to a QOpenGLWidget, but is usually
 *       better integrated into a desktop application.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlot 的替代画布，派生自 QOpenGLWidget
 * @details 即使 QwtPlotOpenGLCanvas 不是从 QFrame 派生的，它也模仿了其 API。
 *          使用样式表时，它支持盒模型 - 包括带有圆角边框的背景。
 * 
 * @sa QwtPlot::setCanvas(), QwtPlotCanvas, QwtPlotCanvas::OpenGLBuffer
 * 
 * @note 获取硬件加速图形的另一种方法是使用 QwtPlotCanvas::OpenGLBuffer 与 QwtPlotCanvas。
 *       性能比直接渲染到 QOpenGLWidget 差，但通常更好地集成到桌面应用程序中。
 * \endif
 */
class QWT_EXPORT QwtPlotOpenGLCanvas : public QOpenGLWidget, public QwtPlotAbstractGLCanvas
{
    Q_OBJECT

    Q_PROPERTY(QFrame::Shadow frameShadow READ frameShadow WRITE setFrameShadow)
    Q_PROPERTY(QFrame::Shape frameShape READ frameShape WRITE setFrameShape)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(int midLineWidth READ midLineWidth WRITE setMidLineWidth)
    Q_PROPERTY(int frameWidth READ frameWidth)
    Q_PROPERTY(QRect frameRect READ frameRect DESIGNABLE false)

    Q_PROPERTY(double borderRadius READ borderRadius WRITE setBorderRadius)

public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     *
     * \if CHINESE
     * @brief 构造函数
     * \endif
     */
    explicit QwtPlotOpenGLCanvas(QwtPlot* = nullptr);
    /**
     * \if ENGLISH
     * @brief Constructor with surface format
     * \endif
     *
     * \if CHINESE
     * @brief 构造函数（带表面格式）
     * \endif
     */
    explicit QwtPlotOpenGLCanvas(const QSurfaceFormat&, QwtPlot* = nullptr);
    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     *
     * \if CHINESE
     * @brief 析构函数
     * \endif
     */
    virtual ~QwtPlotOpenGLCanvas();

    /**
     * \if ENGLISH
     * @brief Invalidate the backing store
     * \endif
     *
     * \if CHINESE
     * @brief 使后备存储失效
     * \endif
     */
    Q_INVOKABLE virtual void invalidateBackingStore() override;
    /**
     * \if ENGLISH
     * @brief Get the border path
     * \endif
     *
     * \if CHINESE
     * @brief 获取边界路径
     * \endif
     */
    Q_INVOKABLE QPainterPath borderPath(const QRect&) const;

    /**
     * \if ENGLISH
     * @brief Handle events
     * \endif
     *
     * \if CHINESE
     * @brief 处理事件
     * \endif
     */
    virtual bool event(QEvent*) override;

public Q_SLOTS:
    /**
     * \if ENGLISH
     * @brief Replot the canvas
     * \endif
     *
     * \if CHINESE
     * @brief 重绘画布
     * \endif
     */
    void replot();

protected:
    /**
     * \if ENGLISH
     * @brief Handle paint events
     * \endif
     */
    virtual void paintEvent(QPaintEvent*) override;

    /**
     * \if ENGLISH
     * @brief Initialize OpenGL
     * \endif
     */
    virtual void initializeGL() override;
    /**
     * \if ENGLISH
     * @brief Paint OpenGL
     * \endif
     */
    virtual void paintGL() override;
    /**
     * \if ENGLISH
     * @brief Resize OpenGL
     * \endif
     */
    virtual void resizeGL(int width, int height) override;

private:
    /**
     * \if ENGLISH
     * @brief Initialize the canvas
     * \endif
     */
    void init(const QSurfaceFormat&);
    /**
     * \if ENGLISH
     * @brief Clear the backing store
     * \endif
     */
    virtual void clearBackingStore() override;

    class PrivateData;
    PrivateData* m_data;
};

#endif
