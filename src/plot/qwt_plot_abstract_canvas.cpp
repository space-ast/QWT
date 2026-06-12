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
 *        - QwtGridRasterData (2-d table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 *****************************************************************************/

#include "qwt_plot_abstract_canvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"
#include "qwt_null_paintdevice.h"
#include "qwt_math.h"
#include "qwt_stylesheet_recorder.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qstyle.h>
#include <qstyleoption.h>

static void qwtUpdateContentsRect(int fw, QWidget* canvas)
{
    canvas->setContentsMargins(fw, fw, fw, fw);
}

static inline void qwtRevertPath(QPainterPath& path)
{
    if (path.elementCount() == 4) {
        QPainterPath::Element el0 = path.elementAt(0);
        QPainterPath::Element el3 = path.elementAt(3);

        path.setElementPositionAt(0, el3.x, el3.y);
        path.setElementPositionAt(3, el0.x, el0.y);
    }
}

static QPainterPath qwtCombinePathList(const QRectF& rect, const QList< QPainterPath >& pathList)
{
    if (pathList.isEmpty())
        return QPainterPath();

    QPainterPath ordered[ 8 ];  // starting top left

    for (int i = 0; i < pathList.size(); i++) {
        int index            = -1;
        QPainterPath subPath = pathList[ i ];

        const QRectF br = pathList[ i ].controlPointRect();
        if (br.center().x() < rect.center().x()) {
            if (br.center().y() < rect.center().y()) {
                if (qAbs(br.top() - rect.top()) < qAbs(br.left() - rect.left())) {
                    index = 1;
                } else {
                    index = 0;
                }
            } else {
                if (qAbs(br.bottom() - rect.bottom()) < qAbs(br.left() - rect.left())) {
                    index = 6;
                } else {
                    index = 7;
                }
            }

            if (subPath.currentPosition().y() > br.center().y())
                qwtRevertPath(subPath);
        } else {
            if (br.center().y() < rect.center().y()) {
                if (qAbs(br.top() - rect.top()) < qAbs(br.right() - rect.right())) {
                    index = 2;
                } else {
                    index = 3;
                }
            } else {
                if (qAbs(br.bottom() - rect.bottom()) < qAbs(br.right() - rect.right())) {
                    index = 5;
                } else {
                    index = 4;
                }
            }
            if (subPath.currentPosition().y() < br.center().y())
                qwtRevertPath(subPath);
        }
        ordered[ index ] = subPath;
    }

    for (int i = 0; i < 4; i++) {
        if (ordered[ 2 * i ].isEmpty() != ordered[ 2 * i + 1 ].isEmpty()) {
            // we don't accept incomplete rounded borders
            return QPainterPath();
        }
    }

    const QPolygonF corners(rect);

    QPainterPath path;
    // path.moveTo( rect.topLeft() );

    for (int i = 0; i < 4; i++) {
        if (ordered[ 2 * i ].isEmpty()) {
            path.lineTo(corners[ i ]);
        } else {
            path.connectPath(ordered[ 2 * i ]);
            path.connectPath(ordered[ 2 * i + 1 ]);
        }
    }

    path.closeSubpath();

#if 0
    return path.simplified();
#else
    return path;
#endif
}

static QPainterPath qwtBorderPath(const QWidget* canvas, const QRect& rect)
{
    if (canvas->testAttribute(Qt::WA_StyledBackground)) {
        QwtStyleSheetRecorder recorder(rect.size());

        QPainter painter(&recorder);

        QStyleOption opt;
        opt.initFrom(canvas);
        opt.rect = rect;
        canvas->style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, canvas);

        painter.end();

        if (!recorder.background.path.isEmpty())
            return recorder.background.path;

        if (!recorder.border.rectList.isEmpty())
            return qwtCombinePathList(rect, recorder.border.pathList);
    } else {
        const double borderRadius = canvas->property("borderRadius").toDouble();

        if (borderRadius > 0.0) {
            double fw2 = canvas->property("frameWidth").toInt() * 0.5;
            QRectF r   = QRectF(rect).adjusted(fw2, fw2, -fw2, -fw2);

            QPainterPath path;
            path.addRoundedRect(r, borderRadius, borderRadius);
            return path;
        }
    }

    return QPainterPath();
}

class QwtPlotAbstractCanvas::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotAbstractCanvas)
public:
    PrivateData(QwtPlotAbstractCanvas* p) : q_ptr(p), focusIndicator(NoFocusIndicator), borderRadius(0)
    {
        styleSheet.hasBorder = false;
    }

    FocusIndicator focusIndicator;
    double borderRadius;

    struct StyleSheet
    {
        bool hasBorder;
        QPainterPath borderPath;
        QVector< QRectF > cornerRects;

        struct StyleSheetBackground
        {
            QBrush brush;
            QPointF origin;
        } background;

    } styleSheet;

    QWidget* canvasWidget;
};

/**
 * @brief Constructor
 * @param[in] canvasWidget plot canvas widget
 */
QwtPlotAbstractCanvas::QwtPlotAbstractCanvas(QWidget* canvasWidget) : QWT_PIMPL_CONSTRUCT
{
    m_data->canvasWidget = canvasWidget;

#ifndef QT_NO_CURSOR
    canvasWidget->setCursor(Qt::CrossCursor);
#endif
    canvasWidget->setAutoFillBackground(true);
}

/**
 * @brief Destructor
 */
QwtPlotAbstractCanvas::~QwtPlotAbstractCanvas()
{
}

/**
 * @brief Return parent plot widget
 * @return Parent plot widget
 */
QwtPlot* QwtPlotAbstractCanvas::plot()
{
    QWT_D(d);
    return qobject_cast< QwtPlot* >(d->canvasWidget->parent());
}

/**
 * @brief Return parent plot widget
 * @return Parent plot widget
 */
const QwtPlot* QwtPlotAbstractCanvas::plot() const
{
    QWT_DC(d);
    return qobject_cast< const QwtPlot* >(d->canvasWidget->parent());
}

/**
 * @brief Set the focus indicator
 * @param[in] focusIndicator Focus indicator type
 * @sa FocusIndicator, focusIndicator()
 */
void QwtPlotAbstractCanvas::setFocusIndicator(FocusIndicator focusIndicator)
{
    QWT_D(d);
    d->focusIndicator = focusIndicator;
}

/**
 * @brief Get the focus indicator
 * @return Focus indicator
 * @sa FocusIndicator, setFocusIndicator()
 */
QwtPlotAbstractCanvas::FocusIndicator QwtPlotAbstractCanvas::focusIndicator() const
{
    QWT_DC(d);
    return d->focusIndicator;
}

/*!
   Draw the focus indication
   @param painter Painter
 */
void QwtPlotAbstractCanvas::drawFocusIndicator(QPainter* painter)
{
    QWT_D(d);

    const int margin = 1;

    QRect focusRect = d->canvasWidget->contentsRect();
    focusRect.setRect(focusRect.x() + margin,
                      focusRect.y() + margin,
                      focusRect.width() - 2 * margin,
                      focusRect.height() - 2 * margin);

    QwtPainter::drawFocusRect(painter, d->canvasWidget, focusRect);
}

/**
 * @brief Set the radius for the corners of the border frame
 * @param[in] radius Radius of a rounded corner
 * @sa borderRadius()
 */
void QwtPlotAbstractCanvas::setBorderRadius(double radius)
{
    QWT_D(d);
    d->borderRadius = qwtMaxF(0.0, radius);
}

/**
 * @brief Get the radius for the corners of the border frame
 * @return Radius for the corners of the border frame
 * @sa setBorderRadius()
 */
double QwtPlotAbstractCanvas::borderRadius() const
{
    QWT_DC(d);
    return d->borderRadius;
}

//! @return Path for the canvas border
QPainterPath QwtPlotAbstractCanvas::canvasBorderPath(const QRect& rect) const
{
    return qwtBorderPath(canvasWidget(), rect);
}

/*!
   Draw the border of the canvas
   @param painter Painter
 */
void QwtPlotAbstractCanvas::drawBorder(QPainter* painter)
{
    QWT_D(d);

    const QWidget* w = canvasWidget();

    if (d->borderRadius > 0) {
        const int frameWidth = w->property("frameWidth").toInt();
        if (frameWidth > 0) {
            const int frameShape  = w->property("frameShape").toInt();
            const int frameShadow = w->property("frameShadow").toInt();

            const QRectF frameRect = w->property("frameRect").toRect();

            QwtPainter::drawRoundedFrame(painter,
                                         frameRect,
                                         d->borderRadius,
                                         d->borderRadius,
                                         w->palette(),
                                         frameWidth,
                                         frameShape | frameShadow);
        }
    } else {
        const int frameShape  = w->property("frameShape").toInt();
        const int frameShadow = w->property("frameShadow").toInt();

#if QT_VERSION < 0x050000
        QStyleOptionFrameV3 opt;
#else
        QStyleOptionFrame opt;
#endif
        opt.initFrom(w);

        opt.frameShape = QFrame::Shape(int(opt.frameShape) | frameShape);

        switch (frameShape) {
        case QFrame::Box:
        case QFrame::HLine:
        case QFrame::VLine:
        case QFrame::StyledPanel:
        case QFrame::Panel: {
            opt.lineWidth    = w->property("lineWidth").toInt();
            opt.midLineWidth = w->property("midLineWidth").toInt();
            break;
        }
        default: {
            opt.lineWidth = w->property("frameWidth").toInt();
            break;
        }
        }

        if (frameShadow == QFrame::Sunken)
            opt.state |= QStyle::State_Sunken;
        else if (frameShadow == QFrame::Raised)
            opt.state |= QStyle::State_Raised;

        w->style()->drawControl(QStyle::CE_ShapedFrame, &opt, painter, w);
    }
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::drawBackground(QPainter* painter)
{
    QwtPainter::drawCanvasBackgound(painter, canvasWidget());
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::fillBackground(QPainter* painter)
{
    QwtPainter::fillBackground(painter, canvasWidget());
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::drawUnstyled(QPainter* painter)
{
    fillBackground(painter);

    QWidget* w = canvasWidget();

    if (w->autoFillBackground()) {
        const QRect canvasRect = w->rect();

        painter->save();

        painter->setPen(Qt::NoPen);
        painter->setBrush(w->palette().brush(w->backgroundRole()));

        const QRect frameRect = w->property("frameRect").toRect();
        if (borderRadius() > 0.0 && (canvasRect == frameRect)) {
            const int frameWidth = w->property("frameWidth").toInt();
            if (frameWidth > 0) {
                painter->setClipPath(canvasBorderPath(canvasRect));
                painter->drawRect(canvasRect);
            } else {
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawPath(canvasBorderPath(canvasRect));
            }
        } else {
            painter->drawRect(canvasRect);
        }

        painter->restore();
    }

    drawCanvas(painter);
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::drawStyled(QPainter* painter, bool hackStyledBackground)
{
    QWT_D(d);

    fillBackground(painter);

    if (hackStyledBackground) {
        // Antialiasing rounded borders is done by
        // inserting pixels with colors between the
        // border color and the color on the canvas,
        // When the border is painted before the plot items
        // these colors are interpolated for the canvas
        // and the plot items need to be clipped excluding
        // the antialiased pixels. In situations, where
        // the plot items fill the area at the rounded
        // borders this is noticeable.
        // The only way to avoid these annoying "artefacts"
        // is to paint the border on top of the plot items.

        if (!d->styleSheet.hasBorder || d->styleSheet.borderPath.isEmpty()) {
            // We have no border with at least one rounded corner
            hackStyledBackground = false;
        }
    }

    QWidget* w = canvasWidget();

    if (hackStyledBackground) {
        painter->save();

        // paint background without border
        painter->setPen(Qt::NoPen);
        painter->setBrush(d->styleSheet.background.brush);
        painter->setBrushOrigin(d->styleSheet.background.origin);
        painter->setClipPath(d->styleSheet.borderPath);
        painter->drawRect(w->contentsRect());

        painter->restore();

        drawCanvas(painter);

        // Now paint the border on top
        QStyleOptionFrame opt;
        opt.initFrom(w);
        w->style()->drawPrimitive(QStyle::PE_Frame, &opt, painter, w);
    } else {
        QStyleOption opt;
        opt.initFrom(w);
        w->style()->drawPrimitive(QStyle::PE_Widget, &opt, painter, w);

        drawCanvas(painter);
    }
}

//!  @brief Draw the plot to the canvas
void QwtPlotAbstractCanvas::drawCanvas(QPainter* painter)
{
    QWT_D(d);

    QWidget* w = canvasWidget();

    painter->save();

    if (!d->styleSheet.borderPath.isEmpty()) {
        painter->setClipPath(d->styleSheet.borderPath, Qt::IntersectClip);
    } else {
        if (borderRadius() > 0.0) {
            const QRect frameRect = w->property("frameRect").toRect();
            painter->setClipPath(canvasBorderPath(frameRect), Qt::IntersectClip);
        } else {
            painter->setClipRect(w->contentsRect(), Qt::IntersectClip);
        }
    }

    QwtPlot* plot = qobject_cast< QwtPlot* >(w->parent());
    if (plot)
        plot->drawCanvas(painter);

    painter->restore();
}

//! Update the cached information about the current style sheet
void QwtPlotAbstractCanvas::updateStyleSheetInfo()
{
    QWT_D(d);

    QWidget* w = canvasWidget();

    if (!w->testAttribute(Qt::WA_StyledBackground))
        return;

    QwtStyleSheetRecorder recorder(w->size());

    QPainter painter(&recorder);

    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, w);

    painter.end();

    d->styleSheet.hasBorder   = !recorder.border.rectList.isEmpty();
    d->styleSheet.cornerRects = recorder.clipRects;

    if (recorder.background.path.isEmpty()) {
        if (!recorder.border.rectList.isEmpty()) {
            d->styleSheet.borderPath = qwtCombinePathList(w->rect(), recorder.border.pathList);
        }
    } else {
        d->styleSheet.borderPath        = recorder.background.path;
        d->styleSheet.background.brush  = recorder.background.brush;
        d->styleSheet.background.origin = recorder.background.origin;
    }
}

//! @return canvas widget
QWidget* QwtPlotAbstractCanvas::canvasWidget()
{
    QWT_D(d);
    return d->canvasWidget;
}

//! @return canvas widget
const QWidget* QwtPlotAbstractCanvas::canvasWidget() const
{
    QWT_DC(d);
    return d->canvasWidget;
}

class QwtPlotAbstractGLCanvas::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotAbstractGLCanvas)
public:
    PrivateData(QwtPlotAbstractGLCanvas* p) : q_ptr(p), frameStyle(QFrame::Box | QFrame::Plain), lineWidth(1), midLineWidth(0)
    {
    }

    QwtPlotAbstractGLCanvas::PaintAttributes paintAttributes;

    int frameStyle;
    int lineWidth;
    int midLineWidth;
};

/**
 * @brief Constructor
 * @param[in] canvasWidget plot canvas widget
 */
QwtPlotAbstractGLCanvas::QwtPlotAbstractGLCanvas(QWidget* canvasWidget) : QwtPlotAbstractCanvas(canvasWidget), QWT_PIMPL_CONSTRUCT
{
    qwtUpdateContentsRect(frameWidth(), canvasWidget);
    m_data->paintAttributes = QwtPlotAbstractGLCanvas::BackingStore;
}

/**
 * @brief Destructor
 */
QwtPlotAbstractGLCanvas::~QwtPlotAbstractGLCanvas()
{
}

/**
 * @brief Changing the paint attributes
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 */
void QwtPlotAbstractGLCanvas::setPaintAttribute(PaintAttribute attribute, bool on)
{
    QWT_D(d);

    if (bool(d->paintAttributes & attribute) == on)
        return;

    if (on) {
        d->paintAttributes |= attribute;
    } else {
        d->paintAttributes &= ~attribute;

        if (attribute == BackingStore)
            clearBackingStore();
    }
}

/**
 * @brief Test whether a paint attribute is enabled
 * @param[in] attribute Paint attribute
 * @return true, when attribute is enabled
 * @sa setPaintAttribute()
 */
bool QwtPlotAbstractGLCanvas::testPaintAttribute(PaintAttribute attribute) const
{
    QWT_DC(d);
    return d->paintAttributes & attribute;
}

/**
 * @brief Set the frame style
 * @param[in] style The bitwise OR between a shape and a shadow
 * @sa frameStyle(), QFrame::setFrameStyle(), setFrameShadow(), setFrameShape()
 */
void QwtPlotAbstractGLCanvas::setFrameStyle(int style)
{
    QWT_D(d);

    if (style != d->frameStyle) {
        d->frameStyle = style;
        qwtUpdateContentsRect(frameWidth(), canvasWidget());

        canvasWidget()->update();
    }
}

/**
 * @brief Get the frame style
 * @return The bitwise OR between a frameShape() and a frameShadow()
 * @sa setFrameStyle(), QFrame::frameStyle()
 */
int QwtPlotAbstractGLCanvas::frameStyle() const
{
    QWT_DC(d);
    return d->frameStyle;
}

/**
 * @brief Set the frame shadow
 * @param[in] shadow Frame shadow
 * @sa frameShadow(), setFrameShape(), QFrame::setFrameShadow()
 */
void QwtPlotAbstractGLCanvas::setFrameShadow(QFrame::Shadow shadow)
{
    QWT_D(d);
    setFrameStyle((d->frameStyle & QFrame::Shape_Mask) | shadow);
}

/**
 * @brief Get the frame shadow
 * @return Frame shadow
 * @sa setFrameShadow(), QFrame::setFrameShadow()
 */
QFrame::Shadow QwtPlotAbstractGLCanvas::frameShadow() const
{
    QWT_DC(d);
    return (QFrame::Shadow)(d->frameStyle & QFrame::Shadow_Mask);
}

/**
 * @brief Set the frame shape
 * @param[in] shape Frame shape
 * @sa frameShape(), setFrameShadow(), QFrame::frameShape()
 */
void QwtPlotAbstractGLCanvas::setFrameShape(QFrame::Shape shape)
{
    QWT_D(d);
    setFrameStyle((d->frameStyle & QFrame::Shadow_Mask) | shape);
}

/**
 * @brief Get the frame shape
 * @return Frame shape
 * @sa setFrameShape(), QFrame::frameShape()
 */
QFrame::Shape QwtPlotAbstractGLCanvas::frameShape() const
{
    QWT_DC(d);
    return (QFrame::Shape)(d->frameStyle & QFrame::Shape_Mask);
}

/**
 * @brief Set the frame line width
 * @details The default line width is 2 pixels.
 * @param[in] width Line width of the frame
 * @sa lineWidth(), setMidLineWidth()
 */
void QwtPlotAbstractGLCanvas::setLineWidth(int width)
{
    QWT_D(d);

    width = qMax(width, 0);
    if (width != d->lineWidth) {
        d->lineWidth = qMax(width, 0);
        qwtUpdateContentsRect(frameWidth(), canvasWidget());
        canvasWidget()->update();
    }
}

/**
 * @brief Get the line width of the frame
 * @return Line width of the frame
 * @sa setLineWidth(), midLineWidth()
 */
int QwtPlotAbstractGLCanvas::lineWidth() const
{
    QWT_DC(d);
    return d->lineWidth;
}

/**
 * @brief Set the frame mid line width
 * @details The default midline width is 0 pixels.
 * @param[in] width Midline width of the frame
 * @sa midLineWidth(), setLineWidth()
 */
void QwtPlotAbstractGLCanvas::setMidLineWidth(int width)
{
    QWT_D(d);

    width = qMax(width, 0);
    if (width != d->midLineWidth) {
        d->midLineWidth = width;
        qwtUpdateContentsRect(frameWidth(), canvasWidget());
        canvasWidget()->update();
    }
}

/**
 * @brief Get the midline width of the frame
 * @return Midline width of the frame
 * @sa setMidLineWidth(), lineWidth()
 */
int QwtPlotAbstractGLCanvas::midLineWidth() const
{
    QWT_DC(d);
    return d->midLineWidth;
}

/**
 * @brief Get the frame width depending on the style, line width and midline width
 * @return Frame width depending on the style, line width and midline width
 */
int QwtPlotAbstractGLCanvas::frameWidth() const
{
    QWT_DC(d);
    return (frameStyle() != QFrame::NoFrame) ? d->lineWidth : 0;
}

/**
 * @brief Invalidate the paint cache and repaint the canvas
 * @sa invalidatePaintCache()
 */
void QwtPlotAbstractGLCanvas::replot()
{
    invalidateBackingStore();

    QWidget* w = canvasWidget();
    if (testPaintAttribute(QwtPlotAbstractGLCanvas::ImmediatePaint))
        w->repaint(w->contentsRect());
    else
        w->update(w->contentsRect());
}

/**
 * @brief Get the rectangle where the frame is drawn in
 * @return The rectangle where the frame is drawn in
 */
QRect QwtPlotAbstractGLCanvas::frameRect() const
{
    const int fw = frameWidth();
    return canvasWidget()->contentsRect().adjusted(-fw, -fw, fw, fw);
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractGLCanvas::draw(QPainter* painter)
{
#if FIX_GL_TRANSLATION
    if (painter->paintEngine()->type() == QPaintEngine::OpenGL2) {
        // work around a translation bug of QPaintEngine::OpenGL2
        painter->translate(1, 1);
    }
#endif

    if (canvasWidget()->testAttribute(Qt::WA_StyledBackground))
        drawStyled(painter, true);
    else
        drawUnstyled(painter);

    if (frameWidth() > 0)
        drawBorder(painter);
}
