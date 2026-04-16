#ifndef QWT_PLOT_PANNER_H
#define QWT_PLOT_PANNER_H
#include "qwt_global.h"
#include "qwt_picker.h"

// qt
class QWidget;

// qwt
class QwtPlot;

/**
 * \if ENGLISH
 * @brief QwtPlotPanner provides panning of a plot canvas
 * @details QwtPlotPanner is a QwtPicker that translates all pixel coordinates
 *          into plot coordinates and provides panning functionality for the plot canvas.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotPanner 提供绘图画布的平移功能
 * @details QwtPlotPanner 是一个 QwtPicker，它将所有像素坐标转换为绘图坐标，并为绘图画布提供平移功能。
 * \endif
 */
class QWT_EXPORT QwtPlotPanner : public QwtPicker
{
    Q_OBJECT
    QWT_DECLARE_PRIVATE(QwtPlotPanner)
public:
    // Constructor
    explicit QwtPlotPanner(QWidget* canvas);
    // Destructor
    virtual ~QwtPlotPanner();

    // Get the canvas widget
    QWidget* canvas();
    // Get the canvas widget (const)
    const QWidget* canvas() const;

    // Get the plot widget
    QwtPlot* plot();
    // Get the plot widget (const)
    const QwtPlot* plot() const;

    // Set the orientations for panning
    void setOrientations(Qt::Orientations);
    // Get the orientations for panning
    Qt::Orientations orientations() const;
    // Check if an orientation is enabled
    bool isOrientationEnabled(Qt::Orientation) const;

    // Set the mouse button and modifiers for panning
    void setMouseButton(Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    // Get the mouse button and modifiers for panning
    void getMouseButton(Qt::MouseButton& button, Qt::KeyboardModifiers& modifiers) const;

public Q_SLOTS:
    // Move the canvas by dx, dy
    void moveCanvas(int dx, int dy);

Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Signal emitted after the canvas has been panned
     * @param dx Offset in x-direction
     * @param dy Offset in y-direction
     * \endif
     * 
     * \if CHINESE
     * @brief 画布平移后发出的信号
     * @param dx x 方向的偏移
     * @param dy y 方向的偏移
     * \endif
     */
    void panned(int dx, int dy);

protected:
    /// Handle mouse press events
    virtual void widgetMousePressEvent(QMouseEvent* mouseEvent) override;
    /// Move the selection
    virtual void move(const QPoint&) override;
    /// End the panning operation
    virtual bool end(bool ok = true) override;

private:
    /// Initialize the panner
    void init();
};
#endif  // QWT_PLOT_PANNER_H
