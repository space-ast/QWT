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

#include "qwt_date_scale_draw.h"
#include "qwt_text.h"

class QwtDateScaleDraw::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtDateScaleDraw)
public:
    PrivateData(QwtDateScaleDraw* p, Qt::TimeSpec spec)
        : q_ptr(p), timeSpec(spec), utcOffset(0), week0Type(QwtDate::FirstThursday)
    {
        // modify by qwt7.0 change to ISO datetime format
        /**
        dateFormats[ QwtDate::Millisecond ] = "hh:mm:ss:zzz\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Second ] = "hh:mm:ss\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Minute ] = "hh:mm\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Hour ] = "hh:mm\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Day ] = "ddd dd MMM yyyy";
        dateFormats[ QwtDate::Week ] = "Www yyyy";
        dateFormats[ QwtDate::Month ] = "MMM yyyy";
        dateFormats[ QwtDate::Year ] = "yyyy";
        change \n to <br/>
        **/
        dateFormats[ QwtDate::Millisecond ] = "yyyy-MM-dd<br/>hh:mm:ss:zzz";
        dateFormats[ QwtDate::Second ]      = "yyyy-MM-dd<br/>hh:mm:ss";
        dateFormats[ QwtDate::Minute ]      = "yyyy-MM-dd<br/>hh:mm:ss";
        dateFormats[ QwtDate::Hour ]        = "yyyy-MM-dd<br/>hh:mm:ss";
        dateFormats[ QwtDate::Day ]         = "yyyy-MM-dd<br/>hh:mm:ss";
        dateFormats[ QwtDate::Week ]        = "yyyy Www";
        dateFormats[ QwtDate::Month ]       = "yyyy-MM ";
        dateFormats[ QwtDate::Year ]        = "yyyy";
    }

    Qt::TimeSpec timeSpec { Qt::LocalTime };
    int utcOffset;
    QwtDate::Week0Type week0Type;
    QString dateFormats[ QwtDate::Year + 1 ];
};

/**
 * @brief Constructor
 * @details The default setting is to display tick labels for the
 *          given time specification. The first week of a year is defined like
 *          for QwtDate::FirstThursday.
 * @param[in] timeSpec Time specification
 * @sa setTimeSpec(), setWeek0Type()
 *
 */
QwtDateScaleDraw::QwtDateScaleDraw(Qt::TimeSpec timeSpec) : m_data(qwt_make_unique< PrivateData >(this, timeSpec))
{
}

/**
 * @brief Destructor
 *
 */
QwtDateScaleDraw::~QwtDateScaleDraw()
{
}

/**
 * @brief Set the time specification used for the tick labels
 * @param[in] timeSpec Time specification
 * @sa timeSpec(), setUtcOffset(), toDateTime()
 *
 */
void QwtDateScaleDraw::setTimeSpec(Qt::TimeSpec timeSpec)
{
    QWT_D(d);
    d->timeSpec = timeSpec;
}

/**
 * @brief Get the time specification used for the tick labels
 * @return Time specification used for the tick labels
 * @sa setTimeSpec(), utcOffset(), toDateTime()
 *
 */
Qt::TimeSpec QwtDateScaleDraw::timeSpec() const
{
    QWT_DC(d);
    return d->timeSpec;
}

/**
 * @brief Set the offset in seconds from Coordinated Universal Time
 * @param[in] seconds Offset in seconds
 * @note The offset has no effect beside for the time specification Qt::OffsetFromUTC.
 * @sa utcOffset(), setTimeSpec(), toDateTime()
 *
 */
void QwtDateScaleDraw::setUtcOffset(int seconds)
{
    QWT_D(d);
    d->utcOffset = seconds;
}

/**
 * @brief Get the offset in seconds from Coordinated Universal Time
 * @return Offset in seconds from Coordinated Universal Time
 * @note The offset has no effect beside for the time specification Qt::OffsetFromUTC.
 * @sa setUtcOffset(), setTimeSpec(), toDateTime()
 *
 */
int QwtDateScaleDraw::utcOffset() const
{
    QWT_DC(d);
    return d->utcOffset;
}

/**
 * @brief Set how to identify the first week of a year
 * @param[in] week0Type Mode how to identify the first week of a year
 * @note week0Type has no effect beside for intervals classified as QwtDate::Week.
 * @sa week0Type()
 *
 */
void QwtDateScaleDraw::setWeek0Type(QwtDate::Week0Type week0Type)
{
    QWT_D(d);
    d->week0Type = week0Type;
}

/**
 * @brief Get how to identify the first week of a year
 * @return Setting how to identify the first week of a year
 * @sa setWeek0Type()
 *
 */
QwtDate::Week0Type QwtDateScaleDraw::week0Type() const
{
    QWT_DC(d);
    return d->week0Type;
}

/**
 * @brief Set the default format string for a datetime interval type
 * @param[in] intervalType Interval type
 * @param[in] format Default format string
 * @sa dateFormat(), dateFormatOfDate(), QwtDate::toString()
 *
 */
void QwtDateScaleDraw::setDateFormat(QwtDate::IntervalType intervalType, const QString& format)
{
    QWT_D(d);
    if (intervalType >= QwtDate::Millisecond && intervalType <= QwtDate::Year) {
        d->dateFormats[ intervalType ] = format;
    }
}

/**
 * @brief Get the default format string for a datetime interval type
 * @param[in] intervalType Interval type
 * @return Default format string for an datetime interval type
 * @sa setDateFormat(), dateFormatOfDate()
 *
 */
QString QwtDateScaleDraw::dateFormat(QwtDate::IntervalType intervalType) const
{
    QWT_DC(d);
    if (intervalType >= QwtDate::Millisecond && intervalType <= QwtDate::Year) {
        return d->dateFormats[ intervalType ];
    }

    return QString();
}

/*!
   Format string for the representation of a datetime

   dateFormatOfDate() is intended to be overloaded for
   situations, where formats are individual for specific
   datetime values.

   The default setting ignores dateTime and return
   the default format for the interval type.

   @param dateTime Datetime value
   @param intervalType Interval type
   @return Format string

   @sa setDateFormat(), QwtDate::toString()
 */
QString QwtDateScaleDraw::dateFormatOfDate(const QDateTime& dateTime, QwtDate::IntervalType intervalType) const
{
    QWT_DC(d);
    Q_UNUSED(dateTime)

    if (intervalType >= QwtDate::Millisecond && intervalType <= QwtDate::Year) {
        return d->dateFormats[ intervalType ];
    }

    return d->dateFormats[ QwtDate::Second ];
}

/**
 * @brief Convert a value into its representing label
 * @details The value is converted to a datetime value using toDateTime()
 *          and converted to a plain text using QwtDate::toString().
 * @param[in] value Value
 * @return Label string
 * @sa dateFormatOfDate()
 *
 */
QwtText QwtDateScaleDraw::label(double value) const
{
    QWT_DC(d);
    const QDateTime dt = toDateTime(value);
    const QString fmt  = dateFormatOfDate(dt, intervalType(scaleDiv()));

    return QwtDate::toString(dt, fmt, d->week0Type);
}

/*!
   Find the less detailed datetime unit, where no rounding
   errors happen.

   @param scaleDiv Scale division
   @return Interval type

   @sa dateFormatOfDate()
 */
QwtDate::IntervalType QwtDateScaleDraw::intervalType(const QwtScaleDiv& scaleDiv) const
{
    int intvType = QwtDate::Year;

    bool alignedToWeeks = true;

    const QList< double > ticks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
    for (int i = 0; i < ticks.size(); i++) {
        const QDateTime dt = toDateTime(ticks[ i ]);
        for (int j = QwtDate::Second; j <= intvType; j++) {
            const QDateTime dt0 = QwtDate::floor(dt, static_cast< QwtDate::IntervalType >(j));

            if (dt0 != dt) {
                if (j == QwtDate::Week) {
                    alignedToWeeks = false;
                } else {
                    intvType = j - 1;
                    break;
                }
            }
        }

        if (intvType == QwtDate::Millisecond)
            break;
    }

    if (intvType == QwtDate::Week && !alignedToWeeks)
        intvType = QwtDate::Day;

    return static_cast< QwtDate::IntervalType >(intvType);
}

/**
 * @brief Translate a double value into a QDateTime object
 * @return QDateTime object initialized with timeSpec() and utcOffset()
 * @sa timeSpec(), utcOffset(), QwtDate::toDateTime()
 *
 */
QDateTime QwtDateScaleDraw::toDateTime(double value) const
{
    QWT_DC(d);
    QDateTime dt = QwtDate::toDateTime(value, d->timeSpec);
    if (d->timeSpec == Qt::OffsetFromUTC) {
        dt = dt.addSecs(d->utcOffset);
#if QT_VERSION >= 0x050200
        dt.setOffsetFromUtc(d->utcOffset);
#else
        dt.setUtcOffset(d->utcOffset);
#endif
    }

    return dt;
}
