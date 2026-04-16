#include "qwt_parasite_plot_layout.h"
// Qt
#include <QDebug>

// qwt
#include "qwt_plot.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout_engine.h"
#include "qwt_abstract_legend.h"
#include "qwt_math.h"
#include "qwt_plot_layout_engine.h"

#ifndef QWTPLOTPARASITELAYOUT_DEBUG_PRINT
#define QWTPLOTPARASITELAYOUT_DEBUG_PRINT 1
#endif

/**
 * \if ENGLISH
 * @brief Construct a parasite plot layout
 * \endif
 *
 * \if CHINESE
 * @brief 构造寄生绘图布局
 * \endif
 */
QwtParasitePlotLayout::QwtParasitePlotLayout() : QwtPlotLayout()
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
QwtParasitePlotLayout::~QwtParasitePlotLayout()
{
}

/**
 * \if ENGLISH
 * @brief Activate the layout for a parasite plot
 *
 * Copies all layout attributes from the host plot to maintain visual consistency.
 * If no host plot is found, uses default layout behavior.
 *
 * @param[in] plot Parasite plot to activate layout for
 * @param[in] plotRect Rectangle for the plot area
 * @param[in] options Layout options
 * \endif
 *
 * \if CHINESE
 * @brief 激活寄生绘图的布局
 *
 * 从宿主绘图复制所有布局属性以保持视觉一致性。
 * 如果找不到宿主绘图，使用默认布局行为。
 *
 * @param[in] plot 要激活布局的寄生绘图
 * @param[in] plotRect 绘图区域矩形
 * @param[in] options 布局选项
 * \endif
 */
void QwtParasitePlotLayout::activate(const QwtPlot* plot, const QRectF& plotRect, QwtPlotLayout::Options options)
{
    QwtPlot* hostPlot = plot->hostPlot();
    if (!hostPlot) {
        // qDebug() << "QwtPlotParasiteLayout: No host plot found! Using default layout.";
        QwtPlotLayout::activate(plot, plotRect, options);
        return;
    }
    invalidate();
    doActivate(plot, plotRect, options);
    mScaleRects[ QwtAxis::YLeft ]   = scaleRect(QwtAxis::YLeft);
    mScaleRects[ QwtAxis::YRight ]  = scaleRect(QwtAxis::YRight);
    mScaleRects[ QwtAxis::XBottom ] = scaleRect(QwtAxis::XBottom);
    mScaleRects[ QwtAxis::XTop ]    = scaleRect(QwtAxis::XTop);
    // 寄生轴所有部件复制宿主轴
    if (QwtPlotLayout* hostLayout = hostPlot->plotLayout()) {
        setCanvasRect(hostLayout->canvasRect());
        setSpacing(hostLayout->spacing());
        setTitleRect(hostLayout->titleRect());
        setFooterRect(hostLayout->footerRect());
        setLegendRect(hostLayout->legendRect());
        for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; ++axisPos) {
            setCanvasMargin(hostLayout->canvasMargin(axisPos), axisPos);
            setScaleRect(axisPos, hostLayout->scaleRect(axisPos));
        }
    }
}

/**
 * \if ENGLISH
 * @brief Get minimum size hint for a parasite plot
 *
 * Returns the minimum size from the host plot's layout if available,
 * otherwise uses default QwtPlotLayout behavior.
 *
 * @param[in] plot Parasite plot to get size hint for
 * @return Minimum size hint
 * \endif
 *
 * \if CHINESE
 * @brief 获取寄生绘图的最小尺寸提示
 *
 * 如果可用，返回宿主绘图布局的最小尺寸，
 * 否则使用默认的QwtPlotLayout行为。
 *
 * @param[in] plot 要获取尺寸提示的寄生绘图
 * @return 最小尺寸提示
 * \endif
 */
QSize QwtParasitePlotLayout::minimumSizeHint(const QwtPlot* plot) const
{
	// 对于寄生轴，最小尺寸主要由启用的轴决定
	QwtPlot* hostPlot = plot->hostPlot();
	if (hostPlot) {
		QwtPlotLayout* lay = hostPlot->plotLayout();
		if (lay) {
			return lay->minimumSizeHint(hostPlot);
		}
	}
    return QwtPlotLayout::minimumSizeHint(plot);
}

/**
 * \if ENGLISH
 * @brief Get the parasite axis scale rect without offset
 *
 * Returns the scale rectangle for a parasite axis before it was copied
 * from the host plot layout. This is the original unmodified position.
 *
 * @param[in] aid Axis ID to get scale rect for
 * @return Scale rectangle for the axis
 * \endif
 *
 * \if CHINESE
 * @brief 获取寄生轴未偏移时的坐标轴矩形
 *
 * 返回寄生轴在从宿主绘图布局复制之前的比例矩形。
 * 这是原始的未修改位置。
 *
 * @param[in] aid 要获取比例矩形的轴ID
 * @return 轴的比例矩形
 * \endif
 */
QRectF QwtParasitePlotLayout::parasiteScaleRect(QwtAxisId aid) const
{
    return mScaleRects[ aid ];
}
