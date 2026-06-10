/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *   Summary of major modifications (see ChangeLog.md for full history):
 *   1. CMake build system & C++11 throughout.
 *   2. Core panner/ zoomer refactored:
 *        - QwtPanner -> QwtCachePanner (pixmap-cache version)
 *        - New real-time QwtPlotPanner derived from QwtPicker.
 *   3. Zoomer supports multi-axis.
 *   4. Parasite-plot framework:
 *        - QwtFigure, QwtPlotParasiteLayout, QwtPlotTransparentCanvas,
 *        - QwtPlotScaleEventDispatcher, built-in pan/zoom on axis.
 *   5. New picker: QwtPlotSeriesDataPicker (works with date axis).
 *   6. Raster & color-map extensions:
 *        - QwtGridRasterData (2-D table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 ******************************************************************************/

#include "qwt_plot_layout.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_scale_widget.h"
#include "qwt_abstract_legend.h"
#include "qwt_math.h"
#include "qwt_plot_layout_engine.h"
#include <qmargins.h>

namespace
{
class LayoutHintData
{
public:
    LayoutHintData(const QwtPlot* plot);

    int alignedSize(const QwtAxisId) const;

    inline int yAxesWidth() const
    {
        using namespace QwtAxis;
        return axesWidth(YLeft) + axesWidth(YRight);
    }

    inline int yAxesHeight() const
    {
        using namespace QwtAxis;
        return qMax(axesHeight(YLeft), axesHeight(YRight));
    }

    inline int xAxesHeight() const
    {
        using namespace QwtAxis;
        return axesHeight(XTop) + axesHeight(XBottom);
    }

    inline int xAxesWidth() const
    {
        using namespace QwtAxis;
        return qMax(axesWidth(XTop), axesWidth(XBottom));
    }

private:
    struct ScaleData
    {
        ScaleData()
        {
            w = h = minLeft = minRight = tickOffset = 0;
        }

        int w;
        int h;
        int minLeft;
        int minRight;
        int tickOffset;
    };

    const ScaleData& axisData(QwtAxisId axisId) const
    {
        return m_scaleData[ axisId ];
    }

    ScaleData& axisData(QwtAxisId axisId)
    {
        return m_scaleData[ axisId ];
    }

    inline int axesWidth(int axisPos) const
    {
        return m_scaleData[ axisPos ].w;
    }

    inline int axesHeight(int axisPos) const
    {
        return m_scaleData[ axisPos ].h;
    }

    int m_canvasBorder[ QwtAxis::AxisPositions ];
    ScaleData m_scaleData[ QwtAxis::AxisPositions ];
};

LayoutHintData::LayoutHintData(const QwtPlot* plot)
{
    using namespace QwtAxis;

    const QMargins m = plot->canvas()->contentsMargins();

    int contentsMargins[ 4 ];
    contentsMargins[ YLeft ]   = m.left();
    contentsMargins[ XTop ]    = m.top();
    contentsMargins[ YRight ]  = m.right();
    contentsMargins[ XBottom ] = m.bottom();

    for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
        m_canvasBorder[ axisPos ] = contentsMargins[ axisPos ] + plot->plotLayout()->canvasMargin(axisPos) + 1;
        {
            const QwtAxisId axisId(axisPos);

            if (plot->isAxisVisible(axisId)) {
                const QwtScaleWidget* scl = plot->axisWidget(axisId);

                const QSize hint = scl->minimumSizeHint();

                ScaleData& sd = axisData(axisId);
                sd.w          = hint.width();
                sd.h          = hint.height();
                scl->getBorderDistHint(sd.minLeft, sd.minRight);

                {
                    sd.tickOffset = scl->margin();
                    if (scl->scaleDraw()->hasComponent(QwtAbstractScaleDraw::Ticks))
                        sd.tickOffset += qwtCeil(scl->scaleDraw()->maxTickLength());
                }
            }
        }
    }

    for (int axis = 0; axis < AxisPositions; axis++) {
        const int sz = alignedSize(axis);

        ScaleData& sd = axisData(axis);
        if (isXAxis(axis))
            sd.w = sz;
        else
            sd.h = sz;
    }
}

int LayoutHintData::alignedSize(const QwtAxisId axisId) const
{
    using namespace QwtAxis;

    const ScaleData& sd = axisData(axisId);

    if (sd.w && isXAxis(axisId)) {
        int w = sd.w;

        if (const int leftW = axesWidth(YLeft)) {
            const int shiftLeft = sd.minLeft - m_canvasBorder[ YLeft ];
            if (shiftLeft > 0)
                w -= qMin(shiftLeft, leftW);
        }

        if (const int rightW = axesWidth(YRight)) {
            const int shiftRight = sd.minRight - m_canvasBorder[ YRight ];
            if (shiftRight > 0)
                w -= qMin(shiftRight, rightW);
        }

        return w;
    }

    if (sd.h && isYAxis(axisId)) {
        int h = sd.h;

        if (axesHeight(XBottom)) {
            const int shiftBottom = sd.minLeft - m_canvasBorder[ XBottom ];
            if (shiftBottom > 0)
                h -= qMin(shiftBottom, axisData(XBottom).tickOffset);
        }

        if (axesHeight(XTop)) {
            const int shiftTop = sd.minRight - m_canvasBorder[ XTop ];
            if (shiftTop > 0)
                h -= qMin(shiftTop, axisData(XTop).tickOffset);
        }

        return h;
    }

    return 0;
}
}

class QwtPlotLayout::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotLayout)
public:
    PrivateData(QwtPlotLayout* p) : q_ptr(p) {}

    QRectF titleRect;
    QRectF footerRect;
    QRectF legendRect;
    QRectF scaleRects[ QwtAxis::AxisPositions ];
    QRectF canvasRect;

    QwtPlotLayoutEngine engine;
};

/**
 * @brief Constructor
 * @details Creates a new QwtPlotLayout instance with default settings.
 *          The legend position is set to BottomLegend and canvas margin is set to -1.
 */
QwtPlotLayout::QwtPlotLayout()
    : QWT_PIMPL_CONSTRUCT
{
    setLegendPosition(QwtPlot::BottomLegend);
    setCanvasMargin(-1);
    setAlignCanvasToScales(false);

    invalidate();
}

/**
 * @brief Destructor
 * @details Destroys the QwtPlotLayout instance and releases all allocated resources.
 */
QwtPlotLayout::~QwtPlotLayout()
{
}

/**
 * @brief Set the canvas margin for a specific axis position
 * @param margin The margin value. A negative margin (-1) excludes scale borders.
 *                Positive values add space between scale ticks and canvas edge.
 *                0 means tight against tick marks.
 * @param axisPos Axis position (QwtAxis::Position). Use -1 to apply to all axes.
 * @details The margin is the space above/below the scale ticks.
 *          When alignCanvasToScale() is true, the margin has no effect.
 * @sa canvasMargin(), setAlignCanvasToScale()
 */
void QwtPlotLayout::setCanvasMargin(int margin, int axisPos)
{
    QWT_D(d);
    if (margin < -1)
        margin = -1;

    QwtPlotLayoutEngine& engine = d->engine;

    if (axisPos == -1) {
        for (axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++)
            engine.setCanvasMargin(axisPos, margin);
    } else if (QwtAxis::isValid(axisPos)) {
        engine.setCanvasMargin(axisPos, margin);
    }
}

/**
 * @brief Get the canvas margin for a specific axis position
 * @param axisPos Axis position (QwtAxis::Position)
 * @return The margin around the scale tick borders for the specified axis
 * @sa setCanvasMargin()
 */
int QwtPlotLayout::canvasMargin(int axisPos) const
{
    QWT_DC(d);
    if (!QwtAxis::isValid(axisPos))
        return 0;

    return d->engine.canvasMargin(axisPos);
}

/**
 * @brief Set the align-canvas-to-axis-scales flag for all axes
 * @param on True to align canvas to scales, false otherwise
 * @sa setAlignCanvasToScale(), alignCanvasToScale()
 */
void QwtPlotLayout::setAlignCanvasToScales(bool on)
{
    QWT_D(d);
    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++)
        d->engine.setAlignCanvas(axisPos, on);
}

/**
 * @brief Set the align-canvas-to-axis-scales setting for a specific axis
 * @param axisPos Axis position. This identifies a border of the plot.
 *                For example, QwtAxis::YLeft aligns the left end of X-axes.
 * @param on True to align canvas to scales, false to extend beyond scale ends
 * @details The canvas may either extend beyond the axis scale ends to maximize
 *          its size, or align with the axis scale ends to control its size.
 * @note When on is true, canvasMargin() has no effect
 * @sa setAlignCanvasToScales(), alignCanvasToScale(), setCanvasMargin()
 */
void QwtPlotLayout::setAlignCanvasToScale(int axisPos, bool on)
{
    QWT_D(d);
    if (QwtAxis::isValid(axisPos))
        d->engine.setAlignCanvas(axisPos, on);
}

/**
 * @brief Get the align-canvas-to-axis-scales setting for a specific axis
 * @param axisPos Axis position (QwtAxis::Position)
 * @return True if canvas is aligned to scale ends, false if it extends beyond
 * @sa setAlignCanvasToScale(), setAlignCanvasToScales(), setCanvasMargin()
 */
bool QwtPlotLayout::alignCanvasToScale(int axisPos) const
{
    QWT_DC(d);
    if (!QwtAxis::isValid(axisPos))
        return false;

    return d->engine.alignCanvas(axisPos);
}

/**
 * @brief Set the spacing between plot components
 * @param spacing The spacing value in pixels. Must be >= 0.
 * @sa spacing(), setCanvasMargin()
 */
void QwtPlotLayout::setSpacing(int spacing)
{
    QWT_D(d);
    d->engine.setSpacing(qMax(0, spacing));
}

/**
 * @brief Get the spacing between plot components
 * @return The spacing value in pixels
 * @sa setSpacing(), canvasMargin()
 */
int QwtPlotLayout::spacing() const
{
    QWT_DC(d);
    return d->engine.spacing();
}

/**
 * @brief Set the legend position with a specified ratio
 * @param pos The legend position (LeftLegend, RightLegend, TopLegend, BottomLegend)
 * @param ratio Ratio between legend and the bounding rectangle of title, footer,
 *              canvas and axes. Limited to (0.0, 1.0]. Values <= 0.0 use defaults.
 *              Default vertical/horizontal ratio is 0.33/0.5.
 * @sa legendPosition(), setLegendRatio(), QwtPlot::setLegendPosition()
 */
void QwtPlotLayout::setLegendPosition(QwtPlot::LegendPosition pos, double ratio)
{
    QWT_D(d);
    if (ratio > 1.0)
        ratio = 1.0;

    QwtPlotLayoutEngine& engine = d->engine;

    switch (pos) {
    case QwtPlot::TopLegend:
    case QwtPlot::BottomLegend: {
        if (ratio <= 0.0)
            ratio = 0.33;

        engine.setLegendRatio(ratio);
        engine.setLegendPos(pos);

        break;
    }
    case QwtPlot::LeftLegend:
    case QwtPlot::RightLegend: {
        if (ratio <= 0.0)
            ratio = 0.5;

        engine.setLegendRatio(ratio);
        engine.setLegendPos(pos);

        break;
    }
    default:
        break;
    }
}

/**
 * @brief Set the legend position
 * @param pos The legend position. Valid values are QwtPlot::LeftLegend,
 *            QwtPlot::RightLegend, QwtPlot::TopLegend, QwtPlot::BottomLegend.
 * @sa legendPosition(), QwtPlot::setLegendPosition()
 */
void QwtPlotLayout::setLegendPosition(QwtPlot::LegendPosition pos)
{
    setLegendPosition(pos, 0.0);
}

/**
 * @brief Get the legend position
 * @return The current legend position
 * @sa setLegendPosition(), QwtPlot::setLegendPosition(), QwtPlot::legendPosition()
 */
QwtPlot::LegendPosition QwtPlotLayout::legendPosition() const
{
    QWT_DC(d);
    return d->engine.legendPos();
}

/**
 * @brief Set the relative size of the legend in the plot
 * @param ratio Ratio between legend and the bounding rectangle of title, footer,
 *              canvas and axes. Limited to (0.0, 1.0]. Values <= 0.0 use defaults.
 *              Default vertical/horizontal ratio is 0.33/0.5.
 * @sa legendRatio(), setLegendPosition()
 */
void QwtPlotLayout::setLegendRatio(double ratio)
{
    setLegendPosition(legendPosition(), ratio);
}

/**
 * @brief Get the relative size of the legend in the plot
 * @return The legend ratio value
 * @sa setLegendRatio(), setLegendPosition()
 */
double QwtPlotLayout::legendRatio() const
{
    QWT_DC(d);
    return d->engine.legendRatio();
}

/*!
   @brief Set the geometry for the title

   This method is intended to be used from derived layouts
   overloading activate()

   @sa titleRect(), activate()
 */
void QwtPlotLayout::setTitleRect(const QRectF& rect)
{
    QWT_D(d);
    d->titleRect = rect;
}

/**
 * @brief Get the geometry for the title
 * @return The title rectangle
 * @sa activate(), invalidate(), setTitleRect()
 */
QRectF QwtPlotLayout::titleRect() const
{
    QWT_DC(d);
    return d->titleRect;
}

/*!
   @brief Set the geometry for the footer

   This method is intended to be used from derived layouts
   overloading activate()

   @sa footerRect(), activate()
 */
void QwtPlotLayout::setFooterRect(const QRectF& rect)
{
    QWT_D(d);
    d->footerRect = rect;
}

/**
 * @brief Get the geometry for the footer
 * @return The footer rectangle
 * @sa activate(), invalidate(), setFooterRect()
 */
QRectF QwtPlotLayout::footerRect() const
{
    QWT_DC(d);
    return d->footerRect;
}

/*!
   @brief Set the geometry for the legend

   This method is intended to be used from derived layouts
   overloading activate()

   @param rect Rectangle for the legend

   @sa legendRect(), activate()
 */
void QwtPlotLayout::setLegendRect(const QRectF& rect)
{
    QWT_D(d);
    d->legendRect = rect;
}

/**
 * @brief Get the geometry for the legend
 * @return The legend rectangle
 * @sa activate(), invalidate(), setLegendRect()
 */
QRectF QwtPlotLayout::legendRect() const
{
    QWT_DC(d);
    return d->legendRect;
}

/*!
   @brief Set the geometry for an axis

   This method is intended to be used from derived layouts
   overloading activate()

   @param axisId Axis
   @param rect Rectangle for the scale

   @sa scaleRect(), activate()
 */
void QwtPlotLayout::setScaleRect(QwtAxisId axisId, const QRectF& rect)
{
    QWT_D(d);
    if (QwtAxis::isValid(axisId))
        d->scaleRects[ axisId ] = rect;
}

/**
 * @brief Get the geometry for a scale axis
 * @param axisId Axis identifier
 * @return The scale rectangle for the specified axis, or empty QRectF if invalid
 * @sa activate(), invalidate(), setScaleRect()
 */
QRectF QwtPlotLayout::scaleRect(QwtAxisId axisId) const
{
    QWT_DC(d);
    if (QwtAxis::isValid(axisId))
        return d->scaleRects[ axisId ];

    return QRectF();
}

/*!
   @brief Set the geometry for the canvas

   This method is intended to be used from derived layouts
   overloading activate()

   @sa canvasRect(), activate()
 */
void QwtPlotLayout::setCanvasRect(const QRectF& rect)
{
    QWT_D(d);
    d->canvasRect = rect;
}

QwtPlotLayoutEngine* QwtPlotLayout::layoutEngine()
{
    QWT_D(d);
    return &(d->engine);
}

/**
 * @brief Get the geometry for the canvas
 * @return The canvas rectangle
 * @sa activate(), invalidate(), setCanvasRect()
 */
QRectF QwtPlotLayout::canvasRect() const
{
    QWT_DC(d);
    return d->canvasRect;
}

/**
 * @brief Invalidate the geometry of all components
 * @details Resets all geometry rectangles (title, footer, legend, canvas, and scales)
 *          to empty rectangles. Must call activate() to recalculate geometries.
 * @sa activate()
 */
void QwtPlotLayout::invalidate()
{
    QWT_D(d);
    d->titleRect = d->footerRect = d->legendRect = d->canvasRect = QRectF();

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++)
        d->scaleRects[ axisPos ] = QRect();
}

/**
 * @brief Calculate the minimum size hint for the plot
 * @param plot The plot widget to calculate size hint for
 * @return The minimum size hint
 * @sa QwtPlot::minimumSizeHint()
 */
QSize QwtPlotLayout::minimumSizeHint(const QwtPlot* plot) const
{
    QWT_DC(d);
    LayoutHintData hintData(plot);

    const int xAxesWidth  = hintData.xAxesWidth();
    const int yAxesHeight = hintData.yAxesHeight();

    const QWidget* canvas = plot->canvas();

    const QMargins m          = canvas->contentsMargins();
    const QSize minCanvasSize = canvas->minimumSize();

    int w  = hintData.yAxesWidth();
    int cw = xAxesWidth + m.left() + 1 + m.right() + 1;
    w += qMax(cw, minCanvasSize.width());

    int h  = hintData.xAxesHeight();
    int ch = yAxesHeight + m.top() + 1 + m.bottom() + 1;
    h += qMax(ch, minCanvasSize.height());

    const QwtTextLabel* labels[ 2 ];
    labels[ 0 ] = plot->titleLabel();
    labels[ 1 ] = plot->footerLabel();

    for (int i = 0; i < 2; i++) {
        const QwtTextLabel* label = labels[ i ];
        if (label && !label->text().isEmpty()) {
            // we center on the plot canvas.
            const bool centerOnCanvas = !(plot->isAxisVisible(QwtAxis::YLeft) && plot->isAxisVisible(QwtAxis::YRight));

            int labelW = w;
            if (centerOnCanvas) {
                labelW -= hintData.yAxesWidth();
            }

            int labelH = label->heightForWidth(labelW);
            if (labelH > labelW)  // Compensate for a long title
            {
                w = labelW = labelH;
                if (centerOnCanvas)
                    w += hintData.yAxesWidth();

                labelH = label->heightForWidth(labelW);
            }
            h += labelH + spacing();
        }
    }

    // Compute the legend contribution

    const QwtAbstractLegend* legend = plot->legend();
    if (legend && !legend->isEmpty()) {
        const QwtPlotLayoutEngine& engine = d->engine;

        if (engine.legendPos() == QwtPlot::LeftLegend || engine.legendPos() == QwtPlot::RightLegend) {
            int legendW = legend->sizeHint().width();
            int legendH = legend->heightForWidth(legendW);

            if (legend->frameWidth() > 0)
                w += spacing();

            if (legendH > h)
                legendW += legend->scrollExtent(Qt::Horizontal);

            if (engine.legendRatio() < 1.0)
                legendW = qMin(legendW, int(w / (1.0 - engine.legendRatio())));

            w += legendW + spacing();
        } else {
            int legendW = qMin(legend->sizeHint().width(), w);
            int legendH = legend->heightForWidth(legendW);

            if (legend->frameWidth() > 0)
                h += spacing();

            if (engine.legendRatio() < 1.0)
                legendH = qMin(legendH, int(h / (1.0 - engine.legendRatio())));

            h += legendH + spacing();
        }
    }

    return QSize(w, h);
}

/**
 * @brief Recalculate the geometry of all plot components
 * @param plot The plot widget to layout
 * @param plotRect The rectangle where to place the components (in pixels)
 * @param options Layout options (e.g., ignore legend, ignore specific axes)
 * @details Calculates and stores the geometry for all sub-components (title, footer,
 *          legend, 4 axes, and canvas). Results are stored internally and can be
 *          retrieved via titleRect(), footerRect(), legendRect(), canvasRect(), and scaleRect().
 * @sa invalidate(), titleRect(), footerRect(), legendRect(), scaleRect(), canvasRect()
 */
void QwtPlotLayout::activate(const QwtPlot* plot, const QRectF& plotRect, Options options)
{
    invalidate();
    doActivate(plot, plotRect, options);
}

/*!
 * @brief Concrete implementation of QwtPlotLayout::activate
 * @param plotRect
 * @param options
 */
void QwtPlotLayout::doActivate(const QwtPlot* plot, const QRectF& plotRect, Options options)
{
    QWT_D(d);
    QRectF rect(plotRect);  // undistributed rest of the plot rect

    // We extract all layout relevant parameters from the widgets,
    // and save them to d->layoutData.
    QwtPlotLayoutEngine::LayoutData layoutData(plot);

    QSize legendHint;
    // Process legend first -- if it exists, is non-empty, and the option allows
    if (!(options & IgnoreLegend) && plot->legend() && !plot->legend()->isEmpty()) {

        // Let the legend calculate an ideal size based on its own content
        legendHint = layoutData.legendData.legendHint(plot->legend(), rect);

        d->legendRect = d->engine.layoutLegend(options, layoutData.legendData, rect, legendHint);

        // Subtract the legend rectangle from the remaining rect
        const QRegion region(rect.toRect());
        rect = region.subtracted(d->legendRect.toRect()).boundingRect();

        switch (d->engine.legendPos()) {
        case QwtPlot::LeftLegend: {
            rect.setLeft(rect.left() + spacing());
            break;
        }
        case QwtPlot::RightLegend: {
            rect.setRight(rect.right() - spacing());
            break;
        }
        case QwtPlot::TopLegend: {
            rect.setTop(rect.top() + spacing());
            break;
        }
        case QwtPlot::BottomLegend: {
            rect.setBottom(rect.bottom() - spacing());
            break;
        }
        }
    }

    /*
     +---+-----------+---+
     |       Title       |
     +---+-----------+---+
     |   |   Axis    |   |
     +---+-----------+---+
     | A |           | A |
     | x |  Canvas   | x |
     | i |           | i |
     | s |           | s |
     +---+-----------+---+
     |   |   Axis    |   |
     +---+-----------+---+
     |      Footer       |
     +---+-----------+---+
     */

    // title, footer and axes include text labels. The height of each
    // label depends on its line breaks, that depend on the width
    // for the label. A line break in a horizontal text will reduce
    // the available width for vertical texts and vice versa.
    // layoutDimensions finds the height/width for title, footer and axes
    // including all line breaks.
    using namespace QwtAxis;

    const QwtPlotLayoutEngine::Dimensions dimensions = d->engine.layoutDimensions(options, layoutData, rect);

    if (dimensions.dimTitle > 0) {
        QRectF& labelRect = d->titleRect;

        labelRect.setRect(rect.left(), rect.top(), rect.width(), dimensions.dimTitle);

        rect.setTop(labelRect.bottom() + spacing());

        if (!layoutData.hasSymmetricYAxes()) {
            // if only one of the y axes is missing we align
            // the title centered to the canvas

            labelRect = dimensions.centered(rect, labelRect);
        }
    }

    if (dimensions.dimFooter > 0) {
        QRectF& labelRect = d->footerRect;

        labelRect.setRect(rect.left(), rect.bottom() - dimensions.dimFooter, rect.width(), dimensions.dimFooter);

        rect.setBottom(labelRect.top() - spacing());

        if (!layoutData.hasSymmetricYAxes()) {
            // if only one of the y axes is missing we align
            // the footer centered to the canvas

            labelRect = dimensions.centered(rect, labelRect);
        }
    }

    d->canvasRect = dimensions.innerRect(rect);

    for (int axisPos = 0; axisPos < AxisPositions; axisPos++) {
        // set the rects for the axes

        const int pos = 0;
        {
            const QwtAxisId axisId(axisPos);

            if (dimensions.dimAxis(axisId)) {
                const int dim = dimensions.dimAxis(axisId);

                const QRectF& canvasRect = d->canvasRect;

                QRectF& scaleRect = d->scaleRects[ axisId ];
                scaleRect         = canvasRect;

                switch (axisPos) {
                case YLeft: {
                    scaleRect.setX(canvasRect.left() - pos - dim);
                    scaleRect.setWidth(dim);
                    break;
                }
                case YRight: {
                    scaleRect.setX(canvasRect.right() + pos);
                    scaleRect.setWidth(dim);
                    break;
                }
                case XBottom: {
                    scaleRect.setY(canvasRect.bottom() + pos);
                    scaleRect.setHeight(dim);
                    break;
                }
                case XTop: {
                    scaleRect.setY(canvasRect.top() - pos - dim);
                    scaleRect.setHeight(dim);
                    break;
                }
                }
                scaleRect = scaleRect.normalized();
            }
        }
    }

    // +---+-----------+---+
    // |  <-   Axis   ->   |
    // +-^-+-----------+-^-+
    // | | |           | | |
    // |   |           |   |
    // | A |           | A |
    // | x |  Canvas   | x |
    // | i |           | i |
    // | s |           | s |
    // |   |           |   |
    // | | |           | | |
    // +-V-+-----------+-V-+
    // |   <-  Axis   ->   |
    // +---+-----------+---+

    // The ticks of the axes - not the labels above - should
    // be aligned to the canvas. So we try to use the empty
    // corners to extend the axes, so that the label texts
    // left/right of the min/max ticks are moved into them.

    d->engine.alignScales(options, layoutData, d->canvasRect, d->scaleRects);

    if (!d->legendRect.isEmpty()) {
        // We prefer to align the legend to the canvas - not to
        // the complete plot - if possible.

        d->legendRect = d->engine.alignLegend(legendHint, d->canvasRect, d->legendRect);
    }
}
