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

#ifndef QWT_PLOT_SCALE_ITEM_H
#define QWT_PLOT_SCALE_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"
#include "qwt_scale_draw.h"

class QPalette;

/**
 * \if ENGLISH
 * @brief A class which draws a scale inside the plot canvas
 * @details QwtPlotScaleItem can be used to draw an axis inside the plot canvas. 
 *          It might be synchronized to one of the axis of the plot, but can also display its own ticks and labels.
 * 
 *          It is allowed to synchronize the scale item with a disabled axis. In plots with vertical and horizontal scale items,
 *          it might be necessary to remove ticks at the intersections, by overloading updateScaleDiv().
 * 
 *          The scale might be at a specific position (e.g 0.0) or it might be aligned to a canvas border.
 * 
 * @par Example
 * The following example shows how to replace the left axis, by a scale item at the x position 0.0.
 * @code
 *   QwtPlotScaleItem *scaleItem = new QwtPlotScaleItem( QwtScaleDraw::RightScale, 0.0 );
 *   scaleItem->setFont( plot->axisWidget( QwtAxis::YLeft )->font() );
 *   scaleItem->attach(plot);
 * 
 *   plot->setAxisVisible( QwtAxis::YLeft, false );
 * @endcode
 * \endif
 * 
 * \if CHINESE
 * @brief 在绘图画布内绘制刻度的类
 * @details QwtPlotScaleItem 可用于在绘图画布内绘制一条坐标轴。
 *          它可以与绘图的某一轴同步，也可以显示自己的刻度和标签。
 * 
 *          允许将刻度项与禁用的轴同步。在同时存在垂直和水平刻度项的绘图中，
 *          可能需要通过重载 updateScaleDiv() 方法来移除交叉点处的刻度。
 * 
 *          刻度可以位于特定位置（例如 0.0），也可以与画布边缘对齐。
 * 
 * @par 示例
 * 以下示例展示了如何用 x 位置为 0.0 处的刻度项替换左轴。
 * @code
 *   QwtPlotScaleItem *scaleItem = new QwtPlotScaleItem( QwtScaleDraw::RightScale, 0.0 );
 *   scaleItem->setFont( plot->axisWidget( QwtAxis::YLeft )->font() );
 *   scaleItem->attach(plot);
 * 
 *   plot->setAxisVisible( QwtAxis::YLeft, false );
 * @endcode
 * \endif
 */
class QWT_EXPORT QwtPlotScaleItem : public QwtPlotItem
{
public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    explicit QwtPlotScaleItem(QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale, const double pos = 0.0);

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotScaleItem();

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
    virtual int rtti() const override;

    /**
     * \if ENGLISH
     * @brief Set the scale division
     * \endif
     */
    void setScaleDiv(const QwtScaleDiv&);
    /**
     * \if ENGLISH
     * @brief Get the scale division
     * \endif
     */
    const QwtScaleDiv& scaleDiv() const;

    /**
     * \if ENGLISH
     * @brief Enable/disable scale division from axis
     * \endif
     */
    void setScaleDivFromAxis(bool on);
    /**
     * \if ENGLISH
     * @brief Check if scale division is from axis
     * \endif
     */
    bool isScaleDivFromAxis() const;

    /**
     * \if ENGLISH
     * @brief Set the palette
     * \endif
     */
    void setPalette(const QPalette&);
    /**
     * \if ENGLISH
     * @brief Get the palette
     * \endif
     */
    QPalette palette() const;

    /**
     * \if ENGLISH
     * @brief Set the font
     * \endif
     */
    void setFont(const QFont&);
    /**
     * \if ENGLISH
     * @brief Get the font
     * \endif
     */
    QFont font() const;

    /**
     * \if ENGLISH
     * @brief Set the scale draw
     * \endif
     */
    void setScaleDraw(QwtScaleDraw*);

    /**
     * \if ENGLISH
     * @brief Get the scale draw (const version)
     * \endif
     */
    const QwtScaleDraw* scaleDraw() const;
    /**
     * \if ENGLISH
     * @brief Get the scale draw
     * \endif
     */
    QwtScaleDraw* scaleDraw();

    /**
     * \if ENGLISH
     * @brief Set the position
     * \endif
     */
    void setPosition(double pos);
    /**
     * \if ENGLISH
     * @brief Get the position
     * \endif
     */
    double position() const;

    /**
     * \if ENGLISH
     * @brief Set the border distance
     * \endif
     */
    void setBorderDistance(int);
    /**
     * \if ENGLISH
     * @brief Get the border distance
     * \endif
     */
    int borderDistance() const;

    /**
     * \if ENGLISH
     * @brief Set the alignment
     * \endif
     */
    void setAlignment(QwtScaleDraw::Alignment);

    /**
     * \if ENGLISH
     * @brief Draw the scale item
     * \endif
     */
    virtual void draw(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;

    /**
     * \if ENGLISH
     * @brief Update the scale division
     * \endif
     */
    virtual void updateScaleDiv(const QwtScaleDiv&, const QwtScaleDiv&) override;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
