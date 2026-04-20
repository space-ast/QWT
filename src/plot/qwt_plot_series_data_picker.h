/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#ifndef QWT_PLOT_SERIES_DATA_PICKER_H
#define QWT_PLOT_SERIES_DATA_PICKER_H
#include <QList>
#include <QPointF>
#include "qwt_canvas_picker.h"
#include "qwt_text.h"
class QwtPlot;
class QwtPlotItem;

/**
 * \if ENGLISH
 * @brief A plot data picker class for displaying current y-values or nearest points
 * @details QwtPlotSeriesDataPicker is a plot data picker class that displays current y-values
 *          or the nearest points to the mouse cursor position.
 * \endif
 * 
 * \if CHINESE
 * @brief 这是一个绘图数据拾取显示类，用于显示当前的y值，或者显示最近点
 * @details QwtPlotSeriesDataPicker 是一个绘图数据拾取显示类，用于显示当前的y值，
 *          或者显示最接近鼠标光标位置的点。
 * \endif
 */
class QWT_EXPORT QwtPlotSeriesDataPicker : public QwtCanvasPicker
{
    Q_OBJECT
    QWT_DECLARE_PRIVATE(QwtPlotSeriesDataPicker)
public:
    /**
     * \if ENGLISH
     * @brief Pick modes
     * \endif
     * 
     * \if CHINESE
     * @brief 拾取模式
     * \endif
     */
    enum PickSeriesMode
    {
        PickYValue,  ///< Pick y-value (default)
        PickNearestPoint  ///< Pick the nearest point to the mouse cursor position (this mode may be time-consuming, use with caution when there are many curve points)
    };

    /**
     * \if ENGLISH
     * @brief Text placement options
     * \endif
     * 
     * \if CHINESE
     * @brief 文本放置选项
     * \endif
     */
    enum TextPlacement
    {
        TextPlaceAuto,         ///< Auto placement (top for pick y, follow mouse for pick nearest)
        TextFollowOnTop,       ///< On top of the plot area (default)
        TextFollowOnBottom,    ///< On bottom of the plot area
        TextFollowMouse,       ///< Follow mouse pointer
        TextOnCanvasTopRight,  ///< Text on canvas top right
        TextOnCanvasTopLeft,   ///< Text on canvas top left
        TextOnCanvasBottomRight,  ///< Text on canvas bottom right
        TextOnCanvasBottomLeft,   ///< Text on canvas bottom left
        TextOnCanvasTopAuto,  ///< Text on canvas top, left or right auto-detected based on mouse position
        TextOnCanvasBottomAuto  ///< Text on canvas bottom, left or right auto-detected based on mouse position
    };

    /**
     * \if ENGLISH
     * @brief Interpolation modes
     * \endif
     * 
     * \if CHINESE
     * @brief 插值模式
     * \endif
     */
    enum InterpolationMode
    {
        NoInterpolation,     ///< No interpolation, use nearest data point
        LinearInterpolation  ///< Linear interpolation between adjacent data points
    };

    /**
     * \if ENGLISH
     * @brief Feature point structure
     * \endif
     * 
     * \if CHINESE
     * @brief 特征点结构
     * \endif
     */
    struct FeaturePoint
    {
        QwtPlotItem* item { nullptr };  ///< Corresponding item
        QPointF feature { 0, 0 };       ///< Feature point
        size_t index { 0 };             ///< Index in the item
    };

public:
    /// Constructor
    explicit QwtPlotSeriesDataPicker(QWidget* canvas);
    /// Destructor
    ~QwtPlotSeriesDataPicker();

    /// Set pick mode
    void setPickMode(PickSeriesMode mode);
    /// Get pick mode
    PickSeriesMode pickMode() const;

    /// Set text placement
    void setTextArea(TextPlacement t);
    /// Get text placement
    TextPlacement textArea() const;

    /// Set interpolation mode
    void setInterpolationMode(InterpolationMode mode);
    /// Get interpolation mode
    InterpolationMode interpolationMode() const;
    /// Check if interpolation is enabled
    bool isInterpolation() const;

    /// Set nearest search window size
    void setNearestSearchWindowSize(int windowSize);
    /// Get nearest search window size
    int nearestSearchWindowSize() const;

    /// Enable/disable feature point drawing
    void setEnableDrawFeaturePoint(bool on = true);
    /// Check if feature point drawing is enabled
    bool isEnableDrawFeaturePoint() const;

    /// Set feature point size
    void setDrawFeaturePointSize(int px);
    /// Get feature point size
    int drawFeaturePointSize() const;

    /**
     * \if ENGLISH
     * @brief Returns the list of feature points currently picked by the tracker
     * @return List of FeaturePoint structures containing picked data coordinates
     * @note The returned points correspond to the current tracker position.
     *       Call this method from a clicked/doubleClicked signal handler to
     *       get data at the click position.
     * \endif
     * \if CHINESE
     * @brief 返回当前 tracker 拾取到的特征点列表
     * @return 包含拾取数据坐标的 FeaturePoint 结构列表
     * @note 返回的点对应当前 tracker 位置。
     *       从 clicked/doubleClicked 信号处理器中调用此方法
     *       可获取点击位置的数据。
     * \endif
     */
    QList<FeaturePoint> featurePoints() const;

    /// Set text background brush
    void setTextBackgroundBrush(const QBrush& br);
    /// Get text background brush
    QBrush textBackgroundBrush() const;

    /// Set text alignment
    void setTextAlignment(Qt::Alignment al);
    /// Get text alignment
    Qt::Alignment textAlignment() const;

    // 是否显示x值
    void setEnableShowXValue(bool on);
    bool isEnableShowXValue() const;

    // 设置trackerRect在TextFollowMouse模式下的偏移量
    void setTextTrackerOffset(const QPoint& offset);
    QPoint textTrackerOffset() const;

    // 顶部矩形文字
    QwtText trackerText(const QPoint& pos) const override;

    // 让矩形在最顶部
    QRect trackerRect(const QFont& f) const override;

    // 绘制rubberband
    virtual void drawRubberBand(QPainter* painter) const override;

    // 手动设置位置
    virtual void setTrackerPosition(const QPoint& pos) override;

protected:
    // 获取绘图区域屏幕坐标pos上，的所有可拾取的y值,返回获取的个数
    virtual int pickYValue(const QwtPlot* p, const QPoint& pos, bool interpolate = false);
    // 获取绘图区域屏幕坐标pos上，可拾取的最近的一个点，(基于窗口实现快速索引)
    virtual int pickNearestPoint(const QwtPlot* plot, const QPoint& pos, int windowSize = -5);

    virtual void widgetMousePressEvent(QMouseEvent* event) override;
    virtual void widgetMouseDoubleClickEvent(QMouseEvent* event) override;
Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Emitted when the user left-clicks on the plot canvas
     * @param picker Pointer to the picker that was clicked
     * @param pos Screen position of the click event
     * @note A double-click will trigger clicked() before doubleClicked().
     *       Connect only one of these signals if you need to distinguish
     *       single-click from double-click.
     * \endif
     * \if CHINESE
     * @brief 当用户在绘图画布上左键单击时发出
     * @param picker 被点击的picker指针
     * @param pos 点击事件的屏幕位置
     * @note 双击时会先触发 clicked() 再触发 doubleClicked()。
     *       如需区分单击与双击，请只连接其中一个信号。
     * \endif
     */
    void clicked(QwtPlotSeriesDataPicker* picker, const QPoint& pos);

    /**
     * \if ENGLISH
     * @brief Emitted when the user double-left-clicks on the plot canvas
     * @param picker Pointer to the picker that was double-clicked
     * @param pos Screen position of the double-click event
     * @note A double-click also triggers clicked() before this signal.
     * \endif
     * \if CHINESE
     * @brief 当用户在绘图画布上左键双击时发出
     * @param picker 被双击的picker指针
     * @param pos 双击事件的屏幕位置
     * @note 双击时也会先触发 clicked()，再触发此信号。
     * \endif
     */
    void doubleClicked(QwtPlotSeriesDataPicker* picker, const QPoint& pos);

private Q_SLOTS:
    // item删除的槽，用于更新记录
    void onPlotItemDetached(QwtPlotItem* item, bool on);
    void onParasitePlotAttached(QwtPlot* parasiteplot, bool on);

protected:
    // 生成一个item的文字内容
    virtual QString valueString(const QList< FeaturePoint >& fps) const;
    // 绘制特征点
    virtual void drawFeaturePoint(QPainter* painter, const QwtPlot* plot, const QwtPlotItem* item, const QPointF& itemPoint) const;
    // 鼠标移动
    virtual void move(const QPoint& pos) override;
    // 格式化为坐标轴对应的内容，针对时间轴，value是一个大浮点数，用户需要看到的是2024-10-01这样的数字
    QString formatAxisValue(double value, int axisId, QwtPlot* plot) const;

private:
    // 绘制特征点，所谓特征点就是捕获到的点
    void drawAllFeaturePoints(QPainter* painter) const;
    // 更新特征点
    void updateFeaturePoint(const QPoint& pos);
    //
    QRect ensureRectInBounds(const QRect& rect, const QRect& bounds) const;
};

#endif  // QWT_PLOT_SERIES_DATA_PICKER_H
