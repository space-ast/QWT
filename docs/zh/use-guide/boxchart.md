# 箱线图 (Box Chart) 使用指南

`QwtPlotBoxChart` 是 Qwt 提供的箱线图（Box-and-Whisker Plot）绘制类，用于展示数据的统计分布特征。箱线图能够直观地显示数据的中位数、四分位数、异常值等统计信息，是数据分析中常用的可视化工具。

## 主要功能特性

### 1. 数据输入方式灵活

- **预计算数据**：直接提供统计好的五数概括（最小值、Q1、中位数、Q3、最大值）
- **原始数据计算**：提供原始数据数组，自动计算统计量并提取异常值

### 2. 多种显示样式

- **箱体样式**：矩形、菱形、缺口形
- **须须样式**：标准T型、简单线条
- **方向切换**：支持垂直和水平两种显示方向

### 3. 异常值处理

- 自动检测并标记异常值
- 支持异常值符号自定义
- 支持异常值抖动（jitter）避免重叠

### 4. 样式定制

- 箱体填充色和边框
- 中位数线样式
- 须须线样式
- 异常值符号

## 基本概念

### 箱线图组成

```
    │                    ◆ 异常值
    │         ┌──┬──┐
    │         │  │  │ ← 上须（Q3 + 1.5×IQR）
    │         │  ┼  │ ← 中位数
    │         │  │  │
    │    ─────┴──┴──┴───── ← 下须（Q1 - 1.5×IQR）
    │                    ◆ 异常值
    └────────────────────────
              箱体（Q1-Q3）
```

### 相关数据结构

#### QwtBoxSample

表示一个箱线图样本，包含以下字段：

```cpp
class QwtBoxSample {
    double position;      // 在位置轴上的坐标（x轴或y轴）
    double whiskerLower;  // 下须位置
    double q1;            // 第一四分位数（25%）
    double median;        // 中位数（50%）
    double q3;            // 第三四分位数（75%）
    double whiskerUpper;  // 上须位置
    int outlierCount;     // 异常值数量
};
```

#### QwtBoxOutlierSample

表示一个箱子的异常值集合：

```cpp
class QwtBoxOutlierSample {
    double boxPosition;        // 对应箱子的位置
    QVector<double> values;    // 所有异常值
};
```

## 使用方法

### 1. 使用预计算数据

当已有统计好的数据时，直接创建 `QwtBoxSample`：

```cpp
#include <QwtPlotBoxChart>
#include <QwtBoxSample>

// 创建箱线图对象
QwtPlotBoxChart* boxChart = new QwtPlotBoxChart("Box Chart Title");
boxChart->attach(plot);

// 准备数据
QVector<QwtBoxSample> samples;
samples << QwtBoxSample(1.0, 10.0, 20.0, 35.0, 50.0, 60.0);  // 位置1
samples << QwtBoxSample(2.0, 5.0, 15.0, 30.0, 45.0, 55.0);   // 位置2
samples << QwtBoxSample(3.0, 8.0, 25.0, 40.0, 55.0, 70.0);   // 位置3

// 设置数据
boxChart->setSamples(samples);

// 设置样式
boxChart->setBrush(QColor(100, 150, 200, 150));  // 半透明蓝色填充
boxChart->setPen(QPen(Qt::darkBlue, 2.0));       // 深蓝色边框
boxChart->setBoxExtent(0.35);                    // 箱体宽度
```

### 2. 从原始数据计算

当只有原始数据时，使用 `QwtBoxStatisticsCalculator` 自动计算：

```cpp
#include <qwt_box_statistics.h>

// 准备原始数据
QVector<double> rawData;
for (int i = 0; i < 100; i++) {
    rawData << 50.0 + (rand() % 50) - 25.0;  // 随机数据
}
rawData << 5.0;   // 添加异常值
rawData << 100.0; // 添加异常值

// 计算统计量
QwtBoxSample sample;
QwtBoxOutlierSample outlierSample;
QwtBoxStatisticsCalculator::calculateFull(
    1.5,           // 箱子位置
    rawData,       // 原始数据
    sample,        // 输出：箱子统计量
    outlierSample, // 输出：异常值
    QwtBoxStatisticsCalculator::Tukey,  // 计算方法：Tukey
    1.5            // 系数：1.5×IQR
);

// 设置数据
QVector<QwtBoxSample> samples;
samples << sample;
boxChart->setSamples(samples);

// 设置异常值
QVector<QwtBoxOutlierSample> outliers;
outliers << outlierSample;
boxChart->setOutliers(outliers);
```

### 3. 设置异常值样式

```cpp
// 创建异常值符号
QwtSymbol* outlierSymbol = new QwtSymbol(QwtSymbol::Diamond);
outlierSymbol->setSize(8, 8);
outlierSymbol->setBrush(Qt::red);
outlierSymbol->setPen(QPen(Qt::darkRed, 1.0));

// 设置符号和抖动
boxChart->setOutlierSymbol(outlierSymbol);
boxChart->setOutlierJitter(0.1);  // 抖动范围
```

### 4. 切换显示方向

```cpp
// 垂直方向（默认）
boxChart->setOrientation(Qt::Vertical);

// 水平方向
boxChart->setOrientation(Qt::Horizontal);
```

### 5. 设置箱体样式

```cpp
// 矩形箱体（默认）
boxChart->setBoxStyle(QwtPlotBoxChart::Rect);

// 菱形箱体
boxChart->setBoxStyle(QwtPlotBoxChart::Diamond);

// 缺口箱体（显示中位数置信区间）
boxChart->setBoxStyle(QwtPlotBoxChart::Notch);
```

### 6. 设置须须样式

```cpp
// 标准T型须须（默认）
boxChart->setWhiskerStyle(QwtPlotBoxChart::StandardWhisker);

// 简单线条
boxChart->setWhiskerStyle(QwtPlotBoxChart::MinMaxLine);

// 不显示须须
boxChart->setWhiskerStyle(QwtPlotBoxChart::NoWhiskers);
```

## 样式配置详解

### 箱体样式选择建议

| 样式 | 适用场景 |
|------|---------|
| `Rect` | 标准箱线图，最常用 |
| `Diamond` | 强调中位数，适合对比多个组 |
| `Notch` | 显示中位数置信区间，适合统计推断 |

### 须须计算方法

`QwtBoxStatisticsCalculator` 提供四种计算方法：

| 方法 | 说明 | 适用场景 |
|------|------|---------|
| `Tukey` | 1.5×IQR（默认） | 标准箱线图，识别异常值 |
| `Percentile` | 指定百分位数 | 自定义范围 |
| `MinMax` | 最小值到最大值 | 显示完整数据范围 |
| `StandardDeviation` | 均值±n×标准差 | 正态分布数据 |
| `StandardError` | 均值±n×标准误 | 统计推断 |

## 完整示例

```cpp
#include <QwtPlot>
#include <QwtPlotBoxChart>
#include <QwtBoxSample>
#include <qwt_box_statistics.h>
#include <QwtSymbol>
#include <QwtLegend>

class BoxChartDemo : public QwtPlot {
public:
    BoxChartDemo(QWidget* parent = nullptr) : QwtPlot(parent) {
        setTitle("箱线图示例");
        setAxisTitle(QwtAxis::XBottom, "样本组");
        setAxisTitle(QwtAxis::YLeft, "数值");
        
        insertLegend(new QwtLegend());
        
        // 第一组：预计算数据
        QwtPlotBoxChart* chart1 = new QwtPlotBoxChart("手动计算");
        chart1->attach(this);
        
        QVector<QwtBoxSample> samples1;
        samples1 << QwtBoxSample(1.0, 10, 20, 35, 50, 60);
        samples1 << QwtBoxSample(2.0, 15, 25, 40, 55, 70);
        chart1->setSamples(samples1);
        chart1->setBrush(QColor(100, 150, 200, 150));
        
        // 第二组：自动计算
        QwtPlotBoxChart* chart2 = new QwtPlotBoxChart("自动计算");
        chart2->attach(this);
        
        QVector<double> rawData;
        for (int i = 0; i < 100; i++)
            rawData << 50.0 + (rand() % 50) - 25.0;
        rawData << 5.0 << 100.0;  // 异常值
        
        QwtBoxSample sample;
        QwtBoxOutlierSample outlier;
        QwtBoxStatisticsCalculator::calculateFull(1.5, rawData, sample, outlier);
        
        chart2->setSamples(QVector<QwtBoxSample>() << sample);
        chart2->setOutliers(QVector<QwtBoxOutlierSample>() << outlier);
        chart2->setBrush(QColor(200, 100, 100, 150));
        
        // 设置异常值符号
        QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Diamond);
        symbol->setSize(8, 8);
        symbol->setBrush(Qt::red);
        chart2->setOutlierSymbol(symbol);
        chart2->setOutlierJitter(0.1);
    }
};
```

## 注意事项

1. **数据顺序**：`QwtBoxSample` 的构造函数参数顺序为：
   `position, whiskerLower, q1, median, q3, whiskerUpper`

2. **异常值单独存储**：异常值通过 `setOutliers()` 单独设置，不在 `QwtBoxSample` 中

3. **方向切换**：切换方向后建议调用 `replot()` 刷新坐标轴

4. **抖动范围**：`setOutlierJitter()` 的参数是相对值，建议范围 0.0~0.2

5. **箱体宽度**：`setBoxExtent()` 设置的是相对于坐标轴范围的宽度比例

## 参考示例

完整示例代码请参考：`examples/2D/boxchart/`

该示例展示了：
- 手动设置统计数据
- 自动计算统计数据
- 多种箱体样式切换
- 垂直/水平方向切换
- 异常值显示和样式设置
