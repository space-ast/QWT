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
public:
    PrivateData() : focusIndicator(NoFocusIndicator), borderRadius(0)
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
 * \if ENGLISH
 * @brief Constructor
 * @param[in] canvasWidget plot canvas widget
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] canvasWidget 绘图画布部件
 * \endif
 */
QwtPlotAbstractCanvas::QwtPlotAbstractCanvas(QWidget* canvasWidget)
{
    m_data               = new PrivateData;
    m_data->canvasWidget = canvasWidget;

#ifndef QT_NO_CURSOR
    canvasWidget->setCursor(Qt::CrossCursor);
#endif
    canvasWidget->setAutoFillBackground(true);
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
QwtPlotAbstractCanvas::~QwtPlotAbstractCanvas()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Return parent plot widget
 * @return Parent plot widget
 * \endif
 *
 * \if CHINESE
 * @brief 返回父绘图部件
 * @return 父绘图部件
 * \endif
 */
QwtPlot* QwtPlotAbstractCanvas::plot()
{
    return qobject_cast< QwtPlot* >(m_data->canvasWidget->parent());
}

/**
 * \if ENGLISH
 * @brief Return parent plot widget
 * @return Parent plot widget
 * \endif
 *
 * \if CHINESE
 * @brief 返回父绘图部件
 * @return 父绘图部件
 * \endif
 */
const QwtPlot* QwtPlotAbstractCanvas::plot() const
{
    return qobject_cast< const QwtPlot* >(m_data->canvasWidget->parent());
}

/**
 * \if ENGLISH
 * @brief Set the focus indicator
 * @param[in] focusIndicator Focus indicator type
 * @sa FocusIndicator, focusIndicator()
 * \endif
 *
 * \if CHINESE
 * @brief 设置焦点指示器
 * @param[in] focusIndicator 焦点指示器类型
 * @sa FocusIndicator, focusIndicator()
 * \endif
 */
void QwtPlotAbstractCanvas::setFocusIndicator(FocusIndicator focusIndicator)
{
    m_data->focusIndicator = focusIndicator;
}

/**
 * \if ENGLISH
 * @brief Get the focus indicator
 * @return Focus indicator
 * @sa FocusIndicator, setFocusIndicator()
 * \endif
 *
 * \if CHINESE
 * @brief 获取焦点指示器
 * @return 焦点指示器
 * @sa FocusIndicator, setFocusIndicator()
 * \endif
 */
QwtPlotAbstractCanvas::FocusIndicator QwtPlotAbstractCanvas::focusIndicator() const
{
    return m_data->focusIndicator;
}

/*!
   Draw the focus indication
   \param painter Painter
 */
void QwtPlotAbstractCanvas::drawFocusIndicator(QPainter* painter)
{
    const int margin = 1;

    QRect focusRect = m_data->canvasWidget->contentsRect();
    focusRect.setRect(focusRect.x() + margin,
                      focusRect.y() + margin,
                      focusRect.width() - 2 * margin,
                      focusRect.height() - 2 * margin);

    QwtPainter::drawFocusRect(painter, m_data->canvasWidget, focusRect);
}

/**
 * \if ENGLISH
 * @brief Set the radius for the corners of the border frame
 * @param[in] radius Radius of a rounded corner
 * @sa borderRadius()
 * \endif
 *
 * \if CHINESE
 * @brief 设置边框框架角落的半径
 * @param[in] radius 圆角的半径
 * @sa borderRadius()
 * \endif
 */
void QwtPlotAbstractCanvas::setBorderRadius(double radius)
{
    m_data->borderRadius = qwtMaxF(0.0, radius);
}

/**
 * \if ENGLISH
 * @brief Get the radius for the corners of the border frame
 * @return Radius for the corners of the border frame
 * @sa setBorderRadius()
 * \endif
 *
 * \if CHINESE
 * @brief 获取边框框架角落的半径
 * @return 边框框架角落的半径
 * @sa setBorderRadius()
 * \endif
 */
double QwtPlotAbstractCanvas::borderRadius() const
{
    return m_data->borderRadius;
}

//! \return Path for the canvas border
QPainterPath QwtPlotAbstractCanvas::canvasBorderPath(const QRect& rect) const
{
    return qwtBorderPath(canvasWidget(), rect);
}

/*!
   Draw the border of the canvas
   \param painter Painter
 */
void QwtPlotAbstractCanvas::drawBorder(QPainter* painter)
{
    const QWidget* w = canvasWidget();

    if (m_data->borderRadius > 0) {
        const int frameWidth = w->property("frameWidth").toInt();
        if (frameWidth > 0) {
            const int frameShape  = w->property("frameShape").toInt();
            const int frameShadow = w->property("frameShadow").toInt();

            const QRectF frameRect = w->property("frameRect").toRect();

            QwtPainter::drawRoundedFrame(painter,
                                         frameRect,
                                         m_data->borderRadius,
                                         m_data->borderRadius,
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

        if (!m_data->styleSheet.hasBorder || m_data->styleSheet.borderPath.isEmpty()) {
            // We have no border with at least one rounded corner
            hackStyledBackground = false;
        }
    }

    QWidget* w = canvasWidget();

    if (hackStyledBackground) {
        painter->save();

        // paint background without border
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_data->styleSheet.background.brush);
        painter->setBrushOrigin(m_data->styleSheet.background.origin);
        painter->setClipPath(m_data->styleSheet.borderPath);
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

//!  \brief Draw the plot to the canvas
void QwtPlotAbstractCanvas::drawCanvas(QPainter* painter)
{
    QWidget* w = canvasWidget();

    painter->save();

    if (!m_data->styleSheet.borderPath.isEmpty()) {
        painter->setClipPath(m_data->styleSheet.borderPath, Qt::IntersectClip);
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
    QWidget* w = canvasWidget();

    if (!w->testAttribute(Qt::WA_StyledBackground))
        return;

    QwtStyleSheetRecorder recorder(w->size());

    QPainter painter(&recorder);

    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, w);

    painter.end();

    m_data->styleSheet.hasBorder   = !recorder.border.rectList.isEmpty();
    m_data->styleSheet.cornerRects = recorder.clipRects;

    if (recorder.background.path.isEmpty()) {
        if (!recorder.border.rectList.isEmpty()) {
            m_data->styleSheet.borderPath = qwtCombinePathList(w->rect(), recorder.border.pathList);
        }
    } else {
        m_data->styleSheet.borderPath        = recorder.background.path;
        m_data->styleSheet.background.brush  = recorder.background.brush;
        m_data->styleSheet.background.origin = recorder.background.origin;
    }
}

//! \return canvas widget
QWidget* QwtPlotAbstractCanvas::canvasWidget()
{
    return m_data->canvasWidget;
}

//! \return canvas widget
const QWidget* QwtPlotAbstractCanvas::canvasWidget() const
{
    return m_data->canvasWidget;
}

class QwtPlotAbstractGLCanvas::PrivateData
{
public:
    PrivateData() : frameStyle(QFrame::Panel | QFrame::Sunken), lineWidth(2), midLineWidth(0)
    {
    }

    QwtPlotAbstractGLCanvas::PaintAttributes paintAttributes;

    int frameStyle;
    int lineWidth;
    int midLineWidth;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] canvasWidget plot canvas widget
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] canvasWidget 绘图画布部件
 * \endif
 */
QwtPlotAbstractGLCanvas::QwtPlotAbstractGLCanvas(QWidget* canvasWidget) : QwtPlotAbstractCanvas(canvasWidget)
{
    m_data = new PrivateData;

    qwtUpdateContentsRect(frameWidth(), canvasWidget);
    m_data->paintAttributes = QwtPlotAbstractGLCanvas::BackingStore;
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
QwtPlotAbstractGLCanvas::~QwtPlotAbstractGLCanvas()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Changing the paint attributes
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 更改绘制属性
 * @param[in] attribute 绘制属性
 * @param[in] on 开/关
 * @sa testPaintAttribute()
 * \endif
 */
void QwtPlotAbstractGLCanvas::setPaintAttribute(PaintAttribute attribute, bool on)
{
    if (bool(m_data->paintAttributes & attribute) == on)
        return;

    if (on) {
        m_data->paintAttributes |= attribute;
    } else {
        m_data->paintAttributes &= ~attribute;

        if (attribute == BackingStore)
            clearBackingStore();
    }
}

/**
 * \if ENGLISH
 * @brief Test whether a paint attribute is enabled
 * @param[in] attribute Paint attribute
 * @return true, when attribute is enabled
 * @sa setPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试绘制属性是否启用
 * @param[in] attribute 绘制属性
 * @return 如果属性启用则返回 true
 * @sa setPaintAttribute()
 * \endif
 */
bool QwtPlotAbstractGLCanvas::testPaintAttribute(PaintAttribute attribute) const
{
    return m_data->paintAttributes & attribute;
}

/**
 * \if ENGLISH
 * @brief Set the frame style
 * @param[in] style The bitwise OR between a shape and a shadow
 * @sa frameStyle(), QFrame::setFrameStyle(), setFrameShadow(), setFrameShape()
 * \endif
 *
 * \if CHINESE
 * @brief 设置框架样式
 * @param[in] style 形状和阴影的按位或
 * @sa frameStyle(), QFrame::setFrameStyle(), setFrameShadow(), setFrameShape()
 * \endif
 */
void QwtPlotAbstractGLCanvas::setFrameStyle(int style)
{
    if (style != m_data->frameStyle) {
        m_data->frameStyle = style;
        qwtUpdateContentsRect(frameWidth(), canvasWidget());

        canvasWidget()->update();
    }
}

/**
 * \if ENGLISH
 * @brief Get the frame style
 * @return The bitwise OR between a frameShape() and a frameShadow()
 * @sa setFrameStyle(), QFrame::frameStyle()
 * \endif
 *
 * \if CHINESE
 * @brief 获取框架样式
 * @return frameShape() 和 frameShadow() 的按位或
 * @sa setFrameStyle(), QFrame::frameStyle()
 * \endif
 */
int QwtPlotAbstractGLCanvas::frameStyle() const
{
    return m_data->frameStyle;
}

/**
 * \if ENGLISH
 * @brief Set the frame shadow
 * @param[in] shadow Frame shadow
 * @sa frameShadow(), setFrameShape(), QFrame::setFrameShadow()
 * \endif
 *
 * \if CHINESE
 * @brief 设置框架阴影
 * @param[in] shadow 框架阴影
 * @sa frameShadow(), setFrameShape(), QFrame::setFrameShadow()
 * \endif
 */
void QwtPlotAbstractGLCanvas::setFrameShadow(QFrame::Shadow shadow)
{
    setFrameStyle((m_data->frameStyle & QFrame::Shape_Mask) | shadow);
}

/**
 * \if ENGLISH
 * @brief Get the frame shadow
 * @return Frame shadow
 * @sa setFrameShadow(), QFrame::setFrameShadow()
 * \endif
 *
 * \if CHINESE
 * @brief 获取框架阴影
 * @return 框架阴影
 * @sa setFrameShadow(), QFrame::setFrameShadow()
 * \endif
 */
QFrame::Shadow QwtPlotAbstractGLCanvas::frameShadow() const
{
    return (QFrame::Shadow)(m_data->frameStyle & QFrame::Shadow_Mask);
}

/**
 * \if ENGLISH
 * @brief Set the frame shape
 * @param[in] shape Frame shape
 * @sa frameShape(), setFrameShadow(), QFrame::frameShape()
 * \endif
 *
 * \if CHINESE
 * @brief 设置框架形状
 * @param[in] shape 框架形状
 * @sa frameShape(), setFrameShadow(), QFrame::frameShape()
 * \endif
 */
void QwtPlotAbstractGLCanvas::setFrameShape(QFrame::Shape shape)
{
    setFrameStyle((m_data->frameStyle & QFrame::Shadow_Mask) | shape);
}

/**
 * \if ENGLISH
 * @brief Get the frame shape
 * @return Frame shape
 * @sa setFrameShape(), QFrame::frameShape()
 * \endif
 *
 * \if CHINESE
 * @brief 获取框架形状
 * @return 框架形状
 * @sa setFrameShape(), QFrame::frameShape()
 * \endif
 */
QFrame::Shape QwtPlotAbstractGLCanvas::frameShape() const
{
    return (QFrame::Shape)(m_data->frameStyle & QFrame::Shape_Mask);
}

/**
 * \if ENGLISH
 * @brief Set the frame line width
 * @details The default line width is 2 pixels.
 * @param[in] width Line width of the frame
 * @sa lineWidth(), setMidLineWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 设置框架线宽度
 * @details 默认线宽度为 2 像素。
 * @param[in] width 框架的线宽度
 * @sa lineWidth(), setMidLineWidth()
 * \endif
 */
void QwtPlotAbstractGLCanvas::setLineWidth(int width)
{
    width = qMax(width, 0);
    if (width != m_data->lineWidth) {
        m_data->lineWidth = qMax(width, 0);
        qwtUpdateContentsRect(frameWidth(), canvasWidget());
        canvasWidget()->update();
    }
}

/**
 * \if ENGLISH
 * @brief Get the line width of the frame
 * @return Line width of the frame
 * @sa setLineWidth(), midLineWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 获取框架的线宽度
 * @return 框架的线宽度
 * @sa setLineWidth(), midLineWidth()
 * \endif
 */
int QwtPlotAbstractGLCanvas::lineWidth() const
{
    return m_data->lineWidth;
}

/**
 * \if ENGLISH
 * @brief Set the frame mid line width
 * @details The default midline width is 0 pixels.
 * @param[in] width Midline width of the frame
 * @sa midLineWidth(), setLineWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 设置框架中线宽度
 * @details 默认中线宽度为 0 像素。
 * @param[in] width 框架的中线宽度
 * @sa midLineWidth(), setLineWidth()
 * \endif
 */
void QwtPlotAbstractGLCanvas::setMidLineWidth(int width)
{
    width = qMax(width, 0);
    if (width != m_data->midLineWidth) {
        m_data->midLineWidth = width;
        qwtUpdateContentsRect(frameWidth(), canvasWidget());
        canvasWidget()->update();
    }
}

/**
 * \if ENGLISH
 * @brief Get the midline width of the frame
 * @return Midline width of the frame
 * @sa setMidLineWidth(), lineWidth()
 * \endif
 *
 * \if CHINESE
 * @brief 获取框架的中线宽度
 * @return 框架的中线宽度
 * @sa setMidLineWidth(), lineWidth()
 * \endif
 */
int QwtPlotAbstractGLCanvas::midLineWidth() const
{
    return m_data->midLineWidth;
}

/**
 * \if ENGLISH
 * @brief Get the frame width depending on the style, line width and midline width
 * @return Frame width depending on the style, line width and midline width
 * \endif
 *
 * \if CHINESE
 * @brief 获取框架宽度，取决于样式、线宽和中线宽度
 * @return 框架宽度
 * \endif
 */
int QwtPlotAbstractGLCanvas::frameWidth() const
{
    return (frameStyle() != QFrame::NoFrame) ? m_data->lineWidth : 0;
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
 * \if ENGLISH
 * @brief Get the rectangle where the frame is drawn in
 * @return The rectangle where the frame is drawn in
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘制框架的矩形区域
 * @return 绘制框架的矩形区域
 * \endif
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
