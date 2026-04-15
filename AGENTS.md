# Qwt 项目 AI Agent 指引

本文档为 AI Agent 提供项目快速指引，帮助快速理解项目架构、编码规则和关键信息。

## 项目概述

**Qwt 7.x** 是基于原版 Qwt 6.2.0 的维护版本，是一个基于 Qt 的高性能 2D/3D 绘图库，适用于科学计算和工程应用中的数据可视化。

- **协议**: LGPL（商业友好）
- **Qt版本**: 支持 Qt5.12+ 和 Qt6
- **C++标准**: C++11（Qt6时使用C++17）
- **构建系统**: CMake
- **项目地址**: [GitHub](https://github.com/czyt1988/QWT) / [Gitee](https://gitee.com/czyt1988/QWT)

## 目录结构

本项目把qwt的2d绘图和3d绘图进行合并，引入一个库，可以同时拥有2d绘图和3d绘图

```
qwt/
├── src/
│   ├── plot/          # 2D 绘图核心模块（~140个源文件）
│   ├── plot3d/        # 3D 绘图模块（来自qwtplot3d整合）
│   └── qwt_global.h   # 全局定义和兼容性宏
├── src-amalgamate/    # 单文件聚合版本
│   ├── QwtPlot.h      # 合并后的单一头文件
│   └── QwtPlot.cpp    # 合并后的单一源文件
├── examples/          # 示例程序
│   ├── 2D/            # 2D绘图示例
│   └── 3D/            # 3D绘图示例
├── playground/        # 实验性代码
├── classincludes/     # 类名头文件映射（支持 #include <QwtPlot>）
├── docs/              # 用户文档
│   └── zh/            # 中文文档
├── cmake/             # CMake配置模板
├── designer/          # Qt Designer插件
├── tests/             # 测试代码
└── tools/             # 工具脚本
```

## 核心模块

### 2D 绘图模块 (src/plot)

| 核心类 | 功能描述 |
|--------|----------|
| `QwtPlot` | 二维绘图主控件，管理布局、坐标轴、图例、画布 |
| `QwtPlotCurve` | 数据曲线项，支持多种样式（连线、台阶、点等） |
| `QwtPlotGrid` | 网格项，提供主/次网格线 |
| `QwtPlotLegend` | 图例容器 |
| `QwtPlotMarker` | 标记项 |
| `QwtPlotSpectrogram` | 光谱图项 |
| `QwtPlotBarChart` | 柱状图项 |
| `QwtPlotVectorField` | 向量场项 |

### 布局与容器

| 类名 | 功能描述 |
|------|----------|
| `QwtFigure` | 多绘图布局容器（类似matplotlib的Figure） |
| `QwtFigureWidgetOverlay` | Figure操作蒙版，支持拖动、缩放子绘图 |
| `QwtPlotLayout` | 单个绘图布局管理器 |
| `QwtPlotParasiteLayout` | 寄生绘图布局管理器 |

### 交互控件

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

### 3D 绘图模块 (src/plot3d)

| 核心类 | 功能描述 |
|--------|----------|
| `Qwt3DPlot` | 3D绘图主控件 |
| `Qwt3DSurfacePlot` | 3D表面图 |
| `Qwt3DGridPlot` | 3D网格图 |
| `Qwt3DFunction` | 3D函数绘图 |

## 项目引入方式

### 方式一：CMake 包引入（推荐）

```cmake
find_package(qwt REQUIRED)
# 2D绘图
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot)
# 3D绘图
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot3d)
```

### 方式二：单文件直接引入

将 `src-amalgamate/QwtPlot.h` 和 `src-amalgamate/QwtPlot.cpp` 加入项目。

依赖的Qt模块：`Core`, `Gui`, `Widgets`, `Svg`, `Concurrent`, `OpenGL`, `PrintSupport`
Qt6额外需要：`OpenGLWidgets`
3D功能需要：`OpenGL::GLU`

**注意:**单文件是由工具合成，agent不要操作`QwtPlot.h`和`QwtPlot.cpp`文件

## 构建配置选项

| CMake选项 | 默认值 | 说明 |
|-----------|--------|------|
| `QWT_CONFIG_QWTPLOT` | ON | 2D绘图核心类 |
| `QWT_CONFIG_QWTPOLAR` | ON | 极坐标绘图 |
| `QWT_CONFIG_QWTWIDGETS` | ON | 其他控件（滑块、刻度盘等） |
| `QWT_CONFIG_QWTSVG` | ON | SVG支持 |
| `QWT_CONFIG_QWTOPENGL` | ON | OpenGL画布支持 |
| `QWT_CONFIG_QWTPLOT_3D` | ON | 3D绘图模块 |
| `QWT_CONFIG_BUILD_EXAMPLE` | ON | 构建示例 |
| `QWT_CONFIG_BUILD_PLAYGROUND` | ON | 构建实验代码 |

## 开发规范

详细的开发规范文档位于 `docs/zh/dev-guide/` 目录：

| 文档 | 说明 |
|------|------|
| [编码规范](docs/zh/dev-guide/coding-standards.md) | 代码格式化、C++11兼容性宏、命名规范等 |
| [注释规范](docs/zh/dev-guide/comment-standards.md) | Doxygen双语注释格式、注释位置规范等 |
| [PIMPL模式](docs/zh/dev-guide/pimpl-pattern.md) | PIMPL设计模式的使用方法和宏定义 |
| [开发指引首页](docs/zh/dev-guide/index.md) | 开发指引概览和快速开始 |

### 编码规范要点

1. **使用 `override` 和 `final`** 代替旧宏 `QWT_OVERRIDE`、`QWT_FINAL`
2. **使用 `nullptr`** 代替 `NULL`
3. **使用 `static_cast`** 代替C风格强制转换
4. **使用 `using`** 代替 `typedef`
5. **Qt容器迭代** 使用 `qwt_as_const` 防止深拷贝

详细内容请参阅 [编码规范](docs/zh/dev-guide/coding-standards.md)。

### 注释规范要点

所有新增代码必须使用 **Doxygen 双语格式**（中英文）：

- 详细注释写在 `.cpp` 文件
- 简洁注释写在 `.h` 文件
- 类注释需要在头文件中添加双语 Doxygen

详细内容请参阅 [注释规范](docs/zh/dev-guide/comment-standards.md)。

### PIMPL 模式要点

项目提供 PIMPL 模式宏定义：

- `QWT_DECLARE_PRIVATE(Class)` — 声明私有数据指针
- `QWT_DECLARE_PUBLIC(Class)` — 声明公有宿主指针
- `QWT_PIMPL_CONSTRUCT` — 初始化私有数据指针
- `QWT_D(name)` / `QWT_DC(name)` — 访问私有数据

详细使用方法请参阅 [PIMPL模式使用指南](docs/zh/dev-guide/pimpl-pattern.md)。

## 重要命名变更 (v7.0.7+)

| 原名称 | 新名称 | 说明 |
|--------|--------|------|
| `QwtPlotZoomer` | `QwtPlotAxisZoomer` | 只能绑定2个坐标轴 |
| `QwtPanner` | `QwtCachePanner` | 带缓存的拖动 |
| `QwtPlotPanner` | `QwtPlotCachePanner` | 带缓存的绘图拖动 |
| - | `QwtPlotCanvasZoomer` | 新增：整体画布缩放 |
| - | `QwtPlotPanner` | 新增：实时拖动 |

## 关键架构概念

### 寄生绘图 (Parasite Plot)

寄生绘图允许在同一个绘图区域内创建多个独立坐标轴：

- 通过 `QwtPlot::createParasitePlot()` 创建
- 与宿主绘图共享绘图区域
- 支持任意多个X/Y轴
- 自动生命周期管理

```cpp
QwtPlot* hostPlot = new QwtPlot();
QwtPlot* parasitePlot = hostPlot->createParasitePlot(QwtAxis::YLeft);
```

### 坐标轴系统

Qwt使用 `QwtAxis::Position` 枚举标识坐标轴位置：

- `QwtAxis::XBottom`, `QwtAxis::XTop`
- `QwtAxis::YLeft`, `QwtAxis::YRight`

## 构建及编译指引

如果需要构建和编译，你需要阅读`./build.md`文件

## 示例索引

| 示例路径 | 功能演示 |
|----------|----------|
| `examples/2D/simpleplot` | 基础折线图 |
| `examples/figure` | QwtFigure多绘图布局 |
| `examples/parasitePlot` | 寄生轴多坐标轴 |
| `examples/curvedemo` | 曲线样式演示 |
| `examples/barchart` | 柱状图 |
| `examples/spectrogram` | 光谱图 |
| `examples/cpuplot` | 实时数据监控 |
| `examples/realtime` | 实时绘图 |
| `examples/oscilloscope` | 示波器效果 |
| `examples/polardemo` | 极坐标绘图 |
| `examples/stockchart` | 股票图表 |

## 文档资源

- **用户文档**: `docs/zh/` 目录下的中文文档
- **构建说明**: `docs/zh/build-guide/build-instructions.md`
- **引入说明**: `docs/zh/use-guide/import-qwt.md`
- **Figure使用**: `docs/zh/use-guide/figure-widget.md`
- **寄生轴**: `docs/zh/use-guide/parasite-axes.md`
- **变更日志**: `CHANGES.md`
