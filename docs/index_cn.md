# QWT API文档（中文） {#mainpage}

## 概述

**QWT**（Qt Widgets for Technical Applications）是一个基于Qt的高性能2D/3D绘图库，
适用于科学计算和工程应用中的数据可视化。

- **协议**：LGPL（商业友好）
- **Qt支持**：Qt 5.12+ 和 Qt 6
- **C++标准**：C++11（Qt6时使用C++17）
- **构建系统**：CMake

## 基础工具模块 (core)

`qwt::core`（`qwtcore.dll`）是项目的基础工具库，包含 21 个模块，`plot` 和 `plot3d` 都依赖此模块。

```
    ┌────────────────────┐
    │     qwt::core      │  ← 基础工具库（颜色、数学、数据类型、几何、变换、时间等）
    └────────────────────┘
          ↗         ↖
 ┌────────────┐  ┌─────────────┐
 │  qwt::plot  │  │ qwt::plot3d │
 │    (2D)     │  │    (3D)     │
 └────────────┘  └─────────────┘
```

| 类别 | 类/模块 | 说明 |
|------|---------|------|
| 颜色工具 | `QwtColorMap` 及子类 | 颜色映射（线性、HSV、饱和度等） |
| | `QwtColorMapPreset` | 22 种科学可视化色彩映射预设 |
| | `QwtColorCycle` | 颜色循环系统 |
| 数学工具 | `qwt_math.h` | 数学常量、`qwtMinF`/`qwtMaxF` 等工具函数 |
| | `qwtSimdArgMinMax` | SIMD 加速的 argmin/argmax |
| 数据类型 | `QwtInterval` | 区间类型 |
| | `QwtPoint3D` | 三维点 |
| | `QwtPointPolar` | 极坐标点 |
| | `QwtSamples` | 样本数据结构 |
| | `QwtBoxStatistics` | 箱线图统计量 |
| 几何算法 | `QwtBezier` | 贝塞尔曲线（de Casteljau 算法） |
| | `QwtClipper` | 多边形裁剪 |
| 坐标变换 | `QwtTransform` | 坐标变换（线性、对数等） |
| | `QwtScaleMap` | 坐标映射 |
| | `QwtScaleDiv` | 刻度划分 |
| 时间处理 | `QwtDate` | 日期时间工具 |
| | `QwtSystemClock` | 高精度计时器 |
| 通用算法 | `qwt_algorithm.hpp` | 通用算法工具 |
| | `qwt_qt5qt6_compat.hpp` | Qt5/Qt6 兼容层 |
| 数据容器 | `QwtGridData` | 网格数据容器 |

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
# 2D绘图（自动链接 qwt::core）
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot)
# 3D绘图（自动链接 qwt::core）
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot3d)
# 如果只需要基础工具（如数学函数、数据类型），可以只链接 core
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::core)
```

依赖说明：
- `qwt::core` 仅依赖 Qt `Core` + `Gui`
- `qwt::plot` 依赖 Qt `Core` + `Gui` + `Widgets` + `Svg` + `Concurrent` + `OpenGL` + `PrintSupport`
- `qwt::plot3d` 依赖 Qt `Core` + `Gui` + `Widgets` + `OpenGL::GLU` + Qt OpenGL Widgets
- Qt6 额外需要：`OpenGLWidgets`

### 单文件直接引入

将 `src-amalgamate/QwtPlot.h` 和 `src-amalgamate/QwtPlot.cpp` 加入项目。

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