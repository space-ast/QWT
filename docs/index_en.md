# QWT API Documentation (English) {#mainpage}

## Overview

**QWT** (Qt Widgets for Technical Applications) is a high-performance 2D/3D plotting library based on Qt,
designed for data visualization in scientific computing and engineering applications.

- **License**: LGPL (commercial-friendly)
- **Qt Support**: Qt 5.12+ and Qt 6
- **C++ Standard**: C++11 (C++17 for Qt6)
- **Build System**: CMake

## Core 2D Plotting Module

| Class | Description |
|-------|-------------|
| `QwtPlot` | 2D plot widget, manages layout, axes, legend, and canvas |
| `QwtPlotCurve` | Data curve item, supports various styles (lines, steps, dots, etc.) |
| `QwtPlotGrid` | Grid item, provides major/minor grid lines |
| `QwtPlotLegend` | Legend container |
| `QwtPlotMarker` | Marker item |
| `QwtPlotSpectrogram` | Spectrogram item |
| `QwtPlotBarChart` | Bar chart item |
| `QwtPlotVectorField` | Vector field item |

## Layout & Containers

| Class | Description |
|-------|-------------|
| `QwtFigure` | Multi-plot layout container (similar to matplotlib Figure) |
| `QwtFigureWidgetOverlay` | Figure interaction overlay, supports drag and resize |
| `QwtPlotLayout` | Single plot layout manager |
| `QwtPlotParasiteLayout` | Parasite plot layout manager |

## Interaction Controls

| Class | Description |
|-------|-------------|
| `QwtPlotPanner` | Real-time canvas panning (supports multiple axes) |
| `QwtPlotCachePanner` | Cached panning (original QwtPanner) |
| `QwtPlotCanvasZoomer` | Whole-canvas zoom (supports multiple axes) |
| `QwtPlotAxisZoomer` | Axis zoom (original QwtPlotZoomer) |
| `QwtPlotMagnifier` | Magnifier |
| `QwtPlotSeriesDataPicker` | Data picker |
| `QwtPicker` | Base picker |
| `QwtScaleWidget` | Scale widget (supports built-in pan/zoom actions) |

## 3D Plotting Module

| Class | Description |
|-------|-------------|
| `Qwt3DPlot` | 3D plot widget |
| `Qwt3DSurfacePlot` | 3D surface plot |
| `Qwt3DGridPlot` | 3D grid plot |
| `Qwt3DFunction` | 3D function plot |

## Key Architecture Concepts

### Parasite Plot

Parasite plots allow creating multiple independent axis systems within the same plot area:

- Created via `QwtPlot::createParasitePlot()`
- Shares the plot area with the host plot
- Supports arbitrary number of X/Y axes
- Automatic lifecycle management

### Axis System

Qwt uses `QwtAxis::Position` enum to identify axis positions:

- `QwtAxis::XBottom`, `QwtAxis::XTop`
- `QwtAxis::YLeft`, `QwtAxis::YRight`

## Project Integration

### CMake Package (Recommended)

```cmake
find_package(qwt REQUIRED)
# 2D plotting
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot)
# 3D plotting
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot3d)
```

### Single File Integration

Add `QwtPlot.h` and `QwtPlot.cpp` from `src-amalgamate/` to your project.

Required Qt modules: `Core`, `Gui`, `Widgets`, `Svg`, `Concurrent`, `OpenGL`, `PrintSupport`
Qt6 additional: `OpenGLWidgets`
3D features require: `OpenGL::GLU`

## Copyright

```
Qwt Widget Library
Copyright (C) 1997   Josef Wilgen
Copyright (C) 2002   Uwe Rathmann

Qwt is published under the Qwt License, Version 1.0.
You should have received a copy of this licence in the file COPYING.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```