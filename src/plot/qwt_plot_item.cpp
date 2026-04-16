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
 ******************************************************************************/

#include "qwt_plot_item.h"
#include "qwt_text.h"
#include "qwt_plot.h"
#include "qwt_legend_data.h"
#include "qwt_scale_map.h"
#include "qwt_graphic.h"

#include <qpainter.h>

class QwtPlotItem::PrivateData
{
  public:
    PrivateData()
        : plot( nullptr )
        , isVisible( true )
        , renderThreadCount( 1 )
        , z( 0.0 )
        , xAxisId( QwtAxis::XBottom )
        , yAxisId( QwtAxis::YLeft )
        , legendIconSize( 8, 8 )
    {
    }

    mutable QwtPlot* plot;

    bool isVisible;

    QwtPlotItem::ItemAttributes attributes;
    QwtPlotItem::ItemInterests interests;

    QwtPlotItem::RenderHints renderHints;
    uint renderThreadCount;

    double z;

    QwtAxisId xAxisId;
    QwtAxisId yAxisId;

    QwtText title;
    QSize legendIconSize;
};

/**
 * \if ENGLISH
 * @brief Default constructor
 * @details Constructs a QwtPlotItem with default settings.
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数
 * @details 使用默认设置构造一个 QwtPlotItem 对象。
 * \endif
 */
QwtPlotItem::QwtPlotItem()
{
    m_data = new PrivateData;
}

/**
 * \if ENGLISH
 * @brief Constructor with title as QString
 * @param[in] title Title of the item
 * @details Constructs a QwtPlotItem with the given title.
 * \endif
 *
 * \if CHINESE
 * @brief 使用 QString 标题的构造函数
 * @param[in] title 项目的标题
 * @details 使用给定的标题构造一个 QwtPlotItem 对象。
 * \endif
 */
QwtPlotItem::QwtPlotItem( const QString& title )
{
    m_data = new PrivateData;
    m_data->title = title;
}

/**
 * \if ENGLISH
 * @brief Constructor with title as QwtText
 * @param[in] title Title of the item
 * @details Constructs a QwtPlotItem with the given title.
 * \endif
 *
 * \if CHINESE
 * @brief 使用 QwtText 标题的构造函数
 * @param[in] title 项目的标题
 * @details 使用给定的标题构造一个 QwtPlotItem 对象。
 * \endif
 */
QwtPlotItem::QwtPlotItem( const QwtText& title )
{
    m_data = new PrivateData;
    m_data->title = title;
}

/**
 * \if ENGLISH
 * @brief Destructor
 * @details Destroys the QwtPlotItem and detaches it from any plot.
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * @details 销毁 QwtPlotItem 并将其从任何绑定的绘图中分离。
 * \endif
 */
QwtPlotItem::~QwtPlotItem()
{
    attach( nullptr );
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Attach the item to a plot
 * @param[in] plot Plot widget to attach to
 * @details This method will attach a QwtPlotItem to the QwtPlot argument.
 *          It will first detach the QwtPlotItem from any plot from a previous
 *          call to attach (if necessary). If a nullptr argument is passed,
 *          it will detach from any QwtPlot it was attached to.
 * @sa detach()
 * \endif
 *
 * \if CHINESE
 * @brief 将项目附加到绘图
 * @param[in] plot 要附加到的绘图部件
 * @details 此方法将 QwtPlotItem 附加到 QwtPlot 参数。它将首先从任何先前调用
 *          attach 时附加的绘图中分离 QwtPlotItem（如有必要）。如果传递 nullptr 参数，
 *          它将从任何已附加的 QwtPlot 中分离。
 * @sa detach()
 * \endif
 */
void QwtPlotItem::attach( QwtPlot* plot )
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
 * @brief Detach the item from the plot
 * @details This method detaches a QwtPlotItem from any QwtPlot it has been
 *          associated with. detach() is equivalent to calling attach( nullptr ).
 * @sa attach()
 * \endif
 *
 * \if CHINESE
 * @brief 将项目从绘图分离
 * @details 此方法将 QwtPlotItem 从任何已关联的 QwtPlot 中分离。
 *          detach() 等同于调用 attach( nullptr )。
 * @sa attach()
 * \endif
 */
void QwtPlotItem::detach()
{
    attach( nullptr );
}

/**
 * \if ENGLISH
 * @brief Get runtime type information
 * @return rtti value for the specific class
 * @details Return rtti for the specific class represented. QwtPlotItem is simply
 *          a virtual interface class, and base classes will implement this method
 *          with specific rtti values so a user can differentiate them.
 *          The rtti value is useful for environments, where the runtime type
 *          information is disabled and it is not possible to do a dynamic_cast<...>.
 * @sa RttiValues
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return 特定类的 rtti 值
 * @details 返回所表示的特定类的 rtti。QwtPlotItem 只是一个虚拟接口类，
 *          派生类将使用特定的 rtti 值实现此方法，以便用户可以区分它们。
 *          rtti 值适用于禁用运行时类型信息且无法执行 dynamic_cast<...> 的环境。
 * @sa RttiValues
 * \endif
 */
int QwtPlotItem::rtti() const
{
    return Rtti_PlotItem;
}

/**
 * \if ENGLISH
 * @brief Get the plot the item is attached to
 * @return Attached plot, or nullptr if not attached
 * \endif
 *
 * \if CHINESE
 * @brief 获取项目附加的绘图
 * @return 附加的绘图，如果未附加则返回 nullptr
 * \endif
 */
QwtPlot* QwtPlotItem::plot() const
{
    return m_data->plot;
}

/**
 * \if ENGLISH
 * @brief Get the z-value
 * @return Z-value of the item
 * @details Plot items are painted in increasing z-order.
 * @sa setZ(), QwtPlotDict::itemList()
 * \endif
 *
 * \if CHINESE
 * @brief 获取 z 值
 * @return 项目的 z 值
 * @details 绘图项目按递增的 z 顺序绘制。
 * @sa setZ(), QwtPlotDict::itemList()
 * \endif
 */
double QwtPlotItem::z() const
{
    return m_data->z;
}

/**
 * \if ENGLISH
 * @brief Set the z-value
 * @param[in] z Z-value for the item
 * @details Plot items are painted in increasing z-order.
 * @sa z(), QwtPlotDict::itemList()
 * \endif
 *
 * \if CHINESE
 * @brief 设置 z 值
 * @param[in] z 项目的 z 值
 * @details 绘图项目按递增的 z 顺序绘制。
 * @sa z(), QwtPlotDict::itemList()
 * \endif
 */
void QwtPlotItem::setZ( double z )
{
    if ( m_data->z != z )
    {
        if ( m_data->plot ) // update the z order
            m_data->plot->attachItem( this, false );

        m_data->z = z;

        if ( m_data->plot )
            m_data->plot->attachItem( this, true );

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set the title using a QString
 * @param[in] title Title text
 * @sa title()
 * \endif
 *
 * \if CHINESE
 * @brief 使用 QString 设置标题
 * @param[in] title 标题文本
 * @sa title()
 * \endif
 */
void QwtPlotItem::setTitle( const QString& title )
{
    setTitle( QwtText( title ) );
}

/**
 * \if ENGLISH
 * @brief Set the title using a QwtText
 * @param[in] title Title text
 * @sa title()
 * \endif
 *
 * \if CHINESE
 * @brief 使用 QwtText 设置标题
 * @param[in] title 标题文本
 * @sa title()
 * \endif
 */
void QwtPlotItem::setTitle( const QwtText& title )
{
    if ( m_data->title != title )
    {
        m_data->title = title;

        legendChanged();
#if 0
        itemChanged();
#endif
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
 * @brief 获取项目的标题
 * @return 项目的标题
 * @sa setTitle()
 * \endif
 */
const QwtText& QwtPlotItem::title() const
{
    return m_data->title;
}

/**
 * \if ENGLISH
 * @brief Set an item attribute
 * @param[in] attribute Attribute type to set
 * @param[in] on true to enable, false to disable
 * @details Toggle an item attribute.
 * @sa testItemAttribute(), ItemInterest
 * \endif
 *
 * \if CHINESE
 * @brief 设置项目属性
 * @param[in] attribute 要设置的属性类型
 * @param[in] on true 启用，false 禁用
 * @details 切换项目属性。
 * @sa testItemAttribute(), ItemInterest
 * \endif
 */
void QwtPlotItem::setItemAttribute( ItemAttribute attribute, bool on )
{
    if ( m_data->attributes.testFlag( attribute ) != on )
    {
        if ( on )
            m_data->attributes |= attribute;
        else
            m_data->attributes &= ~attribute;

        if ( attribute == QwtPlotItem::Legend )
        {
            if ( on )
            {
                legendChanged();
            }
            else
            {
                /*
                    In the special case of taking an item from
                    the legend we can't use legendChanged() as
                    it depends on QwtPlotItem::Legend being enabled
                 */
                if ( m_data->plot )
                    m_data->plot->updateLegend( this );
            }
        }

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test an item attribute
 * @param[in] attribute Attribute type to test
 * @return true if the attribute is enabled, false otherwise
 * @sa setItemAttribute(), ItemInterest
 * \endif
 *
 * \if CHINESE
 * @brief 测试项目属性
 * @param[in] attribute 要测试的属性类型
 * @return 如果属性已启用则返回 true，否则返回 false
 * @sa setItemAttribute(), ItemInterest
 * \endif
 */
bool QwtPlotItem::testItemAttribute( ItemAttribute attribute ) const
{
    return m_data->attributes.testFlag( attribute );
}

/**
 * \if ENGLISH
 * @brief Set an item interest
 * @param[in] interest Interest type to set
 * @param[in] on true to enable, false to disable
 * @details Toggle an item interest.
 * @sa testItemInterest(), ItemAttribute
 * \endif
 *
 * \if CHINESE
 * @brief 设置项目关注事件
 * @param[in] interest 要设置的关注类型
 * @param[in] on true 启用，false 禁用
 * @details 切换项目关注事件。
 * @sa testItemInterest(), ItemAttribute
 * \endif
 */
void QwtPlotItem::setItemInterest( ItemInterest interest, bool on )
{
    if ( m_data->interests.testFlag( interest ) != on )
    {
        if ( on )
            m_data->interests |= interest;
        else
            m_data->interests &= ~interest;

        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Test an item interest
 * @param[in] interest Interest type to test
 * @return true if the interest is enabled, false otherwise
 * @sa setItemInterest(), ItemAttribute
 * \endif
 *
 * \if CHINESE
 * @brief 测试项目关注事件
 * @param[in] interest 要测试的关注类型
 * @return 如果关注事件已启用则返回 true，否则返回 false
 * @sa setItemInterest(), ItemAttribute
 * \endif
 */
bool QwtPlotItem::testItemInterest( ItemInterest interest ) const
{
    return m_data->interests.testFlag( interest );
}

/**
 * \if ENGLISH
 * @brief Set a render hint
 * @param[in] hint Render hint to set
 * @param[in] on true to enable, false to disable
 * @details Toggle a render hint.
 * @sa testRenderHint(), RenderHint
 * \endif
 *
 * \if CHINESE
 * @brief 设置渲染提示
 * @param[in] hint 要设置的渲染提示
 * @param[in] on true 启用，false 禁用
 * @details 切换渲染提示。
 * @sa testRenderHint(), RenderHint
 * \endif
 */
void QwtPlotItem::setRenderHint( RenderHint hint, bool on )
{
    if ( m_data->renderHints.testFlag( hint ) != on )
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
 * @param[in] hint Render hint to test
 * @return true if the render hint is enabled, false otherwise
 * @sa setRenderHint(), RenderHint
 * \endif
 *
 * \if CHINESE
 * @brief 测试渲染提示
 * @param[in] hint 要测试的渲染提示
 * @return 如果渲染提示已启用则返回 true，否则返回 false
 * @sa setRenderHint(), RenderHint
 * \endif
 */
bool QwtPlotItem::testRenderHint( RenderHint hint ) const
{
    return m_data->renderHints.testFlag( hint );
}

/**
 * \if ENGLISH
 * @brief Set the number of render threads
 * @param[in] numThreads Number of threads to be used for rendering.
 *                       If numThreads is set to 0, the system specific
 *                       ideal thread count is used.
 * @details On multi core systems rendering of certain plot item
 *          (e.g. QwtPlotRasterItem) can be done in parallel in several threads.
 *          The default setting is set to 1.
 * \endif
 *
 * \if CHINESE
 * @brief 设置渲染线程数
 * @param[in] numThreads 用于渲染的线程数。
 *                        如果 numThreads 设置为 0，则使用系统特定的理想线程数。
 * @details 在多核系统上，某些绘图项（如 QwtPlotRasterItem）的渲染
 *          可以在多个线程中并行完成。默认设置为 1。
 * \endif
 */
void QwtPlotItem::setRenderThreadCount( uint numThreads )
{
    m_data->renderThreadCount = numThreads;
}

/**
 * \if ENGLISH
 * @brief Get the number of render threads
 * @return Number of threads to be used for rendering.
 *         If renderThreadCount() is set to 0, the system specific
 *         ideal thread count is used.
 * \endif
 *
 * \if CHINESE
 * @brief 获取渲染线程数
 * @return 用于渲染的线程数。
 *         如果 renderThreadCount() 设置为 0，则使用系统特定的理想线程数。
 * \endif
 */
uint QwtPlotItem::renderThreadCount() const
{
    return m_data->renderThreadCount;
}

/**
 * \if ENGLISH
 * @brief Set the size of the legend icon
 * @param[in] size Icon size
 * @details The default setting is 8x8 pixels.
 * @sa legendIconSize(), legendIcon()
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例图标大小
 * @param[in] size 图标大小
 * @details 默认设置为 8x8 像素。
 * @sa legendIconSize(), legendIcon()
 * \endif
 */
void QwtPlotItem::setLegendIconSize( const QSize& size )
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
QSize QwtPlotItem::legendIconSize() const
{
    return m_data->legendIconSize;
}

/**
 * \if ENGLISH
 * @brief Get the icon representing the item on the legend
 * @param[in] index Index of the legend entry (usually there is only one)
 * @param[in] size Icon size
 * @return Icon representing the item on the legend
 * @details The default implementation returns an invalid icon.
 * @sa setLegendIconSize(), legendData()
 * \endif
 *
 * \if CHINESE
 * @brief 获取表示项目的图例图标
 * @param[in] index 图例条目的索引（通常只有一个）
 * @param[in] size 图标大小
 * @return 表示项目的图例图标
 * @details 默认实现返回一个无效图标。
 * @sa setLegendIconSize(), legendData()
 * \endif
 */
QwtGraphic QwtPlotItem::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index )
    Q_UNUSED( size )

    return QwtGraphic();
}

//! Return a default icon from a brush
QwtGraphic QwtPlotItem::defaultIcon(
    const QBrush& brush, const QSizeF& size ) const
{
    QwtGraphic icon;
    if ( !size.isEmpty() )
    {
        icon.setDefaultSize( size );

        QRectF r( 0, 0, size.width(), size.height() );

        QPainter painter( &icon );
        painter.fillRect( r, brush );
    }

    return icon;
}

/**
 * \if ENGLISH
 * @brief Show the item
 * @details Equivalent to calling setVisible( true ).
 * @sa hide(), setVisible(), isVisible()
 * \endif
 *
 * \if CHINESE
 * @brief 显示项目
 * @details 等同于调用 setVisible( true )。
 * @sa hide(), setVisible(), isVisible()
 * \endif
 */
void QwtPlotItem::show()
{
    setVisible( true );
}

/**
 * \if ENGLISH
 * @brief Hide the item
 * @details Equivalent to calling setVisible( false ).
 * @sa show(), setVisible(), isVisible()
 * \endif
 *
 * \if CHINESE
 * @brief 隐藏项目
 * @details 等同于调用 setVisible( false )。
 * @sa show(), setVisible(), isVisible()
 * \endif
 */
void QwtPlotItem::hide()
{
    setVisible( false );
}

/**
 * \if ENGLISH
 * @brief Set the visibility of the item
 * @param[in] on Show if true, otherwise hide
 * @sa isVisible(), show(), hide()
 * \endif
 *
 * \if CHINESE
 * @brief 设置项目的可见性
 * @param[in] on 如果为 true 则显示，否则隐藏
 * @sa isVisible(), show(), hide()
 * \endif
 */
void QwtPlotItem::setVisible( bool on )
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
 * @return true if visible, false otherwise
 * @sa setVisible(), show(), hide()
 * \endif
 *
 * \if CHINESE
 * @brief 检查项目是否可见
 * @return 如果可见则返回 true，否则返回 false
 * @sa setVisible(), show(), hide()
 * \endif
 */
bool QwtPlotItem::isVisible() const
{
    return m_data->isVisible;
}

/**
 * \if ENGLISH
 * @brief Notify the plot that the item has changed
 * @details Update the legend and call QwtPlot::autoRefresh() for the parent plot.
 * @sa QwtPlot::legendChanged(), QwtPlot::autoRefresh()
 * \endif
 *
 * \if CHINESE
 * @brief 通知绘图项目已更改
 * @details 更新图例并调用父绘图的 QwtPlot::autoRefresh()。
 * @sa QwtPlot::legendChanged(), QwtPlot::autoRefresh()
 * \endif
 */
void QwtPlotItem::itemChanged()
{
    if ( m_data->plot )
        m_data->plot->autoRefresh();
}

/**
 * \if ENGLISH
 * @brief Notify that the legend has changed
 * @details Update the legend of the parent plot.
 * @sa QwtPlot::updateLegend(), itemChanged()
 * \endif
 *
 * \if CHINESE
 * @brief 通知图例已更改
 * @details 更新父绘图的图例。
 * @sa QwtPlot::updateLegend(), itemChanged()
 * \endif
 */
void QwtPlotItem::legendChanged()
{
    if ( testItemAttribute( QwtPlotItem::Legend ) && m_data->plot )
        m_data->plot->updateLegend( this );
}

/**
 * \if ENGLISH
 * @brief Set X and Y axes
 * @param[in] xAxisId X Axis identifier
 * @param[in] yAxisId Y Axis identifier
 * @details The item will be painted according to the coordinates of its axes.
 * @sa setXAxis(), setYAxis(), xAxis(), yAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 设置 X 轴和 Y 轴
 * @param[in] xAxisId X 轴标识符
 * @param[in] yAxisId Y 轴标识符
 * @details 项目将根据其轴的坐标进行绘制。
 * @sa setXAxis(), setYAxis(), xAxis(), yAxis()
 * \endif
 */
void QwtPlotItem::setAxes( QwtAxisId xAxisId, QwtAxisId yAxisId )
{
    if ( QwtAxis::isXAxis( xAxisId ) )
        m_data->xAxisId = xAxisId;

    if ( QwtAxis::isYAxis( yAxisId ) )
        m_data->yAxisId = yAxisId;

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Set the X axis
 * @param[in] axisId X Axis identifier
 * @details The item will be painted according to the coordinates of its axis.
 * @sa setAxes(), setYAxis(), xAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 设置 X 轴
 * @param[in] axisId X 轴标识符
 * @details 项目将根据其轴的坐标进行绘制。
 * @sa setAxes(), setYAxis(), xAxis()
 * \endif
 */
void QwtPlotItem::setXAxis( QwtAxisId axisId )
{
    if ( QwtAxis::isXAxis( axisId ) )
    {
        m_data->xAxisId = axisId;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set the Y axis
 * @param[in] axisId Y Axis identifier
 * @details The item will be painted according to the coordinates of its axis.
 * @sa setAxes(), setXAxis(), yAxis()
 * \endif
 *
 * \if CHINESE
 * @brief 设置 Y 轴
 * @param[in] axisId Y 轴标识符
 * @details 项目将根据其轴的坐标进行绘制。
 * @sa setAxes(), setXAxis(), yAxis()
 * \endif
 */
void QwtPlotItem::setYAxis( QwtAxisId axisId )
{
    if ( QwtAxis::isYAxis( axisId ) )
    {
        m_data->yAxisId = axisId;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the X axis
 * @return X Axis identifier
 * \endif
 *
 * \if CHINESE
 * @brief 获取 X 轴
 * @return X 轴标识符
 * \endif
 */
QwtAxisId QwtPlotItem::xAxis() const
{
    return m_data->xAxisId;
}

/**
 * \if ENGLISH
 * @brief Get the Y axis
 * @return Y Axis identifier
 * \endif
 *
 * \if CHINESE
 * @brief 获取 Y 轴
 * @return Y 轴标识符
 * \endif
 */
QwtAxisId QwtPlotItem::yAxis() const
{
    return m_data->yAxisId;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle
 * @return An invalid bounding rect: QRectF(1.0, 1.0, -2.0, -2.0)
 * @note A width or height < 0.0 is ignored by the autoscaler.
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界矩形
 * @return 一个无效的边界矩形：QRectF(1.0, 1.0, -2.0, -2.0)
 * @note 宽度或高度 < 0.0 会被自动缩放器忽略。
 * \endif
 */
QRectF QwtPlotItem::boundingRect() const
{
    return QRectF( 1.0, 1.0, -2.0, -2.0 ); // invalid
}

/**
 * \if ENGLISH
 * @brief Calculate a hint for the canvas margin
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * @param[out] left Returns the left margin
 * @param[out] top Returns the top margin
 * @param[out] right Returns the right margin
 * @param[out] bottom Returns the bottom margin
 * @details When the QwtPlotItem::Margins flag is enabled the plot item
 *          indicates, that it needs some margins at the borders of the canvas.
 *          This is e.g. used by bar charts to reserve space for displaying the bars.
 *          The margins are in target device coordinates (pixels on screen).
 *          The default implementation returns 0 for all margins.
 * @sa QwtPlot::getCanvasMarginsHint(), QwtPlot::updateCanvasMargins()
 * \endif
 *
 * \if CHINESE
 * @brief 计算画布边距提示
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @param[in] canvasRect 画布在画笔坐标中的内容矩形
 * @param[out] left 返回左边距
 * @param[out] top 返回上边距
 * @param[out] right 返回右边距
 * @param[out] bottom 返回下边距
 * @details 当启用 QwtPlotItem::Margins 标志时，绘图项表示需要在画布边界处
 *          留出一些边距。例如，柱状图使用此功能来预留显示柱的空间。
 *          边距以目标设备坐标（屏幕上的像素）表示。
 *          默认实现为所有边距返回 0。
 * @sa QwtPlot::getCanvasMarginsHint(), QwtPlot::updateCanvasMargins()
 * \endif
 */
void QwtPlotItem::getCanvasMarginHint( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& canvasRect,
    double& left, double& top, double& right, double& bottom ) const
{
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );
    Q_UNUSED( canvasRect );

    // use QMargins, when we don't need to support Qt < 4.6 anymore
    left = top = right = bottom = 0.0;
}

/**
 * \if ENGLISH
 * @brief Return all information needed to represent the item on the legend
 * @return Data that is needed to represent the item on the legend
 * @details Most items are represented by one entry on the legend showing an icon
 *          and a text, but e.g. QwtPlotMultiBarChart displays one entry for each bar.
 *          QwtLegendData is basically a list of QVariants that makes it possible
 *          to overload and reimplement legendData() to return almost any type of
 *          information, that is understood by the receiver that acts as the legend.
 *          The default implementation returns one entry with the title() of the item
 *          and the legendIcon().
 * @sa title(), legendIcon(), QwtLegend, QwtPlotLegendItem
 * \endif
 *
 * \if CHINESE
 * @brief 返回在图例中表示项目所需的所有信息
 * @return 在图例中表示项目所需的数据
 * @details 大多数项目在图例中由一个条目表示，显示一个图标和一个文本，
 *          但例如 QwtPlotMultiBarChart 为每个柱显示一个条目。
 *          QwtLegendData 基本上是一个 QVariant 列表，使得重载和重新实现 legendData()
 *          可以返回几乎任何类型的信息，这些信息可以被作为图例的接收器理解。
 *          默认实现返回一个包含项目 title() 和 legendIcon() 的条目。
 * @sa title(), legendIcon(), QwtLegend, QwtPlotLegendItem
 * \endif
 */
QList< QwtLegendData > QwtPlotItem::legendData() const
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
 * @brief Update the item to changes of the axes scale division
 * @param[in] xScaleDiv Scale division of the x-axis
 * @param[in] yScaleDiv Scale division of the y-axis
 * @details Update the item, when the axes of plot have changed.
 *          The default implementation does nothing, but items that depend
 *          on the scale division (like QwtPlotGrid()) have to reimplement updateScaleDiv().
 *          updateScaleDiv() is only called when the ScaleInterest interest is enabled.
 *          The default implementation does nothing.
 * @sa QwtPlot::updateAxes(), ScaleInterest
 * \endif
 *
 * \if CHINESE
 * @brief 更新项目以响应坐标轴刻度划分的变化
 * @param[in] xScaleDiv x 轴的刻度划分
 * @param[in] yScaleDiv y 轴的刻度划分
 * @details 当绘图的轴发生变化时更新项目。默认实现不做任何事情，
 *          但依赖于刻度划分的项目（如 QwtPlotGrid()）必须重新实现 updateScaleDiv()。
 *          updateScaleDiv() 仅在启用 ScaleInterest 关注时调用。
 *          默认实现不做任何事情。
 * @sa QwtPlot::updateAxes(), ScaleInterest
 * \endif
 */
void QwtPlotItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    Q_UNUSED( xScaleDiv );
    Q_UNUSED( yScaleDiv );
}

/**
 * \if ENGLISH
 * @brief Update the item to changes of the legend info
 * @param[in] item Plot item to be displayed on a legend
 * @param[in] data Attributes how to display item on the legend
 * @details Plot items that want to display a legend (not those, that want to
 *          be displayed on a legend!) will have to implement updateLegend().
 *          updateLegend() is only called when the LegendInterest interest is enabled.
 *          The default implementation does nothing.
 * @note Plot items, that want to be displayed on a legend need to enable
 *       the QwtPlotItem::Legend flag and to implement legendData() and legendIcon().
 * @sa QwtPlotLegendItem
 * \endif
 *
 * \if CHINESE
 * @brief 更新项目以响应图例信息的变化
 * @param[in] item 要在图例上显示的绘图项目
 * @param[in] data 如何在图例上显示项目的属性
 * @details 想要显示图例的绘图项目（不是那些想要在图例上显示的项目！）
 *          需要实现 updateLegend()。updateLegend() 仅在启用 LegendInterest 关注时调用。
 *          默认实现不做任何事情。
 * @note 想要在图例上显示的绘图项目需要启用 QwtPlotItem::Legend 标志，
 *       并实现 legendData() 和 legendIcon()。
 * @sa QwtPlotLegendItem
 * \endif
 */
void QwtPlotItem::updateLegend( const QwtPlotItem* item,
    const QList< QwtLegendData >& data )
{
    Q_UNUSED( item );
    Q_UNUSED( data );
}

/**
 * \if ENGLISH
 * @brief Calculate the bounding scale rectangle of 2 maps
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @return Bounding scale rect of the scale maps, not normalized
 * \endif
 *
 * \if CHINESE
 * @brief 计算 2 个映射的边界刻度矩形
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @return 刻度映射的边界刻度矩形，未标准化
 * \endif
 */
QRectF QwtPlotItem::scaleRect( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap ) const
{
    return QRectF( xMap.s1(), yMap.s1(),
        xMap.sDist(), yMap.sDist() );
}

/**
 * \if ENGLISH
 * @brief Calculate the bounding paint rectangle of 2 maps
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @return Bounding paint rectangle of the scale maps, not normalized
 * \endif
 *
 * \if CHINESE
 * @brief 计算 2 个映射的边界绘制矩形
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @return 刻度映射的边界绘制矩形，未标准化
 * \endif
 */
QRectF QwtPlotItem::paintRect( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap ) const
{
    const QRectF rect( xMap.p1(), yMap.p1(),
        xMap.pDist(), yMap.pDist() );

    return rect;
}
