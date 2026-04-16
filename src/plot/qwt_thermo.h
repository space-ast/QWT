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

#ifndef QWT_THERMO_H
#define QWT_THERMO_H

#include "qwt_global.h"
#include "qwt_abstract_scale.h"
#include "qwt_interval.h"

class QwtScaleDraw;
class QwtColorMap;

/**
 * \if ENGLISH
 * @brief The Thermometer Widget
 * @details QwtThermo is a widget which displays a value in an interval. It supports:
 *          - a horizontal or vertical layout;
 *          - a range;
 *          - a scale;
 *          - an alarm level.
 *          The fill colors might be calculated from an optional color map.
 *          If no color map has been assigned QwtThermo uses the following
 *          colors/brushes from the widget palette:
 *          - QPalette::Base: Background of the pipe
 *          - QPalette::ButtonText: Fill brush below the alarm level
 *          - QPalette::Highlight: Fill brush for the values above the alarm level
 *          - QPalette::WindowText: For the axis of the scale
 *          - QPalette::Text: For the labels of the scale
 * \sa QwtAbstractScale
 * \endif
 * \if CHINESE
 * @brief 温度计控件
 * @details QwtThermo 是一个在区间内显示值的控件。它支持：
 *          - 水平或垂直布局；
 *          - 范围；
 *          - 刻度；
 *          - 报警级别。
 *          填充颜色可以从可选的颜色映射计算。
 *          如果没有分配颜色映射，QwtThermo 使用控件调色板的以下颜色/画刷：
 *          - QPalette::Base：管道背景
 *          - QPalette::ButtonText：报警级别以下的填充画刷
 *          - QPalette::Highlight：报警级别以上的值的填充画刷
 *          - QPalette::WindowText：用于刻度轴
 *          - QPalette::Text：用于刻度标签
 * \sa QwtAbstractScale
 * \endif
 */
class QWT_EXPORT QwtThermo : public QwtAbstractScale
{
    Q_OBJECT

    Q_ENUMS( ScalePosition )
    Q_ENUMS( OriginMode )

    Q_PROPERTY( Qt::Orientation orientation
        READ orientation WRITE setOrientation )
    Q_PROPERTY( ScalePosition scalePosition
        READ scalePosition WRITE setScalePosition )
    Q_PROPERTY( OriginMode originMode READ originMode WRITE setOriginMode )

    Q_PROPERTY( bool alarmEnabled READ alarmEnabled WRITE setAlarmEnabled )
    Q_PROPERTY( double alarmLevel READ alarmLevel WRITE setAlarmLevel )
    Q_PROPERTY( double origin READ origin WRITE setOrigin )
    Q_PROPERTY( int spacing READ spacing WRITE setSpacing )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    Q_PROPERTY( int pipeWidth READ pipeWidth WRITE setPipeWidth )
    Q_PROPERTY( double value READ value WRITE setValue USER true )

  public:

    /**
     * \if ENGLISH
     * @brief Position of the scale
     * \sa setScalePosition(), setOrientation()
     * \endif
     * \if CHINESE
     * @brief 刻度的位置
     * \sa setScalePosition(), setOrientation()
     * \endif
     */
    enum ScalePosition
    {
        /// The slider has no scale
        NoScale,

        /// The scale is right of a vertical or below of a horizontal slider
        LeadingScale,

        /// The scale is left of a vertical or above of a horizontal slider
        TrailingScale
    };

    /**
     * \if ENGLISH
     * @brief Origin mode. This property specifies where the beginning of the liquid is placed.
     * \sa setOriginMode(), setOrigin()
     * \endif
     * \if CHINESE
     * @brief 原点模式。此属性指定液体的起始位置。
     * \sa setOriginMode(), setOrigin()
     * \endif
     */
    enum OriginMode
    {
        /// The origin is the minimum of the scale
        OriginMinimum,

        /// The origin is the maximum of the scale
        OriginMaximum,

        /// The origin is specified using the origin() property
        OriginCustom
    };

    // Constructor
    explicit QwtThermo( QWidget* parent = nullptr );
    // Destructor
    virtual ~QwtThermo();

    // Set the orientation
    void setOrientation( Qt::Orientation );
    // Return the orientation
    Qt::Orientation orientation() const;

    // Set the scale position
    void setScalePosition( ScalePosition );
    // Return the scale position
    ScalePosition scalePosition() const;

    // Set the spacing
    void setSpacing( int );
    // Return the spacing
    int spacing() const;

    // Set the border width
    void setBorderWidth( int );
    // Return the border width
    int borderWidth() const;

    // Set the origin mode
    void setOriginMode( OriginMode );
    // Return the origin mode
    OriginMode originMode() const;

    // Set the origin
    void setOrigin( double );
    // Return the origin
    double origin() const;

    // Set the fill brush
    void setFillBrush( const QBrush& );
    // Return the fill brush
    QBrush fillBrush() const;

    // Set the alarm brush
    void setAlarmBrush( const QBrush& );
    // Return the alarm brush
    QBrush alarmBrush() const;

    // Set the alarm level
    void setAlarmLevel( double );
    // Return the alarm level
    double alarmLevel() const;

    // Set whether alarm is enabled
    void setAlarmEnabled( bool );
    // Return whether alarm is enabled
    bool alarmEnabled() const;

    // Set the color map
    void setColorMap( QwtColorMap* );
    // Return the color map
    QwtColorMap* colorMap();
    // Return the color map (const version)
    const QwtColorMap* colorMap() const;

    // Set the pipe width
    void setPipeWidth( int );
    // Return the pipe width
    int pipeWidth() const;

    // Set the range flags
    void setRangeFlags( QwtInterval::BorderFlags );
    // Return the range flags
    QwtInterval::BorderFlags rangeFlags() const;

    // Return the current value
    double value() const;

    // Return the size hint
    virtual QSize sizeHint() const override;
    // Return the minimum size hint
    virtual QSize minimumSizeHint() const override;

    // Set the scale draw
    void setScaleDraw( QwtScaleDraw* );
    // Return the scale draw
    const QwtScaleDraw* scaleDraw() const;

  public Q_SLOTS:
    // Set the current value
    virtual void setValue( double );

  protected:
    /// Draw the liquid
    virtual void drawLiquid( QPainter*, const QRect& ) const;
    /// Handle scale changes
    virtual void scaleChange() override;

    /// Handle paint events
    virtual void paintEvent( QPaintEvent* ) override;
    /// Handle resize events
    virtual void resizeEvent( QResizeEvent* ) override;
    /// Handle change events
    virtual void changeEvent( QEvent* ) override;

    /// Return the scale draw
    QwtScaleDraw* scaleDraw();

    /// Return the pipe rectangle
    QRect pipeRect() const;
    /// Return the fill rectangle
    QRect fillRect( const QRect& ) const;
    /// Return the alarm rectangle
    QRect alarmRect( const QRect& ) const;

  private:
    /// Layout the thermo
    void layoutThermo( bool );

    class PrivateData;
    PrivateData* m_data;
};

#endif
