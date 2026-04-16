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

#include "qwt_plot_opengl_canvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qcoreevent.h>
#include <qopenglframebufferobject.h>
#include <qopenglpaintdevice.h>

class QwtPlotOpenGLCanvas::PrivateData
{
public:
    PrivateData() : isPolished(false), fboDirty(true), fbo(nullptr)
    {
    }

    ~PrivateData()
    {
        delete fbo;
    }

    int numSamples;

    bool isPolished;
    bool fboDirty;
    QOpenGLFramebufferObject* fbo;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] plot Parent plot widget
 * @sa QwtPlot::setCanvas()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] plot 父绘图部件
 * @sa QwtPlot::setCanvas()
 * \endif
 */
QwtPlotOpenGLCanvas::QwtPlotOpenGLCanvas(QwtPlot* plot) : QOpenGLWidget(plot), QwtPlotAbstractGLCanvas(this)
{
    QSurfaceFormat fmt = format();
    fmt.setSamples(4);

    init(fmt);
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] format OpenGL surface format
 * @param[in] plot Parent plot widget
 * @sa QwtPlot::setCanvas()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] format OpenGL 表面格式
 * @param[in] plot 父绘图部件
 * @sa QwtPlot::setCanvas()
 * \endif
 */
QwtPlotOpenGLCanvas::QwtPlotOpenGLCanvas(const QSurfaceFormat& format, QwtPlot* plot)
    : QOpenGLWidget(plot), QwtPlotAbstractGLCanvas(this)
{
    init(format);
}

void QwtPlotOpenGLCanvas::init(const QSurfaceFormat& format)
{
    m_data             = new PrivateData;
    m_data->numSamples = format.samples();

    setFormat(format);

#if 1
    setAttribute(Qt::WA_OpaquePaintEvent, true);
#endif

    setLineWidth(2);
    setFrameShadow(QFrame::Sunken);
    setFrameShape(QFrame::Panel);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPlotOpenGLCanvas::~QwtPlotOpenGLCanvas()
{
    delete m_data;
}

/*!
   Paint event

   \param event Paint event
   \sa QwtPlot::drawCanvas()
 */
void QwtPlotOpenGLCanvas::paintEvent(QPaintEvent* event)
{
    if (m_data->isPolished)
        QOpenGLWidget::paintEvent(event);
}

/**
 * \if ENGLISH
 * @brief Qt event handler for QEvent::PolishRequest and QEvent::StyleChange
 * @param[in] event Qt Event
 * @return See QOpenGLWidget::event()
 * \endif
 *
 * \if CHINESE
 * @brief Qt 事件处理器，处理 QEvent::PolishRequest 和 QEvent::StyleChange
 * @param[in] event Qt 事件
 * @return 请参阅 QOpenGLWidget::event()
 * \endif
 */
bool QwtPlotOpenGLCanvas::event(QEvent* event)
{
    const bool ok = QOpenGLWidget::event(event);

    if (event->type() == QEvent::PolishRequest) {
        // In opposite to non OpenGL widgets receive pointless
        // early repaints. As we always have a QEvent::PolishRequest
        // followed by QEvent::Paint, we can ignore all these repaints.

        m_data->isPolished = true;
    }

    if (event->type() == QEvent::PolishRequest || event->type() == QEvent::StyleChange) {
        // assuming, that we always have a styled background
        // when we have a style sheet

        setAttribute(Qt::WA_StyledBackground, testAttribute(Qt::WA_StyleSheet));
    }

    return ok;
}

/**
 * \if ENGLISH
 * @brief Invalidate the paint cache and repaint the canvas
 * @sa invalidatePaintCache()
 * \endif
 *
 * \if CHINESE
 * @brief 使绘制缓存失效并重绘画布
 * @sa invalidatePaintCache()
 * \endif
 */
void QwtPlotOpenGLCanvas::replot()
{
    QwtPlotAbstractGLCanvas::replot();
}

/**
 * \if ENGLISH
 * @brief Invalidate the internal backing store
 * \endif
 *
 * \if CHINESE
 * @brief 使内部后备存储失效
 * \endif
 */
void QwtPlotOpenGLCanvas::invalidateBackingStore()
{
    m_data->fboDirty = true;
}

void QwtPlotOpenGLCanvas::clearBackingStore()
{
    delete m_data->fbo;
    m_data->fbo = nullptr;
}

/**
 * \if ENGLISH
 * @brief Calculate the painter path for a styled or rounded border
 * @details When the canvas has no styled background or rounded borders
 *          the painter path is empty.
 * @param[in] rect Bounding rectangle of the canvas
 * @return Painter path, that can be used for clipping
 * \endif
 *
 * \if CHINESE
 * @brief 计算样式化或圆角边界的绘制路径
 * @details 当画布没有样式化背景或圆角边界时，
 *          绘制路径为空。
 * @param[in] rect 画布的边界矩形
 * @return 可用于裁剪的绘制路径
 * \endif
 */
QPainterPath QwtPlotOpenGLCanvas::borderPath(const QRect& rect) const
{
    return canvasBorderPath(rect);
}

//! No operation - reserved for some potential use in the future
void QwtPlotOpenGLCanvas::initializeGL()
{
}

//! Paint the plot
void QwtPlotOpenGLCanvas::paintGL()
{
    const bool hasFocusIndicator = hasFocus() && focusIndicator() == CanvasFocusIndicator;

    QPainter painter;

    if (testPaintAttribute(QwtPlotOpenGLCanvas::BackingStore) && QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) {
        const qreal pixelRatio = QwtPainter::devicePixelRatio(nullptr);
        const QSize fboSize    = size() * pixelRatio;

        if (hasFocusIndicator)
            painter.begin(this);

        /*
           QOpenGLWidget has its own internal FBO, that is used to restore
           its content without having to repaint. This works fine when f.e
           a rubberband is moving on top, but there are still situations,
           where we can repaint without an potentially expensive replot:

               - when having the focus the top level window gets activated/deactivated
               - ???
         */

        if (m_data->fbo) {
            if (m_data->fbo->size() != fboSize) {
                delete m_data->fbo;
                m_data->fbo = nullptr;
            }
        }

        if (m_data->fbo == nullptr) {
            QOpenGLFramebufferObjectFormat fboFormat;
            fboFormat.setSamples(m_data->numSamples);
            fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

            m_data->fbo      = new QOpenGLFramebufferObject(fboSize, fboFormat);
            m_data->fboDirty = true;
        }

        if (m_data->fboDirty) {
            m_data->fbo->bind();

            QOpenGLPaintDevice pd(fboSize);

            QPainter fboPainter(&pd);
            fboPainter.scale(pixelRatio, pixelRatio);
            draw(&fboPainter);
            fboPainter.end();

            m_data->fboDirty = false;
        }

        QOpenGLFramebufferObject::blitFramebuffer(nullptr, m_data->fbo);
    } else {
        painter.begin(this);
        draw(&painter);
    }

    if (hasFocusIndicator)
        drawFocusIndicator(&painter);
}

//! No operation - reserved for some potential use in the future
void QwtPlotOpenGLCanvas::resizeGL(int, int)
{
    // nothing to do
}
