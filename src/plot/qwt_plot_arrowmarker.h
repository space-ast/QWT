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

#ifndef QWT_PLOT_ARROWMARKER_H
#define QWT_PLOT_ARROWMARKER_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

class QString;
class QRectF;
class QwtText;
class QPainterPath;
class QPen;
class QBrush;

/**
 * \if ENGLISH
 * @brief A class for drawing arrow markers on plots
 * @details QwtPlotArrowMarker is a specialized plot item for drawing arrows
 *          with customizable start and end points, arrow styles, lengths,
 *          and colors. The arrow is drawn in canvas pixel coordinates,
 *          maintaining its size during zoom operations while following
 *          canvas panning.
 *
 *          Key features:
 *          - Support for explicit start and end point specification
 *          - Arrow positioning via start point, length, and rotation angle
 *          - Customizable arrow head and tail styles
 *          - Canvas pixel-based drawing (size preserved during zoom)
 *          - Flexible styling system with predefined and custom shapes
 *
 * \endif
 *
 * \if CHINESE
 * @brief 用于在绘图上绘制箭头标记的类
 * @details QwtPlotArrowMarker 是一个专门的绘图项，用于绘制具有可自定义起点和终点、
 *          箭头样式、长度和颜色的箭头。箭头在画布像素坐标中绘制，在缩放操作期间保持其大小，
 *          同时跟随画布平移。
 *
 *          主要特性：
 *          - 支持显式起点和终点指定
 *          - 通过起点、长度和旋转角度定位箭头
 *          - 可自定义箭头头部和尾部样式
 *          - 基于画布像素的绘制（缩放时大小保持不变）
 *          - 具有预定义和自定义形状的灵活样式系统
 * \endif
 */

class QWT_EXPORT QwtPlotArrowMarker : public QwtPlotItem
{
public:
    /**
     * \if ENGLISH
     * @brief Arrow endpoint style types
     * @sa setHeadStyle(), setTailStyle(), headStyle(), tailStyle()
     * \endif
     *
     * \if CHINESE
     * @brief 箭头端点样式类型
     * @sa setHeadStyle(), setTailStyle(), headStyle(), tailStyle()
     * \endif
     */
    enum EndpointStyle
    {
        /**
         * \if ENGLISH
         * @brief  No endpoint (invisible)
         * \endif
         *
         * \if CHINESE
         * @brief 无端点（不可见）
         * \endif
         */
        NoEndpoint = 0,

        /**
         * \if ENGLISH
         * @brief Arrow head style
         * \endif
         *
         * \if CHINESE
         * @brief 箭头头部样式
         * \endif
         */
        ArrowHead,

        /**
         * \if ENGLISH
         * @brief Circle endpoint
         * \endif
         *
         * \if CHINESE
         * @brief 圆形端点
         * \endif
         */
        Circle,

        /**
         * \if ENGLISH
         * @brief Square endpoint
         * \endif
         *
         * \if CHINESE
         * @brief 方形端点
         * \endif
         */
        Square,

        /**
         * \if ENGLISH
         * @brief Diamond endpoint
         * \endif
         *
         * \if CHINESE
         * @brief 菱形端点
         * \endif
         */
        Diamond,

        /**
         * \if ENGLISH
         * @brief Triangle endpoint
         * \endif
         *
         * \if CHINESE
         * @brief 三角形端点
         * \endif
         */
        Triangle,

        /**
         * \if ENGLISH
         * @brief Custom QPainterPath endpoint
         * \endif
         *
         * \if CHINESE
         * @brief 自定义 QPainterPath 端点
         * \endif
         */
        CustomPath
    };

    /**
     * \if ENGLISH
     * @brief Arrow positioning mode
     * @sa setPositionMode(), positionMode()
     * \endif
     *
     * \if CHINESE
     * @brief 箭头定位模式
     * @sa setPositionMode(), positionMode()
     * \endif
     */
    enum PositionMode
    {
        /**
         * \if ENGLISH
         * @brief  Use explicit start and end points
         *
         * In this mode, the arrow's start and end points are specified in plot coordinates.
         * The arrow's size will change proportionally to the plot's zoom level.
         * \endif
         *
         * \if CHINESE
         * @brief 使用显式起点和终点
         *
         * 使用此模式，箭头的起点和终点基于绘图坐标，会跟随画布的缩放而改变大小
         * \endif
         */
        ExplicitPoints,

        /**
         * \if ENGLISH
         * @brief  start point, length, and angle
         *
         * in this mode, the arrow length will be fixed regardless of the zoom level
         * \endif
         *
         * \if CHINESE
         * @brief 使用起点、长度和角度
         *
         * 使用此模式，箭头在画布的长度不会随着画布的缩放而改变，箭头的长度将是固定的
         * \endif
         */
        StartLengthAngle
    };

    /// Constructor
    explicit QwtPlotArrowMarker();

    /// Constructor with title
    explicit QwtPlotArrowMarker(const QString& title);

    /// Constructor with QwtText title
    explicit QwtPlotArrowMarker(const QwtText& title);

    /// Destructor
    virtual ~QwtPlotArrowMarker();

    /// Get the runtime type information
    virtual int rtti() const override;

    // Position and geometry methods

    /// Get the start point
    QPointF startPoint() const;

    /// Get the end point
    QPointF endPoint() const;

    /// Set the start point
    void setStartPoint(const QPointF& point);

    /// Set the end point
    void setEndPoint(const QPointF& point);

    /// Set both start and end points
    void setPoints(const QPointF& start, const QPointF& end);

    /// Get the arrow length in pixels
    double length() const;

    /// Set the arrow length in pixels
    void setLength(double length);

    /// Get the rotation angle in degrees
    double angle() const;

    /// Set the rotation angle in degrees
    void setAngle(double angle);

    /// Get the positioning mode
    PositionMode positionMode() const;

    /// Set the positioning mode
    void setPositionMode(PositionMode mode);

    // Style and appearance methods

    /// Get the arrow line pen
    const QPen& linePen() const;

    /// Set the arrow line pen
    void setLinePen(const QPen& pen);

    /// Convenience method to set line color and width
    void setLinePen(const QColor& color, qreal width = 1.0, Qt::PenStyle style = Qt::SolidLine);

    /// Get the head style
    EndpointStyle headStyle() const;

    /// Set the head style
    void setHeadStyle(EndpointStyle style);

    /// Get the tail style
    EndpointStyle tailStyle() const;

    /// Set the tail style
    void setTailStyle(EndpointStyle style);

    /// Get the head size in pixels
    QSizeF headSize() const;

    /// Set the head size in pixels
    void setHeadSize(const QSizeF& size);

    /// Convenience method to set head size with equal width and height
    void setHeadSize(qreal size);

    /// Get the tail size in pixels
    QSizeF tailSize() const;

    /// Set the tail size in pixels
    void setTailSize(const QSizeF& size);

    /// Convenience method to set tail size with equal width and height
    void setTailSize(qreal size);

    /// Get the head brush
    const QBrush& headBrush() const;

    /// Set the head brush
    void setHeadBrush(const QBrush& brush);

    /// Get the tail brush
    const QBrush& tailBrush() const;

    /// Set the tail brush
    void setTailBrush(const QBrush& brush);

    /// Get the head pen
    const QPen& headPen() const;

    /// Set the head pen
    void setHeadPen(const QPen& pen);

    /// Get the tail pen
    const QPen& tailPen() const;

    /// Set the tail pen
    void setTailPen(const QPen& pen);

    /// Set a custom path for head endpoint (requires headStyle() == CustomPath)
    void setHeadCustomPath(const QPainterPath& path);

    /// Get the custom head path
    QPainterPath headCustomPath() const;

    /// Set a custom path for tail endpoint (requires tailStyle() == CustomPath)
    void setTailCustomPath(const QPainterPath& path);

    /// Get the custom tail path
    QPainterPath tailCustomPath() const;

    // Drawing methods

    /// Draw the arrow marker
    virtual void
    draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;

    /// Get the bounding rectangle
    virtual QRectF boundingRect() const override;

    /// Get the legend icon
    virtual QwtGraphic legendIcon(int index, const QSizeF& size) const override;

protected:
    /// Draw the arrow line
    virtual void drawArrowLine(QPainter* painter, const QPointF& canvasStart, const QPointF& canvasEnd) const;
    /// Draw a cached endpoint path
    virtual void drawCachedEndpoint(QPainter* painter,
                                    const QPointF& position,
                                    const QPainterPath& cachedPath,
                                    const QSizeF& size,
                                    const QPen& pen,
                                    const QBrush& brush,
                                    double rotation = 0.0) const;

    /// Convert plot coordinates to canvas coordinates
    virtual QPointF toCanvasPoint(const QPointF& plotPoint, const QwtScaleMap& xMap, const QwtScaleMap& yMap) const;

    /// Calculate end point based on start point, length, and angle
    virtual QPointF calculateEndPoint() const;

private:
    // Disable copy constructor and assignment operator
    QwtPlotArrowMarker(const QwtPlotArrowMarker&);
    QwtPlotArrowMarker& operator=(const QwtPlotArrowMarker&);

    class PrivateData;
    PrivateData* m_data;
};

#endif  // QWT_PLOT_ARROWMARKER_H