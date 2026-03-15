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

#ifndef QWT_ABSTRACT_SCALE_DRAW_H
#define QWT_ABSTRACT_SCALE_DRAW_H

#include "qwt_global.h"
#include "qwt_scale_div.h"

class QwtText;
class QPalette;
class QPainter;
class QFont;
class QwtTransform;
class QwtScaleMap;

/**
 * \if ENGLISH
 * @brief An abstract base class for drawing scales
 * @details QwtAbstractScaleDraw can be used to draw linear or logarithmic scales.
 *          After a scale division has been specified as a QwtScaleDiv object
 *          using setScaleDiv(), the scale can be drawn with the draw() member.
 * \endif
 * \if CHINESE
 * @brief 绘制刻度的抽象基类
 * @details QwtAbstractScaleDraw 可用于绘制线性或对数刻度。
 *          在使用 setScaleDiv() 指定刻度划分为 QwtScaleDiv 对象后，
 *          可以使用 draw() 成员绘制刻度。
 * \endif
 */
class QWT_EXPORT QwtAbstractScaleDraw
{
public:
    /**
     * \if ENGLISH
     * @brief Components of a scale
     * \sa enableComponent(), hasComponent
     * \endif
     * \if CHINESE
     * @brief 刻度的组件
     * \sa enableComponent(), hasComponent
     * \endif
     */
    enum ScaleComponent
    {
        //! \if ENGLISH Backbone = the line where the ticks are located \endif \if CHINESE 主干 = 刻度线所在的位置 \endif
        Backbone = 0x01,

        //! \if ENGLISH Ticks \endif \if CHINESE 刻度线 \endif
        Ticks = 0x02,

        //! \if ENGLISH Labels \endif \if CHINESE 标签 \endif
        Labels = 0x04
    };

    Q_DECLARE_FLAGS(ScaleComponents, ScaleComponent)

    QwtAbstractScaleDraw();
    virtual ~QwtAbstractScaleDraw();

    void setScaleDiv(const QwtScaleDiv&);
    const QwtScaleDiv& scaleDiv() const;

    void setTransformation(QwtTransform*);
    const QwtScaleMap& scaleMap() const;
    QwtScaleMap& scaleMap();

    void enableComponent(ScaleComponent, bool enable = true);
    bool hasComponent(ScaleComponent) const;

    void setTickLength(QwtScaleDiv::TickType, double length);
    double tickLength(QwtScaleDiv::TickType) const;
    double maxTickLength() const;

    void setSpacing(double);
    double spacing() const;

    void setPenWidthF(qreal width);
    qreal penWidthF() const;

    // Set whether the scale draw is selected
    void setSelected(bool on);
    // Check if the scale draw is selected
    bool isSelected() const;

    // Set the pen width adjustment after selection
    void setSelectedPenWidthOffset(qreal offset = 1);
    qreal selectedPenWidthOffset() const;

    virtual void draw(QPainter*, const QPalette&) const;

    virtual QwtText label(double) const;

    /**
     * \if ENGLISH
     * @brief Calculate the extent
     * @details The extent is the distance from the baseline to the outermost
     *          pixel of the scale draw in opposite to its orientation.
     *          It is at least minimumExtent() pixels.
     * @param font Font used for drawing the tick labels
     * @return Number of pixels
     * \sa setMinimumExtent(), minimumExtent()
     * \endif
     * \if CHINESE
     * @brief 计算范围
     * @details 范围是从基线到刻度绘制相反方向最外侧像素的距离。
     *          它至少为 minimumExtent() 像素。
     * @param font 用于绘制刻度标签的字体
     * @return 像素数量
     * \sa setMinimumExtent(), minimumExtent()
     * \endif
     */
    virtual double extent(const QFont& font) const = 0;

    void setMinimumExtent(double);
    double minimumExtent() const;

    void invalidateCache();

protected:
    /**
     * \if ENGLISH
     * @brief Draw a tick
     * @param painter Painter
     * @param value Value of the tick
     * @param len Length of the tick
     * \sa drawBackbone(), drawLabel()
     * \endif
     * \if CHINESE
     * @brief 绘制刻度线
     * @param painter 绘制器
     * @param value 刻度值
     * @param len 刻度线长度
     * \sa drawBackbone(), drawLabel()
     * \endif
     */
    virtual void drawTick(QPainter* painter, double value, double len) const = 0;

    /**
     * \if ENGLISH
     * @brief Draws the baseline of the scale
     * @param painter Painter
     * \sa drawTick(), drawLabel()
     * \endif
     * \if CHINESE
     * @brief 绘制刻度的基线
     * @param painter 绘制器
     * \sa drawTick(), drawLabel()
     * \endif
     */
    virtual void drawBackbone(QPainter* painter) const = 0;

    /**
     * \if ENGLISH
     * @brief Draws the label for a major scale tick
     * @param painter Painter
     * @param value Value
     * \sa drawTick(), drawBackbone()
     * \endif
     * \if CHINESE
     * @brief 绘制主刻度标签
     * @param painter 绘制器
     * @param value 值
     * \sa drawTick(), drawBackbone()
     * \endif
     */
    virtual void drawLabel(QPainter* painter, double value) const = 0;

    const QwtText& tickLabel(const QFont&, double value) const;

private:
    Q_DISABLE_COPY(QwtAbstractScaleDraw)

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtAbstractScaleDraw::ScaleComponents)

#endif
