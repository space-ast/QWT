# 注释规范

本文档规定 Qwt 项目代码注释的编写规范，确保文档与代码的一致性和完整性。

## 主要规范要求

**特性**

- ✅ **Doxygen格式**：使用 Doxygen 标准格式编写注释
- ✅ **双语支持**：提供中英文双语注释
- ✅ **位置规范**：详细注释写在 cpp 文件，简洁注释写在头文件
- ✅ **完整性**：类、函数、信号都需要完整的文档注释

## 注释位置原则

!!! info "核心原则"
    - **详细注释**写在 `.cpp` 文件中
    - **简洁注释**写在 `.h` 文件中
    - 保持头文件整洁，避免过多注释内容

## 源文件注释规范

所有新增代码必须使用 **Doxygen 格式**，并区分中英文。

doxygen关键字中，除了`if`和`endif`关键字使用`\`，其余关键字使用`@`进行标记

### 函数注释模板

```cpp
/**
 * \if ENGLISH
 * @brief [English brief description]
 * @param[in] param_name [English parameter description]
 * @param[out] param_name [English output parameter description]
 * @return [English return value description]
 * @details [English detailed explanation]
 * \endif
 * 
 * \if CHINESE
 * @brief [中文简要说明]
 * @param[in] param_name [中文参数描述]
 * @param[out] param_name [中文输出参数描述]
 * @return [中文返回值描述]
 * @details [中文详细说明]
 * \endif
 */
void MyClass::myFunction(int param_name)
{
    // ...
}
```

### 参数说明

使用 `@param[in]` 和 `@param[out]` 标注参数方向：

```cpp
/**
 * \if ENGLISH
 * @brief Calculate the intersection point of two curves
 * @param[in] curve1 First curve to intersect
 * @param[in] curve2 Second curve to intersect
 * @param[out] intersectionPoint The calculated intersection point
 * @return true if intersection found, false otherwise
 * \endif
 * 
 * \if CHINESE
 * @brief 计算两条曲线的交点
 * @param[in] curve1 第一条曲线
 * @param[in] curve2 第二条曲线
 * @param[out] intersectionPoint 计算出的交点坐标
 * @return 找到交点返回 true，否则返回 false
 * \endif
 */
bool calculateIntersection(const QwtPlotCurve* curve1,
                           const QwtPlotCurve* curve2,
                           QPointF& intersectionPoint);
```

### 详细说明示例

```cpp
/**
 * \if ENGLISH
 * @brief Set the samples for the curve
 * @param[in] xData Array of x coordinates
 * @param[in] yData Array of y coordinates
 * @param[in] size Number of samples
 * @details This method creates a new sample series from the provided arrays.
 *          The data is copied internally, so the original arrays can be
 *          modified or deleted after calling this method.
 * \endif
 * 
 * \if CHINESE
 * @brief 设置曲线的数据样本
 * @param[in] xData x坐标数组
 * @param[in] yData y坐标数组
 * @param[in] size 样本数量
 * @details 此方法从提供的数组创建新的样本序列。
 *          数据会在内部被复制，因此调用此方法后，
 *          原数组可以被修改或删除。
 * \endif
 */
void QwtPlotCurve::setSamples(const double* xData,
                              const double* yData,
                              int size);
```

## 头文件注释规范

头文件中的 `public` 函数声明仅添加**单行英文简要注释**。

### 简洁注释示例

```cpp
class MyClass {
public:
    // Constructor for MyClass (English only)
    MyClass();
    
    // Set the title text (English only)
    void setTitle(const QString& title);
    
    // Get the current title (English only)
    QString title() const;
};
```

!!! warning "注意"
    头文件中**不要**写入详细的双语 Doxygen 块，详细内容应保留在对应的 `.cpp` 文件中。

## 类注释规范

类的 doxygen 注释需要在头文件中按双语要求添加。

### 类注释模板

```cpp
/**
 * \if ENGLISH
 * @brief 2D plotting widget for displaying curves and other plot items
 * @details QwtPlot is the main widget for displaying 2D data. It manages
 *          axes, legends, and plot items like curves, markers, and grids.
 * \endif
 *
 * \if CHINESE
 * @brief 二维绘图控件，用于显示曲线和其他绘图项
 * @details QwtPlot 是显示二维数据的主控件，管理坐标轴、
 *          图例以及曲线、标记、网格等绘图项。
 * \endif
 */
class QwtPlot : public QWidget
{
    QWT_DECLARE_PRIVATE(QwtPlot)

public:
    // Constructor
    QwtPlot(QWidget* parent = nullptr);
    
    // ...
};
```

### 带示例的类注释

对于功能性较强的类，应加入使用示例：

```cpp
/**
 * \if ENGLISH
 * @brief Container widget for organizing multiple QwtPlot instances
 * @details QwtFigure provides a layout container similar to matplotlib's Figure,
 *          allowing multiple plots to be arranged in a single window.
 *
 * @code
 * QwtFigure* figure = new QwtFigure();
 * figure->setSizeInches(8, 6);
 *
 * QwtPlot* plot1 = new QwtPlot();
 * figure->addAxes(plot1, 0.0, 0.0, 0.5, 0.5);  // Left-top quadrant
 * @endcode
 * \endif
 *
 * \if CHINESE
 * @brief 多绘图布局容器，用于组织多个 QwtPlot 实例
 * @details QwtFigure 提供类似 matplotlib Figure 的布局容器，
 *          允许在单个窗口中排列多个绘图。
 *
 * @code
 * QwtFigure* figure = new QwtFigure();
 * figure->setSizeInches(8, 6);
 *
 * QwtPlot* plot1 = new QwtPlot();
 * figure->addAxes(plot1, 0.0, 0.0, 0.5, 0.5);  // 左上四分之一区域
 * @endcode
 * \endif
 */
class QwtFigure : public QWidget
{
    // ...
};
```

## 信号注释规范

Qt 的信号在头文件中声明，没有对应的 cpp 定义，因此信号的注释需要在头文件中完整添加。

### 信号注释示例

```cpp
class QwtPlot : public QWidget
{
Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Signal emitted when the plot scale changes
     * @param axisId The axis that changed its scale
     * \endif
     *
     * \if CHINESE
     * @brief 绘图比例变化时发出的信号
     * @param axisId 发生比例变化的坐标轴ID
     * \endif
     */
    void scaleChanged(QwtAxis::Position axisId);
};
```

## 注释风格检查表

在编写注释时，请检查以下要点：

| 检查项 | 位置 | 要求 |
|--------|------|------|
| 类注释 | 头文件 `.h` | 双语 Doxygen，包含使用示例 |
| public函数详细注释 | 源文件 `.cpp` | 双语 Doxygen |
| public函数简要注释 | 头文件 `.h` | 单行英文 |
| 信号注释 | 头文件 `.h` | 双语 Doxygen |
| private/protected函数 | 源文件 `.cpp` | 可选，建议双语 |

## 常见错误示例

### ❌ 错误：头文件中写详细注释

```cpp
// 错误示例 - 头文件中写太多注释
class MyClass {
public:
    /**
     * @brief Constructor for MyClass
     * @param parent Parent widget pointer
     * @details This constructor initializes the class with...
     */
    MyClass(QWidget* parent = nullptr);
};
```

### ✅ 正确：简洁头文件注释 + 详细cpp注释

头文件 `MyClass.h`：

```cpp
class MyClass {
public:
    // Constructor (English only)
    MyClass(QWidget* parent = nullptr);
};
```

源文件 `MyClass.cpp`：

```cpp
/**
 * \if ENGLISH
 * @brief Constructor for MyClass
 * @param[in] parent Parent widget pointer
 * @details This constructor initializes the widget and sets up
 *          the internal data structures.
 * \endif
 * 
 * \if CHINESE
 * @brief MyClass 构造函数
 * @param[in] parent 父控件指针
 * @details 此构造函数初始化控件并设置内部数据结构。
 * \endif
 */
MyClass::MyClass(QWidget* parent)
    : QWidget(parent)
{
}
```

## 相关文档

- [编码规范](coding-standards.md)
- [PIMPL模式使用指南](pimpl-pattern.md)
