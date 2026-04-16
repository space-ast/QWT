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

#include "qwt_analog_clock.h"
#include "qwt_dial_needle.h"
#include "qwt_round_scale_draw.h"
#include "qwt_text.h"
#include "qwt_math.h"

#include <qlocale.h>
#include <qdatetime.h>

namespace
{
class QwtAnalogClockScaleDraw final : public QwtRoundScaleDraw
{
public:
    QwtAnalogClockScaleDraw()
    {
        setSpacing(8);

        enableComponent(QwtAbstractScaleDraw::Backbone, false);

        setTickLength(QwtScaleDiv::MinorTick, 2);
        setTickLength(QwtScaleDiv::MediumTick, 4);
        setTickLength(QwtScaleDiv::MajorTick, 8);

        setPenWidthF(1.0);
    }

    virtual QwtText label(double value) const override
    {
        if (qFuzzyCompare(value + 1.0, 1.0))
            value = 60.0 * 60.0 * 12.0;

        return QLocale().toString(qRound(value / (60.0 * 60.0)));
    }
};
}

/**
 * \if ENGLISH
 *   \brief Constructor
 *   \param[in] parent Parent widget
 *   \details Creates an analog clock widget with default appearance.
 *            The clock is read-only and displays the current time.
 * \endif
 * \if CHINESE
 *   \brief 构造函数
 *   \param[in] parent 父控件
 *   \details 创建一个具有默认外观的模拟时钟控件。
 *            时钟为只读模式，显示当前时间。
 * \endif
 */
QwtAnalogClock::QwtAnalogClock(QWidget* parent) : QwtDial(parent)
{
    setWrapping(true);
    setReadOnly(true);

    setOrigin(270.0);
    setScaleDraw(new QwtAnalogClockScaleDraw());

    setTotalSteps(60);

    const int secondsPerHour = 60.0 * 60.0;

    QList< double > majorTicks;
    QList< double > minorTicks;

    for (int i = 0; i < 12; i++) {
        majorTicks += i * secondsPerHour;

        for (int j = 1; j < 5; j++)
            minorTicks += i * secondsPerHour + j * secondsPerHour / 5.0;
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval(0.0, 12.0 * secondsPerHour);
    scaleDiv.setTicks(QwtScaleDiv::MajorTick, majorTicks);
    scaleDiv.setTicks(QwtScaleDiv::MinorTick, minorTicks);
    setScale(scaleDiv);

    QColor knobColor = palette().color(QPalette::Active, QPalette::Text);
    knobColor        = knobColor.darker(120);

    QColor handColor;
    int width;

    for (int i = 0; i < NHands; i++) {
        if (i == SecondHand) {
            width     = 2;
            handColor = knobColor.darker(120);
        } else {
            width     = 8;
            handColor = knobColor;
        }

        QwtDialSimpleNeedle* hand = new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow, true, handColor, knobColor);
        hand->setWidth(width);

        m_hand[ i ] = nullptr;
        setHand(static_cast< Hand >(i), hand);
    }
}

/**
 * \if ENGLISH
 *   \brief Destructor
 * \endif
 * \if CHINESE
 *   \brief 析构函数
 * \endif
 */
QwtAnalogClock::~QwtAnalogClock()
{
    for (int i = 0; i < NHands; i++)
        delete m_hand[ i ];
}

/**
 * \if ENGLISH
 *   \brief No-op method, use setHand() instead
 *   \sa setHand()
 * \endif
 * \if CHINESE
 *   \brief 空操作方法，请使用setHand()
 *   \sa setHand()
 * \endif
 */
void QwtAnalogClock::setNeedle(QwtDialNeedle*)
{
    // no op
    return;
}

/**
 * \if ENGLISH
 *   \brief Set a clock hand
 *   \param[in] hand Specifies the type of hand (SecondHand, MinuteHand, HourHand)
 *   \param[in] needle Needle object representing the hand
 *   \sa hand()
 * \endif
 * \if CHINESE
 *   \brief 设置时钟指针
 *   \param[in] hand 指针类型（SecondHand、MinuteHand、HourHand）
 *   \param[in] needle 表示指针的针对象
 *   \sa hand()
 * \endif
 */
void QwtAnalogClock::setHand(Hand hand, QwtDialNeedle* needle)
{
    if (hand >= 0 && hand < NHands) {
        delete m_hand[ hand ];
        m_hand[ hand ] = needle;
    }
}

/**
 * \if ENGLISH
 *   \brief Get a clock hand
 *   \param[in] hd Specifies the type of hand
 *   \return Clock hand needle, or nullptr if invalid hand type
 *   \sa setHand()
 * \endif
 * \if CHINESE
 *   \brief 获取时钟指针
 *   \param[in] hd 指针类型
 *   \return 时钟指针针对象，若类型无效则返回nullptr
 *   \sa setHand()
 * \endif
 */
QwtDialNeedle* QwtAnalogClock::hand(Hand hd)
{
    if (hd < 0 || hd >= NHands)
        return nullptr;

    return m_hand[ hd ];
}

/**
 * \if ENGLISH
 *   \brief Get a clock hand (const version)
 *   \param[in] hd Specifies the type of hand
 *   \return Clock hand needle (const), or nullptr if invalid hand type
 *   \sa setHand()
 * \endif
 * \if CHINESE
 *   \brief 获取时钟指针（const版本）
 *   \param[in] hd 指针类型
 *   \return 时钟指针针对象（const），若类型无效则返回nullptr
 *   \sa setHand()
 * \endif
 */
const QwtDialNeedle* QwtAnalogClock::hand(Hand hd) const
{
    return const_cast< QwtAnalogClock* >(this)->hand(hd);
}

/**
 * \if ENGLISH
 *   \brief Set the clock to display the current time
 *   \details Updates the clock to show the current system time.
 * \endif
 * \if CHINESE
 *   \brief 设置时钟显示当前时间
 *   \details 更新时钟显示为当前系统时间。
 * \endif
 */
void QwtAnalogClock::setCurrentTime()
{
    setTime(QTime::currentTime());
}

/**
 * \if ENGLISH
 *   \brief Set the clock to display a specific time
 *   \param[in] time Time to display
 *   \details Sets the clock to display the given time. If the time is invalid,
 *            the clock display is invalidated.
 * \endif
 * \if CHINESE
 *   \brief 设置时钟显示特定时间
 *   \param[in] time 要显示的时间
 *   \details 设置时钟显示给定的时间。如果时间无效，时钟显示将被置为无效状态。
 * \endif
 */
void QwtAnalogClock::setTime(const QTime& time)
{
    if (time.isValid()) {
        setValue((time.hour() % 12) * 60.0 * 60.0 + time.minute() * 60.0 + time.second());
    } else
        setValid(false);
}

/*!
   \brief Draw the needle

   A clock has no single needle but three hands instead. drawNeedle()
   translates value() into directions for the hands and calls
   drawHand().

   \param painter Painter
   \param center Center of the clock
   \param radius Maximum length for the hands
   \param direction Dummy, not used.
   \param colorGroup ColorGroup

   \sa drawHand()
 */
void QwtAnalogClock::drawNeedle(QPainter* painter,
                                const QPointF& center,
                                double radius,
                                double direction,
                                QPalette::ColorGroup colorGroup) const
{
    Q_UNUSED(direction);

    if (isValid()) {
        const double hours   = value() / (60.0 * 60.0);
        const double minutes = (value() - std::floor(hours) * 60.0 * 60.0) / 60.0;
        const double seconds = value() - std::floor(hours) * 60.0 * 60.0 - std::floor(minutes) * 60.0;

        double angle[ NHands ];
        angle[ HourHand ]   = 360.0 * hours / 12.0;
        angle[ MinuteHand ] = 360.0 * minutes / 60.0;
        angle[ SecondHand ] = 360.0 * seconds / 60.0;

        for (int hand = 0; hand < NHands; hand++) {
            const double d = 360.0 - angle[ hand ] - origin();
            drawHand(painter, static_cast< Hand >(hand), center, radius, d, colorGroup);
        }
    }
}

/*!
   Draw a clock hand

   \param painter Painter
   \param hd Specify the type of hand
   \param center Center of the clock
   \param radius Maximum length for the hands
   \param direction Direction of the hand in degrees, counter clockwise
   \param cg ColorGroup
 */
void QwtAnalogClock::drawHand(QPainter* painter,
                              Hand hd,
                              const QPointF& center,
                              double radius,
                              double direction,
                              QPalette::ColorGroup cg) const
{
    const QwtDialNeedle* needle = hand(hd);
    if (needle) {
        if (hd == HourHand)
            radius = qRound(0.8 * radius);

        needle->draw(painter, center, radius, direction, cg);
    }
}
