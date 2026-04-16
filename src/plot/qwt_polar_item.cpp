/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_polar_item.h"
#include "qwt_legend.h"
#include "qwt_scale_div.h"

#include <qpainter.h>

class QwtPolarItem::PrivateData
{
  public:
    PrivateData()
        : plot( nullptr )
        , isVisible( true )
        , renderThreadCount( 1 )
        , z( 0.0 )
        , legendIconSize( 8, 8 )
    {
    }

    mutable QwtPolarPlot* plot;

    bool isVisible;
    QwtPolarItem::ItemAttributes attributes;
    QwtPolarItem::RenderHints renderHints;
    uint renderThreadCount;

    double z;

    QwtText title;
    QSize legendIconSize;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Item title, f.e used on a legend
 * @sa setTitle()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 项标题，例如用于图例
 * @sa setTitle()
 * \endif
 */
QwtPolarItem::QwtPolarItem( const QwtText& title )
{
    m_data = new PrivateData;
    m_data->title = title;
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
QwtPolarItem::~QwtPolarItem()
{
    attach( nullptr );
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Attach the item to a plot
 * @details This method will attach a QwtPolarItem to the QwtPolarPlot argument.
 *          It will first detach the QwtPolarItem from any plot from a previous
 *          call to attach (if necessary). If a nullptr argument is passed,
 *          it will detach from any QwtPolarPlot it was attached to.
 * @param[in] plot Plot widget
 * @sa QwtPolarItem::detach()
 * \endif
 *
 * \if CHINESE
 * @brief 将项附加到绘图
 * @details 此方法将 QwtPolarItem 附加到 QwtPolarPlot 参数。
 *          它将首先从之前调用 attach 的任何绘图中分离 QwtPolarItem（如果必要）。
 *          如果传递 nullptr 参数，它将从任何已附加的 QwtPolarPlot 分离。
 * @param[in] plot 绘图控件
 * @sa QwtPolarItem::detach()
 * \endif
 */
void QwtPolarItem::attach( QwtPolarPlot* plot )
{
    if ( plot == m_data->plot )
        return;

    if ( m_data->plot )
        m_data->plot->attachItem( this, false );

    m_data->plot = plot;

    if ( m_data->plot )
        m_data->plot->attachItem( this, true );
}

/**
 * \if ENGLISH
 * @brief Detach the item from its associated plot
 * @details This method detaches a QwtPolarItem from the QwtPolarPlot it has been associated with.
 *          detach() is equivalent to calling attach( nullptr ).
 * @sa attach()
 * \endif
 *
 * \if CHINESE
 * @brief 从关联的绘图分离项
 * @details 此方法将 QwtPolarItem 从其关联的 QwtPolarPlot 分离。
 *          detach() 等同于调用 attach( nullptr )。
 * @sa attach()
 * \endif
 */
void QwtPolarItem::detach()
{
    attach( nullptr );
}

/**
 * \if ENGLISH
 * @brief Get runtime type information for the specific class represented
 * @details QwtPolarItem is simply a virtual interface class, and base classes will implement this method
 *          with specific rtti values so a user can differentiate them. The rtti value is useful for
 *          environments where the runtime type information is disabled and it is not possible
 *          to do a dynamic_cast<...>.
 * @return rtti value
 * @sa RttiValues
 * \endif
 *
 * \if CHINESE
 * @brief 获取所表示的特定类的运行时类型信息
 * @details QwtPolarItem 只是一个虚拟接口类，基类将用特定的 rtti 值实现此方法，
 *          以便用户可以区分它们。rtti 值对于禁用运行时类型信息且无法进行 dynamic_cast<...> 的环境很有用。
 * @return rtti 值
 * @sa RttiValues
 * \endif
 */
int QwtPolarItem::rtti() const
{
    return Rtti_PolarItem;
}

/**
 * \if ENGLISH
 * @brief Get the attached plot
 * @return Attached plot
 * \endif
 *
 * \if CHINESE
 * @brief 获取附加的绘图
 * @return 附加的绘图
 * \endif
 */
QwtPolarPlot* QwtPolarItem::plot() const
{
    return m_data->plot;
}

/**
 * \if ENGLISH
 * @brief Get the z value
 * @details Plot items are painted in increasing z-order.
 * @return Z value
 * @sa setZ(), QwtPolarItemDict::itemList()
 * \endif
 *
 * \if CHINESE
 * @brief 获取 z 值
 * @details 绘图项按递增的 z 顺序绘制。
 * @return Z 值
 * @sa setZ(), QwtPolarItemDict::itemList()
 * \endif
 */
double QwtPolarItem::z() const
{
    return m_data->z;
}

/**
 * \if ENGLISH
 * @brief Set the z value
 * @details Plot items are painted in increasing z-order.
 * @param[in] z Z-value
 * @sa z(), QwtPolarItemDict::itemList()
 * \endif
 *
 * \if CHINESE
 * @brief 设置 z 值
 * @details 绘图项按递增的 z 顺序绘制。
 * @param[in] z Z 值
 * @sa z(), QwtPolarItemDict::itemList()
 * \endif
 */
void QwtPolarItem::setZ( double z )
{
    if ( m_data->z != z )
    {
        if ( m_data->plot )
            m_data->plot->attachItem( this, false );

        m_data->z = z;

        if ( m_data->plot )
            m_data->plot->attachItem( this, true );

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set a new title
 * @param[in] title Title
 * @sa title()
 * \endif
 *
 * \if CHINESE
 * @brief 设置新标题
 * @param[in] title 标题
 * @sa title()
 * \endif
 */
void QwtPolarItem::setTitle( const QString& title )
{
    setTitle( QwtText( title ) );
}

/**
 * \if ENGLISH
 * @brief Set a new title
 * @param[in] title Title
 * @sa title()
 * \endif
 *
 * \if CHINESE
 * @brief 设置新标题
 * @param[in] title 标题
 * @sa title()
 * \endif
 */
void QwtPolarItem::setTitle( const QwtText& title )
{
    if ( m_data->title != title )
    {
        m_data->title = title;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the title of the item
 * @return Title of the item
 * @sa setTitle()
 * \endif
 *
 * \if CHINESE
 * @brief 获取项的标题
 * @return 项的标题
 * @sa setTitle()
 * \endif
 */
const QwtText& QwtPolarItem::title() const
{
    return m_data->title;
}

/**
 * \if ENGLISH
 * @brief Toggle an item attribute
 * @param[in] attribute Attribute type
 * @param[in] on true/false
 * @sa testItemAttribute(), ItemAttribute
 * \endif
 *
 * \if CHINESE
 * @brief 切换项属性
 * @param[in] attribute 属性类型
 * @param[in] on true/false
 * @sa testItemAttribute(), ItemAttribute
 * \endif
 */
void QwtPolarItem::setItemAttribute( ItemAttribute attribute, bool on )
{
    if ( bool( m_data->attributes & attribute ) != on )
    {
        if ( on )
            m_data->attributes |= attribute;
        else
            m_data->attributes &= ~attribute;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test an item attribute
 * @param[in] attribute Attribute type
 * @return true/false
 * @sa setItemAttribute(), ItemAttribute
 * \endif
 *
 * \if CHINESE
 * @brief 测试项属性
 * @param[in] attribute 属性类型
 * @return true/false
 * @sa setItemAttribute(), ItemAttribute
 * \endif
 */
bool QwtPolarItem::testItemAttribute( ItemAttribute attribute ) const
{
    return m_data->attributes & attribute;
}

/**
 * \if ENGLISH
 * @brief Toggle a render hint
 * @param[in] hint Render hint
 * @param[in] on true/false
 * @sa testRenderHint(), RenderHint
 * \endif
 *
 * \if CHINESE
 * @brief 切换渲染提示
 * @param[in] hint 渲染提示
 * @param[in] on true/false
 * @sa testRenderHint(), RenderHint
 * \endif
 */
void QwtPolarItem::setRenderHint( RenderHint hint, bool on )
{
    if ( ( ( m_data->renderHints & hint ) != 0 ) != on )
    {
        if ( on )
            m_data->renderHints |= hint;
        else
            m_data->renderHints &= ~hint;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test a render hint
 * @param[in] hint Render hint
 * @return true/false
 * @sa setRenderHint(), RenderHint
 * \endif
 *
 * \if CHINESE
 * @brief 测试渲染提示
 * @param[in] hint 渲染提示
 * @return true/false
 * @sa setRenderHint(), RenderHint
 * \endif
 */
bool QwtPolarItem::testRenderHint( RenderHint hint ) const
{
    return ( m_data->renderHints & hint );
}

/**
 * \if ENGLISH
 * @brief Set the number of render threads
 * @details On multi core systems rendering of certain plot item (f.e QwtPolarSpectrogram)
 *          can be done in parallel in several threads. The default setting is set to 1.
 * @param[in] numThreads Number of threads to be used for rendering.
 *                       If numThreads is set to 0, the system specific ideal thread count is used.
 *                       The default thread count is 1 ( = no additional threads ).
 * \endif
 *
 * \if CHINESE
 * @brief 设置渲染线程数
 * @details 在多核系统上，某些绘图项（例如 QwtPolarSpectrogram）的渲染可以在多个线程中并行完成。
 *          默认设置为 1。
 * @param[in] numThreads 用于渲染的线程数。
 *                       如果 numThreads 设置为 0，则使用系统特定的理想线程数。
 *                       默认线程数为 1（即没有额外线程）。
 * \endif
 */
void QwtPolarItem::setRenderThreadCount( uint numThreads )
{
    m_data->renderThreadCount = numThreads;
}

/**
 * \if ENGLISH
 * @brief Get the number of render threads
 * @return Number of threads to be used for rendering.
 *         If numThreads() is set to 0, the system specific ideal thread count is used.
 * \endif
 *
 * \if CHINESE
 * @brief 获取渲染线程数
 * @return 用于渲染的线程数。
 *         如果 numThreads() 设置为 0，则使用系统特定的理想线程数。
 * \endif
 */
uint QwtPolarItem::renderThreadCount() const
{
    return m_data->renderThreadCount;
}

/**
 * \if ENGLISH
 * @brief Set the size of the legend icon
 * @details The default setting is 8x8 pixels.
 * @param[in] size Size
 * @sa legendIconSize(), legendIcon()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例图标的大小
 * @details 默认设置为 8x8 像素。
 * @param[in] size 大小
 * @sa legendIconSize(), legendIcon()
 * \endif
 */
void QwtPolarItem::setLegendIconSize( const QSize& size )
{
    if ( m_data->legendIconSize != size )
    {
        m_data->legendIconSize = size;
        legendChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the legend icon size
 * @return Legend icon size
 * @sa setLegendIconSize(), legendIcon()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例图标大小
 * @return 图例图标大小
 * @sa setLegendIconSize(), legendIcon()
 * \endif
 */
QSize QwtPolarItem::legendIconSize() const
{
    return m_data->legendIconSize;
}

//! \if ENGLISH Show the item \endif \if CHINESE 显示项 \endif
void QwtPolarItem::show()
{
    setVisible( true );
}

//! \if ENGLISH Hide the item \endif \if CHINESE 隐藏项 \endif
void QwtPolarItem::hide()
{
    setVisible( false );
}

/**
 * \if ENGLISH
 * @brief Show/Hide the item
 * @param[in] on Show if true, otherwise hide
 * @sa isVisible(), show(), hide()
 * \endif
 *
 * \if CHINESE
 * @brief 显示/隐藏项
 * @param[in] on 如果为 true 则显示，否则隐藏
 * @sa isVisible(), show(), hide()
 * \endif
 */
void QwtPolarItem::setVisible( bool on )
{
    if ( on != m_data->isVisible )
    {
        m_data->isVisible = on;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Check if the item is visible
 * @return true if visible
 * @sa setVisible(), show(), hide()
 * \endif
 *
 * \if CHINESE
 * @brief 检查项是否可见
 * @return 如果可见则返回 true
 * @sa setVisible(), show(), hide()
 * \endif
 */
bool QwtPolarItem::isVisible() const
{
    return m_data->isVisible;
}

/**
 * \if ENGLISH
 * @brief Update the legend and call QwtPolarPlot::autoRefresh for the parent plot
 * @sa updateLegend()
 * \endif
 *
 * \if CHINESE
 * @brief 更新图例并调用父绘图的 QwtPolarPlot::autoRefresh
 * @sa updateLegend()
 * \endif
 */
void QwtPolarItem::itemChanged()
{
    if ( m_data->plot )
        m_data->plot->autoRefresh();
}

/**
 * \if ENGLISH
 * @brief Update the legend of the parent plot
 * @sa QwtPolarPlot::updateLegend(), itemChanged()
 * \endif
 *
 * \if CHINESE
 * @brief 更新父绘图的图例
 * @sa QwtPolarPlot::updateLegend(), itemChanged()
 * \endif
 */
void QwtPolarItem::legendChanged()
{
    if ( testItemAttribute( QwtPolarItem::Legend ) && m_data->plot )
        m_data->plot->updateLegend( this );
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval for a scale
 * @details Interval, that is necessary to display the item.
 *          This interval can be useful for operations like clipping or autoscaling.
 *          For items (like the grid), where a bounding interval makes no sense, an invalid interval is returned.
 * @param[in] scaleId Scale id ( QwtPolar::Scale )
 * @return Bounding interval of the plot item for a specific scale
 * \endif
 *
 * \if CHINESE
 * @brief 获取刻度的边界区间
 * @details 显示项所需的区间。
 *          此区间可用于裁剪或自动缩放等操作。
 *          对于边界区间无意义的项（如网格），返回无效区间。
 * @param[in] scaleId 刻度 ID（QwtPolar::Scale）
 * @return 绘图项在特定刻度的边界区间
 * \endif
 */
QwtInterval QwtPolarItem::boundingInterval( int scaleId ) const
{
    Q_UNUSED( scaleId );

    return QwtInterval(); // invalid
}

/**
 * \if ENGLISH
 * @brief Update the item to changes of the axes scale division
 * @details Update the item, when the axes of plot have changed.
 *          The default implementation does nothing, but items that depend on the scale division
 *          (like QwtPolarGrid()) have to reimplement updateScaleDiv().
 * @param[in] azimuthScaleDiv Scale division of the azimuth-scale
 * @param[in] radialScaleDiv Scale division of the radius-axis
 * @param[in] interval The interval of the radius-axis, that is visible on the canvas
 * @sa QwtPolarPlot::updateAxes()
 * \endif
 *
 * \if CHINESE
 * @brief 更新项以适应轴刻度划分的变化
 * @details 当绘图的轴发生变化时更新项。
 *          默认实现不执行任何操作，但依赖于刻度划分的项（如 QwtPolarGrid()）
 *          必须重新实现 updateScaleDiv()。
 * @param[in] azimuthScaleDiv 方位角刻度的刻度划分
 * @param[in] radialScaleDiv 半径轴的刻度划分
 * @param[in] interval 画布上可见的半径轴区间
 * @sa QwtPolarPlot::updateAxes()
 * \endif
 */
void QwtPolarItem::updateScaleDiv( const QwtScaleDiv& azimuthScaleDiv,
    const QwtScaleDiv& radialScaleDiv, const QwtInterval& interval )
{
    Q_UNUSED( azimuthScaleDiv );
    Q_UNUSED( radialScaleDiv );
    Q_UNUSED( interval );
}

/**
 * \if ENGLISH
 * @brief Return all information that is needed to represent the item on the legend
 * @details Most items are represented by one entry on the legend showing an icon and a text.
 *          QwtLegendData is basically a list of QVariants that makes it possible to overload
 *          and reimplement legendData() to return almost any type of information, that is
 *          understood by the receiver that acts as the legend.
 *          The default implementation returns one entry with the title() of the item and the legendIcon().
 * @return List of legend data
 * @sa title(), legendIcon(), QwtLegend
 * \endif
 *
 * \if CHINESE
 * @brief 返回在图例上表示项所需的所有信息
 * @details 大多数项在图例上由一个条目表示，显示图标和文本。
 *          QwtLegendData 基本上是 QVariant 列表，使得可以重载并重新实现 legendData()
 *          以返回几乎任何类型的可被充当图例的接收者理解的信息。
 *          默认实现返回一个条目，包含项的 title() 和 legendIcon()。
 * @return 图例数据列表
 * @sa title(), legendIcon(), QwtLegend
 * \endif
 */
QList< QwtLegendData > QwtPolarItem::legendData() const
{
    QwtLegendData data;

    QwtText label = title();
    label.setRenderFlags( label.renderFlags() & Qt::AlignLeft );

    data.setValue( QwtLegendData::TitleRole,
        QVariant::fromValue( label ) );

    const QwtGraphic graphic = legendIcon( 0, legendIconSize() );
    if ( !graphic.isNull() )
    {
        data.setValue( QwtLegendData::IconRole,
            QVariant::fromValue( graphic ) );
    }

    QList< QwtLegendData > list;
    list += data;

    return list;
}

/**
 * \if ENGLISH
 * @brief Get icon representing the item on the legend
 * @details The default implementation returns an invalid icon.
 * @param[in] index Index of the legend entry (usually there is only one)
 * @param[in] size Icon size
 * @return Icon representing the item on the legend
 * @sa setLegendIconSize(), legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取在图例上表示项的图标
 * @details 默认实现返回无效图标。
 * @param[in] index 图例条目的索引（通常只有一个）
 * @param[in] size 图标大小
 * @return 在图例上表示项的图标
 * @sa setLegendIconSize(), legendData()
 * \endif
 */
QwtGraphic QwtPolarItem::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index )
    Q_UNUSED( size )

    return QwtGraphic();
}

/**
 * \if ENGLISH
 * @brief Get the margin hint
 * @details Some items like to display something (f.e. the azimuth axis) outside of the area
 *          of the interval of the radial scale. The default implementation returns 0 pixels.
 * @return Hint for the margin
 * \endif
 *
 * \if CHINESE
 * @brief 获取边距提示
 * @details 某些项希望在径向刻度区间区域之外显示内容（例如方位角轴）。
 *          默认实现返回 0 像素。
 * @return 边距提示
 * \endif
 */
int QwtPolarItem::marginHint() const
{
    return 0;
}
