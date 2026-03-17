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

#ifndef QWT_PLOT_TEXT_LABEL_H
#define QWT_PLOT_TEXT_LABEL_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

class QwtText;

/**
 * \if ENGLISH
 * @brief A plot item, which displays a text label
 * @details QwtPlotTextLabel displays a text label aligned to the plot canvas.
 * 
 *          In opposite to QwtPlotMarker the position of the label is unrelated to
 *          plot coordinates.
 * 
 *          As drawing a text is an expensive operation the label is cached
 *          in a pixmap to speed up replots.
 * 
 * @par Example
 *  The following code shows how to add a title.
 *  \code
 *    QwtText title( "Plot Title" );
 *    title.setRenderFlags( Qt::AlignHCenter | Qt::AlignTop );
 * 
 *    QFont font;
 *    font.setBold( true );
 *    title.setFont( font );
 * 
 *    QwtPlotTextLabel *titleItem = new QwtPlotTextLabel();
 *    titleItem->setText( title );
 *    titleItem->attach( plot );
 *  \endcode
 * 
 * @sa QwtPlotMarker
 * \endif
 * 
 * \if CHINESE
 * @brief 显示文本标签的绘图项
 * @details QwtPlotTextLabel 显示一个与绘图画布对齐的文本标签。
 * 
 *          与 QwtPlotMarker 不同，标签的位置与绘图坐标无关。
 * 
 *          由于绘制文本是一项昂贵的操作，标签会被缓存在 pixmap 中以加快重绘速度。
 * 
 * @par 示例
 *  以下代码显示如何添加标题。
 *  \code
 *    QwtText title( "Plot Title" );
 *    title.setRenderFlags( Qt::AlignHCenter | Qt::AlignTop );
 * 
 *    QFont font;
 *    font.setBold( true );
 *    title.setFont( font );
 * 
 *    QwtPlotTextLabel *titleItem = new QwtPlotTextLabel();
 *    titleItem->setText( title );
 *    titleItem->attach( plot );
 *  \endcode
 * 
 * @sa QwtPlotMarker
 * \endif
 */

class QWT_EXPORT QwtPlotTextLabel : public QwtPlotItem
{
  public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    QwtPlotTextLabel();
    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotTextLabel();

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
    virtual int rtti() const override;

    /**
     * \if ENGLISH
     * @brief Set the text
     * \endif
     */
    void setText( const QwtText& );
    /**
     * \if ENGLISH
     * @brief Get the text
     * \endif
     */
    QwtText text() const;

    /**
     * \if ENGLISH
     * @brief Set the margin
     * \endif
     */
    void setMargin( int margin );
    /**
     * \if ENGLISH
     * @brief Get the margin
     * \endif
     */
    int margin() const;

    /**
     * \if ENGLISH
     * @brief Calculate the text rectangle
     * \endif
     */
    virtual QRectF textRect( const QRectF&, const QSizeF& ) const;

  protected:
    /**
     * \if ENGLISH
     * @brief Draw the text label
     * \endif
     */
    virtual void draw( QPainter*,
        const QwtScaleMap&, const QwtScaleMap&,
        const QRectF&) const override;

    /**
     * \if ENGLISH
     * @brief Invalidate the cached pixmap
     * \endif
     */
    void invalidateCache();

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
