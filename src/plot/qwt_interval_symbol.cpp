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

#include "qwt_interval_symbol.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qmath.h>

class QwtIntervalSymbol::PrivateData
{
  public:
    PrivateData()
        : style( QwtIntervalSymbol::NoSymbol )
        , width( 6 )
    {
    }

    bool operator==( const PrivateData& other ) const
    {
        return ( style == other.style )
               && ( width == other.width )
               && ( brush == other.brush )
               && ( pen == other.pen );
    }

    QwtIntervalSymbol::Style style;
    int width;

    QPen pen;
    QBrush brush;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] style Style of the symbol
 * @sa setStyle(), style(), Style
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @param[in] style 符号的样式
 * @sa setStyle(), style(), Style
 * \endif
 */
QwtIntervalSymbol::QwtIntervalSymbol( Style style )
{
    m_data = new PrivateData();
    m_data->style = style;
}

/**
 * \if ENGLISH
 * @brief Copy constructor
 * @param[in] other Symbol to copy
 * \endif
 * \if CHINESE
 * @brief 拷贝构造函数
 * @param[in] other 要拷贝的符号
 * \endif
 */
QwtIntervalSymbol::QwtIntervalSymbol( const QwtIntervalSymbol& other )
{
    m_data = new PrivateData();
    *m_data = *other.m_data;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtIntervalSymbol::~QwtIntervalSymbol()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Assignment operator
 * @param[in] other Symbol to assign
 * @return Reference to this symbol
 * \endif
 * \if CHINESE
 * @brief 赋值运算符
 * @param[in] other 要赋值的符号
 * @return 本符号的引用
 * \endif
 */
QwtIntervalSymbol& QwtIntervalSymbol::operator=(
    const QwtIntervalSymbol& other )
{
    *m_data = *other.m_data;
    return *this;
}

/**
 * \if ENGLISH
 * @brief Compare two symbols
 * @param[in] other Symbol to compare
 * @return True if symbols are equal
 * \endif
 * \if CHINESE
 * @brief 比较两个符号
 * @param[in] other 要比较的符号
 * @return 如果符号相等则返回 true
 * \endif
 */
bool QwtIntervalSymbol::operator==(
    const QwtIntervalSymbol& other ) const
{
    return *m_data == *other.m_data;
}

/**
 * \if ENGLISH
 * @brief Compare two symbols
 * @param[in] other Symbol to compare
 * @return True if symbols are not equal
 * \endif
 * \if CHINESE
 * @brief 比较两个符号
 * @param[in] other 要比较的符号
 * @return 如果符号不相等则返回 true
 * \endif
 */
bool QwtIntervalSymbol::operator!=(
    const QwtIntervalSymbol& other ) const
{
    return !( *m_data == *other.m_data );
}

/**
 * \if ENGLISH
 * @brief Specify the symbol style
 * @param[in] style Style
 * @sa style(), Style
 * \endif
 * \if CHINESE
 * @brief 指定符号样式
 * @param[in] style 样式
 * @sa style(), Style
 * \endif
 */
void QwtIntervalSymbol::setStyle( Style style )
{
    m_data->style = style;
}

/**
 * \if ENGLISH
 * @brief Get the current symbol style
 * @return Current symbol style
 * @sa setStyle()
 * \endif
 * \if CHINESE
 * @brief 获取当前符号样式
 * @return 当前符号样式
 * @sa setStyle()
 * \endif
 */
QwtIntervalSymbol::Style QwtIntervalSymbol::style() const
{
    return m_data->style;
}

/**
 * \if ENGLISH
 * @brief Specify the width of the symbol
 * @details It is used depending on the style.
 * @param[in] width Width
 * @sa width(), setStyle()
 * \endif
 * \if CHINESE
 * @brief 指定符号的宽度
 * @details 根据样式使用。
 * @param[in] width 宽度
 * @sa width(), setStyle()
 * \endif
 */
void QwtIntervalSymbol::setWidth( int width )
{
    m_data->width = width;
}

/**
 * \if ENGLISH
 * @brief Get the width of the symbol
 * @return Width of the symbol
 * @sa setWidth(), setStyle()
 * \endif
 * \if CHINESE
 * @brief 获取符号的宽度
 * @return 符号的宽度
 * @sa setWidth(), setStyle()
 * \endif
 */
int QwtIntervalSymbol::width() const
{
    return m_data->width;
}

/**
 * \if ENGLISH
 * @brief Assign a brush
 * @details The brush is used for the Box style.
 * @param[in] brush Brush
 * @sa brush()
 * \endif
 * \if CHINESE
 * @brief 分配画刷
 * @details 画刷用于 Box 样式。
 * @param[in] brush 画刷
 * @sa brush()
 * \endif
 */
void QwtIntervalSymbol::setBrush( const QBrush& brush )
{
    m_data->brush = brush;
}

/**
 * \if ENGLISH
 * @brief Get the brush
 * @return Brush
 * @sa setBrush()
 * \endif
 * \if CHINESE
 * @brief 获取画刷
 * @return 画刷
 * @sa setBrush()
 * \endif
 */
const QBrush& QwtIntervalSymbol::brush() const
{
    return m_data->brush;
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen
 * @details In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
 *          non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa pen(), brush()
 * \endif
 * \if CHINESE
 * @brief 构建并分配画笔
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），这使得它
 *          不是装饰性的（参见 QPen::isCosmetic()）。此方法是为了
 *          隐藏此不兼容性而引入的。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa pen(), brush()
 * \endif
 */
void QwtIntervalSymbol::setPen( const QColor& color,
    qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Assign a pen
 * @param[in] pen Pen
 * @sa pen(), setBrush()
 * \endif
 * \if CHINESE
 * @brief 分配画笔
 * @param[in] pen 画笔
 * @sa pen(), setBrush()
 * \endif
 */
void QwtIntervalSymbol::setPen( const QPen& pen )
{
    m_data->pen = pen;
}

/**
 * \if ENGLISH
 * @brief Get the pen
 * @return Pen
 * @sa setPen(), brush()
 * \endif
 * \if CHINESE
 * @brief 获取画笔
 * @return 画笔
 * @sa setPen(), brush()
 * \endif
 */
const QPen& QwtIntervalSymbol::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Draw a symbol depending on its style
 * @param[in] painter Painter
 * @param[in] orientation Orientation
 * @param[in] from Start point of the interval in target device coordinates
 * @param[in] to End point of the interval in target device coordinates
 * @sa setStyle()
 * \endif
 * \if CHINESE
 * @brief 根据样式绘制符号
 * @param[in] painter 绘图器
 * @param[in] orientation 方向
 * @param[in] from 区间起点（目标设备坐标）
 * @param[in] to 区间终点（目标设备坐标）
 * @sa setStyle()
 * \endif
 */
void QwtIntervalSymbol::draw( QPainter* painter, Qt::Orientation orientation,
    const QPointF& from, const QPointF& to ) const
{
    const qreal pw = QwtPainter::effectivePenWidth( painter->pen() );

    QPointF p1 = from;
    QPointF p2 = to;
    if ( QwtPainter::roundingAlignment( painter ) )
    {
        p1 = p1.toPoint();
        p2 = p2.toPoint();
    }

    switch ( m_data->style )
    {
        case QwtIntervalSymbol::Bar:
        {
            QwtPainter::drawLine( painter, p1, p2 );
            if ( m_data->width > pw )
            {
                if ( ( orientation == Qt::Horizontal )
                    && ( p1.y() == p2.y() ) )
                {
                    const double sw = m_data->width;

                    const double y = p1.y() - sw / 2;
                    QwtPainter::drawLine( painter,
                        p1.x(), y, p1.x(), y + sw );
                    QwtPainter::drawLine( painter,
                        p2.x(), y, p2.x(), y + sw );
                }
                else if ( ( orientation == Qt::Vertical )
                    && ( p1.x() == p2.x() ) )
                {
                    const double sw = m_data->width;

                    const double x = p1.x() - sw / 2;
                    QwtPainter::drawLine( painter,
                        x, p1.y(), x + sw, p1.y() );
                    QwtPainter::drawLine( painter,
                        x, p2.y(), x + sw, p2.y() );
                }
                else
                {
                    const double sw = m_data->width;

                    const double dx = p2.x() - p1.x();
                    const double dy = p2.y() - p1.y();
                    const double angle = std::atan2( dy, dx ) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const double cx = qFastCos( angle ) * dw2;
                    const double sy = qFastSin( angle ) * dw2;

                    QwtPainter::drawLine( painter,
                        p1.x() - cx, p1.y() - sy,
                        p1.x() + cx, p1.y() + sy );
                    QwtPainter::drawLine( painter,
                        p2.x() - cx, p2.y() - sy,
                        p2.x() + cx, p2.y() + sy );
                }
            }
            break;
        }
        case QwtIntervalSymbol::Box:
        {
            if ( m_data->width <= pw )
            {
                QwtPainter::drawLine( painter, p1, p2 );
            }
            else
            {
                if ( ( orientation == Qt::Horizontal )
                    && ( p1.y() == p2.y() ) )
                {
                    const double sw = m_data->width;

                    const double y = p1.y() - m_data->width / 2;
                    QwtPainter::drawRect( painter,
                        p1.x(), y, p2.x() - p1.x(),  sw );
                }
                else if ( ( orientation == Qt::Vertical )
                    && ( p1.x() == p2.x() ) )
                {
                    const double sw = m_data->width;

                    const double x = p1.x() - m_data->width / 2;
                    QwtPainter::drawRect( painter,
                        x, p1.y(), sw, p2.y() - p1.y() );
                }
                else
                {
                    const double sw = m_data->width;

                    const double dx = p2.x() - p1.x();
                    const double dy = p2.y() - p1.y();
                    const double angle = std::atan2( dy, dx ) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const double cx = qFastCos( angle ) * dw2;
                    const double sy = qFastSin( angle ) * dw2;

                    QPolygonF polygon;
                    polygon += QPointF( p1.x() - cx, p1.y() - sy );
                    polygon += QPointF( p1.x() + cx, p1.y() + sy );
                    polygon += QPointF( p2.x() + cx, p2.y() + sy );
                    polygon += QPointF( p2.x() - cx, p2.y() - sy );

                    QwtPainter::drawPolygon( painter, polygon );
                }
            }
            break;
        }
        default:;
    }
}
