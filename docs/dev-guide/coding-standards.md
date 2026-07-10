# Coding Standards

This document defines the coding standards for the Qwt project to ensure consistent code style and quality.

## Key Requirements

**Features**

- ✅ **Code Formatting**: Automatic formatting using `.clang-format` configuration
- ✅ **C++11 Compatibility**: Use the project-provided compatibility macros
- ✅ **Modern C++ Style**: Use modern syntax such as `override`, `nullptr`, `using`, etc.
- ✅ **Qt Best Practices**: Follow Qt development guidelines

## Code Formatting

The project uses a `.clang-format` configuration file for code formatting.

!!! tip "Formatting Suggestions"
    - Run clang-format before committing code
    - Configure your IDE for automatic formatting (triggered on save)
    - Maintain consistency with existing code style

## C++11 Compatibility Macros

The project provides a set of compatibility macros to ensure code compiles in both C++11 and C++17 environments.

### Smart Pointers

```cpp
// Create unique_ptr (C++11 compatible)
auto ptr = qwt_make_unique<MyClass>(args...);
```

### Container Iteration

```cpp
// Safe Qt container iteration to prevent deep copies
for (const auto& item : qwt_as_const(container)) {
    // Process item
}
```

### Compile-Time Constants

```cpp
// Use constexpr for compile-time constants
constexpr int MAX_SAMPLES = 1000;
```

## Key Code Standards

### 1. Use override and final

Use standard C++ keywords instead of legacy macros:

```cpp
// ❌ Wrong: using legacy macro
virtual void draw(QPainter* painter) QWT_OVERRIDE;

// ✅ Correct: using standard keyword
virtual void draw(QPainter* painter) override;
```

| Legacy Macro | New Keyword |
|------|----------|
| `QWT_OVERRIDE` | `override` |
| `QWT_FINAL` | `final` |

### 2. Use nullptr Instead of NULL

```cpp
// ❌ Wrong
QwtPlot* plot = NULL;

// ✅ Correct
QwtPlot* plot = nullptr;
```

### 3. Use static_cast Instead of C-Style Casts

```cpp
// ❌ Wrong: C-style cast
int value = (int)doubleValue;

// ✅ Correct: static_cast
int value = static_cast<int>(doubleValue);
```

### 4. Use using Instead of typedef

```cpp
// ❌ Wrong: old-style typedef
typedef QVector<QwtSample> SampleVector;

// ✅ Correct: using alias
using SampleVector = QVector<QwtSample>;
```

### 5. Qt Container Iteration Standard

Use `qwt_as_const` to prevent Qt container deep copies:

```cpp
// ❌ May cause deep copy
for (const auto& item : container) { }

// ✅ Safe iteration
for (const auto& item : qwt_as_const(container)) { }
```

!!! warning "Important"
    Qt containers may undergo implicit sharing detachment (deep copy) during iteration. Using `qwt_as_const` avoids this issue.

## Qt Development Best Practices

### Signal-Slot Syntax

Use the Qt5+ signal-slot syntax:

```cpp
// ❌ Wrong: using old SIGNAL/SLOT macros
connect(sender, SIGNAL(valueChanged(int)), receiver, SLOT(updateValue(int)));

// ✅ Correct: using new function pointer syntax
connect(sender, &Sender::valueChanged, receiver, &Receiver::updateValue);
```

### Q_PROPERTY Usage

Use the `Q_PROPERTY` macro correctly:

```cpp
class MyWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    QString title() const;
    void setTitle(const QString& title);

Q_SIGNALS:
    void titleChanged(const QString& title);
};
```

### Keyword Usage Standards

Use the macros officially recommended by Qt:

```cpp
// ✅ Correct
Q_SIGNALS:    // instead of signals:
Q_SLOT:       // instead of slot:
public Q_SLOTS:  // instead of public slots:
```

!!! warning "Prohibited"
    Do not use lowercase macros such as `slot` or `signal`, as they may cause issues with certain compilers.

## Naming Conventions

### Class Names

- Use camel case with an uppercase first letter
- Prefix with `Qwt`: `QwtPlotCurve`

### Method Names

- Use camel case with a lowercase first letter
- No `get` prefix for getters: `boundingRect()`
- Use `set` prefix for setters: `setSamples()`

### Variable Names

- Member variables use the `m_` prefix: `m_data`
- Local variables use camel case: `sampleCount`
- Constants use all uppercase: `MAX_WIDTH`

## Related Documentation

- [Comment Standards](comment-standards.md)
- [PIMPL Pattern Guide](pimpl-pattern.md)
