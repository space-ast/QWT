/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#include "qwt_figure.h"
// Qt
#include <QPainter>
#include <QPointer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QWidgetItem>
#include <QMap>
#include <QDebug>
// qwt
#include "qwt_figure_layout.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_layout.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_transparent_canvas.h"
#include "qwt_parasite_plot_layout.h"

#ifndef QWTFIGURE_SAFEGET_LAY
#define QWTFIGURE_SAFEGET_LAY(lay)                                                                                     \
    QwtFigureLayout* lay = qobject_cast< QwtFigureLayout* >(layout());                                                 \
    if (!lay) {                                                                                                        \
        return;                                                                                                        \
    }
#endif

#ifndef QWTFIGURE_SAFEGET_LAY_RET
#define QWTFIGURE_SAFEGET_LAY_RET(lay, ret)                                                                            \
    QwtFigureLayout* lay = qobject_cast< QwtFigureLayout* >(layout());                                                 \
    if (!lay) {                                                                                                        \
        return ret;                                                                                                    \
    }
#endif

class QwtFigure::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtFigure)
public:
    PrivateData(QwtFigure* p);
    //绘图将要移除
    void plotWillRemove(QwtPlot* p);
    // 存储对齐配置的结构体
    struct AlignmentConfig
    {
        QList< QwtPlot* > plots;  // 需要对齐的plot列表
        int axisId;               // 需要对齐的轴ID
    };

public:
    QBrush faceBrush { Qt::white };             ///< Background color of the figure / 图形背景颜色
    QColor edgeColor { Qt::black };             ///< Border color of the figure / 图形边框颜色
    int edgeLineWidth { 0 };                    ///< Border line width / 边框线宽
    QPointer< QwtPlot > currentAxes;            ///< Current active axes / 当前活动坐标轴
    QList< AlignmentConfig > alignmentConfigs;  // 所有对齐配置
};

QwtFigure::PrivateData::PrivateData(QwtFigure* p) : q_ptr(p)
{
}

void QwtFigure::PrivateData::plotWillRemove(QwtPlot* p)
{
    for(auto& alCfg : alignmentConfigs){
        alCfg.plots.removeAll(p);
    }
}

//----------------------------------------------------
// QwtFigure
//----------------------------------------------------

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] parent Parent widget
 * @param[in] f Window flags
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] parent 父窗口部件
 * @param[in] f 窗口标志
 * \endif
 */
QwtFigure::QwtFigure(QWidget* parent, Qt::WindowFlags f) : QFrame(parent, f), QWT_PIMPL_CONSTRUCT
{
    setLayout(new QwtFigureLayout());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

}

QwtFigure::~QwtFigure()
{
}

/**
 * \if ENGLISH
 * @brief Add a widget with normalized coordinates
 * @param[in] widget QWidget to add
 * @param[in] left Normalized coordinates left in range [0,1]
 * @param[in] top Normalized coordinates top in range [0,1]
 * @param[in] width Normalized coordinates width in range [0,1]
 * @param[in] height Normalized coordinates height in range [0,1]
 *
 * @note Even if the added widget is a QwtPlot, this function will not emit axesAdded signal.
 *       Use addAxes() instead if you need to add QwtPlot widgets.
 * @sa addAxes
 * \endif
 *
 * \if CHINESE
 * @brief 使用归一化坐标添加窗口
 * @param[in] widget 要添加的QWidget
 * @param[in] left 归一化坐标左边界，范围[0,1]
 * @param[in] top 归一化坐标上边界，范围[0,1]
 * @param[in] width 归一化坐标宽度，范围[0,1]
 * @param[in] height 归一化坐标高度，范围[0,1]
 *
 * @note 即使添加的窗口是QwtPlot，此函数也不会发射axesAdded信号，因此，如果你需要添加QwtPlot窗口，
 *       你应该使用addAxes函数，此函数是为了在figure窗口添加除QwtPlot以外的窗口使用的
 * @sa addAxes
 * \endif
 */
void QwtFigure::addWidget(QWidget* widget, qreal left, qreal top, qreal width, qreal height)
{
    QWTFIGURE_SAFEGET_LAY(lay)
    if (widget && widget->parentWidget() != this) {
        widget->setParent(this);
    }
    lay->addAxes(widget, left, top, width, height);
}

/**
 * \if ENGLISH
 * @brief Add a widget by grid layout
 * @details This method adds a widget to the grid layout at the specified position with optional row and column spans.
 * @param[in] widget Widget to add
 * @param[in] rowCnt Number of rows in the grid
 * @param[in] colCnt Number of columns in the grid
 * @param[in] row Grid row position (0-based)
 * @param[in] col Grid column position (0-based)
 * @param[in] rowSpan Number of rows to span (default: 1)
 * @param[in] colSpan Number of columns to span (default: 1)
 * @param[in] wspace Horizontal space between subplots [0,1]
 * @param[in] hspace Vertical space between subplots [0,1]
 *
 * @note Even if the added widget is a QwtPlot, this function will not emit axesAdded signal.
 *       Use addAxes() instead if you need to add QwtPlot widgets.
 * @sa addAxes
 * \endif
 *
 * \if CHINESE
 * @brief 添加窗口部件到网格布局
 * @details 此方法将widget添加到网格布局中的指定位置，可选择跨行和跨列。
 * @param[in] widget 要添加的widget
 * @param[in] rowCnt 网格行数
 * @param[in] colCnt 网格列数
 * @param[in] row 网格行位置（从0开始）
 * @param[in] col 网格列位置（从0开始）
 * @param[in] rowSpan 跨行数（默认：1）
 * @param[in] colSpan 跨列数（默认：1）
 * @param[in] wspace 子图之间的水平间距 [0,1]
 * @param[in] hspace 子图之间的垂直间距 [0,1]
 *
 * @note 即使添加的窗口是QwtPlot，此函数也不会发射axesAdded信号，因此，如果你需要添加QwtPlot窗口，
 *       你应该使用addAxes函数，此函数是为了在figure窗口添加除QwtPlot以外的窗口使用的
 * @sa addAxes
 * \endif
 */
void QwtFigure::addWidget(QWidget* widget, int rowCnt, int colCnt, int row, int col, int rowSpan, int colSpan, qreal wspace, qreal hspace)
{
    QWTFIGURE_SAFEGET_LAY(lay)
    if (widget && widget->parentWidget() != this) {
        widget->setParent(this);
    }
    lay->addGridAxes(widget, rowCnt, colCnt, row, col, rowSpan, colSpan, wspace, hspace);
}

/**
 * \if ENGLISH
 * @brief Add a plot with normalized coordinates
 * @details This method adds a QwtPlot to the figure using normalized coordinates in the range [0,1].
 *          The coordinates are specified as [left, bottom, width, height].
 * @param[in] plot QwtPlot to add
 * @param[in] rect Normalized coordinates [left, bottom, width, height] in range [0,1]
 *
 * @note This function will emit axesAdded signal, followed by currentAxesChanged signal
 * \endif
 *
 * \if CHINESE
 * @brief 使用归一化坐标添加绘图
 * @details 此方法使用[0,1]范围内的归一化坐标将QwtPlot添加到图形中。
 *          坐标指定为[左, 下, 宽, 高]。
 * @param[in] plot 要添加的QwtPlot
 * @param[in] rect 归一化坐标 [左, 下, 宽, 高]，范围 [0,1]
 *
 * @note 此函数会发射axesAdded信号，此信号发射后发射currentAxesChanged信号
 * \endif
 */
void QwtFigure::addAxes(QwtPlot* plot, const QRectF& rect)
{
    addAxes(plot, rect.x(), rect.y(), rect.width(), rect.height());
}

/**
 * \if ENGLISH
 * @brief Add a plot with normalized coordinates using separate parameters
 * @param[in] plot QwtPlot to add
 * @param[in] left Normalized coordinates left in range [0,1]
 * @param[in] top Normalized coordinates top in range [0,1]
 * @param[in] width Normalized coordinates width in range [0,1]
 * @param[in] height Normalized coordinates height in range [0,1]
 *
 * @note This function will emit axesAdded signal, followed by currentAxesChanged signal
 * \endif
 *
 * \if CHINESE
 * @brief 使用归一化坐标添加绘图（分离参数）
 * @param[in] plot 要添加的QwtPlot
 * @param[in] left 归一化坐标左边界，范围[0,1]
 * @param[in] top 归一化坐标上边界，范围[0,1]
 * @param[in] width 归一化坐标宽度，范围[0,1]
 * @param[in] height 归一化坐标高度，范围[0,1]
 *
 * @note 此函数会发射axesAdded信号，此信号发射后发射currentAxesChanged信号
 * \endif
 */
void QwtFigure::addAxes(QwtPlot* plot, qreal left, qreal top, qreal width, qreal height)
{
    addWidget(plot, left, top, width, height);
    Q_EMIT axesAdded(plot);
    setCurrentAxes(plot);
}

/**
 * \if ENGLISH
 * @brief Add a plot by grid layout
 * @details This method adds a QwtPlot to the grid layout at the specified position with optional row and column spans.
 * @param[in] plot QwtPlot to add
 * @param[in] rowCnt Number of rows in the grid
 * @param[in] colCnt Number of columns in the grid
 * @param[in] row Grid row position (0-based)
 * @param[in] col Grid column position (0-based)
 * @param[in] rowSpan Number of rows to span (default: 1)
 * @param[in] colSpan Number of columns to span (default: 1)
 * @param[in] wspace Horizontal space between subplots [0,1]
 * @param[in] hspace Vertical space between subplots [0,1]
 *
 * @note This function will emit axesAdded signal, followed by currentAxesChanged signal
 * \endif
 *
 * \if CHINESE
 * @brief 添加窗口部件到网格布局
 * @details 此方法将QwtPlot添加到网格布局中的指定位置，可选择跨行和跨列。
 * @param[in] plot 要添加的QwtPlot
 * @param[in] rowCnt 网格行数
 * @param[in] colCnt 网格列数
 * @param[in] row 网格行位置（从0开始）
 * @param[in] col 网格列位置（从0开始）
 * @param[in] rowSpan 跨行数（默认：1）
 * @param[in] colSpan 跨列数（默认：1）
 * @param[in] wspace 子图之间的水平间距 [0,1]
 * @param[in] hspace 子图之间的垂直间距 [0,1]
 *
 * @note 此函数会发射axesAdded信号，此信号发射后发射currentAxesChanged信号
 * \endif
 */
void QwtFigure::addGridAxes(QwtPlot* plot, int rowCnt, int colCnt, int row, int col, int rowSpan, int colSpan, qreal wspace, qreal hspace)
{
    addWidget(plot, rowCnt, colCnt, row, col, rowSpan, colSpan, wspace, hspace);
    Q_EMIT axesAdded(plot);
    setCurrentAxes(plot);
}

/**
 * @brief 改变已经添加的窗口的位置占比,如果窗口还没添加，此函数无效
 * @param widget
 * @param rect
 */
void QwtFigure::setWidgetNormPos(QWidget* widget, const QRectF& rect)
{
    QWTFIGURE_SAFEGET_LAY(lay)
    lay->setAxesNormPos(widget, rect);
    lay->invalidate();
}

/**
 * \if ENGLISH
 * @brief Get all axes (plots) in the figure (not including parasite axes)
 * @details This method returns a list of all QwtPlot objects added to the figure.
 * @param[in] byZOrder If true, sort by z-order from top to bottom
 * @return List of all QwtPlot objects (not including parasite axes)
 *
 * @note Parasite axes are not included in the returned list
 * \endif
 *
 * \if CHINESE
 * @brief 获取图形中的所有坐标轴（绘图）(不包含寄生轴)
 * @details 此方法返回添加到图形中的所有QwtPlot对象的列表。
 * @param[in] byZOrder 是否按z-order排序，如果按z-order排序，按z序从高到低排序
 * @return 所有QwtPlot对象的列表(不包含寄生轴)
 *
 * @note 此方法获取的绘图不包含寄生轴
 * \endif
 */
QList< QwtPlot* > QwtFigure::allAxes(bool byZOrder) const
{
    QList< QwtPlot* > plots;
    QLayout* lay = layout();
    if (lay) {
        for (int i = 0; i < lay->count(); ++i) {
            QLayoutItem* item = lay->itemAt(i);
            if (item && item->widget()) {
                if (QwtPlot* plot = qobject_cast< QwtPlot* >(item->widget())) {
                    if (plot->isHostPlot()) {
                        plots.append(plot);
                    }
                }
            }
        }
    }
    if (!byZOrder || plots.isEmpty()) {
        return plots;  // 原顺序直接返回
    }

    /* 按 z-order 从高到低重新排（children() 越靠后 z 越高） */
    const QObjectList& oc = children();
    QList< QwtPlot* > zOrdered;
    zOrdered.reserve(plots.size());

    // 倒序扫一次，命中就搬
    for (auto it = oc.crbegin(); it != oc.crend(); ++it) {
        if (QwtPlot* p = qobject_cast< QwtPlot* >(*it)) {
            if (plots.contains(p)) {  // 只有 O(n) 小列表查找
                zOrdered.append(p);
            }
        }
    }

    return zOrdered;  // 已经是从顶到底
}

/**
 * \if ENGLISH
 * @brief Check if the figure has any axes
 * @details This method returns true if the figure contains at least one QwtPlot.
 * @return true if the figure has axes, false otherwise
 * \endif
 *
 * \if CHINESE
 * @brief 检查图形是否有坐标轴
 * @details 如果图形包含至少一个QwtPlot，则此方法返回true。
 * @return 如果图形有坐标轴返回true，否则返回false
 * \endif
 */
bool QwtFigure::hasAxes() const
{
    QLayout* lay = layout();
    if (!lay) {
        return false;
    }

    for (int i = 0; i < lay->count(); ++i) {
        QLayoutItem* item = lay->itemAt(i);
        if (item && item->widget() && qobject_cast< QwtPlot* >(item->widget())) {
            return true;
        }
    }

    return false;
}

/**
 * \if ENGLISH
 * @brief Check if the figure contains a specific plot
 * @details This method returns true if the figure contains the specified QwtPlot.
 * @param[in] plot QwtPlot to check
 * @return true if the figure contains the plot, false otherwise
 * \endif
 *
 * \if CHINESE
 * @brief 检查图形是否存在特定plot
 * @details 如果图形包含指定的QwtPlot，则此方法返回true。
 * @param[in] plot 要检测的QwtPlot
 * @return 如果图形存在该绘图返回true，否则返回false
 * \endif
 */
bool QwtFigure::hasAxes(QwtPlot* plot) const
{
    QLayout* lay = layout();
    if (!lay || !plot) {
        return false;
    }

    for (int i = 0; i < lay->count(); ++i) {
        QLayoutItem* item = lay->itemAt(i);
        if (item) {
            if (QwtPlot* ax = qobject_cast< QwtPlot* >(item->widget())) {
                if (ax == plot) {
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * \if ENGLISH
 * @brief Remove a specific axes (plot) from the figure
 * @details This method removes the specified QwtPlot from the figure.
 * @param[in] plot QwtPlot to remove
 *
 * @note This function does not destroy the QwtPlot object. You need to call deleteLater() manually.
 * \endif
 *
 * \if CHINESE
 * @brief 从图形中移除特定的坐标轴（绘图）
 * @details 此方法从图形中移除指定的QwtPlot。
 * @param[in] plot 要移除的QwtPlot
 *
 * @note 此函数不会销毁QwtPlot对象，你需要手动调用deleteLater()。
 * \endif
 */
void QwtFigure::removeAxes(QwtPlot* plot)
{
    takeAxes(plot);
}

/**
 * \if ENGLISH
 * @brief Take a specific axes (plot) from the figure without deleting it
 * @param[in] plot Pointer to the QwtPlot to take
 * @return true if successfully taken, false otherwise
 *
 * @note If the removed plot is the current active axes, currentAxesChanged signal is emitted first, then axesRemoved signal.
 * @note If the figure has no plots after removal, currentAxesChanged signal is emitted with nullptr.
 * @note If a plot has parasite axes, they will be hidden and have parent widget set to nullptr.
 * \endif
 *
 * \if CHINESE
 * @brief 从图形中取出特定的坐标轴（绘图）但不删除它
 * @param[in] plot 要取出的QwtPlot指针
 * @return 成功取出返回true，否则返回false
 *
 * @note 如果当前的绘图是选择的激活坐标系，在移除时，会先发射currentAxesChanged信号，再发射axesRemoved信号。
 * @note 如果只有一个绘图，在移除后，整个figure没有绘图的情况下，也会发射currentAxesChanged信号，信号携带的内容为nullptr。
 * @note 如果一个绘图有寄生轴，在takeAxes后，它的寄生轴会设置为隐藏，并把parent widget设置为nullptr。
 * \endif
 */
bool QwtFigure::takeAxes(QwtPlot* plot)
{
    if (!plot) {
        return false;
    }

    // Remove from layout
    bool isRemove = false;
    // Check if the plot to remove is the current axes
    // 检查要移除的绘图是否是当前坐标轴
    bool removingCurrent = (plot == currentAxes());
    m_data->plotWillRemove(plot);
    QLayout* lay         = layout();
    if (lay) {
        for (int i = 0; i < lay->count(); ++i) {
            QLayoutItem* item = lay->itemAt(i);
            if (!item) {
                continue;
            }
            QWidget* w = item->widget();
            if (!w) {
                continue;
            }
            if (w == plot) {
                lay->removeItem(item);
                delete item;
                isRemove = true;
                break;
            }
        }
        if (removingCurrent) {
            // 说明移除了当前axes，需要更新currentAxes
            const int count = lay->count();
            if (count == 0) {
                // 如果figure已经清空，也发射currentAxesChanged，携带nullptr
                setCurrentAxes(nullptr);
            } else {
                for (int i = 0; i < count; ++i) {
                    QLayoutItem* item = lay->itemAt(i);
                    if (!item) {
                        continue;
                    }
                    if (QwtPlot* w = qobject_cast< QwtPlot* >(item->widget())) {
                        setCurrentAxes(w);
                    }
                }
            }
        }
    }
    if (isRemove) {
        // 处理寄生轴
        const QList< QwtPlot* > parasites = plot->parasitePlots();
        for (QwtPlot* para : parasites) {
            para->setParent(nullptr);
            para->hide();
        }
        Q_EMIT axesRemoved(plot);
    }
    return isRemove;
}

/**
 * \if ENGLISH
 * @brief Clear all axes from the figure
 * @details This method removes all QwtPlot objects from the figure and deletes them.
 *
 * @note This method emits axesRemoved signal during removal process.
 * @note After all removals, currentAxesChanged signal is emitted with nullptr, then figureCleared signal.
 * @note This method deletes all held plot widgets.
 * \endif
 *
 * \if CHINESE
 * @brief 清除图形中的所有坐标轴
 * @details 此方法从图形中移除所有QwtPlot对象并删除它们。
 *
 * @note 此方法在移除过程中会发射axesRemoved信号。
 * @note 此方法还会发射currentAxesChanged信号（携带nullptr），最后发射figureCleared信号。
 * @note 此方法会删除已经持有的所有plot窗口。
 * \endif
 */
void QwtFigure::clear()
{
    // Remove from layout
    QLayout* lay  = layout();
    int removeCnt = 0;
    if (lay) {
        // lay->count()不能放到for循环里面，每次循环会变化
        const int itemCnt = lay->count();
        // 先删除窗口，最后再统一删除item，这个循环里面不能调用removeItem，否则每次都改变队列大小，就不能正常遍历
        for (int i = 0; i < itemCnt; ++i) {
            QLayoutItem* item = lay->itemAt(i);
            if (item) {
                if (QwtPlot* plot = qobject_cast< QwtPlot* >(item->widget())) {
                    Q_EMIT axesRemoved(plot);
                }
                if (QWidget* w = item->widget()) {
                    w->hide();
                    w->deleteLater();
                }
                ++removeCnt;
            }
        }
        // 最后再统一删除item
        for (int i = 0; i < itemCnt; ++i) {
            QLayoutItem* item = lay->itemAt(i);
            lay->removeItem(item);
            delete item;
        }
    }
    setCurrentAxes(nullptr);
    if (removeCnt > 0) {
        Q_EMIT figureCleared();
    }
}

/**
 * \if ENGLISH
 * @brief Get the size of the figure in inches
 * @details This method calculates the physical size of the figure in inches based on
 *          the current pixel size and screen DPI.
 * @return Size of the figure in inches
 * \endif
 *
 * \if CHINESE
 * @brief 获取图形的英寸尺寸
 * @details 此方法基于当前像素尺寸和屏幕DPI计算图形的物理尺寸（英寸）。
 * @return 图形的英寸尺寸
 * \endif
 */
QSize QwtFigure::getSizeInches() const
{
    QScreen* screen = QGuiApplication::primaryScreen();
    int dpi         = screen ? screen->logicalDotsPerInch() : 96;

    QSize size = this->size();
    return QSize(size.width() / dpi, size.height() / dpi);
}

/**
 * \if ENGLISH
 * @brief Set the size of the figure in inches
 * @details This method sets the size of the figure in inches, converting to pixels
 *          based on the screen DPI.
 * @param[in] width Width in inches
 * @param[in] height Height in inches
 * \endif
 *
 * \if CHINESE
 * @brief 设置图形的英寸尺寸
 * @details 此方法设置图形的英寸尺寸，基于屏幕DPI转换为像素。
 * @param[in] width 宽度（英寸）
 * @param[in] height 高度（英寸）
 * \endif
 */
void QwtFigure::setSizeInches(float width, float height)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    int dpi         = screen ? screen->logicalDotsPerInch() : 96;

    int pixelWidth  = width * dpi;
    int pixelHeight = height * dpi;

    resize(pixelWidth, pixelHeight);
}

/**
 * \if ENGLISH
 * @brief Set the size of the figure in inches
 * @details This method sets the size of the figure in inches, converting to pixels
 *          based on the screen DPI.
 * @param[in] size Size in inches
 * \endif
 *
 * \if CHINESE
 * @brief 设置图形的英寸尺寸
 * @details 此方法设置图形的英寸尺寸，基于屏幕DPI转换为像素。
 * @param[in] size 英寸尺寸
 * \endif
 */
void QwtFigure::setSizeInches(const QSizeF& size)
{
    setSizeInches(size.width(), size.height());
}

/**
 * \if ENGLISH
 * @brief Set the face color of the figure
 * @details This method sets the background color of the figure.
 * @param[in] color Background color
 * \endif
 *
 * \if CHINESE
 * @brief 设置图形的背景颜色
 * @details 此方法设置图形的背景颜色。
 * @param[in] color 背景颜色
 * \endif
 */
void QwtFigure::setFaceColor(const QColor& color)
{
    m_data->faceBrush = color;
}

/**
 * \if ENGLISH
 * @brief Get the face color of the figure
 * @details This method returns the background color of the figure.
 * @return Background color
 * \endif
 *
 * \if CHINESE
 * @brief 获取图形的表面颜色
 * @details 此方法返回图形的背景颜色。
 * @return 背景颜色
 * \endif
 */
QColor QwtFigure::faceColor() const
{
    return m_data->faceBrush.color();
}

/**
 * \if ENGLISH
 * @brief Set the face brush of the figure
 * @details This method sets the background brush of the figure, allowing for
 *          more complex backgrounds (gradients, textures, etc.).
 * @param[in] brush Background brush
 * \endif
 *
 * \if CHINESE
 * @brief 设置图形的背景画刷
 * @details 此方法设置图形的背景画刷，允许更复杂的背景（渐变、纹理等）。
 * @param[in] brush 背景画刷
 * \endif
 */
void QwtFigure::setFaceBrush(const QBrush& brush)
{
    m_data->faceBrush = brush;
}

/**
 * \if ENGLISH
 * @brief Get the face brush of the figure
 * @details This method returns the background brush of the figure.
 * @return Background brush
 * \endif
 *
 * \if CHINESE
 * @brief 获取图形的表面画刷
 * @details 此方法返回图形的背景画刷。
 * @return 背景画刷
 * \endif
 */
QBrush QwtFigure::faceBrush() const
{
    return m_data->faceBrush;
}

/**
 * \if ENGLISH
 * @brief Set the edge color of the figure
 * @details This method sets the border color of the figure.
 * @param[in] color Border color
 * \endif
 *
 * \if CHINESE
 * @brief 设置图形的边缘颜色
 * @details 此方法设置图形的边框颜色。
 * @param[in] color 边框颜色
 * \endif
 */
void QwtFigure::setEdgeColor(const QColor& color)
{
    m_data->edgeColor = color;
}

/**
 * \if ENGLISH
 * @brief Get the edge color of the figure
 * @details This method returns the border color of the figure.
 * @return Border color
 * \endif
 *
 * \if CHINESE
 * @brief 获取图形的边缘颜色
 * @details 此方法返回图形的边框颜色。
 * @return 边框颜色
 * \endif
 */
QColor QwtFigure::edgeColor() const
{
    return m_data->edgeColor;
}

/**
 * \if ENGLISH
 * @brief Set the edge line width of the figure
 * @details This method sets the border line width of the figure.
 * @param[in] width Border line width in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 设置图形的边缘线宽
 * @details 此方法设置图形的边框线宽。
 * @param[in] width 边框线宽（像素）
 * \endif
 */
void QwtFigure::setEdgeLineWidth(int width)
{
    m_data->edgeLineWidth = width;
}

/**
 * \if ENGLISH
 * @brief Get the edge line width of the figure
 * @details This method returns the border line width of the figure.
 * @return Border line width in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 获取图形的边缘线宽
 * @details 此方法返回图形的边框线宽。
 * @return 边框线宽（像素）
 * \endif
 */
int QwtFigure::edgeLineWidth() const
{
    return m_data->edgeLineWidth;
}

/**
 * \if ENGLISH
 * @brief Create parasite axes for a host plot
 * @details This method creates a parasite axes that shares the same plotting area as the host plot
 *          but with independent axis scaling and labeling. The parasite axes will be positioned
 *          exactly on top of the host plot and will automatically synchronize its geometry.
 * @param[in] hostPlot Pointer to the host QwtPlot
 * @param[in] enableAxis The axis position to enable on the parasite axes
 * @return Pointer to the created parasite QwtPlot
 * @retval nullptr if hostPlot is invalid or not in the figure
 * @note The parasite axes will have a transparent background and only the specified axis will be visible.
 * @note The parasite axes will automatically be deleted when the host plot is removed from the figure.
 * @note Parasitic axes are not stored in QwtFigureLayout, but are separately controlled by QwtFigure for layout management.
 * @note Parasite axes must be managed by the figure because they only overlap the plotting area with the host, while the coordinate window positions are different from the host.
 * \endif
 *
 * \if CHINESE
 * @brief 为宿主绘图创建寄生轴
 * @details 此方法创建一个寄生轴，它与宿主绘图共享相同的绘图区域，但具有独立的轴缩放和标签。
 *          寄生轴将精确定位在宿主绘图之上，并自动同步其几何形状。
 * @param[in] hostPlot 指向宿主QwtPlot的指针
 * @param[in] enableAxis 在寄生轴上启用的轴位置
 * @return 指向创建的寄生QwtPlot的指针
 * @retval nullptr 如果hostPlot无效或不在图形中则返回nullptr
 * @note 寄生轴将具有透明背景，只有指定的轴可见。
 * @note 当宿主绘图从图形中移除时，寄生轴将自动被删除。
 * @note 寄生轴不会存入QwtFigureLayout中，单独由QwtFigure进行布局控制。
 * @note 寄生轴必须有figure来管理，这是因为寄生轴仅仅是绘图区域和宿主重叠，坐标窗口的位置都和宿主不一样。
 * \endif
 */
QwtPlot* QwtFigure::createParasiteAxes(QwtPlot* hostPlot, QwtAxis::Position enableAxis)
{
    if (!hostPlot || !hasAxes(hostPlot)) {
        qWarning() << "Invalid host plot or host plot not in figure";
        return nullptr;
    }
    if (hostPlot->isParasitePlot()) {
        // 不是宿主，切换为宿主
        hostPlot = hostPlot->hostPlot();
    }
    // 创建寄生轴
    QwtPlot* parasitePlot = hostPlot->createParasitePlot(enableAxis);

    return parasitePlot;
}

/**
 * \if ENGLISH
 * @brief Get all parasite axes for a host plot
 * @details This method returns a list of all parasite axes associated with the specified host plot.
 * @param[in] hostPlot Pointer to the host QwtPlot
 * @return List of parasite QwtPlot pointers
 * @retval Empty list if hostPlot is invalid or has no parasite axes
 * \endif
 *
 * \if CHINESE
 * @brief 获取宿主绘图的所有寄生轴
 * @details 此方法返回与指定宿主绘图关联的所有寄生轴的列表。
 * @param[in] hostPlot 指向宿主QwtPlot的指针
 * @return 寄生QwtPlot指针列表
 * @retval 空列表 如果hostPlot无效或没有寄生轴则返回空列表
 * \endif
 */
QList< QwtPlot* > QwtFigure::getParasiteAxes(QwtPlot* hostPlot) const
{
    if (!hostPlot) {
        return QList< QwtPlot* >();
    }
    return hostPlot->parasitePlots();
}

/**
 * \if ENGLISH
 * @brief Save the figure to a QPixmap with specified DPI
 * @details This method renders the figure to a QPixmap with the specified DPI.
 *          If DPI is -1, the current screen DPI is used.
 * @param[in] dpi Dots per inch for the saved image (-1 to use screen DPI)
 * @return QPixmap containing the rendered figure
 * \endif
 *
 * \if CHINESE
 * @brief 使用指定DPI将图形保存为QPixmap
 * @details 此方法将图形渲染为具有指定DPI的QPixmap。
 *          如果DPI为-1，则使用当前屏幕DPI。
 * @param[in] dpi 保存图像的DPI（-1表示使用屏幕DPI）
 * @return 包含渲染图形的QPixmap
 * \endif
 */
QPixmap QwtFigure::saveFig(int dpi) const
{
    // Calculate the target size based on DPI
    QSize targetSize;
    int targetDpi;

    if (dpi <= 0) {
        QScreen* screen = QGuiApplication::primaryScreen();
        targetDpi       = screen ? screen->logicalDotsPerInch() : 96;
        targetSize      = size();
    } else {
        targetDpi                 = dpi;
        QSizeF physicalSizeInches = getSizeInches();
        targetSize                = QSize(static_cast< int >(physicalSizeInches.width() * dpi),
                           static_cast< int >(physicalSizeInches.height() * dpi));
    }

    // Use const_cast to call non-const methods
    QwtFigure* nonConstThis = const_cast< QwtFigure* >(this);

    if (dpi <= 0) {
        // No scaling needed, just grab the current state
        return nonConstThis->grab();
    }
    // Create pixmap with target size
    QPixmap pixmap(targetSize);

    // Use QPainter for high-quality scaling
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Calculate scaling factors
    qreal scaleX = static_cast< qreal >(targetSize.width()) / width();
    qreal scaleY = static_cast< qreal >(targetSize.height()) / height();
    painter.scale(scaleX, scaleY);

    // Render the figure with scaling
    nonConstThis->render(&painter);
    painter.end();

    // Set DPI information if needed
    // 当你在图像文件中设置DPI信息时，图像处理软件（如Photoshop、GIMP等）和打印机会知道如何正确解释图像的物理尺寸。
    // 如果没有DPI信息，软件通常会使用默认的DPI（通常是72或96），这会导致物理尺寸计算错误。
    // 不同的设备和软件可能有不同的默认DPI设置。明确设置DPI可以确保图像在所有平台上显示一致的物理尺寸。
    QImage image = pixmap.toImage();
    // Convert DPI to dots per meter (1 inch = 2.54 cm, so 1 meter = 100/2.54 inches)
    // 将DPI转换为每米的点数（1英寸=2.54厘米，所以1米=100/2.54英寸）
    image.setDotsPerMeterX(targetDpi * 100 / 2.54);
    image.setDotsPerMeterY(targetDpi * 100 / 2.54);
    return QPixmap::fromImage(image);
}

/**
 * \if ENGLISH
 * @brief Save the figure to a QPixmap with specified size in inches
 * @details This method renders the figure to a QPixmap with the specified physical size in inches.
 *          The current DPI setting of the figure is used to calculate the pixel size.
 * @param[in] inchesSize Physical size in inches
 * @return QPixmap containing the rendered figure
 * \endif
 *
 * \if CHINESE
 * @brief 使用指定英寸尺寸将图形保存为QPixmap
 * @details 此方法将图形渲染为具有指定物理尺寸（英寸）的QPixmap。
 *          使用图形当前的DPI设置来计算像素尺寸。
 * @param[in] inchesSize 物理尺寸（英寸）
 * @return 包含渲染图形的QPixmap
 * \endif
 */
QPixmap QwtFigure::saveFig(QSizeF& inchesSize) const
{
    // Use current DPI to calculate target pixel size
    // 使用当前DPI计算目标像素尺寸
    QScreen* screen = QGuiApplication::primaryScreen();
    int currentDpi  = screen ? screen->logicalDotsPerInch() : 96;
    QSize targetSize(static_cast< int >(inchesSize.width() * currentDpi),
                     static_cast< int >(inchesSize.height() * currentDpi));

    // Use const_cast to call non-const methods
    // 使用const_cast调用非const方法
    QwtFigure* nonConstThis = const_cast< QwtFigure* >(this);

    // Create pixmap with target size
    // 创建目标尺寸的pixmap
    QPixmap pixmap(targetSize);

    // Use QPainter for high-quality scaling
    // 使用QPainter进行高质量缩放
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Calculate scaling factors
    // 计算缩放因子
    qreal scaleX = static_cast< qreal >(targetSize.width()) / width();
    qreal scaleY = static_cast< qreal >(targetSize.height()) / height();
    painter.scale(scaleX, scaleY);

    // Render the figure with scaling
    // 渲染图形并应用缩放
    nonConstThis->render(&painter);
    painter.end();

    // Set DPI information
    // 设置DPI信息
    QImage image = pixmap.toImage();
    image.setDotsPerMeterX(currentDpi * 100 / 2.54);
    image.setDotsPerMeterY(currentDpi * 100 / 2.54);
    return QPixmap::fromImage(image);
}

/**
 * \if ENGLISH
 * @brief Save the figure to a file with specified DPI
 * @details This method saves the figure to an image file with the specified DPI.
 * @param[in] filename Name of the file to save
 * @param[in] dpi Dots per inch for the saved image (-1 to use screen DPI)
 * @return true if saved successfully, false otherwise
 * \endif
 *
 * \if CHINESE
 * @brief 使用指定DPI将图形保存到文件
 * @details 此方法将图形保存为具有指定DPI的图像文件。
 * @param[in] filename 要保存的文件名
 * @param[in] dpi 保存图像的DPI（-1表示使用屏幕DPI）
 * @return 成功保存返回true，否则返回false
 * \endif
 */
bool QwtFigure::saveFig(const QString& filename, int dpi) const
{
    QPixmap pixmap = saveFig(dpi);
    return pixmap.save(filename, nullptr, -1);
}

/**
 * \if ENGLISH
 * @brief Set the current axes (plot)
 * @details This method sets the specified QwtPlot as the current active axes in the figure.
 * @param[in] plot QwtPlot to set as current
 * \endif
 *
 * \if CHINESE
 * @brief 设置当前坐标轴（绘图）
 * @details 此方法将指定的QwtPlot设置为图形中当前活动的坐标轴。
 * @param[in] plot 要设置为当前的QwtPlot
 * \endif
 */
void QwtFigure::setCurrentAxes(QwtPlot* plot)
{
    // 允许设置为 nullptr，或仅当 plot 属于本 figure 管理时才设置
    if (plot == nullptr || hasAxes(plot)) {
        m_data->currentAxes = plot;
        Q_EMIT currentAxesChanged(plot);
    }
}

/**
 * \if ENGLISH
 * @brief Set the current axes (plot)
 * @details This is a convenience method that calls setCurrentAxes.
 * @param[in] plot QwtPlot to set as current
 * \endif
 *
 * \if CHINESE
 * @brief 设置当前坐标轴（绘图）
 * @details 这是一个便捷方法，调用setCurrentAxes。
 * @param[in] plot 要设置为当前的QwtPlot
 * \endif
 */
void QwtFigure::sca(QwtPlot* plot)
{
    setCurrentAxes(plot);
}

/**
 * \if ENGLISH
 * @brief Get the current axes (plot)
 * @details This method returns the current active QwtPlot in the figure.
 *          The current axes is typically the last axes that was added, modified, or plotted on.
 * @return Pointer to the current QwtPlot, or nullptr if no axes exist
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前坐标轴（绘图）
 * @details 此方法返回图形中当前活动的QwtPlot。
 *          当前坐标轴通常是最后添加、修改或绘图的坐标轴。
 * @return 指向当前QwtPlot的指针，如果没有坐标轴则返回nullptr
 * \endif
 */
QwtPlot* QwtFigure::currentAxes() const
{
    return m_data->currentAxes;
}

/**
 * \if ENGLISH
 * @brief Get the current axes (plot)
 * @details This is a convenience method that calls currentAxes.
 * @return Pointer to the current QwtPlot, or nullptr if no axes exist
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前坐标轴（绘图）
 * @details 这是一个便捷方法，调用currentAxes。
 * @return 指向当前QwtPlot的指针，如果没有坐标轴则返回nullptr
 * \endif
 */
QwtPlot* QwtFigure::gca() const
{
    return currentAxes();
}

/**
 * \if ENGLISH
 * @brief Get the normalized rectangle for a axes
 * @details This method returns the normalized coordinates [0,1] for the specified axes
 *          in the figure. If the axes is not found in the figure, an invalid QRectF is returned.
 * @param[in] widget Widget to query
 * @return Normalized coordinates [left, top, width, height] in range [0,1], or invalid QRectF if not found
 * \endif
 *
 * \if CHINESE
 * @brief 获取坐标系的归一化矩形
 * @details 此方法返回布局中指定坐标系的归一化坐标[0,1]。如果在绘图中未找到该坐标系，则返回无效的QRectF。
 * @param[in] widget 要查询的坐标系
 * @return 归一化坐标 [左, 上, 宽, 高]，范围 [0,1]，如果未找到则返回无效QRectF
 * \endif
 */
QRectF QwtFigure::axesNormRect(QwtPlot* plot) const
{
    QWTFIGURE_SAFEGET_LAY_RET(lay, QRect())
    return lay->widgetNormRect(plot);
}

/**
 * \if ENGLISH
 * @brief Get the normalized rectangle for a widget
 * @details This method returns the normalized coordinates [0,1] for the specified axes
 *          in the figure. If the widget is not found in the figure, an invalid QRectF is returned.
 * @param[in] widget Widget to query
 * @return Normalized coordinates [left, top, width, height] in range [0,1], or invalid QRectF if not found
 * \endif
 *
 * \if CHINESE
 * @brief 获取窗口的归一化矩形
 * @details 此方法返回布局中指定坐标系的归一化坐标[0,1]。如果在绘图中未找到该窗口，则返回无效的QRectF。
 * @param[in] widget 要查询的窗口
 * @return 归一化坐标 [左, 上, 宽, 高]，范围 [0,1]，如果未找到则返回无效QRectF
 * \endif
 */
QRectF QwtFigure::widgetNormRect(QWidget* w) const
{
    QWTFIGURE_SAFEGET_LAY_RET(lay, QRect())
    return lay->widgetNormRect(w);
}

/**
 * \if ENGLISH
 * @brief Get the plot under a position
 * @details This method returns the QwtPlot under the specified position.
 *          If no plot is found, nullptr is returned. Hidden windows are not considered.
 * @param[in] pos Position to query
 * @return Pointer to the QwtPlot under the position, or nullptr if not found
 * \endif
 *
 * \if CHINESE
 * @brief 获取在此坐标下的绘图
 * @details 此方法返回指定坐标下的QwtPlot。如果此坐标下没有绘图，则返回nullptr。隐藏的窗口不会被获取到。
 * @param[in] pos 坐标
 * @return 如果此坐标下没有绘图，则返回nullptr
 * \endif
 */
QwtPlot* QwtFigure::plotUnderPos(const QPoint& pos) const
{
    const QList< QwtPlot* > result = findChildren< QwtPlot* >(QString(), Qt::FindDirectChildrenOnly);
    if (result.empty()) {
        return nullptr;
    }
    for (QwtPlot* plot : result) {
        if (!(plot->isVisibleTo(this))) {
            continue;
        }
        // 判断子窗口的区域是否包含转换后的点
        if (plot->geometry().contains(pos)) {
            return plot;
        }
    }
    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Calculate normalized coordinates from actual window coordinates
 * @details This method converts actual window coordinates (geometry) to normalized coordinates.
 * @param[in] geoRect Actual window coordinates (geometry)
 * @return Normalized coordinates QRectF
 * \endif
 *
 * \if CHINESE
 * @brief 通过真实的窗口坐标计算归一化坐标
 * @details 此方法将真实的窗口坐标（geometry）转换为归一化坐标。
 * @param[in] geoRect 真实窗口坐标，就是子窗口的geometry()
 * @return 归一化坐标QRectF
 * \endif
 */
QRectF QwtFigure::calcNormRect(const QRect& geoRect) const
{
    return QwtFigureLayout::calcNormRect(rect(), geoRect);
}

/**
 * \if ENGLISH
 * @brief Calculate actual rectangle from normalized coordinates
 * @details This method converts normalized coordinates to actual window coordinates.
 * @param[in] normRect Normalized coordinates QRectF
 * @return Actual window coordinates QRect
 * \endif
 *
 * \if CHINESE
 * @brief 通过归一化矩形计算真实矩形
 * @details 此方法将归一化坐标转换为真实的窗口坐标。
 * @param[in] normRect 归一化坐标QRectF
 * @return 真实窗口坐标QRect
 * \endif
 */
QRect QwtFigure::calcActualRect(const QRectF& normRect)
{
    QWTFIGURE_SAFEGET_LAY_RET(lay, QRect())
    return lay->calcActualRect(rect(), normRect);
}

/**
 * \if ENGLISH
 * @brief Update all plots in the figure
 * @details This method calls replot on all plots in the figure.
 * \endif
 *
 * \if CHINESE
 * @brief 更新所有的绘图
 * @details 此方法对所有图形中的绘图调用replot。
 * \endif
 */
void QwtFigure::replotAll()
{
    const QList< QwtPlot* > plots = allAxes();
    for (QwtPlot* plot : plots) {
        plot->replotAll();
    }
}

/**
 * \if ENGLISH
 * @brief Add axis alignment configuration
 * @details This method adds an alignment configuration for the specified plots and axis.
 * @param[in] plots List of plots to align
 * @param[in] axisId Axis ID to align (QwtAxis::XTop/XBottom/YLeft/YRight)
 * \endif
 *
 * \if CHINESE
 * @brief 添加轴对齐配置
 * @details 此方法添加指定绘图和轴的对齐配置。
 * @param[in] plots 需要对齐的plot列表
 * @param[in] axisId 要对齐的轴ID（QwtAxis::XTop/XBottom/YLeft/YRight）
 * \endif
 */
void QwtFigure::addAxisAlignment(const QList< QwtPlot* >& plots, int axisId)
{
    if (plots.isEmpty() || !QwtAxis::isValid(axisId)) {
        return;
    }

    // 过滤掉不在当前figure中的plot
    QList< QwtPlot* > validPlots;
    for (QwtPlot* plot : plots) {
        if (plot && hasAxes(plot)) {
            validPlots.append(plot);
        }
    }

    if (validPlots.isEmpty()) {
        return;
    }

    // 添加到配置列表
    PrivateData::AlignmentConfig config;
    config.plots  = validPlots;
    config.axisId = axisId;
    m_data->alignmentConfigs.append(config);
}

/**
 * \if ENGLISH
 * @brief Remove specified axis alignment configuration
 * @details This method removes the alignment configuration for the specified plots and axis.
 * @param[in] plots List of plots to remove from alignment
 * @param[in] axisId Axis ID to remove from alignment
 * @return true if successfully removed, false otherwise
 * \endif
 *
 * \if CHINESE
 * @brief 移除指定的轴对齐配置
 * @details 此方法移除指定绘图和轴的对齐配置。
 * @param[in] plots 需要移除的对齐配置中的plot列表
 * @param[in] axisId 要移除的对齐配置中的轴ID
 * @return 是否成功移除
 * \endif
 */
bool QwtFigure::removeAxisAlignment(const QList< QwtPlot* >& plots, int axisId)
{
    if (plots.isEmpty() || !QwtAxis::isValid(axisId)) {
        return false;
    }

    bool removed = false;
    auto it      = m_data->alignmentConfigs.begin();
    while (it != m_data->alignmentConfigs.end()) {
        if (it->axisId == axisId && it->plots == plots) {
            it      = m_data->alignmentConfigs.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    return removed;
}
/**
 * \if ENGLISH
 * @brief Clear all axis alignment configurations
 * @details This method removes all alignment configurations from the figure.
 * \endif
 *
 * \if CHINESE
 * @brief 清除所有轴对齐配置
 * @details 此方法移除图形中的所有对齐配置。
 * \endif
 */
void QwtFigure::clearAxisAlignment()
{
    m_data->alignmentConfigs.clear();
}

/**
 * \if ENGLISH
 * @brief Apply all axis alignment configurations
 * @details This method applies all recorded alignment configurations to the plots.
 * @param[in] replot If true, replot all affected plots after alignment
 * \endif
 *
 * \if CHINESE
 * @brief 应用所有轴对齐配置
 * @details 此方法对所有记录的对齐配置进行对齐。
 * @param[in] replot 如果为true，对齐后重新绘制所有受影响的绘图
 * \endif
 */
void QwtFigure::applyAllAxisAlignments(bool replot)
{
    for (const auto& config : qwt_as_const(m_data->alignmentConfigs)) {
        alignAxes(config.plots, config.axisId, replot);
    }
}

/**
 * \if ENGLISH
 * @brief Apply alignment configurations for a specific axis
 * @details This method applies all alignment configurations for the specified axis ID.
 * @param[in] axisId Axis ID to apply alignments for
 * \endif
 *
 * \if CHINESE
 * @brief 应用指定轴ID的所有对齐配置
 * @details 此方法应用指定轴ID的所有对齐配置。
 * @param[in] axisId 轴ID
 * \endif
 */
void QwtFigure::applyAlignmentsForAxis(int axisId)
{
    if (!QwtAxis::isValid(axisId)) {
        return;
    }

    for (const auto& config : qwt_as_const(m_data->alignmentConfigs)) {
        if (config.axisId == axisId) {
            alignAxes(config.plots, config.axisId);
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get the number of axis alignment configurations
 * @details This method returns the count of alignment configurations added via addAxisAlignment.
 * @return Number of alignment configurations
 * \endif
 *
 * \if CHINESE
 * @brief 获取轴对齐信息数量
 * @details 此函数用于获取当前有多少个轴对齐信息。调用addAxisAlignment多少次，就有多少个轴对齐信息。
 * @return 轴对齐配置数量
 * \endif
 */
int QwtFigure::axisAligmentCount() const
{
    return m_data->alignmentConfigs.size();
}

/**
 * \if ENGLISH
 * @brief Get axis alignment information by index
 * @details This method returns the alignment configuration at the specified index.
 * @param[in] index Index of the alignment configuration to retrieve
 * @return QPair containing the plot list and axis ID
 * \endif
 *
 * \if CHINESE
 * @brief 获取轴对齐信息
 * @details 此方法返回指定索引的对齐配置信息。
 * @param[in] index 对齐配置的索引
 * @return QPair包含plot列表和轴ID
 * \endif
 */
QPair<QList<QwtPlot*>, int> QwtFigure::axisAligmentInfo(int index) const
{
    if(index >= axisAligmentCount() || index < 0 ){
        return {};
    }
    auto ali = m_data->alignmentConfigs.value(index);
    return qMakePair(ali.plots,ali.axisId);
}

/**
 * \if ENGLISH
 * @brief Align axes of multiple plots
 * @details This function unifies the minimumExtent and minBorderDist of the specified axis
 *          to ensure visual alignment of axes.
 * @param[in] plots List of QwtPlot to align (must be non-empty)
 * @param[in] axisId Axis ID to align (QwtAxis::XTop/XBottom/YLeft/YRight)
 * @param[in] replot If true, update layout and replot after alignment
 * @note This function should be called after widget initialization (e.g., in showEvent/resizeEvent).
 * @note Supports any number of plots and any valid axis type, adapting to horizontal/vertical layouts.
 * @note Do not pass parasite axes, currently only supports host axes.
 * \endif
 *
 * \if CHINESE
 * @brief QwtPlot轴对齐函数
 * @details 此函数统一指定轴的minimumExtent和minBorderDist，确保轴视觉对齐。
 * @param[in] plots 待对齐的QwtPlot列表（非空）
 * @param[in] axisId 要对齐的轴ID（QwtAxis::XTop/XBottom/YLeft/YRight）
 * @param[in] replot 如果为true，对齐后更新布局并重新绘制
 * @note 函数会统一指定轴的minimumExtent和minBorderDist，确保轴视觉对齐。
 * @note 需在控件初始化完成后调用（如showEvent/resizeEvent中）。
 * @note 支持任意数量Plot、任意合法轴类型，适配水平/垂直布局。
 * @note 不要传入寄生轴，目前仅支持宿主轴。
 * \endif
 */
void QwtFigure::alignAxes(QList< QwtPlot* > plots, int axisId, bool update)
{
    // ========== 步骤1：参数有效性校验 ==========
    if (plots.isEmpty()) {
        return;
    }

    if (!QwtAxis::isValid(axisId)) {
        return;
    }

    // 过滤掉空指针Plot
    plots.erase(std::remove_if(plots.begin(), plots.end(), [](QwtPlot* p) { return p == nullptr; }), plots.end());
    if (plots.isEmpty()) {
        return;
    }

    // ========== 步骤2：统一轴的minimumExtent（保证轴宽度/高度一致）/同时统一轴的EdgeMargin（保证绘图区域和边界的偏移一致） ==========
    double maxExtent  = 0.0;
    int maxEdgeMargin = 0;
    int maxStartDist = 0, maxEndDist = 0;
    // 计算所有Plot对应轴的最大extent（真实延伸尺寸），edgeMargin，BorderDistHint
    for (QwtPlot* plot : qwt_as_const(plots)) {
        QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
        if (!scaleWidget) {
            continue;
        }

        QwtScaleDraw* scaleDraw = scaleWidget->scaleDraw();
        if (!scaleDraw)
            continue;

        // 重置最小延伸尺寸，确保计算真实的extent
        scaleDraw->setMinimumExtent(0.0);
        // 计算当前轴的延伸尺寸（含刻度标签、刻度线、轴标题）
        double extent = scaleDraw->extent(scaleWidget->font());
        if (extent > maxExtent) {
            maxExtent = extent;
        }
        // 查询edgeMargin，记录最大的edgeMargin
        int em = scaleWidget->edgeMargin();
        if (em > maxEdgeMargin) {
            maxEdgeMargin = em;
        }
        // 统一轴的minBorderDist（保证绘图区域偏移一致）
        int startDist = 0, endDist = 0;
        scaleWidget->getBorderDistHint(startDist, endDist);

        // 更新最大值
        maxStartDist = qMax(maxStartDist, startDist);
        maxEndDist   = qMax(maxEndDist, endDist);
    }

    // 给所有Plot更新数据
    for (QwtPlot* plot : qwt_as_const(plots)) {
        QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
        if (!scaleWidget) {
            continue;
        }
        scaleWidget->scaleDraw()->setMinimumExtent(maxExtent);
        scaleWidget->setEdgeMargin(maxEdgeMargin);
        scaleWidget->setMinBorderDist(maxStartDist, maxEndDist);
    }
    // ========== 步骤4：强制更新轴和重绘，确保设置生效 ==========
    if (update) {
        for (QwtPlot* plot : qwt_as_const(plots)) {
            plot->updateLayout();
            plot->replot();      // 重绘Plot
        }
    }
}



void QwtFigure::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), m_data->faceBrush);

    // Draw border
    if (m_data->edgeLineWidth > 0) {
        QPen pen(m_data->edgeColor);
        pen.setWidth(m_data->edgeLineWidth);
        painter.setPen(pen);
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
    }

    QFrame::paintEvent(event);
}

void QwtFigure::resizeEvent(QResizeEvent* event)
{
    applyAllAxisAlignments(false);  // 窗口大小改变时重新对齐
    QFrame::resizeEvent(event);
}
