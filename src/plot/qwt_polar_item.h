/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_ITEM_H
#define QWT_POLAR_ITEM_H

#include "qwt_global.h"
#include "qwt_text.h"
#include "qwt_legend_data.h"
#include "qwt_graphic.h"
#include "qwt_interval.h"

class QString;
class QRect;
class QPointF;
class QPainter;
class QwtPolarPlot;
class QwtScaleMap;
class QwtScaleDiv;

/**
 * \if ENGLISH
 * @brief Base class for items on a polar plot
 * @details A QwtPolarItem is "something that can be painted on the canvas".
 *          It is connected to the QwtPolar framework by a couple of virtual methods,
 *          that are individually implemented in derived item classes.
 *          QwtPolar offers an implementation of the most common types of items,
 *          but deriving from QwtPolarItem makes it easy to implement additional types of items.
 * \endif
 *
 * \if CHINESE
 * @brief 极坐标绘图项的基类
 * @details QwtPolarItem 是"可以在画布上绘制的东西"。
 *          它通过几个虚方法连接到 QwtPolar 框架，这些方法在派生项类中单独实现。
 *          QwtPolar 提供了最常见项类型的实现，但派生自 QwtPolarItem 可以轻松实现其他类型的项。
 * \endif
 */
class QWT_EXPORT QwtPolarItem
{
  public:
    /*!
     * \if ENGLISH
     * @brief Runtime type information
     * @details RttiValues is used to cast plot items, without having to enable runtime type information of the compiler.
     * \endif
     *
     * \if CHINESE
     * @brief 运行时类型信息
     * @details RttiValues 用于转换绘图项，而无需启用编译器的运行时类型信息。
     * \endif
     */
    enum RttiValues
    {
        //! Unspecific value, that can be used, when it doesn't matter
        Rtti_PolarItem = 0,

        //! For QwtPolarGrid
        Rtti_PolarGrid,

        //! For QwtPolarMarker
        Rtti_PolarMarker,

        //! For QwtPolarCurve
        Rtti_PolarCurve,

        //! For QwtPolarSpectrogram
        Rtti_PolarSpectrogram,

        /*!
           Values >= Rtti_PolarUserItem are reserved for plot items
           not implemented in the QwtPolar library.
         */
        Rtti_PolarUserItem = 1000
    };

    /*!
     * \if ENGLISH
     * @brief Plot Item Attributes
     * @sa setItemAttribute(), testItemAttribute()
     * \endif
     *
     * \if CHINESE
     * @brief 绘图项属性
     * @sa setItemAttribute(), testItemAttribute()
     * \endif
     */
    enum ItemAttribute
    {
        //! The item is represented on the legend.
        Legend    = 0x01,

        /*!
           The boundingRect() of the item is included in the
           autoscaling calculation.
         */
        AutoScale = 0x02
    };

    Q_DECLARE_FLAGS( ItemAttributes, ItemAttribute )

    /*!
     * \if ENGLISH
     * @brief Render hints
     * @sa setRenderHint(), testRenderHint()
     * \endif
     *
     * \if CHINESE
     * @brief 渲染提示
     * @sa setRenderHint(), testRenderHint()
     * \endif
     */
    enum RenderHint
    {
        //! Enable antialiasing
        RenderAntialiased = 0x01
    };

    Q_DECLARE_FLAGS( RenderHints, RenderHint )

    /// Constructor
    explicit QwtPolarItem( const QwtText& title = QwtText() );
    /// Destructor
    virtual ~QwtPolarItem();

    /// Attach the item to a plot
    void attach( QwtPolarPlot* plot );
    /// Detach the item from its plot
    void detach();

    /// Get the attached plot
    QwtPolarPlot* plot() const;

    /// Set the title from a QString
    void setTitle( const QString& title );
    /// Set the title from a QwtText
    void setTitle( const QwtText& title );
    /// Get the title
    const QwtText& title() const;

    /// Get the runtime type information
    virtual int rtti() const;

    /// Set an item attribute
    void setItemAttribute( ItemAttribute, bool on = true );
    /// Test an item attribute
    bool testItemAttribute( ItemAttribute ) const;

    /// Set a render hint
    void setRenderHint( RenderHint, bool on = true );
    /// Test a render hint
    bool testRenderHint( RenderHint ) const;

    /// Set the number of render threads
    void setRenderThreadCount( uint numThreads );
    /// Get the number of render threads
    uint renderThreadCount() const;

    /// Get the z value
    double z() const;
    /// Set the z value
    void setZ( double z );

    /// Show the item
    void show();
    /// Hide the item
    void hide();
    /// Set the visibility
    virtual void setVisible( bool );
    /// Get the visibility
    bool isVisible () const;

    /// Update the item and trigger autoRefresh
    virtual void itemChanged();
    /// Update the legend
    virtual void legendChanged();

    /*!
     * \if ENGLISH
     * @brief Draw the item
     * @param[in] painter Painter
     * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
     * @param[in] radialMap Maps radius values into painter coordinates
     * @param[in] pole Position of the pole in painter coordinates
     * @param[in] radius Radius of the complete plot area in painter coordinates
     * @param[in] canvasRect Contents rect of the canvas in painter coordinates
     * \endif
     *
     * \if CHINESE
     * @brief 绘制项
     * @param[in] painter 画师
     * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
     * @param[in] radialMap 将半径值映射到画师坐标
     * @param[in] pole 画师坐标中极点的位置
     * @param[in] radius 画师坐标中完整绘图区域的半径
     * @param[in] canvasRect 画师坐标中画布的内容矩形
     * \endif
     */
    virtual void draw( QPainter* painter,
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, double radius,
        const QRectF& canvasRect ) const = 0;

    /// Get the bounding interval for a scale
    virtual QwtInterval boundingInterval( int scaleId ) const;

    /// Update the scale division
    virtual void updateScaleDiv( const QwtScaleDiv&,
        const QwtScaleDiv&, const QwtInterval& );

    /// Get the margin hint
    virtual int marginHint() const;

    /// Set the legend icon size
    void setLegendIconSize( const QSize& );
    /// Get the legend icon size
    QSize legendIconSize() const;

    /// Get the legend data
    virtual QList< QwtLegendData > legendData() const;
    /// Get the legend icon
    virtual QwtGraphic legendIcon( int index, const QSizeF& ) const;

  private:
    Q_DISABLE_COPY( QwtPolarItem )

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarItem::ItemAttributes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarItem::RenderHints )

Q_DECLARE_METATYPE( QwtPolarItem* )

#endif
