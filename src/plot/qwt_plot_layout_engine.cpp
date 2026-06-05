#include "qwt_plot_layout_engine.h"
#include "qwt_abstract_legend.h"
#include "qwt_math.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_scale_widget.h"
#include "qwt_plot.h"
#include "qwt_plot_layout.h"
//----------------------------------------------------
// QwtPlotLayoutEngine::Dimensions
//----------------------------------------------------

/**
 * @brief Constructor for Dimensions structure
 * @details Initializes all dimension values to zero for title, footer, and all axis positions.
 */
QwtPlotLayoutEngine::Dimensions::Dimensions()
{
	dimTitle = dimFooter = 0;
	for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++)
		m_dimAxes[ axisPos ] = 0;
}

/**
 * @brief Get the stored dimension for a given axis
 * @param axisId Axis identifier (QwtAxis::XTop, XBottom, YLeft, YRight)
 * @return The reserved pixels for this axis
 */
int QwtPlotLayoutEngine::Dimensions::dimAxis(QwtAxisId axisId) const
{
    return m_dimAxes[ axisId ];
}

/**
 * @brief Set the stored dimension for a given axis
 * @param axisId Axis identifier
 * @param dim New dimension in pixels
 */
void QwtPlotLayoutEngine::Dimensions::setDimAxis(QwtAxisId axisId, int dim)
{
    m_dimAxes[ axisId ] = dim;
}

/**
 * @brief Read dimension by axis position index
 * @param axisPos Axis position enum value (YLeft, YRight, XTop, XBottom)
 * @return Pixel size of the axis at the given position
 */
int QwtPlotLayoutEngine::Dimensions::dimAxes(int axisPos) const
{
    return m_dimAxes[ axisPos ];
}

/**
 * @brief Sum width of left and right Y axes
 * @return Combined width in pixels
 */
int QwtPlotLayoutEngine::Dimensions::dimYAxes() const
{
    return dimAxes(QwtAxis::YLeft) + dimAxes(QwtAxis::YRight);
}

/**
 * @brief Sum height of top and bottom X axes
 * @return Combined height in pixels
 */
int QwtPlotLayoutEngine::Dimensions::dimXAxes() const
{
    return dimAxes(QwtAxis::XTop) + dimAxes(QwtAxis::XBottom);
}

/**
 * @brief Center a label rectangle within available space
 * @details Adjusts the label rectangle to be centered horizontally within the available space after accounting for Y axis dimensions.
 * @param rect The available rectangle
 * @param labelRect The label rectangle to center
 * @return Centered label rectangle
 */
QRectF QwtPlotLayoutEngine::Dimensions::centered(const QRectF& rect, const QRectF& labelRect) const
{
	QRectF r = labelRect;
	r.setX(rect.left() + dimAxes(QwtAxis::YLeft));
	r.setWidth(rect.width() - dimYAxes());

	return r;
}

/**
 * @brief Calculate inner rectangle after accounting for axis dimensions
 * @details Calculates the available space for the canvas after reserving space for all axes.
 * @param rect The outer rectangle
 * @return Inner rectangle for canvas
 */
QRectF QwtPlotLayoutEngine::Dimensions::innerRect(const QRectF& rect) const
{
	QRectF r(rect.x() + dimAxes(QwtAxis::YLeft),
             rect.y() + dimAxes(QwtAxis::XTop),
             rect.width() - dimYAxes(),
             rect.height() - dimXAxes());

	if (r.width() < 0) {
		r.setX(rect.center().x());
		r.setWidth(0);
	}
	if (r.height() < 0) {
		r.setY(rect.center().y());
		r.setHeight(0);
	}

	return r;
}
//----------------------------------------------------
// QwtPlotLayoutEngine::LayoutData::LegendData
//----------------------------------------------------

/**
 * @brief Initialize legend data from a QwtAbstractLegend
 * @details Extracts frame width, scroll extents and size hint from the legend widget.
 * @param legend Pointer to the legend widget
 */
void QwtPlotLayoutEngine::LayoutData::LegendData::init(const QwtAbstractLegend* legend)
{
	if (legend) {
		frameWidth    = legend->frameWidth();
		hScrollExtent = legend->scrollExtent(Qt::Horizontal);
		vScrollExtent = legend->scrollExtent(Qt::Vertical);

		hint = legend->sizeHint();
	}
}

/**
 * \if ENGLISH
 * @brief Calculate optimal legend size for given rectangle
 * @details Determines the best size for the legend within the available space, considering both width and height constraints.
 * @param legend Pointer to the legend widget
 * @param rect Available rectangle for the legend
 * @return Optimal size for the legend
 */
QSize QwtPlotLayoutEngine::LayoutData::LegendData::legendHint(const QwtAbstractLegend* legend, const QRectF& rect) const
{
	const int w = qMin(hint.width(), qwtFloor(rect.width()));

	int h = legend->heightForWidth(w);
	if (h <= 0)
		h = hint.height();

	return QSize(w, h);
}

//----------------------------------------------------
// QwtPlotLayoutEngine::LayoutData::LabelData
//----------------------------------------------------

/**
 * \if ENGLISH
 * @brief Initialize label data from a QwtTextLabel
 * @details Extracts text content and frame width from the label widget.
 * @param label Pointer to the text label widget
 */
void QwtPlotLayoutEngine::LayoutData::LabelData::init(const QwtTextLabel* label)
{
	frameWidth = 0;
	text       = QwtText();

	if (label) {
		text = label->text();
		if (!(text.testPaintAttribute(QwtText::PaintUsingTextFont)))
			text.setFont(label->font());

		frameWidth = label->frameWidth();
	}
}

//----------------------------------------------------
// QwtPlotLayoutEngine::LayoutData::ScaleData
//----------------------------------------------------

/**
 * \if ENGLISH
 * @brief Initialize scale data from a QwtScaleWidget
 * @details Extracts various geometric properties from the scale widget including border distances, tick offsets, and dimensions without title.
 * @param axisWidget Pointer to the scale widget
 */
void QwtPlotLayoutEngine::LayoutData::ScaleData::init(const QwtScaleWidget* axisWidget)
{
	isVisible = true;

	scaleWidget = axisWidget;
	scaleFont   = axisWidget->font();

	start = axisWidget->startBorderDist();
	end   = axisWidget->endBorderDist();

	baseLineOffset = axisWidget->margin();
    edgeMargin     = axisWidget->edgeMargin();
    tickOffset     = axisWidget->margin();
	if (axisWidget->scaleDraw()->hasComponent(QwtAbstractScaleDraw::Ticks))
		tickOffset += axisWidget->scaleDraw()->maxTickLength();

	dimWithoutTitle = axisWidget->dimForLength(QWIDGETSIZE_MAX, scaleFont);
	if (!axisWidget->title().isEmpty())
		dimWithoutTitle -= axisWidget->titleHeightForWidth(QWIDGETSIZE_MAX);
}

/**
 * \if ENGLISH
 * @brief Reset scale data to default values
 * @details Sets all scale data properties to zero or false, effectively making the axis invisible in layout calculations.
 */
void QwtPlotLayoutEngine::LayoutData::ScaleData::reset()
{
	isVisible       = false;
	start           = 0;
	end             = 0;
	baseLineOffset  = 0;
	tickOffset      = 0.0;
	dimWithoutTitle = 0;
}

//----------------------------------------------------
// QwtPlotLayoutEngine::LayoutData::CanvasData
//----------------------------------------------------

/**
 * \if ENGLISH
 * @brief Initialize canvas data from a QWidget
 * @details Extracts content margins from the canvas widget for all four sides.
 * @param canvas Pointer to the canvas widget
 */
void QwtPlotLayoutEngine::LayoutData::CanvasData::init(const QWidget* canvas)
{
	const QMargins m = canvas->contentsMargins();

	contentsMargins[ QwtAxis::YLeft ]   = m.left();
	contentsMargins[ QwtAxis::XTop ]    = m.top();
	contentsMargins[ QwtAxis::YRight ]  = m.right();
	contentsMargins[ QwtAxis::XBottom ] = m.bottom();
}
//----------------------------------------------------
// QwtPlotLayoutEngine::LayoutData
//----------------------------------------------------

/**
 * \if ENGLISH
 * @brief Construct LayoutData from a QwtPlot
 * @details Initializes all layout data by extracting information from the plot's components including legend, labels, axes and canvas.
 * @param plot Pointer to the QwtPlot
 */
QwtPlotLayoutEngine::LayoutData::LayoutData(const QwtPlot* plot)
{
	legendData.init(plot->legend());
	labelData[ Title ].init(plot->titleLabel());
	labelData[ Footer ].init(plot->footerLabel());

	for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
		{
			const QwtAxisId axisId(axisPos);

			ScaleData& scaleData = axisData(axisId);

			if (plot->isAxisVisible(axisId)) {
				const QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
				scaleData.init(scaleWidget);
			} else {
				scaleData.reset();
			}
		}
	}

	canvasData.init(plot->canvas());
}

/**
 * \if ENGLISH
 * @brief Check if Y axes are symmetric
 * @details Determines if both left and right Y axes are visible or both are hidden.
 * @return True if both Y axes have the same visibility state
 */
bool QwtPlotLayoutEngine::LayoutData::hasSymmetricYAxes() const
{
    return m_scaleData[ QwtAxis::YLeft ].isVisible == m_scaleData[ QwtAxis::YRight ].isVisible;
}

/**
 * @brief Get scale data for a specific axis (const version)
 * @param axisId Axis identifier
 * @return Constant reference to the scale data
 */
const QwtPlotLayoutEngine::LayoutData::ScaleData& QwtPlotLayoutEngine::LayoutData::axisData(QwtAxisId axisId) const
{
    return m_scaleData[ axisId ];
}

/**
 * @brief Get scale data for a specific axis (mutable version)
 * @param axisId Axis identifier
 * @return Mutable reference to the scale data
 */
QwtPlotLayoutEngine::LayoutData::ScaleData& QwtPlotLayoutEngine::LayoutData::axisData(QwtAxisId axisId)
{
    return m_scaleData[ axisId ];
}

/**
 * @brief Get tick offset for a specific axis position
 * @param axisPos Axis position (0-3)
 * @return Tick offset value
 */
double QwtPlotLayoutEngine::LayoutData::tickOffset(int axisPos) const
{
    return axisData(axisPos).tickOffset;
}

//----------------------------------------------------
// QwtPlotLayoutEngine
//----------------------------------------------------

/**
 * @brief Default constructor for QwtPlotLayoutEngine
 * @details Initializes the layout engine with default values: legend position Bottom, legend ratio 1.0, spacing 5 pixels.
 */
QwtPlotLayoutEngine::QwtPlotLayoutEngine() : m_legendPos(QwtPlot::BottomLegend), m_legendRatio(1.0), m_spacing(5)
{
}
/**
 * \if ENGLISH
 * @brief Calculate legend rectangle within available space
 * @details Determines the optimal position and size for the legend based on the specified legend position and ratio constraints.
 * @param[in] plotLayoutOptions Layout options bitmask
 * @param[in] legendData Precalculated legend data
 * @param[in] rect Available rectangle for layout
 * @param[in] legendHint Preferred size hint for the legend
 * @return Calculated legend rectangle
 */
QRectF QwtPlotLayoutEngine::layoutLegend(int plotLayoutOptions,
                                          const LayoutData::LegendData& legendData,
                                          const QRectF& rect,
                                          const QSize& legendHint) const
{
	QwtPlotLayout::Options options = static_cast< QwtPlotLayout::Options >(plotLayoutOptions);
	int dim;
	if (m_legendPos == QwtPlot::LeftLegend || m_legendPos == QwtPlot::RightLegend) {
		// We don't allow vertical legends to take more than
		// half of the available space.

		dim = qMin(legendHint.width(), int(rect.width() * m_legendRatio));

		if (!(options & QwtPlotLayout::IgnoreScrollbars)) {
			if (legendHint.height() > rect.height()) {
				// The legend will need additional
				// space for the vertical scrollbar.

				dim += legendData.hScrollExtent;
			}
		}
	} else {
		dim = qMin(legendHint.height(), int(rect.height() * m_legendRatio));
		dim = qMax(dim, legendData.vScrollExtent);
	}

	QRectF legendRect = rect;
	switch (m_legendPos) {
	case QwtPlot::LeftLegend: {
		legendRect.setWidth(dim);
		break;
	}
	case QwtPlot::RightLegend: {
		legendRect.setX(rect.right() - dim);
		legendRect.setWidth(dim);
		break;
	}
	case QwtPlot::TopLegend: {
		legendRect.setHeight(dim);
		break;
	}
	case QwtPlot::BottomLegend: {
		legendRect.setY(rect.bottom() - dim);
		legendRect.setHeight(dim);
		break;
	}
	}

	return legendRect;
}

/**
 * \if ENGLISH
 * @brief Align legend rectangle relative to canvas
 * @details Adjusts the legend rectangle to be properly aligned with the canvas, ensuring it doesn't extend beyond the canvas boundaries when possible.
 * @param[in] legendHint Preferred size hint for the legend
 * @param[in] canvasRect Canvas rectangle
 * @param[in] legendRect Initial legend rectangle
 * @return Aligned legend rectangle
 */
QRectF QwtPlotLayoutEngine::alignLegend(const QSize& legendHint, const QRectF& canvasRect, const QRectF& legendRect) const
{
	QRectF alignedRect = legendRect;

	if (m_legendPos == QwtPlot::BottomLegend || m_legendPos == QwtPlot::TopLegend) {
		if (legendHint.width() < canvasRect.width()) {
			alignedRect.setX(canvasRect.x());
			alignedRect.setWidth(canvasRect.width());
		}
	} else {
		if (legendHint.height() < canvasRect.height()) {
			alignedRect.setY(canvasRect.y());
			alignedRect.setHeight(canvasRect.height());
		}
	}

	return alignedRect;
}
/**
 * \if ENGLISH
 * @brief Align scale rectangles with canvas
 * @details Adjusts the positions of scale rectangles to ensure proper alignment with the canvas, taking into account border distances and tick offsets.
 *          This function handles the complex layout calculation involving multiple axes and canvas spatial coordination.
 * @param[in] plotLayoutOptions Layout options bitmask
 * @param[in] layoutData Precalculated layout data
 * @param[out] canvasRect Canvas rectangle (may be modified)
 * @param[out] scaleRect Array of scale rectangles (will be modified)
 */
void QwtPlotLayoutEngine::alignScales(int plotLayoutOptions,
                                      const LayoutData& layoutData,
                                      QRectF& canvasRect,
                                      QRectF scaleRect[ QwtAxis::AxisPositions ]) const
{
	QwtPlotLayout::Options options = static_cast< QwtPlotLayout::Options >(plotLayoutOptions);
	using namespace QwtAxis;

	//! 1. Initialize backbone offset
	//! First calculate the backbone offset for each axis, which consists of:
	//! - Add canvas margin if not aligning canvas to scale
	//! - Add canvas content margins if not ignoring frames
	int backboneOffset[ AxisPositions ];
	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		backboneOffset[ axisPos ] = 0;

		if (!m_alignCanvas[ axisPos ]) {
			backboneOffset[ axisPos ] += m_canvasMargin[ axisPos ];
		}

		if (!(options & QwtPlotLayout::IgnoreFrames)) {
			backboneOffset[ axisPos ] += layoutData.canvasData.contentsMargins[ axisPos ];
		}
	}

	//! 2. Handle the mutual influence of X and Y axes
	//! Iterate all axis positions and handle the spatial interaction between X and Y axes:
	//!

	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		{
			QRectF& axisRect = scaleRect[ axisPos ];
			if (!axisRect.isValid())
				continue;

			const QwtAxisId axisId(axisPos);

			const int startDist = layoutData.axisData(axisId).start;
			const int endDist   = layoutData.axisData(axisId).end;
			//! 对于X轴（水平轴）：
			//! 检查左侧Y轴和右侧Y轴的影响
			//! 计算左右偏移量，考虑轴起始/结束距离和骨干偏移
			//! 调整X轴矩形位置，必要时调整画布矩形
			//!
			//! 对于Y轴（垂直轴）：
			//! 检查底部X轴和顶部X轴的影响
			//! 计算上下偏移量，考虑轴起始/结束距离和骨干偏移
			//! 调整Y轴矩形位置，必要时调整画布矩形
			if (isXAxis(axisPos)) {
				const QRectF& leftScaleRect = scaleRect[ YLeft ];
				const int leftOffset        = backboneOffset[ YLeft ] - startDist;

				if (leftScaleRect.isValid()) {
					const double dx = leftOffset + leftScaleRect.width();

					//! When the axis needs more space than available, the function adjusts the canvas rectangle
					if (m_alignCanvas[ YLeft ] && dx < 0.0) {
						/*
						   The axis needs more space than the width
						   of the left scale.
						 */
						const double cLeft = canvasRect.left();  // qreal -> double
						canvasRect.setLeft(qwtMaxF(cLeft, axisRect.left() - dx));
					} else {
						const double minLeft = leftScaleRect.left();
						const double left    = axisRect.left() + leftOffset;
						axisRect.setLeft(qwtMaxF(left, minLeft));
					}
				} else {
					if (m_alignCanvas[ YLeft ] && leftOffset < 0) {
						canvasRect.setLeft(qwtMaxF(canvasRect.left(), axisRect.left() - leftOffset));
					} else {
						if (leftOffset > 0)
							axisRect.setLeft(axisRect.left() + leftOffset);
					}
				}

				const QRectF& rightScaleRect = scaleRect[ YRight ];
				const int rightOffset        = backboneOffset[ YRight ] - endDist + 1;

				if (rightScaleRect.isValid()) {
					const double dx = rightOffset + rightScaleRect.width();
					if (m_alignCanvas[ YRight ] && dx < 0) {
						/*
						   The axis needs more space than the width
						   of the right scale.
						 */
						const double cRight = canvasRect.right();  // qreal -> double
						canvasRect.setRight(qwtMinF(cRight, axisRect.right() + dx));
					}

					const double maxRight = rightScaleRect.right();
					const double right    = axisRect.right() - rightOffset;
					axisRect.setRight(qwtMinF(right, maxRight));
				} else {
					if (m_alignCanvas[ YRight ] && rightOffset < 0) {
						canvasRect.setRight(qwtMinF(canvasRect.right(), axisRect.right() + rightOffset));
					} else {
						if (rightOffset > 0)
							axisRect.setRight(axisRect.right() - rightOffset);
					}
				}
			} else  // y axes
			{
				const QRectF& bottomScaleRect = scaleRect[ XBottom ];
				const int bottomOffset        = backboneOffset[ XBottom ] - endDist + 1;

				if (bottomScaleRect.isValid()) {
					const double dy = bottomOffset + bottomScaleRect.height();
					if (m_alignCanvas[ XBottom ] && dy < 0) {
						/*
						   The axis needs more space than the height
						   of the bottom scale.
						 */
						const double cBottom = canvasRect.bottom();  // qreal -> double
						canvasRect.setBottom(qwtMinF(cBottom, axisRect.bottom() + dy));
					} else {
						const double maxBottom = bottomScaleRect.top() + layoutData.tickOffset(XBottom);
						const double bottom    = axisRect.bottom() - bottomOffset;
						axisRect.setBottom(qwtMinF(bottom, maxBottom));
					}
				} else {
					if (m_alignCanvas[ XBottom ] && bottomOffset < 0) {
						canvasRect.setBottom(qwtMinF(canvasRect.bottom(), axisRect.bottom() + bottomOffset));
					} else {
						if (bottomOffset > 0)
							axisRect.setBottom(axisRect.bottom() - bottomOffset);
					}
				}

				const QRectF& topScaleRect = scaleRect[ XTop ];
				const int topOffset        = backboneOffset[ XTop ] - startDist;

				if (topScaleRect.isValid()) {
					const double dy = topOffset + topScaleRect.height();
					if (m_alignCanvas[ XTop ] && dy < 0) {
						/*
						   The axis needs more space than the height
						   of the top scale.
						 */
						const double cTop = canvasRect.top();  // qreal -> double
						canvasRect.setTop(qwtMaxF(cTop, axisRect.top() - dy));
					} else {
						const double minTop = topScaleRect.bottom() - layoutData.tickOffset(XTop);

						const double top = axisRect.top() + topOffset;
						axisRect.setTop(qwtMaxF(top, minTop));
					}
				} else {
					if (m_alignCanvas[ XTop ] && topOffset < 0) {
						canvasRect.setTop(qwtMaxF(canvasRect.top(), axisRect.top() - topOffset));
					} else {
						if (topOffset > 0)
							axisRect.setTop(axisRect.top() + topOffset);
					}
				}
			}
		}
	}

	/*
	   The canvas has been aligned to the scale with largest
	   border distances. Now we have to realign the other scale.
	 */

	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		{
			const QwtAxisId axisId(axisPos);

			QRectF& sRect                         = scaleRect[ axisPos ];
			const LayoutData::ScaleData& axisData = layoutData.axisData(axisId);

			if (!sRect.isValid())
				continue;

			if (isXAxis(axisPos)) {

				//! After mutual adjustment of all axes, the function performs final alignment to ensure axes are properly aligned with canvas edges
				if (m_alignCanvas[ YLeft ]) {
					double y = canvasRect.left() - axisData.start;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						y += layoutData.canvasData.contentsMargins[ YLeft ];

					sRect.setLeft(y);
				}

				if (m_alignCanvas[ YRight ]) {
					double y = canvasRect.right() - 1 + axisData.end;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						y -= layoutData.canvasData.contentsMargins[ YRight ];

					sRect.setRight(y);
				}

				if (m_alignCanvas[ axisPos ]) {
					if (axisPos == XTop)
						sRect.setBottom(canvasRect.top());
					else
						sRect.setTop(canvasRect.bottom());
				}
			} else {
				if (m_alignCanvas[ XTop ]) {
					double x = canvasRect.top() - axisData.start;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						x += layoutData.canvasData.contentsMargins[ XTop ];

					sRect.setTop(x);
				}

				if (m_alignCanvas[ XBottom ]) {
					double x = canvasRect.bottom() - 1 + axisData.end;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						x -= layoutData.canvasData.contentsMargins[ XBottom ];

					sRect.setBottom(x);
				}

				if (m_alignCanvas[ axisPos ]) {
					if (axisPos == YLeft)
						sRect.setRight(canvasRect.left());
					else
						sRect.setLeft(canvasRect.right());
				}
			}
		}
	}
}
/**
 * \if ENGLISH
 * @brief Align scale rectangles to canvas boundaries
 * @details Adjusts the positions of scale rectangles to align with canvas boundaries, taking into account border distances and tick offsets.
 *          Unlike alignScales, this function does not modify the canvas rectangle.
 * @param[in] plotLayoutOptions Layout options bitmask
 * @param[in] layoutData Precalculated layout data
 * @param[in] canvasRect Canvas rectangle
 * @param[out] scaleRect Array of scale rectangles (will be modified)
 */
void QwtPlotLayoutEngine::alignScalesToCanvas(int plotLayoutOptions,
                                              const QwtPlotLayoutEngine::LayoutData& layoutData,
                                              const QRectF& canvasRect,
                                              QRectF scaleRect[ QwtAxis::AxisPositions ]) const
{
	QwtPlotLayout::Options options = static_cast< QwtPlotLayout::Options >(plotLayoutOptions);
	using namespace QwtAxis;

	//! 1. Initialize backbone offset
	//! First calculate the backbone offset for each axis, which consists of:
	//! - Add canvas margin if not aligning canvas to scale
	//! - Add canvas content margins if not ignoring frames
	int backboneOffset[ AxisPositions ];
	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		backboneOffset[ axisPos ] = 0;

		if (!m_alignCanvas[ axisPos ]) {
			backboneOffset[ axisPos ] += m_canvasMargin[ axisPos ];
		}

		if (!(options & QwtPlotLayout::IgnoreFrames)) {
			backboneOffset[ axisPos ] += layoutData.canvasData.contentsMargins[ axisPos ];
		}
	}

	//! 2. Handle the mutual influence of X and Y axes
	//! Iterate all axis positions and handle the spatial interaction between X and Y axes:
	//!

	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		{
			QRectF& axisRect = scaleRect[ axisPos ];
			if (!axisRect.isValid())
				continue;

			const QwtAxisId axisId(axisPos);

			const int startDist = layoutData.axisData(axisId).start;
			const int endDist   = layoutData.axisData(axisId).end;
			//! 对于X轴（水平轴）：
			//! 检查左侧Y轴和右侧Y轴的影响
			//! 计算左右偏移量，考虑轴起始/结束距离和骨干偏移
			//! 调整X轴矩形位置，必要时调整画布矩形
			//!
			//! 对于Y轴（垂直轴）：
			//! 检查底部X轴和顶部X轴的影响
			//! 计算上下偏移量，考虑轴起始/结束距离和骨干偏移
			//! 调整Y轴矩形位置，必要时调整画布矩形
			if (isXAxis(axisPos)) {
				const QRectF& leftScaleRect = scaleRect[ YLeft ];
				const int leftOffset        = backboneOffset[ YLeft ] - startDist;

				if (leftScaleRect.isValid()) {
					const double minLeft = leftScaleRect.left();
					const double left    = axisRect.left() + leftOffset;
					axisRect.setLeft(qwtMaxF(left, minLeft));
				} else {
					if (leftOffset > 0) {
						axisRect.setLeft(axisRect.left() + leftOffset);
					}
				}

				const QRectF& rightScaleRect = scaleRect[ YRight ];
				const int rightOffset        = backboneOffset[ YRight ] - endDist + 1;

				if (rightScaleRect.isValid()) {
					const double maxRight = rightScaleRect.right();
					const double right    = axisRect.right() - rightOffset;
					axisRect.setRight(qwtMinF(right, maxRight));
				} else {
					if (rightOffset > 0) {
						axisRect.setRight(axisRect.right() - rightOffset);
					}
				}
			} else {  // y axes
				const QRectF& bottomScaleRect = scaleRect[ XBottom ];
				const int bottomOffset        = backboneOffset[ XBottom ] - endDist + 1;

				if (bottomScaleRect.isValid()) {
					const double maxBottom = bottomScaleRect.top() + layoutData.tickOffset(XBottom);
					const double bottom    = axisRect.bottom() - bottomOffset;
					axisRect.setBottom(qwtMinF(bottom, maxBottom));
				} else {

					if (bottomOffset > 0) {
						axisRect.setBottom(axisRect.bottom() - bottomOffset);
					}
				}

				const QRectF& topScaleRect = scaleRect[ XTop ];
				const int topOffset        = backboneOffset[ XTop ] - startDist;

				if (topScaleRect.isValid()) {
					const double minTop = topScaleRect.bottom() - layoutData.tickOffset(XTop);
					const double top    = axisRect.top() + topOffset;
					axisRect.setTop(qwtMaxF(top, minTop));
				} else {
					if (topOffset > 0) {
						axisRect.setTop(axisRect.top() + topOffset);
					}
				}
			}
		}
	}

	/*
	   The canvas has been aligned to the scale with largest
	   border distances. Now we have to realign the other scale.
	 */

	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		{
			const QwtAxisId axisId(axisPos);

			QRectF& sRect                         = scaleRect[ axisPos ];
			const LayoutData::ScaleData& axisData = layoutData.axisData(axisId);

			if (!sRect.isValid())
				continue;

			if (isXAxis(axisPos)) {

				//! After mutual adjustment of all axes, the function performs final alignment to ensure axes are properly aligned with canvas edges
				if (m_alignCanvas[ YLeft ]) {
					double y = canvasRect.left() - axisData.start;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						y += layoutData.canvasData.contentsMargins[ YLeft ];

					sRect.setLeft(y);
				}

				if (m_alignCanvas[ YRight ]) {
					double y = canvasRect.right() - 1 + axisData.end;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						y -= layoutData.canvasData.contentsMargins[ YRight ];

					sRect.setRight(y);
				}

				if (m_alignCanvas[ axisPos ]) {
					if (axisPos == XTop)
						sRect.setBottom(canvasRect.top());
					else
						sRect.setTop(canvasRect.bottom());
				}
			} else {
				if (m_alignCanvas[ XTop ]) {
					double x = canvasRect.top() - axisData.start;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						x += layoutData.canvasData.contentsMargins[ XTop ];

					sRect.setTop(x);
				}

				if (m_alignCanvas[ XBottom ]) {
					double x = canvasRect.bottom() - 1 + axisData.end;
					if (!(options & QwtPlotLayout::IgnoreFrames))
						x -= layoutData.canvasData.contentsMargins[ XBottom ];

					sRect.setBottom(x);
				}

				if (m_alignCanvas[ axisPos ]) {
					if (axisPos == YLeft)
						sRect.setRight(canvasRect.left());
					else
						sRect.setLeft(canvasRect.right());
				}
			}
		}
	}
}
/**
 * \if ENGLISH
 * @brief Iteratively calculate the layout dimensions for a QwtPlot
 * @details Determines the exact space (in pixels) that every visual component of a QwtPlot—title, footer, and the four axes—needs inside a given rectangle.
 *          Because the required size of one component affects the available space for all others, the algorithm loops until the dimensions stabilize.
 * @param[in] plotLayoutOptions Layout options bitmask
 * @param[in] layoutData Precalculated layout data
 * @param[in] rect Available rectangle for layout
 * @return Dimensions structure with calculated sizes
 * \endif
 *
 * \if CHINESE
 * @brief 迭代计算QwtPlot的布局尺寸
 * @details 本函数在给定矩形区域内，精确地计算QwtPlot中每一个可视部件（标题、页脚以及四个坐标轴）所占用的像素尺寸。
 *          由于各部件相互影响，因此采用迭代方式，直到所有尺寸不再变化。
 * @param[in] plotLayoutOptions 布局选项位掩码
 * @param[in] layoutData 预计算的布局数据
 * @param[in] rect 布局的可用矩形区域
 * @return 包含计算尺寸的Dimensions结构体
 * \endif
 */
QwtPlotLayoutEngine::Dimensions QwtPlotLayoutEngine::layoutDimensions(int plotLayoutOptions,
                                                                       const LayoutData& layoutData,
                                                                       const QRectF& rect) const
{
	using namespace QwtAxis;
	QwtPlotLayout::Options options = static_cast< QwtPlotLayout::Options >(plotLayoutOptions);
	Dimensions dimensions;

	int backboneOffset[ AxisPositions ];
	for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
		backboneOffset[ axisPos ] = 0;
		if (!(options & QwtPlotLayout::IgnoreFrames))
			backboneOffset[ axisPos ] += layoutData.canvasData.contentsMargins[ axisPos ];

		if (!m_alignCanvas[ axisPos ])
			backboneOffset[ axisPos ] += m_canvasMargin[ axisPos ];
	}

	bool done = false;
	while (!done) {
		done = true;

		// the size for the 4 axis depend on each other. Expanding
		// the height of a horizontal axis will shrink the height
		// for the vertical axis, shrinking the height of a vertical
		// axis will result in a line break what will expand the
		// width and results in shrinking the width of a horizontal
		// axis what might result in a line break of a horizontal
		// axis ... . So we loop as long until no size changes.
        // 四个坐标轴的尺寸彼此相互影响：
        // 增加水平轴的高度会压缩垂直轴的高度；
        // 垂直轴高度一旦减小，就可能引发换行，从而增加其宽度，
        // 进而压缩水平轴的可用宽度，又可能导致水平轴换行……
        // 因此需要循环计算，直到所有尺寸都不再变化为止。

		if (!(options & QwtPlotLayout::IgnoreTitle)) {
			const int d = heightForWidth(LayoutData::Title, layoutData, options, rect.width(), dimensions.dimYAxes());

			if (d > dimensions.dimTitle) {
				dimensions.dimTitle = d;
				done                = false;
			}
		}

		if (!(options & QwtPlotLayout::IgnoreFooter)) {
			const int d = heightForWidth(LayoutData::Footer, layoutData, options, rect.width(), dimensions.dimYAxes());

			if (d > dimensions.dimFooter) {
				dimensions.dimFooter = d;
				done                 = false;
			}
		}

		for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
			{
				const QwtAxisId axisId(axisPos);

				const LayoutData::ScaleData& scaleData = layoutData.axisData(axisId);

				if (scaleData.isVisible) {
					double length;
					if (isXAxis(axisPos)) {
						length = rect.width() - dimensions.dimYAxes();
						length -= scaleData.start + scaleData.end;

						if (dimensions.dimAxes(YRight) > 0)
							length -= 1;

						length += qMin(dimensions.dimAxes(YLeft), scaleData.start - backboneOffset[ YLeft ]);

						length += qMin(dimensions.dimAxes(YRight), scaleData.end - backboneOffset[ YRight ]);
                    } else {  // y axis
						length = rect.height() - dimensions.dimXAxes();
						length -= scaleData.start + scaleData.end;
						length -= 1;

						if (dimensions.dimAxes(XBottom) <= 0)
							length -= 1;

						if (dimensions.dimAxes(XTop) <= 0)
							length -= 1;

						/*
						   The tick labels of the y axes are always left/right from the
						   backbone/ticks of the x axes - but we have to take care,
						   that the labels don't overlap.
						 */
						if (dimensions.dimAxes(XBottom) > 0) {
							length += qMin(layoutData.tickOffset(XBottom),
                                           double(scaleData.start - backboneOffset[ XBottom ]));
						}

						if (dimensions.dimAxes(XTop) > 0) {
							length += qMin(layoutData.tickOffset(XTop), double(scaleData.end - backboneOffset[ XTop ]));
						}

						if (dimensions.dimTitle > 0)
							length -= dimensions.dimTitle + m_spacing;
					}

					int d = scaleData.dimWithoutTitle;
					if (!scaleData.scaleWidget->title().isEmpty()) {
						d += scaleData.scaleWidget->titleHeightForWidth(qwtFloor(length));
					}

					if (d > dimensions.dimAxis(axisId)) {
						dimensions.setDimAxis(axisId, d);
						done = false;
					}
				}
			}
		}
	}

	return dimensions;
}
/**
 * \if ENGLISH
 * @brief Get the legend ratio
 * @return Current legend ratio value
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例比例
 * @return 当前图例比例值
 * \endif
 */
double QwtPlotLayoutEngine::legendRatio() const
{
    return m_legendRatio;
}

/**
 * \if ENGLISH
 * @brief Set the legend ratio
 * @param[in] ratio New legend ratio value
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例比例
 * @param[in] ratio 新的图例比例值
 * \endif
 */
void QwtPlotLayoutEngine::setLegendRatio(double ratio)
{
    m_legendRatio = ratio;
}

/**
 * \if ENGLISH
 * @brief Get the legend position
 * @return Current legend position
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例位置
 * @return 当前图例位置
 * \endif
 */
QwtPlot::LegendPosition QwtPlotLayoutEngine::legendPos() const
{
    return m_legendPos;
}

/**
 * \if ENGLISH
 * @brief Set the legend position
 * @param[in] pos New legend position
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例位置
 * @param[in] pos 新的图例位置
 * \endif
 */
void QwtPlotLayoutEngine::setLegendPos(QwtPlot::LegendPosition pos)
{
    m_legendPos = pos;
}

/**
 * \if ENGLISH
 * @brief Get canvas margin for a specific axis position
 * @param[in] axisPos Axis position (0-3)
 * @return Canvas margin value
 * \endif
 *
 * \if CHINESE
 * @brief 获取特定轴位置的画布边距
 * @param[in] axisPos 轴位置（0-3）
 * @return 画布边距值
 * \endif
 */
int QwtPlotLayoutEngine::canvasMargin(int axisPos) const
{
    return m_canvasMargin[ axisPos ];
}

/**
 * \if ENGLISH
 * @brief Set canvas margin for a specific axis position
 * @param[in] axisPos Axis position (0-3)
 * @param[in] margin New canvas margin value
 * \endif
 *
 * \if CHINESE
 * @brief 设置特定轴位置的画布边距
 * @param[in] axisPos 轴位置（0-3）
 * @param[in] margin 新的画布边距值
 * \endif
 */
void QwtPlotLayoutEngine::setCanvasMargin(int axisPos, int margin)
{
    m_canvasMargin[ axisPos ] = margin;
}

/**
 * \if ENGLISH
 * @brief Check if canvas is aligned to scale for a specific axis position
 * @param[in] axisPos Axis position (0-3)
 * @return True if canvas is aligned to scale
 * \endif
 *
 * \if CHINESE
 * @brief 检查画布是否对齐到特定轴位置的刻度
 * @param[in] axisPos 轴位置（0-3）
 * @return 如果画布对齐到刻度，则返回true
 * \endif
 */
bool QwtPlotLayoutEngine::alignCanvas(int axisPos) const
{
    return m_alignCanvas[ axisPos ];
}

/**
 * \if ENGLISH
 * @brief Set canvas alignment to scale for a specific axis position
 * @param[in] axisPos Axis position (0-3)
 * @param[in] on True to align canvas to scale
 * \endif
 *
 * \if CHINESE
 * @brief 设置画布对齐到特定轴位置的刻度
 * @param[in] axisPos 轴位置（0-3）
 * @param[in] on 为true时对齐画布到刻度
 * \endif
 */
void QwtPlotLayoutEngine::setAlignCanvas(int axisPos, bool on)
{
    m_alignCanvas[ axisPos ] = on;
}

/**
 * \if ENGLISH
 * @brief Get spacing value
 * @return Current spacing value in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 获取间距值
 * @return 当前的间距值（像素）
 * \endif
 */
unsigned int QwtPlotLayoutEngine::spacing() const
{
    return m_spacing;
}

/**
 * \if ENGLISH
 * @brief Set spacing value
 * @param[in] spacing New spacing value in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 设置间距值
 * @param[in] spacing 新的间距值（像素）
 * \endif
 */
void QwtPlotLayoutEngine::setSpacing(unsigned int spacing)
{
    m_spacing = spacing;
}
/**
 * @brief Calculate height for width for a label / 计算标签的宽度对应高度
 *
 * Determines the required height for a label given a specific width,
 * taking into account text wrapping and frame width.
 *
 * 在给定特定宽度的情况下确定标签所需的高度，考虑文本换行和边框宽度。
 *
 * @param labelType Type of label (Title or Footer) / 标签类型（标题或页脚）
 * @param layoutData Precalculated layout data / 预计算的布局数据
 * @param plotLayoutOptions Layout options bitmask / 布局选项位掩码
 * @param width Available width / 可用宽度
 * @param axesWidth Total width of axes / 轴的总宽度
 * @return Calculated height / 计算出的高度
 */
int QwtPlotLayoutEngine::heightForWidth(LayoutData::Label labelType,
                                         const LayoutData& layoutData,
                                         int plotLayoutOptions,
                                         double width,
                                         int axesWidth) const
{
	QwtPlotLayout::Options options         = static_cast< QwtPlotLayout::Options >(plotLayoutOptions);
	const LayoutData::LabelData& labelData = layoutData.labelData[ labelType ];

	if (labelData.text.isEmpty())
		return 0;

	double w = width;

	if (!layoutData.hasSymmetricYAxes()) {
		// center to the canvas
		w -= axesWidth;
	}

	int d = qwtCeil(labelData.text.heightForWidth(w));
	if (!(options & QwtPlotLayout::IgnoreFrames))
		d += 2 * labelData.frameWidth;

	return d;
}
