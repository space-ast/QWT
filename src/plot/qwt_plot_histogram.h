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

#ifndef QWT_PLOT_HISTOGRAM_H
#define QWT_PLOT_HISTOGRAM_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"

class QwtColumnSymbol;
class QwtColumnRect;
class QColor;
class QPolygonF;

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/**
 * \if ENGLISH
 * @brief QwtPlotHistogram represents a series of samples, where an interval
 *        is associated with a value ( \f$y = f([x1,x2])\f$ )
 * @details The representation depends on the style() and an optional symbol()
 *          that is displayed for each interval.
 * 
 * @note The term "histogram" is used in a different way in the areas of
 *       digital image processing and statistics. Wikipedia introduces the
 *       terms "image histogram" and "color histogram" to avoid confusions.
 *       While "image histograms" can be displayed by a QwtPlotCurve there
 *       is no applicable plot item for a "color histogram" yet.
 * 
 * @sa QwtPlotBarChart, QwtPlotMultiBarChart
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotHistogram 表示一系列样本，其中每个区间都与一个值相关联 ( \f$y = f([x1,x2])\f$ )
 * @details 表示方式取决于 style() 和为每个区间显示的可选 symbol()。
 * 
 * @note "直方图"一词在数字图像处理和统计学领域有不同的用法。维基百科引入了
 *       "图像直方图"和"颜色直方图"等术语以避免混淆。
 *       虽然"图像直方图"可以通过 QwtPlotCurve 显示，但目前还没有适用于"颜色直方图"的绘图项。
 * 
 * @sa QwtPlotBarChart, QwtPlotMultiBarChart
 * \endif
 */

class QWT_EXPORT QwtPlotHistogram
    : public QwtPlotSeriesItem
    , public QwtSeriesStore< QwtIntervalSample >
{
  public:
    /**
     * \if ENGLISH
     * @brief Histogram styles
     * @details The default style is QwtPlotHistogram::Columns.
     * @sa setStyle(), style(), setSymbol(), symbol(), setBaseline()
     * \endif
     * 
     * \if CHINESE
     * @brief 直方图样式
     * @details 默认样式是 QwtPlotHistogram::Columns。
     * @sa setStyle(), style(), setSymbol(), symbol(), setBaseline()
     * \endif
     */
    enum HistogramStyle
    {
        /**
         * \if ENGLISH
         * Draw an outline around the area, that is build by all intervals
         * using the pen() and fill it with the brush(). The outline style
         * requires, that the intervals are in increasing order and
         * not overlapping.
         * \endif
         * 
         * \if CHINESE
         * 使用 pen() 在所有区间构建的区域周围绘制轮廓，并用 brush() 填充它。
         * 轮廓样式要求区间按递增顺序排列且不重叠。
         * \endif
         */
        Outline,

        /**
         * \if ENGLISH
         * Draw a column for each interval. When a symbol() has been set
         * the symbol is used otherwise the column is displayed as
         * plain rectangle using pen() and brush().
         * \endif
         * 
         * \if CHINESE
         * 为每个区间绘制一个列。当设置了 symbol() 时，使用该符号；
         * 否则，使用 pen() 和 brush() 将列显示为普通矩形。
         * \endif
         */
        Columns,

        /**
         * \if ENGLISH
         * Draw a simple line using the pen() for each interval.
         * \endif
         * 
         * \if CHINESE
         * 使用 pen() 为每个区间绘制一条简单的线。
         * \endif
         */
        Lines,

        /**
         * \if ENGLISH
         * Styles >= UserStyle are reserved for derived
         * classes that overload drawSeries() with
         * additional application specific ways to display a histogram.
         * \endif
         * 
         * \if CHINESE
         * 样式 >= UserStyle 保留给派生类使用，
         * 这些派生类用额外的应用特定方式重载 drawSeries() 来显示直方图。
         * \endif
         */
        UserStyle = 100
    };

    // Constructor
explicit QwtPlotHistogram( const QString& title = QString() );

    // Constructor with QwtText title
explicit QwtPlotHistogram( const QwtText& title );

    // Destructor
virtual ~QwtPlotHistogram();

    // Get the runtime type information
virtual int rtti() const override;

    // Set pen
void setPen( const QColor&,
    qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    // Set pen
void setPen( const QPen& );

    // Get pen
const QPen& pen() const;

    // Set brush
void setBrush( const QBrush& );

    // Get brush
const QBrush& brush() const;

    // Set samples from a vector
void setSamples( const QVector< QwtIntervalSample >& );

    // Set samples from a series data
void setSamples( QwtSeriesData< QwtIntervalSample >* );

    // Set baseline
void setBaseline( double );

    // Get baseline
double baseline() const;

    // Set histogram style
void setStyle( HistogramStyle style );

    // Get histogram style
HistogramStyle style() const;

    // Set symbol
void setSymbol( const QwtColumnSymbol* );

    // Get symbol
const QwtColumnSymbol* symbol() const;

    // Draw the series
virtual void drawSeries( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const override;

    // Get the bounding rectangle
virtual QRectF boundingRect() const override;

    // Get the legend icon
virtual QwtGraphic legendIcon(
    int index, const QSizeF& ) const override;

  protected:
    /**
     * \if ENGLISH
     * @brief Get the column rectangle
     * \endif
     */
virtual QwtColumnRect columnRect( const QwtIntervalSample&,
    const QwtScaleMap&, const QwtScaleMap& ) const;

    /**
     * \if ENGLISH
     * @brief Draw a column
     * \endif
     */
virtual void drawColumn( QPainter*, const QwtColumnRect&,
    const QwtIntervalSample& ) const;

    /**
     * \if ENGLISH
     * @brief Draw columns
     * \endif
     */
void drawColumns( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    int from, int to ) const;

    /**
     * \if ENGLISH
     * @brief Draw outline
     * \endif
     */
void drawOutline( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    int from, int to ) const;

    /**
     * \if ENGLISH
     * @brief Draw lines
     * \endif
     */
void drawLines( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    int from, int to ) const;

  private:
    /**
     * \if ENGLISH
     * @brief Initialize the histogram
     * \endif
     */
void init();

    /**
     * \if ENGLISH
     * @brief Flush polygon
     * \endif
     */
void flushPolygon( QPainter*, double baseLine, QPolygonF& ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
