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

#ifndef QWT_PLOT_LEGEND_ITEM_H
#define QWT_PLOT_LEGEND_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

class QFont;

/**
 * \if ENGLISH
 * @brief A class which draws a legend inside the plot canvas
 * @details QwtPlotLegendItem can be used to draw a legend inside the plot canvas.
 *          It can be used together with a QwtLegend or instead of it
 *          to have more space for the plot canvas.
 *
 *          In opposite to QwtLegend the legend item is not interactive.
 *          To identify mouse clicks on a legend item an event filter
 *          needs to be installed catching mouse events on the plot canvas.
 *          The geometries of the legend items are available using
 *          legendGeometries().
 *
 *          The legend item is aligned to plot canvas according to
 *          its alignment() flags. It might have a background for the
 *          complete legend ( usually semi transparent ) or for
 *          each legend item.
 *
 * @note An external QwtLegend with a transparent background
 *       on top the plot canvas might be another option
 *       with a similar effect.
 * \endif
 *
 * \if CHINESE
 * @brief 在绘图画布内绘制图例的类
 * @details QwtPlotLegendItem 可用于在绘图画布内绘制图例。
 *          它可以与 QwtLegend 一起使用，或替代它，
 *          为绘图画布留出更多空间。
 *
 *          与 QwtLegend 不同，图例项是非交互式的。
 *          要识别对图例项的鼠标点击，需要安装事件过滤器来捕获绘图画布上的鼠标事件。
 *          图例项的几何形状可通过 legendGeometries() 获取。
 *
 *          图例项根据其 alignment() 标志与绘图画布对齐。
 *          它可能为整个图例（通常是半透明的）或每个图例项设置背景。
 *
 * @note 在绘图画布顶部使用具有透明背景的外部 QwtLegend
 *       可能是另一个具有类似效果的选项。
 * \endif
 */

class QWT_EXPORT QwtPlotLegendItem : public QwtPlotItem
{
public:
    /**
     * \if ENGLISH
     * @brief Background mode
     * @details Depending on the mode the complete legend or each item
     *          might have an background.
     *
     *          The default setting is LegendBackground.
     *
     * @sa setBackgroundMode(), setBackgroundBrush(), drawBackground()
     * \endif
     *
     * \if CHINESE
     * @brief 背景模式
     * @details 根据模式，整个图例或每个项目可能有背景。
     *
     *          默认设置是 LegendBackground。
     *
     * @sa setBackgroundMode(), setBackgroundBrush(), drawBackground()
     * \endif
     */
    enum BackgroundMode
    {
        /// The legend has a background
        LegendBackground,

        /// Each item has a background
        ItemBackground
    };

    /// Constructor
    explicit QwtPlotLegendItem();
    /// Destructor
    virtual ~QwtPlotLegendItem();

    /// Get the runtime type information
    virtual int rtti() const override;

    /// Set the alignment in canvas
    void setAlignmentInCanvas(Qt::Alignment);
    /// Get the alignment in canvas
    Qt::Alignment alignmentInCanvas() const;

    /// Set the offset in canvas
    void setOffsetInCanvas(Qt::Orientations, int numPixels);
    /// Get the offset in canvas
    int offsetInCanvas(Qt::Orientation) const;

    /// Set the maximum number of columns
    void setMaxColumns(uint);
    /// Get the maximum number of columns
    uint maxColumns() const;

    /// Set the margin
    void setMargin(int);
    /// Get the margin
    int margin() const;

    /// Set the spacing
    void setSpacing(int);
    /// Get the spacing
    int spacing() const;

    /// Set the item margin
    void setItemMargin(int);
    /// Get the item margin
    int itemMargin() const;

    /// Set the item spacing
    void setItemSpacing(int);
    /// Get the item spacing
    int itemSpacing() const;

    /// Set the font
    void setFont(const QFont&);
    /// Get the font
    QFont font() const;

    /// Set the border radius
    void setBorderRadius(double);
    /// Get the border radius
    double borderRadius() const;

    /// Set the border pen
    void setBorderPen(const QPen&);
    /// Get the border pen
    QPen borderPen() const;

    /// Set the background brush
    void setBackgroundBrush(const QBrush&);
    /// Get the background brush
    QBrush backgroundBrush() const;

    /// Set the background mode
    void setBackgroundMode(BackgroundMode);
    /// Get the background mode
    BackgroundMode backgroundMode() const;

    /// Set the text pen
    void setTextPen(const QPen&);
    /// Get the text pen
    QPen textPen() const;

    /// Draw the legend item
    virtual void draw(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;

    /// Clear the legend
    void clearLegend();

    /// Update the legend
    virtual void updateLegend(const QwtPlotItem*, const QList< QwtLegendData >&) override;

    /// Get the geometry
    virtual QRect geometry(const QRectF& canvasRect) const;

    /// Get the minimum size
    virtual QSize minimumSize(const QwtLegendData&) const;
    /// Get the height for width
    virtual int heightForWidth(const QwtLegendData&, int width) const;

    /// Get the plot items
    QList< const QwtPlotItem* > plotItems() const;
    /// Get the legend geometries
    QList< QRect > legendGeometries(const QwtPlotItem*) const;

protected:
    /// Draw the legend data
    virtual void drawLegendData(QPainter*, const QwtPlotItem*, const QwtLegendData&, const QRectF&) const;

    /// Draw the background
    virtual void drawBackground(QPainter*, const QRectF& rect) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
