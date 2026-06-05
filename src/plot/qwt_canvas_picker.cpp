#include "qwt_canvas_picker.h"
#include <QWidget>
#include "qwt_plot.h"

/**
 * @brief Constructor with canvas widget
 *
 * @param[in] canvas Canvas widget to attach the picker to
 */
QwtCanvasPicker::QwtCanvasPicker(QWidget* canvas) : QwtPicker(canvas)
{
}

/**
 * @brief Destructor
 */
QwtCanvasPicker::~QwtCanvasPicker()
{
}

/**
 * @brief Get the associated plot
 *
 * @details Finds the parent QwtPlot by traversing the widget hierarchy.
 *
 * @return Pointer to the associated plot, or nullptr if not found
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
 * @brief Get the associated plot (const version)
 *
 * @details Finds the parent QwtPlot by traversing the widget hierarchy.
 *
 * @return Const pointer to the associated plot, or nullptr if not found
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
 * @brief Get the canvas widget
 *
 * @return Pointer to the canvas widget
 */
QWidget* QwtCanvasPicker::canvas()
{
    return parentWidget();
}

/**
 * @brief Get the canvas widget (const version)
 *
 * @return Const pointer to the canvas widget
 */
const QWidget* QwtCanvasPicker::canvas() const
{
    return parentWidget();
}
