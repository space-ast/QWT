# QWT API文档（中文） {#mainpage}

## 概述

**QWT**（Qt Widgets for Technical Applications）是一个基于Qt的高性能2D/3D绘图库，
适用于科学计算和工程应用中的数据可视化。

- **协议**：LGPL（商业友好）
- **Qt支持**：Qt 5.12+ 和 Qt 6
- **C++标准**：C++11（Qt6时使用C++17）
- **构建系统**：CMake

## 2D绘图核心模块

| 类名 | 功能描述 |
|------|----------|
| `QwtPlot` | 二维绘图主控件，管理布局、坐标轴、图例、画布 |
| `QwtPlotCurve` | 数据曲线项，支持多种样式（连线、台阶、点等） |
| `QwtPlotGrid` | 网格项，提供主/次网格线 |
| `QwtPlotLegend` | 图例容器 |
| `QwtPlotMarker` | 标记项 |
| `QwtPlotSpectrogram` | 光谱图项 |
| `QwtPlotBarChart` | 柱状图项 |
| `QwtPlotVectorField` | 向量场项 |

## 布局与容器

| 类名 | 功能描述 |
|------|----------|
| `QwtFigure` | 多绘图布局容器（类似matplotlib的Figure） |
| `QwtFigureWidgetOverlay` | Figure操作蒙版，支持拖动、缩放子绘图 |
| `QwtPlotLayout` | 单个绘图布局管理器 |
| `QwtPlotParasiteLayout` | 寄生绘图布局管理器 |

## 交互控件

| 类名 | 功能描述 |
|------|----------|
| `QwtPlotPanner` | 画布实时拖动（支持多坐标轴） |
| `QwtPlotCachePanner` | 带缓存的拖动（原QwtPanner） |
| `QwtPlotCanvasZoomer` | 画布整体缩放（支持多坐标轴） |
| `QwtPlotAxisZoomer` | 坐标轴缩放（原QwtPlotZoomer） |
| `QwtPlotMagnifier` | 放大器 |
| `QwtPlotSeriesDataPicker` | 数据拾取器 |
| `QwtPicker` | 基础拾取器 |
| `QwtScaleWidget` | 坐标轴控件（支持内置pan/zoom动作） |

## 3D绘图模块

| 类名 | 功能描述 |
|------|----------|
| `Qwt3DPlot` | 3D绘图主控件 |
| `Qwt3DSurfacePlot` | 3D表面图 |
| `Qwt3DGridPlot` | 3D网格图 |
| `Qwt3DFunction` | 3D函数绘图 |

## 关键架构概念

### 寄生绘图

寄生绘图允许在同一个绘图区域内创建多个独立坐标轴：

- 通过 `QwtPlot::createParasitePlot()` 创建
- 与宿主绘图共享绘图区域
- 支持任意多个X/Y轴
- 自动生命周期管理

### 坐标轴系统

Qwt使用 `QwtAxis::Position` 枚举标识坐标轴位置：

- `QwtAxis::XBottom`, `QwtAxis::XTop`
- `QwtAxis::YLeft`, `QwtAxis::YRight`

## 项目引入方式

### CMake包引入（推荐）

```cmake
find_package(qwt REQUIRED)
# 2D绘图
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot)
# 3D绘图
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot3d)
```

### 单文件直接引入

将 `src-amalgamate/QwtPlot.h` 和 `src-amalgamate/QwtPlot.cpp` 加入项目。

依赖的Qt模块：`Core`, `Gui`, `Widgets`, `Svg`, `Concurrent`, `OpenGL`, `PrintSupport`
Qt6额外需要：`OpenGLWidgets`
3D功能需要：`OpenGL::GLU`

## 版权信息

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