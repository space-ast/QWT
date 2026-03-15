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

#include "qwt_legend_data.h"
#include "qwt_text.h"
#include "qwt_graphic.h"

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtLegendData::QwtLegendData()
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtLegendData::~QwtLegendData()
{
}

/**
 * \if ENGLISH
 * @brief Set the legend attributes
 * @details QwtLegendData actually is a QMap<int, QVariant> with some
 *          convenience interfaces
 * @param map Values
 * \sa values()
 * \endif
 * \if CHINESE
 * @brief 设置图例属性
 * @details QwtLegendData 实际上是一个带有便捷接口的 QMap<int, QVariant>
 * @param map 值
 * \sa values()
 * \endif
 */
void QwtLegendData::setValues( const QMap< int, QVariant >& map )
{
    m_map = map;
}

/**
 * \if ENGLISH
 * @brief Return the legend attributes
 * @return Legend attributes
 * \sa setValues()
 * \endif
 * \if CHINESE
 * @brief 返回图例属性
 * @return 图例属性
 * \sa setValues()
 * \endif
 */
const QMap< int, QVariant >& QwtLegendData::values() const
{
    return m_map;
}

/**
 * \if ENGLISH
 * @brief Check if a value exists for a specific role
 * @param role Attribute role
 * @return True, when the internal map has an entry for role
 * \sa setValue(), value()
 * \endif
 * \if CHINESE
 * @brief 检查特定角色是否有值
 * @param role 属性角色
 * @return 当内部映射包含该角色时返回 true
 * \sa setValue(), value()
 * \endif
 */
bool QwtLegendData::hasRole( int role ) const
{
    return m_map.contains( role );
}

/**
 * \if ENGLISH
 * @brief Set an attribute value
 * @param role Attribute role
 * @param data Attribute value
 * \sa value()
 * \endif
 * \if CHINESE
 * @brief 设置属性值
 * @param role 属性角色
 * @param data 属性值
 * \sa value()
 * \endif
 */
void QwtLegendData::setValue( int role, const QVariant& data )
{
    m_map[role] = data;
}

/**
 * \if ENGLISH
 * @brief Return the attribute value for a specific role
 * @param role Attribute role
 * @return Attribute value for a specific role
 * \sa setValue()
 * \endif
 * \if CHINESE
 * @brief 返回特定角色的属性值
 * @param role 属性角色
 * @return 特定角色的属性值
 * \sa setValue()
 * \endif
 */
QVariant QwtLegendData::value( int role ) const
{
    if ( !m_map.contains( role ) )
        return QVariant();

    return m_map[role];
}

/**
 * \if ENGLISH
 * @brief Check if the legend data is valid
 * @return True, when the internal map is empty
 * \sa setValues(), setValue()
 * \endif
 * \if CHINESE
 * @brief 检查图例数据是否有效
 * @return 当内部映射为空时返回 true
 * \sa setValues(), setValue()
 * \endif
 */
bool QwtLegendData::isValid() const
{
    return !m_map.isEmpty();
}

/**
 * \if ENGLISH
 * @brief Return the title
 * @return Value of the TitleRole attribute
 * \sa icon(), mode()
 * \endif
 * \if CHINESE
 * @brief 返回标题
 * @return TitleRole 属性的值
 * \sa icon(), mode()
 * \endif
 */
QwtText QwtLegendData::title() const
{
    QwtText text;

    const QVariant titleValue = value( QwtLegendData::TitleRole );
    if ( titleValue.canConvert< QwtText >() )
    {
        text = qvariant_cast< QwtText >( titleValue );
    }
    else if ( titleValue.canConvert< QString >() )
    {
        text.setText( qvariant_cast< QString >( titleValue ) );
    }

    return text;
}

/**
 * \if ENGLISH
 * @brief Return the icon
 * @return Value of the IconRole attribute
 * \sa title(), mode()
 * \endif
 * \if CHINESE
 * @brief 返回图标
 * @return IconRole 属性的值
 * \sa title(), mode()
 * \endif
 */
QwtGraphic QwtLegendData::icon() const
{
    const QVariant iconValue = value( QwtLegendData::IconRole );

    QwtGraphic graphic;
    if ( iconValue.canConvert< QwtGraphic >() )
    {
        graphic = qvariant_cast< QwtGraphic >( iconValue );
    }

    return graphic;
}

/**
 * \if ENGLISH
 * @brief Return the mode
 * @return Value of the ModeRole attribute
 * @return ReadOnly when no mode is set
 * \sa title(), icon()
 * \endif
 * \if CHINESE
 * @brief 返回模式
 * @return ModeRole 属性的值
 * @return 如果未设置模式则返回 ReadOnly
 * \sa title(), icon()
 * \endif
 */
QwtLegendData::Mode QwtLegendData::mode() const
{
    const QVariant modeValue = value( QwtLegendData::ModeRole );
    if ( modeValue.canConvert< int >() )
    {
        const int mode = qvariant_cast< int >( modeValue );
        return static_cast< QwtLegendData::Mode >( mode );
    }

    return QwtLegendData::ReadOnly;
}

