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
public:
    explicit PrivateData(Qt::TimeSpec spec) : timeSpec(spec), utcOffset(0), week0Type(QwtDate::FirstThursday)
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
 * \if ENGLISH
 * @brief Constructor
 * @details The default setting is to display tick labels for the
 *          given time specification. The first week of a year is defined like
 *          for QwtDate::FirstThursday.
 * @param[in] timeSpec Time specification
 * @sa setTimeSpec(), setWeek0Type()
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 默认设置为显示给定时间规范的刻度标签。
 *          一年的第一周按照 QwtDate::FirstThursday 定义。
 * @param[in] timeSpec 时间规范
 * @sa setTimeSpec(), setWeek0Type()
 * \endif
 */
QwtDateScaleDraw::QwtDateScaleDraw(Qt::TimeSpec timeSpec)
{
    m_data = new PrivateData(timeSpec);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtDateScaleDraw::~QwtDateScaleDraw()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the time specification used for the tick labels
 * @param[in] timeSpec Time specification
 * @sa timeSpec(), setUtcOffset(), toDateTime()
 * \endif
 *
 * \if CHINESE
 * @brief 设置用于刻度标签的时间规范
 * @param[in] timeSpec 时间规范
 * @sa timeSpec(), setUtcOffset(), toDateTime()
 * \endif
 */
void QwtDateScaleDraw::setTimeSpec(Qt::TimeSpec timeSpec)
{
    m_data->timeSpec = timeSpec;
}

/**
 * \if ENGLISH
 * @brief Get the time specification used for the tick labels
 * @return Time specification used for the tick labels
 * @sa setTimeSpec(), utcOffset(), toDateTime()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于刻度标签的时间规范
 * @return 用于刻度标签的时间规范
 * @sa setTimeSpec(), utcOffset(), toDateTime()
 * \endif
 */
Qt::TimeSpec QwtDateScaleDraw::timeSpec() const
{
    return m_data->timeSpec;
}

/**
 * \if ENGLISH
 * @brief Set the offset in seconds from Coordinated Universal Time
 * @param[in] seconds Offset in seconds
 * @note The offset has no effect beside for the time specification Qt::OffsetFromUTC.
 * @sa utcOffset(), setTimeSpec(), toDateTime()
 * \endif
 *
 * \if CHINESE
 * @brief 设置与世界协调时间的偏移量（秒）
 * @param[in] seconds 偏移量（秒）
 * @note 该偏移量仅对 Qt::OffsetFromUTC 时间规范有效。
 * @sa utcOffset(), setTimeSpec(), toDateTime()
 * \endif
 */
void QwtDateScaleDraw::setUtcOffset(int seconds)
{
    m_data->utcOffset = seconds;
}

/**
 * \if ENGLISH
 * @brief Get the offset in seconds from Coordinated Universal Time
 * @return Offset in seconds from Coordinated Universal Time
 * @note The offset has no effect beside for the time specification Qt::OffsetFromUTC.
 * @sa setUtcOffset(), setTimeSpec(), toDateTime()
 * \endif
 *
 * \if CHINESE
 * @brief 获取与世界协调时间的偏移量（秒）
 * @return 与世界协调时间的偏移量（秒）
 * @note 该偏移量仅对 Qt::OffsetFromUTC 时间规范有效。
 * @sa setUtcOffset(), setTimeSpec(), toDateTime()
 * \endif
 */
int QwtDateScaleDraw::utcOffset() const
{
    return m_data->utcOffset;
}

/**
 * \if ENGLISH
 * @brief Set how to identify the first week of a year
 * @param[in] week0Type Mode how to identify the first week of a year
 * @note week0Type has no effect beside for intervals classified as QwtDate::Week.
 * @sa week0Type()
 * \endif
 *
 * \if CHINESE
 * @brief 设置如何确定一年的第一周
 * @param[in] week0Type 确定一年第一周的模式
 * @note week0Type 仅对被分类为 QwtDate::Week 的间隔有效。
 * @sa week0Type()
 * \endif
 */
void QwtDateScaleDraw::setWeek0Type(QwtDate::Week0Type week0Type)
{
    m_data->week0Type = week0Type;
}

/**
 * \if ENGLISH
 * @brief Get how to identify the first week of a year
 * @return Setting how to identify the first week of a year
 * @sa setWeek0Type()
 * \endif
 *
 * \if CHINESE
 * @brief 获取如何确定一年的第一周
 * @return 确定一年第一周的设置
 * @sa setWeek0Type()
 * \endif
 */
QwtDate::Week0Type QwtDateScaleDraw::week0Type() const
{
    return m_data->week0Type;
}

/**
 * \if ENGLISH
 * @brief Set the default format string for a datetime interval type
 * @param[in] intervalType Interval type
 * @param[in] format Default format string
 * @sa dateFormat(), dateFormatOfDate(), QwtDate::toString()
 * \endif
 *
 * \if CHINESE
 * @brief 设置日期时间间隔类型的默认格式字符串
 * @param[in] intervalType 间隔类型
 * @param[in] format 默认格式字符串
 * @sa dateFormat(), dateFormatOfDate(), QwtDate::toString()
 * \endif
 */
void QwtDateScaleDraw::setDateFormat(QwtDate::IntervalType intervalType, const QString& format)
{
    if (intervalType >= QwtDate::Millisecond && intervalType <= QwtDate::Year) {
        m_data->dateFormats[ intervalType ] = format;
    }
}

/**
 * \if ENGLISH
 * @brief Get the default format string for a datetime interval type
 * @param[in] intervalType Interval type
 * @return Default format string for an datetime interval type
 * @sa setDateFormat(), dateFormatOfDate()
 * \endif
 *
 * \if CHINESE
 * @brief 获取日期时间间隔类型的默认格式字符串
 * @param[in] intervalType 间隔类型
 * @return 日期时间间隔类型的默认格式字符串
 * @sa setDateFormat(), dateFormatOfDate()
 * \endif
 */
QString QwtDateScaleDraw::dateFormat(QwtDate::IntervalType intervalType) const
{
    if (intervalType >= QwtDate::Millisecond && intervalType <= QwtDate::Year) {
        return m_data->dateFormats[ intervalType ];
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

   \param dateTime Datetime value
   \param intervalType Interval type
   \return Format string

   \sa setDateFormat(), QwtDate::toString()
 */
QString QwtDateScaleDraw::dateFormatOfDate(const QDateTime& dateTime, QwtDate::IntervalType intervalType) const
{
    Q_UNUSED(dateTime)

    if (intervalType >= QwtDate::Millisecond && intervalType <= QwtDate::Year) {
        return m_data->dateFormats[ intervalType ];
    }

    return m_data->dateFormats[ QwtDate::Second ];
}

/**
 * \if ENGLISH
 * @brief Convert a value into its representing label
 * @details The value is converted to a datetime value using toDateTime()
 *          and converted to a plain text using QwtDate::toString().
 * @param[in] value Value
 * @return Label string
 * @sa dateFormatOfDate()
 * \endif
 *
 * \if CHINESE
 * @brief 将值转换为其表示的标签
 * @details 值通过 toDateTime() 转换为日期时间值，
 *          并通过 QwtDate::toString() 转换为纯文本。
 * @param[in] value 值
 * @return 标签字符串
 * @sa dateFormatOfDate()
 * \endif
 */
QwtText QwtDateScaleDraw::label(double value) const
{
    const QDateTime dt = toDateTime(value);
    const QString fmt  = dateFormatOfDate(dt, intervalType(scaleDiv()));

    return QwtDate::toString(dt, fmt, m_data->week0Type);
}

/*!
   Find the less detailed datetime unit, where no rounding
   errors happen.

   \param scaleDiv Scale division
   \return Interval type

   \sa dateFormatOfDate()
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
 * \if ENGLISH
 * @brief Translate a double value into a QDateTime object
 * @return QDateTime object initialized with timeSpec() and utcOffset()
 * @sa timeSpec(), utcOffset(), QwtDate::toDateTime()
 * \endif
 *
 * \if CHINESE
 * @brief 将 double 值转换为 QDateTime 对象
 * @return 使用 timeSpec() 和 utcOffset() 初始化的 QDateTime 对象
 * @sa timeSpec(), utcOffset(), QwtDate::toDateTime()
 * \endif
 */
QDateTime QwtDateScaleDraw::toDateTime(double value) const
{
    QDateTime dt = QwtDate::toDateTime(value, m_data->timeSpec);
    if (m_data->timeSpec == Qt::OffsetFromUTC) {
        dt = dt.addSecs(m_data->utcOffset);
#if QT_VERSION >= 0x050200
        dt.setOffsetFromUtc(m_data->utcOffset);
#else
        dt.setUtcOffset(m_data->utcOffset);
#endif
    }

    return dt;
}
