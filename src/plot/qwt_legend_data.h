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

#ifndef QWT_LEGEND_DATA_H
#define QWT_LEGEND_DATA_H

#include "qwt_global.h"

#include <qvariant.h>
#include <qmap.h>

class QwtText;
class QwtGraphic;

/**
 * \if ENGLISH
 * @brief Attributes of an entry on a legend
 * @details QwtLegendData is an abstract container (like QAbstractModel)
 *          to exchange attributes that are only known between the plot item and the legend.
 *          By overloading QwtPlotItem::legendData() any other set of attributes
 *          could be used that can be handled by a modified (or completely different)
 *          implementation of a legend.
 * \sa QwtLegend, QwtPlotLegendItem
 * \note The stockchart example implements a legend as a tree with checkable items
 * \endif
 * \if CHINESE
 * @brief 图例条目的属性
 * @details QwtLegendData 是一个抽象容器（如 QAbstractModel），
 *          用于在绘图项和图例之间交换属性。
 *          通过重载 QwtPlotItem::legendData() 可以使用任何其他属性集，
 *          这些属性集可以由修改过的（或完全不同的）图例实现处理。
 * \sa QwtLegend, QwtPlotLegendItem
 * \note stockchart 示例将图例实现为带有可检查项的树
 * \endif
 */
class QWT_EXPORT QwtLegendData
{
  public:
    /**
     * \if ENGLISH
     * @brief Mode defining how a legend entry interacts
     * \endif
     * \if CHINESE
     * @brief 定义图例条目如何交互的模式
     * \endif
     */
    enum Mode
    {
        /**
         * \if ENGLISH
         * @brief The legend item is not interactive, like a label
         * \endif
         * \if CHINESE
         * @brief 图例项不可交互，如标签
         * \endif
         */
        ReadOnly,

        /**
         * \if ENGLISH
         * @brief The legend item is clickable, like a push button
         * \endif
         * \if CHINESE
         * @brief 图例项可点击，如按钮
         * \endif
         */
        Clickable,

        /**
         * \if ENGLISH
         * @brief The legend item is checkable, like a checkable button
         * \endif
         * \if CHINESE
         * @brief 图例项可检查，如复选框
         * \endif
         */
        Checkable
    };

    /**
     * \if ENGLISH
     * @brief Identifier how to interpret a QVariant
     * \endif
     * \if CHINESE
     * @brief 如何解释 QVariant 的标识符
     * \endif
     */
    enum Role
    {
        /**
         * \if ENGLISH
         * @brief The value is a Mode
         * \endif
         * \if CHINESE
         * @brief 值是 Mode
         * \endif
         */
        ModeRole,

        /**
         * \if ENGLISH
         * @brief The value is a title
         * \endif
         * \if CHINESE
         * @brief 值是标题
         * \endif
         */
        TitleRole,

        /**
         * \if ENGLISH
         * @brief The value is an icon
         * \endif
         * \if CHINESE
         * @brief 值是图标
         * \endif
         */
        IconRole,

        /**
         * \if ENGLISH
         * @brief Values < UserRole are reserved for internal use
         * \endif
         * \if CHINESE
         * @brief 值 < UserRole 保留给内部使用
         * \endif
         */
        UserRole  = 32
    };

    // Constructor
    QwtLegendData();
    // Destructor
    ~QwtLegendData();

    // Set all values
    void setValues( const QMap< int, QVariant >& );
    // Return all values
    const QMap< int, QVariant >& values() const;

    // Set a value for a specific role
    void setValue( int role, const QVariant& );
    // Return the value for a specific role
    QVariant value( int role ) const;

    // Check if a value exists for a specific role
    bool hasRole( int role ) const;
    // Check if the legend data is valid
    bool isValid() const;

    // Return the icon
    QwtGraphic icon() const;
    // Return the title
    QwtText title() const;
    // Return the mode
    Mode mode() const;

  private:
    QMap< int, QVariant > m_map;
};

#endif
