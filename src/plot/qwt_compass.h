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

#ifndef QWT_COMPASS_H
#define QWT_COMPASS_H

#include "qwt_global.h"
#include "qwt_dial.h"
#include "qwt_round_scale_draw.h"

class QwtCompassRose;
class QString;
template< class Key, class T > class QMap;


/**
 * \if ENGLISH
 *   \brief A special scale draw made for QwtCompass
 *   \details QwtCompassScaleDraw maps values to strings using a special map,
 *            that can be modified by the application.
 *            The default map consists of the labels N, NE, E, SE, S, SW, W, NW.
 *   \sa QwtCompass
 * \endif
 * \if CHINESE
 *   \brief 专为QwtCompass设计的特殊刻度绘制类
 *   \details QwtCompassScaleDraw使用特殊映射将数值转换为字符串，
 *            该映射可由应用程序修改。
 *            默认映射包含标签N、NE、E、SE、S、SW、W、NW。
 *   \sa QwtCompass
 * \endif
 */
class QWT_EXPORT QwtCompassScaleDraw : public QwtRoundScaleDraw
{
  public:
    // Constructs a compass scale draw with default label map (N, NE, E, SE, S, SW, W, NW)
    explicit QwtCompassScaleDraw();
    // Constructs a compass scale draw with a custom label map
    explicit QwtCompassScaleDraw( const QMap< double, QString >& map );

    // Destructor
    virtual ~QwtCompassScaleDraw();

    // Sets the map that maps values to labels
    void setLabelMap( const QMap< double, QString >& map );
    // Returns the map that maps values to labels
    QMap< double, QString > labelMap() const;

    // Returns the label for a given value by looking up the label map
    virtual QwtText label( double value ) const override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/**
 * \if ENGLISH
 *   \brief A Compass Widget
 *   \details QwtCompass is a widget to display and enter directions.
 *            It consists of a scale, an optional needle and rose.
 *   \image html dials1.png
 *   \note The examples/dials example shows how to use QwtCompass.
 * \endif
 * \if CHINESE
 *   \brief 指南针控件
 *   \details QwtCompass是一个用于显示和输入方向的控件。
 *            它由刻度盘、可选的指针和罗盘花组成。
 *   \image html dials1.png
 *   \note examples/dials示例展示了如何使用QwtCompass。
 * \endif
 */

class QWT_EXPORT QwtCompass : public QwtDial
{
    Q_OBJECT

  public:
    // Constructs a compass widget with a scale, no needle and no rose
    explicit QwtCompass( QWidget* parent = nullptr );
    // Destructor
    virtual ~QwtCompass();

    // Sets a compass rose that will be drawn inside the compass
    void setRose( QwtCompassRose* rose );
    // Returns the compass rose (const version)
    const QwtCompassRose* rose() const;
    // Returns the compass rose
    QwtCompassRose* rose();

  protected:
    virtual void drawRose( QPainter*, const QPointF& center,
        double radius, double north, QPalette::ColorGroup ) const;

    virtual void drawScaleContents( QPainter*,
        const QPointF& center, double radius ) const override;

    virtual void keyPressEvent( QKeyEvent* ) override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
