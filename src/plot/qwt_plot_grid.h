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

#ifndef QWT_PLOT_GRID_H
#define QWT_PLOT_GRID_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

class QPainter;
class QPen;
class QwtScaleMap;
class QwtScaleDiv;

/**
 * \if ENGLISH
 * @brief A class which draws a coordinate grid
 * @details The QwtPlotGrid class can be used to draw a coordinate grid.
 *          A coordinate grid consists of major and minor vertical
 *          and horizontal grid lines. The locations of the grid lines
 *          are determined by the X and Y scale divisions which can
 *          be assigned with setXDiv() and setYDiv().
 *          The draw() member draws the grid within a bounding
 *          rectangle.
 * \endif
 * 
 * \if CHINESE
 * @brief 绘制坐标网格的类
 * @details QwtPlotGrid 类可用于绘制坐标网格。
 *          坐标网格由主要和次要的垂直和水平网格线组成。
 *          网格线的位置由 X 和 Y 比例尺划分决定，
 *          可以通过 setXDiv() 和 setYDiv() 分配。
 *          draw() 成员在边界矩形内绘制网格。
 * \endif
 */

class QWT_EXPORT QwtPlotGrid : public QwtPlotItem
{
  public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
explicit QwtPlotGrid();

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
virtual ~QwtPlotGrid();

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
virtual int rtti() const override;

    /**
     * \if ENGLISH
     * @brief Enable/disable x-axis grid
     * \endif
     */
void enableX( bool );

    /**
     * \if ENGLISH
     * @brief Check if x-axis grid is enabled
     * \endif
     */
bool xEnabled() const;

    /**
     * \if ENGLISH
     * @brief Enable/disable y-axis grid
     * \endif
     */
void enableY( bool );

    /**
     * \if ENGLISH
     * @brief Check if y-axis grid is enabled
     * \endif
     */
bool yEnabled() const;

    /**
     * \if ENGLISH
     * @brief Enable/disable minor x-axis grid
     * \endif
     */
void enableXMin( bool );

    /**
     * \if ENGLISH
     * @brief Check if minor x-axis grid is enabled
     * \endif
     */
bool xMinEnabled() const;

    /**
     * \if ENGLISH
     * @brief Enable/disable minor y-axis grid
     * \endif
     */
void enableYMin( bool );

    /**
     * \if ENGLISH
     * @brief Check if minor y-axis grid is enabled
     * \endif
     */
bool yMinEnabled() const;

    /**
     * \if ENGLISH
     * @brief Set x-axis scale division
     * \endif
     */
void setXDiv( const QwtScaleDiv& );

    /**
     * \if ENGLISH
     * @brief Get x-axis scale division
     * \endif
     */
const QwtScaleDiv& xScaleDiv() const;

    /**
     * \if ENGLISH
     * @brief Set y-axis scale division
     * \endif
     */
void setYDiv( const QwtScaleDiv& );

    /**
     * \if ENGLISH
     * @brief Get y-axis scale division
     * \endif
     */
const QwtScaleDiv& yScaleDiv() const;

    /**
     * \if ENGLISH
     * @brief Set pen for both major and minor grid lines
     * \endif
     */
void setPen( const QColor&,
    qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    /**
     * \if ENGLISH
     * @brief Set pen for both major and minor grid lines
     * \endif
     */
void setPen( const QPen& );

    /**
     * \if ENGLISH
     * @brief Set pen for major grid lines
     * \endif
     */
void setMajorPen( const QColor&,
    qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    /**
     * \if ENGLISH
     * @brief Set pen for major grid lines
     * \endif
     */
void setMajorPen( const QPen& );

    /**
     * \if ENGLISH
     * @brief Get pen for major grid lines
     * \endif
     */
const QPen& majorPen() const;

    /**
     * \if ENGLISH
     * @brief Set pen for minor grid lines
     * \endif
     */
void setMinorPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    /**
     * \if ENGLISH
     * @brief Set pen for minor grid lines
     * \endif
     */
void setMinorPen( const QPen& );

    /**
     * \if ENGLISH
     * @brief Get pen for minor grid lines
     * \endif
     */
const QPen& minorPen() const;

    /**
     * \if ENGLISH
     * @brief Draw the grid
     * \endif
     */
virtual void draw( QPainter*,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const override;

    /**
     * \if ENGLISH
     * @brief Update scale divisions
     * \endif
     */
virtual void updateScaleDiv(
    const QwtScaleDiv& xScaleDiv, const QwtScaleDiv& yScaleDiv ) override;

  private:
    /**
     * \if ENGLISH
     * @brief Draw grid lines
     * \endif
     */
void drawLines( QPainter*, const QRectF&,
    Qt::Orientation, const QwtScaleMap&,
    const QList< double >& ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
