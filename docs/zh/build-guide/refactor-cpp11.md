# QWT C++11 重构记录

本文档记录了 QWT 库从 C++98 重构为 C++11 标准的完整过程。此次重构由 AI 辅助完成（Qoder），确保代码在 Qt 5 和 Qt 6 环境下均可正常编译，同时保持所有业务逻辑不变。

## 重构背景

QWT 原始代码使用了部分 C++17 特性（如 `std::make_unique`、`std::as_const`），为了提高代码的可移植性和对旧编译器的兼容性，需要将代码标准降低到 C++11，同时通过兼容性宏层保留对高版本 C++ 特性的支持。

## 重构约束

重构过程遵循以下约束条件：

1. **枚举类型**：保持原有 `enum` 不变，不转换为 `enum class`
2. **override/final**：将 `QWT_OVERRIDE`、`QWT_FINAL` 宏替换为 C++11 原生关键字 `override`、`final`
3. **指针管理**：仅修改 PIMPL（Private Implementation）模式相关的指针，使用 `std::unique_ptr`
4. **CMake 标准**：根据 Qt 版本条件设置 C++ 标准（Qt 6 需要 C++17，Qt 5 使用 C++11）
5. **业务逻辑**：所有重构仅涉及语法层面，不改变任何业务逻辑

## 重构范围

- **涉及文件**：239 个文件
- **代码变更**：约 73,253 行插入，75,647 行删除
- **模块覆盖**：`src/plot`（2D 绘图模块）、`src/plot3d`（3D 绘图模块）、`examples/`、`designer/`、`src-amalgamate/`
- **提交分支**：`cpp11`
- **提交哈希**：`4db9767`

## 重构步骤

### Phase 1：核心基础设施变更

**目标**：建立 C++11 兼容性基础设施。

**执行动作**：

1. **移动 `qwt_global.h`**：从 `src/plot/qwt_global.h` 移动到 `src/qwt_global.h`，使其成为 `plot` 和 `plot3d` 模块共享的兼容性头文件

2. **引入兼容性宏**：
   - `qwt_make_unique`：C++14 及以上版本委托给 `std::make_unique`，C++11 提供自定义实现

     ```cpp
     // C++14+
     using std::make_unique;
     // C++11 自定义实现
     template<typename T, typename... Args>
     std::unique_ptr<T> qwt_make_unique(Args&&... args) {
         return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
     }
     ```

   - `qwt_as_const`：C++17 使用 `std::as_const`，C++11/14 使用 Qt 的 `qAsConst`

3. **修改 `QWT_PIMPL_CONSTRUCT` 宏**：改用 `qwt_make_unique` 替代 `std::make_unique`

4. **简化 `QWT_CONSTEXPR`**：直接使用 `constexpr` 关键字（Qt 5.12+ 编译器均支持）

5. **移除宏定义**：删除 `QWT_OVERRIDE` 和 `QWT_FINAL` 宏定义（C++11 原生支持这些关键字）

**涉及文件**：

- `src/qwt_global.h`（从 `src/plot/` 移入）
- `src/plot/CMakeLists.txt`（移除 `qwt_global.h` 条目）
- `classincludes/QwtGlobal`（更新 include 路径）
- `src/plot3d/qwt3d_global.h`（添加 `#include "qwt_global.h"`）

### Phase 2：QWT_OVERRIDE / QWT_FINAL 宏替换

**目标**：将兼容性宏替换为 C++11 原生关键字。

**执行动作**：

- `QWT_OVERRIDE` → `override`（约 422 处）
- `QWT_FINAL` → `final`（约 12 处）
- `QWT_CONSTEXPR` → `constexpr`（约 8 处，主要在 `qwt_math.h`）

**涉及文件**：`src/plot/` 下所有 `.h` 和 `.cpp` 文件

### Phase 3：NULL → nullptr 全局替换

**目标**：使用 C++11 的类型安全空指针。

**执行动作**：

- 全局搜索并替换 `NULL` 为 `nullptr`
- 约 278 处替换，涉及约 88 个文件
- 覆盖 `src/plot/` 和 `src/plot3d/` 所有源文件

**涉及文件**：`src/plot/` 和 `src/plot3d/` 下约 88 个 `.h` 和 `.cpp` 文件

### Phase 4：C 风格强制转换 → static_cast

**目标**：使用 C++ 类型安全的强制转换。

**执行动作**：

- 将 `src/plot3d/` 模块中的 C 风格类型转换替换为 `static_cast` 或 `reinterpret_cast`
- 包含 `int`、`GLfloat*`、`GLubyte*` 等类型的转换

**涉及文件**：

- `src/plot3d/qwt3d_autoscaler.cpp`（约 4 处 `int` 转换）
- `src/plot3d/qwt3d_coordsys.cpp`（约 8 处 `int` 转换）
- `src/plot3d/qwt3d_color.cpp`（约 3 处转换）
- `src/plot3d/qwt3d_io_gl2ps.cpp`（`GLfloat*`、`GLubyte*` 等指针转换）

### Phase 5：typedef → using 声明

**目标**：使用 C++11 的类型别名语法。

**执行动作**：

- 将传统 `typedef` 语法替换为现代 `using` 别名声明
- 示例：

  ```cpp
  // 旧语法
  typedef int QwtAxisId;
  // 新语法
  using QwtAxisId = int;
  ```

**涉及文件**：

- `src/plot/qwt_axis_id.h`
- `src/plot/qwt_clipper.cpp`
- `src/plot/qwt_date.cpp`
- `src/plot/qwt_plot_dict.h`
- `src/plot/qwt_polar_itemdict.h`
- `src/plot/qwt_raster_data.h`
- `src/plot/qwt_text.cpp`
- `src/plot3d/qwt3d_types.h`（8 处转换）
- `src/plot3d/qwt3d_types.cpp`
- `src/plot3d/qwt3d_io.h`（3 处转换）
- `src/plot3d/qwt3d_plot.h`（2 处转换）

### Phase 6：迭代器现代化

**目标**：使用 C++11 range-based for 循环。

**执行动作**：

- 将旧式 `const_iterator` 循环替换为 range-based for
- 修复 `auto` → `auto*` 指针类型推导问题

**涉及文件**：

- `src/plot/qwt_painter.cpp`（`QRegion::const_iterator` → range-based for）
- `src/plot/qwt_widget_overlay.cpp`（2 处 `QRegion::const_iterator` 转换）
- `src/plot/qwt_dyngrid_layout.cpp`（`const_iterator` → range-based for）
- `src/plot/qwt_plot_panner.cpp`（`auto` → `auto*` 修复）
- `src/plot3d/qwt3d_drawable.cpp`（`std::list<>::iterator` → range-based for）

### Phase 7：Qt 容器安全迭代

**目标**：防止 Qt 隐式共享容器在 range-based for 中触发深拷贝。

**执行动作**：

- 对 Qt 容器（如 `QList`、`QVector` 等隐式共享容器）的 range-based for 循环添加 `qwt_as_const` 包装
- 这是因为 Qt 容器使用 Copy-on-Write 机制，非 const 迭代器会触发 `detach()` 导致不必要的深拷贝
- 示例：

  ```cpp
  // 修改前（可能触发深拷贝）
  for (const auto& item : container) { ... }
  // 修改后（安全迭代）
  for (const auto& item : qwt_as_const(container)) { ... }
  ```

**涉及文件**：

- `src/plot/qwt_dyngrid_layout.cpp`

### Phase 8：CMake 构建系统适配

**目标**：根据 Qt 版本自动选择合适的 C++ 标准。

**执行动作**：

1. 将 Qt 版本检测提前到 `project()` 调用之前
2. 根据 Qt 版本条件设置 C++ 标准：

   ```cmake
   if(QT_VERSION_MAJOR GREATER_EQUAL 6)
       set(CMAKE_CXX_STANDARD 17)  # Qt 6 强制要求 C++17
   else()
       set(CMAKE_CXX_STANDARD 11)  # Qt 5 使用 C++11
   endif()
   ```

3. 更新模块 CMakeLists.txt 以适应文件迁移

**涉及文件**：

- `CMakeLists.txt`（根目录）
- `src/plot/CMakeLists.txt`

## 编译验证

重构完成后使用 MSVC (Visual Studio 2019) 进行了完整编译验证：

| 模块 | 编译文件数 | 结果 | 输出 |
|------|-----------|------|------|
| plot (2D) | 139/139 | 全部成功 | `qwtplotd.dll` |
| plot3d (3D) | 28/28 | 全部成功 | `qwtplot3dd.dll` |

### 遇到的问题及解决方案

**问题**：初始将 CMake C++ 标准设置为 C++11 后，Qt 6.7.3 编译失败，报错 `Cannot open include file: 'type_traits'`。

**原因**：Qt 6 内部代码依赖 C++17 特性，强制要求编译器使用 C++17 标准。

**解决方案**：采用条件编译策略，Qt 6 使用 C++17，Qt 5 使用 C++11。代码层面通过兼容性宏（`qwt_make_unique`、`qwt_as_const`）确保 C++11 兼容性，CMake 层面根据 Qt 版本自动选择编译标准。

## 兼容性宏参考

| 宏 | C++11 行为 | C++14+ 行为 | 用途 |
|----|-----------|-------------|------|
| `qwt_make_unique<T>(args...)` | 自定义实现（`new T(...)` 包装） | 委托给 `std::make_unique` | PIMPL 模式中创建实现对象 |
| `qwt_as_const(obj)` | 使用 `qAsConst`（Qt 提供） | C++17 使用 `std::as_const` | Qt 容器安全只读迭代 |
| `QWT_CONSTEXPR` | `constexpr` | `constexpr` | 编译期常量（已简化） |

## 工具与环境

- **AI 工具**：Qoder
- **编译器**：MSVC (Visual Studio 2019 Community)
- **构建系统**：CMake
- **Qt 版本**：Qt 6.7.3（验证编译）
- **重构日期**：2026-03-06
- **提交分支**：`cpp11`
- **提交哈希**：`4db976790bcdae840d9a1ab11aa616bd684f4ea6`

---

此文件由Qoder生成
