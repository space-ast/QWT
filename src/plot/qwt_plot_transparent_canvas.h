#ifndef QWT_PLOT_TRANSPARENT_CANVAS_H
#define QWT_PLOT_TRANSPARENT_CANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"
#include <qframe.h>

/**
 * \if ENGLISH
 * @brief A transparent canvas for QwtPlot
 * @details QwtPlotTransparentCanvas provides a transparent canvas for QwtPlot
 *          that allows the background to show through.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlot 的透明画布
 * @details QwtPlotTransparentCanvas 为 QwtPlot 提供一个透明画布，
 *          允许背景透显出来。
 * \endif
 */
class QWT_EXPORT QwtPlotTransparentCanvas : public QFrame, public QwtPlotAbstractCanvas
{
    Q_OBJECT
public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    explicit QwtPlotTransparentCanvas(QwtPlot* plot = nullptr);
    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotTransparentCanvas();
public Q_SLOTS:
    /**
     * \if ENGLISH
     * @brief Replot the canvas
     * \endif
     */
    virtual void replot();

protected:
    /**
     * \if ENGLISH
     * @brief Paint event handler
     * \endif
     */
    virtual void paintEvent(QPaintEvent* event) override;
    /**
     * \if ENGLISH
     * @brief Draw the border
     * \endif
     */
    virtual void drawBorder(QPainter* painter) override;
    /**
     * \if ENGLISH
     * @brief Get the border path
     * \endif
     */
    virtual QPainterPath borderPath(const QRect& rect) const;
};

#endif  // QWT_PLOT_TRANSPARENT_CANVAS_H
