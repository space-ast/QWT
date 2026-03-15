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

#ifndef QWT_INTERVAL_SYMBOL_H
#define QWT_INTERVAL_SYMBOL_H

#include "qwt_global.h"
#include <qnamespace.h>

class QPainter;
class QPen;
class QBrush;
class QPointF;
class QColor;

/**
 * \if ENGLISH
 * @brief A drawing primitive for displaying an interval like an error bar
 * \sa QwtPlotIntervalCurve
 * \endif
 * \if CHINESE
 * @brief 用于显示区间（如误差条）的绘图基元
 * \sa QwtPlotIntervalCurve
 * \endif
 */
class QWT_EXPORT QwtIntervalSymbol
{
  public:
    //! \if ENGLISH Symbol style \endif \if CHINESE 符号样式 \endif
    enum Style
    {
        //! \if ENGLISH No Style. The symbol cannot be drawn. \endif \if CHINESE 无样式。符号无法绘制。 \endif
        NoSymbol = -1,

        /*!
           \if ENGLISH
           The symbol displays a line with caps at the beginning/end.
           The size of the caps depends on the symbol width().
           \endif
           \if CHINESE
           符号显示一条线，两端有帽盖。
           帽盖的大小取决于符号的宽度。
           \endif
         */
        Bar,

        /*!
           \if ENGLISH
           The symbol displays a plain rectangle using pen() and brush().
           The size of the rectangle depends on the translated interval and the width().
           \endif
           \if CHINESE
           符号使用 pen() 和 brush() 显示一个简单的矩形。
           矩形的大小取决于转换后的区间和宽度。
           \endif
         */
        Box,

        /*!
           \if ENGLISH
           Styles >= UserSymbol are reserved for derived classes of QwtIntervalSymbol
           that overload draw() with additional application specific symbol types.
           \endif
           \if CHINESE
           样式 >= UserSymbol 保留给 QwtIntervalSymbol 的派生类，
           这些类重载 draw() 以添加特定于应用程序的符号类型。
           \endif
         */
        UserSymbol = 1000
    };

  public:
    explicit QwtIntervalSymbol( Style = NoSymbol );
    QwtIntervalSymbol( const QwtIntervalSymbol& );

    virtual ~QwtIntervalSymbol();

    QwtIntervalSymbol& operator=( const QwtIntervalSymbol& );
    bool operator==( const QwtIntervalSymbol& ) const;
    bool operator!=( const QwtIntervalSymbol& ) const;

    void setWidth( int );
    int width() const;

    void setBrush( const QBrush& );
    const QBrush& brush() const;

    void setPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setPen( const QPen& );
    const QPen& pen() const;

    void setStyle( Style );
    Style style() const;

    virtual void draw( QPainter*, Qt::Orientation,
        const QPointF& from, const QPointF& to ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
