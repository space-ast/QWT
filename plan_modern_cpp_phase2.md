# Qwt 现代 C++ 改造 — Phase 2 计划

> 本文档记录中等优先级的现代化改进项，供后续迭代执行。
> Phase 1（高优先级）已在单独提交中完成。

## 1. constexpr 扩展

### 现状
- `QWT_CONSTEXPR` 宏已在 `qwt_global.h` 中定义，但仅 `qwt_math.h` 中使用
- 大量编译期可求值的简单函数/构造函数未标记 constexpr

### 候选目标

| 文件 | 函数/构造函数 | 说明 |
|------|-------------|------|
| `qwt_interval.h` | `QwtInterval()` 默认构造 | trivial 构造 |
| `qwt_interval.h` | `minValue()`, `maxValue()`, `width()` | 简单 getter |
| `qwt_interval.h` | `isValid()`, `isNull()` | 简单判断 |
| `qwt_axis_id.h` | `QwtAxisId()` 构造函数 | trivial |
| `qwt_axis_id.h` | `isValid()` | 简单判断 |
| `qwt_scale_map.h` | `QwtScaleMap()` 默认构造 | trivial 初始化 |
| `qwt_scale_map.h` | `p1()`, `p2()`, `s1()`, `s2()` | 简单 getter |
| `qwt_text.h` | `QwtText()` 默认构造 | 可评估 |
| `qwt_date.h` | 静态常量方法 | `toDateTime` 等工具方法 |
| `qwt_math.h` | `qwtMinF`, `qwtMaxF` 等 | 已标记但可扩展 |

### 注意事项
- C++11 的 constexpr 函数仅允许单条 return 语句
- C++14+ 可放宽限制，可通过 `__cplusplus` 守卫区分
- 涉及 Qt 类型的方法需确认 Qt 侧是否 constexpr（大多数 Qt 类型在 C++11 下不是 constexpr）

---

## 2. 范围 for 循环（Range-based for）

### 现状
- 622 个 for 循环中仅 19 个（3%）使用范围 for
- 传统索引循环大量可改进

### 改进模式

```cpp
// Before:
for (int i = 0; i < list.size(); i++) {
    process(list[i]);
}

// After:
for (const auto& item : qwt_as_const(list)) {
    process(item);
}
```

### 注意事项
- Qt 容器（QList, QVector, QMap 等）必须使用 `qwt_as_const()` 防止 COW 深拷贝
- 需要修改循环体内使用索引 `i` 的情况时，考虑 `enumerate` 模式或保留索引
- 指针数组、原始 C 数组等场景需单独判断

### 高价值候选文件（循环最密集）
- `qwt_plot.cpp` — 大量 axis/item 遍历
- `qwt_scale_engine.cpp` — tick 计算循环
- `qwt_painter.cpp` — 绘制循环
- `qwt_spline*.cpp` — 数学计算循环
- `qwt_clipper.cpp` — 裁剪算法循环
- `qwt3d_*.cpp` — 3D 数据处理循环

---

## 3. 移动语义（Move Semantics）

### 现状
- 仅 12 处 `std::move`，集中在数据容器和 3D 模块
- `= delete` 完全未使用
- 大多数类未声明移动构造/赋值

### 候选目标

| 类 | 改进 |
|----|------|
| `QwtText` | 添加移动构造/赋值（内含 QString + QPixmap） |
| `QwtInterval` | 添加移动构造/赋值（trivial，可 default） |
| `QwtScaleMap` | 添加移动构造/赋值（trivial，可 default） |
| `QwtLegendData` | 添加移动构造/赋值（内含 QVariant map） |
| `QwtPainterCommand` | 添加移动构造/赋值（内含 union + 指针） |
| `QwtSeriesData<T>` | 添加移动构造/赋值（内含 QVector） |
| `QwtScaleDiv` | 添加移动构造/赋值 |

### 注意事项
- 仅对数据持有类（非 QObject 派生）添加移动操作
- 使用 `= default` 让编译器生成 trivial 移动操作（当所有成员可移动时）
- PIMPL 类（QWT_DECLARE_PRIVATE）的移动需特殊处理：unique_ptr 可移动但需自定义析构可见性
- 避免对 QObject 派生类添加移动操作（Qt 对象不可移动）

---

## 4. auto 关键字扩展

### 现状
- 约 126 处使用，集中在新增代码和 3D 模块
- 传统模块使用率极低

### 适用场景
- 迭代器类型：`auto it = map.begin()` 代替 `QMap<int, double>::iterator it = map.begin()`
- 指针转换：`auto* item = qobject_cast<QwtPlotItem*>(obj)` 
- 复杂返回类型：`const auto& list = getList()`
- Lambda 捕获

### 不适用场景
- 简单类型不需要 auto：`int i = 0`, `double x = 0.0` 保持原样
- 避免 `auto x = someFunction()` 当类型不明显时

---

## 5. 其他改进项

### 5a. noexcept 标记
- 简单 getter、trivial 函数可标记 noexcept
- 移动操作应标记 noexcept（提升 STL 容器性能）
- 示例：`QwtInterval::width() noexcept`, `QwtScaleMap::transform() noexcept`

### 5b. 统一初始化（Uniform Initialization）
- 部分构造函数使用旧式 `m_x(0), m_y(0.0)` 风格
- 可改为 C++11 成员初始化器 `int m_x{0}; double m_y{0.0};`
- 注意：这会改变未显式初始化成员的行为

### 5c. 旧式枚举
- 检查是否有裸枚举可以改为 `enum class`
- 注意：Qt 风格代码倾向于裸枚举以保持 API 兼容性
- 仅对新增的内部枚举使用 `enum class`

---

## 执行建议

1. **优先级排序**：constexpr（安全且无破坏性） > 范围 for（提高可读性） > 移动语义（需仔细评估）
2. **分批提交**：每个改进类别单独提交，便于 review 和回滚
3. **构建验证**：每批修改后执行 Debug + Release 构建，确保无 warning/error
4. **测试覆盖**：启用 `-DQWT_CONFIG_BUILD_TESTS=ON` 运行测试套件
