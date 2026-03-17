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

#ifndef QWT_PLOT_GLCANVAS_H
#define QWT_PLOT_GLCANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"

#include <qgl.h>

class QwtPlot;

/**
 * \if ENGLISH
 * @brief An alternative canvas for a QwtPlot derived from QGLWidget
 * @details QwtPlotGLCanvas implements the very basics to act as canvas
 *          inside of a QwtPlot widget. It might be extended to a full
 *          featured alternative to QwtPlotCanvas in a future version of Qwt.
 * 
 *          Even if QwtPlotGLCanvas is not derived from QFrame it imitates
 *          its API. When using style sheets it supports the box model - beside
 *          backgrounds with rounded borders.
 * 
 *          Since Qt 5.4 QOpenGLWidget is available, that is used by QwtPlotOpenGLCanvas.
 * 
 * @sa QwtPlot::setCanvas(), QwtPlotCanvas, QwtPlotOpenGLCanvas
 * 
 * @note With Qt4 you might want to use the QPaintEngine::OpenGL paint engine
 *       ( see QGL::setPreferredPaintEngine() ). On a Linux test system
 *       QPaintEngine::OpenGL2 shows very basic problems like translated
 *       geometries.
 * 
 * @note Another way for getting hardware accelerated graphics is using
 *       an OpenGL offscreen buffer ( QwtPlotCanvas::OpenGLBuffer ) with QwtPlotCanvas.
 *       Performance is worse, than rendering straight to a QGLWidget, but is usually
 *       better integrated into a desktop application.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlot 的替代画布，派生自 QGLWidget
 * @details QwtPlotGLCanvas 实现了作为 QwtPlot 部件内部画布的基本功能。
 *          它可能会在 Qwt 的未来版本中扩展为 QwtPlotCanvas 的全功能替代方案。
 * 
 *          即使 QwtPlotGLCanvas 不是从 QFrame 派生的，它也模仿了其 API。
 *          使用样式表时，它支持盒模型 - 包括带有圆角边框的背景。
 * 
 *          从 Qt 5.4 开始，QOpenGLWidget 可用，QwtPlotOpenGLCanvas 使用它。
 * 
 * @sa QwtPlot::setCanvas(), QwtPlotCanvas, QwtPlotOpenGLCanvas
 * 
 * @note 在 Qt4 中，您可能希望使用 QPaintEngine::OpenGL 绘图引擎
 *       ( 请参阅 QGL::setPreferredPaintEngine() )。在 Linux 测试系统上，
 *       QPaintEngine::OpenGL2 显示出非常基本的问题，如几何图形平移。
 * 
 * @note 获得硬件加速图形的另一种方法是使用 QwtPlotCanvas 的 OpenGL 离屏缓冲区
 *       ( QwtPlotCanvas::OpenGLBuffer )。性能比直接渲染到 QGLWidget 差，
 *       但通常更好地集成到桌面应用程序中。
 * \endif
 */
class QWT_EXPORT QwtPlotGLCanvas : public QGLWidget, public QwtPlotAbstractGLCanvas
{
    Q_OBJECT

    Q_PROPERTY( QFrame::Shadow frameShadow READ frameShadow WRITE setFrameShadow )
    Q_PROPERTY( QFrame::Shape frameShape READ frameShape WRITE setFrameShape )
    Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
    Q_PROPERTY( int midLineWidth READ midLineWidth WRITE setMidLineWidth )
    Q_PROPERTY( int frameWidth READ frameWidth )
    Q_PROPERTY( QRect frameRect READ frameRect DESIGNABLE false )

    Q_PROPERTY( double borderRadius READ borderRadius WRITE setBorderRadius )

  public:
    /// Constructor
    explicit QwtPlotGLCanvas( QwtPlot* = nullptr );
    /// Constructor with QGLFormat
    explicit QwtPlotGLCanvas( const QGLFormat&, QwtPlot* = nullptr );
    /// Destructor
    virtual ~QwtPlotGLCanvas();

    /// Invalidate the backing store
    Q_INVOKABLE virtual void invalidateBackingStore() override;
    /// Get the border path
    Q_INVOKABLE QPainterPath borderPath( const QRect& ) const;

    /// Handle events
    virtual bool event( QEvent* ) override;

  public Q_SLOTS:
    /// Replot the canvas
    void replot();

  protected:
    /// Paint event handler
    virtual void paintEvent( QPaintEvent* ) override;

    /// Initialize OpenGL
    virtual void initializeGL() override;
    /// Paint OpenGL scene
    virtual void paintGL() override;
    /// Resize OpenGL view
    virtual void resizeGL( int width, int height ) override;

  private:
    /// Initialize the canvas
    void init();
    /// Clear the backing store
    virtual void clearBackingStore() override;

    class PrivateData;
    PrivateData* m_data;
};

#endif
