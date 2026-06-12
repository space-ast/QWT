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
    QWT_DECLARE_PUBLIC(QwtPlotGLCanvas)
public:
    PrivateData(QwtPlotGLCanvas* p) : q_ptr(p), fboDirty(true), fbo(nullptr)
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
 * @brief Constructor
 * @param[in] plot Parent plot widget
 * @sa QwtPlot::setCanvas()
 */
QwtPlotGLCanvas::QwtPlotGLCanvas(QwtPlot* plot)
    : QGLWidget(QwtPlotGLCanvasFormat(), plot), QwtPlotAbstractGLCanvas(this), QWT_PIMPL_CONSTRUCT
{
    init();
}
/**
 * @brief Constructor
 * @param[in] format OpenGL rendering options
 * @param[in] plot Parent plot widget
 * @sa QwtPlot::setCanvas()
 */
QwtPlotGLCanvas::QwtPlotGLCanvas(const QGLFormat& format, QwtPlot* plot)
    : QGLWidget(format, plot), QwtPlotAbstractGLCanvas(this), QWT_PIMPL_CONSTRUCT
{
    init();
}

/**
 * @brief Destructor
 */
QwtPlotGLCanvas::~QwtPlotGLCanvas()
{
}

void QwtPlotGLCanvas::init()
{
#if 1
    setAttribute(Qt::WA_OpaquePaintEvent, true);
#endif
    setLineWidth(1);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::Box);
}

/*!
   Paint event

   @param event Paint event
   @sa QwtPlot::drawCanvas()
 */
void QwtPlotGLCanvas::paintEvent(QPaintEvent* event)
{
    QGLWidget::paintEvent(event);
}

/**
 * @brief Qt event handler for QEvent::PolishRequest and QEvent::StyleChange
 * @param[in] event Qt Event
 * @return See QGLWidget::event()
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
 * @brief Invalidate the paint cache and repaint the canvas
 * @sa invalidatePaintCache()
 */
void QwtPlotGLCanvas::replot()
{
    QwtPlotAbstractGLCanvas::replot();
}

/**
 * @brief Invalidate the internal backing store
 */
void QwtPlotGLCanvas::invalidateBackingStore()
{
    QWT_D(d);
    d->fboDirty = true;
}

void QwtPlotGLCanvas::clearBackingStore()
{
    QWT_D(d);
    delete d->fbo;
    d->fbo = nullptr;
}

/**
 * @brief Calculate the painter path for a styled or rounded border
 * @details When the canvas has no styled background or rounded borders
 *          the painter path is empty.
 * @param[in] rect Bounding rectangle of the canvas
 * @return Painter path, that can be used for clipping
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
    QWT_D(d);
    const bool hasFocusIndicator = hasFocus() && focusIndicator() == CanvasFocusIndicator;

    QPainter painter;

    if (testPaintAttribute(QwtPlotGLCanvas::BackingStore)) {
        const qreal pixelRatio = QwtPainter::devicePixelRatio(nullptr);
        const QRect rect(0, 0, width() * pixelRatio, height() * pixelRatio);

        if (hasFocusIndicator)
            painter.begin(this);

        if (d->fbo) {
            if (d->fbo->size() != rect.size()) {
                delete d->fbo;
                d->fbo = nullptr;
            }
        }

        if (d->fbo == nullptr) {
            QGLFramebufferObjectFormat format;
            format.setSamples(4);
            format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

            d->fbo      = new QGLFramebufferObject(rect.size(), format);
            d->fboDirty = true;
        }

        if (d->fboDirty) {
            QPainter fboPainter(d->fbo);
            fboPainter.scale(pixelRatio, pixelRatio);
            draw(&fboPainter);
            fboPainter.end();

            d->fboDirty = false;
        }

        /*
            Why do we have this strange translation - but, anyway
            QwtPlotGLCanvas in combination with scaling factor
            is not very likely to happen as using QwtPlotOpenGLCanvas
            usually makes more sense then.
         */

        QGLFramebufferObject::blitFramebuffer(nullptr, rect.translated(0, height() - rect.height()), d->fbo, rect);
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
