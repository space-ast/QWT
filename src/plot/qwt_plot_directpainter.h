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

#ifndef QWT_PLOT_DIRECT_PAINTER_H
#define QWT_PLOT_DIRECT_PAINTER_H

#include "qwt_global.h"
#include <qobject.h>

class QRegion;
class QwtPlotSeriesItem;

/**
 * \if ENGLISH
 * @brief Painter object trying to paint incrementally
 * @details Often applications want to display samples while they are
 *          collected. When there are too many samples complete replots
 *          will be expensive to be processed in a collection cycle.
 * 
 *          QwtPlotDirectPainter offers an API to paint
 *          subsets ( f.e all additions points ) without erasing/repainting
 *          the plot canvas.
 * 
 *          On certain environments it might be important to calculate a proper
 *          clip region before painting. F.e. for Qt Embedded only the clipped part
 *          of the backing store will be copied to a ( maybe unaccelerated )
 *          frame buffer.
 * 
 * @warning Incremental painting will only help when no replot is triggered
 *          by another operation ( like changing scales ) and nothing needs
 *          to be erased.
 * \endif
 * 
 * \if CHINESE
 * @brief 尝试增量绘制的绘制器对象
 * @details 通常应用程序希望在收集样本时显示它们。当样本太多时，
 *          在收集周期中处理完整的重绘会很昂贵。
 * 
 *          QwtPlotDirectPainter 提供了一个 API 来绘制
 *          子集（例如所有添加的点），而无需擦除/重绘
 *          绘图画布。
 * 
 *          在某些环境中，在绘制前计算适当的裁剪区域可能很重要。
 *          例如，对于 Qt Embedded，只有裁剪部分的后备存储会被复制到
 *          （可能未加速的）帧缓冲区。
 * 
 * @warning 只有当没有其他操作（如更改比例尺）触发重绘且不需要
 *          擦除任何内容时，增量绘制才会有帮助。
 * \endif
 */
class QWT_EXPORT QwtPlotDirectPainter : public QObject
{
  public:
    /**
     * \if ENGLISH
     * @brief Paint attributes
     * @sa setAttribute(), testAttribute(), drawSeries()
     * \endif
     * 
     * \if CHINESE
     * @brief 绘制属性
     * @sa setAttribute(), testAttribute(), drawSeries()
     * \endif
     */
    enum Attribute
    {
        /**
         * \if ENGLISH
         * Initializing a QPainter is an expensive operation.
         * When AtomicPainter is set each call of drawSeries() opens/closes
         * a temporary QPainter. Otherwise QwtPlotDirectPainter tries to
         * use the same QPainter as long as possible.
         * \endif
         * 
         * \if CHINESE
         * 初始化 QPainter 是一项昂贵的操作。
         * 当设置了 AtomicPainter 时，每次调用 drawSeries() 都会打开/关闭
         * 一个临时的 QPainter。否则，QwtPlotDirectPainter 会尝试
         * 尽可能长时间地使用同一个 QPainter。
         * \endif
         */
        AtomicPainter = 0x01,

        /**
         * \if ENGLISH
         * When FullRepaint is set the plot canvas is explicitly repainted
         * after the samples have been rendered.
         * \endif
         * 
         * \if CHINESE
         * 当设置了 FullRepaint 时，在样本渲染后会显式重绘绘图画布。
         * \endif
         */
        FullRepaint = 0x02,

        /**
         * \if ENGLISH
         * When QwtPlotCanvas::BackingStore is enabled the painter
         * has to paint to the backing store and the widget. In certain
         * situations/environments it might be faster to paint to
         * the backing store only and then copy the backing store to the canvas.
         * This flag can also be useful for settings, where Qt fills the
         * the clip region with the widget background.
         * \endif
         * 
         * \if CHINESE
         * 当 QwtPlotCanvas::BackingStore 启用时，绘制器
         * 必须绘制到后备存储和部件。在某些情况下/环境中，
         * 只绘制到后备存储然后将后备存储复制到画布可能会更快。
         * 此标志对于 Qt 用部件背景填充裁剪区域的设置也很有用。
         * \endif
         */
        CopyBackingStore = 0x04
    };

    Q_DECLARE_FLAGS( Attributes, Attribute )

    /// Constructor
    explicit QwtPlotDirectPainter( QObject* parent = nullptr );
    /// Destructor
    virtual ~QwtPlotDirectPainter();

    /// Set attribute
    void setAttribute( Attribute, bool on );
    /// Test attribute
    bool testAttribute( Attribute ) const;

    /// Set clipping
    void setClipping( bool );
    /// Check if clipping is enabled
    bool hasClipping() const;

    /// Set clip region
    void setClipRegion( const QRegion& );
    /// Get clip region
    QRegion clipRegion() const;

    /// Draw series
    void drawSeries( QwtPlotSeriesItem*, int from, int to );
    /// Reset the painter
    void reset();

    /// Event filter
    virtual bool eventFilter( QObject*, QEvent* ) override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotDirectPainter::Attributes )

#endif
