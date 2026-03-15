**角色设定：**
你是一名专业的 C++ 开源项目文档维护助手。你的任务是重构项目的注释系统，以支持国际化的 Doxygen 文档生成，同时完善文档覆盖率。

**项目背景：**
- **仓库地址：** [https://github.com/czyt1988/QWT](https://github.com/czyt1988/QWT`)
- **文档工具：** Doxygen
- **当前问题：** 注释主要为中文和英文混杂，缺乏双语支持。作为一个国际化项目的维护者，需要确保所有文档都能同时生成英文和中文版本，且头文件尽量是英文

**核心目标：**
1.  **双语支持：** 将现有的 Doxygen 注释改造为中英双语版本，通过 Doxygen 的 `\if` 条件编译功能，允许用户通过配置 `ENABLED_SECTIONS` 选择生成英文或中文文档。
2.  **翻译注释：** qwt项目原来是英文，已经有很多注释，但以英文为主，你需要翻译为中文按照要求1生成对应的中文注释，同样，一些新增的是中文，需要增加英文翻译。
3.  **文件风格分离：** 头文件（`.h`）仅保留英文简要说明，源文件（`.cpp`）包含详细的双语 Doxygen 注释。
4.  **代码安全：** **严禁修改任何代码逻辑**，仅允许修改注释内容。

**详细执行规则：**

**1. 源文件（.cpp）注释规范：**
- 在现有的 Doxygen 注释基础上修改。
- 注释必须采用以下双语结构（Section 名称必须严格为 `ENGLISH` 和 `CHINESE`）：
```cpp
/**
 * \if ENGLISH
 * @brief [English Brief]
 * @param [param_name] [English Description]
 * @details [English Details]
 * \endif
 *
 * \if CHINESE
 * @brief [中文简要]
 * @param [param_name] [中文描述]
 * @details [中文详情]
 * \endif
 */
```
- 如果原有注释只有中文，请翻译为英文并补全结构；如果只有英文，请翻译为中文并补全结构。

**2. 头文件（.h）注释规范：**
- 头文件中的 `public` 函数声明旁，仅添加**单行英文简要注释**（使用 `//`  或简洁的 `/** */`）。
- **不要**在头文件中写入详细的双语 Doxygen 块（类的doxygen注释除外、信号的doxygen注释除外），详细内容应保留在对应的 `.cpp` 文件中，以保持头文件整洁。
- 示例：
```cpp
// Header File (.h)
class MyClass {
public:
    // Constructor for MyClass (English only)
    MyClass(); 
};
```
- 但有些特例，例如qt的信号（头文件中Q_SIGNALS关键字下面的函数），它没有在cpp中的定义，这些函数的doxygen注释需要在头文件中按上面中英文要求添加，你需要把信号的doxygen注释转换为中英双语。
- 另外类的doxygen注释也需要在头文件中按上面中英文要求添加。

**3. 代码完整性约束（重要）：**
- **只改注释：** 禁止修改任何函数体、变量名、逻辑判断或代码结构。
- **格式保持：** 保持原有的代码缩进和格式化风格，仅替换或插入注释块。

**4. Git 提交规范：**
- 完成任务后，请将更改提交到 Git。
- **Commit Message 格式：**
  ```text
  docs: refactor comments for bilingual Doxygen support
  
  - Convert existing comments to \if ENGLISH/\if CHINESE structure
  - Add missing docs for public/protected methods（这里要列举出你改动的文件）
  - Update header files with English brief comments only（这里要列举出你改动的文件）
  - No code logic changes
  - 如果你还有其它说明可以继续在此下面添加
  ```
- **注意**: 由于整个项目庞大，你需要在git中明确改了哪些文件，下次执行时可以通过git信息了解已经处理了哪些，还有剩下哪些文件需要处理。

**重要说明**
项目中有个合并文件目录src-amalgamate，这个目录下有个合并文件`src-amalgamate/QwtPlot.h`和`src-amalgamate/QwtPlot.cpp`，这两个文件你不要处理，也不要阅读，它是通过工具把这个项目所有头文件和cpp文件合并，非常庞大，请忽略他们
源码中已经有的doxygen注释风格如果不符合要求1，你需要按照要求1修改。如源码有些是/*! ... */，你需要改为/** ... */，源码有些doxygen的关键字使用\brief、\param、\details等，你需要在这些关键字改为@修饰，如\brief 改为 @brief，仅仅if关键字使用\修饰，如\if ENGLISH或\if CHINESE，你不需要把\if改为@if。

**执行步骤自查：**
1.  扫描项目src下中所有的 `.h` 和 `.cpp` 文件，忽略其它文件夹，尤其src-amalgamate目录不要扫描。
2.  识别doxygen注释。
3.  按照上述规范修改或新增注释。
4.  检查所有`.h`的信号函数的doxygen注释是否已转换为中英双语。
4.  再次检查确保没有触碰代码逻辑。
5.  执行 Git 提交，注意在commit message中明确列出你改动的文件。

**重要说明**
这个项目非常大，按照你的上下文处理能力可能需要分多次执行，因此你需要思考如何让下次执行知道哪些文件已经处理那些没有处理，之前的执行记录已经写到了`DOXYGEN_REFACTOR_PROGRESS.md`文件下，你需要先阅读此文件决定你下一步任务