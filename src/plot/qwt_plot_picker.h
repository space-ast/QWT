/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/

#ifndef QWT_PLOT_PICKER_H
#define QWT_PLOT_PICKER_H

#include "qwt_global.h"
#include "qwt_picker.h"
#include "qwt_axis_id.h"

class QwtPlot;
class QPointF;
class QRectF;

#if QT_VERSION < 0x060000
template< typename T >
class QVector;
#endif

/**
 * \if ENGLISH
 * @brief QwtPlotPicker provides selections on a plot canvas
 * @details QwtPlotPicker is a QwtPicker tailored for selections on
 *          a plot canvas. It is set to a x-Axis and y-Axis and
 *          translates all pixel coordinates into this coordinate system.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotPicker 提供绘图画布上的选择功能
 * @details QwtPlotPicker 是一款专为绘图画布选择场景设计的 QwtPicker 类。
 *          它会绑定到一个 X 轴和一个 Y 轴，并将所有像素坐标转换为该坐标系下的坐标。
 * \endif
 */
class QWT_EXPORT QwtPlotPicker : public QwtPicker
{
    Q_OBJECT

public:
    /// Constructor
    explicit QwtPlotPicker(QWidget* canvas);
    /// Destructor
    virtual ~QwtPlotPicker();

    /// Constructor with axes
    explicit QwtPlotPicker(QwtAxisId xAxisId, QwtAxisId yAxisId, QWidget*);

    /// Constructor with axes, rubber band and tracker mode
    explicit QwtPlotPicker(QwtAxisId xAxisId, QwtAxisId yAxisId, RubberBand rubberBand, DisplayMode trackerMode, QWidget*);

    /// Set the axes
    virtual void setAxes(QwtAxisId xAxisId, QwtAxisId yAxisId);

    /// Get the x-axis
    QwtAxisId xAxis() const;
    /// Get the y-axis
    QwtAxisId yAxis() const;

    /// Get the plot
    QwtPlot* plot();
    /// Get the plot (const version)
    const QwtPlot* plot() const;

    /// Get the canvas
    QWidget* canvas();
    /// Get the canvas (const version)
    const QWidget* canvas() const;

Q_SIGNALS:

    /**
     * \if ENGLISH
     * @brief Signal emitted in case of QwtPickerMachine::PointSelection
     * @param pos Selected point
     * \endif
     * 
     * \if CHINESE
     * @brief 在 QwtPickerMachine::PointSelection 情况下发出的信号
     * @param pos 选中的点
     * \endif
     */
    void selected(const QPointF& pos);

    /**
     * \if ENGLISH
     * @brief Signal emitted in case of QwtPickerMachine::RectSelection
     * @param rect Selected rectangle
     * \endif
     * 
     * \if CHINESE
     * @brief 在 QwtPickerMachine::RectSelection 情况下发出的信号
     * @param rect 选中的矩形
     * \endif
     */
    void selected(const QRectF& rect);

    /**
     * \if ENGLISH
     * @brief Signal emitting the selected points at the end of a selection
     * @param pa Selected points
     * \endif
     * 
     * \if CHINESE
     * @brief 在选择结束时发出选中点的信号
     * @param pa 选中的点
     * \endif
     */
    void selected(const QVector< QPointF >& pa);

    /**
     * \if ENGLISH
     * @brief Signal emitted when a point has been appended to the selection
     * @param pos Position of the appended point
     * @sa append(), moved()
     * \endif
     * 
     * \if CHINESE
     * @brief 当一个点被添加到选择中时发出的信号
     * @param pos 添加点的位置
     * @sa append(), moved()
     * \endif
     */
    void appended(const QPointF& pos);

    /**
     * \if ENGLISH
     * @brief Signal emitted whenever the last appended point of the selection has been moved
     * @param pos Position of the moved last point of the selection
     * @sa move(), appended()
     * \endif
     * 
     * \if CHINESE
     * @brief 当选择中最后添加的点被移动时发出的信号
     * @param pos 选择中最后一个点的移动位置
     * @sa move(), appended()
     * \endif
     */
    void moved(const QPointF& pos);

protected:
    /// Get the scale rectangle
    QRectF scaleRect() const;

    /// Inverse transform a rectangle
    QRectF invTransform(const QRect&) const;
    /// Transform a rectangle
    QRect transform(const QRectF&) const;

    /// Inverse transform a point
    QPointF invTransform(const QPoint&) const;
    /// Transform a point
    QPoint transform(const QPointF&) const;

    /// Get the tracker text for a point
    virtual QwtText trackerText(const QPoint&) const override;
    /// Get the tracker text for a point (floating point)
    virtual QwtText trackerTextF(const QPointF&) const;

    /// Move the selection
    virtual void move(const QPoint&) override;
    /// Append a point to the selection
    virtual void append(const QPoint&) override;
    /// End the selection
    virtual bool end(bool ok = true) override;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
