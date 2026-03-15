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

#ifndef QWT_SLIDER_H
#define QWT_SLIDER_H

#include "qwt_global.h"
#include "qwt_abstract_slider.h"

class QwtScaleDraw;

/**
 * \if ENGLISH
 * @brief The Slider Widget
 * @details QwtSlider is a slider widget which operates on an interval
 *          of type double. Its position is related to a scale showing
 *          the current value.
 *          The slider can be customized by having a through, a groove - or both.
 * \image html sliders.png
 * \endif
 * \if CHINESE
 * @brief 滑块控件
 * @details QwtSlider 是一个在 double 类型区间上操作的滑块控件。其位置与显示
 *          当前值的刻度相关联。
 *          滑块可以通过拥有槽、凹槽或两者来进行自定义。
 * \image html sliders.png
 * \endif
 */

class QWT_EXPORT QwtSlider : public QwtAbstractSlider
{
    Q_OBJECT

    Q_ENUMS( ScalePosition BackgroundStyle )

    Q_PROPERTY( Qt::Orientation orientation
        READ orientation WRITE setOrientation )
    Q_PROPERTY( ScalePosition scalePosition READ scalePosition
        WRITE setScalePosition )

    Q_PROPERTY( bool trough READ hasTrough WRITE setTrough )
    Q_PROPERTY( bool groove READ hasGroove WRITE setGroove )

    Q_PROPERTY( QSize handleSize READ handleSize WRITE setHandleSize )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    Q_PROPERTY( int spacing READ spacing WRITE setSpacing )

  public:

    /**
     * \if ENGLISH
     * @brief Position of the scale
     * \sa QwtSlider(), setScalePosition(), setOrientation()
     * \endif
     * \if CHINESE
     * @brief 刻度位置
     * \sa QwtSlider(), setScalePosition(), setOrientation()
     * \endif
     */
    enum ScalePosition
    {
        /// The slider has no scale
        NoScale,

        /// The scale is right of a vertical or below a horizontal slider
        LeadingScale,

        /// The scale is left of a vertical or above a horizontal slider
        TrailingScale
    };

    /// Constructor
    explicit QwtSlider( QWidget* parent = nullptr );
    /// Constructor with orientation
    explicit QwtSlider( Qt::Orientation, QWidget* parent = nullptr );

    /// Destructor
    virtual ~QwtSlider();

    /// Set orientation
    void setOrientation( Qt::Orientation );
    /// Return orientation
    Qt::Orientation orientation() const;

    /// Set scale position
    void setScalePosition( ScalePosition );
    /// Return scale position
    ScalePosition scalePosition() const;

    /// Enable/disable trough
    void setTrough( bool );
    /// Check if has trough
    bool hasTrough() const;

    /// Enable/disable groove
    void setGroove( bool );
    /// Check if has groove
    bool hasGroove() const;

    /// Set handle size
    void setHandleSize( const QSize& );
    /// Return handle size
    QSize handleSize() const;

    /// Set border width
    void setBorderWidth( int );
    /// Return border width
    int borderWidth() const;

    /// Set spacing
    void setSpacing( int );
    /// Return spacing
    int spacing() const;

    /// Return size hint
    virtual QSize sizeHint() const override;
    /// Return minimum size hint
    virtual QSize minimumSizeHint() const override;

    /// Set scale draw
    void setScaleDraw( QwtScaleDraw* );
    /// Return scale draw (const version)
    const QwtScaleDraw* scaleDraw() const;

    /// Set update interval
    void setUpdateInterval( int );
    /// Return update interval
    int updateInterval() const;

  protected:
    /// Calculate scrolled position
    virtual double scrolledTo( const QPoint& ) const override;
    /// Check if position is scroll position
    virtual bool isScrollPosition( const QPoint& ) const override;

    /// Draw slider
    virtual void drawSlider ( QPainter*, const QRect& ) const;
    /// Draw handle
    virtual void drawHandle( QPainter*, const QRect&, int pos ) const;

    /// Mouse press event
    virtual void mousePressEvent( QMouseEvent* ) override;
    /// Mouse release event
    virtual void mouseReleaseEvent( QMouseEvent* ) override;
    /// Resize event
    virtual void resizeEvent( QResizeEvent* ) override;
    /// Paint event
    virtual void paintEvent ( QPaintEvent* ) override;
    /// Change event
    virtual void changeEvent( QEvent* ) override;
    /// Timer event
    virtual void timerEvent( QTimerEvent* ) override;

    /// Event handler
    virtual bool event( QEvent* ) override;

    /// Scale change notification
    virtual void scaleChange() override;

    /// Return slider rectangle
    QRect sliderRect() const;
    /// Return handle rectangle
    QRect handleRect() const;

  private:
    /// Return scale draw (non-const version)
    QwtScaleDraw* scaleDraw();

    /// Layout slider
    void layoutSlider( bool );
    /// Initialize slider
    void initSlider( Qt::Orientation );

    class PrivateData;
    PrivateData* m_data;
};

#endif
