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

#include "qwt_plot_marker.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>

class QwtPlotMarker::PrivateData
{
  public:
    PrivateData()
        : labelAlignment( Qt::AlignCenter )
        , labelOrientation( Qt::Horizontal )
        , spacing( 2 )
        , symbol( nullptr )
        , style( QwtPlotMarker::NoLine )
        , xValue( 0.0 )
        , yValue( 0.0 )
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtText label;
    Qt::Alignment labelAlignment;
    Qt::Orientation labelOrientation;
    int spacing;

    QPen pen;
    const QwtSymbol* symbol;
    LineStyle style;

    double xValue;
    double yValue;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 设置对齐为 Qt::AlignCenter，样式为 QwtPlotMarker::NoLine
 * \endif
 */
QwtPlotMarker::QwtPlotMarker()
{
    m_data = new PrivateData;
    setZ( 30.0 );
}

/**
 * \if ENGLISH
 * @brief Constructor with QString title
 * @details Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
 * @param[in] title Title of the marker
 * \endif
 *
 * \if CHINESE
 * @brief QString标题的构造函数
 * @details 设置对齐为 Qt::AlignCenter，样式为 QwtPlotMarker::NoLine
 * @param[in] title 标记标题
 * \endif
 */
QwtPlotMarker::QwtPlotMarker( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    m_data = new PrivateData;
    setZ( 30.0 );
}

/**
 * \if ENGLISH
 * @brief Constructor with QwtText title
 * @details Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
 * @param[in] title Title of the marker
 * \endif
 *
 * \if CHINESE
 * @brief QwtText标题的构造函数
 * @details 设置对齐为 Qt::AlignCenter，样式为 QwtPlotMarker::NoLine
 * @param[in] title 标记标题
 * \endif
 */
QwtPlotMarker::QwtPlotMarker( const QwtText& title )
    : QwtPlotItem( title )
{
    m_data = new PrivateData;
    setZ( 30.0 );
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
QwtPlotMarker::~QwtPlotMarker()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotMarker
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotMarker
 * \endif
 */
int QwtPlotMarker::rtti() const
{
    return QwtPlotItem::Rtti_PlotMarker;
}

/**
 * \if ENGLISH
 * @brief Get the value as a point
 * @return Value as QPointF
 * \endif
 *
 * \if CHINESE
 * @brief 获取作为点的值
 * @return QPointF形式的值
 * \endif
 */
QPointF QwtPlotMarker::value() const
{
    return QPointF( m_data->xValue, m_data->yValue );
}

/**
 * \if ENGLISH
 * @brief Get the x-value
 * @return X value
 * \endif
 *
 * \if CHINESE
 * @brief 获取x值
 * @return x值
 * \endif
 */
double QwtPlotMarker::xValue() const
{
    return m_data->xValue;
}

/**
 * \if ENGLISH
 * @brief Get the y-value
 * @return Y value
 * \endif
 *
 * \if CHINESE
 * @brief 获取y值
 * @return y值
 * \endif
 */
double QwtPlotMarker::yValue() const
{
    return m_data->yValue;
}

/**
 * \if ENGLISH
 * @brief Set the value from a point
 * @param[in] pos Position as QPointF
 * \endif
 *
 * \if CHINESE
 * @brief 从点设置值
 * @param[in] pos QPointF形式的位置
 * \endif
 */
void QwtPlotMarker::setValue( const QPointF& pos )
{
    setValue( pos.x(), pos.y() );
}

/**
 * \if ENGLISH
 * @brief Set the value
 * @param[in] x X value
 * @param[in] y Y value
 * \endif
 *
 * \if CHINESE
 * @brief 设置值
 * @param[in] x x值
 * @param[in] y y值
 * \endif
 */
void QwtPlotMarker::setValue( double x, double y )
{
    if ( x != m_data->xValue || y != m_data->yValue )
    {
        m_data->xValue = x;
        m_data->yValue = y;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set the x-value
 * @param[in] x X value
 * \endif
 *
 * \if CHINESE
 * @brief 设置x值
 * @param[in] x x值
 * \endif
 */
void QwtPlotMarker::setXValue( double x )
{
    setValue( x, m_data->yValue );
}

/**
 * \if ENGLISH
 * @brief Set the y-value
 * @param[in] y Y value
 * \endif
 *
 * \if CHINESE
 * @brief 设置y值
 * @param[in] y y值
 * \endif
 */
void QwtPlotMarker::setYValue( double y )
{
    setValue( m_data->xValue, y );
}

/**
 * \if ENGLISH
 * @brief Draw the marker
 * @param[in] painter Painter
 * @param[in] xMap X Scale Map
 * @param[in] yMap Y Scale Map
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制标记
 * @param[in] painter 绘图器
 * @param[in] xMap X比例映射
 * @param[in] yMap Y比例映射
 * @param[in] canvasRect 画布的内容矩形（绘图器坐标）
 * \endif
 */
void QwtPlotMarker::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    const QPointF pos( xMap.transform( m_data->xValue ),
        yMap.transform( m_data->yValue ) );

    drawLines( painter, canvasRect, pos );
    drawSymbol( painter, canvasRect, pos );
    drawLabel( painter, canvasRect, pos );
}

/*!
   Draw the lines marker

   \param painter Painter
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param pos Position of the marker, translated into widget coordinates

   \sa drawLabel(), drawSymbol()
 */
void QwtPlotMarker::drawLines( QPainter* painter,
    const QRectF& canvasRect, const QPointF& pos ) const
{
    if ( m_data->style == NoLine )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    painter->setPen( m_data->pen );
    if ( m_data->style == QwtPlotMarker::HLine ||
        m_data->style == QwtPlotMarker::Cross )
    {
        double y = pos.y();
        if ( doAlign )
            y = qRound( y );

        QwtPainter::drawLine( painter, canvasRect.left(),
            y, canvasRect.right() - 1.0, y );
    }
    if ( m_data->style == QwtPlotMarker::VLine ||
        m_data->style == QwtPlotMarker::Cross )
    {
        double x = pos.x();
        if ( doAlign )
            x = qRound( x );

        QwtPainter::drawLine( painter, x,
            canvasRect.top(), x, canvasRect.bottom() - 1.0 );
    }
}

/*!
   Draw the symbol of the marker

   \param painter Painter
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param pos Position of the marker, translated into widget coordinates

   \sa drawLabel(), QwtSymbol::drawSymbol()
 */
void QwtPlotMarker::drawSymbol( QPainter* painter,
    const QRectF& canvasRect, const QPointF& pos ) const
{
    if ( m_data->symbol == nullptr )
        return;

    const QwtSymbol& symbol = *m_data->symbol;

    if ( symbol.style() != QwtSymbol::NoSymbol )
    {
        const QSizeF sz = symbol.size();

        const QRectF clipRect = canvasRect.adjusted(
            -sz.width(), -sz.height(), sz.width(), sz.height() );

        if ( clipRect.contains( pos ) )
            symbol.drawSymbol( painter, pos );
    }
}

/*!
   Align and draw the text label of the marker

   \param painter Painter
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param pos Position of the marker, translated into widget coordinates

   \sa drawLabel(), drawSymbol()
 */
void QwtPlotMarker::drawLabel( QPainter* painter,
    const QRectF& canvasRect, const QPointF& pos ) const
{
    if ( m_data->label.isEmpty() )
        return;

    Qt::Alignment align = m_data->labelAlignment;
    QPointF alignPos = pos;

    QSizeF symbolOff( 0, 0 );

    switch ( m_data->style )
    {
        case QwtPlotMarker::VLine:
        {
            // In VLine-style the y-position is pointless and
            // the alignment flags are relative to the canvas

            if ( m_data->labelAlignment & Qt::AlignTop )
            {
                alignPos.setY( canvasRect.top() );
                align &= ~Qt::AlignTop;
                align |= Qt::AlignBottom;
            }
            else if ( m_data->labelAlignment & Qt::AlignBottom )
            {
                // In HLine-style the x-position is pointless and
                // the alignment flags are relative to the canvas

                alignPos.setY( canvasRect.bottom() - 1 );
                align &= ~Qt::AlignBottom;
                align |= Qt::AlignTop;
            }
            else
            {
                alignPos.setY( canvasRect.center().y() );
            }
            break;
        }
        case QwtPlotMarker::HLine:
        {
            if ( m_data->labelAlignment & Qt::AlignLeft )
            {
                alignPos.setX( canvasRect.left() );
                align &= ~Qt::AlignLeft;
                align |= Qt::AlignRight;
            }
            else if ( m_data->labelAlignment & Qt::AlignRight )
            {
                alignPos.setX( canvasRect.right() - 1 );
                align &= ~Qt::AlignRight;
                align |= Qt::AlignLeft;
            }
            else
            {
                alignPos.setX( canvasRect.center().x() );
            }
            break;
        }
        default:
        {
            if ( m_data->symbol &&
                ( m_data->symbol->style() != QwtSymbol::NoSymbol ) )
            {
                symbolOff = m_data->symbol->size() + QSizeF( 1, 1 );
                symbolOff /= 2;
            }
        }
    }

    qreal pw2 = m_data->pen.widthF() / 2.0;
    if ( pw2 == 0.0 )
        pw2 = 0.5;

    const int spacing = m_data->spacing;

    const qreal xOff = qwtMaxF( pw2, symbolOff.width() );
    const qreal yOff = qwtMaxF( pw2, symbolOff.height() );

    const QSizeF textSize = m_data->label.textSize( painter->font() );

    if ( align & Qt::AlignLeft )
    {
        alignPos.rx() -= xOff + spacing;
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.rx() -= textSize.height();
        else
            alignPos.rx() -= textSize.width();
    }
    else if ( align & Qt::AlignRight )
    {
        alignPos.rx() += xOff + spacing;
    }
    else
    {
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.rx() -= textSize.height() / 2;
        else
            alignPos.rx() -= textSize.width() / 2;
    }

    if ( align & Qt::AlignTop )
    {
        alignPos.ry() -= yOff + spacing;
        if ( m_data->labelOrientation != Qt::Vertical )
            alignPos.ry() -= textSize.height();
    }
    else if ( align & Qt::AlignBottom )
    {
        alignPos.ry() += yOff + spacing;
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.ry() += textSize.width();
    }
    else
    {
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.ry() += textSize.width() / 2;
        else
            alignPos.ry() -= textSize.height() / 2;
    }

    painter->translate( alignPos.x(), alignPos.y() );
    if ( m_data->labelOrientation == Qt::Vertical )
        painter->rotate( -90.0 );

    const QRectF textRect( 0, 0, textSize.width(), textSize.height() );
    m_data->label.draw( painter, textRect );
}

/**
 * \if ENGLISH
 * @brief Set the line style
 * @param[in] style Line style
 * @sa lineStyle()
 * \endif
 *
 * \if CHINESE
 * @brief 设置线条样式
 * @param[in] style 线条样式
 * @sa lineStyle()
 * \endif
 */
void QwtPlotMarker::setLineStyle( LineStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the line style
 * @return Line style
 * @sa setLineStyle()
 * \endif
 *
 * \if CHINESE
 * @brief 获取线条样式
 * @return 线条样式
 * @sa setLineStyle()
 * \endif
 */
QwtPlotMarker::LineStyle QwtPlotMarker::lineStyle() const
{
    return m_data->style;
}

/**
 * \if ENGLISH
 * @brief Assign a symbol
 * @param[in] symbol New symbol
 * @sa symbol()
 * \endif
 *
 * \if CHINESE
 * @brief 分配符号
 * @param[in] symbol 新符号
 * @sa symbol()
 * \endif
 */
void QwtPlotMarker::setSymbol( const QwtSymbol* symbol )
{
    if ( symbol != m_data->symbol )
    {
        delete m_data->symbol;
        m_data->symbol = symbol;

        if ( symbol )
            setLegendIconSize( symbol->boundingRect().size() );

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the symbol
 * @return The symbol
 * @sa setSymbol(), QwtSymbol
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号
 * @return 符号
 * @sa setSymbol(), QwtSymbol
 * \endif
 */
const QwtSymbol* QwtPlotMarker::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Set the label
 * @param[in] label Label text
 * @sa label()
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签
 * @param[in] label 标签文本
 * @sa label()
 * \endif
 */
void QwtPlotMarker::setLabel( const QwtText& label )
{
    if ( label != m_data->label )
    {
        m_data->label = label;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the label
 * @return Label text
 * @sa setLabel()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标签
 * @return 标签文本
 * @sa setLabel()
 * \endif
 */
QwtText QwtPlotMarker::label() const
{
    return m_data->label;
}

/**
 * \if ENGLISH
 * @brief Set the alignment of the label
 * @details In case of QwtPlotMarker::HLine the alignment is relative to the
 *          y position of the marker, but the horizontal flags correspond to the
 *          canvas rectangle. In case of QwtPlotMarker::VLine the alignment is
 *          relative to the x position of the marker, but the vertical flags
 *          correspond to the canvas rectangle.
 *          In all other styles the alignment is relative to the marker's position.
 * @param[in] align Alignment
 * @sa labelAlignment(), labelOrientation()
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签的对齐方式
 * @details 对于 QwtPlotMarker::HLine，对齐相对于标记的 y 位置，但水平标志对应于画布矩形。
 *          对于 QwtPlotMarker::VLine，对齐相对于标记的 x 位置，但垂直标志对应于画布矩形。
 *          对于所有其他样式，对齐相对于标记的位置。
 * @param[in] align 对齐方式
 * @sa labelAlignment(), labelOrientation()
 * \endif
 */
void QwtPlotMarker::setLabelAlignment( Qt::Alignment align )
{
    if ( align != m_data->labelAlignment )
    {
        m_data->labelAlignment = align;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the label alignment
 * @return Label alignment
 * @sa setLabelAlignment(), setLabelOrientation()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标签对齐方式
 * @return 标签对齐方式
 * @sa setLabelAlignment(), setLabelOrientation()
 * \endif
 */
Qt::Alignment QwtPlotMarker::labelAlignment() const
{
    return m_data->labelAlignment;
}

/**
 * \if ENGLISH
 * @brief Set the orientation of the label
 * @details When orientation is Qt::Vertical the label is rotated by 90.0 degrees
 *          (from bottom to top).
 * @param[in] orientation Orientation of the label
 * @sa labelOrientation(), setLabelAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签的方向
 * @details 当方向为 Qt::Vertical 时，标签旋转 90.0 度（从下到上）。
 * @param[in] orientation 标签方向
 * @sa labelOrientation(), setLabelAlignment()
 * \endif
 */
void QwtPlotMarker::setLabelOrientation( Qt::Orientation orientation )
{
    if ( orientation != m_data->labelOrientation )
    {
        m_data->labelOrientation = orientation;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the label orientation
 * @return Label orientation
 * @sa setLabelOrientation(), labelAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标签方向
 * @return 标签方向
 * @sa setLabelOrientation(), labelAlignment()
 * \endif
 */
Qt::Orientation QwtPlotMarker::labelOrientation() const
{
    return m_data->labelOrientation;
}

/**
 * \if ENGLISH
 * @brief Set the spacing
 * @details When the label is not centered on the marker position, the spacing
 *          is the distance between the position and the label.
 * @param[in] spacing Spacing
 * @sa spacing(), setLabelAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 设置间距
 * @details 当标签不在标记位置居中时，间距是位置和标签之间的距离。
 * @param[in] spacing 间距
 * @sa spacing(), setLabelAlignment()
 * \endif
 */
void QwtPlotMarker::setSpacing( int spacing )
{
    if ( spacing < 0 )
        spacing = 0;

    if ( spacing == m_data->spacing )
        return;

    m_data->spacing = spacing;
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the spacing
 * @return Spacing
 * @sa setSpacing()
 * \endif
 *
 * \if CHINESE
 * @brief 获取间距
 * @return 间距
 * @sa setSpacing()
 * \endif
 */
int QwtPlotMarker::spacing() const
{
    return m_data->spacing;
}

/**
 * \if ENGLISH
 * @brief Build and assign a line pen
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) which makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并分配线条画笔
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），使其非装饰性
 *          （见 QPen::isCosmetic()）。此方法用于隐藏此不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotMarker::setLinePen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setLinePen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Specify a pen for the line
 * @param[in] pen New pen
 * @sa linePen()
 * \endif
 *
 * \if CHINESE
 * @brief 指定线条的画笔
 * @param[in] pen 新画笔
 * @sa linePen()
 * \endif
 */
void QwtPlotMarker::setLinePen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the line pen
 * @return Line pen
 * @sa setLinePen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取线条画笔
 * @return 线条画笔
 * @sa setLinePen()
 * \endif
 */
const QPen& QwtPlotMarker::linePen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle
 * @return Bounding rectangle
 * @details width/height of -1 does not affect the autoscale calculation
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界矩形
 * @return 边界矩形
 * @details -1的宽度/高度不影响自动缩放计算
 * \endif
 */
QRectF QwtPlotMarker::boundingRect() const
{
    // width/height of -1 does not affect the autoscale calculation

    switch (m_data->style)
    {
        case QwtPlotMarker::HLine:
            return QRectF( m_data->xValue, m_data->yValue, -1.0, 0.0 );

        case QwtPlotMarker::VLine:
            return QRectF( m_data->xValue, m_data->yValue, 0.0, -1.0 );

        default:
            return QRectF( m_data->xValue, m_data->yValue, 0.0, 0.0 );
    }
}

/**
 * \if ENGLISH
 * @brief Get the icon representing the marker on the legend
 * @param[in] index Index of the legend entry (usually there is only one)
 * @param[in] size Icon size
 * @return Icon representing the marker on the legend
 * @sa setLegendIconSize(), legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例上代表标记的图标
 * @param[in] index 图例条目的索引（通常只有一个）
 * @param[in] size 图标大小
 * @return 图例上代表标记的图标
 * @sa setLegendIconSize(), legendData()
 * \endif
 */
QwtGraphic QwtPlotMarker::legendIcon( int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    if ( size.isEmpty() )
        return QwtGraphic();

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    if ( m_data->style != QwtPlotMarker::NoLine )
    {
        painter.setPen( m_data->pen );

        if ( m_data->style == QwtPlotMarker::HLine ||
            m_data->style == QwtPlotMarker::Cross )
        {
            const double y = 0.5 * size.height();

            QwtPainter::drawLine( &painter,
                0.0, y, size.width(), y );
        }

        if ( m_data->style == QwtPlotMarker::VLine ||
            m_data->style == QwtPlotMarker::Cross )
        {
            const double x = 0.5 * size.width();

            QwtPainter::drawLine( &painter,
                x, 0.0, x, size.height() );
        }
    }

    if ( m_data->symbol )
    {
        const QRect r( 0.0, 0.0, size.width(), size.height() );
        m_data->symbol->drawSymbol( &painter, r );
    }

    return icon;
}

