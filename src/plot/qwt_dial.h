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

#ifndef QWT_DIAL_H
#define QWT_DIAL_H

#include "qwt_global.h"
#include "qwt_abstract_slider.h"

#include <qframe.h>
#include <qpalette.h>

class QwtDialNeedle;
class QwtRoundScaleDraw;
class QwtAbstractScaleDraw;

/**
 * \if ENGLISH
 * @brief QwtDial class provides a rounded range control
 * @details QwtDial is intended as base class for dial widgets like
 *          speedometers, compass widgets, clocks...
 *          A dial contains a scale and a needle indicating the current value
 *          of the dial. Depending on Mode one of them is fixed and the
 *          other is rotating. If not isReadOnly() the
 *          dial can be rotated by dragging the mouse or using keyboard inputs
 *          (see QwtAbstractSlider::keyPressEvent()). A dial might be wrapping, what means
 *          a rotation below/above one limit continues on the other limit (f.e compass).
 *          The scale might cover any arc of the dial, its values are related to
 *          the origin() of the dial.
 *          Often dials have to be updated very often according to values from external
 *          devices. For these high refresh rates QwtDial caches as much as possible.
 *          For derived classes it might be necessary to clear these caches manually
 *          according to attribute changes using invalidateCache().
 * \sa QwtCompass, QwtAnalogClock, QwtDialNeedle
 * \note The controls and dials examples shows different types of dials.
 * \note QDial is more similar to QwtKnob than to QwtDial
 * \endif
 * \if CHINESE
 * @brief QwtDial 类提供圆形范围控件
 * @details QwtDial 旨在作为表盘控件（如速度计、指南针控件、时钟等）的基类。
 *          表盘包含一个刻度和一个指示当前值的指针。根据 Mode，刻度或指针是固定的，
 *          另一个是旋转的。如果不是 isReadOnly()，可以通过拖动鼠标或使用键盘输入
 *          （参见 QwtAbstractSlider::keyPressEvent())）来旋转表盘。
 *          表盘可以循环，这意味着低于/高于一个限制的旋转会在另一个限制上继续（例如指南针）。
 *          刻度可以覆盖表盘的任意弧段，其值与表盘的 origin() 相关。
 *          表盘通常需要根据外部设备的值频繁更新。对于这些高刷新率，QwtDial 尽可能缓存。
 *          对于派生类，可能需要根据属性变化使用 invalidateCache() 手动清除这些缓存。
 * \sa QwtCompass, QwtAnalogClock, QwtDialNeedle
 * \note controls 和 dials 示例展示了不同类型的表盘。
 * \note QDial 更类似于 QwtKnob 而不是 QwtDial
 * \endif
 */

class QWT_EXPORT QwtDial : public QwtAbstractSlider
{
    Q_OBJECT

    Q_ENUMS( Shadow Mode Direction )

    Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
    Q_PROPERTY( Shadow frameShadow READ frameShadow WRITE setFrameShadow )
    Q_PROPERTY( Mode mode READ mode WRITE setMode )
    Q_PROPERTY( double origin READ origin WRITE setOrigin )
    Q_PROPERTY( double minScaleArc READ minScaleArc WRITE setMinScaleArc )
    Q_PROPERTY( double maxScaleArc READ maxScaleArc WRITE setMaxScaleArc )

  public:

    /**
     * \if ENGLISH
     * @brief Frame shadow
     * @details Unfortunately it is not possible to use QFrame::Shadow
     *          as a property of a widget that is not derived from QFrame.
     *          The following enum is made for the designer only. It is safe
     *          to use QFrame::Shadow instead.
     * \endif
     * \if CHINESE
     * @brief 框架阴影
     * @details 不幸的是，不能将 QFrame::Shadow 作为非 QFrame派生控件的属性。
     *          以下枚举仅为设计器设计。使用 QFrame::Shadow 是安全的。
     * \endif
     */
    enum Shadow
    {
        /// QFrame::Plain
        Plain = QFrame::Plain,

        /// QFrame::Raised
        Raised = QFrame::Raised,

        /// QFrame::Sunken
        Sunken = QFrame::Sunken
    };

    /**
     * \if ENGLISH
     * @brief Mode controlling whether the needle or the scale is rotating
     * \endif
     * \if CHINESE
     * @brief 控制指针或刻度是否旋转的模式
     * \endif
     */
    enum Mode
    {
        /// The needle is rotating
        RotateNeedle,

        /// The needle is fixed, the scales are rotating
        RotateScale
    };

    /// Constructor
    explicit QwtDial( QWidget* parent = nullptr );
    /// Destructor
    virtual ~QwtDial();

    /// Set the frame shadow
    void setFrameShadow( Shadow );
    /// Return the frame shadow
    Shadow frameShadow() const;

    /// Set the line width
    void setLineWidth( int );
    /// Return the line width
    int lineWidth() const;

    /// Set the mode
    void setMode( Mode );
    /// Return the mode
    Mode mode() const;

    /// Set the scale arc range
    void setScaleArc( double minArc, double maxArc );

    /// Set the minimum scale arc
    void setMinScaleArc( double );
    /// Return the minimum scale arc
    double minScaleArc() const;

    /// Set the maximum scale arc
    void setMaxScaleArc( double );
    /// Return the maximum scale arc
    double maxScaleArc() const;

    /// Set the origin
    virtual void setOrigin( double );
    /// Return the origin
    double origin() const;

    /// Set the needle
    void setNeedle( QwtDialNeedle* );
    /// Return the needle (const version)
    const QwtDialNeedle* needle() const;
    /// Return the needle
    QwtDialNeedle* needle();

    /// Return the bounding rectangle
    QRect boundingRect() const;
    /// Return the inner rectangle
    QRect innerRect() const;

    /// Return the scale inner rectangle
    virtual QRect scaleInnerRect() const;

    /// Return the size hint
    virtual QSize sizeHint() const override;
    /// Return the minimum size hint
    virtual QSize minimumSizeHint() const override;

    /// Set the scale draw
    void setScaleDraw( QwtRoundScaleDraw* );

    /// Return the scale draw
    QwtRoundScaleDraw* scaleDraw();
    /// Return the scale draw (const version)
    const QwtRoundScaleDraw* scaleDraw() const;

  protected:
    virtual void wheelEvent( QWheelEvent* ) override;
    virtual void paintEvent( QPaintEvent* ) override;
    virtual void changeEvent( QEvent* ) override;

    virtual void drawFrame( QPainter* );
    virtual void drawContents( QPainter* ) const;
    virtual void drawFocusIndicator( QPainter* ) const;

    void invalidateCache();

    virtual void drawScale( QPainter*,
        const QPointF& center, double radius ) const;

    virtual void drawScaleContents( QPainter* painter,
        const QPointF& center, double radius ) const;

    virtual void drawNeedle( QPainter*, const QPointF&,
        double radius, double direction, QPalette::ColorGroup ) const;

    virtual double scrolledTo( const QPoint& ) const override;
    virtual bool isScrollPosition( const QPoint& ) const override;

    virtual void sliderChange() override;
    virtual void scaleChange() override;

  private:
    void setAngleRange( double angle, double span );
    void drawNeedle( QPainter* ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
