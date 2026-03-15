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

#include "qwt_text_label.h"
#include "qwt_text.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmargins.h>

class QwtTextLabel::PrivateData
{
public:
    PrivateData() : indent(4), margin(0)
    {
    }

    int indent;
    int margin;
    QwtText text;
};

/**
 * \if ENGLISH
 * @brief Constructs an empty label
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief 构造一个空标签
 * @param parent 父控件
 * \endif
 */
QwtTextLabel::QwtTextLabel(QWidget* parent) : QFrame(parent)
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructs a label that displays the text
 * @param parent Parent widget
 * @param text Text to display
 * \endif
 * \if CHINESE
 * @brief 构造一个显示文本的标签
 * @param parent 父控件
 * @param text 要显示的文本
 * \endif
 */
QwtTextLabel::QwtTextLabel(const QwtText& text, QWidget* parent) : QFrame(parent)
{
    init();
    m_data->text = text;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtTextLabel::~QwtTextLabel()
{
    delete m_data;
}

void QwtTextLabel::init()
{
    m_data = new PrivateData();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

/**
 * \if ENGLISH
 * @brief Interface for the designer plugin - does the same as setText()
 * @param text Plain text to set
 * \sa plainText()
 * \endif
 * \if CHINESE
 * @brief 设计师插件接口 - 与 setText() 功能相同
 * @param text 要设置的纯文本
 * \sa plainText()
 * \endif
 */
void QwtTextLabel::setPlainText(const QString& text)
{
    setText(QwtText(text));
}

/**
 * \if ENGLISH
 * @brief Interface for the designer plugin
 * @return Text as plain text
 * \sa setPlainText(), text()
 * \endif
 * \if CHINESE
 * @brief 设计师插件接口
 * @return 纯文本形式的文本
 * \sa setPlainText(), text()
 * \endif
 */
QString QwtTextLabel::plainText() const
{
    return m_data->text.text();
}

/**
 * \if ENGLISH
 * @brief Change the label's text, keeping all other QwtText attributes
 * @param text New text
 * @param textFormat Format of text
 * \sa QwtText
 * \endif
 * \if CHINESE
 * @brief 更改标签的文本，保留所有其他 QwtText 属性
 * @param text 新文本
 * @param textFormat 文本格式
 * \sa QwtText
 * \endif
 */
void QwtTextLabel::setText(const QString& text, QwtText::TextFormat textFormat)
{
    m_data->text.setText(text, textFormat);

    update();
    updateGeometry();
}

/**
 * \if ENGLISH
 * @brief Change the label's text
 * @param text New text
 * \sa setText(), clear()
 * \endif
 * \if CHINESE
 * @brief 更改标签的文本
 * @param text 新文本
 * \sa setText(), clear()
 * \endif
 */
void QwtTextLabel::setText(const QwtText& text)
{
    m_data->text = text;

    update();
    updateGeometry();
}

/**
 * \if ENGLISH
 * @brief Return the text
 * @return Current text
 * \sa setText(), plainText()
 * \endif
 * \if CHINESE
 * @brief 返回文本
 * @return 当前文本
 * \sa setText(), plainText()
 * \endif
 */
const QwtText& QwtTextLabel::text() const
{
    return m_data->text;
}

/**
 * \if ENGLISH
 * @brief Clear the text and all QwtText attributes
 * \sa setText()
 * \endif
 * \if CHINESE
 * @brief 清除文本和所有 QwtText 属性
 * \sa setText()
 * \endif
 */
void QwtTextLabel::clear()
{
    m_data->text = QwtText();

    update();
    updateGeometry();
}

/**
 * \if ENGLISH
 * @brief Return label's text indent in pixels
 * @return Indent in pixels
 * \sa setIndent()
 * \endif
 * \if CHINESE
 * @brief 返回标签的文本缩进（像素）
 * @return 缩进像素数
 * \sa setIndent()
 * \endif
 */
int QwtTextLabel::indent() const
{
    return m_data->indent;
}

/**
 * \if ENGLISH
 * @brief Set label's text indent in pixels
 * @param indent Indentation in pixels
 * \sa indent()
 * \endif
 * \if CHINESE
 * @brief 设置标签的文本缩进（像素）
 * @param indent 缩进像素数
 * \sa indent()
 * \endif
 */
void QwtTextLabel::setIndent(int indent)
{
    if (indent < 0)
        indent = 0;

    m_data->indent = indent;

    update();
    updateGeometry();
}

/**
 * \if ENGLISH
 * @brief Return label's margin in pixels
 * @return Margin in pixels
 * \sa setMargin()
 * \endif
 * \if CHINESE
 * @brief 返回标签的边距（像素）
 * @return 边距像素数
 * \sa setMargin()
 * \endif
 */
int QwtTextLabel::margin() const
{
    return m_data->margin;
}

/**
 * \if ENGLISH
 * @brief Set label's margin in pixels
 * @param margin Margin in pixels
 * \sa margin()
 * \endif
 * \if CHINESE
 * @brief 设置标签的边距（像素）
 * @param margin 边距像素数
 * \sa margin()
 * \endif
 */
void QwtTextLabel::setMargin(int margin)
{
    m_data->margin = margin;

    update();
    updateGeometry();
}

/**
 * \if ENGLISH
 * @brief Return a size hint
 * @return Size hint
 * \sa minimumSizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回大小提示
 * @return 大小提示
 * \sa minimumSizeHint()
 * \endif
 */
QSize QwtTextLabel::sizeHint() const
{
    return minimumSizeHint();
}

/**
 * \if ENGLISH
 * @brief Return a minimum size hint
 * @return Minimum size hint
 * \sa sizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回最小大小提示
 * @return 最小大小提示
 * \sa sizeHint()
 * \endif
 */
QSize QwtTextLabel::minimumSizeHint() const
{
    QSizeF sz = m_data->text.textSize(font());

    const QMargins m = contentsMargins();

    int mw = m.left() + m.right() + 2 * m_data->margin;
    int mh = m.top() + m.bottom() + 2 * m_data->margin;

    int indent = m_data->indent;
    if (indent <= 0)
        indent = defaultIndent();

    if (indent > 0) {
        const int align = m_data->text.renderFlags();
        if (align & Qt::AlignLeft || align & Qt::AlignRight)
            mw += m_data->indent;
        else if (align & Qt::AlignTop || align & Qt::AlignBottom)
            mh += m_data->indent;
    }

    sz += QSizeF(mw, mh);

    return QSize(qwtCeil(sz.width()), qwtCeil(sz.height()));
}

/**
 * \if ENGLISH
 * @brief Return the preferred height for a given width
 * @param width Given width
 * @return Preferred height for this widget, given the width
 * \sa sizeHint(), minimumSizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回给定宽度的首选高度
 * @param width 给定宽度
 * @return 给定宽度下控件的首选高度
 * \sa sizeHint(), minimumSizeHint()
 * \endif
 */
int QwtTextLabel::heightForWidth(int width) const
{
    const int renderFlags = m_data->text.renderFlags();

    int indent = m_data->indent;
    if (indent <= 0)
        indent = defaultIndent();

    const QMargins m = contentsMargins();

    width -= m.left() + m.right() - 2 * m_data->margin;
    if (renderFlags & Qt::AlignLeft || renderFlags & Qt::AlignRight)
        width -= indent;

    int height = qwtCeil(m_data->text.heightForWidth(width, font()));
    if ((renderFlags & Qt::AlignTop) || (renderFlags & Qt::AlignBottom))
        height += indent;

    height += m.top() + m.bottom() + 2 * m_data->margin;

    return height;
}

/**
 * \if ENGLISH
 * @brief Qt paint event handler
 * @param event Paint event
 * \sa drawContents(), paintEvent()
 * \endif
 * \if CHINESE
 * @brief Qt 绘制事件处理器
 * @param event 绘制事件
 * \sa drawContents(), paintEvent()
 * \endif
 */
void QwtTextLabel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (!contentsRect().contains(event->rect())) {
        painter.setClipRegion(event->region() & frameRect());
        drawFrame(&painter);
    }

    painter.setClipRegion(event->region() & contentsRect());

    drawContents(&painter);
}

//! Redraw the text and focus indicator
void QwtTextLabel::drawContents(QPainter* painter)
{
    const QRect r = textRect();
    if (r.isEmpty())
        return;

    painter->setFont(font());
    painter->setPen(palette().color(QPalette::Active, QPalette::Text));

    drawText(painter, QRectF(r));

    if (hasFocus()) {
        const int m = 2;

        QRect focusRect = contentsRect().adjusted(m, m, -m + 1, -m + 1);

        QwtPainter::drawFocusRect(painter, this, focusRect);
    }
}

/**
 * \if ENGLISH
 * @brief Redraw the text
 * @param painter Painter used for drawing
 * @param textRect Rectangle where to draw the text
 * \sa drawContents()
 * \endif
 * \if CHINESE
 * @brief 重绘文本
 * @param painter 用于绘制的绘制器
 * @param textRect 绘制文本的矩形区域
 * \sa drawContents()
 * \endif
 */
void QwtTextLabel::drawText(QPainter* painter, const QRectF& textRect)
{
    m_data->text.draw(painter, textRect);
}

/**
 * \if ENGLISH
 * @brief Calculate geometry for the text in widget coordinates
 * @return Geometry for the text
 * \sa drawText(), drawContents()
 * \endif
 * \if CHINESE
 * @brief 计算控件坐标中文本的几何区域
 * @return 文本的几何区域
 * \sa drawText(), drawContents()
 * \endif
 */
QRect QwtTextLabel::textRect() const
{
    QRect r = contentsRect();

    if (!r.isEmpty() && m_data->margin > 0) {
        const int m = m_data->margin;
        r.adjust(m, m, -m, -m);
    }

    if (!r.isEmpty()) {
        int indent = m_data->indent;
        if (indent <= 0)
            indent = defaultIndent();

        if (indent > 0) {
            const int renderFlags = m_data->text.renderFlags();

            if (renderFlags & Qt::AlignLeft) {
                r.setX(r.x() + indent);
            } else if (renderFlags & Qt::AlignRight) {
                r.setWidth(r.width() - indent);
            } else if (renderFlags & Qt::AlignTop) {
                r.setY(r.y() + indent);
            } else if (renderFlags & Qt::AlignBottom) {
                r.setHeight(r.height() - indent);
            }
        }
    }

    return r;
}

/**
 * \if ENGLISH
 * @brief Calculate the default indent based on font
 * @return Default indent in pixels
 * \sa indent(), setIndent()
 * \endif
 * \if CHINESE
 * @brief 根据字体计算默认缩进
 * @return 默认缩进像素数
 * \sa indent(), setIndent()
 * \endif
 */
int QwtTextLabel::defaultIndent() const
{
    if (frameWidth() <= 0)
        return 0;

    QFont fnt;
    if (m_data->text.testPaintAttribute(QwtText::PaintUsingTextFont))
        fnt = m_data->text.font();
    else
        fnt = font();

    return QwtPainter::horizontalAdvance(QFontMetrics(fnt), 'x') / 2;
}
