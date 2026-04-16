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

#include "qwt_plot_glcanvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"

#include <qcoreevent.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qglframebufferobject.h>

namespace
{
class QwtPlotGLCanvasFormat : public QGLFormat
{
public:
    QwtPlotGLCanvasFormat() : QGLFormat(QGLFormat::defaultFormat())
    {
        setSampleBuffers(true);
    }
};
}

class QwtPlotGLCanvas::PrivateData
{
public:
    PrivateData() : fboDirty(true), fbo(nullptr)
    {
    }

    ~PrivateData()
    {
        delete fbo;
    }

    bool fboDirty;
    QGLFramebufferObject* fbo;
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
QwtPlotGLCanvas::QwtPlotGLCanvas(QwtPlot* plot)
    : QGLWidget(QwtPlotGLCanvasFormat(), plot), QwtPlotAbstractGLCanvas(this)
{
    init();
}
/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] format OpenGL rendering options
 * @param[in] plot Parent plot widget
 * @sa QwtPlot::setCanvas()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] format OpenGL 渲染选项
 * @param[in] plot 父绘图部件
 * @sa QwtPlot::setCanvas()
 * \endif
 */
QwtPlotGLCanvas::QwtPlotGLCanvas(const QGLFormat& format, QwtPlot* plot)
    : QGLWidget(format, plot), QwtPlotAbstractGLCanvas(this)
{
    init();
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
QwtPlotGLCanvas::~QwtPlotGLCanvas()
{
    delete m_data;
}

void QwtPlotGLCanvas::init()
{
    m_data = new PrivateData;

#if 1
    setAttribute(Qt::WA_OpaquePaintEvent, true);
#endif
    setLineWidth(2);
    setFrameShadow(QFrame::Sunken);
    setFrameShape(QFrame::Panel);
}

/*!
   Paint event

   \param event Paint event
   \sa QwtPlot::drawCanvas()
 */
void QwtPlotGLCanvas::paintEvent(QPaintEvent* event)
{
    QGLWidget::paintEvent(event);
}

/**
 * \if ENGLISH
 * @brief Qt event handler for QEvent::PolishRequest and QEvent::StyleChange
 * @param[in] event Qt Event
 * @return See QGLWidget::event()
 * \endif
 *
 * \if CHINESE
 * @brief Qt 事件处理器，处理 QEvent::PolishRequest 和 QEvent::StyleChange
 * @param[in] event Qt 事件
 * @return 请参阅 QGLWidget::event()
 * \endif
 */
bool QwtPlotGLCanvas::event(QEvent* event)
{
    const bool ok = QGLWidget::event(event);

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
void QwtPlotGLCanvas::replot()
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
void QwtPlotGLCanvas::invalidateBackingStore()
{
    m_data->fboDirty = true;
}

void QwtPlotGLCanvas::clearBackingStore()
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
QPainterPath QwtPlotGLCanvas::borderPath(const QRect& rect) const
{
    return canvasBorderPath(rect);
}

//! No operation - reserved for some potential use in the future
void QwtPlotGLCanvas::initializeGL()
{
}

//! Paint the plot
void QwtPlotGLCanvas::paintGL()
{
    const bool hasFocusIndicator = hasFocus() && focusIndicator() == CanvasFocusIndicator;

    QPainter painter;

    if (testPaintAttribute(QwtPlotGLCanvas::BackingStore)) {
        const qreal pixelRatio = QwtPainter::devicePixelRatio(nullptr);
        const QRect rect(0, 0, width() * pixelRatio, height() * pixelRatio);

        if (hasFocusIndicator)
            painter.begin(this);

        if (m_data->fbo) {
            if (m_data->fbo->size() != rect.size()) {
                delete m_data->fbo;
                m_data->fbo = nullptr;
            }
        }

        if (m_data->fbo == nullptr) {
            QGLFramebufferObjectFormat format;
            format.setSamples(4);
            format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

            m_data->fbo      = new QGLFramebufferObject(rect.size(), format);
            m_data->fboDirty = true;
        }

        if (m_data->fboDirty) {
            QPainter fboPainter(m_data->fbo);
            fboPainter.scale(pixelRatio, pixelRatio);
            draw(&fboPainter);
            fboPainter.end();

            m_data->fboDirty = false;
        }

        /*
            Why do we have this strange translation - but, anyway
            QwtPlotGLCanvas in combination with scaling factor
            is not very likely to happen as using QwtPlotOpenGLCanvas
            usually makes more sense then.
         */

        QGLFramebufferObject::blitFramebuffer(nullptr, rect.translated(0, height() - rect.height()), m_data->fbo, rect);
    } else {
        painter.begin(this);
        draw(&painter);
    }

    if (hasFocusIndicator)
        drawFocusIndicator(&painter);
}

//! No operation - reserved for some potential use in the future
void QwtPlotGLCanvas::resizeGL(int, int)
{
    // nothing to do
}
