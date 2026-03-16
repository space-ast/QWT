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

#ifndef QWT_SYMBOL_H
#define QWT_SYMBOL_H

#include "qwt_global.h"

#include <qpolygon.h>
#include <qpen.h>
#include <qbrush.h>

class QPainter;
class QSize;
class QBrush;
class QPen;
class QColor;
class QPointF;
class QPainterPath;
class QPixmap;
class QByteArray;
class QwtGraphic;

/**
 * \if ENGLISH
 * @brief A class for drawing symbols
 * \endif
 * \if CHINESE
 * @brief 用于绘制符号的类
 * \endif
 */
class QWT_EXPORT QwtSymbol
{
  public:
    /**
     * \if ENGLISH
     * @brief Symbol Style
     * @sa setStyle(), style()
     * \endif
     * \if CHINESE
     * @brief 符号样式
     * @sa setStyle(), style()
     * \endif
     */
    enum Style
    {
        //! \if ENGLISH No Style. The symbol cannot be drawn. \endif \if CHINESE 无样式，符号无法绘制。 \endif
        NoSymbol = -1,

        //! \if ENGLISH Ellipse or circle \endif \if CHINESE 椭圆或圆形 \endif
        Ellipse,

        //! \if ENGLISH Rectangle \endif \if CHINESE 矩形 \endif
        Rect,

        //! \if ENGLISH Diamond \endif \if CHINESE 菱形 \endif
        Diamond,

        //! \if ENGLISH Triangle pointing upwards \endif \if CHINESE 向上指向的三角形 \endif
        Triangle,

        //! \if ENGLISH Triangle pointing downwards \endif \if CHINESE 向下指向的三角形 \endif
        DTriangle,

        //! \if ENGLISH Triangle pointing upwards \endif \if CHINESE 向上指向的三角形 \endif
        UTriangle,

        //! \if ENGLISH Triangle pointing left \endif \if CHINESE 向左指向的三角形 \endif
        LTriangle,

        //! \if ENGLISH Triangle pointing right \endif \if CHINESE 向右指向的三角形 \endif
        RTriangle,

        //! \if ENGLISH Cross (+) \endif \if CHINESE 十字 (+) \endif
        Cross,

        //! \if ENGLISH Diagonal cross (X) \endif \if CHINESE 对角十字 (X) \endif
        XCross,

        //! \if ENGLISH Horizontal line \endif \if CHINESE 水平线 \endif
        HLine,

        //! \if ENGLISH Vertical line \endif \if CHINESE 垂直线 \endif
        VLine,

        //! \if ENGLISH X combined with + \endif \if CHINESE X 与 + 组合 \endif
        Star1,

        //! \if ENGLISH Six-pointed star \endif \if CHINESE 六角星 \endif
        Star2,

        //! \if ENGLISH Hexagon \endif \if CHINESE 六边形 \endif
        Hexagon,

        /**
         * \if ENGLISH
         * The symbol is represented by a painter path, where the
         * origin ( 0, 0 ) of the path coordinate system is mapped to
         * the position of the symbol.
         * @sa setPath(), path()
         * \endif
         * \if CHINESE
         * 符号由绘图路径表示，其中路径坐标系的原点 (0, 0) 映射到符号的位置。
         * @sa setPath(), path()
         * \endif
         */
        Path,

        /**
         * \if ENGLISH
         * The symbol is represented by a pixmap. The pixmap is centered
         * or aligned to its pin point.
         * @sa setPinPoint()
         * \endif
         * \if CHINESE
         * 符号由像素图表示。像素图居中或对齐到其锚点。
         * @sa setPinPoint()
         * \endif
         */
        Pixmap,

        /**
         * \if ENGLISH
         * The symbol is represented by a graphic. The graphic is centered
         * or aligned to its pin point.
         * @sa setPinPoint()
         * \endif
         * \if CHINESE
         * 符号由图形表示。图形居中或对齐到其锚点。
         * @sa setPinPoint()
         * \endif
         */
        Graphic,

        /**
         * \if ENGLISH
         * The symbol is represented by a SVG graphic. The graphic is centered
         * or aligned to its pin point.
         * @sa setPinPoint()
         * \endif
         * \if CHINESE
         * 符号由 SVG 图形表示。图形居中或对齐到其锚点。
         * @sa setPinPoint()
         * \endif
         */
        SvgDocument,

        /**
         * \if ENGLISH
         * Styles >= QwtSymbol::UserSymbol are reserved for derived
         * classes of QwtSymbol that overload drawSymbols() with
         * additional application specific symbol types.
         * \endif
         * \if CHINESE
         * 样式 >= QwtSymbol::UserSymbol 保留给 QwtSymbol 的派生类，
         * 这些派生类使用额外的应用程序特定符号类型重载 drawSymbols()。
         * \endif
         */
        UserStyle = 1000
    };

    /**
     * \if ENGLISH
     * @brief Cache policy for symbol rendering
     *
     * Depending on the render engine and the complexity of the
     * symbol shape it might be faster to render the symbol
     * to a pixmap and to paint this pixmap.
     *
     * F.e. the raster paint engine is a pure software renderer
     * where in cache mode a draw operation usually ends in
     * raster operation with the the backing store, that are usually
     * faster, than the algorithms for rendering polygons.
     * But the opposite can be expected for graphic pipelines
     * that can make use of hardware acceleration.
     *
     * The default setting is AutoCache
     *
     * @sa setCachePolicy(), cachePolicy()
     *
     * @note The policy has no effect, when the symbol is painted
     *       to a vector graphics format ( PDF, SVG ).
     * @warning Since Qt 4.8 raster is the default backend on X11
     * \endif
     * \if CHINESE
     * @brief 符号渲染的缓存策略
     *
     * 根据渲染引擎和符号形状的复杂性，将符号渲染到像素图然后绘制该像素图可能更快。
     *
     * 例如，光栅绘制引擎是纯软件渲染器，在缓存模式下，绘制操作通常以与后备存储区的光栅操作结束，
     * 这通常比渲染多边形的算法更快。但对于可以利用硬件加速的图形管道，情况可能相反。
     *
     * 默认设置是 AutoCache
     *
     * @sa setCachePolicy(), cachePolicy()
     *
     * @note 当符号绘制到矢量图形格式（PDF、SVG）时，此策略无效。
     * @warning 从 Qt 4.8 开始，光栅是 X11 上的默认后端
     * \endif
     */
    enum CachePolicy
    {
        //! \if ENGLISH Don't use a pixmap cache \endif \if CHINESE 不使用像素图缓存 \endif
        NoCache,

        //! \if ENGLISH Always use a pixmap cache \endif \if CHINESE 始终使用像素图缓存 \endif
        Cache,

        /**
         * \if ENGLISH
         * Use a cache when one of the following conditions is true:
         * - The symbol is rendered with the software renderer ( QPaintEngine::Raster )
         * \endif
         * \if CHINESE
         * 当以下条件之一为真时使用缓存：
         * - 符号使用软件渲染器渲染 ( QPaintEngine::Raster )
         * \endif
         */
        AutoCache
    };

  public:
    /// \if ENGLISH Constructor with style \endif \if CHINESE 带样式的构造函数 \endif
    explicit QwtSymbol( Style = NoSymbol );
    /// \if ENGLISH Constructor with style, brush, pen and size \endif \if CHINESE 带样式、画刷、画笔和大小的构造函数 \endif
    QwtSymbol( Style, const QBrush&, const QPen&, const QSize& );
    /// \if ENGLISH Constructor with painter path, brush and pen \endif \if CHINESE 带绘图路径、画刷和画笔的构造函数 \endif
    QwtSymbol( const QPainterPath&, const QBrush&, const QPen& );

    /// \if ENGLISH Destructor \endif \if CHINESE 析构函数 \endif
    virtual ~QwtSymbol();

    /// \if ENGLISH Set cache policy \endif \if CHINESE 设置缓存策略 \endif
    void setCachePolicy( CachePolicy );
    /// \if ENGLISH Get cache policy \endif \if CHINESE 获取缓存策略 \endif
    CachePolicy cachePolicy() const;

    /// \if ENGLISH Set size \endif \if CHINESE 设置大小 \endif
    void setSize( const QSize& );
    /// \if ENGLISH Set size with width and height \endif \if CHINESE 使用宽度和高度设置大小 \endif
    void setSize( int width, int height = -1 );
    /// \if ENGLISH Get size \endif \if CHINESE 获取大小 \endif
    const QSize& size() const;

    /// \if ENGLISH Set pin point \endif \if CHINESE 设置锚点 \endif
    void setPinPoint( const QPointF& pos, bool enable = true );
    /// \if ENGLISH Get pin point \endif \if CHINESE 获取锚点 \endif
    QPointF pinPoint() const;

    /// \if ENGLISH Enable/disable pin point \endif \if CHINESE 启用/禁用锚点 \endif
    void setPinPointEnabled( bool );
    /// \if ENGLISH Check if pin point is enabled \endif \if CHINESE 检查锚点是否启用 \endif
    bool isPinPointEnabled() const;

    /// \if ENGLISH Set color \endif \if CHINESE 设置颜色 \endif
    virtual void setColor( const QColor& );

    /// \if ENGLISH Set brush \endif \if CHINESE 设置画刷 \endif
    void setBrush( const QBrush& );
    /// \if ENGLISH Get brush \endif \if CHINESE 获取画刷 \endif
    const QBrush& brush() const;

    /// \if ENGLISH Set pen with color, width and style \endif \if CHINESE 使用颜色、宽度和样式设置画笔 \endif
    void setPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    /// \if ENGLISH Set pen \endif \if CHINESE 设置画笔 \endif
    void setPen( const QPen& );
    /// \if ENGLISH Get pen \endif \if CHINESE 获取画笔 \endif
    const QPen& pen() const;

    /// \if ENGLISH Set style \endif \if CHINESE 设置样式 \endif
    void setStyle( Style );
    /// \if ENGLISH Get style \endif \if CHINESE 获取样式 \endif
    Style style() const;

    /// \if ENGLISH Set painter path \endif \if CHINESE 设置绘图路径 \endif
    void setPath( const QPainterPath& );
    /// \if ENGLISH Get painter path \endif \if CHINESE 获取绘图路径 \endif
    const QPainterPath& path() const;

    /// \if ENGLISH Set pixmap \endif \if CHINESE 设置像素图 \endif
    void setPixmap( const QPixmap& );
    /// \if ENGLISH Get pixmap \endif \if CHINESE 获取像素图 \endif
    const QPixmap& pixmap() const;

    /// \if ENGLISH Set graphic \endif \if CHINESE 设置图形 \endif
    void setGraphic( const QwtGraphic& );
    /// \if ENGLISH Get graphic \endif \if CHINESE 获取图形 \endif
    const QwtGraphic& graphic() const;

#ifndef QWT_NO_SVG
    /// \if ENGLISH Set SVG document \endif \if CHINESE 设置 SVG 文档 \endif
    void setSvgDocument( const QByteArray& );
#endif

    /// \if ENGLISH Draw symbol in rectangle \endif \if CHINESE 在矩形中绘制符号 \endif
    void drawSymbol( QPainter*, const QRectF& ) const;
    /// \if ENGLISH Draw symbol at point \endif \if CHINESE 在点处绘制符号 \endif
    void drawSymbol( QPainter*, const QPointF& ) const;
    /// \if ENGLISH Draw symbols at polygon points \endif \if CHINESE 在多边形点处绘制符号 \endif
    void drawSymbols( QPainter*, const QPolygonF& ) const;
    /// \if ENGLISH Draw symbols at points \endif \if CHINESE 在点处绘制符号 \endif
    void drawSymbols( QPainter*,
        const QPointF*, int numPoints ) const;

    /// \if ENGLISH Get bounding rectangle \endif \if CHINESE 获取边界矩形 \endif
    virtual QRect boundingRect() const;
    /// \if ENGLISH Invalidate cache \endif \if CHINESE 使缓存无效 \endif
    void invalidateCache();

  protected:
    virtual void renderSymbols( QPainter*,
        const QPointF*, int numPoints ) const;

  private:
    Q_DISABLE_COPY(QwtSymbol)

    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief Draw the symbol at a specified position

   \param painter Painter
   \param pos Position of the symbol in screen coordinates
 */
inline void QwtSymbol::drawSymbol(
    QPainter* painter, const QPointF& pos ) const
{
    drawSymbols( painter, &pos, 1 );
}

/*!
   \brief Draw symbols at the specified points

   \param painter Painter
   \param points Positions of the symbols in screen coordinates
 */

inline void QwtSymbol::drawSymbols(
    QPainter* painter, const QPolygonF& points ) const
{
    drawSymbols( painter, points.data(), points.size() );
}

#endif
