/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#ifndef QWTCANVASPICKER_H
#define QWTCANVASPICKER_H
#include "qwt_picker.h"
// qt
class QWidget;
// qwt
class QwtPlot;

/**
 * \if ENGLISH
 * @brief Base picker class specifically for canvas
 *
 * @details Provides convenient access methods for canvas and plot,
 * serving as a base class for other canvas-related tools.
 * \endif
 *
 * \if CHINESE
 * @brief 专门针对 canvas 的 picker 基类
 *
 * @details 提供 canvas 和 plot 的便捷访问方法，
 * 作为其他 canvas 相关工具的基类。
 * \endif
 */
class QWT_EXPORT QwtCanvasPicker : public QwtPicker
{
    Q_OBJECT
public:
    // Constructor with canvas widget
    explicit QwtCanvasPicker(QWidget* canvas);
    // Destructor
    ~QwtCanvasPicker();

    // Get the associated plot (non-const version)
    QwtPlot* plot();
    // Get the associated plot (const version)
    const QwtPlot* plot() const;

    // Get the canvas widget (non-const version)
    QWidget* canvas();
    // Get the canvas widget (const version)
    const QWidget* canvas() const;
};

#endif  // QWTCANVASPICKER_H
