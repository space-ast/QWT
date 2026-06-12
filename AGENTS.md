# Qwt 项目 AI Agent 指引

Qwt 7.x（v7.3.0）是基于原版 Qwt 6.2.0 的维护分支。
基于 Qt 的高性能 2D/3D 绘图库，适用于科学计算和工程数据可视化。许可证：LGPL（Qwt License v1.0）。

快速链接：[GitHub](https://github.com/czyt1988/QWT) | [Gitee](https://gitee.com/czyt1988/QWT) | [文档](https://czyt1988.github.io/QWT/zh/)

## 构建命令

```powershell
# 推荐：使用 build.ps1 一键构建（自动探测 Qt、VS、CMake）
.\build.ps1                          # 全量构建 (Release)
.\build.ps1 build                    # 增量编译
.\build.ps1 rebuild                  # 清除 + 重配 + 编译
.\build.ps1 configure -Examples OFF -Playground OFF  # 仅主库

# 手动构建：Visual Studio 生成器（自动处理 MSVC 环境变量）
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="D:/Qt/6.7.3/msvc2019_64"
cmake --build build --config Debug

# Ninja（需先初始化 MSVC 环境变量：INCLUDE, LIB, PATH）
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="..."
```

- CMake 3.5+, Qt 5.12+ / Qt 6.x, MSVC 2019+
- 先检查 `build/CMakeCache.txt` 中 Qt 路径是否正确，清理构建只需删 `CMakeCache.txt` 和 `CMakeFiles/`
- CI 使用 GitHub Actions（Linux + Windows, Qt6.8），构建时跳过 examples/playground
- `build.ps1` 参数：`-Examples`、`-Playground`、`-Tests`、`-OpenGL`、`-Plot3D`（均接受 `ON`/`OFF`）

## 架构关键点

### 双库结构（非单库）

项目生成两个独立的 shared library：

| 目标 | CMake target | 输出名 | DLL 导出宏 |
|------|-------------|--------|-----------|
| 2D 绘图 | `qwt::plot` | `qwtplot.dll` / `libqwtplot.so` | `QWT_EXPORT` |
| 3D 绘图 | `qwt::plot3d` | `qwtplot3d.dll` / `libqwtplot3d.so` | `QWT3D_EXPORT` |

两者独立构建，3D 模块通过 `QWT_CONFIG_QWTPLOT_3D` 控制。依赖：
- 2D：`Core Gui Widgets`(public) + `Concurrent PrintSupport`(private)，可选 `OpenGL OpenGLWidgets Svg`
- 3D：`OpenGL::GLU` + `Qt OpenGL Widgets`，内置 `gl2ps` 回退

### 条件编译模块

| CMake 选项 | 默认 | 控制内容 |
|-----------|------|---------|
| `QWT_CONFIG_QWTPLOT` | ON | 核心绘图：QwtPlot、曲线、网格、缩放等 |
| `QWT_CONFIG_QWTPOLAR` | ON | 极坐标绘图子模块 |
| `QWT_CONFIG_QWTWIDGETS` | ON | 控件：滑块、旋钮、刻度盘、温度计等 |
| `QWT_CONFIG_QWTSVG` | ON | SVG 导出/渲染 |
| `QWT_CONFIG_QWTOPENGL` | ON | OpenGL 画布 |
| `QWT_CONFIG_QWTPLOT_3D` | ON | 3D 绘图模块 |
| `QWT_CONFIG_BUILD_EXAMPLE` | ON | 构建示例程序 |
| `QWT_CONFIG_BUILD_PLAYGROUND` | ON | 构建实验性代码 |
| `QWT_CONFIG_BUILD_STATIC_EXAMPLE` | ON | 构建静态链接示例 |
| `QWT_CONFIG_BUILD_TESTS` | OFF | 测试构建 |

### PIMPL 模式（自定义宏）

使用 `qwt_global.h` 定义的宏，而非 Qt 的 `Q_DECLARE_PRIVATE`：

```cpp
// 头文件：声明 PIMPL
class QWT_EXPORT QwtFoo : public QWidget {
    QWT_DECLARE_PRIVATE(QwtFoo)  // → class PrivateData; unique_ptr<PrivateData> m_data;
};

// 源文件：实现 PrivateData
class QwtFoo::PrivateData {
    QWT_DECLARE_PUBLIC(QwtFoo)  // → QwtFoo* q_ptr;
};

// 构造函数初始化
QwtFoo::QwtFoo() : QWT_PIMPL_CONSTRUCT {}  // → m_data(qwt_make_unique<PrivateData>(this))

// 访问私有时使用
QWT_D(d);     // PrivateData* d = d_func()
QWT_DC(d);    // const PrivateData* d = d_func()
```

**关键原则**：`m_data` 直接存储 PrivateData（不使用堆指针重定向）。非 const 方法通过 `QWT_D()` 访问，const 方法通过 `QWT_DC()` 访问。

### Qt5/Qt6 兼容层

`qwt_qt5qt6_compat.hpp` 提供 `qwt::compat::` 命名空间下的兼容函数。涉及鼠标/滚轮/字体度量差异时，**必须使用**：
- `qwt::compat::eventPos(event)` → `event->pos()`(Qt5) / `event->position().toPoint()`(Qt6)
- `qwt::compat::wheelEventDelta(event)` → `event->delta()`(Qt5) / `event->angleDelta().y()`(Qt6)
- `qwt::compat::horizontalAdvance(fm, str)` → `fm.width(str)`(Qt5.12-) / `fm.horizontalAdvance(str)`(Qt5.12+)

## 编码规范（必须遵守）

### 格式化

`.clang-format` 基于 WebKit 风格：
- 缩进 4 空格，列宽 120
- 花括号：类/函数/枚举/命名空间后换行，控制语句不换行
- 指针左对齐（`int* p`），模板 `template<` 后无空格
- `SortIncludes: false` —— include 顺序手动控制

### 命名

- 类名：`Qwt` 开头大驼峰，如 `QwtPlotCurve`
- 方法：小驼峰，getter 不加 `get` 前缀，setter 加 `set`
- 成员变量：`m_` 前缀，如 `m_data`
- 局部变量：小驼峰
- 常量：`MAX_WIDTH`

### Include 顺序（手动排序，不可乱）

```cpp
#include "own_header.h"   // 本文件对应头文件优先
// 空行
#include <qnamespace.h>   // Qt 系统头文件 <q...>
#include <QWidget>        // Qt 类头文件 <Q...>
// 空行
#include <algorithm>      // STL
// 空行
#include "qwt_plot.h"     // 本项目其他头文件
```

### 注释规范（Doxygen 纯英文）

所有源码注释**一律使用英文**，禁止中文。Doxygen 关键字统一用 `@` 前缀（`@brief`、`@param`、`@return`、`@details`）。

| 注释类型 | 位置 | 要求 |
|---------|------|------|
| 类注释 | `.h` | 英文 Doxygen，含使用示例 |
| public 函数详细 | `.cpp` | 英文 Doxygen（`@brief`/`@param`/`@return`/`@details`） |
| public 函数简要 | `.h` | 单行英文 `// Comment`（无 Doxygen 块） |
| 信号注释 | `.h` | 英文 Doxygen（信号无 cpp 定义） |
| private/protected 函数 | `.cpp` | 可选，建议英文 |
| 行内注释 | 任意 | 英文 |

参数方向：`@param[out]`、`@param[in,out]` 仅在非 const 引用/指针时必须标注；value 和 const ref 省略。

### 现代 C++ 要求

- 使用 `override`/`final`（不用旧宏 `QWT_OVERRIDE`/`QWT_FINAL`）
- 使用 `nullptr`（不用 `NULL`）
- 使用 `static_cast<>` / `using`（不用 C 风格转换 / `typedef`）
- Qt 容器迭代使用 `qwt_as_const(container)` 防止 COW 深拷贝
- 智能指针使用 `qwt_make_unique<T>(args...)`（C++11 兼容）

### 信号槽

- 使用 `Q_SIGNALS:` / `public Q_SLOTS:` 宏（不用 `signals:` / `public slots:`）
- 优先新式 `connect(sender, &Sender::signal, receiver, &Receiver::slot)` 语法

## 重要命名变更（v7.0.7+）

旧名称已在代码库中移除，**不要使用旧名**：

| 已废弃的旧名 | 当前使用的新名 |
|-------------|--------------|
| `QwtPlotZoomer` | `QwtPlotAxisZoomer`（仅绑定 2 个轴） |
| `QwtPanner` | `QwtCachePanner`（带像素缓存） |
| `QwtPlotPanner` (旧) | `QwtPlotCachePanner` |
| - | `QwtPlotCanvasZoomer`（新增：整体画布缩放，不限轴数） |
| - | `QwtPlotPanner`（新增：实时拖动，基于 QwtPicker） |

## 关键架构概念

### 坐标轴系统

使用 `QwtAxisId`（包含 Position + 序号），枚举 `QwtAxis::Position`：
- `QwtAxis::XBottom`, `QwtAxis::XTop`, `QwtAxis::YLeft`, `QwtAxis::YRight`

兼容旧版 `Axis` 枚举：`QwtPlot::yLeft` 等仍可用（通过 `QWT_AXIS_COMPAT` 宏）。

### 寄生绘图 (Parasite Plot)

通过 `QwtPlot::createParasitePlot(QwtAxis::Position)` 创建寄生绘图，共享宿主画布区域，支持任意多轴。寄生绘图和宿主通过 `plotList()` / `hostPlot()` / `parasitePlots()` 管理。

### QwtFigure

类似 matplotlib Figure 的多绘图布局容器，支持网格排列。通过 `QwtFigureWidgetOverlay` 提供交互操作（拖动、缩放子绘图）。

## 不可触碰的文件

| 文件 | 原因 |
|------|------|
| `src-amalgamate/QwtPlot.h` | 自动合成，不要手动编辑 |
| `src-amalgamate/QwtPlot.cpp` | 自动合成，不要手动编辑 |
| `tools/` | 合成脚本，非核心库代码 |

## 测试

- 位于 `tests/`，使用 Qt Test 框架
- 构建时需 `-DQWT_CONFIG_BUILD_TESTS=ON`
- 当前活跃测试：`picker_click_test`, `picker_group_click_test`
- `splineprof/` 和 `splinetest/` 为性能/功能测试，暂未接入 CMake

## 文档

- MkDocs Material 主题 + i18n 插件（中文/英文）
- Doxygen API 文档：`docs/doxygen-doc-file/Doxyfile_en.cfg` / `Doxyfile_cn.cfg`
- CI 自动部署 GitHub Pages（仅 master 分支 push 触发）
- 构建文档依赖：`pip install -r requirements-docs.txt`
- 在线文档：https://czyt1988.github.io/QWT/zh/
