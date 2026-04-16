/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_layout.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_legend.h"

class QwtPolarLayout::LayoutData
{
public:
    void init(const QwtPolarPlot*, const QRectF& rect);

    struct t_legendData
    {
        int frameWidth;
        int hScrollExtent;
        int vScrollExtent;
        QSizeF hint;
    } legend;

    struct t_titleData
    {
        QwtText text;
        int frameWidth;
    } title;

    struct t_canvasData
    {
        int frameWidth;
    } canvas;
};

void QwtPolarLayout::LayoutData::init(const QwtPolarPlot* plot, const QRectF& rect)
{
    // legend

    if (plot->plotLayout()->legendPosition() != QwtPolarPlot::ExternalLegend && plot->legend()) {
        legend.frameWidth    = plot->legend()->frameWidth();
        legend.hScrollExtent = plot->legend()->scrollExtent(Qt::Horizontal);
        legend.vScrollExtent = plot->legend()->scrollExtent(Qt::Vertical);

        const QSizeF hint = plot->legend()->sizeHint();

        double w = qMin(hint.width(), rect.width());
        double h = plot->legend()->heightForWidth(w);
        if (h == 0.0)
            h = hint.height();

        if (h > rect.height())
            w += legend.hScrollExtent;

        legend.hint = QSizeF(w, h);
    }

    // title

    title.frameWidth = 0;
    title.text       = QwtText();

    if (plot->titleLabel()) {
        const QwtTextLabel* label = plot->titleLabel();
        title.text                = label->text();
        if (!(title.text.testPaintAttribute(QwtText::PaintUsingTextFont)))
            title.text.setFont(label->font());

        title.frameWidth = plot->titleLabel()->frameWidth();
    }

    // canvas

    canvas.frameWidth = plot->canvas()->frameWidth();
}

class QwtPolarLayout::PrivateData
{
public:
    PrivateData() : margin(0), spacing(0)
    {
    }

    QRectF titleRect;
    QRectF legendRect;
    QRectF canvasRect;

    QwtPolarLayout::LayoutData layoutData;

    QwtPolarPlot::LegendPosition legendPos;
    double legendRatio;

    unsigned int margin;
    unsigned int spacing;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * \endif
 */
QwtPolarLayout::QwtPolarLayout()
{
    m_data = new PrivateData;

    setLegendPosition(QwtPolarPlot::BottomLegend);
    invalidate();
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
QwtPolarLayout::~QwtPolarLayout()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Specify the position of the legend
 * @param[in] pos The legend's position
 * @param[in] ratio Ratio between legend and the bounding rect of title, canvas and axes.
 *                  The legend will be shrunk if it would need more space than the given ratio.
 *                  The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0 it will be reset to the default ratio.
 *                  The default vertical/horizontal ratio is 0.33/0.5.
 * @sa QwtPolarPlot::setLegendPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 指定图例的位置
 * @param[in] pos 图例的位置
 * @param[in] ratio 图例与标题、画布和轴的边界矩形之间的比例。
 *                  如果图例需要的空间超过给定比例，它将被缩小。
 *                  比例限制在 ]0.0 .. 1.0]。如果 <= 0.0，将重置为默认比例。
 *                  默认的垂直/水平比例为 0.33/0.5。
 * @sa QwtPolarPlot::setLegendPosition()
 * \endif
 */
void QwtPolarLayout::setLegendPosition(QwtPolarPlot::LegendPosition pos, double ratio)
{
    if (ratio > 1.0)
        ratio = 1.0;

    switch (pos) {
    case QwtPolarPlot::TopLegend:
    case QwtPolarPlot::BottomLegend: {
        if (ratio <= 0.0)
            ratio = 0.33;
        m_data->legendRatio = ratio;
        m_data->legendPos   = pos;
        break;
    }
    case QwtPolarPlot::LeftLegend:
    case QwtPolarPlot::RightLegend: {
        if (ratio <= 0.0)
            ratio = 0.5;
        m_data->legendRatio = ratio;
        m_data->legendPos   = pos;
        break;
    }
    case QwtPolarPlot::ExternalLegend: {
        m_data->legendRatio = ratio;  // meaningless
        m_data->legendPos   = pos;
        break;
    }
    default:
        break;
    }
}

/**
 * \if ENGLISH
 * @brief Specify the position of the legend
 * @param[in] pos The legend's position. Valid values are \c QwtPolarPlot::LeftLegend,
 *                \c QwtPolarPlot::RightLegend, \c QwtPolarPlot::TopLegend, \c QwtPolarPlot::BottomLegend.
 * @sa QwtPolarPlot::setLegendPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 指定图例的位置
 * @param[in] pos 图例的位置。有效值为 \c QwtPolarPlot::LeftLegend,
 *                \c QwtPolarPlot::RightLegend, \c QwtPolarPlot::TopLegend, \c QwtPolarPlot::BottomLegend。
 * @sa QwtPolarPlot::setLegendPosition()
 * \endif
 */
void QwtPolarLayout::setLegendPosition(QwtPolarPlot::LegendPosition pos)
{
    setLegendPosition(pos, 0.0);
}

/**
 * \if ENGLISH
 * @brief Get the position of the legend
 * @return Position of the legend
 * @sa setLegendPosition(), QwtPolarPlot::setLegendPosition(), QwtPolarPlot::legendPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例的位置
 * @return 图例的位置
 * @sa setLegendPosition(), QwtPolarPlot::setLegendPosition(), QwtPolarPlot::legendPosition()
 * \endif
 */
QwtPolarPlot::LegendPosition QwtPolarLayout::legendPosition() const
{
    return m_data->legendPos;
}

/**
 * \if ENGLISH
 * @brief Specify the relative size of the legend in the plot
 * @param[in] ratio Ratio between legend and the bounding rect of title, canvas and axes.
 *                  The legend will be shrunk if it would need more space than the given ratio.
 *                  The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0 it will be reset to the default ratio.
 *                  The default vertical/horizontal ratio is 0.33/0.5.
 * @sa setLegendPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 指定图例在绘图中的相对大小
 * @param[in] ratio 图例与标题、画布和轴的边界矩形之间的比例。
 *                  如果图例需要的空间超过给定比例，它将被缩小。
 *                  比例限制在 ]0.0 .. 1.0]。如果 <= 0.0，将重置为默认比例。
 *                  默认的垂直/水平比例为 0.33/0.5。
 * @sa setLegendPosition()
 * \endif
 */
void QwtPolarLayout::setLegendRatio(double ratio)
{
    setLegendPosition(legendPosition(), ratio);
}

/**
 * \if ENGLISH
 * @brief Get the relative size of the legend in the plot
 * @return The relative size of the legend in the plot
 * @sa setLegendPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例在绘图中的相对大小
 * @return 图例在绘图中的相对大小
 * @sa setLegendPosition()
 * \endif
 */
double QwtPolarLayout::legendRatio() const
{
    return m_data->legendRatio;
}

/**
 * \if ENGLISH
 * @brief Get geometry for the title
 * @return Geometry for the title
 * @sa activate(), invalidate()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标题的几何形状
 * @return 标题的几何形状
 * @sa activate(), invalidate()
 * \endif
 */
const QRectF& QwtPolarLayout::titleRect() const
{
    return m_data->titleRect;
}

/**
 * \if ENGLISH
 * @brief Get geometry for the legend
 * @return Geometry for the legend
 * @sa activate(), invalidate()
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例的几何形状
 * @return 图例的几何形状
 * @sa activate(), invalidate()
 * \endif
 */
const QRectF& QwtPolarLayout::legendRect() const
{
    return m_data->legendRect;
}

/**
 * \if ENGLISH
 * @brief Get geometry for the canvas
 * @return Geometry for the canvas
 * @sa activate(), invalidate()
 * \endif
 *
 * \if CHINESE
 * @brief 获取画布的几何形状
 * @return 画布的几何形状
 * @sa activate(), invalidate()
 * \endif
 */
const QRectF& QwtPolarLayout::canvasRect() const
{
    return m_data->canvasRect;
}

/**
 * \if ENGLISH
 * @brief Invalidate the geometry of all components
 * @sa activate()
 * \endif
 *
 * \if CHINESE
 * @brief 使所有组件的几何形状无效
 * @sa activate()
 * \endif
 */
void QwtPolarLayout::invalidate()
{
    m_data->titleRect = m_data->legendRect = m_data->canvasRect = QRect();
}

/**
 * \if ENGLISH
 * @brief Find the geometry for the legend
 * @param[in] options Options how to layout the legend
 * @param[in,out] rect Rectangle where to place the legend
 * @return Geometry for the legend
 * \endif
 *
 * \if CHINESE
 * @brief 查找图例的几何形状
 * @param[in] options 如何布局图例的选项
 * @param[in,out] rect 放置图例的矩形
 * @return 图例的几何形状
 * \endif
 */
QRectF QwtPolarLayout::layoutLegend(Options options, QRectF& rect) const
{
    const QSizeF hint(m_data->layoutData.legend.hint);

    int dim;
    if (m_data->legendPos == QwtPolarPlot::LeftLegend || m_data->legendPos == QwtPolarPlot::RightLegend) {
        // We don't allow vertical legends to take more than
        // half of the available space.

        dim = qMin(double(hint.width()), rect.width() * m_data->legendRatio);

        if (!(options & IgnoreScrollbars)) {
            if (hint.height() > rect.height()) {
                // The legend will need additional
                // space for the vertical scrollbar.

                dim += m_data->layoutData.legend.hScrollExtent;
            }
        }
    } else {
        dim = qMin(double(hint.height()), rect.height() * m_data->legendRatio);
        dim = qMax(dim, m_data->layoutData.legend.vScrollExtent);
    }

    QRectF legendRect = rect;
    switch (m_data->legendPos) {
    case QwtPolarPlot::LeftLegend: {
        legendRect.setWidth(dim);
        rect.setLeft(legendRect.right());
        break;
    }
    case QwtPolarPlot::RightLegend: {
        legendRect.setX(rect.right() - dim + 1);
        legendRect.setWidth(dim);
        rect.setRight(legendRect.left());
        break;
    }
    case QwtPolarPlot::TopLegend: {
        legendRect.setHeight(dim);
        rect.setTop(legendRect.bottom());
        break;
    }
    case QwtPolarPlot::BottomLegend: {
        legendRect.setY(rect.bottom() - dim + 1);
        legendRect.setHeight(dim);
        rect.setBottom(legendRect.top());
        break;
    }
    case QwtPolarPlot::ExternalLegend:
        break;
    }

    return legendRect;
}

/**
 * \if ENGLISH
 * @brief Recalculate the geometry of all components
 * @param[in] plot Plot to be layout
 * @param[in] boundingRect Rect where to place the components
 * @param[in] options Options
 * @sa invalidate(), titleRect(), legendRect(), canvasRect()
 * \endif
 *
 * \if CHINESE
 * @brief 重新计算所有组件的几何形状
 * @param[in] plot 要布局的绘图
 * @param[in] boundingRect 放置组件的矩形
 * @param[in] options 选项
 * @sa invalidate(), titleRect(), legendRect(), canvasRect()
 * \endif
 */
void QwtPolarLayout::activate(const QwtPolarPlot* plot, const QRectF& boundingRect, Options options)
{
    invalidate();

    QRectF rect(boundingRect);  // undistributed rest of the plot rect
    int margin = static_cast< int >(m_data->margin);
    rect.adjust(margin, margin, -margin, -margin);

    // We extract all layout relevant data from the widgets
    // and save them to m_data->layoutData.

    m_data->layoutData.init(plot, rect);
    if (!(options & IgnoreLegend) && m_data->legendPos != QwtPolarPlot::ExternalLegend && plot->legend()
        && !plot->legend()->isEmpty()) {
        m_data->legendRect = layoutLegend(options, rect);
        if (m_data->layoutData.legend.frameWidth && !(options & IgnoreFrames)) {
            // In case of a frame we have to insert a spacing.
            // Otherwise the leading of the font separates
            // legend and scale/canvas

            switch (m_data->legendPos) {
            case QwtPolarPlot::LeftLegend:
                rect.setLeft(rect.left() + m_data->spacing);
                break;

            case QwtPolarPlot::RightLegend:
                rect.setRight(rect.right() - m_data->spacing);
                break;

            case QwtPolarPlot::TopLegend:
                rect.setTop(rect.top() + m_data->spacing);
                break;

            case QwtPolarPlot::BottomLegend:
                rect.setBottom(rect.bottom() - m_data->spacing);
                break;

            case QwtPolarPlot::ExternalLegend:
                break;  // suppress compiler warning
            }
        }
    }

    if (!(options & IgnoreTitle) && !m_data->layoutData.title.text.isEmpty()) {
        int h = m_data->layoutData.title.text.heightForWidth(rect.width());
        if (!(options & IgnoreFrames))
            h += 2 * m_data->layoutData.title.frameWidth;

        m_data->titleRect = QRectF(rect.x(), rect.y(), rect.width(), h);

        // subtract title
        rect.setTop(rect.top() + h + m_data->spacing);
    }

    if (plot->zoomPos().radius() > 0.0 || plot->zoomFactor() < 1.0) {
        // In zoomed state we have no idea about the geometry that
        // is best for the plot. So we use the complete rectangle
        // accepting, that there might a lot of space wasted
        // around the plot.

        m_data->canvasRect = rect;
    } else {
        // In full state we know, that we want
        // to display something circular.

        const int dim = qMin(rect.width(), rect.height());

        m_data->canvasRect.setX(rect.center().x() - dim / 2);
        m_data->canvasRect.setY(rect.y());
        m_data->canvasRect.setSize(QSize(dim, dim));
    }

    if (!m_data->legendRect.isEmpty()) {
        if (m_data->legendPos == QwtPolarPlot::LeftLegend || m_data->legendPos == QwtPolarPlot::RightLegend) {
            // We prefer to align the legend to the canvas - not to
            // the complete plot - if possible.

            if (m_data->layoutData.legend.hint.height() < m_data->canvasRect.height()) {
                m_data->legendRect.setY(m_data->canvasRect.y());
                m_data->legendRect.setHeight(m_data->canvasRect.height());
            }
        }
    }
}
