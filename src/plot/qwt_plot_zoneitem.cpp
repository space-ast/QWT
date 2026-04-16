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

#include "qwt_plot_zoneitem.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_interval.h"

#include <qpainter.h>

class QwtPlotZoneItem::PrivateData
{
  public:
    PrivateData()
        : orientation( Qt::Vertical )
        , pen( Qt::NoPen )
    {
        QColor c( Qt::darkGray );
        c.setAlpha( 100 );
        brush = QBrush( c );
    }

    Qt::Orientation orientation;
    QPen pen;
    QBrush brush;
    QwtInterval interval;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Initializes the zone with no pen and a semi transparent gray brush.
 *          Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: false
 *          - QwtPlotItem::Legend: false
 *          The z value is initialized by 5.
 * @sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 初始化区域，无画笔和半透明灰色画刷。
 *          设置以下项目属性：
 *          - QwtPlotItem::AutoScale: false
 *          - QwtPlotItem::Legend: false
 *          z 值初始化为 5。
 * @sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 * \endif
 */
QwtPlotZoneItem::QwtPlotZoneItem()
    : QwtPlotItem( QwtText( "Zone" ) )
{
    m_data = new PrivateData;

    setItemAttribute( QwtPlotItem::AutoScale, false );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 5 );
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
QwtPlotZoneItem::~QwtPlotZoneItem()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotZone
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotZone
 * \endif
 */
int QwtPlotZoneItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotZone;
}

/**
 * \if ENGLISH
 * @brief Build and assign a pen
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @details In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it non cosmetic.
 *          This method has been introduced to hide this incompatibility.
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 构建并分配画笔
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @details 在 Qt5 中，默认画笔宽度为 1.0（Qt4 中为 0.0），使其非装饰性。
 *          此方法已引入以隐藏此不兼容性。
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotZoneItem::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/**
 * \if ENGLISH
 * @brief Assign a pen
 * @param[in] pen Pen
 * @details The pen is used to draw the border lines of the zone.
 * @sa pen(), setBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 分配画笔
 * @param[in] pen 画笔
 * @details 画笔用于绘制区域的边界线。
 * @sa pen(), setBrush()
 * \endif
 */
void QwtPlotZoneItem::setPen( const QPen& pen )
{
    if ( m_data->pen != pen )
    {
        m_data->pen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen used to draw the border lines
 * @return Pen used to draw the border lines
 * @sa setPen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制边界线的画笔
 * @return 用于绘制边界线的画笔
 * @sa setPen(), brush()
 * \endif
 */
const QPen& QwtPlotZoneItem::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Assign a brush
 * @param[in] brush Brush
 * @details The brush is used to fill the zone.
 * @sa pen(), setBrush()
 * \endif
 *
 * \if CHINESE
 * @brief 分配画刷
 * @param[in] brush 画刷
 * @details 画刷用于填充区域。
 * @sa pen(), setBrush()
 * \endif
 */
void QwtPlotZoneItem::setBrush( const QBrush& brush )
{
    if ( m_data->brush != brush )
    {
        m_data->brush = brush;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the brush used to fill the zone
 * @return Brush used to fill the zone
 * @sa setPen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于填充区域的画刷
 * @return 用于填充区域的画刷
 * @sa setPen(), brush()
 * \endif
 */
const QBrush& QwtPlotZoneItem::brush() const
{
    return m_data->brush;
}

/**
 * \if ENGLISH
 * @brief Set the orientation of the zone
 * @param[in] orientation Orientation
 * @details A horizontal zone highlights an interval of the y axis,
 *          a vertical zone of the x axis. It is unbounded in the opposite direction.
 * @sa orientation(), QwtPlotItem::setAxes()
 * \endif
 *
 * \if CHINESE
 * @brief 设置区域的方向
 * @param[in] orientation 方向
 * @details 水平区域高亮显示 y 轴的一个区间，垂直区域高亮显示 x 轴的一个区间，
 *          在相反方向上无限延伸。
 * @sa orientation(), QwtPlotItem::setAxes()
 * \endif
 */
void QwtPlotZoneItem::setOrientation( Qt::Orientation orientation )
{
    if ( m_data->orientation != orientation )
    {
        m_data->orientation = orientation;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the orientation of the zone
 * @return Orientation of the zone
 * @sa setOrientation()
 * \endif
 *
 * \if CHINESE
 * @brief 获取区域的方向
 * @return 区域的方向
 * @sa setOrientation()
 * \endif
 */
Qt::Orientation QwtPlotZoneItem::orientation() const
{
    return m_data->orientation;
}

/**
 * \if ENGLISH
 * @brief Set the interval of the zone
 * @param[in] min Minimum of the interval
 * @param[in] max Maximum of the interval
 * @details For a horizontal zone the interval is related to the y axis,
 *          for a vertical zone it is related to the x axis.
 * @sa interval(), setOrientation()
 * \endif
 *
 * \if CHINESE
 * @brief 设置区域的区间
 * @param[in] min 区间的最小值
 * @param[in] max 区间的最大值
 * @details 对于水平区域，区间与 y 轴相关，对于垂直区域，区间与 x 轴相关。
 * @sa interval(), setOrientation()
 * \endif
 */
void QwtPlotZoneItem::setInterval( double min, double max )
{
    setInterval( QwtInterval( min, max ) );
}

/**
 * \if ENGLISH
 * @brief Set the interval of the zone
 * @param[in] interval Zone interval
 * @details For a horizontal zone the interval is related to the y axis,
 *          for a vertical zone it is related to the x axis.
 * @sa interval(), setOrientation()
 * \endif
 *
 * \if CHINESE
 * @brief 设置区域的区间
 * @param[in] interval 区域区间
 * @details 对于水平区域，区间与 y 轴相关，对于垂直区域，区间与 x 轴相关。
 * @sa interval(), setOrientation()
 * \endif
 */
void QwtPlotZoneItem::setInterval( const QwtInterval& interval )
{
    if ( m_data->interval != interval )
    {
        m_data->interval = interval;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the interval of the zone
 * @return Zone interval
 * @sa setInterval(), orientation()
 * \endif
 *
 * \if CHINESE
 * @brief 获取区域的区间
 * @return 区域区间
 * @sa setInterval(), orientation()
 * \endif
 */
QwtInterval QwtPlotZoneItem::interval() const
{
    return m_data->interval;
}

/**
 * \if ENGLISH
 * @brief Draw the zone
 * @param[in] painter Painter
 * @param[in] xMap x Scale Map
 * @param[in] yMap y Scale Map
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制区域
 * @param[in] painter 画笔
 * @param[in] xMap x 轴比例映射
 * @param[in] yMap y 轴比例映射
 * @param[in] canvasRect 画布的内容矩形（画笔坐标）
 * \endif
 */
void QwtPlotZoneItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    if ( !m_data->interval.isValid() )
        return;

    QPen pen = m_data->pen;
    pen.setCapStyle( Qt::FlatCap );

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    if ( m_data->orientation == Qt::Horizontal )
    {
        double y1 = yMap.transform( m_data->interval.minValue() );
        double y2 = yMap.transform( m_data->interval.maxValue() );

        if ( doAlign )
        {
            y1 = qRound( y1 );
            y2 = qRound( y2 );
        }

        QRectF r( canvasRect.left(), y1, canvasRect.width(), y2 - y1 );
        r = r.normalized();

        if ( ( m_data->brush.style() != Qt::NoBrush ) && ( y1 != y2 ) )
        {
            QwtPainter::fillRect( painter, r, m_data->brush );
        }

        if ( m_data->pen.style() != Qt::NoPen )
        {
            painter->setPen( m_data->pen );

            QwtPainter::drawLine( painter, r.left(), r.top(), r.right(), r.top() );
            QwtPainter::drawLine( painter, r.left(), r.bottom(), r.right(), r.bottom() );
        }
    }
    else
    {
        double x1 = xMap.transform( m_data->interval.minValue() );
        double x2 = xMap.transform( m_data->interval.maxValue() );

        if ( doAlign )
        {
            x1 = qRound( x1 );
            x2 = qRound( x2 );
        }

        QRectF r( x1, canvasRect.top(), x2 - x1, canvasRect.height() );
        r = r.normalized();

        if ( ( m_data->brush.style() != Qt::NoBrush ) && ( x1 != x2 ) )
        {
            QwtPainter::fillRect( painter, r, m_data->brush );
        }

        if ( m_data->pen.style() != Qt::NoPen )
        {
            painter->setPen( m_data->pen );

            QwtPainter::drawLine( painter, r.left(), r.top(), r.left(), r.bottom() );
            QwtPainter::drawLine( painter, r.right(), r.top(), r.right(), r.bottom() );
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle
 * @details The bounding rectangle is built from the interval in one direction
 *          and something invalid for the opposite direction.
 * @return An invalid rectangle with valid boundaries in one direction
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界矩形
 * @details 边界矩形由一个方向的区间和相反方向的无效值构成。
 * @return 一个在一个方向上有有效边界的无效矩形
 * \endif
 */
QRectF QwtPlotZoneItem::boundingRect() const
{
    QRectF br = QwtPlotItem::boundingRect();

    const QwtInterval& intv = m_data->interval;

    if ( intv.isValid() )
    {
        if ( m_data->orientation == Qt::Horizontal )
        {
            br.setTop( intv.minValue() );
            br.setBottom( intv.maxValue() );
        }
        else
        {
            br.setLeft( intv.minValue() );
            br.setRight( intv.maxValue() );
        }
    }

    return br;
}
