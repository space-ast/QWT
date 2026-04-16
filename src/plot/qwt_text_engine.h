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

#ifndef QWT_TEXT_ENGINE_H
#define QWT_TEXT_ENGINE_H

#include "qwt_global.h"
#include <qsize.h>

class QFont;
class QRectF;
class QString;
class QPainter;

/**
 * \if ENGLISH
 * @brief Abstract base class for rendering text strings
 * @details A text engine is responsible for rendering texts for a
 *          specific text format. They are used by QwtText to render a text.
 * \sa QwtText::setTextEngine()
 * \endif
 * \if CHINESE
 * @brief 用于渲染文本字符串的抽象基类
 * @details 文本引擎负责渲染特定文本格式的文本。它们被 QwtText 用于渲染文本。
 * \sa QwtText::setTextEngine()
 * \endif
 */

class QWT_EXPORT QwtTextEngine
{
  public:
    // Virtual destructor
    virtual ~QwtTextEngine();

    /**
     * \if ENGLISH
     * @brief Find the height for a given width
     * @param font Font of the text
     * @param flags Bitwise OR of the flags used like in QPainter::drawText
     * @param text Text to be rendered
     * @param width Width
     * @return Calculated height
     * \endif
     * \if CHINESE
     * @brief 计算给定宽度的高度
     * @param font 文本字体
     * @param flags 标志的按位或，如 QPainter::drawText 中使用
     * @param text 要渲染的文本
     * @param width 宽度
     * @return 计算的高度
     * \endif
     */
    virtual double heightForWidth( const QFont& font, int flags,
        const QString& text, double width ) const = 0;

    /**
     * \if ENGLISH
     * @brief Return the size needed to render text
     * @param font Font of the text
     * @param flags Bitwise OR of the flags like in QPainter::drawText
     * @param text Text to be rendered
     * @return Calculated size
     * \endif
     * \if CHINESE
     * @brief 返回渲染文本所需的尺寸
     * @param font 文本字体
     * @param flags 标志的按位或，如 QPainter::drawText 中使用
     * @param text 要渲染的文本
     * @return 计算的尺寸
     * \endif
     */
    virtual QSizeF textSize( const QFont& font, int flags,
        const QString& text ) const = 0;

    /**
     * \if ENGLISH
     * @brief Test if a string can be rendered by this text engine
     * @param text Text to be tested
     * @return true, if it can be rendered
     * \endif
     * \if CHINESE
     * @brief 测试字符串是否可以被此文本引擎渲染
     * @param text 要测试的文本
     * @return 如果可以渲染返回 true
     * \endif
     */
    virtual bool mightRender( const QString& text ) const = 0;

    /**
     * \if ENGLISH
     * @brief Return margins around the texts
     * @details The textSize might include margins around the
     *          text, like QFontMetrics::descent(). In situations
     *          where texts need to be aligned in detail, knowing
     *          these margins might improve the layout calculations.
     * @param font Font of the text
     * @param text Text to be rendered
     * @param left Return value for the left margin
     * @param right Return value for the right margin
     * @param top Return value for the top margin
     * @param bottom Return value for the bottom margin
     * \endif
     * \if CHINESE
     * @brief 返回文本周围的边距
     * @details textSize 可能包括文本周围的边距，如 QFontMetrics::descent()。
     *          在需要详细对齐文本的情况下，了解这些边距可能会改进布局计算。
     * @param font 文本字体
     * @param text 要渲染的文本
     * @param left 左边距返回值
     * @param right 右边距返回值
     * @param top 上边距返回值
     * @param bottom 下边距返回值
     * \endif
     */
    virtual void textMargins( const QFont& font, const QString& text,
        double& left, double& right, double& top, double& bottom ) const = 0;

    /**
     * \if ENGLISH
     * @brief Draw the text in a clipping rectangle
     * @param painter Painter
     * @param rect Clipping rectangle
     * @param flags Bitwise OR of the flags like in QPainter::drawText()
     * @param text Text to be rendered
     * \endif
     * \if CHINESE
     * @brief 在裁剪矩形中绘制文本
     * @param painter 绘制器
     * @param rect 裁剪矩形
     * @param flags 标志的按位或，如 QPainter::drawText() 中使用
     * @param text 要渲染的文本
     * \endif
     */
    virtual void draw( QPainter* painter, const QRectF& rect,
        int flags, const QString& text ) const = 0;

  protected:
    /// Protected constructor
    QwtTextEngine();

  private:
    Q_DISABLE_COPY(QwtTextEngine)
};


/**
 * \if ENGLISH
 * @brief A text engine for plain texts
 * @details QwtPlainTextEngine renders texts using the basic Qt classes
 *          QPainter and QFontMetrics.
 * \sa QwtRichTextEngine
 * \endif
 * \if CHINESE
 * @brief 纯文本的文本引擎
 * @details QwtPlainTextEngine 使用基本 Qt 类 QPainter 和 QFontMetrics 渲染文本。
 * \sa QwtRichTextEngine
 * \endif
 */
class QWT_EXPORT QwtPlainTextEngine : public QwtTextEngine
{
  public:
    // Constructor
    QwtPlainTextEngine();
    // Destructor
    virtual ~QwtPlainTextEngine();

    // Calculate height for a given width
    virtual double heightForWidth( const QFont& font, int flags,
        const QString& text, double width ) const override;

    // Return the size needed to render text
    virtual QSizeF textSize( const QFont& font, int flags,
        const QString& text ) const override;

    // Draw the text
    virtual void draw( QPainter*, const QRectF& rect,
        int flags, const QString& text ) const override;

    // Test if a string can be rendered
    virtual bool mightRender( const QString& ) const override;

    // Return text margins
    virtual void textMargins(
        const QFont&, const QString&,
        double& left, double& right,
        double& top, double& bottom ) const override;

  private:
    class PrivateData;
    PrivateData* m_data;
};


#ifndef QT_NO_RICHTEXT

/**
 * \if ENGLISH
 * @brief A text engine for Qt rich texts
 * @details QwtRichTextEngine renders Qt rich texts using the classes
 *          of the Scribe framework of Qt.
 * \sa QwtPlainTextEngine
 * \endif
 * \if CHINESE
 * @brief Qt 富文本的文本引擎
 * @details QwtRichTextEngine 使用 Qt 的 Scribe 框架类渲染 Qt 富文本。
 * \sa QwtPlainTextEngine
 * \endif
 */
class QWT_EXPORT QwtRichTextEngine : public QwtTextEngine
{
  public:
    // Constructor
    QwtRichTextEngine();

    // Calculate height for a given width
    virtual double heightForWidth( const QFont& font, int flags,
        const QString& text, double width ) const override;

    // Return the size needed to render text
    virtual QSizeF textSize( const QFont& font, int flags,
        const QString& text ) const override;

    // Draw the text
    virtual void draw( QPainter*, const QRectF& rect,
        int flags, const QString& text ) const override;

    // Test if a string can be rendered
    virtual bool mightRender( const QString& ) const override;

    // Return text margins
    virtual void textMargins(
        const QFont&, const QString&,
        double& left, double& right,
        double& top, double& bottom ) const override;

  private:
    QString taggedText( const QString&, int flags ) const;
};

#endif // !QT_NO_RICHTEXT

#endif
