# Boxplot Component Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a complete boxplot visualization component (`QwtPlotBoxChart`) for Qwt with whisker calculation, box rendering, outlier support, and comprehensive customization.

**Architecture:** Three-layer: (1) Data structures in qwt_samples.h, (2) Calculator helper class, (3) Main plot item class. Outliers stored separately for independent styling.

**Tech Stack:** Qt5/Qt6, Qwt framework, C++11/17, CMake

---

## File Structure

### New Files
| File | Purpose |
|------|---------|
| `src/plot/qwt_box_statistics.h` | Calculator class header |
| `src/plot/qwt_box_statistics.cpp` | Calculator implementation |
| `src/plot/qwt_plot_boxchart.h` | Main plot item header |
| `src/plot/qwt_plot_boxchart.cpp` | Main plot item implementation |
| `classincludes/QwtPlotBoxChart` | Class include mapping |

### Modified Files
| File | Modification |
|------|---------|
| `src/plot/qwt_samples.h` | Add QwtStatisticalSample, QwtBoxSample, QwtBoxOutlierSample |
| `src/plot/qwt_series_data.h` | Add QwtBoxChartData, QwtBoxOutlierChartData |
| `src/plot/qwt_plot_item.h` | Add Rtti_PlotBoxChart enum value |
| `src/plot/CMakeLists.txt` | Add new source files |

---

## Task 1: Add Base Statistical Sample Structure

**Files:**
- Modify: `src/plot/qwt_samples.h`

- [ ] **Step 1: Add QwtStatisticalSample class before QwtIntervalSample**

Insert after the includes, before QwtIntervalSample (around line 36):

```cpp
/**
 * \if ENGLISH
 * @brief Base class for statistical samples with position and range
 * @details Provides common fields for samples that have a position on one axis
 *          and a statistical range on the other axis (used by boxplots, OHLC charts).
 * \endif
 * 
 * \if CHINESE
 * @brief 统计样本的基类，包含位置和范围
 * @details 为在一个轴上有位置、在另一个轴上有统计范围的样本提供公共字段
 *          （用于箱线图、OHLC 图表等）。
 * \endif
 */
class QWT_EXPORT QwtStatisticalSample
{
public:
    /**
     * \if ENGLISH
     * @brief Default constructor
     * @details All values set to 0.0
     * \endif
     * \if CHINESE
     * @brief 默认构造函数
     * @details 所有值设置为 0.0
     * \endif
     */
    QwtStatisticalSample(double position = 0.0);
    
    //! Position on the "time" axis (x for vertical, y for horizontal orientation)
    double position;
    
    //! Lower bound of the statistical range
    double lower;
    
    //! Upper bound of the statistical range
    double upper;
    
    //! Central reference value
    double center;
};
```

- [ ] **Step 2: Add inline constructor implementation**

After the class declaration:

```cpp
inline QwtStatisticalSample::QwtStatisticalSample(double pos)
    : position(pos)
    , lower(0.0)
    , upper(0.0)
    , center(0.0)
{
}
```

- [ ] **Step 3: Commit**

```bash
git add src/plot/qwt_samples.h
git commit -m "feat: add QwtStatisticalSample base class for statistical samples"
```

---

## Task 2: Add Box Sample Data Structure

**Files:**
- Modify: `src/plot/qwt_samples.h`

- [ ] **Step 1: Add QwtBoxSample class after QwtVectorFieldSample**

Insert at end of file, before the final `#endif`:

```cpp
/**
 * \if ENGLISH
 * @brief Sample for box-and-whisker plot (boxplot) visualization
 * @details Contains all statistical values needed to render a boxplot:
 *          whisker endpoints, quartiles, median, and outlier count.
 *          Actual outlier values are stored separately in QwtBoxOutlierSample.
 * \endif
 * 
 * \if CHINESE
 * @brief 箱线图（boxplot）样本
 * @details 包含绘制箱线图所需的所有统计值：
 *          须须端点、四分位数、中位数和异常值计数。
 *          实际异常值单独存储在 QwtBoxOutlierSample 中。
 * \endif
 */
class QWT_EXPORT QwtBoxSample : public QwtStatisticalSample
{
public:
    /**
     * \if ENGLISH
     * @brief Default constructor
     * @details All values set to 0.0
     * \endif
     * \if CHINESE
     * @brief 默认构造函数
     * @details 所有值设置为 0.0
     * \endif
     */
    QwtBoxSample(double position = 0.0);
    
    /**
     * \if ENGLISH
     * @brief Full constructor with all statistical values
     * @param position Position on the axis
     * @param whiskerLower Lower whisker endpoint
     * @param q1 First quartile (25th percentile)
     * @param median Median value (50th percentile)
     * @param q3 Third quartile (75th percentile)
     * @param whiskerUpper Upper whisker endpoint
     * \endif
     * \if CHINESE
     * @brief 包含所有统计值的完整构造函数
     * @param position 轴上的位置
     * @param whiskerLower 下须须端点
     * @param q1 第一四分位数（第25百分位）
     * @param median 中位数（第50百分位）
     * @param q3 第三四分位数（第75百分位）
     * @param whiskerUpper 上须须端点
     * \endif
     */
    QwtBoxSample(double position, double whiskerLower, double q1,
                 double median, double q3, double whiskerUpper);
    
    /**
     * \if ENGLISH
     * @brief Check if sample has valid ordering
     * @details Returns true if whiskerLower <= q1 <= median <= q3 <= whiskerUpper
     * \endif
     * \if CHINESE
     * @brief 检查样本顺序是否有效
     * @details 当 whiskerLower <= q1 <= median <= q3 <= whiskerUpper 时返回 true
     * \endif
     */
    bool isValid() const;
    
    /**
     * \if ENGLISH
     * @brief Get bounding interval including whiskers
     * @return Interval from whiskerLower to whiskerUpper
     * \endif
     * \if CHINESE
     * @brief 获取包含须须的边界区间
     * @return 从 whiskerLower 到 whiskerUpper 的区间
     * \endif
     */
    QwtInterval boundingInterval() const;
    
    /**
     * \if ENGLISH
     * @brief Get box body interval (Q1 to Q3)
     * @return Interval from q1 to q3
     * \endif
     * \if CHINESE
     * @brief 获取箱体区间（Q1 到 Q3）
     * @return 从 q1 到 q3 的区间
     * \endif
     */
    QwtInterval boxInterval() const;
    
    //! Lower whisker endpoint
    double whiskerLower;
    
    //! First quartile (25th percentile)
    double q1;
    
    //! Median (50th percentile) - also stored in inherited 'center' field
    double median;
    
    //! Third quartile (75th percentile)
    double q3;
    
    //! Upper whisker endpoint
    double whiskerUpper;
    
    //! Number of outliers (stored separately, this is count only)
    int outlierCount;
};
```

- [ ] **Step 2: Add inline implementations**

```cpp
inline QwtBoxSample::QwtBoxSample(double pos)
    : QwtStatisticalSample(pos)
    , whiskerLower(0.0)
    , q1(0.0)
    , median(0.0)
    , q3(0.0)
    , whiskerUpper(0.0)
    , outlierCount(0)
{
}

inline QwtBoxSample::QwtBoxSample(double pos, double wl, double q1v,
                                   double med, double q3v, double wu)
    : QwtStatisticalSample(pos)
    , whiskerLower(wl)
    , q1(q1v)
    , median(med)
    , q3(q3v)
    , whiskerUpper(wu)
    , outlierCount(0)
{
    center = median;
}

inline bool QwtBoxSample::isValid() const
{
    return (whiskerLower <= q1) && (q1 <= median) && 
           (median <= q3) && (q3 <= whiskerUpper);
}

inline QwtInterval QwtBoxSample::boundingInterval() const
{
    return QwtInterval(whiskerLower, whiskerUpper);
}

inline QwtInterval QwtBoxSample::boxInterval() const
{
    return QwtInterval(q1, q3);
}
```

- [ ] **Step 3: Commit**

```bash
git add src/plot/qwt_samples.h
git commit -m "feat: add QwtBoxSample data structure for boxplot"
```

---

## Task 3: Add Outlier Sample Data Structure

**Files:**
- Modify: `src/plot/qwt_samples.h`

- [ ] **Step 1: Add QwtBoxOutlierSample class after QwtBoxSample**

```cpp
/**
 * \if ENGLISH
 * @brief Outlier values for a single boxplot position
 * @details Contains all outlier values associated with one box position.
 *          One QwtBoxOutlierSample corresponds to one QwtBoxSample.
 * \endif
 * 
 * \if CHINESE
 * @brief 单个箱线图位置的异常值
 * @details 包含与一个箱位置关联的所有异常值。
 *          一个 QwtBoxOutlierSample 对应一个 QwtBoxSample。
 * \endif
 */
class QWT_EXPORT QwtBoxOutlierSample
{
public:
    /**
     * \if ENGLISH
     * @brief Default constructor
     * \endif
     * \if CHINESE
     * @brief 默认构造函数
     * \endif
     */
    QwtBoxOutlierSample(double boxPosition = 0.0);
    
    /**
     * \if ENGLISH
     * @brief Constructor with position and outlier values
     * @param boxPosition Position matching parent QwtBoxSample
     * @param values All outlier values for this box
     * \endif
     * \if CHINESE
     * @brief 包含位置和异常值的构造函数
     * @param boxPosition 匹配父 QwtBoxSample 的位置
     * @param values 此箱的所有异常值
     * \endif
     */
    QwtBoxOutlierSample(double boxPosition, const QVector<double>& values);
    
    /**
     * \if ENGLISH
     * @brief Constructor with move semantics
     * \endif
     * \if CHINESE
     * @brief 移动语义构造函数
     * \endif
     */
    QwtBoxOutlierSample(double boxPosition, QVector<double>&& values);
    
    //! Check if no outliers present
    bool isEmpty() const { return values.isEmpty(); }
    
    //! Get number of outliers
    int count() const { return values.size(); }
    
    //! Position of the parent box (matches QwtBoxSample.position)
    double boxPosition;
    
    //! All outlier values for this box
    QVector<double> values;
};
```

- [ ] **Step 2: Add inline implementations**

```cpp
inline QwtBoxOutlierSample::QwtBoxOutlierSample(double pos)
    : boxPosition(pos)
    , values()
{
}

inline QwtBoxOutlierSample::QwtBoxOutlierSample(double pos, const QVector<double>& vals)
    : boxPosition(pos)
    , values(vals)
{
}

inline QwtBoxOutlierSample::QwtBoxOutlierSample(double pos, QVector<double>&& vals)
    : boxPosition(pos)
    , values(std::move(vals))
{
}
```

- [ ] **Step 3: Commit**

```bash
git add src/plot/qwt_samples.h
git commit -m "feat: add QwtBoxOutlierSample for outlier data storage"
```

---

## Task 4: Add Box Data Series Classes

**Files:**
- Modify: `src/plot/qwt_series_data.h`

- [ ] **Step 1: Read existing file to find insertion point**

Check where `QwtArraySeriesData` is defined and where other series data classes are located.

- [ ] **Step 2: Add QwtBoxChartData class**

Insert after existing series data classes (after TradingChartData if present, or near end):

```cpp
/**
 * \if ENGLISH
 * @brief Series data storing boxplot samples in a vector
 * \endif
 * \if CHINESE
 * @brief 在向量中存储箱线图样本的序列数据
 * \endif
 */
class QWT_EXPORT QwtBoxChartData : public QwtArraySeriesData<QwtBoxSample>
{
public:
    QwtBoxChartData();
    QwtBoxChartData(const QVector<QwtBoxSample>& samples);
    QwtBoxChartData(QVector<QwtBoxSample>&& samples);
    
    virtual QRectF boundingRect() const override;
};
```

- [ ] **Step 3: Add inline implementations**

```cpp
inline QwtBoxChartData::QwtBoxChartData()
    : QwtArraySeriesData<QwtBoxSample>()
{
}

inline QwtBoxChartData::QwtBoxChartData(const QVector<QwtBoxSample>& samples)
    : QwtArraySeriesData<QwtBoxSample>(samples)
{
}

inline QwtBoxChartData::QwtBoxChartData(QVector<QwtBoxSample>&& samples)
    : QwtArraySeriesData<QwtBoxSample>(std::move(samples))
{
}

inline QRectF QwtBoxChartData::boundingRect() const
{
    if (size() == 0)
        return QRectF(1.0, 1.0, -2.0, -2.0); // invalid
    
    double minPos = sample(0).position;
    double maxPos = minPos;
    double minVal = sample(0).whiskerLower;
    double maxVal = sample(0).whiskerUpper;
    
    for (size_t i = 1; i < size(); ++i)
    {
        const QwtBoxSample& s = sample(i);
        minPos = qwtMinF(minPos, s.position);
        maxPos = qwtMaxF(maxPos, s.position);
        minVal = qwtMinF(minVal, s.whiskerLower);
        maxVal = qwtMaxF(maxVal, s.whiskerUpper);
    }
    
    return QRectF(minPos, minVal, maxPos - minPos, maxVal - minVal);
}
```

- [ ] **Step 4: Add QwtBoxOutlierChartData class**

```cpp
/**
 * \if ENGLISH
 * @brief Series data storing outlier samples in a vector
 * \endif
 * \if CHINESE
 * @brief 在向量中存储异常值样本的序列数据
 * \endif
 */
class QWT_EXPORT QwtBoxOutlierChartData : public QwtArraySeriesData<QwtBoxOutlierSample>
{
public:
    QwtBoxOutlierChartData();
    QwtBoxOutlierChartData(const QVector<QwtBoxOutlierSample>& samples);
    QwtBoxOutlierChartData(QVector<QwtBoxOutlierSample>&& samples);
    
    virtual QRectF boundingRect() const override;
    
    //! Get total outlier count across all boxes
    int totalOutlierCount() const;
};
```

- [ ] **Step 5: Add inline implementations for outlier data**

```cpp
inline QwtBoxOutlierChartData::QwtBoxOutlierChartData()
    : QwtArraySeriesData<QwtBoxOutlierSample>()
{
}

inline QwtBoxOutlierChartData::QwtBoxOutlierChartData(const QVector<QwtBoxOutlierSample>& samples)
    : QwtArraySeriesData<QwtBoxOutlierSample>(samples)
{
}

inline QwtBoxOutlierChartData::QwtBoxOutlierChartData(QVector<QwtBoxOutlierSample>&& samples)
    : QwtArraySeriesData<QwtBoxOutlierSample>(std::move(samples))
{
}

inline QRectF QwtBoxOutlierChartData::boundingRect() const
{
    if (size() == 0)
        return QRectF(1.0, 1.0, -2.0, -2.0); // invalid
    
    double minPos = sample(0).boxPosition;
    double maxPos = minPos;
    double minVal = 0.0;
    double maxVal = 0.0;
    bool hasValues = false;
    
    for (size_t i = 0; i < size(); ++i)
    {
        const QwtBoxOutlierSample& s = sample(i);
        minPos = qwtMinF(minPos, s.boxPosition);
        maxPos = qwtMaxF(maxPos, s.boxPosition);
        
        for (int j = 0; j < s.count(); ++j)
        {
            if (!hasValues)
            {
                minVal = s.values[j];
                maxVal = s.values[j];
                hasValues = true;
            }
            else
            {
                minVal = qwtMinF(minVal, s.values[j]);
                maxVal = qwtMaxF(maxVal, s.values[j]);
            }
        }
    }
    
    if (!hasValues)
        return QRectF(minPos, 0.0, maxPos - minPos, 0.0);
    
    return QRectF(minPos, minVal, maxPos - minPos, maxVal - minVal);
}

inline int QwtBoxOutlierChartData::totalOutlierCount() const
{
    int count = 0;
    for (size_t i = 0; i < size(); ++i)
        count += sample(i).count();
    return count;
}
```

- [ ] **Step 6: Commit**

```bash
git add src/plot/qwt_series_data.h
git commit -m "feat: add QwtBoxChartData and QwtBoxOutlierChartData series classes"
```

---

## Task 5: Add RTTI Value for BoxChart

**Files:**
- Modify: `src/plot/qwt_plot_item.h`

- [ ] **Step 1: Find existing RttiValues enum**

Check the RTTI enum values to determine the next available number.

- [ ] **Step 2: Add Rtti_PlotBoxChart to enum**

Add after existing plot item types:

```cpp
enum RttiValues
{
    // ... existing values ...
    
    //! Boxplot chart item
    Rtti_PlotBoxChart,
    
    // ... rest of existing values ...
};
```

- [ ] **Step 3: Commit**

```bash
git add src/plot/qwt_plot_item.h
git commit -m "feat: add Rtti_PlotBoxChart enum value"
```

---

## Task 6: Create Statistics Calculator Header

**Files:**
- Create: `src/plot/qwt_box_statistics.h`

- [ ] **Step 1: Create header file with complete class definition**

```cpp
/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *****************************************************************************/

#ifndef QWT_BOX_STATISTICS_H
#define QWT_BOX_STATISTICS_H

#include "qwt_global.h"
#include "qwt_samples.h"

#include <qvector.h>

/**
 * \if ENGLISH
 * @brief Helper class for computing boxplot statistics from raw data
 * @details Provides static methods to calculate whisker bounds, quartiles,
 *          median, and extract outliers using various methods (Tukey, percentile, SD, SE).
 * \endif
 * 
 * \if CHINESE
 * @brief 从原始数据计算箱线图统计量的辅助类
 * @details 提供静态方法，使用各种方法（Tukey、百分位、SD、SE）
 *          计算须须边界、四分位数、中位数并提取异常值。
 * \endif
 */
class QWT_EXPORT QwtBoxStatisticsCalculator
{
public:
    /**
     * \if ENGLISH
     * @brief Whisker calculation method
     * \endif
     * \if CHINESE
     * @brief 须须计算方法
     * \endif
     */
    enum WhiskerMethod
    {
        //! Tukey's method: whiskers extend to 1.5×IQR from Q1/Q3 (default)
        Tukey,
        
        //! Percentile method: whiskers at specified percentile range
        Percentile,
        
        //! Min/Max method: whiskers at actual data min/max
        MinMax,
        
        //! Standard deviation method: whiskers at mean ± coeff×SD
        StandardDeviation,
        
        //! Standard error method: whiskers at mean ± coeff×SE
        StandardError
    };
    
    /**
     * \if ENGLISH
     * @brief Constructor with default Tukey method
     * \endif
     * \if CHINESE
     * @brief 默认使用 Tukey 方法的构造函数
     * \endif
     */
    QwtBoxStatisticsCalculator();
    
    /**
     * \if ENGLISH
     * @brief Set whisker calculation method
     * \endif
     * \if CHINESE
     * @brief 设置须须计算方法
     * \endif
     */
    void setWhiskerMethod(WhiskerMethod method);
    
    /**
     * \if ENGLISH
     * @brief Get whisker calculation method
     * \endif
     * \if CHINESE
     * @brief 获取须须计算方法
     * \endif
     */
    WhiskerMethod whiskerMethod() const;
    
    /**
     * \if ENGLISH
     * @brief Set whisker coefficient (1.5 for Tukey, percentile for Percentile method)
     * \endif
     * \if CHINESE
     * @brief 设置须须系数（Tukey 方法用 1.5，百分位方法用百分位值）
     * \endif
     */
    void setWhiskerCoefficient(double coeff);
    
    /**
     * \if ENGLISH
     * @brief Get whisker coefficient
     * \endif
     * \if CHINESE
     * @brief 获取须须系数
     * \endif
     */
    double whiskerCoefficient() const;
    
    /**
     * \if ENGLISH
     * @brief Compute QwtBoxSample from sorted raw data
     * @param position Position value for the sample
     * @param sortedData Data vector (must be pre-sorted ascending)
     * @param method Whisker calculation method
     * @param coefficient Whisker coefficient
     * @return Computed box sample with all statistics
     * \endif
     * \if CHINESE
     * @brief 从已排序的原始数据计算 QwtBoxSample
     * @param position 样本的位置值
     * @param sortedData 数据向量（必须预先升序排序）
     * @param method 须须计算方法
     * @param coefficient 须须系数
     * @return 包含所有统计量的箱样本
     * \endif
     */
    static QwtBoxSample calculate(
        double position,
        const QVector<double>& sortedData,
        WhiskerMethod method = Tukey,
        double coefficient = 1.5);
    
    /**
     * \if ENGLISH
     * @brief Compute QwtBoxSample from unsorted raw data
     * @details Sorts data internally before computing statistics
     * @param position Position value for the sample
     * @param rawData Unsorted data vector
     * @param method Whisker calculation method
     * @param coefficient Whisker coefficient
     * @return Computed box sample with all statistics
     * \endif
     * \if CHINESE
     * @brief 从未排序的原始数据计算 QwtBoxSample
     * @details 在计算统计量之前内部对数据进行排序
     * @param position 样本的位置值
     * @param rawData 未排序的数据向量
     * @param method 须须计算方法
     * @param coefficient 须须系数
     * @return 包含所有统计量的箱样本
     * \endif
     */
    static QwtBoxSample calculateFromRaw(
        double position,
        const QVector<double>& rawData,
        WhiskerMethod method = Tukey,
        double coefficient = 1.5);
    
    /**
     * \if ENGLISH
     * @brief Extract outliers given calculated box sample and sorted data
     * @param sample Pre-calculated box sample
     * @param sortedData Data vector (must be pre-sorted ascending)
     * @return Vector of outlier values
     * \endif
     * \if CHINESE
     * @brief 根据已计算的箱样本和已排序数据提取异常值
     * @param sample 预计算的箱样本
     * @param sortedData 数据向量（必须预先升序排序）
     * @return 异常值向量
     * \endif
     */
    static QVector<double> extractOutliers(
        const QwtBoxSample& sample,
        const QVector<double>& sortedData);
    
    /**
     * \if ENGLISH
     * @brief Compute full statistics (sample + outliers) in one call
     * @param position Position value for the sample
     * @param rawData Unsorted data vector
     * @param[out] sample Output box sample
     * @param[out] outliers Output outlier sample
     * @param method Whisker calculation method
     * @param coefficient Whisker coefficient
     * \endif
     * \if CHINESE
     * @brief 在一次调用中计算完整统计量（样本 + 异常值）
     * @param position 样本的位置值
     * @param rawData 未排序的数据向量
     * @param[out] sample 输出的箱样本
     * @param[out] outliers 输出的异常值样本
     * @param method 须须计算方法
     * @param coefficient 须须系数
     * \endif
     */
    static void calculateFull(
        double position,
        const QVector<double>& rawData,
        QwtBoxSample& sample,
        QwtBoxOutlierSample& outliers,
        WhiskerMethod method = Tukey,
        double coefficient = 1.5);
    
private:
    //! Helper: compute quantile from sorted data (p in [0,1])
    static double quantile(const QVector<double>& sorted, double p);
    
    //! Helper: compute median from sorted data
    static double median(const QVector<double>& sorted);
    
    //! Helper: compute IQR from sorted data
    static double iqr(const QVector<double>& sorted);
    
    //! Helper: compute mean from data
    static double mean(const QVector<double>& data);
    
    //! Helper: compute standard deviation
    static double standardDeviation(const QVector<double>& data);
    
    //! Helper: sort data (returns new sorted vector)
    static QVector<double> sortData(const QVector<double>& data);
    
    WhiskerMethod m_method;
    double m_coefficient;
};

#endif // QWT_BOX_STATISTICS_H
```

- [ ] **Step 2: Commit**

```bash
git add src/plot/qwt_box_statistics.h
git commit -m "feat: add QwtBoxStatisticsCalculator header"
```

---

## Task 7: Implement Statistics Calculator

**Files:**
- Create: `src/plot/qwt_box_statistics.cpp`

- [ ] **Step 1: Create implementation file**

```cpp
/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *****************************************************************************/

#include "qwt_box_statistics.h"
#include "qwt_math.h"

#include <algorithm>

QwtBoxStatisticsCalculator::QwtBoxStatisticsCalculator()
    : m_method(Tukey)
    , m_coefficient(1.5)
{
}

void QwtBoxStatisticsCalculator::setWhiskerMethod(WhiskerMethod method)
{
    m_method = method;
}

QwtBoxStatisticsCalculator::WhiskerMethod QwtBoxStatisticsCalculator::whiskerMethod() const
{
    return m_method;
}

void QwtBoxStatisticsCalculator::setWhiskerCoefficient(double coeff)
{
    m_coefficient = coeff;
}

double QwtBoxStatisticsCalculator::whiskerCoefficient() const
{
    return m_coefficient;
}

QVector<double> QwtBoxStatisticsCalculator::sortData(const QVector<double>& data)
{
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}

double QwtBoxStatisticsCalculator::quantile(const QVector<double>& sorted, double p)
{
    if (sorted.isEmpty())
        return 0.0;
    
    const int n = sorted.size();
    const double index = p * (n - 1);
    const int lower = static_cast<int>(std::floor(index));
    const int upper = static_cast<int>(std::ceil(index));
    
    if (lower == upper)
        return sorted[lower];
    
    // Linear interpolation
    const double frac = index - lower;
    return sorted[lower] + frac * (sorted[upper] - sorted[lower]);
}

double QwtBoxStatisticsCalculator::median(const QVector<double>& sorted)
{
    return quantile(sorted, 0.5);
}

double QwtBoxStatisticsCalculator::iqr(const QVector<double>& sorted)
{
    return quantile(sorted, 0.75) - quantile(sorted, 0.25);
}

double QwtBoxStatisticsCalculator::mean(const QVector<double>& data)
{
    if (data.isEmpty())
        return 0.0;
    
    double sum = 0.0;
    for (int i = 0; i < data.size(); ++i)
        sum += data[i];
    
    return sum / data.size();
}

double QwtBoxStatisticsCalculator::standardDeviation(const QVector<double>& data)
{
    if (data.size() < 2)
        return 0.0;
    
    const double m = mean(data);
    double sumSq = 0.0;
    for (int i = 0; i < data.size(); ++i)
    {
        const double diff = data[i] - m;
        sumSq += diff * diff;
    }
    
    return std::sqrt(sumSq / (data.size() - 1));
}

QwtBoxSample QwtBoxStatisticsCalculator::calculate(
    double position,
    const QVector<double>& sortedData,
    WhiskerMethod method,
    double coefficient)
{
    if (sortedData.isEmpty())
        return QwtBoxSample(position);
    
    const double minVal = sortedData.first();
    const double maxVal = sortedData.last();
    const double med = median(sortedData);
    const double q1 = quantile(sortedData, 0.25);
    const double q3 = quantile(sortedData, 0.75);
    
    double whiskerLower, whiskerUpper;
    
    switch (method)
    {
        case Tukey:
        {
            const double iqrVal = q3 - q1;
            whiskerLower = qwtMaxF(minVal, q1 - coefficient * iqrVal);
            whiskerUpper = qwtMinF(maxVal, q3 + coefficient * iqrVal);
            break;
        }
        case Percentile:
        {
            whiskerLower = quantile(sortedData, 1.0 - coefficient / 100.0);
            whiskerUpper = quantile(sortedData, coefficient / 100.0);
            break;
        }
        case MinMax:
        {
            whiskerLower = minVal;
            whiskerUpper = maxVal;
            break;
        }
        case StandardDeviation:
        {
            const double m = mean(sortedData);
            const double sd = standardDeviation(sortedData);
            whiskerLower = qwtMaxF(minVal, m - coefficient * sd);
            whiskerUpper = qwtMinF(maxVal, m + coefficient * sd);
            break;
        }
        case StandardError:
        {
            const double m = mean(sortedData);
            const double sd = standardDeviation(sortedData);
            const double se = sd / std::sqrt(static_cast<double>(sortedData.size()));
            whiskerLower = qwtMaxF(minVal, m - coefficient * se);
            whiskerUpper = qwtMinF(maxVal, m + coefficient * se);
            break;
        }
        default:
        {
            whiskerLower = minVal;
            whiskerUpper = maxVal;
            break;
        }
    }
    
    QwtBoxSample sample(position, whiskerLower, q1, med, q3, whiskerUpper);
    
    // Count outliers
    int outlierCount = 0;
    for (int i = 0; i < sortedData.size(); ++i)
    {
        if (sortedData[i] < whiskerLower || sortedData[i] > whiskerUpper)
            outlierCount++;
    }
    sample.outlierCount = outlierCount;
    
    return sample;
}

QwtBoxSample QwtBoxStatisticsCalculator::calculateFromRaw(
    double position,
    const QVector<double>& rawData,
    WhiskerMethod method,
    double coefficient)
{
    QVector<double> sorted = sortData(rawData);
    return calculate(position, sorted, method, coefficient);
}

QVector<double> QwtBoxStatisticsCalculator::extractOutliers(
    const QwtBoxSample& sample,
    const QVector<double>& sortedData)
{
    QVector<double> outliers;
    
    for (int i = 0; i < sortedData.size(); ++i)
    {
        const double val = sortedData[i];
        if (val < sample.whiskerLower || val > sample.whiskerUpper)
            outliers.append(val);
    }
    
    return outliers;
}

void QwtBoxStatisticsCalculator::calculateFull(
    double position,
    const QVector<double>& rawData,
    QwtBoxSample& sample,
    QwtBoxOutlierSample& outliers,
    WhiskerMethod method,
    double coefficient)
{
    QVector<double> sorted = sortData(rawData);
    sample = calculate(position, sorted, method, coefficient);
    
    QVector<double> outlierValues = extractOutliers(sample, sorted);
    outliers = QwtBoxOutlierSample(position, std::move(outlierValues));
}
```

- [ ] **Step 2: Commit**

```bash
git add src/plot/qwt_box_statistics.cpp
git commit -m "feat: implement QwtBoxStatisticsCalculator with Tukey/percentile/SD/SE methods"
```

---

## Task 8: Create BoxChart Header

**Files:**
- Create: `src/plot/qwt_plot_boxchart.h`

- [ ] **Step 1: Create complete header file**

```cpp
/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *****************************************************************************/

#ifndef QWT_PLOT_BOXCHART_H
#define QWT_PLOT_BOXCHART_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_store.h"
#include "qwt_samples.h"

class QwtSymbol;
class QwtSeriesData;
template<typename T> class QwtSeriesData;

/**
 * \if ENGLISH
 * @brief Plot item for box-and-whisker (boxplot) visualization
 * @details QwtPlotBoxChart displays statistical distributions using the boxplot format:
 *          - Box body showing Q1-Q3 interquartile range
 *          - Median line inside the box
 *          - Whiskers extending to data range or calculated bounds
 *          - Outlier symbols for points outside whiskers
 *          
 *          Supports multiple box styles (Rectangle, Diamond, Notch),
 *          horizontal/vertical orientation, and extensive customization.
 * \endif
 * 
 * \if CHINESE
 * @brief 箱线图（boxplot）绘图项
 * @details QwtPlotBoxChart 使用箱线图格式显示统计分布：
 *          - 箱体显示 Q1-Q3 四分位距
 *          - 箱内的中位数线
 *          - 延伸到数据范围或计算边界的须须
 *          - 须须外部点的异常值符号
 *          
 *          支持多种箱样式（矩形、菱形、缺口），
 *          水平/垂直方向，以及丰富的自定义选项。
 * \endif
 */
class QWT_EXPORT QwtPlotBoxChart
    : public QwtPlotSeriesItem
    , public QwtSeriesStore<QwtBoxSample>
{
public:
    /**
     * \if ENGLISH
     * @brief Box body display style
     * \endif
     * \if CHINESE
     * @brief 箱体显示样式
     * \endif
     */
    enum BoxStyle
    {
        //! No box body, only whiskers and outliers
        NoBox,
        
        //! Traditional rectangular box (Q1-Q3)
        Rect,
        
        //! Diamond shape connecting extremes
        Diamond,
        
        //! Rectangle with notch indentation at median
        Notch
    };
    
    /**
     * \if ENGLISH
     * @brief Whisker display style
     * \endif
     * \if CHINESE
     * @brief 须须显示样式
     * \endif
     */
    enum WhiskerStyle
    {
        //! No whisker lines drawn
        NoWhiskers,
        
        //! Traditional T-bar whiskers with horizontal caps (default)
        StandardWhisker,
        
        //! Simple line from whiskerLower to whiskerUpper
        MinMaxLine
    };
    
    /**
     * \if ENGLISH
     * @brief Paint attributes for performance optimization
     * \endif
     * \if CHINESE
     * @brief 性能优化的绘制属性
     * \endif
     */
    enum PaintAttribute
    {
        //! Clip boxes before painting (performance for zoomed views)
        ClipBoxes = 0x01,
        
        //! Clip outlier symbols before painting
        ClipOutliers = 0x02,
        
        //! Use image buffer for rendering (optimization for many boxes)
        ImageBuffer = 0x04
    };
    
    Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)
    
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     * \if CHINESE
     * @brief 构造函数
     * \endif
     */
    explicit QwtPlotBoxChart(const QString& title = QString());
    
    /**
     * \if ENGLISH
     * @brief Constructor with QwtText title
     * \endif
     * \if CHINESE
     * @brief 带 QwtText 标题的构造函数
     * \endif
     */
    explicit QwtPlotBoxChart(const QwtText& title);
    
    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     * \if CHINESE
     * @brief 析构函数
     * \endif
     */
    virtual ~QwtPlotBoxChart();
    
    /**
     * \if ENGLISH
     * @brief Get runtime type information
     * @return Rtti_PlotBoxChart
     * \endif
     * \if CHINESE
     * @brief 获取运行时类型信息
     * @return Rtti_PlotBoxChart
     * \endif
     */
    virtual int rtti() const override;
    
    //! Set paint attribute
    void setPaintAttribute(PaintAttribute, bool on = true);
    
    //! Test paint attribute
    bool testPaintAttribute(PaintAttribute) const;
    
    //! Set box samples
    void setSamples(const QVector<QwtBoxSample>&);
    void setSamples(QwtSeriesData<QwtBoxSample>*);
    
    //! Set outlier samples (optional)
    void setOutliers(const QVector<QwtBoxOutlierSample>&);
    void setOutliers(QwtSeriesData<QwtBoxOutlierSample>*);
    
    //! Get outlier data
    const QwtSeriesData<QwtBoxOutlierSample>* outlierData() const;
    
    //! Set box style
    void setBoxStyle(BoxStyle);
    BoxStyle boxStyle() const;
    
    //! Set whisker style
    void setWhiskerStyle(WhiskerStyle);
    WhiskerStyle whiskerStyle() const;
    
    //! Set orientation (vertical: x-position, horizontal: y-position)
    void setOrientation(Qt::Orientation);
    Qt::Orientation orientation() const;
    
    //! Set box width in scale coordinates
    void setBoxExtent(double extent);
    double boxExtent() const;
    
    //! Set minimum box width in pixels
    void setMinBoxWidth(double pixels);
    double minBoxWidth() const;
    
    //! Set maximum box width in pixels (negative = unlimited)
    void setMaxBoxWidth(double pixels);
    double maxBoxWidth() const;
    
    //! Set pen for box outline and whiskers
    void setPen(const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine);
    void setPen(const QPen&);
    const QPen& pen() const;
    
    //! Set brush for box body fill
    void setBrush(const QBrush&);
    const QBrush& brush() const;
    
    //! Set pen for median line
    void setMedianPen(const QPen&);
    QPen medianPen() const;
    
    //! Set symbol for outliers
    void setOutlierSymbol(const QwtSymbol*);
    const QwtSymbol* outlierSymbol() const;
    
    //! Set symbol for mean marker
    void setMeanSymbol(const QwtSymbol*);
    const QwtSymbol* meanSymbol() const;
    
    //! Show/hide median line
    void setMedianVisible(bool);
    bool isMedianVisible() const;
    
    //! Show/hide mean marker
    void setMeanVisible(bool);
    bool isMeanVisible() const;
    
    //! Set outlier jitter width (for overlapping outliers)
    void setOutlierJitter(double jitterWidth);
    double outlierJitter() const;
    
    //! Draw the series
    virtual void drawSeries(QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to) const override;
    
    //! Get bounding rectangle
    virtual QRectF boundingRect() const override;
    
    //! Get legend icon
    virtual QwtGraphic legendIcon(int index, const QSizeF&) const override;
    
protected:
    //! Initialize
    void init();
    
    //! Calculate scaled box width in pixels
    virtual double scaledBoxWidth(
        const QwtScaleMap& posMap,
        const QwtScaleMap& valueMap,
        const QRectF& canvasRect) const;
    
    //! Draw a single box
    virtual void drawBox(QPainter*, const QwtBoxSample&,
        Qt::Orientation, double boxWidth, double posPixel,
        const QwtScaleMap& valueMap) const;
    
    //! Draw whiskers for a single box
    virtual void drawWhiskers(QPainter*, const QwtBoxSample&,
        Qt::Orientation, double boxWidth, double posPixel,
        const QwtScaleMap& valueMap) const;
    
    //! Draw median line for a single box
    virtual void drawMedian(QPainter*, const QwtBoxSample&,
        Qt::Orientation, double boxWidth, double posPixel,
        const QwtScaleMap& valueMap) const;
    
    //! Draw outliers for boxes in range
    virtual void drawOutliers(QPainter*,
        const QwtScaleMap& posMap, const QwtScaleMap& valueMap,
        const QRectF& canvasRect, int from, int to) const;
    
    //! Draw a single outlier symbol
    virtual void drawOutlierSymbol(QPainter*, double posPixel, double valuePixel,
        const QwtSymbol& symbol) const;
    
private:
    class PrivateData;
    PrivateData* m_data;
    
    QwtSeriesData<QwtBoxOutlierSample>* m_outlierData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotBoxChart::PaintAttributes)

#endif // QWT_PLOT_BOXCHART_H
```

- [ ] **Step 2: Commit**

```bash
git add src/plot/qwt_plot_boxchart.h
git commit -m "feat: add QwtPlotBoxChart header with complete API"
```

---

## Task 9: Implement BoxChart - Core Methods

**Files:**
- Create: `src/plot/qwt_plot_boxchart.cpp`

- [ ] **Step 1: Create implementation file skeleton with init, rtti, basic setters**

```cpp
/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *****************************************************************************/

#include "qwt_plot_boxchart.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_symbol.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qrandom.h>

class QwtPlotBoxChart::PrivateData
{
public:
    PrivateData()
        : boxStyle(Rect)
        , whiskerStyle(StandardWhisker)
        , orientation(Qt::Vertical)
        , boxExtent(0.6)
        , minBoxWidth(2.0)
        , maxBoxWidth(-1.0)
        , medianVisible(true)
        , meanVisible(false)
        , outlierJitter(0.0)
        , paintAttributes(ClipBoxes | ClipOutliers)
    {
        pen = QPen(Qt::black, 1.0);
        brush = QBrush(Qt::NoBrush);
        medianPen = QPen(Qt::black, 2.0);
        outlierSymbol = nullptr;
        meanSymbol = nullptr;
    }
    
    BoxStyle boxStyle;
    WhiskerStyle whiskerStyle;
    Qt::Orientation orientation;
    double boxExtent;
    double minBoxWidth;
    double maxBoxWidth;
    QPen pen;
    QPen medianPen;
    QBrush brush;
    const QwtSymbol* outlierSymbol;
    const QwtSymbol* meanSymbol;
    bool medianVisible;
    bool meanVisible;
    double outlierJitter;
    PaintAttributes paintAttributes;
};

QwtPlotBoxChart::QwtPlotBoxChart(const QString& title)
    : QwtPlotSeriesItem(QwtText(title))
    , m_outlierData(nullptr)
{
    init();
}

QwtPlotBoxChart::QwtPlotBoxChart(const QwtText& title)
    : QwtPlotSeriesItem(title)
    , m_outlierData(nullptr)
{
    init();
}

QwtPlotBoxChart::~QwtPlotBoxChart()
{
    delete m_data;
    delete m_outlierData;
}

void QwtPlotBoxChart::init()
{
    m_data = new PrivateData();
    
    setItemAttribute(QwtPlotItem::Legend, true);
    setItemAttribute(QwtPlotItem::AutoScale, true);
    
    setData(new QwtBoxChartData());
    setZ(20.0);
}

int QwtPlotBoxChart::rtti() const
{
    return Rtti_PlotBoxChart;
}

void QwtPlotBoxChart::setPaintAttribute(PaintAttribute attr, bool on)
{
    if (on)
        m_data->paintAttributes |= attr;
    else
        m_data->paintAttributes &= ~attr;
}

bool QwtPlotBoxChart::testPaintAttribute(PaintAttribute attr) const
{
    return (m_data->paintAttributes & attr);
}

void QwtPlotBoxChart::setBoxStyle(BoxStyle style)
{
    if (m_data->boxStyle != style)
    {
        m_data->boxStyle = style;
        legendChanged();
        itemChanged();
    }
}

QwtPlotBoxChart::BoxStyle QwtPlotBoxChart::boxStyle() const
{
    return m_data->boxStyle;
}

void QwtPlotBoxChart::setWhiskerStyle(WhiskerStyle style)
{
    if (m_data->whiskerStyle != style)
    {
        m_data->whiskerStyle = style;
        legendChanged();
        itemChanged();
    }
}

QwtPlotBoxChart::WhiskerStyle QwtPlotBoxChart::whiskerStyle() const
{
    return m_data->whiskerStyle;
}

void QwtPlotBoxChart::setOrientation(Qt::Orientation orient)
{
    if (m_data->orientation != orient)
    {
        m_data->orientation = orient;
        legendChanged();
        itemChanged();
    }
}

Qt::Orientation QwtPlotBoxChart::orientation() const
{
    return m_data->orientation;
}

void QwtPlotBoxChart::setBoxExtent(double extent)
{
    extent = qwtMaxF(0.0, extent);
    if (m_data->boxExtent != extent)
    {
        m_data->boxExtent = extent;
        legendChanged();
        itemChanged();
    }
}

double QwtPlotBoxChart::boxExtent() const
{
    return m_data->boxExtent;
}

void QwtPlotBoxChart::setMinBoxWidth(double pixels)
{
    pixels = qwtMaxF(0.0, pixels);
    if (m_data->minBoxWidth != pixels)
    {
        m_data->minBoxWidth = pixels;
        legendChanged();
        itemChanged();
    }
}

double QwtPlotBoxChart::minBoxWidth() const
{
    return m_data->minBoxWidth;
}

void QwtPlotBoxChart::setMaxBoxWidth(double pixels)
{
    if (m_data->maxBoxWidth != pixels)
    {
        m_data->maxBoxWidth = pixels;
        legendChanged();
        itemChanged();
    }
}

double QwtPlotBoxChart::maxBoxWidth() const
{
    return m_data->maxBoxWidth;
}
```

- [ ] **Step 2: Add pen/brush/symbol setters**

```cpp
void QwtPlotBoxChart::setPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setPen(QPen(color, width, style));
}

void QwtPlotBoxChart::setPen(const QPen& pen)
{
    if (m_data->pen != pen)
    {
        m_data->pen = pen;
        legendChanged();
        itemChanged();
    }
}

const QPen& QwtPlotBoxChart::pen() const
{
    return m_data->pen;
}

void QwtPlotBoxChart::setBrush(const QBrush& brush)
{
    if (m_data->brush != brush)
    {
        m_data->brush = brush;
        legendChanged();
        itemChanged();
    }
}

const QBrush& QwtPlotBoxChart::brush() const
{
    return m_data->brush;
}

void QwtPlotBoxChart::setMedianPen(const QPen& pen)
{
    if (m_data->medianPen != pen)
    {
        m_data->medianPen = pen;
        legendChanged();
        itemChanged();
    }
}

QPen QwtPlotBoxChart::medianPen() const
{
    return m_data->medianPen;
}

void QwtPlotBoxChart::setOutlierSymbol(const QwtSymbol* symbol)
{
    if (m_data->outlierSymbol != symbol)
    {
        delete m_data->outlierSymbol;
        m_data->outlierSymbol = symbol;
        legendChanged();
        itemChanged();
    }
}

const QwtSymbol* QwtPlotBoxChart::outlierSymbol() const
{
    return m_data->outlierSymbol;
}

void QwtPlotBoxChart::setMeanSymbol(const QwtSymbol* symbol)
{
    if (m_data->meanSymbol != symbol)
    {
        delete m_data->meanSymbol;
        m_data->meanSymbol = symbol;
        legendChanged();
        itemChanged();
    }
}

const QwtSymbol* QwtPlotBoxChart::meanSymbol() const
{
    return m_data->meanSymbol;
}

void QwtPlotBoxChart::setMedianVisible(bool visible)
{
    if (m_data->medianVisible != visible)
    {
        m_data->medianVisible = visible;
        itemChanged();
    }
}

bool QwtPlotBoxChart::isMedianVisible() const
{
    return m_data->medianVisible;
}

void QwtPlotBoxChart::setMeanVisible(bool visible)
{
    if (m_data->meanVisible != visible)
    {
        m_data->meanVisible = visible;
        itemChanged();
    }
}

bool QwtPlotBoxChart::isMeanVisible() const
{
    return m_data->meanVisible;
}

void QwtPlotBoxChart::setOutlierJitter(double jitterWidth)
{
    m_data->outlierJitter = qwtMaxF(0.0, jitterWidth);
}

double QwtPlotBoxChart::outlierJitter() const
{
    return m_data->outlierJitter;
}
```

- [ ] **Step 3: Add sample/outlier data setters**

```cpp
void QwtPlotBoxChart::setSamples(const QVector<QwtBoxSample>& samples)
{
    setData(new QwtBoxChartData(samples));
}

void QwtPlotBoxChart::setSamples(QwtSeriesData<QwtBoxSample>* data)
{
    setData(data);
}

void QwtPlotBoxChart::setOutliers(const QVector<QwtBoxOutlierSample>& samples)
{
    delete m_outlierData;
    m_outlierData = new QwtBoxOutlierChartData(samples);
    itemChanged();
}

void QwtPlotBoxChart::setOutliers(QwtSeriesData<QwtBoxOutlierSample>* data)
{
    delete m_outlierData;
    m_outlierData = data;
    itemChanged();
}

const QwtSeriesData<QwtBoxOutlierSample>* QwtPlotBoxChart::outlierData() const
{
    return m_outlierData;
}
```

- [ ] **Step 4: Commit**

```bash
git add src/plot/qwt_plot_boxchart.cpp
git commit -m "feat: implement QwtPlotBoxChart core methods and setters"
```

---

## Task 10: Implement BoxChart - Drawing Methods

**Files:**
- Modify: `src/plot/qwt_plot_boxchart.cpp`

- [ ] **Step 1: Add boundingRect and scaledBoxWidth**

```cpp
QRectF QwtPlotBoxChart::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();
    
    // Include box extent padding
    if (rect.isValid())
    {
        const double padding = m_data->boxExtent * 0.5;
        if (m_data->orientation == Qt::Vertical)
        {
            rect.setLeft(rect.left() - padding);
            rect.setRight(rect.right() + padding);
        }
        else
        {
            rect.setTop(rect.top() - padding);
            rect.setBottom(rect.bottom() + padding);
        }
    }
    
    // Include outlier data if present
    if (m_outlierData && m_outlierData->size() > 0)
    {
        const QRectF outlierRect = m_outlierData->boundingRect();
        if (outlierRect.isValid())
        {
            rect = rect.united(outlierRect);
        }
    }
    
    return rect;
}

double QwtPlotBoxChart::scaledBoxWidth(
    const QwtScaleMap& posMap,
    const QwtScaleMap& valueMap,
    const QRectF& canvasRect) const
{
    Q_UNUSED(valueMap);
    Q_UNUSED(canvasRect);
    
    // Fixed width if min == max
    if (m_data->maxBoxWidth > 0.0 && m_data->minBoxWidth >= m_data->maxBoxWidth)
        return m_data->minBoxWidth;
    
    const double pos = posMap.transform(posMap.s1() + m_data->boxExtent);
    double width = qAbs(pos - posMap.p1());
    
    width = qwtMaxF(width, m_data->minBoxWidth);
    if (m_data->maxBoxWidth > 0.0)
        width = qwtMinF(width, m_data->maxBoxWidth);
    
    return width;
}
```

- [ ] **Step 2: Add drawSeries method**

```cpp
void QwtPlotBoxChart::drawSeries(QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to) const
{
    if (to < 0)
        to = dataSize() - 1;
    
    if (from < 0)
        from = 0;
    
    if (from > to || dataSize() == 0)
        return;
    
    painter->save();
    
    const Qt::Orientation orient = m_data->orientation;
    const QwtScaleMap* posMap = (orient == Qt::Vertical) ? &xMap : &yMap;
    const QwtScaleMap* valueMap = (orient == Qt::Vertical) ? &yMap : &xMap;
    
    const double boxWidth = scaledBoxWidth(xMap, yMap, canvasRect);
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    
    painter->setPen(m_data->pen);
    
    for (int i = from; i <= to; i++)
    {
        const QwtBoxSample& sample = this->sample(i);
        
        double posPixel = posMap->transform(sample.position);
        if (doAlign)
            posPixel = qRound(posPixel);
        
        // Draw in order: box -> whiskers -> median -> mean -> outliers
        
        if (m_data->boxStyle != NoBox)
        {
            painter->setBrush(m_data->brush);
            drawBox(painter, sample, orient, boxWidth, posPixel, *valueMap);
        }
        
        if (m_data->whiskerStyle != NoWhiskers)
        {
            drawWhiskers(painter, sample, orient, boxWidth, posPixel, *valueMap);
        }
        
        if (m_data->medianVisible)
        {
            drawMedian(painter, sample, orient, boxWidth, posPixel, *valueMap);
        }
        
        if (m_data->meanVisible && m_data->meanSymbol)
        {
            // Mean not stored in sample - would need extension or user calculation
            // Placeholder: draw at median position for now
            const double medianPixel = valueMap->transform(sample.median);
            if (doAlign)
                medianPixel = qRound(medianPixel);
            
            m_data->meanSymbol->drawSymbol(painter,
                (orient == Qt::Vertical) ? QPointF(posPixel, medianPixel)
                                         : QPointF(medianPixel, posPixel));
        }
    }
    
    // Draw outliers from outlier series
    if (m_outlierData && m_outlierData->size() > 0)
    {
        drawOutliers(painter, xMap, yMap, canvasRect, from, to);
    }
    
    painter->restore();
}
```

- [ ] **Step 3: Add drawBox method**

```cpp
void QwtPlotBoxChart::drawBox(QPainter* painter, const QwtBoxSample& sample,
    Qt::Orientation orient, double boxWidth, double posPixel,
    const QwtScaleMap& valueMap) const
{
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    
    const double q1Pixel = valueMap.transform(sample.q1);
    const double q3Pixel = valueMap.transform(sample.q3);
    const double medianPixel = valueMap.transform(sample.median);
    
    if (doAlign)
    {
        // Align to integers
        const double aligned = qRound(posPixel);
        posPixel = aligned;
    }
    
    const double halfWidth = boxWidth * 0.5;
    
    switch (m_data->boxStyle)
    {
        case Rect:
        {
            if (orient == Qt::Vertical)
            {
                QRectF rect(posPixel - halfWidth, q3Pixel,
                           boxWidth, q1Pixel - q3Pixel);
                QwtPainter::drawRect(painter, rect);
            }
            else
            {
                QRectF rect(q1Pixel, posPixel - halfWidth,
                           q3Pixel - q1Pixel, boxWidth);
                QwtPainter::drawRect(painter, rect);
            }
            break;
        }
        
        case Diamond:
        {
            QPolygonF poly(4);
            if (orient == Qt::Vertical)
            {
                poly[0] = QPointF(posPixel, q3Pixel);
                poly[1] = QPointF(posPixel + halfWidth, medianPixel);
                poly[2] = QPointF(posPixel, q1Pixel);
                poly[3] = QPointF(posPixel - halfWidth, medianPixel);
            }
            else
            {
                poly[0] = QPointF(q3Pixel, posPixel);
                poly[1] = QPointF(medianPixel, posPixel + halfWidth);
                poly[2] = QPointF(q1Pixel, posPixel);
                poly[3] = QPointF(medianPixel, posPixel - halfWidth);
            }
            QwtPainter::drawPolygon(painter, poly);
            break;
        }
        
        case Notch:
        {
            const double notchWidth = halfWidth * 0.25;
            const double notchOffset = (q3Pixel - q1Pixel) * 0.1; // Simplified notch
            
            QPolygonF poly(10);
            if (orient == Qt::Vertical)
            {
                poly[0] = QPointF(posPixel - halfWidth, q3Pixel);
                poly[1] = QPointF(posPixel - halfWidth, medianPixel - notchOffset);
                poly[2] = QPointF(posPixel - notchWidth, medianPixel);
                poly[3] = QPointF(posPixel - halfWidth, medianPixel + notchOffset);
                poly[4] = QPointF(posPixel - halfWidth, q1Pixel);
                poly[5] = QPointF(posPixel + halfWidth, q1Pixel);
                poly[6] = QPointF(posPixel + halfWidth, medianPixel + notchOffset);
                poly[7] = QPointF(posPixel + notchWidth, medianPixel);
                poly[8] = QPointF(posPixel + halfWidth, medianPixel - notchOffset);
                poly[9] = QPointF(posPixel + halfWidth, q3Pixel);
            }
            else
            {
                poly[0] = QPointF(q1Pixel, posPixel - halfWidth);
                poly[1] = QPointF(medianPixel - notchOffset, posPixel - halfWidth);
                poly[2] = QPointF(medianPixel, posPixel - notchWidth);
                poly[3] = QPointF(medianPixel + notchOffset, posPixel - halfWidth);
                poly[4] = QPointF(q3Pixel, posPixel - halfWidth);
                poly[5] = QPointF(q3Pixel, posPixel + halfWidth);
                poly[6] = QPointF(medianPixel + notchOffset, posPixel + halfWidth);
                poly[7] = QPointF(medianPixel, posPixel + notchWidth);
                poly[8] = QPointF(medianPixel - notchOffset, posPixel + halfWidth);
                poly[9] = QPointF(q1Pixel, posPixel + halfWidth);
            }
            QwtPainter::drawPolygon(painter, poly);
            break;
        }
        
        default:
            break;
    }
}
```

- [ ] **Step 4: Add drawWhiskers method**

```cpp
void QwtPlotBoxChart::drawWhiskers(QPainter* painter, const QwtBoxSample& sample,
    Qt::Orientation orient, double boxWidth, double posPixel,
    const QwtScaleMap& valueMap) const
{
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    
    const double whiskerLowerPixel = valueMap.transform(sample.whiskerLower);
    const double whiskerUpperPixel = valueMap.transform(sample.whiskerUpper);
    const double q1Pixel = valueMap.transform(sample.q1);
    const double q3Pixel = valueMap.transform(sample.q3);
    
    if (doAlign)
    {
        // Already aligned posPixel from caller
    }
    
    const double capWidth = boxWidth * 0.3;
    
    QPen whiskerPen = m_data->pen;
    whiskerPen.setCapStyle(Qt::FlatCap);
    painter->setPen(whiskerPen);
    
    switch (m_data->whiskerStyle)
    {
        case StandardWhisker:
        {
            if (orient == Qt::Vertical)
            {
                // Lower whisker line
                QwtPainter::drawLine(painter, posPixel, whiskerLowerPixel,
                                     posPixel, q1Pixel);
                // Lower cap
                QwtPainter::drawLine(painter, posPixel - capWidth * 0.5, whiskerLowerPixel,
                                     posPixel + capWidth * 0.5, whiskerLowerPixel);
                
                // Upper whisker line
                QwtPainter::drawLine(painter, posPixel, q3Pixel,
                                     posPixel, whiskerUpperPixel);
                // Upper cap
                QwtPainter::drawLine(painter, posPixel - capWidth * 0.5, whiskerUpperPixel,
                                     posPixel + capWidth * 0.5, whiskerUpperPixel);
            }
            else
            {
                // Lower whisker line (left side for horizontal)
                QwtPainter::drawLine(painter, whiskerLowerPixel, posPixel,
                                     q1Pixel, posPixel);
                // Lower cap
                QwtPainter::drawLine(painter, whiskerLowerPixel, posPixel - capWidth * 0.5,
                                     whiskerLowerPixel, posPixel + capWidth * 0.5);
                
                // Upper whisker line (right side)
                QwtPainter::drawLine(painter, q3Pixel, posPixel,
                                     whiskerUpperPixel, posPixel);
                // Upper cap
                QwtPainter::drawLine(painter, whiskerUpperPixel, posPixel - capWidth * 0.5,
                                     whiskerUpperPixel, posPixel + capWidth * 0.5);
            }
            break;
        }
        
        case MinMaxLine:
        {
            if (orient == Qt::Vertical)
            {
                QwtPainter::drawLine(painter, posPixel, whiskerLowerPixel,
                                     posPixel, whiskerUpperPixel);
            }
            else
            {
                QwtPainter::drawLine(painter, whiskerLowerPixel, posPixel,
                                     whiskerUpperPixel, posPixel);
            }
            break;
        }
        
        default:
            break;
    }
}
```

- [ ] **Step 5: Add drawMedian method**

```cpp
void QwtPlotBoxChart::drawMedian(QPainter* painter, const QwtBoxSample& sample,
    Qt::Orientation orient, double boxWidth, double posPixel,
    const QwtScaleMap& valueMap) const
{
    const double medianPixel = valueMap.transform(sample.median);
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    
    if (doAlign)
    {
        // PosPixel already aligned
    }
    
    const double halfWidth = boxWidth * 0.5;
    
    QPen medPen = m_data->medianPen;
    medPen.setCapStyle(Qt::FlatCap);
    painter->setPen(medPen);
    
    // Adjust width based on box style
    double lineHalfWidth = halfWidth;
    if (m_data->boxStyle == Diamond)
        lineHalfWidth *= 0.5;
    else if (m_data->boxStyle == Notch)
        lineHalfWidth *= 0.8;
    
    if (orient == Qt::Vertical)
    {
        QwtPainter::drawLine(painter, posPixel - lineHalfWidth, medianPixel,
                             posPixel + lineHalfWidth, medianPixel);
    }
    else
    {
        QwtPainter::drawLine(painter, medianPixel, posPixel - lineHalfWidth,
                             medianPixel, posPixel + lineHalfWidth);
    }
}
```

- [ ] **Step 6: Add drawOutliers method**

```cpp
void QwtPlotBoxChart::drawOutliers(QPainter* painter,
    const QwtScaleMap& posMap, const QwtScaleMap& valueMap,
    const QRectF& canvasRect, int from, int to) const
{
    if (!m_outlierData || m_outlierData->size() == 0)
        return;
    
    const QwtSymbol* symbol = m_data->outlierSymbol;
    if (!symbol)
    {
        // Default symbol
        static QwtSymbol defaultSymbol(QwtSymbol::XCross, QBrush(), QPen(Qt::black), QSize(8, 8));
        symbol = &defaultSymbol;
    }
    
    const Qt::Orientation orient = m_data->orientation;
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    const double jitter = m_data->outlierJitter;
    
    // Setup random generator for jitter
    QRandomGenerator* rng = nullptr;
    if (jitter > 0)
        rng = QRandomGenerator::global();
    
    for (size_t i = 0; i < m_outlierData->size(); ++i)
    {
        const QwtBoxOutlierSample& outlierSample = m_outlierData->sample(i);
        
        // Check if this outlier belongs to a box in our drawing range
        // (by position matching)
        double basePosPixel = posMap.transform(outlierSample.boxPosition);
        if (doAlign)
            basePosPixel = qRound(basePosPixel);
        
        for (int j = 0; j < outlierSample.count(); ++j)
        {
            double valuePixel = valueMap.transform(outlierSample.values[j]);
            if (doAlign)
                valuePixel = qRound(valuePixel);
            
            // Apply jitter
            double posPixel = basePosPixel;
            if (jitter > 0 && rng)
            {
                const double offset = rng->bounded(jitter) - jitter * 0.5;
                posPixel += offset;
            }
            
            QPointF point = (orient == Qt::Vertical)
                ? QPointF(posPixel, valuePixel)
                : QPointF(valuePixel, posPixel);
            
            // Clip check
            if (m_data->paintAttributes & ClipOutliers)
            {
                if (!canvasRect.contains(point))
                    continue;
            }
            
            symbol->drawSymbol(painter, point);
        }
    }
}
```

- [ ] **Step 7: Add legendIcon method**

```cpp
QwtGraphic QwtPlotBoxChart::legendIcon(int index, const QSizeF& size) const
{
    Q_UNUSED(index);
    
    QwtGraphic graphic;
    graphic.setDefaultSize(size);
    
    QPainter painter(&graphic);
    
    const QRectF rect(0, 0, size.width(), size.height());
    
    // Draw a mini boxplot in the legend icon
    const double centerX = rect.center().x();
    const double centerY = rect.center().y();
    const double boxHeight = rect.height() * 0.4;
    const double boxWidth = rect.width() * 0.3;
    
    painter.setPen(m_data->pen);
    painter.setBrush(m_data->brush);
    
    // Mini box
    if (m_data->boxStyle != NoBox)
    {
        QRectF boxRect(centerX - boxWidth * 0.5, centerY - boxHeight * 0.5,
                       boxWidth, boxHeight);
        painter.drawRect(boxRect);
    }
    
    // Mini whiskers
    if (m_data->whiskerStyle != NoWhiskers)
    {
        const double whiskerLen = rect.height() * 0.2;
        painter.drawLine(centerX, centerY - boxHeight * 0.5,
                         centerX, centerY - boxHeight * 0.5 - whiskerLen);
        painter.drawLine(centerX - boxWidth * 0.15, centerY - boxHeight * 0.5 - whiskerLen,
                         centerX + boxWidth * 0.15, centerY - boxHeight * 0.5 - whiskerLen);
        painter.drawLine(centerX, centerY + boxHeight * 0.5,
                         centerX, centerY + boxHeight * 0.5 + whiskerLen);
        painter.drawLine(centerX - boxWidth * 0.15, centerY + boxHeight * 0.5 + whiskerLen,
                         centerX + boxWidth * 0.15, centerY + boxHeight * 0.5 + whiskerLen);
    }
    
    // Mini median
    if (m_data->medianVisible)
    {
        painter.setPen(m_data->medianPen);
        painter.drawLine(centerX - boxWidth * 0.5, centerY,
                         centerX + boxWidth * 0.5, centerY);
    }
    
    painter.end();
    
    return graphic;
}
```

- [ ] **Step 8: Commit**

```bash
git add src/plot/qwt_plot_boxchart.cpp
git commit -m "feat: implement QwtPlotBoxChart drawing methods (box/whisker/median/outliers)"
```

---

## Task 11: Update CMakeLists.txt

**Files:**
- Modify: `src/plot/CMakeLists.txt`

- [ ] **Step 1: Find existing CMakeLists.txt and add new source files**

Add the new source files to the library target:

```cmake
# Add to qwt_plot library sources
qwt_box_statistics.h
qwt_box_statistics.cpp
qwt_plot_boxchart.h
qwt_plot_boxchart.cpp
```

- [ ] **Step 2: Commit**

```bash
git add src/plot/CMakeLists.txt
git commit -m "feat: add boxplot source files to CMakeLists.txt"
```

---

## Task 12: Create Class Include Mapping

**Files:**
- Create: `classincludes/QwtPlotBoxChart`

- [ ] **Step 1: Create class include file**

The file should just contain the path to the main header:

```
#include "qwt_plot_boxchart.h"
```

Or following existing pattern in the classincludes directory.

- [ ] **Step 2: Commit**

```bash
git add classincludes/QwtPlotBoxChart
git commit -m "feat: add QwtPlotBoxChart class include mapping"
```

---

## Task 13: Build and Verify

- [ ] **Step 1: Configure CMake**

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
```

- [ ] **Step 2: Build the project**

```bash
cmake --build build --target qwt_plot
```

- [ ] **Step 3: Verify compilation succeeds**

Expected: No compilation errors for the new boxplot files.

---

## Task 14: Create Basic Test Example

**Files:**
- Create: `tests/boxchart_test.cpp` (or appropriate test location)

- [ ] **Step 1: Create a simple test that instantiates QwtPlotBoxChart**

```cpp
#include "qwt_plot_boxchart.h"
#include "qwt_box_statistics.h"
#include <QApplication>
#include <QMainWindow>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    QwtPlot* plot = new QwtPlot(&window);
    window.setCentralWidget(plot);
    
    // Create box chart with pre-computed samples
    QwtPlotBoxChart* boxChart = new QwtPlotBoxChart("Test Data");
    
    QVector<QwtBoxSample> samples;
    samples << QwtBoxSample(1.0, 10, 20, 35, 50, 60);
    samples << QwtBoxSample(2.0, 15, 25, 40, 55, 70);
    samples << QwtBoxSample(3.0, 5, 15, 30, 45, 65);
    
    boxChart->setSamples(samples);
    boxChart->setBrush(QBrush(QColor(100, 150, 200, 150)));
    boxChart->setBoxStyle(QwtPlotBoxChart::Rect);
    boxChart->attach(plot);
    
    // Test calculator
    QVector<double> rawData = {1, 2, 3, 5, 8, 10, 15, 20, 25, 30, 100};
    QwtBoxSample calcSample;
    QwtBoxOutlierSample outliers;
    QwtBoxStatisticsCalculator::calculateFull(4.0, rawData, calcSample, outliers);
    
    QVector<QwtBoxSample> calcSamples;
    calcSamples << calcSample;
    
    QwtPlotBoxChart* calcChart = new QwtPlotBoxChart("Calculated");
    calcChart->setSamples(calcSamples);
    calcChart->setOutliers(QVector<QwtBoxOutlierSample>() << outliers);
    calcChart->setBoxStyle(QwtPlotBoxChart::Notch);
    calcChart->attach(plot);
    
    window.resize(600, 400);
    window.show();
    
    return app.exec();
}
```

- [ ] **Step 2: Build and run test**

Verify the boxplots display correctly with boxes, whiskers, median lines, and outliers.

---

## Summary

This implementation plan covers:

1. **Data structures**: QwtStatisticalSample, QwtBoxSample, QwtBoxOutlierSample
2. **Series data classes**: QwtBoxChartData, QwtBoxOutlierChartData
3. **Calculator helper**: QwtBoxStatisticsCalculator with Tukey/percentile/SD/SE methods
4. **Main plot item**: QwtPlotBoxChart with full rendering pipeline
5. **Build integration**: CMakeLists.txt, classincludes
6. **Testing**: Basic example to verify functionality

Each task produces self-contained, testable changes following TDD principles where applicable.