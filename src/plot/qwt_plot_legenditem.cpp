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

#include "qwt_plot_legenditem.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_legend_data.h"
#include "qwt_math.h"

#include <qlayoutitem.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>

namespace
{
    class LayoutItem final : public QLayoutItem
    {
      public:
        LayoutItem( const QwtPlotLegendItem*, const QwtPlotItem* );
        virtual ~LayoutItem();

        const QwtPlotItem* plotItem() const;

        void setData( const QwtLegendData& );
        const QwtLegendData& data() const;

        virtual Qt::Orientations expandingDirections() const override;
        virtual QRect geometry() const override;
        virtual bool hasHeightForWidth() const override;
        virtual int heightForWidth( int ) const override;
        virtual bool isEmpty() const override;
        virtual QSize maximumSize() const override;
        virtual int minimumHeightForWidth( int ) const override;
        virtual QSize minimumSize() const override;
        virtual void setGeometry( const QRect& ) override;
        virtual QSize sizeHint() const override;

      private:

        const QwtPlotLegendItem* m_legendItem;
        const QwtPlotItem* m_plotItem;
        QwtLegendData m_data;

        QRect m_rect;
    };

    LayoutItem::LayoutItem(
            const QwtPlotLegendItem* legendItem, const QwtPlotItem* plotItem )
        : m_legendItem( legendItem )
        , m_plotItem( plotItem)
    {
    }

    LayoutItem::~LayoutItem()
    {
    }

    const QwtPlotItem* LayoutItem::plotItem() const
    {
        return m_plotItem;
    }

    void LayoutItem::setData( const QwtLegendData& data )
    {
        m_data = data;
    }

    const QwtLegendData& LayoutItem::data() const
    {
        return m_data;
    }

    Qt::Orientations LayoutItem::expandingDirections() const
    {
        return Qt::Horizontal;
    }

    bool LayoutItem::hasHeightForWidth() const
    {
        return !m_data.title().isEmpty();
    }

    int LayoutItem::minimumHeightForWidth( int w ) const
    {
        return m_legendItem->heightForWidth( m_data, w );
    }

    int LayoutItem::heightForWidth( int w ) const
    {
        return m_legendItem->heightForWidth( m_data, w );
    }

    bool LayoutItem::isEmpty() const
    {
        return false;
    }

    QSize LayoutItem::maximumSize() const
    {
        return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX );
    }

    QSize LayoutItem::minimumSize() const
    {
        return m_legendItem->minimumSize( m_data );
    }

    QSize LayoutItem::sizeHint() const
    {
        return minimumSize();
    }

    void LayoutItem::setGeometry( const QRect& rect )
    {
        m_rect = rect;
    }

    QRect LayoutItem::geometry() const
    {
        return m_rect;
    }
}

class QwtPlotLegendItem::PrivateData
{
  public:
    PrivateData()
        : itemMargin( 4 )
        , itemSpacing( 4 )
        , borderRadius( 0.0 )
        , borderPen( Qt::NoPen )
        , backgroundBrush( Qt::NoBrush )
        , backgroundMode( QwtPlotLegendItem::LegendBackground )
        , canvasAlignment( Qt::AlignRight | Qt::AlignBottom )
    {
        canvasOffset[ 0 ] = canvasOffset[1] = 10;
        layout = new QwtDynGridLayout();
        layout->setMaxColumns( 2 );

        layout->setSpacing( 0 );
        layout->setContentsMargins( 0, 0, 0, 0 );
    }

    ~PrivateData()
    {
        delete layout;
    }

    QFont font;
    QPen textPen;
    int itemMargin;
    int itemSpacing;

    double borderRadius;
    QPen borderPen;
    QBrush backgroundBrush;
    QwtPlotLegendItem::BackgroundMode backgroundMode;

    int canvasOffset[2];
    Qt::Alignment canvasAlignment;

    QMap< const QwtPlotItem*, QList< LayoutItem* > > map;
    QwtDynGridLayout* layout;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Creates a legend item with default settings.
 *          The legend item is initialized with LegendInterest enabled
 *          and a z-value of 100.0.
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 创建一个具有默认设置的图例项。
 *          图例项初始化时启用 LegendInterest，z 值为 100.0。
 * \endif
 */
QwtPlotLegendItem::QwtPlotLegendItem()
    : QwtPlotItem( QwtText( "Legend" ) )
{
    m_data = new PrivateData;

    setItemInterest( QwtPlotItem::LegendInterest, true );
    setZ( 100.0 );
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPlotLegendItem::~QwtPlotLegendItem()
{
    clearLegend();
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotLegend
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotLegend
 * \endif
 */
int QwtPlotLegendItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotLegend;
}

/**
 * \if ENGLISH
 * @brief Set the alignment of the legend within the canvas
 * @param[in] alignment Alignment flags (e.g., Qt::AlignRight | Qt::AlignBottom)
 * @details Alignment means the position of the legend relative to the geometry
 *          of the plot canvas.
 * @note To align a legend with many items horizontally, the number of columns
 *       needs to be limited.
 * @sa alignmentInCanvas(), setMaxColumns()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例在画布内的对齐方式
 * @param[in] alignment 对齐标志（例如 Qt::AlignRight | Qt::AlignBottom）
 * @details 对齐是指图例相对于绘图画布几何形状的位置。
 * @note 要水平对齐包含多个项目的图例，需要限制列数。
 * @sa alignmentInCanvas(), setMaxColumns()
 * \endif
 */
void QwtPlotLegendItem::setAlignmentInCanvas( Qt::Alignment alignment )
{
    if ( m_data->canvasAlignment != alignment )
    {
        m_data->canvasAlignment = alignment;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the alignment flags
 * @return Alignment flags
 * @sa setAlignmentInCanvas()
 * \endif
 *
 * \if CHINESE
 * @brief 获取对齐标志
 * @return 对齐标志
 * @sa setAlignmentInCanvas()
 * \endif
 */
Qt::Alignment QwtPlotLegendItem::alignmentInCanvas() const
{
    return m_data->canvasAlignment;
}

/**
 * \if ENGLISH
 * @brief Limit the number of columns
 * @param[in] maxColumns Maximum number of columns. 0 means unlimited.
 * @details When aligning the legend horizontally (Qt::AlignLeft, Qt::AlignRight),
 *          the number of columns needs to be limited to avoid the width of
 *          the legend growing with an increasing number of entries.
 * @sa maxColumns(), QwtDynGridLayout::setMaxColumns()
 * \endif
 *
 * \if CHINESE
 * @brief 限制列数
 * @param[in] maxColumns 最大列数。0 表示无限制。
 * @details 当水平对齐图例（Qt::AlignLeft、Qt::AlignRight）时，
 *          需要限制列数以避免图例宽度随条目数量增加而增长。
 * @sa maxColumns(), QwtDynGridLayout::setMaxColumns()
 * \endif
 */
void QwtPlotLegendItem::setMaxColumns( uint maxColumns )
{
    if ( maxColumns != m_data->layout->maxColumns() )
    {
        m_data->layout->setMaxColumns( maxColumns );
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum number of columns
 * @return Maximum number of columns
 * @sa setMaxColumns(), QwtDynGridLayout::maxColumns()
 * \endif
 *
 * \if CHINESE
 * @brief 获取最大列数
 * @return 最大列数
 * @sa setMaxColumns(), QwtDynGridLayout::maxColumns()
 * \endif
 */
uint QwtPlotLegendItem::maxColumns() const
{
    return m_data->layout->maxColumns();
}

/**
 * \if ENGLISH
 * @brief Set the margin around legend items
 * @param[in] margin Margin in pixels
 * @details The default setting for the margin is 0.
 * @sa margin(), setSpacing(), setItemMargin(), setItemSpacing
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例项周围的边距
 * @param[in] margin 边距（像素）
 * @details 边距的默认设置为 0。
 * @sa margin(), setSpacing(), setItemMargin(), setItemSpacing
 * \endif
 */
void QwtPlotLegendItem::setMargin( int margin )
{
    margin = qMax( margin, 0 );
    if ( margin != this->margin() )
    {
        m_data->layout->setContentsMargins(
            margin, margin, margin, margin );

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the margin around the legend items
 * @return Margin around the legend items
 * @sa setMargin(), spacing(), itemMargin(), itemSpacing()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例项周围的边距
 * @return 图例项周围的边距
 * @sa setMargin(), spacing(), itemMargin(), itemSpacing()
 * \endif
 */
int QwtPlotLegendItem::margin() const
{
    int left;
    m_data->layout->getContentsMargins( &left, nullptr, nullptr, nullptr );

    return left;
}

/**
 * \if ENGLISH
 * @brief Set the spacing between the legend items
 * @param[in] spacing Spacing in pixels
 * @sa spacing(), setMargin()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例项之间的间距
 * @param[in] spacing 间距（像素）
 * @sa spacing(), setMargin()
 * \endif
 */
void QwtPlotLegendItem::setSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != m_data->layout->spacing() )
    {
        m_data->layout->setSpacing( spacing );
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the spacing between the legend items
 * @return Spacing between the legend items
 * @sa setSpacing(), margin(), itemSpacing(), itemMargin()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例项之间的间距
 * @return 图例项之间的间距
 * @sa setSpacing(), margin(), itemSpacing(), itemMargin()
 * \endif
 */
int QwtPlotLegendItem::spacing() const
{
    return m_data->layout->spacing();
}

/**
 * \if ENGLISH
 * @brief Set the margin around each item
 * @param[in] margin Margin
 * @sa itemMargin(), setItemSpacing(), setMargin(), setSpacing()
 * \endif
 *
 * \if CHINESE
 * @brief 设置每个项目周围的边距
 * @param[in] margin 边距
 * @sa itemMargin(), setItemSpacing(), setMargin(), setSpacing()
 * \endif
 */
void QwtPlotLegendItem::setItemMargin( int margin )
{
    margin = qMax( margin, 0 );
    if ( margin != m_data->itemMargin )
    {
        m_data->itemMargin = margin;

        m_data->layout->invalidate();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the margin around each item
 * @return Margin around each item
 * @sa setItemMargin(), itemSpacing(), margin(), spacing()
 * \endif
 *
 * \if CHINESE
 * @brief 获取每个项目周围的边距
 * @return 每个项目周围的边距
 * @sa setItemMargin(), itemSpacing(), margin(), spacing()
 * \endif
 */
int QwtPlotLegendItem::itemMargin() const
{
    return m_data->itemMargin;
}

/**
 * \if ENGLISH
 * @brief Set the spacing inside of each item
 * @param[in] spacing Spacing
 * @sa itemSpacing(), setItemMargin(), setMargin(), setSpacing()
 * \endif
 *
 * \if CHINESE
 * @brief 设置每个项目内部的间距
 * @param[in] spacing 间距
 * @sa itemSpacing(), setItemMargin(), setMargin(), setSpacing()
 * \endif
 */
void QwtPlotLegendItem::setItemSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != m_data->itemSpacing )
    {
        m_data->itemSpacing = spacing;

        m_data->layout->invalidate();
        itemChanged();
    }

}

/**
 * \if ENGLISH
 * @brief Get the spacing inside of each item
 * @return Spacing inside of each item
 * @sa setItemSpacing(), itemMargin(), margin(), spacing()
 * \endif
 *
 * \if CHINESE
 * @brief 获取每个项目内部的间距
 * @return 每个项目内部的间距
 * @sa setItemSpacing(), itemMargin(), margin(), spacing()
 * \endif
 */
int QwtPlotLegendItem::itemSpacing() const
{
    return m_data->itemSpacing;
}

/**
 * \if ENGLISH
 * @brief Change the font used for drawing the text label
 * @param[in] font Legend font
 * @sa font()
 * \endif
 *
 * \if CHINESE
 * @brief 更改用于绘制文本标签的字体
 * @param[in] font 图例字体
 * @sa font()
 * \endif
 */
void QwtPlotLegendItem::setFont( const QFont& font )
{
    if ( font != m_data->font )
    {
        m_data->font = font;

        m_data->layout->invalidate();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the font used for drawing the text label
 * @return Font used for drawing the text label
 * @sa setFont()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制文本标签的字体
 * @return 用于绘制文本标签的字体
 * @sa setFont()
 * \endif
 */
QFont QwtPlotLegendItem::font() const
{
    return m_data->font;
}

/**
 * \if ENGLISH
 * @brief Set the distance between the legend and the canvas border
 * @param[in] orientations Qt::Horizontal is for the left/right, Qt::Vertical for the top/bottom offset.
 * @param[in] numPixels Distance in pixels
 * @details The default setting is 10 pixels.
 * @sa setMargin()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例与画布边缘之间的距离
 * @param[in] orientations Qt::Horizontal 用于左/右，Qt::Vertical 用于上/下偏移。
 * @param[in] numPixels 距离（像素）
 * @details 默认设置为 10 像素。
 * @sa setMargin()
 * \endif
 */
void QwtPlotLegendItem::setOffsetInCanvas(
    Qt::Orientations orientations, int numPixels )
{
    if ( numPixels < 0 )
        numPixels = -1;

    bool isChanged = false;

    int* offset = m_data->canvasOffset;

    if ( orientations & Qt::Horizontal )
    {
        if ( numPixels != offset[0] )
        {
            offset[0] = numPixels;
            isChanged = true;
        }
    }

    if ( orientations & Qt::Vertical )
    {
        if ( numPixels != offset[1] )
        {
            offset[1] = numPixels;
            isChanged = true;
        }
    }

    if ( isChanged )
        itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the distance between the legend and the canvas border
 * @param[in] orientation Qt::Horizontal is for the left/right, Qt::Vertical for the top/bottom padding.
 * @return Distance between the legend and the canvas border
 * @sa setOffsetInCanvas()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例与画布边缘之间的距离
 * @param[in] orientation Qt::Horizontal 用于左/右，Qt::Vertical 用于上/下填充。
 * @return 图例与画布边缘之间的距离
 * @sa setOffsetInCanvas()
 * \endif
 */
int QwtPlotLegendItem::offsetInCanvas(
    Qt::Orientation orientation ) const
{
    const int index = ( orientation == Qt::Vertical ) ? 1 : 0;
    return m_data->canvasOffset[index];
}

/**
 * \if ENGLISH
 * @brief Set the radius for the border
 * @param[in] radius A value <= 0 defines a rectangular border
 * @sa borderRadius(), setBorderPen()
 * \endif
 *
 * \if CHINESE
 * @brief 设置边框半径
 * @param[in] radius 值 <= 0 定义矩形边框
 * @sa borderRadius(), setBorderPen()
 * \endif
 */
void QwtPlotLegendItem::setBorderRadius( double radius )
{
    radius = qwtMaxF( 0.0, radius );

    if ( radius != m_data->borderRadius )
    {
        m_data->borderRadius = radius;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the radius of the border
 * @return Radius of the border
 * @sa setBorderRadius(), setBorderPen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取边框半径
 * @return 边框半径
 * @sa setBorderRadius(), setBorderPen()
 * \endif
 */
double QwtPlotLegendItem::borderRadius() const
{
    return m_data->borderRadius;
}

/**
 * \if ENGLISH
 * @brief Set the pen for drawing the border
 * @param[in] pen Border pen
 * @sa borderPen(), setBackgroundBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 设置用于绘制边框的画笔
 * @param[in] pen 边框画笔
 * @sa borderPen(), setBackgroundBrush()
 * \endif
 */
void QwtPlotLegendItem::setBorderPen( const QPen& pen )
{
    if ( m_data->borderPen != pen )
    {
        m_data->borderPen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen for drawing the border
 * @return Pen for drawing the border
 * @sa setBorderPen(), backgroundBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制边框的画笔
 * @return 用于绘制边框的画笔
 * @sa setBorderPen(), backgroundBrush()
 * \endif
 */
QPen QwtPlotLegendItem::borderPen() const
{
    return m_data->borderPen;
}

/**
 * \if ENGLISH
 * @brief Set the background brush
 * @param[in] brush Brush used to fill the background
 * @sa backgroundBrush(), setBackgroundMode(), drawBackground()
 * \endif
 *
 * \if CHINESE
 * @brief 设置背景画刷
 * @param[in] brush 用于填充背景的画刷
 * @sa backgroundBrush(), setBackgroundMode(), drawBackground()
 * \endif
 */
void QwtPlotLegendItem::setBackgroundBrush( const QBrush& brush )
{
    if ( m_data->backgroundBrush != brush )
    {
        m_data->backgroundBrush = brush;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the brush used to fill the background
 * @return Brush used to fill the background
 * @sa setBackgroundBrush(), backgroundMode(), drawBackground()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于填充背景的画刷
 * @return 用于填充背景的画刷
 * @sa setBackgroundBrush(), backgroundMode(), drawBackground()
 * \endif
 */
QBrush QwtPlotLegendItem::backgroundBrush() const
{
    return m_data->backgroundBrush;
}

/**
 * \if ENGLISH
 * @brief Set the background mode
 * @param[in] mode Background mode
 * @details Depending on the mode the complete legend or each item
 *          might have a background. The default setting is LegendBackground.
 * @sa backgroundMode(), setBackgroundBrush(), drawBackground()
 * \endif
 *
 * \if CHINESE
 * @brief 设置背景模式
 * @param[in] mode 背景模式
 * @details 根据模式，整个图例或每个项目可能有背景。默认设置为 LegendBackground。
 * @sa backgroundMode(), setBackgroundBrush(), drawBackground()
 * \endif
 */
void QwtPlotLegendItem::setBackgroundMode( BackgroundMode mode )
{
    if ( mode != m_data->backgroundMode )
    {
        m_data->backgroundMode = mode;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the background mode
 * @return Background mode
 * @sa setBackgroundMode(), backgroundBrush(), drawBackground()
 * \endif
 *
 * \if CHINESE
 * @brief 获取背景模式
 * @return 背景模式
 * @sa setBackgroundMode(), backgroundBrush(), drawBackground()
 * \endif
 */
QwtPlotLegendItem::BackgroundMode QwtPlotLegendItem::backgroundMode() const
{
    return m_data->backgroundMode;
}

/**
 * \if ENGLISH
 * @brief Set the pen for drawing text labels
 * @param[in] pen Text pen
 * @sa textPen(), setFont()
 * \endif
 *
 * \if CHINESE
 * @brief 设置用于绘制文本标签的画笔
 * @param[in] pen 文本画笔
 * @sa textPen(), setFont()
 * \endif
 */
void QwtPlotLegendItem::setTextPen( const QPen& pen )
{
    if ( m_data->textPen != pen )
    {
        m_data->textPen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen for drawing text labels
 * @return Pen for drawing text labels
 * @sa setTextPen(), font()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制文本标签的画笔
 * @return 用于绘制文本标签的画笔
 * @sa setTextPen(), font()
 * \endif
 */
QPen QwtPlotLegendItem::textPen() const
{
    return m_data->textPen;
}

/**
 * \if ENGLISH
 * @brief Draw the legend
 * @param[in] painter Painter
 * @param[in] xMap X Scale Map
 * @param[in] yMap Y Scale Map
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制图例
 * @param[in] painter 绘制器
 * @param[in] xMap X 比例映射
 * @param[in] yMap Y 比例映射
 * @param[in] canvasRect 画布的内容矩形（绘制器坐标）
 * \endif
 */
void QwtPlotLegendItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );

    m_data->layout->setGeometry( geometry( canvasRect ) );
    if ( m_data->layout->geometry().isEmpty() )
    {
        // don't draw a legend when having no content
        return;
    }

    if ( m_data->backgroundMode == QwtPlotLegendItem::LegendBackground )
        drawBackground( painter, m_data->layout->geometry() );

    for ( int i = 0; i < m_data->layout->count(); i++ )
    {
        const LayoutItem* layoutItem =
            static_cast< LayoutItem* >( m_data->layout->itemAt( i ) );

        if ( m_data->backgroundMode == QwtPlotLegendItem::ItemBackground )
            drawBackground( painter, layoutItem->geometry() );

        painter->save();

        drawLegendData( painter, layoutItem->plotItem(),
            layoutItem->data(), layoutItem->geometry() );

        painter->restore();
    }
}

/*!
   Draw a rounded rect

   \param painter Painter
   \param rect Bounding rectangle

   \sa setBorderRadius(), setBorderPen(),
      setBackgroundBrush(), setBackgroundMode()
 */
void QwtPlotLegendItem::drawBackground(
    QPainter* painter, const QRectF& rect ) const
{
    painter->save();

    painter->setPen( m_data->borderPen );
    painter->setBrush( m_data->backgroundBrush );

    const double radius = m_data->borderRadius;
    painter->drawRoundedRect( rect, radius, radius );

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Calculate the geometry of the legend on the canvas
 * @param[in] canvasRect Geometry of the canvas
 * @return Geometry of the legend
 * \endif
 *
 * \if CHINESE
 * @brief 计算图例在画布上的几何形状
 * @param[in] canvasRect 画布的几何形状
 * @return 图例的几何形状
 * \endif
 */
QRect QwtPlotLegendItem::geometry( const QRectF& canvasRect ) const
{
    QRect rect;
    rect.setSize( m_data->layout->sizeHint() );

    if ( m_data->canvasAlignment & Qt::AlignHCenter )
    {
        int x = qRound( canvasRect.center().x() );
        rect.moveCenter( QPoint( x, rect.center().y() ) );
    }
    else if ( m_data->canvasAlignment & Qt::AlignRight )
    {
        const int offset = offsetInCanvas( Qt::Horizontal );
        rect.moveRight( qwtFloor( canvasRect.right() - offset ) );
    }
    else
    {
        const int offset = offsetInCanvas( Qt::Horizontal );
        rect.moveLeft( qwtCeil( canvasRect.left() + offset ) );
    }

    if ( m_data->canvasAlignment & Qt::AlignVCenter )
    {
        int y = qRound( canvasRect.center().y() );
        rect.moveCenter( QPoint( rect.center().x(), y ) );
    }
    else if ( m_data->canvasAlignment & Qt::AlignBottom )
    {
        const int offset = offsetInCanvas( Qt::Vertical );
        rect.moveBottom( qwtFloor( canvasRect.bottom() - offset ) );
    }
    else
    {
        const int offset = offsetInCanvas( Qt::Vertical );
        rect.moveTop( qwtCeil( canvasRect.top() + offset ) );
    }

    return rect;
}

/**
 * \if ENGLISH
 * @brief Update the legend items according to modifications of a plot item
 * @param[in] plotItem Plot item
 * @param[in] data Attributes of the legend entries
 * \endif
 *
 * \if CHINESE
 * @brief 根据绘图项的修改更新图例项
 * @param[in] plotItem 绘图项
 * @param[in] data 图例条目的属性
 * \endif
 */
void QwtPlotLegendItem::updateLegend( const QwtPlotItem* plotItem,
    const QList< QwtLegendData >& data )
{
    if ( plotItem == nullptr )
        return;

    QList< LayoutItem* > layoutItems;

    QMap< const QwtPlotItem*, QList< LayoutItem* > >::const_iterator it =
        m_data->map.constFind( plotItem );
    if ( it != m_data->map.constEnd() )
        layoutItems = it.value();

    bool changed = false;

    if ( data.size() != layoutItems.size() )
    {
        changed = true;

        for ( int i = 0; i < layoutItems.size(); i++ )
        {
            m_data->layout->removeItem( layoutItems[i] );
            delete layoutItems[i];
        }
        layoutItems.clear();

        if ( it != m_data->map.constEnd() )
            m_data->map.remove( plotItem );

        if ( !data.isEmpty() )
        {
            layoutItems.reserve( data.size() );

            for ( int i = 0; i < data.size(); i++ )
            {
                LayoutItem* layoutItem =
                    new LayoutItem( this, plotItem );
                m_data->layout->addItem( layoutItem );
                layoutItems += layoutItem;
            }

            m_data->map.insert( plotItem, layoutItems );
        }
    }

    for ( int i = 0; i < data.size(); i++ )
    {
        if ( layoutItems[i]->data().values() != data[i].values() )
        {
            layoutItems[i]->setData( data[i] );
            changed = true;
        }
    }

    if ( changed )
    {
        m_data->layout->invalidate();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Remove all items from the legend
 * \endif
 *
 * \if CHINESE
 * @brief 从图例中移除所有项目
 * \endif
 */
void QwtPlotLegendItem::clearLegend()
{
    if ( !m_data->map.isEmpty() )
    {
        m_data->map.clear();

        for ( int i = m_data->layout->count() - 1; i >= 0; i-- )
            delete m_data->layout->takeAt( i );

        itemChanged();
    }
}

/*!
   Draw an entry on the legend

   \param painter Qt Painter
   \param plotItem Plot item, represented by the entry
   \param data Attributes of the legend entry
   \param rect Bounding rectangle for the entry
 */
void QwtPlotLegendItem::drawLegendData( QPainter* painter,
    const QwtPlotItem* plotItem, const QwtLegendData& data,
    const QRectF& rect ) const
{
    Q_UNUSED( plotItem );

    const int m = m_data->itemMargin;
    const QRectF r = rect.toRect().adjusted( m, m, -m, -m );

    painter->setClipRect( r, Qt::IntersectClip );

    int titleOff = 0;

    const QwtGraphic graphic = data.icon();
    if ( !graphic.isEmpty() )
    {
        QRectF iconRect( r.topLeft(), graphic.defaultSize() );

        iconRect.moveCenter(
            QPoint( iconRect.center().x(), rect.center().y() ) );

        graphic.render( painter, iconRect, Qt::KeepAspectRatio );

        titleOff += iconRect.width() + m_data->itemSpacing;
    }

    const QwtText text = data.title();
    if ( !text.isEmpty() )
    {
        painter->setPen( textPen() );
        painter->setFont( font() );

        const QRectF textRect = r.adjusted( titleOff, 0, 0, 0 );
        text.draw( painter, textRect );
    }
}

/**
 * \if ENGLISH
 * @brief Get the minimum size hint needed to display an entry
 * @param[in] data Attributes of the legend entry
 * @return Minimum size
 * \endif
 *
 * \if CHINESE
 * @brief 获取显示条目所需的最小尺寸提示
 * @param[in] data 图例条目的属性
 * @return 最小尺寸
 * \endif
 */
QSize QwtPlotLegendItem::minimumSize( const QwtLegendData& data ) const
{
    QSize size( 2 * m_data->itemMargin, 2 * m_data->itemMargin );

    if ( !data.isValid() )
        return size;

    const QwtGraphic graphic = data.icon();
    const QwtText text = data.title();

    int w = 0;
    int h = 0;

    if ( !graphic.isNull() )
    {
        w = graphic.width();
        h = graphic.height();
    }

    if ( !text.isEmpty() )
    {
        const QSizeF sz = text.textSize( font() );

        w += qwtCeil( sz.width() );
        h = qMax( h, qwtCeil( sz.height() ) );
    }

    if ( graphic.width() > 0 && !text.isEmpty() )
        w += m_data->itemSpacing;

    size += QSize( w, h );
    return size;
}

/**
 * \if ENGLISH
 * @brief Get the preferred height for a given width
 * @param[in] data Attributes of the legend entry
 * @param[in] width Width
 * @return The preferred height
 * \endif
 *
 * \if CHINESE
 * @brief 获取给定宽度的首选高度
 * @param[in] data 图例条目的属性
 * @param[in] width 宽度
 * @return 首选高度
 * \endif
 */
int QwtPlotLegendItem::heightForWidth(
    const QwtLegendData& data, int width ) const
{
    width -= 2 * m_data->itemMargin;

    const QwtGraphic graphic = data.icon();
    const QwtText text = data.title();

    if ( text.isEmpty() )
        return graphic.height();

    if ( graphic.width() > 0 )
        width -= graphic.width() + m_data->itemSpacing;

    int h = text.heightForWidth( width, font() );
    h += 2 * m_data->itemMargin;

    return qMax( graphic.height(), h );
}

/**
 * \if ENGLISH
 * @brief Get all plot items with an entry on the legend
 * @return All plot items with an entry on the legend
 * @note A plot item might have more than one entry on the legend
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例上有条目的所有绘图项
 * @return 图例上有条目的所有绘图项
 * @note 一个绘图项可能在图例上有多个条目
 * \endif
 */
QList< const QwtPlotItem* > QwtPlotLegendItem::plotItems() const
{
    return m_data->map.keys();
}

/**
 * \if ENGLISH
 * @brief Get geometries of the items of a plot item
 * @param[in] plotItem Plot item
 * @return Geometries of the items of a plot item
 * @note Usually a plot item has only one entry on the legend
 * \endif
 *
 * \if CHINESE
 * @brief 获取绘图项的项目的几何形状
 * @param[in] plotItem 绘图项
 * @return 绘图项的项目的几何形状
 * @note 通常一个绘图项在图例上只有一个条目
 * \endif
 */
QList< QRect > QwtPlotLegendItem::legendGeometries(
    const QwtPlotItem* plotItem ) const
{
    QList< LayoutItem* > layoutItems;

    QMap< const QwtPlotItem*, QList< LayoutItem* > >::const_iterator it =
        m_data->map.constFind( plotItem );
    if ( it != m_data->map.constEnd() )
        layoutItems = it.value();

    QList< QRect > geometries;
    geometries.reserve(layoutItems.size() );

    for ( int i = 0; i < layoutItems.size(); i++ )
        geometries += layoutItems[i]->geometry();

    return geometries;
}
