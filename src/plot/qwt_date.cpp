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

#include "qwt_date.h"
#include "qwt_math.h"

#include <qdebug.h>
#include <qlocale.h>

#include <limits>

#if QT_VERSION >= 0x050000

using QwtJulianDay                              = qint64;
static const QwtJulianDay cs_date_minJulianDayD = Q_INT64_C(-784350574879);
static const QwtJulianDay cs_date_maxJulianDayD = Q_INT64_C(784354017364);

#else

// QDate stores the Julian day as unsigned int, but
// there is QDate::fromJulianDay( int ). That's why
// we have the range [ 1, INT_MAX ]

using QwtJulianDay                              = int;
static const QwtJulianDay cs_date_minJulianDayD = 1;
static const QwtJulianDay cs_date_maxJulianDayD = std::numeric_limits< int >::max();

#endif

static QString qwtExpandedFormat(const QString& format, const QDateTime& dateTime, QwtDate::Week0Type week0Type)
{
    const int week = QwtDate::weekNumber(dateTime.date(), week0Type);

    QString weekNo;
    weekNo.setNum(week);

    QString weekNoWW;
    if (weekNo.length() == 1)
        weekNoWW += QLatin1Char('0');

    weekNoWW += weekNo;

    QString fmt = format;
    fmt.replace(QLatin1String("ww"), weekNoWW);
    fmt.replace(QLatin1Char('w'), weekNo);

    if (week == 1 && dateTime.date().month() != 1) {
        // in case of week 1, we might need to increment the year

        QLatin1String s_yyyy("yyyy");
        QLatin1String s_yy("yy");

        // week 1 might start in the previous year

        bool doReplaceYear = fmt.contains(s_yy);

        if (doReplaceYear) {
            if (fmt.contains('M')) {
                // in case of also having 'M' we have a conflict about
                // which year to show

                doReplaceYear = false;
            } else {
                // in case of also having 'd' or 'dd' we have a conflict about
                // which year to show

                int numD = 0;

                for (int i = 0; i < fmt.size(); i++) {
                    if (fmt[ i ] == 'd') {
                        numD++;
                    } else {
                        if (numD > 0 && numD <= 2)
                            break;

                        numD = 0;
                    }
                }

                if (numD > 0 && numD <= 2)
                    doReplaceYear = false;
            }
        }

        if (doReplaceYear) {
            const QDate dt(dateTime.date().year() + 1, 1, 1);
            const QString dtString = QLocale().toString(dt, s_yyyy);

            if (fmt.contains(s_yyyy)) {
                fmt.replace(s_yyyy, dtString);
            } else {
                fmt.replace(s_yy, dtString);
            }
        }
    }

    return fmt;
}

static inline Qt::DayOfWeek qwtFirstDayOfWeek()
{
    return QLocale().firstDayOfWeek();
}

static inline void qwtFloorTime(QwtDate::IntervalType intervalType, QDateTime& dt)
{
    // when dt is inside the special hour where DST is ending
    // an hour is no unique. Therefore we have to
    // use UTC time.

    const Qt::TimeSpec timeSpec = dt.timeSpec();

    if (timeSpec == Qt::LocalTime)
        dt = dt.toTimeSpec(Qt::UTC);

    const QTime t = dt.time();
    switch (intervalType) {
    case QwtDate::Second: {
        dt.setTime(QTime(t.hour(), t.minute(), t.second()));
        break;
    }
    case QwtDate::Minute: {
        dt.setTime(QTime(t.hour(), t.minute(), 0));
        break;
    }
    case QwtDate::Hour: {
        dt.setTime(QTime(t.hour(), 0, 0));
        break;
    }
    default:
        break;
    }

    if (timeSpec == Qt::LocalTime)
        dt = dt.toTimeSpec(Qt::LocalTime);
}

static inline QDateTime qwtToTimeSpec(const QDateTime& dt, Qt::TimeSpec spec)
{
    if (dt.timeSpec() == spec)
        return dt;

    const qint64 jd = dt.date().toJulianDay();
    if (jd < 0 || jd >= std::numeric_limits< int >::max()) {
        // the conversion between local time and UTC
        // is internally limited. To avoid
        // overflows we simply ignore the difference
        // for those dates

        QDateTime dt2 = dt;
        dt2.setTimeSpec(spec);
        return dt2;
    }

    return dt.toTimeSpec(spec);
}

#if 0

static inline double qwtToJulianDay( int year, int month, int day )
{
    // code from QDate but using doubles to avoid overflows
    // for large values

    const int m1 = ( month - 14 ) / 12;
    const int m2 = ( 367 * ( month - 2 - 12 * m1 ) ) / 12;
    const double y1 = std::floor( ( 4900.0 + year + m1 ) / 100 );

    return std::floor( ( 1461.0 * ( year + 4800 + m1 ) ) / 4 ) + m2
           - std::floor( ( 3 * y1 ) / 4 ) + day - 32075;
}

static inline qint64 qwtFloorDiv64( qint64 a, int b )
{
    if ( a < 0 )
        a -= b - 1;

    return a / b;
}

static inline qint64 qwtFloorDiv( int a, int b )
{
    if ( a < 0 )
        a -= b - 1;

    return a / b;
}

#endif

static inline QDate qwtToDate(int year, int month = 1, int day = 1)
{
#if QT_VERSION >= 0x050000
    return QDate(year, month, day);
#else
    if (year > 100000) {
        // code from QDate but using doubles to avoid overflows
        // for large values

        const int m1    = (month - 14) / 12;
        const int m2    = (367 * (month - 2 - 12 * m1)) / 12;
        const double y1 = std::floor((4900.0 + year + m1) / 100);

        const double jd = std::floor((1461.0 * (year + 4800 + m1)) / 4) + m2 - std::floor((3 * y1) / 4) + day - 32075;

        if (jd > cs_date_maxJulianDayD) {
            qWarning() << "qwtToDate: overflow";
            return QDate();
        }

        return QDate::fromJulianDay(static_cast< QwtJulianDay >(jd));
    } else {
        return QDate(year, month, day);
    }
#endif
}

/**
 * \if ENGLISH
 * @brief Translate from double to QDateTime
 *
 * @param value Number of milliseconds since the epoch,
 *              1970-01-01T00:00:00 UTC
 * @param timeSpec Time specification
 * @return Datetime value
 *
 * @sa toDouble(), QDateTime::setMSecsSinceEpoch()
 * @note The return datetime for Qt::OffsetFromUTC will be Qt::UTC
 * \endif
 *
 * \if CHINESE
 * @brief 将 double 转换为 QDateTime
 *
 * @param value 自纪元 1970-01-01T00:00:00 UTC 以来的毫秒数
 * @param timeSpec 时间规范
 * @return 日期时间值
 *
 * @sa toDouble(), QDateTime::setMSecsSinceEpoch()
 * @note Qt::OffsetFromUTC 的返回日期时间将是 Qt::UTC
 * \endif
 */
QDateTime QwtDate::toDateTime(double value, Qt::TimeSpec timeSpec)
{
    const int msecsPerDay = 86400000;

    const double days = static_cast< qint64 >(std::floor(value / msecsPerDay));

    const double jd = QwtDate::JulianDayForEpoch + days;
    if ((jd > cs_date_maxJulianDayD) || (jd < cs_date_minJulianDayD)) {
        qWarning() << "QwtDate::toDateTime: overflow";
        return QDateTime();
    }

    const QDate d = QDate::fromJulianDay(static_cast< QwtJulianDay >(jd));

    const int msecs = static_cast< int >(value - days * msecsPerDay);

    static const QTime timeNull(0, 0, 0, 0);

    QDateTime dt(d, timeNull.addMSecs(msecs), Qt::UTC);

    if (timeSpec == Qt::LocalTime)
        dt = qwtToTimeSpec(dt, timeSpec);

    return dt;
}

/**
 * \if ENGLISH
 * @brief Translate from QDateTime to double
 *
 * @param dateTime Datetime value
 * @return Number of milliseconds since 1970-01-01T00:00:00 UTC has passed.
 *
 * @sa toDateTime(), QDateTime::toMSecsSinceEpoch()
 * @warning For values very far below or above 1970-01-01 UTC rounding errors
 *          will happen due to the limited significance of a double.
 * \endif
 *
 * \if CHINESE
 * @brief 将 QDateTime 转换为 double
 *
 * @param dateTime 日期时间值
 * @return 自 1970-01-01T00:00:00 UTC 以来经过的毫秒数。
 *
 * @sa toDateTime(), QDateTime::toMSecsSinceEpoch()
 * @warning 对于远低于或远高于 1970-01-01 UTC 的值，由于双精度浮点数的有限精度，
 *          会出现舍入误差。
 * \endif
 */
double QwtDate::toDouble(const QDateTime& dateTime)
{
    const int msecsPerDay = 86400000;

    const QDateTime dt = qwtToTimeSpec(dateTime, Qt::UTC);

    const double days = dt.date().toJulianDay() - QwtDate::JulianDayForEpoch;

    const QTime time  = dt.time();
    const double secs = 3600.0 * time.hour() + 60.0 * time.minute() + time.second();

    return days * msecsPerDay + time.msec() + 1000.0 * secs;
}

/**
 * \if ENGLISH
 * @brief Ceil a datetime according the interval type
 *
 * @param dateTime Datetime value
 * @param intervalType Interval type, how to ceil.
 *                     F.e. when intervalType = QwtDate::Months, the result
 *                     will be ceiled to the next beginning of a month
 * @return Ceiled datetime
 * @sa floor()
 * \endif
 *
 * \if CHINESE
 * @brief 根据间隔类型向上取整日期时间
 *
 * @param dateTime 日期时间值
 * @param intervalType 间隔类型，如何向上取整。
 *                     例如，当 intervalType = QwtDate::Months 时，
 *                     结果将向上取整到下个月的开始
 * @return 向上取整后的日期时间
 * @sa floor()
 * \endif
 */
QDateTime QwtDate::ceil(const QDateTime& dateTime, IntervalType intervalType)
{
    if (dateTime.date() >= QwtDate::maxDate())
        return dateTime;

    QDateTime dt = dateTime;

    switch (intervalType) {
    case QwtDate::Millisecond: {
        break;
    }
    case QwtDate::Second: {
        qwtFloorTime(QwtDate::Second, dt);
        if (dt < dateTime)
            dt = dt.addSecs(1);

        break;
    }
    case QwtDate::Minute: {
        qwtFloorTime(QwtDate::Minute, dt);
        if (dt < dateTime)
            dt = dt.addSecs(60);

        break;
    }
    case QwtDate::Hour: {
        qwtFloorTime(QwtDate::Hour, dt);
        if (dt < dateTime)
            dt = dt.addSecs(3600);

        break;
    }
    case QwtDate::Day: {
        dt.setTime(QTime(0, 0));
        if (dt < dateTime)
            dt = dt.addDays(1);

        break;
    }
    case QwtDate::Week: {
        dt.setTime(QTime(0, 0));
        if (dt < dateTime)
            dt = dt.addDays(1);

        int days = qwtFirstDayOfWeek() - dt.date().dayOfWeek();
        if (days < 0)
            days += 7;

        dt = dt.addDays(days);

        break;
    }
    case QwtDate::Month: {
        dt.setTime(QTime(0, 0));
        dt.setDate(qwtToDate(dateTime.date().year(), dateTime.date().month()));

        if (dt < dateTime)
            dt = dt.addMonths(1);

        break;
    }
    case QwtDate::Year: {
        dt.setTime(QTime(0, 0));

        const QDate d = dateTime.date();

        int year = d.year();
        if (d.month() > 1 || d.day() > 1 || !dateTime.time().isNull())
            year++;

        if (year == 0)
            year++;  // there is no year 0

        dt.setDate(qwtToDate(year));
        break;
    }
    }

    return dt;
}

/**
 * \if ENGLISH
 * @brief Floor a datetime according the interval type
 *
 * @param dateTime Datetime value
 * @param intervalType Interval type, how to floor.
 *                     F.e. when intervalType = QwtDate::Months,
 *                     the result will be floored to the beginning of a month
 * @return Floored datetime
 * @sa ceil()
 * \endif
 *
 * \if CHINESE
 * @brief 根据间隔类型向下取整日期时间
 *
 * @param dateTime 日期时间值
 * @param intervalType 间隔类型，如何向下取整。
 *                     例如，当 intervalType = QwtDate::Months 时，
 *                     结果将向下取整到当月的开始
 * @return 向下取整后的日期时间
 * @sa ceil()
 * \endif
 */
QDateTime QwtDate::floor(const QDateTime& dateTime, IntervalType intervalType)
{
    if (dateTime.date() <= QwtDate::minDate())
        return dateTime;

    QDateTime dt = dateTime;

    switch (intervalType) {
    case QwtDate::Millisecond: {
        break;
    }
    case QwtDate::Second:
    case QwtDate::Minute:
    case QwtDate::Hour: {
        qwtFloorTime(intervalType, dt);
        break;
    }
    case QwtDate::Day: {
        dt.setTime(QTime(0, 0));
        break;
    }
    case QwtDate::Week: {
        dt.setTime(QTime(0, 0));

        int days = dt.date().dayOfWeek() - qwtFirstDayOfWeek();
        if (days < 0)
            days += 7;

        dt = dt.addDays(-days);

        break;
    }
    case QwtDate::Month: {
        dt.setTime(QTime(0, 0));

        const QDate date = qwtToDate(dt.date().year(), dt.date().month());
        dt.setDate(date);

        break;
    }
    case QwtDate::Year: {
        dt.setTime(QTime(0, 0));

        const QDate date = qwtToDate(dt.date().year());
        dt.setDate(date);

        break;
    }
    }

    return dt;
}

/**
 * \if ENGLISH
 * @brief Minimum for the supported date range
 *
 * The range of valid dates depends on how QDate stores the
 * Julian day internally.
 *
 * - For Qt4 it is "Tue Jan 2 -4713"
 * - For Qt5 it is "Thu Jan 1 -2147483648"
 *
 * @return minimum of the date range
 * @sa maxDate()
 * \endif
 *
 * \if CHINESE
 * @brief 支持的日期范围的最小值
 *
 * 有效日期的范围取决于 QDate 内部存储儒略日的方式。
 *
 * - 对于 Qt4 是 "Tue Jan 2 -4713"
 * - 对于 Qt5 是 "Thu Jan 1 -2147483648"
 *
 * @return 日期范围的最小值
 * @sa maxDate()
 * \endif
 */
QDate QwtDate::minDate()
{
    static QDate date;
    if (!date.isValid())
        date = QDate::fromJulianDay(cs_date_minJulianDayD);

    return date;
}

/**
 * \if ENGLISH
 * @brief Maximum for the supported date range
 *
 * The range of valid dates depends on how QDate stores the
 * Julian day internally.
 *
 * - For Qt4 it is "Tue Jun 3 5874898"
 * - For Qt5 it is "Tue Dec 31 2147483647"
 *
 * @return maximum of the date range
 * @sa minDate()
 * @note The maximum differs between Qt4 and Qt5
 * \endif
 *
 * \if CHINESE
 * @brief 支持的日期范围的最大值
 *
 * 有效日期的范围取决于 QDate 内部存储儒略日的方式。
 *
 * - 对于 Qt4 是 "Tue Jun 3 5874898"
 * - 对于 Qt5 是 "Tue Dec 31 2147483647"
 *
 * @return 日期范围的最大值
 * @sa minDate()
 * @note 最大值在 Qt4 和 Qt5 之间有所不同
 * \endif
 */
QDate QwtDate::maxDate()
{
    static QDate date;
    if (!date.isValid())
        date = QDate::fromJulianDay(cs_date_maxJulianDayD);

    return date;
}

/**
 * \if ENGLISH
 * @brief Date of the first day of the first week for a year
 *
 * The first day of a week depends on the current locale
 * ( QLocale::firstDayOfWeek() ).
 *
 * @param year Year
 * @param type Option how to identify the first week
 * @return First day of week 0
 *
 * @sa QLocale::firstDayOfWeek(), weekNumber()
 * \endif
 *
 * \if CHINESE
 * @brief 一年中第一周第一天的日期
 *
 * 一周的第一天取决于当前区域设置 ( QLocale::firstDayOfWeek() )。
 *
 * @param year 年份
 * @param type 如何确定第一周的选项
 * @return 第0周的第一天
 *
 * @sa QLocale::firstDayOfWeek(), weekNumber()
 * \endif
 */
QDate QwtDate::dateOfWeek0(int year, Week0Type type)
{
    const Qt::DayOfWeek firstDayOfWeek = qwtFirstDayOfWeek();

    QDate dt0(year, 1, 1);

    // floor to the first day of the week
    int days = dt0.dayOfWeek() - firstDayOfWeek;
    if (days < 0)
        days += 7;

    dt0 = dt0.addDays(-days);

    if (type == QwtDate::FirstThursday) {
        // according to ISO 8601 the first week is defined
        // by the first Thursday.

        int d = Qt::Thursday - firstDayOfWeek;
        if (d < 0)
            d += 7;

        if (dt0.addDays(d).year() < year)
            dt0 = dt0.addDays(7);
    }

    return dt0;
}

/**
 * \if ENGLISH
 * @brief Find the week number of a date
 *
 * - QwtDate::FirstThursday: Corresponding to ISO 8601 ( see QDate::weekNumber() ).
 * - QwtDate::FirstDay: Number of weeks that have begun since dateOfWeek0().
 *
 * @param date Date
 * @param type Option how to identify the first week
 *
 * @return Week number, starting with 1
 * \endif
 *
 * \if CHINESE
 * @brief 查找日期的周数
 *
 * - QwtDate::FirstThursday: 对应于 ISO 8601 (参见 QDate::weekNumber() )。
 * - QwtDate::FirstDay: 自 dateOfWeek0() 以来开始的周数。
 *
 * @param date 日期
 * @param type 如何确定第一周的选项
 *
 * @return 周数，从1开始
 * \endif
 */
int QwtDate::weekNumber(const QDate& date, Week0Type type)
{
    int weekNo;

    if (type == QwtDate::FirstDay) {
        QDate day0;

        if (date.month() == 12 && date.day() >= 24) {
            // week 1 usually starts in the previous years.
            // and we have to check if we are already there

            day0 = dateOfWeek0(date.year() + 1, type);
            if (day0.daysTo(date) < 0)
                day0 = dateOfWeek0(date.year(), type);
        } else {
            day0 = dateOfWeek0(date.year(), type);
        }

        weekNo = day0.daysTo(date) / 7 + 1;
    } else {
        weekNo = date.weekNumber();
    }

    return weekNo;
}

/**
 * \if ENGLISH
 * @brief Offset in seconds from Coordinated Universal Time
 *
 * The offset depends on the time specification of dateTime:
 *
 * - Qt::UTC: 0, dateTime has no offset
 * - Qt::OffsetFromUTC: returns dateTime.offsetFromUtc()
 * - Qt::LocalTime: number of seconds from the UTC
 *
 * For Qt::LocalTime the offset depends on the timezone and
 * daylight savings.
 *
 * @param dateTime Datetime value
 * @return Offset in seconds
 * \endif
 *
 * \if CHINESE
 * @brief 与世界协调时间的偏移量（秒）
 *
 * 偏移量取决于 dateTime 的时间规范：
 *
 * - Qt::UTC: 0，dateTime 没有偏移
 * - Qt::OffsetFromUTC: 返回 dateTime.offsetFromUtc()
 * - Qt::LocalTime: 与 UTC 的秒数差
 *
 * 对于 Qt::LocalTime，偏移量取决于时区和夏令时。
 *
 * @param dateTime 日期时间值
 * @return 偏移量（秒）
 * \endif
 */
int QwtDate::utcOffset(const QDateTime& dateTime)
{
    int seconds = 0;

    switch (dateTime.timeSpec()) {
    case Qt::UTC: {
        break;
    }
    case Qt::OffsetFromUTC: {
#if QT_VERSION >= 0x050200
        seconds = dateTime.offsetFromUtc();
#else
        seconds = dateTime.utcOffset();
#endif
        break;
    }
    default: {
        const QDateTime dt1(dateTime.date(), dateTime.time(), Qt::UTC);
        seconds = dateTime.secsTo(dt1);
    }
    }

    return seconds;
}

/**
 * \if ENGLISH
 * @brief Translate a datetime into a string
 *
 * Beside the format expressions documented in QDateTime::toString()
 * the following expressions are supported:
 *
 * - w: week number ( 1 - 53 )
 * - ww: week number with a leading zero ( 01 - 53 )
 *
 * As week 1 usually starts in the previous year a special rule
 * is applied for formats, where the year is expected to match the
 * week number - even if the date belongs to the previous year.
 *
 * @param dateTime Datetime value
 * @param format Format string
 * @param week0Type Specification of week 0
 *
 * @return Datetime string
 * @sa QDateTime::toString(), weekNumber(), QwtDateScaleDraw
 * \endif
 *
 * \if CHINESE
 * @brief 将日期时间转换为字符串
 *
 * 除了 QDateTime::toString() 中记录的格式表达式外，还支持以下表达式：
 *
 * - w: 周数 ( 1 - 53 )
 * - ww: 带前导零的周数 ( 01 - 53 )
 *
 * 由于第1周通常从前一年开始，因此对于期望年份与周数匹配的格式，
 * 会应用特殊规则 - 即使日期属于前一年。
 *
 * @param dateTime 日期时间值
 * @param format 格式字符串
 * @param week0Type 第0周的规范
 *
 * @return 日期时间字符串
 * @sa QDateTime::toString(), weekNumber(), QwtDateScaleDraw
 * \endif
 */
QString QwtDate::toString(const QDateTime& dateTime, const QString& format, Week0Type week0Type)
{
    QString fmt = format;
    if (fmt.contains('w')) {
        fmt = qwtExpandedFormat(fmt, dateTime, week0Type);
    }

    return QLocale().toString(dateTime, fmt);
}
