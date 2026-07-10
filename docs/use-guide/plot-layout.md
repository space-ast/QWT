# QwtPlot Layout Management

`QwtPlotLayout` is the layout engine for QwtPlot, responsible for organizing the positions and sizes of internal components within the plot window, including the title, legend, axes, and canvas area.

## Key Features

**Features**

- **Automatic layout calculation**: Automatically calculates reasonable space allocation based on component content
- **Flexible spacing control**: Supports custom canvas margins and component spacing
- **Canvas alignment options**: Choose to align the canvas with axis ticks or keep it independent
- **Legend position configuration**: Supports placing the legend at four boundary positions with configurable ratio
- **Export optimization options**: Can ignore scrollbars, frames, and other elements during export

## Basic Concepts

### Plot Window Structure

A QwtPlot window consists of the following components:

```text
+---------------------------------------------+
|                  Title Area                  |
+---------------------------------------------+
|  +-----------+---------------+-----------+  |
|  |           |               |           |  |
|  | Left      |    Canvas     | Right     |  |
|  | Legend    |    Area       | Legend    |  |
|  | (optional)|               | (optional)|  |
|  |           |               |           |  |
|  +-----------+---------------+-----------+  |
|  | Left Axis |  Bottom Axis  | Right Axis|  |
|  +-----------+---------------+-----------+  |
|                  Footer Area                 |
+---------------------------------------------+
```

### Getting Component Regions

QwtPlotLayout provides methods to retrieve the layout rectangles of each component:

| Method | Description |
|--------|-------------|
| `titleRect()` | Get the title area rectangle |
| `footerRect()` | Get the footer area rectangle |
| `legendRect()` | Get the legend area rectangle |
| `scaleRect(QwtAxisId)` | Get the specified axis area rectangle |
| `canvasRect()` | Get the canvas area rectangle |

## Usage

### Setting Canvas Margins

Canvas margin is the whitespace between the canvas and the axes:

```cpp
QwtPlot* plot = new QwtPlot();

// Get the layout object (already created internally by QwtPlot)
QwtPlotLayout* layout = plot->plotLayout();

// Set canvas margin to 10 pixels for all axes
layout->setCanvasMargin(10);

// Set canvas margin only for the left Y-axis
layout->setCanvasMargin(15, QwtAxis::YLeft);

// Get canvas margin for a specific axis
int margin = layout->canvasMargin(QwtAxis::XBottom);
```

!!! tip "Margin Purpose"
    Canvas margins are primarily used to control the distance between axis tick labels and the canvas edge, preventing tick labels from being clipped.

### Setting Component Spacing

Spacing controls the distance between the title, canvas, legend, and footer:

```cpp
// Set component spacing to 5 pixels
layout->setSpacing(5);

// Get current spacing
int spacing = layout->spacing();
```

### Canvas and Axis Alignment

By default, canvas boundaries align with axis tick marks, ensuring ticks fall exactly at the canvas edge. You can also disable alignment to keep the canvas independent:

```cpp
// Align canvas with all axes (default behavior)
layout->setAlignCanvasToScales(true);

// Align canvas only with the bottom X-axis
layout->setAlignCanvasToScale(QwtAxis::XBottom, true);

// Disable canvas alignment with the left Y-axis
layout->setAlignCanvasToScale(QwtAxis::YLeft, false);

// Check alignment status for a specific axis
bool aligned = layout->alignCanvasToScale(QwtAxis::YRight);
```

!!! info "Alignment Effect Description"
    - **Aligned mode**: Canvas boundaries precisely align with axis tick marks; tick labels may extend beyond the canvas
    - **Non-aligned mode**: Canvas maintains a fixed size; axis areas are calculated independently; tick labels are fully displayed

### Legend Position Configuration

QwtPlotLayout supports placing the legend at four boundary positions:

```cpp
// Set legend on the right side, occupying 20%
layout->setLegendPosition(QwtPlot::RightLegend, 0.2);

// Set legend at the bottom
layout->setLegendPosition(QwtPlot::BottomLegend);

// Set legend position only (without changing ratio)
layout->setLegendPosition(QwtPlot::LeftLegend);

// Set legend ratio (percentage of boundary space)
layout->setLegendRatio(0.15);  // Legend occupies 15%

// Get current legend position and ratio
QwtPlot::LegendPosition pos = layout->legendPosition();
double ratio = layout->legendRatio();
```

Legend position options:

| Position | Enum Value | Description |
|----------|------------|-------------|
| Left | `QwtPlot::LeftLegend` | Legend to the left of the YLeft axis |
| Right | `QwtPlot::RightLegend` | Legend to the right of the YRight axis |
| Bottom | `QwtPlot::BottomLegend` | Legend below the footer |
| Top | `QwtPlot::TopLegend` | Legend above the title |

### Layout Activation and Refresh

The layout needs to be recalculated when the QwtPlot size changes or component content changes:

```cpp
// Manually activate layout (usually called automatically by QwtPlot)
layout->activate(plot, QRectF(0, 0, 600, 400));

// Invalidate layout, forcing recalculation on next refresh
layout->invalidate();
```

### Export Layout Options

Special options can be used to ignore certain components when exporting a plot:

```cpp
// Ignore title and legend during export
QwtPlotLayout::Options options;
options |= QwtPlotLayout::IgnoreTitle;
options |= QwtPlotLayout::IgnoreLegend;

layout->activate(plot, exportRect, options);
```

| Option | Description |
|--------|-------------|
| `AlignScales` | Align axes (unused) |
| `IgnoreScrollbars` | Ignore scrollbar dimensions |
| `IgnoreFrames` | Ignore all frame borders |
| `IgnoreLegend` | Ignore legend |
| `IgnoreTitle` | Ignore title |
| `IgnoreFooter` | Ignore footer |

## Custom Layout Example

The following example demonstrates how to customize the plot layout:

```cpp
#include <QwtPlot>
#include <QwtPlotLayout>

// Create plot
QwtPlot* plot = new QwtPlot();
plot->setTitle("Custom Layout Example");

// Get layout object
QwtPlotLayout* layout = plot->plotLayout();

// Set component spacing
layout->setSpacing(8);

// Set canvas margins
layout->setCanvasMargin(20, QwtAxis::XBottom);
layout->setCanvasMargin(15, QwtAxis::YLeft);

// Disable canvas-to-axis alignment (maintain fixed size)
layout->setAlignCanvasToScales(false);

// Place legend on the right, occupying 25% width
layout->setLegendPosition(QwtPlot::RightLegend, 0.25);

// Refresh layout
plot->replot();
```

!!! warning "Layout Change Timing"
    After layout settings are changed, you need to call `plot->replot()` or wait for the next automatic refresh to see the effect.

## Minimum Size Hints

QwtPlotLayout can calculate minimum dimensions based on component content:

```cpp
// Get the plot's minimum size hint
QSize minSize = layout->minimumSizeHint(plot);

// Apply minimum size
plot->setMinimumSize(minSize);
```

!!! example "Related Examples"
    - Basic layout: All examples under `examples/2D/`
    - Multi-plot layout: `examples/figure` (using QwtFigure)
