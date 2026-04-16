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

#include "qwt_plot_vectorfield.h"
#include "qwt_vectorfield_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qdebug.h>
#include <cstdlib>
#include <limits>

#define DEBUG_RENDER 0

#if DEBUG_RENDER
#include <qelapsedtimer.h>
#endif


static inline double qwtVector2Radians( double vx, double vy )
{
    if ( vx == 0.0 )
        return ( vy >= 0 ) ? M_PI_2 : 3 * M_PI_2;

    return std::atan2( vy, vx );
}

static inline double qwtVector2Magnitude( double vx, double vy )
{
    return sqrt( vx * vx + vy * vy );
}

static QwtInterval qwtMagnitudeRange(
    const QwtSeriesData< QwtVectorFieldSample >* series )
{
    if ( series->size() == 0 )
        return QwtInterval( 0, 1 );

    const QwtVectorFieldSample s0 = series->sample( 0 );

    double min = s0.vx * s0.vx + s0.vy * s0.vy;
    double max = min;

    for ( uint i = 1; i < series->size(); i++ )
    {
        const QwtVectorFieldSample s = series->sample( i );
        const double l = s.vx * s.vx + s.vy * s.vy;

        if ( l < min )
            min = l;

        if ( l > max )
            max = l;
    }

    min = std::sqrt( min );
    max = std::sqrt( max );

    if ( max == min )
        max += 1.0;

    return QwtInterval( min, max );
}

static inline QTransform qwtSymbolTransformation(
    const QTransform& oldTransform, double x, double y,
    double vx, double vy, double magnitude )
{
    QTransform transform = oldTransform;

    if ( !transform.isIdentity() )
    {
        transform.translate( x, y );

        const double radians = qwtVector2Radians( vx, vy );
        transform.rotateRadians( radians );
    }
    else
    {
        /*
            When starting with no transformation ( f.e on screen )
            the matrix can be found without having to use
            trigonometric functions
         */

        qreal sin, cos;
        if ( magnitude == 0.0 )
        {
            // something
            sin = 1.0;
            cos = 0.0;
        }
        else
        {
            sin = vy / magnitude;
            cos = vx / magnitude;
        }

        transform.setMatrix( cos, sin, 0.0, -sin, cos, 0.0, x, y, 1.0 );
    }

    return transform;
}

namespace
{
    class FilterMatrix
    {
      public:
        class Entry
        {
          public:
            inline void addSample( double sx, double sy,
                double svx, double svy )
            {
                x += sx;
                y += sy;

                vx += svx;
                vy += svy;

                count++;
            }

            quint32 count;

            // screen positions -> float is good enough
            float x;
            float y;
            float vx;
            float vy;
        };

        FilterMatrix( const QRectF& dataRect,
            const QRectF& canvasRect, const QSizeF& cellSize )
        {
            m_dx = cellSize.width();
            m_dy = cellSize.height();

            m_x0 = dataRect.x();
            if ( m_x0 < canvasRect.x() )
                m_x0 += int( ( canvasRect.x() - m_x0 ) / m_dx ) * m_dx;

            m_y0 = dataRect.y();
            if ( m_y0 < canvasRect.y() )
                m_y0 += int( ( canvasRect.y() - m_y0 ) / m_dy ) * m_dy;

            m_numColumns = canvasRect.width() / m_dx + 1;
            m_numRows = canvasRect.height() / m_dy + 1;

#if 1
            /*
                limit column and row count to a maximum of 1000000,
                so that memory usage is not an issue
             */
            if ( m_numColumns > 1000 )
            {
                m_dx = canvasRect.width() / 1000;
                m_numColumns = canvasRect.width() / m_dx + 1;
            }

            if ( m_numRows > 1000 )
            {
                m_dy = canvasRect.height() / 1000;
                m_numRows = canvasRect.height() / m_dx + 1;
            }
#endif

            m_x1 = m_x0 + m_numColumns * m_dx;
            m_y1 = m_y0 + m_numRows * m_dy;

            m_entries = ( Entry* )::calloc( m_numRows * m_numColumns, sizeof( Entry ) );
            if ( m_entries == nullptr )
            {
                qWarning() << "QwtPlotVectorField: raster for filtering too fine - running out of memory";
            }
        }

        ~FilterMatrix()
        {
            if ( m_entries )
                std::free( m_entries );
        }

        inline int numColumns() const
        {
            return m_numColumns;
        }

        inline int numRows() const
        {
            return m_numRows;
        }

        inline void addSample( double x, double y,
            double u, double v )
        {
            if ( x >= m_x0 && x < m_x1
                && y >= m_y0 && y < m_y1 )
            {
                Entry& entry = m_entries[ indexOf( x, y ) ];
                entry.addSample( x, y, u, v );
            }
        }

        const FilterMatrix::Entry* entries() const
        {
            return m_entries;
        }

      private:
        inline int indexOf( qreal x, qreal y ) const
        {
            const int col = ( x - m_x0 ) / m_dx;
            const int row = ( y - m_y0 ) / m_dy;

            return row * m_numColumns + col;
        }

        qreal m_x0, m_x1, m_y0, m_y1, m_dx, m_dy;
        int m_numColumns;
        int m_numRows;

        Entry* m_entries;
    };
}

class QwtPlotVectorField::PrivateData
{
  public:
    PrivateData()
        : pen( Qt::black )
        , brush( Qt::black )
        , indicatorOrigin( QwtPlotVectorField::OriginHead )
        , magnitudeScaleFactor( 1.0 )
        , rasterSize( 20, 20 )
        , minArrowLength( 0.0 )
        , maxArrowLength( std::numeric_limits< short >::max() )
        , magnitudeModes( MagnitudeAsLength )
    {
        colorMap = nullptr;
        symbol = new QwtVectorFieldThinArrow();
    }

    ~PrivateData()
    {
        delete colorMap;
        delete symbol;
    }

    QPen pen;
    QBrush brush;

    IndicatorOrigin indicatorOrigin;
    QwtVectorFieldSymbol* symbol;
    QwtColorMap* colorMap;

    /*
        Stores the range of magnitudes to be used for the color map.
        If invalid (min=max or negative values), the range is determined
        from the data samples themselves.
     */
    QwtInterval magnitudeRange;
    QwtInterval boundingMagnitudeRange;

    qreal magnitudeScaleFactor;
    QSizeF rasterSize;

    double minArrowLength;
    double maxArrowLength;

    PaintAttributes paintAttributes;
    MagnitudeModes magnitudeModes;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the curve
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPlotVectorField::QwtPlotVectorField( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the curve
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 曲线标题
 * \endif
 */
QwtPlotVectorField::QwtPlotVectorField( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
{
    init();
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
QwtPlotVectorField::~QwtPlotVectorField()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Initialize data members
 * @details Initializes the internal data members and sets default attributes.
 * \endif
 *
 * \if CHINESE
 * @brief 初始化数据成员
 * @details 初始化内部数据成员并设置默认属性。
 * \endif
 */
void QwtPlotVectorField::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    m_data = new PrivateData;
    setData( new QwtVectorFieldData() );

    setZ( 20.0 );
}

/**
 * \if ENGLISH
 * @brief Assign a pen
 * @param[in] pen New pen
 * @note The pen is ignored in MagnitudeAsColor mode
 * @sa pen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 设置画笔
 * @param[in] pen 新的画笔
 * @note 在 MagnitudeAsColor 模式下画笔被忽略
 * @sa pen(), brush()
 * \endif
 */
void QwtPlotVectorField::setPen( const QPen& pen )
{
    if ( m_data->pen != pen )
    {
        m_data->pen = pen;

        itemChanged();
        legendChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the pen used to draw the lines
 * @return Pen used for drawing
 * @sa setPen(), brush()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于绘制线条的画笔
 * @return 用于绘制的画笔
 * @sa setPen(), brush()
 * \endif
 */
QPen QwtPlotVectorField::pen() const
{
    return m_data->pen;
}

/**
 * \if ENGLISH
 * @brief Assign a brush
 * @param[in] brush New brush
 * @note The brush is ignored in MagnitudeAsColor mode
 * @sa brush(), pen()
 * \endif
 *
 * \if CHINESE
 * @brief 设置画刷
 * @param[in] brush 新的画刷
 * @note 在 MagnitudeAsColor 模式下画刷被忽略
 * @sa brush(), pen()
 * \endif
 */
void QwtPlotVectorField::setBrush( const QBrush& brush )
{
    if ( m_data->brush != brush )
    {
        m_data->brush = brush;

        itemChanged();
        legendChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the brush used to fill the symbol
 * @return Brush used for filling
 * @sa setBrush(), pen()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于填充符号的画刷
 * @return 用于填充的画刷
 * @sa setBrush(), pen()
 * \endif
 */
QBrush QwtPlotVectorField::brush() const
{
    return m_data->brush;
}

/**
 * \if ENGLISH
 * @brief Set the origin for the symbols/arrows
 * @param[in] origin Origin position
 * @sa indicatorOrigin()
 * \endif
 *
 * \if CHINESE
 * @brief 设置符号/箭头的原点
 * @param[in] origin 原点位置
 * @sa indicatorOrigin()
 * \endif
 */
void QwtPlotVectorField::setIndicatorOrigin( IndicatorOrigin origin )
{
    m_data->indicatorOrigin = origin;
    if ( m_data->indicatorOrigin != origin )
    {
        m_data->indicatorOrigin = origin;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the origin for the symbols/arrows
 * @return Origin position
 * @sa setIndicatorOrigin()
 * \endif
 *
 * \if CHINESE
 * @brief 获取符号/箭头的原点
 * @return 原点位置
 * @sa setIndicatorOrigin()
 * \endif
 */
QwtPlotVectorField::IndicatorOrigin QwtPlotVectorField::indicatorOrigin() const
{
    return m_data->indicatorOrigin;
}

/**
 * \if ENGLISH
 * @brief Set the magnitude scale factor
 * @details The length of the arrow in screen coordinate units is calculated by
 *          scaling the magnitude by the magnitudeScaleFactor.
 * @param[in] factor Scale factor
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa magnitudeScaleFactor(), arrowLength()
 * \endif
 *
 * \if CHINESE
 * @brief 设置大小缩放因子
 * @details 箭头在屏幕坐标单位中的长度通过将大小乘以 magnitudeScaleFactor 来计算。
 * @param[in] factor 缩放因子
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa magnitudeScaleFactor(), arrowLength()
 * \endif
 */
void QwtPlotVectorField::setMagnitudeScaleFactor( double factor )
{
    if ( factor != m_data->magnitudeScaleFactor )
    {
        m_data->magnitudeScaleFactor = factor;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the scale factor used to calculate the arrow length from the magnitude
 * @details The length of the arrow in screen coordinate units is calculated by
 *          scaling the magnitude by the magnitudeScaleFactor.
 *          Default implementation simply scales the vector using the magnitudeScaleFactor property.
 *          Re-implement this function to provide special handling for zero/non-zero magnitude arrows,
 *          or impose minimum/maximum arrow length limits.
 * @return Scale factor
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa setMagnitudeScaleFactor()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于从大小计算箭头长度的缩放因子
 * @details 箭头在屏幕坐标单位中的长度通过将大小乘以 magnitudeScaleFactor 来计算。
 *          默认实现只是使用 magnitudeScaleFactor 属性缩放向量。
 *          重写此函数可为零/非零大小箭头提供特殊处理，或设置最小/最大箭头长度限制。
 * @return 缩放因子
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa setMagnitudeScaleFactor()
 * \endif
 */
double QwtPlotVectorField::magnitudeScaleFactor() const
{
    return m_data->magnitudeScaleFactor;
}

/**
 * \if ENGLISH
 * @brief Set the raster size used for filtering samples
 * @param[in] size Raster size
 * @sa rasterSize(), QwtPlotVectorField::FilterVectors
 * \endif
 *
 * \if CHINESE
 * @brief 设置用于过滤样本的栅格大小
 * @param[in] size 栅格大小
 * @sa rasterSize(), QwtPlotVectorField::FilterVectors
 * \endif
 */
void QwtPlotVectorField::setRasterSize( const QSizeF& size )
{
    if ( size != m_data->rasterSize )
    {
        m_data->rasterSize = size;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the raster size used for filtering samples
 * @return Raster size
 * @sa setRasterSize(), QwtPlotVectorField::FilterVectors
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于过滤样本的栅格大小
 * @return 栅格大小
 * @sa setRasterSize(), QwtPlotVectorField::FilterVectors
 * \endif
 */
QSizeF QwtPlotVectorField::rasterSize() const
{
    return m_data->rasterSize;
}

/**
 * \if ENGLISH
 * @brief Specify an attribute how to draw the curve
 * @param[in] attribute Paint attribute
 * @param[in] on On/Off
 * @sa testPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 指定绘制曲线的属性
 * @param[in] attribute 绘制属性
 * @param[in] on 开启/关闭
 * @sa testPaintAttribute()
 * \endif
 */
void QwtPlotVectorField::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    PaintAttributes attributes = m_data->paintAttributes;

    if ( on )
        attributes |= attribute;
    else
        attributes &= ~attribute;

    if ( m_data->paintAttributes != attributes )
    {
        m_data->paintAttributes = attributes;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test a paint attribute
 * @return True when attribute is enabled
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 *
 * \if CHINESE
 * @brief 测试绘制属性
 * @return 当属性启用时返回 true
 * @sa PaintAttribute, setPaintAttribute()
 * \endif
 */
bool QwtPlotVectorField::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotVectorField
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotVectorField
 * \endif
 */
int QwtPlotVectorField::rtti() const
{
    return QwtPlotItem::Rtti_PlotVectorField;
}

/**
 * \if ENGLISH
 * @brief Set a new arrow symbol
 * @details Sets a new arrow symbol (implementation of arrow drawing code).
 * @param[in] symbol Arrow symbol
 * @note Ownership is transferred to QwtPlotVectorField.
 * @sa symbol(), drawSymbol()
 * \endif
 *
 * \if CHINESE
 * @brief 设置新的箭头符号
 * @details 设置新的箭头符号（箭头绘制代码的实现）。
 * @param[in] symbol 箭头符号
 * @note 所有权转移给 QwtPlotVectorField。
 * @sa symbol(), drawSymbol()
 * \endif
 */
void QwtPlotVectorField::setSymbol( QwtVectorFieldSymbol* symbol )
{
    if ( m_data->symbol == symbol )
        return;

    delete m_data->symbol;
    m_data->symbol = symbol;

    itemChanged();
    legendChanged();
}

/**
 * \if ENGLISH
 * @brief Get the arrow symbol
 * @return Arrow symbol
 * @sa setSymbol(), drawSymbol()
 * \endif
 *
 * \if CHINESE
 * @brief 获取箭头符号
 * @return 箭头符号
 * @sa setSymbol(), drawSymbol()
 * \endif
 */
const QwtVectorFieldSymbol* QwtPlotVectorField::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of samples
 * @param[in] samples Vector of points
 * \endif
 *
 * \if CHINESE
 * @brief 用样本数组初始化数据
 * @param[in] samples 点向量
 * \endif
 */
void QwtPlotVectorField::setSamples( const QVector< QwtVectorFieldSample >& samples )
{
    setData( new QwtVectorFieldData( samples ) );
}

/**
 * \if ENGLISH
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting it when its not used anymore.
 * \endif
 *
 * \if CHINESE
 * @brief 分配一系列样本
 * @details setSamples() 只是 setData() 的包装器，没有任何额外价值——除了让开发者更容易找到。
 * @param[in] data 数据
 * @warning 该项获取数据对象的所有权，当不再使用时会删除它。
 * \endif
 */
void QwtPlotVectorField::setSamples( QwtVectorFieldData* data )
{
    setData( data );
}

/**
 * \if ENGLISH
 * @brief Change the color map
 * @details The color map is used to map the magnitude of a sample into
 *          a color using a known range for the magnitudes.
 * @param[in] colorMap Color Map
 * @sa colorMap(), magnitudeRange()
 * \endif
 *
 * \if CHINESE
 * @brief 更改颜色映射
 * @details 颜色映射用于使用已知的大小范围将样本的大小映射到颜色。
 * @param[in] colorMap 颜色映射
 * @sa colorMap(), magnitudeRange()
 * \endif
 */
void QwtPlotVectorField::setColorMap( QwtColorMap* colorMap )
{
    if ( colorMap == nullptr )
        return;

    if ( colorMap != m_data->colorMap )
    {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color Map
 * @sa setColorMap()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于将强度值映射到颜色的颜色映射
 * @return 颜色映射
 * @sa setColorMap()
 * \endif
 */
const QwtColorMap* QwtPlotVectorField::colorMap() const
{
    return m_data->colorMap;
}

/**
 * \if ENGLISH
 * @brief Specify a mode how to represent the magnitude of an arrow/symbol
 * @param[in] mode Mode
 * @param[in] on On/Off
 * @sa testMagnitudeMode()
 * \endif
 *
 * \if CHINESE
 * @brief 指定表示箭头/符号大小的方式
 * @param[in] mode 模式
 * @param[in] on 开启/关闭
 * @sa testMagnitudeMode()
 * \endif
 */
void QwtPlotVectorField::setMagnitudeMode( MagnitudeMode mode, bool on )
{
    if ( on == testMagnitudeMode( mode ) )
        return;

    if ( on )
        m_data->magnitudeModes |= mode;
    else
        m_data->magnitudeModes &= ~mode;

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Test a magnitude mode
 * @return True when mode is enabled
 * @sa MagnitudeMode, setMagnitudeMode()
 * \endif
 *
 * \if CHINESE
 * @brief 测试大小模式
 * @return 当模式启用时返回 true
 * @sa MagnitudeMode, setMagnitudeMode()
 * \endif
 */
bool QwtPlotVectorField::testMagnitudeMode( MagnitudeMode mode ) const
{
    return m_data->magnitudeModes & mode;
}

/**
 * \if ENGLISH
 * @brief Set the magnitude range for color map lookups
 * @details Sets the min/max magnitudes to be used for color map lookups.
 *          If invalid (min=max=0 or negative values), the range is determined from
 *          the current range of magnitudes in the vector samples.
 * @param[in] magnitudeRange Magnitude range
 * @sa magnitudeRange(), colorMap()
 * \endif
 *
 * \if CHINESE
 * @brief 设置用于颜色映射查找的大小范围
 * @details 设置用于颜色映射查找的最小/最大大小。
 *          如果无效（min=max=0 或负值），则范围从向量样本的当前大小范围确定。
 * @param[in] magnitudeRange 大小范围
 * @sa magnitudeRange(), colorMap()
 * \endif
 */
void QwtPlotVectorField::setMagnitudeRange( const QwtInterval& magnitudeRange )
{
    if ( m_data->magnitudeRange != magnitudeRange )
    {
        m_data->magnitudeRange = magnitudeRange;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the magnitude range for color map lookups
 * @return Magnitude range
 * @sa setMagnitudeRange(), colorMap()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于颜色映射查找的大小范围
 * @return 大小范围
 * @sa setMagnitudeRange(), colorMap()
 * \endif
 */
QwtInterval QwtPlotVectorField::magnitudeRange() const
{
    return m_data->magnitudeRange;
}

/**
 * \if ENGLISH
 * @brief Set a minimum for the arrow length of non zero vectors
 * @param[in] length Minimum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa minArrowLength(), setMaxArrowLength(), arrowLength()
 * \endif
 *
 * \if CHINESE
 * @brief 设置非零向量箭头长度的最小值
 * @param[in] length 箭头长度的最小值（像素）
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa minArrowLength(), setMaxArrowLength(), arrowLength()
 * \endif
 */
void QwtPlotVectorField::setMinArrowLength( double length )
{
    length = qMax( length, 0.0 );

    if ( m_data->minArrowLength != length )
    {
        m_data->minArrowLength = length;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the minimum for the arrow length of non zero vectors
 * @return Minimum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa setMinArrowLength(), maxArrowLength(), arrowLength()
 * \endif
 *
 * \if CHINESE
 * @brief 获取非零向量箭头长度的最小值
 * @return 箭头长度的最小值（像素）
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa setMinArrowLength(), maxArrowLength(), arrowLength()
 * \endif
 */
double QwtPlotVectorField::minArrowLength() const
{
    return m_data->minArrowLength;
}

/**
 * \if ENGLISH
 * @brief Set a maximum for the arrow length
 * @param[in] length Maximum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa maxArrowLength(), setMinArrowLength(), arrowLength()
 * \endif
 *
 * \if CHINESE
 * @brief 设置箭头长度的最大值
 * @param[in] length 箭头长度的最大值（像素）
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa maxArrowLength(), setMinArrowLength(), arrowLength()
 * \endif
 */
void QwtPlotVectorField::setMaxArrowLength( double length )
{
    length = qMax( length, 0.0 );

    if ( m_data->maxArrowLength != length )
    {
        m_data->maxArrowLength = length;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the maximum for the arrow length
 * @return Maximum for the arrow length in pixels
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa setMinArrowLength(), maxArrowLength(), arrowLength()
 * \endif
 *
 * \if CHINESE
 * @brief 获取箭头长度的最大值
 * @return 箭头长度的最大值（像素）
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa setMinArrowLength(), maxArrowLength(), arrowLength()
 * \endif
 */
double QwtPlotVectorField::maxArrowLength() const
{
    return m_data->maxArrowLength;
}

/**
 * \if ENGLISH
 * @brief Calculate the arrow length for a given magnitude
 * @details Computes length of the arrow in screen coordinate units based on its magnitude.
 *          Default implementation simply scales the vector using the magnitudeScaleFactor().
 *          If the result is not null, the length is then bounded into the interval
 *          [ minArrowLength(), maxArrowLength() ].
 *          Re-implement this function to provide special handling for
 *          zero/non-zero magnitude arrows, or impose minimum/maximum arrow length limits.
 * @param[in] magnitude Magnitude
 * @return Length of arrow to be drawn in dependence of vector magnitude.
 * @note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 * @sa magnitudeScaleFactor, minArrowLength(), maxArrowLength()
 * \endif
 *
 * \if CHINESE
 * @brief 根据给定大小计算箭头长度
 * @details 根据大小计算箭头在屏幕坐标单位中的长度。
 *          默认实现只是使用 magnitudeScaleFactor() 缩放向量。
 *          如果结果不为空，长度将被限制在区间 [ minArrowLength(), maxArrowLength() ] 中。
 *          重写此函数可为零/非零大小箭头提供特殊处理，或设置最小/最大箭头长度限制。
 * @param[in] magnitude 大小
 * @return 根据向量大小绘制的箭头长度
 * @note 当未启用 QwtPlotVectorField::MagnitudeAsLength 时无效
 * @sa magnitudeScaleFactor, minArrowLength(), maxArrowLength()
 * \endif
 */
double QwtPlotVectorField::arrowLength( double magnitude ) const
{
#if 0
    /*
       Normalize magnitude with respect to value range.  Then, magnitudeScaleFactor
       is the number of pixels to draw for a vector of length equal to
       magnitudeRange.maxValue(). The relative scaling ensures that change of data
       samples of very different magnitudes will always lead to a reasonable
       display on screen.
     */
    const QwtVectorFieldData* vectorData = dynamic_cast< const QwtVectorFieldData* >( data() );
    if ( m_data->magnitudeRange.maxValue() > 0 )
        magnitude /= m_data->magnitudeRange.maxValue();
#endif

    double length = magnitude * m_data->magnitudeScaleFactor;

    if ( length > 0.0 )
        length = qBound( m_data->minArrowLength, length, m_data->maxArrowLength );

    return length;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle
 * @return Bounding rectangle of all samples
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界矩形
 * @return 所有样本的边界矩形
 * \endif
 */
QRectF QwtPlotVectorField::boundingRect() const
{
#if 0
    /*
        The bounding rectangle of the samples comes from the origins
        only, but as we know the scaling factor for the magnitude
        ( qwtVector2Magnitude ) here, we could try to include it ?
     */
#endif

    return QwtPlotSeriesItem::boundingRect();
}

/**
 * \if ENGLISH
 * @brief Get the icon representing the vector fields on the legend
 * @param[in] index Index of the legend entry ( ignored as there is only one )
 * @param[in] size Icon size
 * @return Legend icon
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例上表示矢量场的图标
 * @param[in] index 图例条目的索引（忽略，因为只有一个）
 * @param[in] size 图标大小
 * @return 图例图标
 * @sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 * \endif
 */
QwtGraphic QwtPlotVectorField::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    QwtGraphic icon;
    icon.setDefaultSize( size );

    if ( size.isEmpty() )
        return icon;

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    painter.translate( -size.width(), -0.5 * size.height() );

    painter.setPen( m_data->pen );
    painter.setBrush( m_data->brush );

    m_data->symbol->setLength( size.width() - 2 );
    m_data->symbol->paint( &painter );

    return icon;
}

/**
 * \if ENGLISH
 * @brief Draw a subset of the points
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rectangle of the canvas
 * @param[in] from Index of the first sample to be painted
 * @param[in] to Index of the last sample to be painted. If to < 0 the
 *               series will be painted to its last sample.
 * \endif
 *
 * \if CHINESE
 * @brief 绘制点的一部分
 * @param[in] painter 绘图器
 * @param[in] xMap 将 x 值映射到像素坐标。
 * @param[in] yMap 将 y 值映射到像素坐标。
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 要绘制的第一个样本的索引
 * @param[in] to 要绘制的最后一个样本的索引。如果 to < 0，则绘制到最后一个样本。
 * \endif
 */
void QwtPlotVectorField::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

#if DEBUG_RENDER
    QElapsedTimer timer;
    timer.start();
#endif

    drawSymbols( painter, xMap, yMap, canvasRect, from, to );

#if DEBUG_RENDER
    qDebug() << timer.elapsed();
#endif
}

/*!
   Draw symbols

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted

   \sa setSymbol(), drawSymbol(), drawSeries()
 */
void QwtPlotVectorField::drawSymbols( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );
    const bool doClip = false;

    const bool isInvertingX = xMap.isInverting();
    const bool isInvertingY = yMap.isInverting();

    const QwtSeriesData< QwtVectorFieldSample >* series = data();

    if ( m_data->magnitudeModes & MagnitudeAsColor )
    {
        // user input error, can't draw without color map
        // TODO: Discuss! Without colormap, silently fall back to uniform colors?
        if ( m_data->colorMap == nullptr)
            return;
    }
    else
    {
        painter->setPen( m_data->pen );
        painter->setBrush( m_data->brush );
    }

    if ( ( m_data->paintAttributes & FilterVectors ) && !m_data->rasterSize.isEmpty() )
    {
        const QRectF dataRect = QwtScaleMap::transform(
            xMap, yMap, boundingRect() );

        // TODO: Discuss. How to handle raster size when switching from screen to print size!
        //       DPI-aware adjustment of rastersize? Or make "rastersize in screen coordinate"
        //       or "rastersize in plotcoordinetes" a user option?
#if 1
        // define filter matrix based on screen/print coordinates
        FilterMatrix matrix( dataRect, canvasRect, m_data->rasterSize );
#else
        // define filter matrix based on real coordinates

        // get scale factor from real coordinates to screen coordinates
        double xScale = 1;
        if (xMap.sDist() != 0)
            xScale = xMap.pDist() / xMap.sDist();

        double yScale = 1;
        if (yMap.sDist() != 0)
            yScale = yMap.pDist() / yMap.sDist();

        QSizeF canvasRasterSize(xScale * m_data->rasterSize.width(), yScale * m_data->rasterSize.height() );
        FilterMatrix matrix( dataRect, canvasRect, canvasRasterSize );
#endif

        for ( int i = from; i <= to; i++ )
        {
            const QwtVectorFieldSample sample = series->sample( i );
            if ( !sample.isNull() )
            {
                matrix.addSample( xMap.transform( sample.x ),
                    yMap.transform( sample.y ), sample.vx, sample.vy );
            }
        }

        const int numEntries = matrix.numRows() * matrix.numColumns();
        const FilterMatrix::Entry* entries = matrix.entries();

        for ( int i = 0; i < numEntries; i++ )
        {
            const FilterMatrix::Entry& entry = entries[i];

            if ( entry.count == 0 )
                continue;

            double xi = entry.x / entry.count;
            double yi = entry.y / entry.count;

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            const double vx = entry.vx / entry.count;
            const double vy = entry.vy / entry.count;

            drawSymbol( painter, xi, yi,
                isInvertingX ? -vx : vx, isInvertingY ? -vy : vy );
        }
    }
    else
    {
        for ( int i = from; i <= to; i++ )
        {
            const QwtVectorFieldSample sample = series->sample( i );

            // arrows with zero length are never drawn
            if ( sample.isNull() )
                continue;

            double xi = xMap.transform( sample.x );
            double yi = yMap.transform( sample.y );

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            if ( doClip )
            {
                if ( !canvasRect.contains( xi, yi ) )
                    continue;
            }

            drawSymbol( painter, xi, yi,
                isInvertingX ? -sample.vx : sample.vx,
                isInvertingY ? -sample.vy : sample.vy );
        }
    }
}

/*!
   Draw a arrow/symbols at a specific position

   x, y, are paint device coordinates, while vx, vy are from
   the corresponding sample.

   \sa setSymbol(), drawSeries()
 */
void QwtPlotVectorField::drawSymbol( QPainter* painter,
    double x, double y, double vx, double vy ) const
{
    const double magnitude = qwtVector2Magnitude( vx, vy );

    const QTransform oldTransform = painter->transform();

    QTransform transform = qwtSymbolTransformation( oldTransform,
        x, y, vx, vy, magnitude );

    QwtVectorFieldSymbol* symbol = m_data->symbol;

    double length = 0.0;

    if ( m_data->magnitudeModes & MagnitudeAsLength )
    {
        length = arrowLength( magnitude );
    }

    symbol->setLength( length );

    if( m_data->indicatorOrigin == OriginTail )
    {
        const qreal dx = symbol->length();
        transform.translate( dx, 0.0 );
    }
    else if ( m_data->indicatorOrigin == OriginCenter )
    {
        const qreal dx = symbol->length();
        transform.translate( 0.5 * dx, 0.0 );
    }

    if ( m_data->magnitudeModes & MagnitudeAsColor )
    {
        // Determine color for arrow if colored by magnitude.

        QwtInterval range = m_data->magnitudeRange;

        if ( !range.isValid() )
        {
            if ( !m_data->boundingMagnitudeRange.isValid() )
                m_data->boundingMagnitudeRange = qwtMagnitudeRange( data() );

            range = m_data->boundingMagnitudeRange;
        }

        const QColor c = m_data->colorMap->rgb( range, magnitude );

#if 1
        painter->setBrush( c );
        painter->setPen( c );
#endif
    }

    painter->setWorldTransform( transform, false );
    symbol->paint( painter );
    painter->setWorldTransform( oldTransform, false );
}

void QwtPlotVectorField::dataChanged()
{
    m_data->boundingMagnitudeRange.invalidate();
    QwtPlotSeriesItem::dataChanged();
}
