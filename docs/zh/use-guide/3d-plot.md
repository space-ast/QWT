# 3D绘图简介

Qwt 7.1 将原 `QwtPlot3D` 库整合进来，提供了三维数据可视化能力。3D绘图模块支持表面图、网格图、函数绘图等类型，适合科学计算和工程分析中的三维数据展示。

## 主要功能特性

**特性**

- ✅ **多种绘图类型**：表面图、网格图、参数曲面等
- ✅ **OpenGL渲染**：利用OpenGL实现高性能三维渲染
- ✅ **交互操作**：支持鼠标旋转、缩放、平移
- ✅ **光照和材质**：支持光照效果和材质配置

## 3D绘图模块结构

```mermaid
classDiagram
    class Qwt3DPlot {
        +setCoordinateStyle()
        +setPlotStyle()
        +setRotation()
        +setScale()
        +updateData()
    }
    
    class Qwt3DSurfacePlot {
        +setData()
        +setResolution()
        +createRepresentation()
    }
    
    class Qwt3DGridPlot {
        +setData()
        +setResolution()
    }
    
    class Qwt3DFunction {
        +operator(x,y)
        +create()
    }
    
    Qwt3DPlot <|-- Qwt3DSurfacePlot
    Qwt3DPlot <|-- Qwt3DGridPlot
    Qwt3DPlot --> Qwt3DFunction : 使用
```

## 核心类介绍

| 类名 | 说明 |
|------|------|
| `Qwt3DPlot` | 3D绘图基类，提供基本框架和交互 |
| `Qwt3DSurfacePlot` | 3D表面图，显示连续曲面 |
| `Qwt3DGridPlot` | 3D网格图，显示离散网格数据 |
| `Qwt3DFunction` | 3D函数绘图，根据数学函数生成曲面 |
| `Qwt3DAxis` | 3D坐标轴配置 |
| `Qwt3DColorLegend` | 3D颜色条 |

## 使用方法

3D绘图的例子位于:`examples/3D/simpleplot3D`，例子截图如下：

![simpleplot3D](../../assets/screenshots-3D/simpleplot3D.png)

### 基本使用示例

```cpp
#include <Qwt3DPlot>
#include <Qwt3DSurfacePlot>
#include <Qwt3DFunction>

// 创建表面图
Qwt3DSurfacePlot* plot = new Qwt3DSurfacePlot();

// 定义函数
class MyFunction : public Qwt3DFunction
{
public:
    virtual double operator()(double x, double y) override
    {
        return std::sin(x) * std::cos(y);  // 数学函数
    }
};

// 创建函数对象
MyFunction* func = new MyFunction();

// 设置数据范围和分辨率
func->setDomain(-5, 5, -5, 5);  // x和y范围
func->setResolution(50);         // 50x50网格

// 创建曲面
func->create(plot);

// 设置旋转角度
plot->setRotation(30, 0, 45);  // X、Y、Z轴旋转角度

// 显示
plot->show();
```

### 数据加载

```cpp
// 从数据数组加载
Qwt3DSurfacePlot* plot = new Qwt3DSurfacePlot();

// 设置数据范围
plot->setDomain(0, 100, 0, 100);  // X、Y范围

// 设置分辨率
plot->setResolution(100);  // 100x100网格

// 加载Z值数据（100x100数组）
double zData[100][100];
// ... 填充数据 ...
plot->loadFromData(zData, 100, 100);
```

### 交互操作

```cpp
// 启用鼠标交互
plot->setMouseInteraction(true);

// 鼠标操作：
// - 左键拖动：旋转视角
// - 中键拖动：平移
// - 滚轮：缩放

// 设置缩放比例
plot->setScale(1.0, 1.0, 1.0);  // X、Y、Z缩放比例

// 设置旋转角度
plot->setRotation(45, 30, 60);  // X、Y、Z轴旋转角度（度）
```

### 颜色映射

```cpp
#include <Qwt3DColorLegend>

// 启用颜色条
Qwt3DColorLegend* legend = new Qwt3DColorLegend();
legend->setLimits(0, 10);  // Z值范围
legend->show(plot);

// 设置颜色映射
plot->setColorFromData();  // 根据Z值自动映射颜色
```

## 构建配置

使用3D功能需要启用 `QWT_CONFIG_QWTPLOT_3D` CMake选项：

```cmake
find_package(qwt REQUIRED)

# 链接2D绘图库
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot)

# 链接3D绘图库
target_link_libraries(${PROJECT_NAME} PRIVATE qwt::plot3d)
```

!!! warning "OpenGL依赖"
    3D绘图模块依赖OpenGL和GLU库。确保系统已安装OpenGL驱动和GLU库。

## 核心方法总结

| 方法 | 说明 |
|------|------|
| `setDomain()` | 设置X/Y数据范围 |
| `setResolution()` | 设置网格分辨率 |
| `setRotation()` | 设置旋转角度 |
| `setScale()` | 设置缩放比例 |
| `createRepresentation()` | 创建曲面表示 |
| `updateData()` | 更新数据 |

!!! tip "3D绘图建议"
    - 数据量不宜过大（推荐100x100网格以下）
    - 复杂曲面可适当降低分辨率提升性能
    - 使用光照效果增强视觉效果

!!! example "相关示例"
    - 基础3D绘图：`examples/3D/simpleplot3D`
    - 3D轴配置：`examples/3D/axes`
    - 3D增强：`examples/3D/enrichments`