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

#ifndef QWT_SERIES_DATA_H
#define QWT_SERIES_DATA_H

#include "qwt_global.h"
#include "qwt_samples.h"
#include "qwt_point_3d.h"

#include <qvector.h>
#include <qrect.h>

class QwtPointPolar;

/**
 * \if ENGLISH
 * @brief Abstract interface for iterating over samples
 * @details Qwt offers several implementations of the QwtSeriesData API,
 *          but in situations, where data of an application specific format
 *          needs to be displayed, without having to copy it, it is recommended
 *          to implement an individual data access.
 *
 *          A subclass of QwtSeriesData<QPointF> must implement:
 *          - size(): Should return number of data points.
 *          - sample(): Should return values x and y values of the sample at specific position
 *            as QPointF object.
 *          - boundingRect(): Should return the bounding rectangle of the data series.
 *            It is used for autoscaling and might help certain algorithms for displaying
 *            the data. You can use qwtBoundingRect() for an implementation but often it is
 *            possible to implement a more efficient algorithm depending on the characteristics
 *            of the series. The member cachedBoundingRect is intended for caching the calculated rectangle.
 * \endif
 * \if CHINESE
 * @brief 遍历样本的抽象接口
 * @details Qwt 提供了 QwtSeriesData API 的几种实现，但在需要显示特定应用程序格式的数据
 *          而无需复制的情况下，建议实现单独的数据访问。
 *
 *          QwtSeriesData<QPointF> 的子类必须实现：
 *          - size(): 应返回数据点的数量。
 *          - sample(): 应返回特定位置的样本的 x 和 y 值作为 QPointF 对象。
 *          - boundingRect(): 应返回数据系列的边界矩形。它用于自动缩放，并可能有助于
 *            某些显示数据的算法。您可以使用 qwtBoundingRect() 来实现，但通常可以根据
 *            系列的特性实现更高效的算法。成员 cachedBoundingRect 用于缓存计算的矩形。
 * \endif
 */
template< typename T >
class QwtSeriesData
{
public:
    /// Constructor
    QwtSeriesData();

    /// Destructor
    virtual ~QwtSeriesData();

#ifndef QWT_PYTHON_WRAPPER

    /// \return Number of samples
    virtual size_t size() const = 0;

    /// Return a sample
    virtual T sample(size_t i) const = 0;

    /// Calculate the bounding rect of all samples
    virtual QRectF boundingRect() const = 0;

#else
    // Needed for generating the python bindings, but not for using them !
    virtual size_t size() const
    {
        return 0;
    }
    virtual T sample(size_t i) const
    {
        return T();
    }
    virtual QRectF boundingRect() const
    {
        return cachedBoundingRect;
    }
#endif

    /// Set a the "rect of interest"
    virtual void setRectOfInterest(const QRectF& rect);

protected:
    /// Can be used to cache a calculated bounding rectangle
    mutable QRectF cachedBoundingRect;

private:
    QwtSeriesData< T >& operator=(const QwtSeriesData< T >&);
};

template< typename T >
QwtSeriesData< T >::QwtSeriesData() : cachedBoundingRect(0.0, 0.0, -1.0, -1.0)
{
}

template< typename T >
QwtSeriesData< T >::~QwtSeriesData()
{
}

template< typename T >
void QwtSeriesData< T >::setRectOfInterest(const QRectF&)
{
}

/**
 * \if ENGLISH
 * @brief Template class for data, that is organized as QVector
 * @details QVector uses implicit data sharing and can be passed around as argument efficiently.
 * \endif
 * \if CHINESE
 * @brief 数据模板类，组织为 QVector
 * @details QVector 使用隐式数据共享，可以高效地作为参数传递。
 * \endif
 */
template< typename T >
class QwtArraySeriesData : public QwtSeriesData< T >
{
public:
    /// Constructor
    QwtArraySeriesData();

    /// Constructor
    explicit QwtArraySeriesData(const QVector< T >& samples);
    explicit QwtArraySeriesData(QVector< T >&& samples);

    /// Assign an array of samples
    void setSamples(const QVector< T >& samples);
    void setSamples(QVector< T >&& samples);

    /// \return Array of samples
    const QVector< T > samples() const;

    /// \return Number of samples
    virtual size_t size() const override;

    /// \return Sample at a specific position
    virtual T sample(size_t index) const override;

protected:
    /// Vector of samples
    QVector< T > m_samples;
};

template< typename T >
QwtArraySeriesData< T >::QwtArraySeriesData()
{
}

template< typename T >
QwtArraySeriesData< T >::QwtArraySeriesData(const QVector< T >& samples) : m_samples(samples)
{
}

template< typename T >
QwtArraySeriesData< T >::QwtArraySeriesData(QVector< T >&& samples)
{
    m_samples = std::move(samples);
}

template< typename T >
void QwtArraySeriesData< T >::setSamples(const QVector< T >& samples)
{
    QwtSeriesData< T >::cachedBoundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    m_samples                              = samples;
}

template< typename T >
void QwtArraySeriesData< T >::setSamples(QVector< T >&& samples)
{
    QwtSeriesData< T >::cachedBoundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    m_samples                              = std::move(samples);
}

template< typename T >
const QVector< T > QwtArraySeriesData< T >::samples() const
{
    return m_samples;
}

template< typename T >
size_t QwtArraySeriesData< T >::size() const
{
    return m_samples.size();
}

template< typename T >
T QwtArraySeriesData< T >::sample(size_t i) const
{
    return m_samples[ static_cast< int >(i) ];
}

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of points
 * @details QwtPointSeriesData provides access to QPointF samples stored in a QVector.
 * \endif
 * \if CHINESE
 * @brief 点数组迭代接口
 * @details QwtPointSeriesData 提供对存储在 QVector 中的 QPointF 样本的访问。
 * \endif
 */
class QWT_EXPORT QwtPointSeriesData : public QwtArraySeriesData< QPointF >
{
public:
    //! Constructor
    QwtPointSeriesData(const QVector< QPointF >& = QVector< QPointF >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of 3D points
 * @details QwtPoint3DSeriesData provides access to QwtPoint3D samples stored in a QVector.
 * \endif
 * \if CHINESE
 * @brief 3D点数组迭代接口
 * @details QwtPoint3DSeriesData 提供对存储在 QVector 中的 QwtPoint3D 样本的访问。
 * \endif
 */
class QWT_EXPORT QwtPoint3DSeriesData : public QwtArraySeriesData< QwtPoint3D >
{
public:
    //! Constructor
    QwtPoint3DSeriesData(const QVector< QwtPoint3D >& = QVector< QwtPoint3D >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of intervals
 * @details QwtIntervalSeriesData provides access to QwtIntervalSample samples stored in a QVector.
 * \endif
 * \if CHINESE
 * @brief 区间数组迭代接口
 * @details QwtIntervalSeriesData 提供对存储在 QVector 中的 QwtIntervalSample 样本的访问。
 * \endif
 */
class QWT_EXPORT QwtIntervalSeriesData : public QwtArraySeriesData< QwtIntervalSample >
{
public:
    //! Constructor
    QwtIntervalSeriesData(const QVector< QwtIntervalSample >& = QVector< QwtIntervalSample >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of set samples
 * @details QwtSetSeriesData provides access to QwtSetSample samples stored in a QVector.
 * \endif
 * \if CHINESE
 * @brief 集合样本数组迭代接口
 * @details QwtSetSeriesData 提供对存储在 QVector 中的 QwtSetSample 样本的访问。
 * \endif
 */
class QWT_EXPORT QwtSetSeriesData : public QwtArraySeriesData< QwtSetSample >
{
public:
    //! Constructor
    QwtSetSeriesData(const QVector< QwtSetSample >& = QVector< QwtSetSample >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of vector field samples
 * @details QwtVectorFieldData provides access to QwtVectorFieldSample samples stored in a QVector.
 * \endif
 * \if CHINESE
 * @brief 向量场样本数组迭代接口
 * @details QwtVectorFieldData 提供对存储在 QVector 中的 QwtVectorFieldSample 样本的访问。
 * \endif
 */
class QWT_EXPORT QwtVectorFieldData : public QwtArraySeriesData< QwtVectorFieldSample >
{
public:
    //! Constructor
    QwtVectorFieldData(const QVector< QwtVectorFieldSample >& = QVector< QwtVectorFieldSample >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of OHLC samples
 * @details QwtTradingChartData provides access to QwtOHLCSample samples stored in a QVector.
 *          Used for candlestick or OHLC chart financial data.
 * \endif
 * \if CHINESE
 * @brief OHLC样本数组迭代接口
 * @details QwtTradingChartData 提供对存储在 QVector 中的 QwtOHLCSample 样本的访问。
 *          用于蜡烛图或OHLC图表的金融数据。
 * \endif
 */
class QWT_EXPORT QwtTradingChartData : public QwtArraySeriesData< QwtOHLCSample >
{
public:
    //! Constructor
    QwtTradingChartData(const QVector< QwtOHLCSample >& = QVector< QwtOHLCSample >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of boxplot samples
 * @details QwtBoxChartData provides access to QwtBoxSample samples stored in a QVector.
 *          Used for box-and-whisker plot statistical data.
 * \endif
 * \if CHINESE
 * @brief 箱线图样本数组迭代接口
 * @details QwtBoxChartData 提供对存储在 QVector 中的 QwtBoxSample 样本的访问。
 *          用于箱线图的统计数据。
 * \endif
 */
class QWT_EXPORT QwtBoxChartData : public QwtArraySeriesData< QwtBoxSample >
{
public:
    //! Constructor
    QwtBoxChartData(const QVector< QwtBoxSample >& = QVector< QwtBoxSample >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;
};

/**
 * \if ENGLISH
 * @brief Interface for iterating over an array of boxplot outlier samples
 * @details QwtBoxOutlierChartData provides access to QwtBoxOutlierSample samples stored in a QVector.
 *          Used for displaying outlier points in box-and-whisker plots.
 * \endif
 * \if CHINESE
 * @brief 箱线图异常值样本数组迭代接口
 * @details QwtBoxOutlierChartData 提供对存储在 QVector 中的 QwtBoxOutlierSample 样本的访问。
 *          用于显示箱线图中的异常值点。
 * \endif
 */
class QWT_EXPORT QwtBoxOutlierChartData : public QwtArraySeriesData< QwtBoxOutlierSample >
{
public:
    //! Constructor
    QwtBoxOutlierChartData(const QVector< QwtBoxOutlierSample >& = QVector< QwtBoxOutlierSample >());

    //! Calculate the bounding rectangle
    virtual QRectF boundingRect() const override;

    //! Get total outlier count across all boxes
    int totalOutlierCount() const;
};

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QPointF >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtPoint3D >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtPointPolar >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtIntervalSample >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtSetSample >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtOHLCSample >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtVectorFieldSample >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtBoxSample >&, size_t from = 0, size_t to = 0);

QWT_EXPORT QRectF qwtBoundingRect(const QwtSeriesData< QwtBoxOutlierSample >&, size_t from = 0, size_t to = 0);

/**
 * \if ENGLISH
 * @brief Binary search for a sorted series of samples
 * @details qwtUpperSampleIndex returns the index of sample that is the upper bound
 *          of value. Is the the value smaller than the smallest value the return
 *          value will be 0. Is the value greater or equal than the largest
 *          value the return value will be series.size().
 *
 * @par Example
 * The following example shows finds a point of curve from an x coordinate
 * @code
 * #include <qwt_series_data.h>
 * #include <qwt_plot_curve.h>
 *
 *   struct compareX
 *   {
 *       inline bool operator()( const double x, const QPointF &pos ) const
 *       {
 *           return ( x < pos.x() );
 *       }
 *   };
 *
 *   QLineF curveLineAt( const QwtPlotCurve *curve, double x )
 *   {
 *       int index = qwtUpperSampleIndex<QPointF>(*curve->data(), x, compareX() );
 *
 *       if ( index == -1 &&
 *           x == curve->sample( curve->dataSize() - 1 ).x() )
 *       {
 *           // the last sample is excluded from qwtUpperSampleIndex
 *           index = curve->dataSize() - 1;
 *       }
 *
 *       QLineF line; // invalid
 *       if ( index > 0 )
 *       {
 *           line.setP1( curve->sample( index - 1 ) );
 *           line.setP2( curve->sample( index ) );
 *       }
 *
 *       return line;
 *   }
 *
 * @endcode
 * @endpar
 *
 * @param series Series of samples
 * @param value Value
 * @param lessThan Compare operation
 * @note The samples must be sorted according to the order specified by the lessThan object
 *
 * @note Starting from qwt7, this function is changed to size_t version. If not found,
 *       it will return series.size(), similar to an end iterator.
 * \endif
 * \if CHINESE
 * @brief 对排序的样本系列进行二分查找
 * @details qwtUpperSampleIndex 返回样本的索引，该样本是值的上界。如果值小于最小值，
 *          则返回值为 0。如果值大于或等于最大值，则返回值为 series.size()。
 *
 * @par 示例
 * 以下示例展示了如何根据 x 坐标找到曲线上的点
 * @code
 * #include <qwt_series_data.h>
 * #include <qwt_plot_curve.h>
 *
 *   struct compareX
 *   {
 *       inline bool operator()( const double x, const QPointF &pos ) const
 *       {
 *           return ( x < pos.x() );
 *       }
 *   };
 *
 *   QLineF curveLineAt( const QwtPlotCurve *curve, double x )
 *   {
 *       int index = qwtUpperSampleIndex<QPointF>(*curve->data(), x, compareX() );
 *
 *       if ( index == -1 &&
 *           x == curve->sample( curve->dataSize() - 1 ).x() )
 *       {
 *           // the last sample is excluded from qwtUpperSampleIndex
 *           index = curve->dataSize() - 1;
 *       }
 *
 *       QLineF line; // invalid
 *       if ( index > 0 )
 *       {
 *           line.setP1( curve->sample( index - 1 ) );
 *           line.setP2( curve->sample( index ) );
 *       }
 *
 *       return line;
 *   }
 *
 * @endcode
 * @endpar
 *
 * @param series 样本系列
 * @param value 值
 * @param lessThan 比较操作
 * @note 样本必须按照 lessThan 对象指定的顺序排序
 *
 * @note 从 qwt7 开始，此函数改为 size_t 版本。如果未找到，将返回 series.size()，类似于 end 迭代器。
 * \endif
 */
template< typename T, typename LessThan >
inline size_t qwtUpperSampleIndex(const QwtSeriesData< T >& series, double value, LessThan lessThan)
{
    const size_t count = series.size();
    if (count == 0) {
        return count;  // 返回 0 作为“未找到”的标记（因为有效索引从 0 开始，count 超出了有效范围）
    }

    const size_t indexMax = count - 1;

    // 如果 value 大于等于最后一个元素，说明没有元素大于 value，返回 count
    if (!lessThan(value, series.sample(indexMax))) {
        return count;
    }

    size_t indexMin = 0;
    size_t n        = indexMax;  // n 表示当前搜索区间的大小

    while (n > 0) {
        const size_t half     = n >> 1;
        const size_t indexMid = indexMin + half;

        if (lessThan(value, series.sample(indexMid))) {
            // 目标在左侧区间 [indexMin, indexMid]
            n = half;
        } else {
            // 目标在右侧区间 [indexMid + 1, indexMin + n - 1]
            indexMin = indexMid + 1;
            n -= half + 1;
        }
    }

    return indexMin;
}

#endif
