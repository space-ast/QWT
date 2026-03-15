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

#ifndef QWT_VECTOR_FIELD_SYMBOL_H
#define QWT_VECTOR_FIELD_SYMBOL_H

#include "qwt_global.h"

class QPainter;
class QPainterPath;

/*!
    \if ENGLISH
    Defines abstract interface for arrow drawing routines.

    Arrow needs to be drawn horizontally with arrow tip at coordinate 0,0.
    arrowLength() shall return the entire length of the arrow (needed
    to translate the arrow for tail/centered alignment).
    setArrowLength() defines arror length in pixels (screen coordinates). It
    can be implemented to adjust other geometric properties such as
    the head size and width of the arrow. It is _always_ called before
    paint().

    A new arrow implementation can be set with QwtPlotVectorField::setArrowSymbol(), whereby
    ownership is transferred to the plot field.
    \endif
    *
    \if CHINESE
    定义箭头绘制例程的抽象接口。

    箭头需要水平绘制，箭头尖端在坐标 0,0。
    arrowLength() 应返回箭头的整个长度（需要
    平移箭头以进行尾部/中心对齐）。
    setArrowLength() 以像素为单位定义箭头长度（屏幕坐标）。它可以
    被实现为调整其他几何属性，例如
    箭头头部的大小和宽度。它 _总是_ 在 paint() 之前被调用。

    新的箭头实现可以通过 QwtPlotVectorField::setArrowSymbol() 设置，
    所有权将转移到绘图字段。
    \endif
 */
class QWT_EXPORT QwtVectorFieldSymbol
{
  public:
    QwtVectorFieldSymbol();
    virtual ~QwtVectorFieldSymbol();

    /*!
        \if ENGLISH
        Set the length of the symbol/arrow
        \sa length()
        \endif
        *
        \if CHINESE
        设置符号/箭头的长度
        \sa length()
        \endif
     */
    virtual void setLength( qreal length ) = 0;

    /*!
        \if ENGLISH
        \return length of the symbol/arrow
        \sa setLength()
        \endif
        *
        \if CHINESE
        \return 符号/箭头的长度
        \sa setLength()
        \endif
     */
    virtual qreal length() const = 0;

    /**
     * \if ENGLISH
     * @brief Draw the symbol/arrow
     * \endif
     *
     * \if CHINESE
     * @brief 绘制符号/箭头
     * \endif
     */
    virtual void paint( QPainter* ) const = 0;

  private:
    Q_DISABLE_COPY(QwtVectorFieldSymbol)
};

/*!
    \if ENGLISH
    Arrow implementation that draws a filled arrow with outline, using
    a triangular head of constant width.
    \endif
    *
    \if CHINESE
    箭头实现，绘制带轮廓的填充箭头，使用
    恒定宽度的三角形头部。
    \endif
 */
class QWT_EXPORT QwtVectorFieldArrow : public QwtVectorFieldSymbol
{
  public:
    QwtVectorFieldArrow( qreal headWidth = 6.0, qreal tailWidth = 1.0 );
    virtual ~QwtVectorFieldArrow() override;

    virtual void setLength( qreal length ) override;
    virtual qreal length() const override;

    virtual void paint( QPainter* ) const override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/*!
    \if ENGLISH
    Arrow implementation that only used lines, with optionally a filled arrow or only
    lines.
    \endif
    *
    \if CHINESE
    箭头实现，仅使用线条，可选择填充箭头或仅
    线条。
    \endif
 */
class QWT_EXPORT QwtVectorFieldThinArrow : public QwtVectorFieldSymbol
{
  public:
    QwtVectorFieldThinArrow( qreal headWidth = 6.0 );
    virtual ~QwtVectorFieldThinArrow() override;

    virtual void setLength( qreal length ) override;
    virtual qreal length() const override;

    virtual void paint( QPainter* ) const override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
