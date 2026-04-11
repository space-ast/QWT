# Qwt Boxplot Component Design Specification

> **For agentic workers:** This spec defines the complete design for a boxplot (box-and-whisker) visualization component for the Qwt plotting library.

**Goal:** Implement a feature-complete boxplot component (`QwtPlotBoxChart`) supporting whisker calculation, box body rendering, outlier detection/display, horizontal/vertical orientation, and comprehensive style customization.

**Architecture:** Three-layer design: (1) Data structures in `qwt_samples.h` for samples, (2) Helper class `QwtBoxStatisticsCalculator` for statistical computation, (3) Main plot item `QwtPlotBoxChart` for rendering. Outliers stored in separate data series for independent styling.

**Tech Stack:** Qt5/Qt6, Qwt plotting framework, C++11/17, CMake build system

---

## 1. Overview

This document specifies the implementation of a boxplot component for Qwt 7.x, following matplotlib's boxplot functionality while adhering to Qwt's existing architectural patterns.

### 1.1 Reference Implementations Analyzed

- **QwtPlotTradingCurve**: OHLC data structure, symbol sizing, orientation support
- **QwtPlotIntervalCurve**: Error bar rendering, interval symbols
- **QtiPlot BoxCurve**: BoxStyle variants, whisker range options, outlier symbols

### 1.2 Functional Requirements

| Feature | Description |
|---------|-------------|
| Whisker calculation | Tukey (1.5×IQR) default, configurable percentile/SD/SE methods |
| Box body display | Q1-Q3 range with optional notch |
| Median line | Visible by default, can be hidden, customizable pen |
| Mean marker | Optional symbol at mean position |
| Outlier rendering | Independent symbol, separate data series |
| Orientation | Horizontal and vertical via property switch |
| Multi-box layout | Adjustable spacing, auto-optimal default |
| Style customization | Pen, brush, symbol, width controls |

---

## 2. Data Structures

### 2.1 Base Class: `QwtStatisticalSample`

Located in `src/plot/qwt_samples.h`.

```cpp
class QWT_EXPORT QwtStatisticalSample
{
public:
    QwtStatisticalSample(double position = 0.0);
    
    //! Position on the "time" axis (x for vertical, y for horizontal)
    double position;
    
    //! Lower bound of the statistical range
    double lower;
    
    //! Upper bound of the statistical range
    double upper;
    
    //! Central reference value
    double center;
};
```

**Design Notes:**
- Generic field names allow reuse across OHLC and boxplot
- `QwtBoxSample` inherits from this base
- `QwtOHLCSample` remains unchanged for backward compatibility (pragmatic compromise)

### 2.2 Box Sample: `QwtBoxSample`

Located in `src/plot/qwt_samples.h`.

```cpp
class QWT_EXPORT QwtBoxSample : public QwtStatisticalSample
{
public:
    QwtBoxSample(double position = 0.0);
    QwtBoxSample(double position, double whiskerLower, double q1,
                 double median, double q3, double whiskerUpper);
    
    bool isValid() const;
    QwtInterval boundingInterval() const;  // Full whisker range
    QwtInterval boxInterval() const;       // Q1-Q3 only
    
    double whiskerLower;  // Lower whisker endpoint
    double q1;            // First quartile (25th percentile)
    double median;        // Median (also stored in inherited 'center')
    double q3;            // Third quartile (75th percentile)
    double whiskerUpper;  // Upper whisker endpoint
    int outlierCount;     // Count only; actual outliers in separate series
};
```

**Validation Rules (`isValid()`):**
- `whiskerLower <= q1 <= median <= q3 <= whiskerUpper`
- All values are finite (not NaN/Inf)

### 2.3 Outlier Sample: `QwtBoxOutlierSample`

Located in `src/plot/qwt_samples.h`.

```cpp
class QWT_EXPORT QwtBoxOutlierSample
{
public:
    QwtBoxOutlierSample(double boxPosition = 0.0);
    QwtBoxOutlierSample(double boxPosition, const QVector<double>& values);
    QwtBoxOutlierSample(double boxPosition, QVector<double>&& values);
    
    bool isEmpty() const { return values.isEmpty(); }
    int count() const { return values.size(); }
    
    double boxPosition;        // Position matching parent QwtBoxSample
    QVector<double> values;    // All outlier values for this box
};
```

**Design Rationale:**
- One sample per box position with all outliers grouped
- Aligns with `QwtSetSample` pattern (one sample = multiple values)
- Simplifies rendering iteration

---

## 3. Statistical Calculator Class

### 3.1 `QwtBoxStatisticsCalculator`

Located in `src/plot/qwt_box_statistics.h` and `.cpp`.

```cpp
class QWT_EXPORT QwtBoxStatisticsCalculator
{
public:
    enum WhiskerMethod
    {
        Tukey,              // 1.5×IQR from Q1/Q3 (default)
        Percentile,         // Custom percentile range
        MinMax,             // Actual data min/max
        StandardDeviation,  // Mean ± coeff×SD
        StandardError       // Mean ± coeff×SE
    };
    
    QwtBoxStatisticsCalculator();
    
    void setWhiskerMethod(WhiskerMethod method);
    WhiskerMethod whiskerMethod() const;
    
    void setWhiskerCoefficient(double coeff);
    double whiskerCoefficient() const;
    
    // Compute from sorted data (user pre-sorted)
    static QwtBoxSample calculate(
        double position,
        const QVector<double>& sortedData,
        WhiskerMethod method = Tukey,
        double coefficient = 1.5);
    
    // Compute from unsorted data (sorts internally)
    static QwtBoxSample calculateFromRaw(
        double position,
        const QVector<double>& rawData,
        WhiskerMethod method = Tukey,
        double coefficient = 1.5);
    
    // Extract outliers given box sample and sorted data
    static QVector<double> extractOutliers(
        const QwtBoxSample& sample,
        const QVector<double>& sortedData);
    
    // One-call convenience: sample + outliers
    static void calculateFull(
        double position,
        const QVector<double>& rawData,
        QwtBoxSample& sample,
        QwtBoxOutlierSample& outliers,
        WhiskerMethod method = Tukey,
        double coefficient = 1.5);
    
private:
    static double quantile(const QVector<double>& sorted, double p);
    static double median(const QVector<double>& sorted);
    static double iqr(const QVector<double>& sorted);
    
    WhiskerMethod m_method;
    double m_coefficient;
};
```

**Default Values:**
- Method: `Tukey`
- Coefficient: `1.5`

**Tukey Method Implementation:**
```
IQR = Q3 - Q1
whiskerLower = max(min(data), Q1 - 1.5×IQR)
whiskerUpper = min(max(data), Q3 + 1.5×IQR)
outliers = values < whiskerLower OR values > whiskerUpper
```

---

## 4. Main Plot Item Class

### 4.1 `QwtPlotBoxChart`

Located in `src/plot/qwt_plot_boxchart.h` and `.cpp`.

#### Inheritance

```cpp
class QWT_EXPORT QwtPlotBoxChart
    : public QwtPlotSeriesItem
    , public QwtSeriesStore<QwtBoxSample>
```

#### Enums

```cpp
enum BoxStyle
{
    NoBox,      // Only whiskers, no body
    Rect,       // Traditional rectangle (Q1-Q3)
    Diamond,    // Diamond shape
    Notch       // Rectangle with notch at median
};

enum WhiskerStyle
{
    NoWhiskers,        // No whisker lines
    StandardWhisker,   // T-bar with horizontal caps (default)
    MinMaxLine         // Simple vertical line
};

enum PaintAttribute
{
    ClipBoxes = 0x01,     // Clip before painting
    ClipOutliers = 0x02,  // Clip outlier symbols
    ImageBuffer = 0x04    // Image buffer for many boxes
};
Q_DECLARE_FLAGS(PaintAttributes, PaintAttribute)
```

#### Public API

```cpp
// Construction
explicit QwtPlotBoxChart(const QString& title = QString());
explicit QwtPlotBoxChart(const QwtText& title);
virtual ~QwtPlotBoxChart();

// RTTI
virtual int rtti() const override;  // Returns Rtti_PlotBoxChart

// Paint attributes
void setPaintAttribute(PaintAttribute, bool on = true);
bool testPaintAttribute(PaintAttribute) const;

// Data
void setSamples(const QVector<QwtBoxSample>&);
void setSamples(QwtSeriesData<QwtBoxSample>*);
void setOutliers(const QVector<QwtBoxOutlierSample>&);
void setOutliers(QwtSeriesData<QwtBoxOutlierSample>*);

// Style
void setBoxStyle(BoxStyle);
BoxStyle boxStyle() const;
void setWhiskerStyle(WhiskerStyle);
WhiskerStyle whiskerStyle() const;

// Orientation
void setOrientation(Qt::Orientation);  // Vertical or Horizontal
Qt::Orientation orientation() const;

// Box width control (follows TradingCurve pattern)
void setBoxExtent(double extent);      // Width in scale coordinates
double boxExtent() const;
void setMinBoxWidth(double pixels);
double minBoxWidth() const;
void setMaxBoxWidth(double pixels);
double maxBoxWidth() const;

// Pen/Brush
void setPen(const QPen&);              // Box outline and whiskers
const QPen& pen() const;
void setBrush(const QBrush&);          // Box body fill
const QBrush& brush() const;
void setMedianPen(const QPen&);        // Median line (defaults to pen())
QPen medianPen() const;

// Symbols
void setOutlierSymbol(const QwtSymbol*);
const QwtSymbol* outlierSymbol() const;
void setMeanSymbol(const QwtSymbol*);
const QwtSymbol* meanSymbol() const;

// Visibility
void setMedianVisible(bool);
bool isMedianVisible() const;
void setMeanVisible(bool);
bool isMeanVisible() const;

// Outlier jitter (for overlapping outliers)
void setOutlierJitter(double width);
double outlierJitter() const;

// Core methods
virtual void drawSeries(...) override;
virtual QRectF boundingRect() const override;
virtual QwtGraphic legendIcon(...) const override;
```

#### Protected Methods

```cpp
void init();
double scaledBoxWidth(const QwtScaleMap&, const QwtScaleMap&, const QRectF&) const;

virtual void drawBox(QPainter*, const QwtBoxSample&, Qt::Orientation, 
                     double boxWidth, double posCoord, const QwtScaleMap&) const;
virtual void drawWhiskers(QPainter*, const QwtBoxSample&, Qt::Orientation,
                          double boxWidth, double posCoord, const QwtScaleMap&) const;
virtual void drawMedian(QPainter*, const QwtBoxSample&, Qt::Orientation,
                        double boxWidth, double posCoord, const QwtScaleMap&) const;
virtual void drawOutliers(QPainter*, const QwtScaleMap&, const QwtScaleMap&,
                          const QRectF&, int from, int to) const;
```

#### Private Data

```cpp
class PrivateData;
PrivateData* m_data;
QwtSeriesData<QwtBoxOutlierSample>* m_outlierData;
```

#### Default Values

| Property | Default |
|----------|---------|
| BoxStyle | `Rect` |
| WhiskerStyle | `StandardWhisker` |
| Orientation | `Qt::Vertical` |
| BoxExtent | `0.6` (scale units) |
| MinBoxWidth | `2.0` (pixels) |
| MaxBoxWidth | `-1.0` (unlimited) |
| Pen | `QPen(Qt::black, 1.0)` |
| Brush | `QBrush(Qt::NoBrush)` |
| MedianVisible | `true` |
| MeanVisible | `false` |
| OutlierJitter | `0.0` |
| OutlierSymbol | `QwtSymbol(QwtSymbol::XCross)` |

---

## 5. Data Series Classes

### 5.1 `QwtBoxChartData`

Located in `src/plot/qwt_series_data.h` (or inline in header).

```cpp
class QWT_EXPORT QwtBoxChartData : public QwtArraySeriesData<QwtBoxSample>
{
public:
    QwtBoxChartData();
    QwtBoxChartData(const QVector<QwtBoxSample>& samples);
    QwtBoxChartData(QVector<QwtBoxSample>&& samples);
    
    virtual QRectF boundingRect() const override;
};
```

### 5.2 `QwtBoxOutlierChartData`

```cpp
class QWT_EXPORT QwtBoxOutlierChartData : public QwtArraySeriesData<QwtBoxOutlierSample>
{
public:
    QwtBoxOutlierChartData();
    QwtBoxChartData(const QVector<QwtBoxOutlierSample>& samples);
    QwtBoxChartData(QVector<QwtBoxOutlierSample>&& samples);
    
    virtual QRectF boundingRect() const override;
    int totalOutlierCount() const;
};
```

---

## 6. Rendering Details

### 6.1 Coordinate Transformation

For `Qt::Vertical` orientation:
- Position → x-coordinate
- whiskerLower/q1/median/q3/whiskerUpper → y-coordinates

For `Qt::Horizontal` orientation:
- Position → y-coordinate
- whiskerLower/q1/median/q3/whiskerUpper → x-coordinates

### 6.2 Box Width Calculation

Follows `QwtPlotTradingCurve::scaledSymbolWidth()` pattern:

```cpp
double scaledBoxWidth(const QwtScaleMap& posMap, ...) const
{
    // Fixed width if min == max
    if (maxBoxWidth > 0 && minBoxWidth >= maxBoxWidth)
        return minBoxWidth;
    
    // Scale extent to pixels
    double pos = posMap.transform(posMap.s1() + boxExtent);
    double width = qAbs(pos - posMap.p1());
    
    // Apply min/max constraints
    width = qwtMaxF(width, minBoxWidth);
    if (maxBoxWidth > 0)
        width = qwtMinF(width, maxBoxWidth);
    
    return width;
}
```

### 6.3 Draw Order

Within each box (from back to front):
1. Box body (filled rectangle/polygon)
2. Whisker lines
3. Median line
4. Mean symbol (if visible)
5. Outlier symbols (from outlier series)

### 6.4 Notch Calculation

For `BoxStyle::Notch`, notch width is computed as:
```
notchWidth = boxWidth / 4  (at median level)
```

Notch polygon vertices (vertical orientation):
```
(left, q3) → (left, median+notchOffset) → (center, median) → 
(right, median+notchOffset) → (right, q3) → ... (mirror for lower)
```

### 6.5 Outlier Jittering

When `outlierJitter > 0`, outlier x-positions are randomly offset:
```
offset = random in [-jitter/2, +jitter/2]
outlierX = boxCenterX + offset
```

---

## 7. RTTI Integration

Add to `QwtPlotItem::RttiValues` in `src/plot/qwt_plot_item.h`:

```cpp
enum RttiValues
{
    // ... existing values ...
    Rtti_PlotBoxChart = ???  // Next available value
};
```

Check existing RTTI values before assigning.

---

## 8. File Organization

### New Files

| File | Purpose |
|------|---------|
| `src/plot/qwt_plot_boxchart.h` | Main plot item header |
| `src/plot/qwt_plot_boxchart.cpp` | Main plot item implementation |
| `src/plot/qwt_box_statistics.h` | Calculator header |
| `src/plot/qwt_box_statistics.cpp` | Calculator implementation |

### Modified Files

| File | Modification |
|------|--------------|
| `src/plot/qwt_samples.h` | Add `QwtStatisticalSample`, `QwtBoxSample`, `QwtBoxOutlierSample` |
| `src/plot/qwt_series_data.h` | Add `QwtBoxChartData`, `QwtBoxOutlierChartData` |
| `src/plot/qwt_plot_seriesitem.h` | Verify/add RTTI constant |
| `src/plot/CMakeLists.txt` | Add new source files |
| `classincludes/QwtPlotBoxChart` | Class include mapping |

**Note:** `src-amalgamate/` files are auto-merged by tool, no manual changes needed.

---

## 9. Performance Considerations

### 9.1 Paint Attributes

| Attribute | Use Case |
|-----------|----------|
| `ClipBoxes` | Deep zoom scenarios with boxes outside canvas |
| `ClipOutliers` | Many outliers, most outside visible area |
| `ImageBuffer` | >100 boxes on screen, reduces QPainter overhead |

### 9.2 Bounding Rect Optimization

`boundingRect()` must include:
- All whisker endpoints (whiskerLower, whiskerUpper)
- All outlier values (if outlier data present)
- Padding for box width (half extent on each side)

---

## 10. Example Usage

```cpp
// Create box chart
QwtPlotBoxChart* boxChart = new QwtPlotBoxChart("Data Distribution");

// Option A: Pre-computed samples
QVector<QwtBoxSample> samples;
samples << QwtBoxSample(1.0, 10, 20, 35, 50, 60);
samples << QwtBoxSample(2.0, 5, 15, 40, 55, 70);
boxChart->setSamples(samples);

// Option B: Compute from raw data
QVector<double> rawData = {1, 2, 3, 5, 8, 10, 15, 20, 25, 30};
QwtBoxSample sample;
QwtBoxOutlierSample outliers;
QwtBoxStatisticsCalculator::calculateFull(1.0, rawData, sample, outliers);

QVector<QwtBoxSample> boxSamples;
boxSamples << sample;
boxChart->setSamples(boxSamples);

QVector<QwtBoxOutlierSample> outlierSamples;
outlierSamples << outliers;
boxChart->setOutliers(outlierSamples);

// Styling
boxChart->setBoxStyle(QwtPlotBoxChart::Notch);
boxChart->setBrush(QBrush(QColor(100, 150, 200, 150)));
boxChart->setMedianPen(QPen(Qt::red, 2));
boxChart->setOrientation(Qt::Horizontal);
boxChart->setBoxExtent(0.8);

// Attach to plot
boxChart->attach(plot);
plot->replot();
```

---

## 11. Clarified Design Decisions Summary

| Question | Decision |
|----------|----------|
| Data format | New `QwtBoxSample` + helper methods for conversion |
| Whisker calculation | Tukey default, configurable via `QwtBoxStatisticsCalculator` |
| Direction | Both orientations via `setOrientation()` |
| Multi-box spacing | Width controls: extent + min/max in pixels |
| Style system | Independent design (not inheriting curve styles) |
| Performance | PaintAttributes matching QwtPlotCurve pattern |
| Base class | Pragmatic: base class for box only, OHLC unchanged |
| Outlier storage | Separate `QwtBoxOutlierSample` series |
| Calculator placement | Helper class + curve convenience methods (Option C) |

---

## 12. Testing Considerations

Test coverage should include:
- Sample validation (`isValid()` with edge cases)
- Whisker calculation methods (Tukey, percentile, SD, SE)
- Outlier extraction correctness
- Orientation switching rendering
- Bounding rect with outliers
- Box width scaling under different zoom levels
- Empty data handling
- Legend icon generation

---

**Spec Version:** 1.0  
**Created:** 2026-04-11  
**Author:** Design session