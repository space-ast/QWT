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

#ifndef QWT_DATE_H
#define QWT_DATE_H

#include "qwt_global.h"
#include <qdatetime.h>

/**
 * \if ENGLISH
 * @brief A collection of methods around date/time values
 *
 * Qt offers convenient classes for dealing with date/time values,
 * but Qwt uses coordinate systems that are based on doubles.
 * QwtDate offers methods to translate from QDateTime to double and v.v.
 *
 * A double is interpreted as the number of milliseconds since
 * 1970-01-01T00:00:00 Universal Coordinated Time - also known
 * as "The Epoch".
 *
 * While the range of the Julian day in Qt4 is limited to [0, MAX_INT],
 * Qt5 stores it as qint64 offering a huge range of valid dates.
 * As the significance of a double is below this ( assuming a
 * fraction of 52 bits ) the translation is not
 * bijective with rounding errors for dates very far from Epoch.
 * For a resolution of 1 ms those start to happen for dates above the
 * year 144683.
 *
 * An axis for a date/time interval is expected to be aligned
 * and divided in time/date units like seconds, minutes, ...
 * QwtDate offers several algorithms that are needed to
 * calculate these axes.
 *
 * @sa QwtDateScaleEngine, QwtDateScaleDraw, QDate, QTime
 * \endif
 *
 * \if CHINESE
 * @brief 日期/时间值相关方法的集合
 *
 * Qt 提供了处理日期/时间值的便捷类，但 Qwt 使用基于双精度浮点数的坐标系统。
 * QwtDate 提供了在 QDateTime 和 double 之间相互转换的方法。
 *
 * 双精度浮点数被解释为自 1970-01-01T00:00:00 世界协调时间（也称为"纪元"）以来的毫秒数。
 *
 * Qt4 中儒略日的范围限制为 [0, MAX_INT]，而 Qt5 将其存储为 qint64，提供了巨大的有效日期范围。
 * 由于双精度浮点数的有效位数低于此值（假设小数部分为 52 位），对于远离纪元的日期，
 * 转换不是双射的，会出现舍入误差。对于 1 毫秒的分辨率，这些误差从公元 144683 年开始出现。
 *
 * 日期/时间间隔的坐标轴需要按秒、分钟等时间/日期单位对齐和划分。
 * QwtDate 提供了计算这些坐标轴所需的几种算法。
 *
 * @sa QwtDateScaleEngine, QwtDateScaleDraw, QDate, QTime
 * \endif
 */
class QWT_EXPORT QwtDate
{
public:
    /**
     * \if ENGLISH
     * @brief How to identify the first week of year differs between countries
     * \endif
     * \if CHINESE
     * @brief 如何确定一年的第一周（不同国家标准不同）
     * \endif
     */
    enum Week0Type
    {
        /**
         * \if ENGLISH
         * According to ISO 8601 the first week of a year is defined
         * as "the week with the year's first Thursday in it".
         * FirstThursday corresponds to the numbering that is
         * implemented in QDate::weekNumber().
         * \endif
         * \if CHINESE
         * 根据 ISO 8601，一年的第一周定义为"包含该年第一个星期四的周"。
         * FirstThursday 对应于 QDate::weekNumber() 中实现的编号方式。
         * \endif
         */
        FirstThursday,

        /**
         * \if ENGLISH
         * "The week with January 1 in it."
         * In the U.S. this definition is more common than FirstThursday.
         * \endif
         * \if CHINESE
         * "包含1月1日的周"。
         * 在美国，这种定义比 FirstThursday 更常见。
         * \endif
         */
        FirstDay
    };

    /**
     * \if ENGLISH
     * @brief Classification of a time interval
     *
     * Time intervals need to be classified to decide how to
     * align and divide it.
     * \endif
     * \if CHINESE
     * @brief 时间间隔的分类
     *
     * 需要对时间间隔进行分类以决定如何对齐和划分它。
     * \endif
     */
    enum IntervalType
    {
        //! \if ENGLISH Millisecond \endif \if CHINESE 毫秒 \endif
        Millisecond,

        //! \if ENGLISH Second \endif \if CHINESE 秒 \endif
        Second,

        //! \if ENGLISH Minute \endif \if CHINESE 分钟 \endif
        Minute,

        //! \if ENGLISH Hour \endif \if CHINESE 小时 \endif
        Hour,

        //! \if ENGLISH Day \endif \if CHINESE 天 \endif
        Day,

        //! \if ENGLISH Week \endif \if CHINESE 周 \endif
        Week,

        //! \if ENGLISH Month \endif \if CHINESE 月 \endif
        Month,

        //! \if ENGLISH Year \endif \if CHINESE 年 \endif
        Year
    };

    enum
    {
        //! \if ENGLISH The Julian day of "The Epoch" \endif \if CHINESE "纪元"的儒略日 \endif
        JulianDayForEpoch = 2440588
    };

    // Get minimum date
    static QDate minDate();
    // Get maximum date
    static QDate maxDate();

    // Convert double to QDateTime
    static QDateTime toDateTime(double value, Qt::TimeSpec = Qt::UTC);

    // Convert QDateTime to double
    static double toDouble(const QDateTime&);

    // Ceil datetime to interval type
    static QDateTime ceil(const QDateTime&, IntervalType);
    // Floor datetime to interval type
    static QDateTime floor(const QDateTime&, IntervalType);

    // Get date of week 0
    static QDate dateOfWeek0(int year, Week0Type);
    // Get week number
    static int weekNumber(const QDate&, Week0Type);

    // Get UTC offset in seconds
    static int utcOffset(const QDateTime&);

    // Format datetime to string
    static QString toString(const QDateTime&, const QString& format, Week0Type);
};

#endif
