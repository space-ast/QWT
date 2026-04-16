#include "qwt_canvas_picker.h"
#include <QWidget>
#include "qwt_plot.h"

/**
 * \if ENGLISH
 * @brief Constructor with canvas widget
 *
 * @param[in] canvas Canvas widget to attach the picker to
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数，带画布控件
 *
 * @param[in] canvas 要附加 picker 的画布控件
 * \endif
 */
QwtCanvasPicker::QwtCanvasPicker(QWidget* canvas) : QwtPicker(canvas)
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtCanvasPicker::~QwtCanvasPicker()
{
}

/**
 * \if ENGLISH
 * @brief Get the associated plot
 *
 * @details Finds the parent QwtPlot by traversing the widget hierarchy.
 *
 * @return Pointer to the associated plot, or nullptr if not found
 * \endif
 *
 * \if CHINESE
 * @brief 获取关联的绘图
 *
 * @details 通过遍历控件层次结构查找父 QwtPlot。
 *
 * @return 关联绘图的指针，如果未找到则返回 nullptr
 * \endif
 */
QwtPlot* QwtCanvasPicker::plot()
{
    QWidget* w = canvas();
    if (w) {
        w = w->parentWidget();
    }

    return qobject_cast< QwtPlot* >(w);
}

/**
 * \if ENGLISH
 * @brief Get the associated plot (const version)
 *
 * @details Finds the parent QwtPlot by traversing the widget hierarchy.
 *
 * @return Const pointer to the associated plot, or nullptr if not found
 * \endif
 *
 * \if CHINESE
 * @brief 获取关联的绘图（const 版本）
 *
 * @details 通过遍历控件层次结构查找父 QwtPlot。
 *
 * @return 关联绘图的常量指针，如果未找到则返回 nullptr
 * \endif
 */
const QwtPlot* QwtCanvasPicker::plot() const
{
    const QWidget* w = canvas();
    if (w) {
        w = w->parentWidget();
    }

    return qobject_cast< const QwtPlot* >(w);
}

/**
 * \if ENGLISH
 * @brief Get the canvas widget
 *
 * @return Pointer to the canvas widget
 * \endif
 *
 * \if CHINESE
 * @brief 获取画布控件
 *
 * @return 画布控件的指针
 * \endif
 */
QWidget* QwtCanvasPicker::canvas()
{
    return parentWidget();
}

/**
 * \if ENGLISH
 * @brief Get the canvas widget (const version)
 *
 * @return Const pointer to the canvas widget
 * \endif
 *
 * \if CHINESE
 * @brief 获取画布控件（const 版本）
 *
 * @return 画布控件的常量指针
 * \endif
 */
const QWidget* QwtCanvasPicker::canvas() const
{
    return parentWidget();
}
