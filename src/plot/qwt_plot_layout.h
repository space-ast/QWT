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
 *        - QwtPanner -> QwtCachePanner (pixmap-cache version)
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

#ifndef QWT_PLOT_LAYOUT_H
#define QWT_PLOT_LAYOUT_H

#include "qwt_global.h"
#include "qwt_plot.h"
#include "qwt_axis_id.h"
class QwtPlotLayoutEngine;

/**
 * \if ENGLISH
 * @brief Layout engine for QwtPlot
 * @details It is used by the QwtPlot widget to organize its internal widgets
 *          or by QwtPlot::print() to render its content to a QPaintDevice like
 *          a QPrinter, QPixmap/QImage or QSvgRenderer.
 * @sa QwtPlot::setPlotLayout()
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlot 的布局引擎
 * @details 它被 QwtPlot 部件用于组织其内部部件，
 *          或被 QwtPlot::print() 用于将其内容渲染到 QPaintDevice 上，
 *          如 QPrinter、QPixmap/QImage 或 QSvgRenderer。
 * @sa QwtPlot::setPlotLayout()
 * \endif
 */
class QWT_EXPORT QwtPlotLayout
{
public:
    /**
     * \if ENGLISH
     * @brief Options to configure the plot layout engine
     * @sa activate(), QwtPlotRenderer
     * \endif
     * 
     * \if CHINESE
     * @brief 用于配置绘图布局引擎的选项
     * @sa activate(), QwtPlotRenderer
     * \endif
     */
    enum Option
    {
        /**
         * \if ENGLISH
         * @brief Unused
         * \endif
         * 
         * \if CHINESE
         * @brief 未使用
         * \endif
         */
        AlignScales = 0x01,

        /**
         * \if ENGLISH
         * @brief Ignore the dimension of the scrollbars.
         * @details There are no scrollbars when the plot is not rendered to widgets.
         * \endif
         * 
         * \if CHINESE
         * @brief 忽略滚动条的尺寸
         * @details 当绘图未渲染到部件时，不存在滚动条。
         * \endif
         */
        IgnoreScrollbars = 0x02,

        /**
         * \if ENGLISH
         * @brief Ignore all frames
         * \endif
         * 
         * \if CHINESE
         * @brief 忽略所有边框
         * \endif
         */
        IgnoreFrames = 0x04,

        /**
         * \if ENGLISH
         * @brief Ignore the legend
         * \endif
         * 
         * \if CHINESE
         * @brief 忽略图例
         * \endif
         */
        IgnoreLegend = 0x08,

        /**
         * \if ENGLISH
         * @brief Ignore the title
         * \endif
         * 
         * \if CHINESE
         * @brief 忽略标题
         * \endif
         */
        IgnoreTitle = 0x10,

        /**
         * \if ENGLISH
         * @brief Ignore the footer
         * \endif
         * 
         * \if CHINESE
         * @brief 忽略页脚
         * \endif
         */
        IgnoreFooter = 0x20
    };

    Q_DECLARE_FLAGS( Options, Option )

    explicit QwtPlotLayout();
    virtual ~QwtPlotLayout();

    void setCanvasMargin( int margin, int axis = -1 );
    int canvasMargin( int axisId ) const;

    void setAlignCanvasToScales( bool );
    void setAlignCanvasToScale( int axisId, bool );
    bool alignCanvasToScale( int axisId ) const;

    void setSpacing( int );
    int spacing() const;

    void setLegendPosition( QwtPlot::LegendPosition pos, double ratio );
    void setLegendPosition( QwtPlot::LegendPosition pos );
    QwtPlot::LegendPosition legendPosition() const;

    void setLegendRatio( double ratio );
    double legendRatio() const;

    virtual QSize minimumSizeHint( const QwtPlot* ) const;
    virtual void activate( const QwtPlot* plot, const QRectF& plotRect,
        Options options = Options() );
    virtual void invalidate();

    QRectF titleRect() const;
    QRectF footerRect() const;
    QRectF legendRect() const;
    QRectF scaleRect( QwtAxisId ) const;
    QRectF canvasRect() const;

protected:
    void setTitleRect( const QRectF& );
    void setFooterRect( const QRectF& );
    void setLegendRect( const QRectF& );
    void setScaleRect( QwtAxisId, const QRectF& );
    void setCanvasRect( const QRectF& );
    QwtPlotLayoutEngine* layoutEngine();
    void doActivate( const QwtPlot* plot, const QRectF& plotRect,
        Options options = Options() );

private:
    Q_DISABLE_COPY( QwtPlotLayout )

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotLayout::Options )

#endif
