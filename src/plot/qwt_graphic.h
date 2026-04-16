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

#ifndef QWT_GRAPHIC_H
#define QWT_GRAPHIC_H

#include "qwt_global.h"
#include "qwt_null_paintdevice.h"
#include "qwt_painter_command.h"

#include <qmetatype.h>

/**
 * \if ENGLISH
 * @brief A paint device for scalable graphics
 *
 * @details QwtGraphic is the representation of a graphic that is tailored for
 * scalability. Like QPicture it will be initialized by QPainter
 * operations and can be replayed later to any target paint device.
 *
 * While the usual image representations QImage and QPixmap are not
 * scalable Qt offers two paint devices, that might be candidates
 * for representing a vector graphic:
 *
 * - QPicture: Unfortunately QPicture had been forgotten, when Qt4
 *   introduced floating point based render engines. Its API
 *   is still on integers, what make it unusable for proper scaling.
 *
 * - QSvgRenderer/QSvgGenerator: Unfortunately QSvgRenderer hides too
 *   much information about its nodes in internal APIs, that are necessary
 *   for proper layout calculations. Also it is derived from QObject and
 *   can't be copied like QImage/QPixmap.
 *
 * QwtGraphic maps all scalable drawing primitives to a QPainterPath
 * and stores them together with the painter state changes
 * ( pen, brush, transformation ... ) in a list of QwtPaintCommands.
 * For being a complete QPaintDevice it also stores pixmaps or images,
 * what is somehow against the idea of the class, because these objects
 * can't be scaled without a loss in quality.
 *
 * The main issue about scaling a QwtGraphic object are the pens used for
 * drawing the outlines of the painter paths. While non cosmetic pens
 * ( QPen::isCosmetic() ) are scaled with the same ratio as the path,
 * cosmetic pens have a fixed width. A graphic might have paths with
 * different pens - cosmetic and non-cosmetic.
 *
 * QwtGraphic caches 2 different rectangles:
 *
 * - control point rectangle: The control point rectangle is the bounding
 *   rectangle of all control point rectangles of the painter paths,
 *   or the target rectangle of the pixmaps/images.
 *
 * - bounding rectangle: The bounding rectangle extends the control point
 *   rectangle by what is needed for rendering the outline with an unscaled pen.
 *
 * Because the offset for drawing the outline depends on the shape
 * of the painter path ( the peak of a triangle is different than the flat side )
 * scaling with a fixed aspect ratio always needs to be calculated from the
 * control point rectangle.
 * \endif
 *
 * \if CHINESE
 * @brief 可缩放图形的绘制设备
 *
 * @details QwtGraphic 是专为可缩放性设计的图形表示。
 * 与 QPicture 类似，它通过 QPainter 操作初始化，
 * 并可以在稍后重放到任何目标绘制设备。
 *
 * 常见的图像表示 QImage 和 QPixmap 不可缩放，
 * Qt 提供了两个可能用于表示矢量图形的绘制设备：
 *
 * - QPicture：不幸的是，Qt4 引入基于浮点数的渲染引擎时，
 *   QPicture 被遗忘了。其 API 仍然基于整数，
 *   使其无法用于正确缩放。
 *
 * - QSvgRenderer/QSvgGenerator：不幸的是，QSvgRenderer 在内部 API 中
 *   隐藏了太多关于其节点的信息，这些信息对于正确的布局计算是必要的。
 *   此外它派生自 QObject，无法像 QImage/QPixmap 一样复制。
 *
 * QwtGraphic 将所有可缩放的绘制原语映射到 QPainterPath，
 * 并将其与绘制状态变更（画笔、画刷、变换...）
 * 一起存储在 QwtPaintCommands 列表中。
 * 为了成为完整的 QPaintDevice，它还存储 pixmap 或 image，
 * 这在某种程度上违背了类的初衷，因为这些对象在缩放时会损失质量。
 *
 * 缩放 QwtGraphic 对象的主要问题是用于绘制路径轮廓的画笔。
 * 非装饰性画笔（QPen::isCosmetic()）与路径以相同比例缩放，
 * 而装饰性画笔具有固定宽度。图形可能包含使用不同画笔的路径——
 * 装饰性和非装饰性。
 *
 * QwtGraphic 缓存两个不同的矩形：
 *
 * - 控制点矩形：控制点矩形是所有绘制路径控制点矩形的边界矩形，
 *   或 pixmap/image 的目标矩形。
 *
 * - 边界矩形：边界矩形扩展了控制点矩形，
 *   以包含使用未缩放画笔渲染轮廓所需的空间。
 *
 * 因为绘制轮廓的偏移取决于绘制路径的形状
 * （三角形的顶点与平边不同），
 * 以固定纵横比缩放时总是需要从控制点矩形计算。
 * \endif
 */
class QWT_EXPORT QwtGraphic : public QwtNullPaintDevice
{
  public:
    /**
     * \if ENGLISH
     * @brief Hint how to render a graphic
     * \endif
     *
     * \if CHINESE
     * @brief 渲染图形的提示
     * \endif
     */
    enum RenderHint
    {
        /**
         * \if ENGLISH
         * When rendering a QwtGraphic a specific scaling between
         * the controlPointRect() and the coordinates of the target rectangle
         * is set up internally in render().
         *
         * When RenderPensUnscaled is set this specific scaling is applied
         * for the control points only, but not for the pens.
         * All other painter transformations ( set up by application code )
         * are supposed to work like usual.
         * \endif
         *
         * \if CHINESE
         * 渲染 QwtGraphic 时，在 render() 内部设置
         * controlPointRect() 与目标矩形坐标之间的特定缩放。
         *
         * 设置 RenderPensUnscaled 时，此特定缩放仅应用于控制点，
         * 不应用于画笔。所有其他绘制变换（由应用程序代码设置）
         * 按常规方式工作。
         * \endif
         */
        RenderPensUnscaled = 0x1
    };

    Q_DECLARE_FLAGS( RenderHints, RenderHint )

    /**
     * \if ENGLISH
     * @brief Indicator if the graphic contains a specific type of painter command
     * \endif
     *
     * \if CHINESE
     * @brief 指示图形是否包含特定类型的绘制命令
     * \endif
     */
    enum CommandType
    {
        //! \if ENGLISH The graphic contains scalable vector data \endif \if CHINESE 图形包含可缩放的矢量数据 \endif
        VectorData     = 1 << 0,

        //! \if ENGLISH The graphic contains raster data ( QPixmap or QImage ) \endif \if CHINESE 图形包含光栅数据（QPixmap 或 QImage） \endif
        RasterData     = 1 << 1,

        //! \if ENGLISH The graphic contains transformations beyond simple translations \endif \if CHINESE 图形包含超出简单平移的变换 \endif
        Transformation = 1 << 2
    };

    Q_DECLARE_FLAGS( CommandTypes, CommandType )

    // Constructor
    QwtGraphic();
    // Copy constructor
    QwtGraphic( const QwtGraphic& );

    // Destructor
    virtual ~QwtGraphic();

    // Assignment operator
    QwtGraphic& operator=( const QwtGraphic& );

    // Clear all stored commands
    void reset();

    // Check if the graphic is null
    bool isNull() const;
    // Check if the graphic is empty
    bool isEmpty() const;

    // Get the types of painter commands being used
    CommandTypes commandTypes() const;

    // Replay all recorded painter commands
    void render( QPainter* ) const;

    // Render graphic scaled to fit into given size
    void render( QPainter*, const QSizeF&,
        Qt::AspectRatioMode = Qt::IgnoreAspectRatio ) const;

    // Render graphic aligned to a position
    void render( QPainter*, const QPointF&,
        Qt::Alignment = Qt::AlignTop | Qt::AlignLeft ) const;

    // Render graphic scaled to fit into given rectangle
    void render( QPainter*, const QRectF&,
        Qt::AspectRatioMode = Qt::IgnoreAspectRatio ) const;

    // Convert graphic to QPixmap in default size
    QPixmap toPixmap( qreal devicePixelRatio = 0.0 ) const;

    // Convert graphic to QPixmap with specified size
    QPixmap toPixmap( const QSize&,
        Qt::AspectRatioMode = Qt::IgnoreAspectRatio,
        qreal devicePixelRatio = 0.0 ) const;

    // Convert graphic to QImage in default size
    QImage toImage( qreal devicePixelRatio = 0.0 ) const;

    // Convert graphic to QImage with specified size
    QImage toImage( const QSize&,
        Qt::AspectRatioMode = Qt::IgnoreAspectRatio,
        qreal devicePixelRatio = 0.0 ) const;

    // Calculate the scaled bounding rectangle
    QRectF scaledBoundingRect( qreal sx, qreal sy ) const;

    // Get the bounding rectangle
    QRectF boundingRect() const;
    // Get the control point rectangle
    QRectF controlPointRect() const;

    // Get the list of recorded paint commands
    const QVector< QwtPainterCommand >& commands() const;
    // Set the paint commands
    void setCommands( const QVector< QwtPainterCommand >& );

    // Set the default size
    void setDefaultSize( const QSizeF& );
    // Get the default size
    QSizeF defaultSize() const;

    // Get the height for a given width
    qreal heightForWidth( qreal width ) const;
    // Get the width for a given height
    qreal widthForHeight( qreal height ) const;

    // Set a render hint
    void setRenderHint( RenderHint, bool on = true );
    // Test a render hint
    bool testRenderHint( RenderHint ) const;

    // Get the render hints
    RenderHints renderHints() const;

  protected:
    virtual QSize sizeMetrics() const override;

    virtual void drawPath( const QPainterPath& ) override;

    virtual void drawPixmap( const QRectF&,
        const QPixmap&, const QRectF& ) override;

    virtual void drawImage( const QRectF&, const QImage&,
        const QRectF&, Qt::ImageConversionFlags ) override;

    virtual void updateState( const QPaintEngineState& ) override;

  private:
    void renderGraphic( QPainter*, QTransform* ) const;

    void updateBoundingRect( const QRectF& );
    void updateControlPointRect( const QRectF& );

    class PathInfo;

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtGraphic::RenderHints )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtGraphic::CommandTypes )
Q_DECLARE_METATYPE( QwtGraphic )

#endif
