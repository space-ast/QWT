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

#ifndef QWT_COLUMN_SYMBOL_H
#define QWT_COLUMN_SYMBOL_H

#include "qwt_global.h"
#include "qwt_interval.h"

#include <qnamespace.h>

class QPainter;
class QPalette;
class QRectF;

/**
 * \if ENGLISH
 * @brief Directed rectangle representing bounding rectangle and orientation of a column
 * \endif
 * \if CHINESE
 * @brief 表示柱的边界矩形和方向的方向矩形
 * \endif
 */
class QWT_EXPORT QwtColumnRect
{
public:
    //! \if ENGLISH Direction of the column \endif \if CHINESE 柱的方向 \endif
    enum Direction
    {
        //! \if ENGLISH From left to right \endif \if CHINESE 从左到右 \endif
        LeftToRight,

        //! \if ENGLISH From right to left \endif \if CHINESE 从右到左 \endif
        RightToLeft,

        //! \if ENGLISH From bottom to top \endif \if CHINESE 从下到上 \endif
        BottomToTop,

        //! \if ENGLISH From top to bottom \endif \if CHINESE 从上到下 \endif
        TopToBottom
    };

    /**
     * \if ENGLISH
     * @brief Build a rectangle with invalid intervals directed BottomToTop
     * \endif
     * \if CHINESE
     * @brief 构建一个具有无效区间、方向为 BottomToTop 的矩形
     * \endif
     */
    QwtColumnRect() : direction(BottomToTop)
    {
    }

    //! \if ENGLISH Return a normalized QRect built from the intervals \endif \if CHINESE 返回从区间构建的标准化 QRect \endif
    QRectF toRect() const;

    //! \if ENGLISH Return Orientation \endif \if CHINESE 返回方向 \endif
    Qt::Orientation orientation() const
    {
        if (direction == LeftToRight || direction == RightToLeft)
            return Qt::Horizontal;

        return Qt::Vertical;
    }

    //! \if ENGLISH Interval for the horizontal coordinates \endif \if CHINESE 水平坐标的区间 \endif
    QwtInterval hInterval;

    //! \if ENGLISH Interval for the vertical coordinates \endif \if CHINESE 垂直坐标的区间 \endif
    QwtInterval vInterval;

    //! \if ENGLISH Direction \endif \if CHINESE 方向 \endif
    Direction direction;
};

//! \if ENGLISH A drawing primitive for columns \endif \if CHINESE 柱的绘图基元 \endif
class QWT_EXPORT QwtColumnSymbol
{
public:
    /*!
       \if ENGLISH
       \brief Style
       \sa setStyle(), style()
       \endif
       \if CHINESE
       \brief 样式
       \sa setStyle(), style()
       \endif
     */
    enum Style
    {
        //! \if ENGLISH No Style, the symbol draws nothing \endif \if CHINESE 无样式，符号不绘制任何内容 \endif
        NoStyle = -1,

        /*!
           \if ENGLISH
           The column is painted with a frame depending on the frameStyle() and lineWidth() using the palette().
           \endif
           \if CHINESE
           柱根据 frameStyle() 和 lineWidth() 使用 palette() 绘制一个框架。
           \endif
         */
        Box,

        /*!
           \if ENGLISH
           Styles >= QwtColumnSymbol::UserStyle are reserved for derived classes of QwtColumnSymbol
           that overload draw() with additional application specific symbol types.
           \endif
           \if CHINESE
           样式 >= QwtColumnSymbol::UserStyle 保留给 QwtColumnSymbol 的派生类，
           这些类重载 draw() 以添加特定于应用程序的符号类型。
           \endif
         */
        UserStyle = 1000
    };

    /*!
       \if ENGLISH
       \brief Frame Style used in Box style()
       \sa Style, setFrameStyle(), frameStyle(), setStyle(), setPalette()
       \endif
       \if CHINESE
       \brief Box 样式中使用的框架样式
       \sa Style, setFrameStyle(), frameStyle(), setStyle(), setPalette()
       \endif
     */
    enum FrameStyle
    {
        //! \if ENGLISH No frame \endif \if CHINESE 无框架 \endif
        NoFrame,

        //! \if ENGLISH A plain frame style \endif \if CHINESE 普通框架样式 \endif
        Plain,

        //! \if ENGLISH A raised frame style \endif \if CHINESE 凸起框架样式 \endif
        Raised
    };

public:
    explicit QwtColumnSymbol(Style = NoStyle);
    virtual ~QwtColumnSymbol();

    void setFrameStyle(FrameStyle);
    FrameStyle frameStyle() const;

    void setLineWidth(int width);
    int lineWidth() const;

    void setStyle(Style);
    Style style() const;

    void setPen(const QPen& pen);
    QPen pen() const;

    void setBrush(const QBrush& b);
    QBrush brush() const;

    virtual void draw(QPainter*, const QwtColumnRect&) const;

protected:
    void drawBox(QPainter*, const QwtColumnRect&) const;

private:
    Q_DISABLE_COPY(QwtColumnSymbol)

    class PrivateData;
    PrivateData* m_data;
};

#endif
