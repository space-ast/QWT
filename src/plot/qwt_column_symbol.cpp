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

#include "qwt_column_symbol.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpalette.h>

/**
 * @brief draw a box
 * __
 * |*|
 * |*|
 *
 * @param p
 * @param rect
 * @param pen
 * @param brush
 */
static void qwtDrawBox(QPainter* p, const QRectF& rect, const QPen& pen, const QBrush& brush)
{
    if (rect.isValid()) {
        // A valid rectangle has a width() > 0 and height() > 0
        p->fillRect(rect, brush);
        if (rect.width() > 2 * pen.widthF()) {
            // 保证线宽不要超过矩形的宽度或者高度的一半
            p->setPen(pen);
            p->drawRect(rect);
        }
    } else {
        // 矩形不合法，那么有可能图被缩放的过小，导致宽或者高为0
        // 仅矩形的宽度大于2倍线宽，才有绘制边线的意义
        if (qFuzzyIsNull(rect.width())) {
            // 没有宽度
            p->setPen(brush.color());
            p->drawLine(rect.topLeft(), rect.bottomLeft());
            return;
        }
        // 没有高度
        if (qFuzzyIsNull(rect.height())) {
            p->setPen(brush.color());
            p->drawLine(rect.topLeft(), rect.topRight());
            return;
        }
    }
}

static void qwtDrawPanel(QPainter* painter, const QRectF& rect, const QPen& pen, const QBrush& brush)
{
    qreal lw = pen.widthF();
    if (lw > 0.0) {
        if (rect.width() == 0.0) {
            painter->setPen(brush.color());
            painter->drawLine(rect.topLeft(), rect.bottomLeft());
            return;
        }

        if (rect.height() == 0.0) {
            painter->setPen(brush.color());
            painter->drawLine(rect.topLeft(), rect.topRight());
            return;
        }

        lw        = qwtMinF(lw, rect.height() / 2.0 - 1.0);
        qreal lw2 = qwtMinF(lw, rect.width() / 2.0 - 1.0);
        lw        = qwtMinF(lw, lw2);

        const QRectF outerRect = rect.adjusted(0, 0, 1, 1);
        const QRectF innerRect = outerRect.adjusted(lw, lw, -lw, -lw);

        QPolygonF lines[ 2 ];

        lines[ 0 ] += outerRect.bottomLeft();
        lines[ 0 ] += outerRect.topLeft();
        lines[ 0 ] += outerRect.topRight();
        lines[ 0 ] += innerRect.topRight();
        lines[ 0 ] += innerRect.topLeft();
        lines[ 0 ] += innerRect.bottomLeft();

        lines[ 1 ] += outerRect.topRight();
        lines[ 1 ] += outerRect.bottomRight();
        lines[ 1 ] += outerRect.bottomLeft();
        lines[ 1 ] += innerRect.bottomLeft();
        lines[ 1 ] += innerRect.bottomRight();
        lines[ 1 ] += innerRect.topRight();

        painter->setPen(Qt::NoPen);

        painter->setBrush(brush.color().lighter());
        painter->drawPolygon(lines[ 0 ]);
        painter->setBrush(brush.color().darker());
        painter->drawPolygon(lines[ 1 ]);
    }

    painter->fillRect(rect.adjusted(lw, lw, -lw + 1, -lw + 1), brush);
}

class QwtColumnSymbol::PrivateData
{
public:
    PrivateData() : style(QwtColumnSymbol::Box), frameStyle(QwtColumnSymbol::Plain)
    {
    }

    QwtColumnSymbol::Style style;
    QwtColumnSymbol::FrameStyle frameStyle;
    // add by qwt 2,you can set pen and brush to draw bar
    QPen pen;
    QBrush brush;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] style Style of the symbol
 * @sa setStyle(), style(), Style
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param[in] style 符号的样式
 * @sa setStyle(), style(), Style
 * \endif
 */
QwtColumnSymbol::QwtColumnSymbol(Style style)
{
    m_data        = new PrivateData();
    m_data->style = style;
    m_data->pen   = QPen(Qt::black, 1);
    m_data->brush = QBrush(Qt::gray);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtColumnSymbol::~QwtColumnSymbol()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Specify the symbol style
 * @param[in] style Style to set
 * @sa style(), setPalette()
 * \endif
 * \if CHINESE
 * @brief 指定符号样式
 * @param[in] style 要设置的样式
 * @sa style(), setPalette()
 * \endif
 */
void QwtColumnSymbol::setStyle(Style style)
{
    m_data->style = style;
}

/**
 * \if ENGLISH
 * @brief Return current symbol style
 * @return Current style
 * @sa setStyle()
 * \endif
 * \if CHINESE
 * @brief 返回当前符号样式
 * @return 当前样式
 * @sa setStyle()
 * \endif
 */
QwtColumnSymbol::Style QwtColumnSymbol::style() const
{
    return m_data->style;
}

/**
 * \if ENGLISH
 * @brief Specify the outline pen
 * @param[in] pen Pen to use for drawing the outline
 * \endif
 * \if CHINESE
 * @brief 指定轮廓画笔
 * @param[in] pen 用于绘制轮廓的画笔
 * \endif
 */
void QwtColumnSymbol::setPen(const QPen& pen)
{
    m_data->pen = pen;
}

/**
 * \if ENGLISH
 * @brief Return the outline pen
 * @return Current pen used for drawing the outline
 * \endif
 * \if CHINESE
 * @brief 返回轮廓画笔
 * @return 当前用于绘制轮廓的画笔
 * \endif
 */
QPen QwtColumnSymbol::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Specify the fill brush
 * @param[in] b Brush to use for filling the column
 * \endif
 * \if CHINESE
 * @brief 指定填充画刷
 * @param[in] b 用于填充柱的画刷
 * \endif
 */
void QwtColumnSymbol::setBrush(const QBrush& b)
{
    m_data->brush = b;
}

/**
 * \if ENGLISH
 * @brief Return the fill brush
 * @return Current brush used for filling the column
 * \endif
 * \if CHINESE
 * @brief 返回填充画刷
 * @return 当前用于填充柱的画刷
 * \endif
 */
QBrush QwtColumnSymbol::brush() const
{
    return m_data->brush;
}

/**
 * \if ENGLISH
 * @brief Set the frame style that is used for the Box style
 * @param[in] frameStyle Frame style to set
 * @sa frameStyle(), setLineWidth(), setStyle()
 * \endif
 * \if CHINESE
 * @brief 设置 Box 样式使用的框架样式
 * @param[in] frameStyle 要设置的框架样式
 * @sa frameStyle(), setLineWidth(), setStyle()
 * \endif
 */
void QwtColumnSymbol::setFrameStyle(FrameStyle frameStyle)
{
    m_data->frameStyle = frameStyle;
}

/**
 * \if ENGLISH
 * @brief Return current frame style that is used for the Box style
 * @return Current frame style
 * @sa setFrameStyle(), lineWidth(), setStyle()
 * \endif
 * \if CHINESE
 * @brief 返回 Box 样式使用的当前框架样式
 * @return 当前框架样式
 * @sa setFrameStyle(), lineWidth(), setStyle()
 * \endif
 */
QwtColumnSymbol::FrameStyle QwtColumnSymbol::frameStyle() const
{
    return m_data->frameStyle;
}

/**
 * \if ENGLISH
 * @brief Set the line width of the frame that is used for the Box style
 * @param[in] width Width of the frame line
 * @sa lineWidth(), setFrameStyle()
 * \endif
 * \if CHINESE
 * @brief 设置 Box 样式使用的框架线宽
 * @param[in] width 框架线的宽度
 * @sa lineWidth(), setFrameStyle()
 * \endif
 */
void QwtColumnSymbol::setLineWidth(int width)
{
    if (width < 0)
        width = 0;

    m_data->pen.setWidth(width);
}

/**
 * \if ENGLISH
 * @brief Return line width of the frame that is used for the Box style
 * @return Current line width
 * @sa setLineWidth(), frameStyle(), setStyle()
 * \endif
 * \if CHINESE
 * @brief 返回 Box 样式使用的框架线宽
 * @return 当前线宽
 * @sa setLineWidth(), frameStyle(), setStyle()
 * \endif
 */
int QwtColumnSymbol::lineWidth() const
{
    return m_data->pen.width();
}

/**
 * \if ENGLISH
 * @brief Draw the symbol depending on its style
 * @param[in] painter Painter to use for drawing
 * @param[in] rect Directed rectangle defining the column bounds
 * @sa drawBox()
 * \endif
 * \if CHINESE
 * @brief 根据样式绘制符号
 * @param[in] painter 用于绘制的画笔
 * @param[in] rect 定义柱边界的方向矩形
 * @sa drawBox()
 * \endif
 */
void QwtColumnSymbol::draw(QPainter* painter, const QwtColumnRect& rect) const
{
    painter->save();

    switch (m_data->style) {
    case QwtColumnSymbol::Box: {
        drawBox(painter, rect);
        break;
    }
    default:;
    }

    painter->restore();
}

void QwtColumnSymbol::drawBox(QPainter* painter, const QwtColumnRect& rect) const
{
    QRectF r = rect.toRect();
    if (QwtPainter::roundingAlignment(painter)) {
        r.setLeft(qRound(r.left()));
        r.setRight(qRound(r.right()));
        r.setTop(qRound(r.top()));
        r.setBottom(qRound(r.bottom()));
    }

    switch (m_data->frameStyle) {
    case QwtColumnSymbol::Raised: {
        qwtDrawPanel(painter, r, m_data->pen, m_data->brush);
        break;
    }
    case QwtColumnSymbol::Plain: {
        // qwtDrawBox(painter, r, m_data->palette, m_data->lineWidth);
        qwtDrawBox(painter, r, m_data->pen, m_data->brush);
        break;
    }
    default: {
        painter->fillRect(r.adjusted(0, 0, 1, 1), m_data->brush);
    }
    }
}

/**
 * \if ENGLISH
 * @brief Return a normalized QRect built from the intervals
 * @details Converts the horizontal and vertical intervals into a QRectF,
 *          handling exclude minimum/maximum flags appropriately.
 * @return Normalized rectangle
 * \endif
 * \if CHINESE
 * @brief 返回从区间构建的标准化 QRect
 * @details 将水平区间和垂直区间转换为 QRectF，
 *          并适当处理排除最小值/最大值标志。
 * @return 标准化的矩形
 * \endif
 */
QRectF QwtColumnRect::toRect() const
{
    QRectF r(hInterval.minValue(),
             vInterval.minValue(),
             hInterval.maxValue() - hInterval.minValue(),
             vInterval.maxValue() - vInterval.minValue());

    r = r.normalized();

    if (hInterval.borderFlags() & QwtInterval::ExcludeMinimum)
        r.adjust(1, 0, 0, 0);

    if (hInterval.borderFlags() & QwtInterval::ExcludeMaximum)
        r.adjust(0, 0, -1, 0);

    if (vInterval.borderFlags() & QwtInterval::ExcludeMinimum)
        r.adjust(0, 1, 0, 0);

    if (vInterval.borderFlags() & QwtInterval::ExcludeMaximum)
        r.adjust(0, 0, 0, -1);

    return r;
}
