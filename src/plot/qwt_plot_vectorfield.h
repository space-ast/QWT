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

#ifndef QWT_PLOT_VECTOR_FIELD_H
#define QWT_PLOT_VECTOR_FIELD_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
class QwtVectorFieldSymbol;
class QwtColorMap;
class QPen;
class QBrush;

/**
 * \if ENGLISH
 * @brief A plot item, that represents a vector field
 * @details A vector field is a representation of a points with a given magnitude and direction
 *          as arrows. While the direction affects the direction of the arrow, the magnitude
 *          might be represented as a color or by the length of the arrow.
 *
 * @sa QwtVectorFieldSymbol, QwtVectorFieldSample
 * \endif
 *
 * \if CHINESE
 * @brief 表示矢量场的绘图项
 * @details 矢量场是点的表示，这些点具有给定的大小和方向，以箭头形式显示。
 *          方向影响箭头的方向，大小可以通过颜色或箭头长度来表示。
 *
 * @sa QwtVectorFieldSymbol, QwtVectorFieldSample
 * \endif
 */
class QWT_EXPORT QwtPlotVectorField : public QwtPlotSeriesItem, public QwtSeriesStore< QwtVectorFieldSample >
{
public:
    /**
     * \if ENGLISH
     * @brief Indicator origin
     * @details Depending on the origin the indicator symbol ( usually an arrow )
     *          will be to the position of the corresponding sample.
     * \endif
     *
     * \if CHINESE
     * @brief 指示器原点
     * @details 根据原点的不同，指示器符号（通常是箭头）
     *          将指向对应样本的位置。
     * \endif
     */
    enum IndicatorOrigin
    {
        /// symbol points to the sample position
        OriginHead,

        /// The arrow starts at the sample position
        OriginTail,

        /// The arrow is centered at the sample position
        OriginCenter
    };

    /**
     * \if ENGLISH
     * @brief Paint attributes
     * @details Attributes to modify the rendering
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     *
     * \if CHINESE
     * @brief 绘制属性
     * @details 用于修改渲染的属性
     * @sa setPaintAttribute(), testPaintAttribute()
     * \endif
     */
    enum PaintAttribute
    {
        /**
         * \if ENGLISH
         * FilterVectors calculates an average sample from all samples
         * that lie in the same cell of a grid that is determined by
         * setting the rasterSize().
         *
         * @sa setRasterSize()
         * \endif
         *
         * \if CHINESE
         * FilterVectors 计算同一网格单元中所有样本的平均样本，
         * 网格由设置 rasterSize() 确定。
         *
         * @sa setRasterSize()
         * \endif
         */
        FilterVectors = 0x01
    };

    Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)

    /**
     * \if ENGLISH
     * @brief Magnitude mode
     * @details Depending on the MagnitudeMode the magnitude component will have
     *          an impact on the attributes of the symbol/arrow.
     *
     * @sa setMagnitudeMode()
     * \endif
     *
     * \if CHINESE
     * @brief 大小模式
     * @details 根据 MagnitudeMode，大小分量将对符号/箭头的属性产生影响。
     *
     * @sa setMagnitudeMode()
     * \endif
     */
    enum MagnitudeMode
    {
        /**
         * \if ENGLISH
         * The magnitude will be mapped to a color using a color map
         * @sa magnitudeRange(), colorMap()
         * \endif
         *
         * \if CHINESE
         * 大小将使用颜色映射映射到颜色
         * @sa magnitudeRange(), colorMap()
         * \endif
         */
        MagnitudeAsColor = 0x01,

        /**
         * \if ENGLISH
         * The magnitude will have an impact on the length of the arrow/symbol
         * @sa arrowLength(), magnitudeScaleFactor()
         * \endif
         *
         * \if CHINESE
         * 大小将对箭头/符号的长度产生影响
         * @sa arrowLength(), magnitudeScaleFactor()
         * \endif
         */
        MagnitudeAsLength = 0x02
    };

    Q_DECLARE_FLAGS(MagnitudeModes, MagnitudeMode)

    // Constructor
    explicit QwtPlotVectorField(const QString& title = QString());
    // Constructor with title
    explicit QwtPlotVectorField(const QwtText& title);

    // Destructor
    virtual ~QwtPlotVectorField();

    // Set a paint attribute
    void setPaintAttribute(PaintAttribute, bool on = true);
    // Test a paint attribute
    bool testPaintAttribute(PaintAttribute) const;

    // Set a magnitude mode
    void setMagnitudeMode(MagnitudeMode, bool on = true);
    // Test a magnitude mode
    bool testMagnitudeMode(MagnitudeMode) const;

    // Set the symbol
    void setSymbol(QwtVectorFieldSymbol*);
    // Get the symbol
    const QwtVectorFieldSymbol* symbol() const;

    // Set the pen
    void setPen(const QPen&);
    // Get the pen
    QPen pen() const;

    // Set the brush
    void setBrush(const QBrush&);
    // Get the brush
    QBrush brush() const;

    // Set the raster size
    void setRasterSize(const QSizeF&);
    // Get the raster size
    QSizeF rasterSize() const;

    // Set the indicator origin
    void setIndicatorOrigin(IndicatorOrigin);
    // Get the indicator origin
    IndicatorOrigin indicatorOrigin() const;

    // Set the samples
    void setSamples(const QVector< QwtVectorFieldSample >&);
    // Set the samples
    void setSamples(QwtVectorFieldData*);

    // Set the color map
    void setColorMap(QwtColorMap*);
    // Get the color map
    const QwtColorMap* colorMap() const;

    // Set the magnitude range
    void setMagnitudeRange(const QwtInterval&);
    // Get the magnitude range
    QwtInterval magnitudeRange() const;

    // Set the minimum arrow length
    void setMinArrowLength(double);
    // Get the minimum arrow length
    double minArrowLength() const;

    // Set the maximum arrow length
    void setMaxArrowLength(double);
    // Get the maximum arrow length
    double maxArrowLength() const;

    // Calculate the arrow length for a given magnitude
    virtual double arrowLength(double magnitude) const;

    // Get the bounding rectangle
    virtual QRectF boundingRect() const override;

    // Draw the series
    virtual void drawSeries(QPainter*,
                            const QwtScaleMap& xMap,
                            const QwtScaleMap& yMap,
                            const QRectF& canvasRect,
                            int from,
                            int to) const override;

    // Get the runtime type information
    virtual int rtti() const override;

    // Get the legend icon
    virtual QwtGraphic legendIcon(int index, const QSizeF&) const override;

    // Set the magnitude scale factor
    void setMagnitudeScaleFactor(double factor);
    // Get the magnitude scale factor
    double magnitudeScaleFactor() const;

protected:
    virtual void
    drawSymbols(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const;

    virtual void drawSymbol(QPainter*, double x, double y, double vx, double vy) const;

    virtual void dataChanged() override;

private:
    void init();

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotVectorField::PaintAttributes)
Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotVectorField::MagnitudeModes)

#endif
