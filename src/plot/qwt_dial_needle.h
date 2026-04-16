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

#ifndef QWT_DIAL_NEEDLE_H
#define QWT_DIAL_NEEDLE_H

#include "qwt_global.h"
#include <qpalette.h>

class QPainter;

/**
 * \if ENGLISH
 * @brief Base class for needles that can be used in a QwtDial
 * @details QwtDialNeedle is a pointer that indicates a value by pointing
 *          to a specific direction.
 * \sa QwtDial, QwtCompass
 * \endif
 * \if CHINESE
 * @brief 可用于 QwtDial 的指针基类
 * @details QwtDialNeedle 是一个指针，通过指向特定方向来指示值。
 * \sa QwtDial, QwtCompass
 * \endif
 */

class QWT_EXPORT QwtDialNeedle
{
  public:
    /// Constructor
    QwtDialNeedle();
    /// Destructor
    virtual ~QwtDialNeedle();

    /// Set the palette for the needle
    virtual void setPalette( const QPalette& );
    /// Return the palette of the needle
    const QPalette& palette() const;

    /// Draw the needle
    virtual void draw( QPainter*, const QPointF& center,
        double length, double direction,
        QPalette::ColorGroup = QPalette::Active ) const;

  protected:
    /*!
       \brief Draw the needle

       The origin of the needle is at position (0.0, 0.0 )
       pointing in direction 0.0 ( = east ).

       The painter is already initialized with translation and
       rotation.

       \param painter Painter
       \param length Length of the needle
       \param colorGroup Color group, used for painting

       \sa setPalette(), palette()
     */
    virtual void drawNeedle( QPainter* painter,
        double length, QPalette::ColorGroup colorGroup ) const = 0;

    virtual void drawKnob( QPainter*, double width,
        const QBrush&, bool sunken ) const;

  private:
    Q_DISABLE_COPY(QwtDialNeedle)

    QPalette m_palette;
};

/**
 * \if ENGLISH
 * @brief A needle for dial widgets
 * @details The following colors are used:
 *          - QPalette::Mid: Pointer
 *          - QPalette::Base: Knob
 * \sa QwtDial, QwtCompass
 * \endif
 * \if CHINESE
 * @brief 表盘控件的指针
 * @details 使用以下颜色：
 *          - QPalette::Mid：指针
 *          - QPalette::Base：旋钮
 * \sa QwtDial, QwtCompass
 * \endif
 */

class QWT_EXPORT QwtDialSimpleNeedle : public QwtDialNeedle
{
  public:
    /**
     * \if ENGLISH
     * @brief Style of the needle
     * \endif
     * \if CHINESE
     * @brief 指针样式
     * \endif
     */
    enum Style
    {
        /**
         * \if ENGLISH
         * @brief Arrow style
         * \endif
         * \if CHINESE
         * @brief 箭头样式
         * \endif
         */
        Arrow,

        /**
         * \if ENGLISH
         * @brief A straight line from the center
         * \endif
         * \if CHINESE
         * @brief 从中心出发的直线
         * \endif
         */
        Ray
    };

    /// Constructor
    QwtDialSimpleNeedle( Style, bool hasKnob = true,
        const QColor& mid = Qt::gray, const QColor& base = Qt::darkGray );

    /// Set the width of the needle
    void setWidth( double width );
    /// Return the width of the needle
    double width() const;

  protected:
    virtual void drawNeedle( QPainter*, double length,
        QPalette::ColorGroup ) const override;

  private:
    Style m_style;
    bool m_hasKnob;
    double m_width;
};

/**
 * \if ENGLISH
 * @brief A magnet needle for compass widgets
 * @details A magnet needle points to two opposite directions indicating
 *          north and south.
 *          The following colors are used:
 *          - QPalette::Light: Used for pointing south
 *          - QPalette::Dark: Used for pointing north
 *          - QPalette::Base: Knob (ThinStyle only)
 * \sa QwtDial, QwtCompass
 * \endif
 * \if CHINESE
 * @brief 罗盘控件的磁针
 * @details 磁针指向两个相反的方向，指示北和南。
 *          使用以下颜色：
 *          - QPalette::Light：用于指向南
 *          - QPalette::Dark：用于指向北
 *          - QPalette::Base：旋钮（仅 ThinStyle）
 * \sa QwtDial, QwtCompass
 * \endif
 */

class QWT_EXPORT QwtCompassMagnetNeedle : public QwtDialNeedle
{
  public:
    /**
     * \if ENGLISH
     * @brief Style of the needle
     * \endif
     * \if CHINESE
     * @brief 指针样式
     * \endif
     */
    enum Style
    {
        /**
         * \if ENGLISH
         * @brief A needle with a triangular shape
         * \endif
         * \if CHINESE
         * @brief 三角形指针
         * \endif
         */
        TriangleStyle,

        /**
         * \if ENGLISH
         * @brief A thin needle
         * \endif
         * \if CHINESE
         * @brief 细指针
         * \endif
         */
        ThinStyle
    };

    /// Constructor
    QwtCompassMagnetNeedle( Style = TriangleStyle,
        const QColor& light = Qt::white, const QColor& dark = Qt::red );

  protected:
    virtual void drawNeedle( QPainter*,
        double length, QPalette::ColorGroup ) const override;

  private:
    Style m_style;
};

/**
 * \if ENGLISH
 * @brief An indicator for the wind direction
 * @details QwtCompassWindArrow shows the direction where the wind comes from.
 *          The following colors are used:
 *          - QPalette::Light: Used for Style1, or the light half of Style2
 *          - QPalette::Dark: Used for the dark half of Style2
 * \sa QwtDial, QwtCompass
 * \endif
 * \if CHINESE
 * @brief 风向指示器
 * @details QwtCompassWindArrow 显示风向的来源方向。
 *          使用以下颜色：
 *          - QPalette::Light：用于 Style1 或 Style2 的亮半部分
 *          - QPalette::Dark：用于 Style2 的暗半部分
 * \sa QwtDial, QwtCompass
 * \endif
 */

class QWT_EXPORT QwtCompassWindArrow : public QwtDialNeedle
{
  public:
    /**
     * \if ENGLISH
     * @brief Style of the arrow
     * \endif
     * \if CHINESE
     * @brief 箭头样式
     * \endif
     */
    enum Style
    {
        /**
         * \if ENGLISH
         * @brief A needle pointing to the center
         * \endif
         * \if CHINESE
         * @brief 指向中心的指针
         * \endif
         */
        Style1,

        /**
         * \if ENGLISH
         * @brief A needle pointing to the center
         * \endif
         * \if CHINESE
         * @brief 指向中心的指针
         * \endif
         */
        Style2
    };

    /// Constructor
    QwtCompassWindArrow( Style, const QColor& light = Qt::white,
        const QColor& dark = Qt::gray );

  protected:
    virtual void drawNeedle( QPainter*,
        double length, QPalette::ColorGroup ) const override;

  private:
    Style m_style;
};

#endif
