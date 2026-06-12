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

#include "qwt_text.h"
#include "qwt_painter.h"
#include "qwt_text_engine.h"
#include "qwt_math.h"

#include <qmap.h>
#include <qfont.h>
#include <qcolor.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>

#if QT_VERSION >= 0x050200

static QwtText qwtStringToText(const QString& text)
{
    return QwtText(text);
}

#endif

namespace
{
static const struct RegisterQwtText
{
    inline RegisterQwtText()
    {
        qRegisterMetaType< QwtText >();

#if QT_VERSION >= 0x050200
        QMetaType::registerConverter< QString, QwtText >(qwtStringToText);
#endif
    }

} qwtRegisterQwtText;
}

namespace
{
class TextEngineDict
{
public:
    static TextEngineDict& dict();

    void setTextEngine(QwtText::TextFormat, QwtTextEngine*);

    const QwtTextEngine* textEngine(QwtText::TextFormat) const;
    const QwtTextEngine* textEngine(const QString&, QwtText::TextFormat) const;

private:
    TextEngineDict();
    ~TextEngineDict();

    using EngineMap = QMap< int, QwtTextEngine* >;

    inline const QwtTextEngine* engine(EngineMap::const_iterator& it) const
    {
        return it.value();
    }

    EngineMap m_map;
};

TextEngineDict& TextEngineDict::dict()
{
    static TextEngineDict engineDict;
    return engineDict;
}

TextEngineDict::TextEngineDict()
{
    m_map.insert(QwtText::PlainText, new QwtPlainTextEngine());
#ifndef QT_NO_RICHTEXT
    m_map.insert(QwtText::RichText, new QwtRichTextEngine());
#endif
}

TextEngineDict::~TextEngineDict()
{
    for (EngineMap::const_iterator it = m_map.constBegin(); it != m_map.constEnd(); ++it) {
        const QwtTextEngine* textEngine = engine(it);
        delete textEngine;
    }
}

const QwtTextEngine* TextEngineDict::textEngine(const QString& text, QwtText::TextFormat format) const
{
    if (format == QwtText::AutoText) {
        for (EngineMap::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
            if (it.key() != QwtText::PlainText) {
                const QwtTextEngine* e = engine(it);
                if (e && e->mightRender(text))
                    return e;
            }
        }
    }

    EngineMap::const_iterator it = m_map.find(format);
    if (it != m_map.end()) {
        const QwtTextEngine* e = engine(it);
        if (e)
            return e;
    }

    it = m_map.find(QwtText::PlainText);
    return engine(it);
}

void TextEngineDict::setTextEngine(QwtText::TextFormat format, QwtTextEngine* engine)
{
    if (format == QwtText::AutoText)
        return;

    if (format == QwtText::PlainText && engine == nullptr)
        return;

    EngineMap::const_iterator it = m_map.constFind(format);
    if (it != m_map.constEnd()) {
        delete this->engine(it);
        m_map.remove(format);
    }

    if (engine != nullptr)
        m_map.insert(format, engine);
}

const QwtTextEngine* TextEngineDict::textEngine(QwtText::TextFormat format) const
{
    const QwtTextEngine* e = nullptr;

    EngineMap::const_iterator it = m_map.find(format);
    if (it != m_map.end())
        e = engine(it);

    return e;
}
}

class QwtText::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtText)
public:
    PrivateData(QwtText* p)
        : q_ptr(p), renderFlags(Qt::AlignCenter), borderRadius(0), borderPen(Qt::NoPen), backgroundBrush(Qt::NoBrush), textEngine(nullptr)
    {
    }

    int renderFlags;
    QString text;
    QFont font;
    QColor color;
    double borderRadius;
    QPen borderPen;
    QBrush backgroundBrush;
    QwtText::TextFormat format;  ///< add by czy,record text format

    QwtText::PaintAttributes paintAttributes;
    QwtText::LayoutAttributes layoutAttributes;

    const QwtTextEngine* textEngine;
};

class QwtText::LayoutCache
{
public:
    void invalidate()
    {
        textSize = QSizeF();
    }

    QFont font;
    QSizeF textSize;
};

/**
 * @brief Default constructor
 *
 */
QwtText::QwtText()
    : QWT_PIMPL_CONSTRUCT
{
    m_data->textEngine = textEngine(m_data->text, PlainText);

    m_layoutCache = new LayoutCache;
}

/**
 * @brief Constructor
 *
 * @param text Text content
 * @param textFormat Text format
 *
 */
QwtText::QwtText(const QString& text, QwtText::TextFormat textFormat)
    : QWT_PIMPL_CONSTRUCT
{
    m_data->text       = text;
    m_data->format     = textFormat;
    m_data->textEngine = textEngine(text, textFormat);

    m_layoutCache = new LayoutCache;
}

/**
 * @brief Copy constructor
 * @param other Other QwtText to copy from
 *
 */
QwtText::QwtText(const QwtText& other)
    : QWT_PIMPL_CONSTRUCT
{
    *m_data = *other.m_data;

    m_layoutCache  = new LayoutCache;
    *m_layoutCache = *other.m_layoutCache;
}

/**
 * @brief Move constructor
 * @param other Other QwtText to move from
 *
 */
QwtText::QwtText(QwtText&& other) noexcept
    : m_data(std::move(other.m_data))
    , m_layoutCache(other.m_layoutCache)
{
    other.m_layoutCache = nullptr;
}

/**
 * @brief Destructor
 *
 */
QwtText::~QwtText()
{
    delete m_layoutCache;
}

/**
 * @brief Assignment operator
 * @param other Other QwtText to assign from
 * @return Reference to this QwtText
 *
 */
QwtText& QwtText::operator=(const QwtText& other)
{
    QWT_D(d);
    const PrivateData* od = other.d_func();
    *d             = *od;
    *m_layoutCache = *other.m_layoutCache;
    return *this;
}

/**
 * @brief Move assignment operator
 * @param other Other QwtText to move from
 * @return Reference to this QwtText
 *
 */
QwtText& QwtText::operator=(QwtText&& other) noexcept
{
    if (this != &other)
    {
        m_data = std::move(other.m_data);
        delete m_layoutCache;
        m_layoutCache       = other.m_layoutCache;
        other.m_layoutCache = nullptr;
    }
    return *this;
}

/**
 * @brief Equality operator
 * @param other Other QwtText to compare with
 * @return true if equal
 *
 */
bool QwtText::operator==(const QwtText& other) const
{
    QWT_DC(d);
    const PrivateData* od = other.d_func();
    return d->renderFlags == od->renderFlags && d->text == od->text
           && d->font == od->font && d->color == od->color
           && d->borderRadius == od->borderRadius && d->borderPen == od->borderPen
           && d->backgroundBrush == od->backgroundBrush
           && d->paintAttributes == od->paintAttributes && d->textEngine == od->textEngine;
}

/**
 * @brief Inequality operator
 * @param other Other QwtText to compare with
 * @return true if not equal
 *
 */
bool QwtText::operator!=(const QwtText& other) const
{
    return !(other == *this);
}

/**
 * @brief Assign a new text content
 *
 * @param text Text content
 * @param textFormat Text format
 *
 * @sa text()
 *
 */
void QwtText::setText(const QString& text, QwtText::TextFormat textFormat)
{
    QWT_D(d);
    d->text       = text;
    d->format     = textFormat;
    d->textEngine = textEngine(text, textFormat);
    m_layoutCache->invalidate();
}

/**
 * @brief Get text as QString
 * @return Text as QString
 * @sa setText()
 *
 */
QString QwtText::text() const
{
    QWT_DC(d);
    return d->text;
}

/**
 * @brief Change the render flags
 *
 * The default setting is Qt::AlignCenter
 *
 * @param renderFlags Bitwise OR of the flags used like in QPainter::drawText()
 *
 * @sa renderFlags(), QwtTextEngine::draw()
 * @note Some renderFlags might have no effect, depending on the text format.
 *
 */
void QwtText::setRenderFlags(int renderFlags)
{
    QWT_D(d);
    if (renderFlags != d->renderFlags) {
        d->renderFlags = renderFlags;
        m_layoutCache->invalidate();
    }
}

/**
 * @brief Get render flags
 * @return Render flags
 * @sa setRenderFlags()
 *
 */
int QwtText::renderFlags() const
{
    QWT_DC(d);
    return d->renderFlags;
}

/**
 * @brief Set the font
 * @param font Font to set
 * @note Setting the font might have no effect, when the text contains control sequences for setting fonts.
 * @sa font(), usedFont()
 *
 */
void QwtText::setFont(const QFont& font)
{
    QWT_D(d);
    d->font = font;
    setPaintAttribute(PaintUsingTextFont);
}

/**
 * @brief Get the font
 * @return Current font
 * @sa setFont(), usedFont()
 *
 */
QFont QwtText::font() const
{
    QWT_DC(d);
    return d->font;
}

/**
 * @brief Return the font of the text, if it has one, otherwise return defaultFont
 * @param defaultFont Default font to use if text has no font
 * @return Font used for drawing the text
 * @sa setFont(), font(), PaintAttributes
 *
 */
QFont QwtText::usedFont(const QFont& defaultFont) const
{
    QWT_DC(d);
    if (d->paintAttributes & PaintUsingTextFont)
        return d->font;

    return defaultFont;
}

/**
 * @brief Return the format of the text
 * @return The format of the text
 * @note This function was introduced in QWT7.0
 *
 */
QwtText::TextFormat QwtText::format() const
{
    QWT_DC(d);
    return d->format;
}

/**
 * @brief Set the pen color used for drawing the text
 * @param color Color to set
 * @note Setting the color might have no effect, when the text contains control sequences for setting colors.
 * @sa color(), usedColor()
 *
 */
void QwtText::setColor(const QColor& color)
{
    QWT_D(d);
    d->color = color;
    setPaintAttribute(PaintUsingTextColor);
}

/**
 * @brief Get the pen color used for painting the text
 * @return Current color
 * @sa setColor(), usedColor()
 *
 */
QColor QwtText::color() const
{
    QWT_DC(d);
    return d->color;
}

/**
 * @brief Return the color of the text, if it has one, otherwise return defaultColor
 * @param defaultColor Default color to use if text has no color
 * @return Color used for drawing the text
 * @sa setColor(), color(), PaintAttributes
 *
 */
QColor QwtText::usedColor(const QColor& defaultColor) const
{
    QWT_DC(d);
    if (d->paintAttributes & PaintUsingTextColor)
        return d->color;

    return defaultColor;
}

/**
 * @brief Set the radius for the corners of the border frame
 * @param radius Radius of a rounded corner
 * @sa borderRadius(), setBorderPen(), setBackgroundBrush()
 *
 */
void QwtText::setBorderRadius(double radius)
{
    QWT_D(d);
    d->borderRadius = qwtMaxF(0.0, radius);
}

/**
 * @brief Get the radius for the corners of the border frame
 * @return Radius for the corners of the border frame
 * @sa setBorderRadius(), borderPen(), backgroundBrush()
 *
 */
double QwtText::borderRadius() const
{
    QWT_DC(d);
    return d->borderRadius;
}

/**
 * @brief Set the background pen (border pen)
 * @param pen Background pen
 * @sa borderPen(), setBackgroundBrush()
 *
 */
void QwtText::setBorderPen(const QPen& pen)
{
    QWT_D(d);
    d->borderPen = pen;
    setPaintAttribute(PaintBackground);
}

/**
 * @brief Get the background pen (border pen)
 * @return Background pen
 * @sa setBorderPen(), backgroundBrush()
 *
 */
QPen QwtText::borderPen() const
{
    QWT_DC(d);
    return d->borderPen;
}

/**
 * @brief Set the background brush
 * @param brush Background brush
 * @sa backgroundBrush(), setBorderPen()
 *
 */
void QwtText::setBackgroundBrush(const QBrush& brush)
{
    QWT_D(d);
    d->backgroundBrush = brush;
    setPaintAttribute(PaintBackground);
}

/**
 * @brief Get the background brush
 * @return Background brush
 * @sa setBackgroundBrush(), borderPen()
 *
 */
QBrush QwtText::backgroundBrush() const
{
    QWT_DC(d);
    return d->backgroundBrush;
}

/**
 * @brief Change a paint attribute
 * @param attribute Paint attribute to change
 * @param on On/Off
 * @note Used by setFont(), setColor(), setBorderPen() and setBackgroundBrush()
 * @sa testPaintAttribute()
 *
 */
void QwtText::setPaintAttribute(PaintAttribute attribute, bool on)
{
    QWT_D(d);
    if (on)
        d->paintAttributes |= attribute;
    else
        d->paintAttributes &= ~attribute;
}

/**
 * @brief Test a paint attribute
 * @param attribute Paint attribute to test
 * @return true, if attribute is enabled
 * @sa setPaintAttribute()
 *
 */
bool QwtText::testPaintAttribute(PaintAttribute attribute) const
{
    QWT_DC(d);
    return d->paintAttributes & attribute;
}

/**
 * @brief Change a layout attribute
 * @param attribute Layout attribute to change
 * @param on On/Off
 * @sa testLayoutAttribute()
 *
 */
void QwtText::setLayoutAttribute(LayoutAttribute attribute, bool on)
{
    QWT_D(d);
    if (on)
        d->layoutAttributes |= attribute;
    else
        d->layoutAttributes &= ~attribute;
}

/**
 * @brief Test a layout attribute
 * @param attribute Layout attribute to test
 * @return true, if attribute is enabled
 * @sa setLayoutAttribute()
 *
 */
bool QwtText::testLayoutAttribute(LayoutAttribute attribute) const
{
    QWT_DC(d);
    return d->layoutAttributes | attribute;
}

/**
 * @brief Find the height for a given width
 * @param width Width
 * @return Calculated height
 * @sa heightForWidth(double, const QFont&)
 *
 */
double QwtText::heightForWidth(double width) const
{
    return heightForWidth(width, QFont());
}

/**
 * @brief Find the height for a given width
 * @param width Width
 * @param defaultFont Font used for the calculation if the text has no font
 * @return Calculated height
 * @sa heightForWidth(double)
 *
 */
double QwtText::heightForWidth(double width, const QFont& defaultFont) const
{
    QWT_DC(d);
    // We want to calculate in screen metrics. So
    // we need a font that uses screen metrics

    const QFont font = QwtPainter::scaledFont(usedFont(defaultFont));

    double h = 0;

    if (d->layoutAttributes & MinimumLayout) {
        double left, right, top, bottom;
        d->textEngine->textMargins(font, d->text, left, right, top, bottom);

        h = d->textEngine->heightForWidth(font, d->renderFlags, d->text, width + left + right);

        h -= top + bottom;
    } else {
        h = d->textEngine->heightForWidth(font, d->renderFlags, d->text, width);
    }

    return h;
}

/**
 * @brief Returns the size that is needed to render text
 * @return Calculated size
 * @sa textSize(const QFont&)
 *
 */
QSizeF QwtText::textSize() const
{
    return textSize(QFont());
}

/**
 * @brief Returns the size that is needed to render text
 * @param defaultFont Font of the text
 * @return Calculated size
 * @sa textSize()
 *
 */
QSizeF QwtText::textSize(const QFont& defaultFont) const
{
    QWT_DC(d);
    // We want to calculate in screen metrics. So
    // we need a font that uses screen metrics

    const QFont font = QwtPainter::scaledFont(usedFont(defaultFont));

    if (!m_layoutCache->textSize.isValid() || m_layoutCache->font != font) {
        m_layoutCache->textSize = d->textEngine->textSize(font, d->renderFlags, d->text);
        m_layoutCache->font     = font;
    }

    QSizeF sz = m_layoutCache->textSize;

    if (d->layoutAttributes & MinimumLayout) {
        double left, right, top, bottom;
        d->textEngine->textMargins(font, d->text, left, right, top, bottom);
        sz -= QSizeF(left + right, top + bottom);
    }

    return sz;
}

/**
 * @brief Draw a text into a rectangle
 * @param painter Painter
 * @param rect Rectangle to draw text into
 * @sa textSize()
 *
 */
void QwtText::draw(QPainter* painter, const QRectF& rect) const
{
    QWT_DC(d);
    if (d->paintAttributes & PaintBackground) {
        if (d->borderPen != Qt::NoPen || d->backgroundBrush != Qt::NoBrush) {
            painter->save();

            painter->setPen(d->borderPen);
            painter->setBrush(d->backgroundBrush);

            if (d->borderRadius == 0) {
                QwtPainter::drawRect(painter, rect);
            } else {
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(rect, d->borderRadius, d->borderRadius);
            }

            painter->restore();
        }
    }

    painter->save();

    if (d->paintAttributes & PaintUsingTextFont) {
        painter->setFont(d->font);
    }

    if (d->paintAttributes & PaintUsingTextColor) {
        if (d->color.isValid())
            painter->setPen(d->color);
    }

    QRectF expandedRect = rect;
    if (d->layoutAttributes & MinimumLayout) {
        // We want to calculate in screen metrics. So
        // we need a font that uses screen metrics

        const QFont font = QwtPainter::scaledFont(painter->font());

        double left, right, top, bottom;
        d->textEngine->textMargins(font, d->text, left, right, top, bottom);

        expandedRect.setTop(rect.top() - top);
        expandedRect.setBottom(rect.bottom() + bottom);
        expandedRect.setLeft(rect.left() - left);
        expandedRect.setRight(rect.right() + right);
    }

    d->textEngine->draw(painter, expandedRect, d->renderFlags, d->text);

    painter->restore();
}

/**
 * @brief Find the text engine for a text format
 * @details In case of QwtText::AutoText the first text engine (beside QwtPlainTextEngine) is returned,
 *          where QwtTextEngine::mightRender returns true. If there is none QwtPlainTextEngine is returned.
 *          If no text engine is registered for the format QwtPlainTextEngine is returned.
 * @param text Text, needed in case of AutoText
 * @param format Text format
 * @return Corresponding text engine
 * @sa textEngine(QwtText::TextFormat)
 *
 */
const QwtTextEngine* QwtText::textEngine(const QString& text, QwtText::TextFormat format)
{
    return TextEngineDict::dict().textEngine(text, format);
}

/**
 * @brief Assign/Replace a text engine for a text format
 * @details With setTextEngine it is possible to extend Qwt with other types of text formats.
 *          For QwtText::PlainText it is not allowed to assign a engine == nullptr.
 * @param format Text format
 * @param engine Text engine
 * @warning Using QwtText::AutoText does nothing.
 * @sa textEngine(QwtText::TextFormat)
 *
 */
void QwtText::setTextEngine(QwtText::TextFormat format, QwtTextEngine* engine)
{
    TextEngineDict::dict().setTextEngine(format, engine);
}

/**
 * @brief Find the text engine for a text format
 * @details textEngine can be used to find out if a text format is supported.
 * @param format Text format
 * @return The text engine, or nullptr if no engine is available.
 * @sa textEngine(const QString&, QwtText::TextFormat)
 *
 */
const QwtTextEngine* QwtText::textEngine(QwtText::TextFormat format)
{
    return TextEngineDict::dict().textEngine(format);
}

/**
 * @brief Check if text is null
 * @return text().isNull()
 *
 */
bool QwtText::isNull() const
{
    QWT_DC(d);
    return d->text.isNull();
}

/**
 * @brief Check if text is empty
 * @return text().isEmpty()
 *
 */
bool QwtText::isEmpty() const
{
    QWT_DC(d);
    return d->text.isEmpty();
}
