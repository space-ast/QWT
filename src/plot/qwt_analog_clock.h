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

#ifndef QWT_ANALOG_CLOCK_H
#define QWT_ANALOG_CLOCK_H

#include "qwt_global.h"
#include "qwt_dial.h"

class QwtDialNeedle;

/**
 * \if ENGLISH
 *   \brief An analog clock widget
 *   \details QwtAnalogClock is a widget that displays an analog clock with hour, minute,
 *            and second hands.
 *   \image html analogclock.png
 *   \par Example
 *   \code
 *   #include <qwt_analog_clock.h>
 *   QwtAnalogClock *clock = new QwtAnalogClock(...);
 *   clock->scaleDraw()->setPenWidth(3);
 *   clock->setLineWidth(6);
 *   clock->setFrameShadow(QwtDial::Sunken);
 *   clock->setTime();
 *   // update the clock every second
 *   QTimer *timer = new QTimer(clock);
 *   timer->connect(timer, SIGNAL(timeout()), clock, SLOT(setCurrentTime()));
 *   timer->start(1000);
 *   \endcode
 *   \note The examples/dials example shows how to use QwtAnalogClock.
 * \endif
 * \if CHINESE
 *   \brief 模拟时钟控件
 *   \details QwtAnalogClock是一个显示带有时针、分针和秒针的模拟时钟的控件。
 *   \image html analogclock.png
 *   \par 示例
 *   \code
 *   #include <qwt_analog_clock.h>
 *   QwtAnalogClock *clock = new QwtAnalogClock(...);
 *   clock->scaleDraw()->setPenWidth(3);
 *   clock->setLineWidth(6);
 *   clock->setFrameShadow(QwtDial::Sunken);
 *   clock->setTime();
 *   // 每秒更新时钟
 *   QTimer *timer = new QTimer(clock);
 *   timer->connect(timer, SIGNAL(timeout()), clock, SLOT(setCurrentTime()));
 *   timer->start(1000);
 *   \endcode
 *   \note examples/dials示例展示了如何使用QwtAnalogClock。
 * \endif
 */

class QWT_EXPORT QwtAnalogClock : public QwtDial
{
    Q_OBJECT

  public:
    /**
     * \if ENGLISH
     *   \brief Hand type enumeration
     *   \details Defines the types of clock hands available.
     *   \sa setHand(), hand()
     * \endif
     * \if CHINESE
     *   \brief 时钟指针类型枚举
     *   \details 定义可用的时钟指针类型。
     *   \sa setHand(), hand()
     * \endif
     */
    enum Hand
    {
        //! \if ENGLISH Needle displaying the seconds \endif \if CHINESE 显示秒的指针 \endif
        SecondHand,

        //! \if ENGLISH Needle displaying the minutes \endif \if CHINESE 显示分的指针 \endif
        MinuteHand,

        //! \if ENGLISH Needle displaying the hours \endif \if CHINESE 显示时的指针 \endif
        HourHand,

        //! \if ENGLISH Number of needles \endif \if CHINESE 指针数量 \endif
        NHands
    };

    // Constructs an analog clock widget
    explicit QwtAnalogClock( QWidget* parent = nullptr );
    // Destructor
    virtual ~QwtAnalogClock();

    // Sets a specific clock hand needle
    void setHand( Hand, QwtDialNeedle* );

    // Returns a specific clock hand needle (const version)
    const QwtDialNeedle* hand( Hand ) const;
    // Returns a specific clock hand needle
    QwtDialNeedle* hand( Hand );

  public Q_SLOTS:
    /**
     * \if ENGLISH
     *   \brief Set the clock to display the current time
     *   \details Updates the clock display to show the current system time.
     * \endif
     * \if CHINESE
     *   \brief 设置时钟显示当前时间
     *   \details 更新时钟显示为当前系统时间。
     * \endif
     */
    void setCurrentTime();

    /**
     * \if ENGLISH
     *   \brief Set the clock to display a specific time
     *   \param[in] time Time to display
     * \endif
     * \if CHINESE
     *   \brief 设置时钟显示特定时间
     *   \param[in] time 要显示的时间
     * \endif
     */
    void setTime( const QTime& );

  protected:
    virtual void drawNeedle( QPainter*, const QPointF&, double radius,
        double direction, QPalette::ColorGroup ) const override;

    virtual void drawHand( QPainter*, Hand, const QPointF&,
        double radius, double direction, QPalette::ColorGroup ) const;

  private:
    // use setHand instead
    void setNeedle( QwtDialNeedle* );

    QwtDialNeedle* m_hand[NHands];
};

#endif
