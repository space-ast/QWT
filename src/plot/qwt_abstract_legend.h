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

#ifndef QWT_ABSTRACT_LEGEND_H
#define QWT_ABSTRACT_LEGEND_H
#include <QFrame>
#include <QVariant>
#include <QList>
#include "qwt_global.h"
#include "qwt_legend_data.h"

/**
 * \if ENGLISH
 * @brief Abstract base class for legend widgets
 * @details Legends that need to be under control of the QwtPlot layout system
 *          need to be derived from QwtAbstractLegend.
 * \note Other type of legends can be implemented by connecting to
 *       the QwtPlot::legendDataChanged() signal. But as these legends
 *       are unknown to the plot layout system the layout code
 *       (on screen and for QwtPlotRenderer) need to be organized
 *       in application code.
 * \sa QwtLegend
 * \endif
 * \if CHINESE
 * @brief 图例控件的抽象基类
 * @details 需要受 QwtPlot 布局系统控制的图例需要继承自 QwtAbstractLegend。
 * \note 其他类型的图例可以通过连接到 QwtPlot::legendDataChanged() 信号来实现。
 *       但由于这些图例对绘图布局系统未知，布局代码（屏幕上和 QwtPlotRenderer）
 *       需要在应用程序代码中组织。
 * \sa QwtLegend
 * \endif
 */
class QWT_EXPORT QwtAbstractLegend : public QFrame
{
        Q_OBJECT

public:
        /// Constructor for QwtAbstractLegend (English only)
        explicit QwtAbstractLegend(QWidget* parent = nullptr);
        
        /// Destructor for QwtAbstractLegend (English only)
        virtual ~QwtAbstractLegend();

        /**
         * \if ENGLISH
         * @brief Render the legend into a given rectangle
         * @param painter Painter
         * @param rect Bounding rectangle
         * @param fillBackground When true, fill rect with the widget background
         * \sa renderLegend() is used by QwtPlotRenderer
         * \endif
         * \if CHINESE
         * @brief 将图例渲染到给定的矩形中
         * @param painter 绘制器
         * @param rect 边界矩形
         * @param fillBackground 如果为 true，用控件背景填充矩形
         * \sa renderLegend() 由 QwtPlotRenderer 使用
         * \endif
         */
        virtual void renderLegend(QPainter* painter, const QRectF& rect, bool fillBackground) const = 0;

        /// \if ENGLISH Return true when no plot item is inserted \endif \if CHINESE 当没有插入绘图项时返回 true \endif
        virtual bool isEmpty() const = 0;

        /// \if ENGLISH Return scroll extent \endif \if CHINESE 返回滚动范围 \endif
        virtual int scrollExtent(Qt::Orientation) const;

public Q_SLOTS:

        /**
         * \if ENGLISH
         * @brief Update the entries for a plot item
         * @param itemInfo Info about an item
         * @param data List of legend entry attributes for the item
         * \endif
         * \if CHINESE
         * @brief 更新绘图项的条目
         * @param itemInfo 关于项的信息
         * @param data 该项的图例条目属性列表
         * \endif
         */
        virtual void updateLegend(const QVariant& itemInfo, const QList< QwtLegendData >& data) = 0;
};

#endif
