#ifndef QWTPARASITEPLOTLAYOUT_H
#define QWTPARASITEPLOTLAYOUT_H
#include "qwt_global.h"
#include "qwt_plot_layout.h"

/**
 * \if ENGLISH
 * @brief Layout manager for parasite plots
 *
 * QwtParasitePlotLayout manages the layout of parasite plots that share
 * the same canvas area with their host plot. It copies layout attributes
 * from the host plot to ensure visual consistency.
 *
 * @sa QwtPlot::createParasitePlot()
 * \endif
 *
 * \if CHINESE
 * @brief 寄生绘图的布局管理器
 *
 * QwtParasitePlotLayout管理与宿主绘图共享相同画布区域的寄生绘图布局。
 * 它从宿主绘图复制布局属性以确保视觉一致性。
 *
 * @sa QwtPlot::createParasitePlot()
 * \endif
 */
class QWT_EXPORT QwtParasitePlotLayout : public QwtPlotLayout
{
public:
    QwtParasitePlotLayout();
    ~QwtParasitePlotLayout();
	virtual void activate(const QwtPlot* plot, const QRectF& plotRect, Options options = Options()) override;

	virtual QSize minimumSizeHint(const QwtPlot* plot) const override;
    /// Get the parasite axis scale rect without offset
    QRectF parasiteScaleRect(QwtAxisId aid) const;

private:
    QRectF mScaleRects[ QwtAxis::AxisPositions ];
};

#endif  // QWTPLOTPARASITELAYOUT_H
