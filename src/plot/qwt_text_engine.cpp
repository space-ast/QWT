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

#include "qwt_text_engine.h"
#include "qwt_painter.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qmap.h>
#include <qwidget.h>
#include <qtextobject.h>
#include <qtextdocument.h>
#include <qabstracttextdocumentlayout.h>

static QString taggedRichText( const QString& text, int flags )
{
    QString richText = text;

    // By default QSimpleRichText is Qt::AlignLeft
    if ( flags & Qt::AlignJustify )
    {
        richText.prepend( QLatin1String( "<div align=\"justify\">" ) );
        richText.append( QLatin1String ( "</div>" ) );
    }
    else if ( flags & Qt::AlignRight )
    {
        richText.prepend( QLatin1String ( "<div align=\"right\">" ) );
        richText.append( QLatin1String ( "</div>" ) );
    }
    else if ( flags & Qt::AlignHCenter )
    {
        richText.prepend( QLatin1String ( "<div align=\"center\">" ) );
        richText.append( QLatin1String ( "</div>" ) );
    }

    return richText;
}

namespace
{
    class QwtRichTextDocument : public QTextDocument
    {
      public:
        QwtRichTextDocument( const QString& text, int flags, const QFont& font )
        {
            setUndoRedoEnabled( false );
            setDefaultFont( font );
            setHtml( text );

            // make sure we have a document layout
            ( void )documentLayout();

            QTextOption option = defaultTextOption();
            if ( flags & Qt::TextWordWrap )
                option.setWrapMode( QTextOption::WordWrap );
            else
                option.setWrapMode( QTextOption::NoWrap );

            option.setAlignment( static_cast< Qt::Alignment >( flags ) );
            setDefaultTextOption( option );

            QTextFrame* root = rootFrame();
            QTextFrameFormat fm = root->frameFormat();
            fm.setBorder( 0 );
            fm.setMargin( 0 );
            fm.setPadding( 0 );
            fm.setBottomMargin( 0 );
            fm.setLeftMargin( 0 );
            root->setFrameFormat( fm );

            adjustSize();
        }
    };
}

class QwtPlainTextEngine::PrivateData
{
  public:
    int effectiveAscent( const QFont& font ) const
    {
        const QString fontKey = font.key();

        QMap< QString, int >::const_iterator it =
            m_ascentCache.constFind( fontKey );

        if ( it != m_ascentCache.constEnd() )
            return *it;

        const int ascent = findAscent( font );
        m_ascentCache.insert( fontKey, ascent );

        return ascent;
    }

  private:
    static int findAscent( const QFont& font )
    {
        static const QString dummy( "E" );
        static const QColor white( Qt::white );

        const QFontMetrics fm( font );

        QPixmap pm( QwtPainter::horizontalAdvance( fm, dummy ), fm.height() );
        pm.fill( white );

        QPainter p( &pm );
        p.setFont( font );
        p.drawText( 0, 0,  pm.width(), pm.height(), 0, dummy );
        p.end();

        const QImage img = pm.toImage();

        int row = 0;
        for ( row = 0; row < img.height(); row++ )
        {
            const QRgb* line = reinterpret_cast< const QRgb* >(
                img.scanLine( row ) );

            const int w = pm.width();
            for ( int col = 0; col < w; col++ )
            {
                if ( line[col] != white.rgb() )
                    return fm.ascent() - row + 1;
            }
        }

        return fm.ascent();
    }

    mutable QMap< QString, int > m_ascentCache;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtTextEngine::QwtTextEngine()
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtTextEngine::~QwtTextEngine()
{
}

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtPlainTextEngine::QwtPlainTextEngine()
{
    m_data = new PrivateData;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPlainTextEngine::~QwtPlainTextEngine()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Find the height for a given width
 * @param font Font of the text
 * @param flags Bitwise OR of the flags used like in QPainter::drawText
 * @param text Text to be rendered
 * @param width Width
 * @return Calculated height
 * \sa textSize()
 * \endif
 * \if CHINESE
 * @brief 计算给定宽度的高度
 * @param font 文本字体
 * @param flags 标志的按位或，如 QPainter::drawText 中使用
 * @param text 要渲染的文本
 * @param width 宽度
 * @return 计算的高度
 * \sa textSize()
 * \endif
 */
double QwtPlainTextEngine::heightForWidth( const QFont& font, int flags,
    const QString& text, double width ) const
{
    const QFontMetricsF fm( font );
    const QRectF rect = fm.boundingRect(
        QRectF( 0, 0, width, QWIDGETSIZE_MAX ), flags, text );

    return rect.height();
}

/**
 * \if ENGLISH
 * @brief Return the size needed to render text
 * @param font Font of the text
 * @param flags Bitwise OR of the flags used like in QPainter::drawText
 * @param text Text to be rendered
 * @return Calculated size
 * \sa heightForWidth()
 * \endif
 * \if CHINESE
 * @brief 返回渲染文本所需的尺寸
 * @param font 文本字体
 * @param flags 标志的按位或，如 QPainter::drawText 中使用
 * @param text 要渲染的文本
 * @return 计算的尺寸
 * \sa heightForWidth()
 * \endif
 */
QSizeF QwtPlainTextEngine::textSize( const QFont& font,
    int flags, const QString& text ) const
{
    const QFontMetricsF fm( font );
    const QRectF rect = fm.boundingRect(
        QRectF( 0, 0, QWIDGETSIZE_MAX, QWIDGETSIZE_MAX ), flags, text );

    return rect.size();
}

/**
 * \if ENGLISH
 * @brief Return margins around the texts
 * @param font Font of the text
 * @param left Return 0
 * @param right Return 0
 * @param top Return value for the top margin
 * @param bottom Return value for the bottom margin
 * \sa textMargins()
 * \endif
 * \if CHINESE
 * @brief 返回文本周围的边距
 * @param font 文本字体
 * @param left 返回 0
 * @param right 返回 0
 * @param top 上边距返回值
 * @param bottom 下边距返回值
 * \sa textMargins()
 * \endif
 */
void QwtPlainTextEngine::textMargins( const QFont& font, const QString&,
    double& left, double& right, double& top, double& bottom ) const
{
    left = right = top = 0;

    const QFontMetricsF fm( font );
    top = fm.ascent() - m_data->effectiveAscent( font );
    bottom = fm.descent();
}

/**
 * \if ENGLISH
 * @brief Draw the text in a clipping rectangle
 * @details A wrapper for QPainter::drawText.
 * @param painter Painter
 * @param rect Clipping rectangle
 * @param flags Bitwise OR of the flags used like in QPainter::drawText
 * @param text Text to be rendered
 * \sa draw()
 * \endif
 * \if CHINESE
 * @brief 在裁剪矩形中绘制文本
 * @details QPainter::drawText 的包装器。
 * @param painter 绘制器
 * @param rect 裁剪矩形
 * @param flags 标志的按位或，如 QPainter::drawText 中使用
 * @param text 要渲染的文本
 * \sa draw()
 * \endif
 */
void QwtPlainTextEngine::draw( QPainter* painter, const QRectF& rect,
    int flags, const QString& text ) const
{
    QwtPainter::drawText( painter, rect, flags, text );
}

/**
 * \if ENGLISH
 * @brief Test if a string can be rendered by this text engine
 * @return Always true. All texts can be rendered by QwtPlainTextEngine
 * \sa mightRender()
 * \endif
 * \if CHINESE
 * @brief 测试字符串是否可以被此文本引擎渲染
 * @return 总是返回 true。所有文本都可以被 QwtPlainTextEngine 渲染
 * \sa mightRender()
 * \endif
 */
bool QwtPlainTextEngine::mightRender( const QString& ) const
{
    return true;
}

#ifndef QT_NO_RICHTEXT

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtRichTextEngine::QwtRichTextEngine()
{
}

/**
 * \if ENGLISH
 * @brief Find the height for a given width
 * @param font Font of the text
 * @param flags Bitwise OR of the flags used like in QPainter::drawText()
 * @param text Text to be rendered
 * @param width Width
 * @return Calculated height
 * \sa textSize()
 * \endif
 * \if CHINESE
 * @brief 计算给定宽度的高度
 * @param font 文本字体
 * @param flags 标志的按位或，如 QPainter::drawText() 中使用
 * @param text 要渲染的文本
 * @param width 宽度
 * @return 计算的高度
 * \sa textSize()
 * \endif
 */
double QwtRichTextEngine::heightForWidth( const QFont& font, int flags,
    const QString& text, double width ) const
{
    QwtRichTextDocument doc( text, flags, font );

    doc.setPageSize( QSizeF( width, QWIDGETSIZE_MAX ) );
    return doc.documentLayout()->documentSize().height();
}

/**
 * \if ENGLISH
 * @brief Return the size needed to render text
 * @param font Font of the text
 * @param flags Bitwise OR of the flags used like in QPainter::drawText()
 * @param text Text to be rendered
 * @return Calculated size
 * \sa heightForWidth()
 * \endif
 * \if CHINESE
 * @brief 返回渲染文本所需的尺寸
 * @param font 文本字体
 * @param flags 标志的按位或，如 QPainter::drawText() 中使用
 * @param text 要渲染的文本
 * @return 计算的尺寸
 * \sa heightForWidth()
 * \endif
 */
QSizeF QwtRichTextEngine::textSize( const QFont& font,
    int flags, const QString& text ) const
{
    QwtRichTextDocument doc( text, flags, font );

    QTextOption option = doc.defaultTextOption();
    if ( option.wrapMode() != QTextOption::NoWrap )
    {
        option.setWrapMode( QTextOption::NoWrap );
        doc.setDefaultTextOption( option );
        doc.adjustSize();
    }

    return doc.size();
}

/**
 * \if ENGLISH
 * @brief Draw the text in a clipping rectangle
 * @param painter Painter
 * @param rect Clipping rectangle
 * @param flags Bitwise OR of the flags like in QPainter::drawText()
 * @param text Text to be rendered
 * \sa draw()
 * \endif
 * \if CHINESE
 * @brief 在裁剪矩形中绘制文本
 * @param painter 绘制器
 * @param rect 裁剪矩形
 * @param flags 标志的按位或，如 QPainter::drawText() 中使用
 * @param text 要渲染的文本
 * \sa draw()
 * \endif
 */
void QwtRichTextEngine::draw( QPainter* painter, const QRectF& rect,
    int flags, const QString& text ) const
{
    QwtRichTextDocument doc( text, flags, painter->font() );
    QwtPainter::drawSimpleRichText( painter, rect, flags, doc );
}

/**
 * \if ENGLISH
 * @brief Wrap text into &lt;div align=...&gt; &lt;/div&gt; tags according to flags
 * @param text Text
 * @param flags Bitwise OR of the flags like in QPainter::drawText()
 * @return Tagged text
 * \sa taggedRichText()
 * \endif
 * \if CHINESE
 * @brief 根据 flags 将文本包装到 &lt;div align=...&gt; &lt;/div&gt; 标签中
 * @param text 文本
 * @param flags 标志的按位或，如 QPainter::drawText() 中使用
 * @return 带标签的文本
 * \sa taggedRichText()
 * \endif
 */
QString QwtRichTextEngine::taggedText( const QString& text, int flags ) const
{
    return taggedRichText( text, flags );
}

/**
 * \if ENGLISH
 * @brief Test if a string can be rendered by this text engine
 * @param text Text to be tested
 * @return Qt::mightBeRichText(text)
 * \sa mightRender()
 * \endif
 * \if CHINESE
 * @brief 测试字符串是否可以被此文本引擎渲染
 * @param text 要测试的文本
 * @return Qt::mightBeRichText(text)
 * \sa mightRender()
 * \endif
 */
bool QwtRichTextEngine::mightRender( const QString& text ) const
{
    return Qt::mightBeRichText( text );
}

/**
 * \if ENGLISH
 * @brief Return margins around the texts
 * @param left Return 0
 * @param right Return 0
 * @param top Return 0
 * @param bottom Return 0
 * \sa textMargins()
 * \endif
 * \if CHINESE
 * @brief 返回文本周围的边距
 * @param left 返回 0
 * @param right 返回 0
 * @param top 返回 0
 * @param bottom 返回 0
 * \sa textMargins()
 * \endif
 */
void QwtRichTextEngine::textMargins( const QFont&, const QString&,
    double& left, double& right, double& top, double& bottom ) const
{
    left = right = top = bottom = 0;
}

#endif // !QT_NO_RICHTEXT
