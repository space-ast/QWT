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

#ifndef QWT_PLOT_SHAPE_ITEM_H
#define QWT_PLOT_SHAPE_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

#include <qstring.h>

class QPainterPath;
class QPolygonF;

/**
 * \if ENGLISH
 * @brief A plot item, which displays any graphical shape that can be defined by a QPainterPath
 * @details QwtPlotShapeItem displays a shape composed from intersecting and uniting
 *          regions, rectangles, ellipses or irregular areas defined by lines, and curves.
 *          The shape is displayed with a pen and brush.
 *
 *          QwtPlotShapeItem offers a couple of optimizations like clipping or weeding.
 *          These algorithms need to convert the painter path into polygons that might be
 *          less performant for paths built from curves and ellipses.
 *
 *          More complex shapes, that can't be expressed by a QPainterPath can be displayed
 *          using QwtPlotGraphicItem.
 *
 * @sa QwtPlotZone, QwtPlotGraphicItem
 * \endif
 *
 * \if CHINESE
 * @brief 显示可由 QPainterPath 定义的任何图形形状的绘图项
 * @details QwtPlotShapeItem 显示由相交和联合的区域、矩形、椭圆或由线条和曲线定义的不规则区域组成的形状。
 *          形状使用钢笔和画笔显示。
 *
 *          QwtPlotShapeItem 提供了一些优化，如裁剪或除草。
 *          这些算法需要将绘制路径转换为多边形，这对于由曲线和椭圆构建的路径可能性能较低。
 *
 *          无法由 QPainterPath 表达的更复杂形状可以使用 QwtPlotGraphicItem 显示。
 *
 * @sa QwtPlotZone, QwtPlotGraphicItem
 * \endif
 */
class QWT_EXPORT QwtPlotShapeItem : public QwtPlotItem
{
public:
    /**
     * \if ENGLISH
     * @brief Paint attributes
     * @details Attributes to modify the drawing algorithm.
     *          The default disables all attributes
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     *
     * \if CHINESE
     * @brief 绘制属性
     * @details 用于修改绘制算法的属性。
     *          默认禁用所有属性
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * Clip polygons before painting them. In situations, where points
         * are far outside the visible area (f.e when zooming deep) this
         * might be a substantial improvement for the painting performance
         *
         * But polygon clipping will convert the painter path into
         * polygons what might introduce a negative impact on the
         * performance of paths composed from curves or ellipses.
         * \endif
         *
         * \if CHINESE
         * 在绘制多边形之前对其进行裁剪。在点远离可见区域的情况下（例如深度缩放时），
         * 这可能会显著提高绘制性能
         *
         * 但多边形裁剪会将绘制路径转换为多边形，这可能会对由曲线或椭圆组成的路径的性能产生负面影响。
         * \endif
         */
        ClipPolygons = 0x01,
    };

    Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)

    /**
     * \if ENGLISH
     * @brief Legend modes
     * @details Mode how to display the item on the legend
     * \endif
     *
     * \if CHINESE
     * @brief 图例模式
     * @details 在图例上显示项目的模式
     * \endif
     */
    enum LegendMode
    {
        /// Display a scaled down version of the shape
        LegendShape,

        /// Display a filled rectangle
        LegendColor
    };

    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    explicit QwtPlotShapeItem(const QString& title = QString());
    /**
     * \if ENGLISH
     * @brief Constructor with title
     * \endif
     */
    explicit QwtPlotShapeItem(const QwtText& title);

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotShapeItem();

    /**
     * \if ENGLISH
     * @brief Set a paint attribute
     * \endif
     */
    void setPaintAttribute(PaintAttribute, bool on = true);
    /**
     * \if ENGLISH
     * @brief Test a paint attribute
     * \endif
     */
    bool testPaintAttribute(PaintAttribute) const;

    /**
     * \if ENGLISH
     * @brief Set the legend mode
     * \endif
     */
    void setLegendMode(LegendMode);
    /**
     * \if ENGLISH
     * @brief Get the legend mode
     * \endif
     */
    LegendMode legendMode() const;

    /**
     * \if ENGLISH
     * @brief Set the shape as a rectangle
     * \endif
     */
    void setRect(const QRectF&);
    /**
     * \if ENGLISH
     * @brief Set the shape as a polygon
     * \endif
     */
    void setPolygon(const QPolygonF&);

    /**
     * \if ENGLISH
     * @brief Set the shape
     * \endif
     */
    void setShape(const QPainterPath&);
    /**
     * \if ENGLISH
     * @brief Get the shape
     * \endif
     */
    QPainterPath shape() const;

    /**
     * \if ENGLISH
     * @brief Set the pen
     * \endif
     */
    void setPen(const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine);
    /**
     * \if ENGLISH
     * @brief Set the pen
     * \endif
     */
    void setPen(const QPen&);
    /**
     * \if ENGLISH
     * @brief Get the pen
     * \endif
     */
    QPen pen() const;

    /**
     * \if ENGLISH
     * @brief Set the brush
     * \endif
     */
    void setBrush(const QBrush&);
    /**
     * \if ENGLISH
     * @brief Get the brush
     * \endif
     */
    QBrush brush() const;

    /**
     * \if ENGLISH
     * @brief Set the render tolerance
     * \endif
     */
    void setRenderTolerance(double);
    /**
     * \if ENGLISH
     * @brief Get the render tolerance
     * \endif
     */
    double renderTolerance() const;

    /**
     * \if ENGLISH
     * @brief Get the bounding rectangle
     * \endif
     */
    virtual QRectF boundingRect() const override;

    /**
     * \if ENGLISH
     * @brief Draw the shape item
     * \endif
     */
    virtual void draw(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;

    /**
     * \if ENGLISH
     * @brief Get the legend icon
     * \endif
     */
    virtual QwtGraphic legendIcon(int index, const QSizeF&) const override;

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
    virtual int rtti() const override;

private:
    /**
     * \if ENGLISH
     * @brief Initialize the shape item
     * \endif
     */
    void init();

    class PrivateData;
    PrivateData* m_data;
};

#endif
