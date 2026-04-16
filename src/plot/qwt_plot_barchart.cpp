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
 *        - QwtPanner → QwtCachePanner (pixmap-cache version)
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
 *****************************************************************************/

#include "qwt_plot_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_column_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_legend_data.h"

#include <qpainter.h>

class QwtPlotBarChart::PrivateData
{
public:
    PrivateData() : symbol(new QwtColumnSymbol(QwtColumnSymbol::Box)), legendMode(QwtPlotBarChart::LegendChartTitle)
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtColumnSymbol* symbol;
    QwtPlotBarChart::LegendMode legendMode;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the chart
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 图表的标题
 * \endif
 */
QwtPlotBarChart::QwtPlotBarChart(const QwtText& title) : QwtPlotAbstractBarChart(title)
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] title Title of the chart
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @param[in] title 图表的标题
 * \endif
 */
QwtPlotBarChart::QwtPlotBarChart(const QString& title) : QwtPlotAbstractBarChart(QwtText(title))
{
    init();
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
QwtPlotBarChart::~QwtPlotBarChart()
{
    delete m_data;
}

void QwtPlotBarChart::init()
{
    m_data = new PrivateData;
    setData(new QwtPointSeriesData());
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotBarChart
 * \endif
 * 
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotBarChart
 * \endif
 */
int QwtPlotBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotBarChart;
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of points
 * @param[in] samples Vector of points
 * @note QVector is implicitly shared.
 * @note QPolygonF is derived from QVector<QPointF>.
 * \endif
 * 
 * \if CHINESE
 * @brief 用点数组初始化数据
 * @param[in] samples 点向量
 * @note QVector 是隐式共享的。
 * @note QPolygonF 派生自 QVector<QPointF>。
 * \endif
 */
void QwtPlotBarChart::setSamples(const QVector< QPointF >& samples)
{
    setData(new QwtPointSeriesData(samples));
}

/**
 * \if ENGLISH
 * @brief Initialize data with an array of doubles
 * @details The indices in the array are taken as x coordinate,
 *          while the doubles are interpreted as y values.
 * @param[in] samples Vector of y coordinates
 * @note QVector is implicitly shared.
 * \endif
 * 
 * \if CHINESE
 * @brief 用双精度数组初始化数据
 * @details 数组中的索引作为 x 坐标，双精度值作为 y 值。
 * @param[in] samples y 坐标向量
 * @note QVector 是隐式共享的。
 * \endif
 */
void QwtPlotBarChart::setSamples(const QVector< double >& samples)
{
    QVector< QPointF > points;
    points.reserve(samples.size());

    for (int i = 0; i < samples.size(); i++)
        points += QPointF(i, samples[ i ]);

    setData(new QwtPointSeriesData(points));
}

/**
 * \if ENGLISH
 * @brief Assign a series of samples
 * @details setSamples() is just a wrapper for setData() without any additional
 *          value - beside that it is easier to find for the developer.
 * @param[in] data Data
 * @warning The item takes ownership of the data object, deleting
 *          it when its not used anymore.
 * \endif
 * 
 * \if CHINESE
 * @brief 分配样本系列
 * @details setSamples() 只是 setData() 的包装，没有额外价值，
 *          除了对开发者更容易找到。
 * @param[in] data 数据
 * @warning 该项获得数据对象的所有权，当不再使用时会删除它。
 * \endif
 */
void QwtPlotBarChart::setSamples(QwtSeriesData< QPointF >* data)
{
    setData(data);
}

/**
 * \if ENGLISH
 * @brief Assign a symbol
 * @details The bar chart will take the ownership of the symbol, hence the previously
 *          set symbol will be deleted by setting a new one. If \p symbol is
 *          \c nullptr no symbol will be drawn.
 * @param[in] symbol Symbol
 * @sa symbol()
 * \endif
 * 
 * \if CHINESE
 * @brief 分配符号
 * @details 条形图将获得符号的所有权，因此设置新符号时将删除先前设置的符号。
 *          如果 \p symbol 为 \c nullptr，则不会绘制符号。
 * @param[in] symbol 符号
 * @sa symbol()
 * \endif
 */
void QwtPlotBarChart::setSymbol(QwtColumnSymbol* symbol)
{
    if (symbol != m_data->symbol) {
        delete m_data->symbol;
        m_data->symbol = symbol;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the current symbol
 * @return Current symbol or nullptr, when no symbol has been assigned.
 * @sa setSymbol()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取当前符号
 * @return 当前符号，如果没有分配符号则返回 nullptr。
 * @sa setSymbol()
 * \endif
 */
const QwtColumnSymbol* QwtPlotBarChart::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Set the bar symbol pen
 * @param[in] p Pen for drawing the bar outline
 * \endif
 * 
 * \if CHINESE
 * @brief 设置条形符号的画笔
 * @param[in] p 用于绘制条形轮廓的画笔
 * \endif
 */
void QwtPlotBarChart::setPen(const QPen& p)
{
    if (!m_data->symbol) {
        m_data->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    }

    m_data->symbol->setPen(p);

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the bar symbol pen
 * @return Pen for drawing the bar outline
 * \endif
 * 
 * \if CHINESE
 * @brief 获取条形符号的画笔
 * @return 用于绘制条形轮廓的画笔
 * \endif
 */
QPen QwtPlotBarChart::pen() const
{
    if (m_data->symbol) {
        return m_data->symbol->pen();
    }
    return QPen();
}

/**
 * \if ENGLISH
 * @brief Set the bar symbol brush
 * @param[in] b Brush for filling the bar
 * \endif
 * 
 * \if CHINESE
 * @brief 设置条形符号的画刷
 * @param[in] b 用于填充条形的画刷
 * \endif
 */
void QwtPlotBarChart::setBrush(const QBrush& b)
{
    if (!m_data->symbol) {
        m_data->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    }

    m_data->symbol->setBrush(b);

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the bar symbol brush
 * @return Brush for filling the bar
 * \endif
 * 
 * \if CHINESE
 * @brief 获取条形符号的画刷
 * @return 用于填充条形的画刷
 * \endif
 */
QBrush QwtPlotBarChart::brush() const
{
    if (m_data->symbol) {
        return m_data->symbol->brush();
    }
    return QBrush();
}

/**
 * \if ENGLISH
 * @brief Set the bar symbol frame style
 * @param[in] f Frame style for the bar
 * \endif
 * 
 * \if CHINESE
 * @brief 设置条形符号的边框样式
 * @param[in] f 条形的边框样式
 * \endif
 */
void QwtPlotBarChart::setFrameStyle(QwtColumnSymbol::FrameStyle f)
{
    if (!m_data->symbol) {
        m_data->symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    }

    m_data->symbol->setFrameStyle(f);

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the bar symbol frame style
 * @return Frame style for the bar
 * \endif
 * 
 * \if CHINESE
 * @brief 获取条形符号的边框样式
 * @return 条形的边框样式
 * \endif
 */
QwtColumnSymbol::FrameStyle QwtPlotBarChart::frameStyle() const
{
    if (m_data->symbol) {
        return m_data->symbol->frameStyle();
    }
    return QwtColumnSymbol::NoFrame;
}

/**
 * \if ENGLISH
 * @brief Set the mode that decides what to display on the legend
 * @details In case of LegendBarTitles barTitle() needs to be overloaded
 *          to return individual titles for each bar.
 * @param[in] mode New mode
 * @sa legendMode(), legendData(), barTitle(), QwtPlotItem::ItemAttribute
 * \endif
 * 
 * \if CHINESE
 * @brief 设置决定图例显示内容的模式
 * @details 如果是 LegendBarTitles 模式，需要重载 barTitle()
 *          为每个条形返回单独的标题。
 * @param[in] mode 新模式
 * @sa legendMode(), legendData(), barTitle(), QwtPlotItem::ItemAttribute
 * \endif
 */
void QwtPlotBarChart::setLegendMode(LegendMode mode)
{
    if (mode != m_data->legendMode) {
        m_data->legendMode = mode;
        legendChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the legend mode
 * @return Legend mode
 * @sa setLegendMode()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取图例模式
 * @return 图例模式
 * @sa setLegendMode()
 * \endif
 */
QwtPlotBarChart::LegendMode QwtPlotBarChart::legendMode() const
{
    return m_data->legendMode;
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle of all samples
 * @return Bounding rectangle of all samples. For an empty series the rectangle is invalid.
 * \endif
 * 
 * \if CHINESE
 * @brief 获取所有样本的边界矩形
 * @return 所有样本的边界矩形。对于空系列，矩形无效。
 * \endif
 */
QRectF QwtPlotBarChart::boundingRect() const
{
    const size_t numSamples = dataSize();
    if (numSamples == 0)
        return QwtPlotSeriesItem::boundingRect();

    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if (rect.height() >= 0) {
        const double baseLine = baseline();

        if (rect.bottom() < baseLine)
            rect.setBottom(baseLine);

        if (rect.top() > baseLine)
            rect.setTop(baseLine);
    }

    if (orientation() == Qt::Horizontal)
        rect.setRect(rect.y(), rect.x(), rect.height(), rect.width());

    return rect;
}

/**
 * \if ENGLISH
 * @brief Draw an interval of the bar chart
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates.
 * @param[in] yMap Maps y-values into pixel coordinates.
 * @param[in] canvasRect Contents rect of the canvas
 * @param[in] from Index of the first point to be painted
 * @param[in] to Index of the last point to be painted. If to < 0 the
 *              curve will be painted to its last point.
 * @sa drawSymbols()
 * \endif
 * 
 * \if CHINESE
 * @brief 绘制条形图的一个区间
 * @param[in] painter 绘图器
 * @param[in] xMap 将 x 值映射到像素坐标。
 * @param[in] yMap 将 y 值映射到像素坐标。
 * @param[in] canvasRect 画布的内容矩形
 * @param[in] from 第一个绘制点的索引
 * @param[in] to 最后一个绘制点的索引。如果 to < 0，
 *              曲线将绘制到最后一个点。
 * @sa drawSymbols()
 * \endif
 */
void QwtPlotBarChart::drawSeries(QPainter* painter,
                                 const QwtScaleMap& xMap,
                                 const QwtScaleMap& yMap,
                                 const QRectF& canvasRect,
                                 int from,
                                 int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    if (from < 0)
        from = 0;

    if (from > to)
        return;

    const QRectF br = data()->boundingRect();
    const QwtInterval interval(br.left(), br.right());

    painter->save();

    for (int i = from; i <= to; i++) {
        drawSample(painter, xMap, yMap, canvasRect, interval, i, sample(i));
    }

    painter->restore();
}

/*!
   Calculate the geometry of a bar in widget coordinates

   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rect of the canvas
   \param boundingInterval Bounding interval of sample values
   \param sample Value of the sample

   \return Geometry of the column
 */
QwtColumnRect QwtPlotBarChart::columnRect(const QwtScaleMap& xMap,
                                          const QwtScaleMap& yMap,
                                          const QRectF& canvasRect,
                                          const QwtInterval& boundingInterval,
                                          const QPointF& sample) const
{
    QwtColumnRect barRect;

    if (orientation() == Qt::Horizontal) {
        const double barHeight = sampleWidth(yMap, canvasRect.height(), boundingInterval.width(), sample.y());

        const double x1 = xMap.transform(baseline());
        const double x2 = xMap.transform(sample.y());

        const double y  = yMap.transform(sample.x());
        const double y1 = y - 0.5 * barHeight;
        const double y2 = y + 0.5 * barHeight;

        barRect.direction = (x1 < x2) ? QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

        barRect.hInterval = QwtInterval(x1, x2).normalized();
        barRect.vInterval = QwtInterval(y1, y2);
    } else {
        const double barWidth = sampleWidth(xMap, canvasRect.width(), boundingInterval.width(), sample.y());

        const double x  = xMap.transform(sample.x());
        const double x1 = x - 0.5 * barWidth;
        const double x2 = x + 0.5 * barWidth;

        const double y1 = yMap.transform(baseline());
        const double y2 = yMap.transform(sample.y());

        barRect.direction = (y1 < y2) ? QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

        barRect.hInterval = QwtInterval(x1, x2);
        barRect.vInterval = QwtInterval(y1, y2).normalized();
    }

    return barRect;
}

/*!
   Draw a sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rect of the canvas
   \param boundingInterval Bounding interval of sample values
   \param index Index of the sample
   \param sample Value of the sample

   \sa drawSeries()
 */
void QwtPlotBarChart::drawSample(QPainter* painter,
                                 const QwtScaleMap& xMap,
                                 const QwtScaleMap& yMap,
                                 const QRectF& canvasRect,
                                 const QwtInterval& boundingInterval,
                                 int index,
                                 const QPointF& sample) const
{
    const QwtColumnRect barRect = columnRect(xMap, yMap, canvasRect, boundingInterval, sample);

    drawBar(painter, index, sample, barRect);
}

/*!
   Draw a bar

   \param painter Painter
   \param sampleIndex Index of the sample represented by the bar
   \param sample Value of the sample
   \param rect Bounding rectangle of the bar
 */
void QwtPlotBarChart::drawBar(QPainter* painter, int sampleIndex, const QPointF& sample, const QwtColumnRect& rect) const
{
    const QwtColumnSymbol* specialSym = specialSymbol(sampleIndex, sample);

    const QwtColumnSymbol* sym = specialSym;
    if (sym == nullptr)
        sym = m_data->symbol;

    if (sym) {
        sym->draw(painter, rect);
    } else {
        // we build a temporary default symbol
        QwtColumnSymbol columnSymbol(QwtColumnSymbol::Box);
        columnSymbol.setLineWidth(1);
        columnSymbol.setFrameStyle(QwtColumnSymbol::Plain);
        columnSymbol.draw(painter, rect);
    }

    delete specialSym;
}

/**
 * \if ENGLISH
 * @brief Get a special symbol for a specific sample
 * @details Needs to be overloaded to return a non default symbol for a specific sample.
 * @param[in] sampleIndex Index of the sample represented by the bar
 * @param[in] sample Value of the sample
 * @return nullptr, indicating to use the default symbol
 * \endif
 * 
 * \if CHINESE
 * @brief 获取特定样本的特殊符号
 * @details 需要重载以返回特定样本的非默认符号。
 * @param[in] sampleIndex 由条形表示的样本索引
 * @param[in] sample 样本的值
 * @return nullptr，表示使用默认符号
 * \endif
 */
QwtColumnSymbol* QwtPlotBarChart::specialSymbol(int sampleIndex, const QPointF& sample) const
{
    Q_UNUSED(sampleIndex);
    Q_UNUSED(sample);

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Return the title of a bar
 * @details In LegendBarTitles mode the title is displayed on
 *          the legend entry corresponding to a bar.
 *          The default implementation is a dummy, that is intended
 *          to be overloaded.
 * @param[in] sampleIndex Index of the bar
 * @return An empty text
 * @sa LegendBarTitles
 * \endif
 * 
 * \if CHINESE
 * @brief 返回条形的标题
 * @details 在 LegendBarTitles 模式下，标题显示在与条形对应的图例条目上。
 *          默认实现是一个空文本，旨在被重载。
 * @param[in] sampleIndex 条形的索引
 * @return 空文本
 * @sa LegendBarTitles
 * \endif
 */
QwtText QwtPlotBarChart::barTitle(int sampleIndex) const
{
    Q_UNUSED(sampleIndex);
    return QwtText();
}

/*!
   \brief Return all information, that is needed to represent
          the item on the legend

   In case of LegendBarTitles an entry for each bar is returned,
   otherwise the chart is represented like any other plot item
   from its title() and the legendIcon().

   \return Information, that is needed to represent the item on the legend
   \sa title(), setLegendMode(), barTitle(), QwtLegend, QwtPlotLegendItem
 */
QList< QwtLegendData > QwtPlotBarChart::legendData() const
{
    QList< QwtLegendData > list;

    if (m_data->legendMode == LegendBarTitles) {
        const size_t numSamples = dataSize();
        list.reserve(numSamples);

        for (size_t i = 0; i < numSamples; i++) {
            QwtLegendData data;

            data.setValue(QwtLegendData::TitleRole, QVariant::fromValue(barTitle(i)));

            if (!legendIconSize().isEmpty()) {
                data.setValue(QwtLegendData::IconRole, QVariant::fromValue(legendIcon(i, legendIconSize())));
            }

            list += data;
        }
    } else {
        return QwtPlotAbstractBarChart::legendData();
    }

    return list;
}

/*!
   \return Icon representing a bar or the chart on the legend

   When the legendMode() is LegendBarTitles the icon shows
   the bar corresponding to index - otherwise the bar
   displays the default symbol.

   \param index Index of the legend entry
   \param size Icon size

   \sa setLegendMode(), drawBar(),
       QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 */
QwtGraphic QwtPlotBarChart::legendIcon(int index, const QSizeF& size) const
{
    QwtColumnRect column;
    column.hInterval = QwtInterval(0.0, size.width() - 1.0);
    column.vInterval = QwtInterval(0.0, size.height() - 1.0);

    QwtGraphic icon;
    icon.setDefaultSize(size);
    icon.setRenderHint(QwtGraphic::RenderPensUnscaled, true);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPlotItem::RenderAntialiased));

    int barIndex = -1;
    if (m_data->legendMode == QwtPlotBarChart::LegendBarTitles)
        barIndex = index;

    drawBar(&painter, barIndex, QPointF(), column);

    return icon;
}
