## tag:v7.3.1 (2026-06-16)

### 新功能

- **qwt::core 共享库**
    - 提取 `qwt::core` 共享库（`qwtcore.dll`），包含颜色映射和调色板公共功能
    - `QwtColorMap` 重构：移除 `QwtInterval` 依赖，新签名 `rgb(vMin, vMax, value)`
    - `QwtColorCycle` 从 plot 模块迁移至 core
    - 新增 `QwtColorMapPreset` 类，提供 22 种科学可视化色彩映射预设（viridis、plasma、inferno、magma、cividis、jet、hot、cool、spring、summer、autumn、winter、gray、bone、copper、rainbow、hsv、turbo、coolwarm、rdylbu、rdylgn、spectral）

- **3D 主题系统**
    - 新增 `Qwt3DTheme` 类，提供 10 种预设主题：Default、Dark、Scientific、Warm、Cool、Matplotlib、EarthTones、Ocean、HighContrast、Presentation
    - 新增 `ColorMapColor` 适配器，桥接 2D 色彩映射到 3D 颜色函数
    - 5 种光照预设：NoLighting、FlatLight、Studio、Outdoor、Soft
    - `Plot3D::setTheme()`、`Plot3D::applyTheme()` 一键应用主题
    - `StandardColor::setPreset()` 快速切换色彩映射预设

### 架构变更

- 三模块架构：`qwtcore.dll` → `qwtplot.dll` + `qwtplot3d.dll`
- `plot` 和 `plot3d` 都依赖 `core`，但彼此独立
- 运行时需要 `qwtcore.dll` + `qwtplot.dll`（+ `qwtplot3d.dll` 如果启用 3D）
- `src/plot/` 中保留向后兼容的转发头文件，支持现有 `#include "qwt_color_map.h"` 路径

### 构建系统

- 新增 `src/core/CMakeLists.txt` 用于 core 库目标
- 更新 `src/plot/CMakeLists.txt` 链接 `qwt::core`
- 更新 `src/plot3d/CMakeLists.txt` 链接 `qwt::core` 并包含主题源文件
- 更新 amalgamate 工具模板以包含 core 模块文件

## tag:v7.3.0 (2026-06-12)

### 新功能

- **QwtColorCycle 颜色循环系统**
    - 新增`QwtColorCycle`类，管理颜色调色板并支持循环使用，内置7种预设调色板及自定义调色板
    - 与`QwtPlot`集成，绘图项在`attach()`时自动从颜色循环中获取颜色
    - 支持的绘图项：`QwtPlotCurve`、`QwtPlotMultiBarChart`、`QwtPlotBoxChart`、`QwtPlotHistogram`、`QwtPlotIntervalCurve`、`QwtPlotBarChart`
    - 新增颜色循环测试用例

- **曲线降采样过滤器**
    - `QwtPlotCurve`新增`FilterPointsPixel`和`FilterPointsLTTB`两种绘制属性
        - `FilterPointsPixel`：基于像素列的降采样，适用于超大数据集的极速渲染
        - `FilterPointsLTTB`：简化版LTTB（MinMax桶）算法，在降采样同时更好地保留视觉形状
    - 默认绘制属性由`FilterPoints`改为`FilterPointsAggressive`
    - `qwtMapPointsQuad`优化NaN/Inf值跳过逻辑和多边形内存预分配

- **SIMD加速的MinMax桶降采样**
    - 新增`qwt_simd_argminmax`模块，提供SIMD加速的argmin/argmax计算
    - 支持SSE2/AVX2/NEON指令集，自动检测并选择最优指令集
    - 显著提升MinMax桶降采样的性能

- **Picker增强**
    - `QwtPlotSeriesDataPickerGroup`增加点击位置获取功能，支持在点击时获取屏幕坐标和数据坐标
    - `QwtPlotSeriesDataPicker`扩展支持所有`QwtPlotSeriesItem`子类型的数据拾取：
        - 交易曲线(K线图)、区间曲线、直方图、向量场
        - 柱状图、箱线图、光谱曲线
    - `FeaturePoint`新增`sampleData`字段（QVariant），可携带完整样本数据（OHLC、区间、向量等）
    - 通过`Q_DECLARE_METATYPE`注册样本类型，支持跨模块传递

- **扁平化风格控件**
    - `QwtSimpleCompassRose`新增`flatStyle`属性，启用后用纯色替代经典双色3D效果
    - `QwtWheel`新增`flatStyle`属性，启用后用扁平颜色和简单线条替代3D渐变
    - `QwtSlider`、`QwtThermo`新增`flatStyle`属性支持现代扁平化UI
    - `QwtDialNeedle`增加多种扁平化指针样式
    - `QwtKnob`默认使用`Flat`模式

### 性能优化

- `qwtProbeOrientation`和`qwtFindVisibleRange`优化采样策略，最多检查50个采样点并确保最小步长为1
- `qwtPixelColumnReduce`优化：预分配polyline向量、使用直接指针赋值替代动态追加
- bins初始化使用`QVector`构造函数简化

### 代码重构

- **PIMPL宏统一迁移**（涉及180+文件）
    - 所有类从手动PIMPL（裸指针`PrivateData* m_data`）迁移至标准宏模式
    - 头文件使用`QWT_DECLARE_PRIVATE(ClassName)`，实现使用`QWT_DECLARE_PUBLIC` + `QWT_PIMPL_CONSTRUCT` + `QWT_D/QWT_DC`
    - 使用`unique_ptr`自动管理生命周期，移除所有`delete m_data`
    - 覆盖绘图核心、坐标轴、文本/图例、颜色映射、画布、柱状图、交互类、数据类、样条、控件、极坐标等全部模块

- **3D绘图模块重构**（涉及53文件）
    - 添加`override`说明符到所有虚函数和析构函数
    - `nullptr`替代`0`作为空指针常量
    - `SurfacePlot`重构为PIMPL模式（`QWT_DECLARE_PRIVATE`）
    - `qwt3d_ptr`重命名为`ClonePtr`，统一include guards

- **现代C++特性**
    - 使用`auto`关键字和基于范围的for循环进行代码现代化
    - 添加移动语义和C++11默认成员初始化器
    - 为简单getter/setter和构造函数添加`noexcept`
    - 扩展`constexpr`到区间、坐标轴、刻度映射和数学工具函数
    - 跨`src/plot`和`src/plot3d`模块进行C++惯用法现代化

### 新增示例

- **renderbench** - 曲线渲染性能基准测试
    - 窗口化数据系列（`WindowedSeriesData`），支持实时数据流模拟
    - 控制面板可调整数据量、降采样模式、线宽等参数
    - 支持生成Markdown格式的性能报告
    - 位于`examples/bench/renderbench/`
- **staticExample** - 重写为Qwt综合特性展示示例
    - 演示曲线、频谱图、柱状图、箱线图、向量场等多种绘图类型
    - 统一更新所有2D示例的颜色调色板和画布背景风格

### 文档

- 新增曲线降采样算法完整文档（中文）
- 新增`QWEN.md` AI Agent指引文件，更新构建说明
- 添加英文文档并重构双语文档站点结构
- 统一执行英文注释规范，所有源码注释改为纯英文Doxygen格式
- 大批量Doxygen双语注释补全（40+批次），覆盖所有核心头文件

### Bug修复

- 修复Qt6下legend图标因`QwtNullPaintDevice::metric()`返回0（设备像素比）导致的空白问题
- 修复`QwtPlotMarker`箭头首尾端点反转问题，改进图例图标渲染
- 修复绘图背景渲染问题，改用`QPalette::Base`动态`backgroundRole`，确保主题切换时背景色正确

## tag:v7.2.1

- 新增刻度朝内显示功能
    - 新增`QwtPlot::TickDirection`枚举，支持`TickOutside`（刻度朝外，默认）和`TickInside`（刻度朝内）
    - 新增`QwtPlot::setAxisTickDirection()`和`QwtPlot::axisTickDirection()`方法，可独立控制每个坐标轴的刻度方向
    - 刻度朝内时，刻度线从画布边缘向内延伸，主干和标签保持在画布外
    - 朝内刻度与朝外刻度共享相同的样式设置（长度、颜色、线宽）
    - 新增示例程序：`examples/2D/ticks_inside/`
    ![ticks_inside](./docs/assets/screenshots/ticks_inside.png)

## tag:v7.2.0

- 新增箱线图(Box Chart)支持
    - 新增`QwtPlotBoxChart`类，用于绘制箱线图（Box-and-Whisker Plot）
    - 新增`QwtBoxStatisticsCalculator`类，用于自动计算统计量
    - 新增数据结构：`QwtBoxSample`、`QwtBoxOutlierSample`、`QwtBoxChartData`、`QwtBoxOutlierChartData`
    - 支持预计算数据和原始数据两种输入方式
    - 支持多种箱体样式：矩形(Rect)、菱形(Diamond)、缺口形(Notch)
    - 支持多种须须计算方法：Tukey(1.5×IQR)、百分位数、最小最大值、标准差、标准误
    - 支持垂直和水平两种显示方向
    - 支持异常值自动检测、自定义符号和抖动显示
    - 新增示例程序：`examples/2D/boxchart/`
	![Box Chart](./docs/assets/screenshots/BoxChart.png)

## tag:v7.0.8

- `QwtFigure`增加`addAxisAlignment`等坐标轴对齐功能，可以指定子绘图的坐标轴进行对齐
	坐标轴对齐的效果如下：
	![axis-alignment](./docs/assets/picture/figure-scale-aligment.png)

- `QwtPointMapper`添加对`NaN`和`Inf`值的异常处理，在数据存在异常值时不会导致坐标映射异常
- `qwt_series_data.cpp`的数据范围判断增加对`NaN`和`Inf`值的异常处理
- 解决合并文件漏了QwtSlider类的问题（Issue #4）

## tag:v7.0.7

- v7.0.5~v7.0.6新增的`QwtScaleWidget::panScale`函数移动到`QwtPlot`中，并改名为`panAxis`，并修复了在对数坐标轴移动异常的问题，解决对数坐标轴坐标移动问题
- 原来`QwtPanner`类改名为`QwtCachePanner`,代表带缓存的`Panner`。Qwt6.0的Panner为了避免频繁刷新使用了一个pixmap进行缓存，因此把这类Panner统称为`CachePanner`
	- 同步`QwtPlotPanner`类改名为`QwtPlotCachePanner`
	- 同步`QwtPolarPanner`类改名为`QwtPolarCachePanner` 
	
	`QwtPlotPanner`效果如下：
	![series-data-picker-yvalue](./docs/assets/screenshots/qwt-realtime-panner.gif)
	
- 重写`QwtPlotPanner`类，继承`QwtPicker`，可实现拖动过程能实时刷新，如果想要原来的带缓存的panner，可使用`CachePanner`相关类
- 新增`QwtCanvasPicker`针对canvas的picker操作都继承此类
- `QwtPlotPanner`支持线性坐标轴、对数坐标轴、多坐标轴的实时平移
- 针对`QwtPlotZoomer`只能绑定2个坐标轴问题，新增`QwtPlotCanvasZoomer`类
	- `QwtPlotCanvasZoomer`无需绑定坐标轴，直接对整个cavas进行缩放
	- `QwtPlotCanvasZoomer`支持多坐标轴的缩放
- 原`QwtPlotZoomer`更名为`QwtPlotAxisZoomer`
- `QwtPlotSeriesDataPicker`将直接继承`QwtPicker`，不再继承`QwtPlotPicker`
- `QwtPlotMagnifier`支持多坐标轴的缩放
- `QwtScaleMap`增加移动语义
- 新增`make-classinclude.py`,可以导出`classincludes`目录，使用方法见`make-classinclude.py`
--

v7.0.7对Qwt的命名进行了调整

|原名称|新名称|备注|
|:--|:--|:--|
|QwtPlotZoomer|QwtPlotAxisZoomer|由于原QwtPlotZoomer只能绑定2个坐标轴，故改名为QwtPlotAxisZoomer|
|新增|QwtPlotCanvasZoomer|针对整个画布缩放的zoomer|
|QwtPanner|QwtCachePanner|原Panner无法实时拖动，故名Cache Panner|
|QwtPlotPanner|QwtPlotCachePanner|原Panner无法实时拖动，故名Cache Panner|
|QwtPolarPanner|QwtPolarCachePanner|原Panner无法实时拖动，故名Cache Panner|
|新增|QwtPlotPanner|原QwtPlotPanner改名为QwtPlotCachePanner后新增QwtPlotPanner|

## tag:v7.0.6

- 新增`QwtPlotSeriesDataPicker`类，提供了绘图数据的拾取
    `QwtPlotSeriesDataPicker`效果如下：
    ![series-data-picker-yvalue](./docs/assets/picture/series-data-picker-yvalue.gif)
    ![series-data-picker-nearest-value](./docs/assets/picture/series-data-picker-nearest-value.gif)
- 完善了寄生绘图的刷新机制，不会在构造时无法完全更新
- 有些接口的索引类型由int改为size_t
- `QwtPlotSeriesDataPicker`支持日期坐标轴的正常显示
- 修正了`QwtFigureWidgetOverlay`的一些事件处理会和坐标轴动作冲突的问题

## tag:v7.0.5

- `QwtScaleWidget`增加坐标轴内置动作功能，实现坐标轴的pan和zoom两种内置动作
	- `QwtScaleWidget`增加`scaleRect`函数，可以获取坐标刻度所在矩形，也就是用来绘制刻度的区域
	- `QwtScaleWidget`增加`QwtScaleWidget::BuiltinActions`枚举，目前包含了两个内置动作
	- 增加`requestScaleRangeUpdate`信号，用于请求绘图刷新坐标轴范围
	- 增加`setSelected`/`isSelected`函数，让坐标轴可选中，增加信号：`selectionChanged`
	- 增加坐标轴动作相关的一些属性设置如：`zoomFactor`、`panScale`等函数
- 增加`QwtPlotScaleEventDispatcher`类，实现绘图上处理坐标轴的相关事件
- `QwtPlot`的寄生绘图相关接口进行了一定调整
- `QwtPLot`增加`setEnableScaleBuildinActions`/`isEnableScaleBuildinActions`/`setupScaleEventDispatcher`等函数用于控制坐标轴的缩放和平移
- `QwtPLot`增加`plotList`函数，能获取所有绘图对象，包括寄生绘图和宿主绘图

## tag:v7.0.4

- 实现寄生绘图功能
- 增加寄生绘图的例子
- `QwtPlot`添加`updateAxisEdgeMargin`函数，让寄生绘图可以自动调整坐标轴位置

## tag:v7.0.2

- 抽取出`QwtPlotLayoutEngine`类
- 增加`QwtPlotParasiteLayout`类，用于实现寄生轴的布局
- 增加`QwtPlotTransparentCanvas`类，用于实现一个完全透明的画布
- `QwtFigure`类实现寄生轴功能
- `QwtScaleWidget`添加了`setEdgeMargin`和`edgeMargin`方法，可以实现刻度和绘图边缘的距离设置
- 调整了`QwtScaleWidget`的布局方案，能支持`edgeMargin`
- 例子增加`parasitePlot`演示如何使用寄生轴
- `QwtPlot`增加`rescaleAxes`,`syncAxis`等方法，用于实现快速调整坐标轴
- 针对nan和inf值的异常处理

## tag:v7.0.1

- 增加`QwtFigure`类，`QwtFigure`用于管理多个QwtPlot，实现类似Matplotlib的Figure功能，支持网格化布局

`QwtFigure`类的效果如下：

![figure](docs/assets/screenshots/qwt_figure.png)

- example增加figure例子，用于演示`QwtFigure`的使用
- 新增`QwtFigureWidgetOverlay`，可以实现在`QwtFigure`上面进行一些动作，目前支持拖动绘图，缩放绘图的功能

## tag:v7.0.0

- 把整个工程合并为QwtPlot.h和QwtPlot.cpp，直接可以引入，文件位于src-amalgamate
- 例子增加staticExample
- 增强`QwtPlotBarChart`的接口以支持pen和brush的设置.
- 增加`QwtGridRasterData`类，相比`QwtMatrixRasterData`，它支持一个二维数据表，以及x,y轴进行网格插值
- `QwtLinearColorMap`增加stopColors函数，修改`QwtLinearColorMap`的`colorStop`函数为`stopPos`
- `Qwt`的初始化参数进行了调整，让默认绘图更符合当前绘图的审美

`QwtPlotCanvas`初始化如下：
```cpp
QwtPlotCanvas::QwtPlotCanvas(QwtPlot* plot) : QFrame(plot), QwtPlotAbstractCanvas(this)
{
    ...

    setLineWidth(0);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::Box);
}
```

`QwtPlotLayout`初始化调整如下

```cpp
QwtPlotLayout::QwtPlotLayout()
{
...
    setCanvasMargin(-1);
...
}
```

- 去除QWT_MOC_INCLUDE
- 调整了一些类的实现，以便能合并到一个文件中

------

**Qwt 7.0** is my modified version based on Qwt 6.2.0 source code. It complies with Qwt's open-source license, and I'm releasing these modifications as open-source.

Starting from this version, Qwt will support:
- The latest **C++11 standard**
- **CMake build system**

**Modifications in this version:**
1. Enhanced `QwtPlotBarChart` interface to support **pen and brush configuration**  
2. Added `QwtGridRasterData` class – supports **2D data tables** with **grid interpolation** on X/Y axes (vs. `QwtMatrixRasterData`)  
3. Added `stopColors()` function to `QwtLinearColorMap` and renamed `colorStop()` to `stopPos()`  


> **Qwt7.0**是我基于Qwt6.2.0版本对源码进行修改后的版本，遵循Qwt的开源协议，我将修改进行开源
>
>从此版本的Qwt，将有如下改动:
>	
>- 支持最新的**C++11标准**
>- **支持CMake构建**
>
>下面是此版本的修改内容：
>- 增强`QwtPlotBarChart`的接口以支持pen和brush的设置.
>- 增加`QwtGridRasterData`类，相比`QwtMatrixRasterData`，它支持一个二维数据表，以及x,y轴进行网格插值
>- `QwtLinearColorMap`增加stopColors函数，修改`QwtLinearColorMap`的`colorStop`函数为`stopPos`



Qwt 6.2.0

------

0) Requirement for Qt >= 4.8

1) Class Includes added

Include files, that match the class names are available now. So
it is possible to write "#include <QwtPlot>" now instead of "include qwt_plot.h"

2) BSD License for examples

Where possible the code of the examples is available under the 3-clause BSD License 

3) MathML text renderer removed

The code can be found at https://github.com/uwerat/qwt-mml-dev now and is intended
to become a standalone lib. Anyone who is interested to workon it, please let me know.

4) Spline interpolation

The broken implementation of QwtSpline has been replaced by a bunch of classes
offering all sort of functionalities around splines.

The most popular spline approximation/interpolation algos have been implemented:

	- Basis
	- Cardinal
	- ParabolicBlending
	- Akima
	- The one used in MS Excel
	- Cubic

An implementation of the de Casteljau’s algorithm has been added

	- QwtBezier

5) New plot items

	- QwtPlotVectorField
      A new type of plot item for vector fields

    - QwtPlotGraphicItem
      An item displaying a QwtGraphic image ( f.e used by QwtPlotSvgItem )

6) Plot Canvas

	- QwtAbstractPlotCanvas introduced
	- QwtPlotOpenGLCanvas added to support QOpenGLWidget

7) QwtPlotCurve

	- QwtPlotCurve::FilterPointsAggressive mode added - a fast weeding algo
      for huge datasets with increasing x or y values

	- QwtPlotCurve::closestPoint is virtual now
	- QwtPlotCurve: polygon clipping includes the painter clip
	- QwtPlotCurve::setLegendAttributes added
	- QwtValuePointData added for curves, where the x values are the index
	- a couple of new QwtPlotCurve::setSamples alternatives

8) QwtPlotSpectrogram

	- QwtPlotSpectrogram::setColorTableSize added
	- QwtRasterData::setInterval/interval changed into a pure virtual getter
	- QwtMatrixRasterData::BicubicInterpolation added
	- QwtMatrixRasterData::interval: API cleanup
	- QwtHueColorMap, QwtSaturationValueColorMap added

9) QwtPlotRenderer

	- using QPdfWriter where possible

10)
	- LOG_MIN/LOG_MAX removed, use QwtTransform::LogMin/LogMax instead ( values differ ! )
	- qwt_compat.h removed
	- qwtFuzzyGreaterOrEqual/qwtFuzzyLessOrEqual removed
	- qwtGetMin/qwtGetMax removed

11)
    - Not aligning unknown paint engines ( f.e EMF )
    - QwtNullPaintDevice is using a different type than QPaintEngine::User now

12) Many other changes ...
