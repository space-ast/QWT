/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *****************************************************************************/

#include "qwt_plot_boxchart.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_symbol.h"
#include "qwt_graphic.h"
#include "qwt_math.h"
#include "qwt_text.h"

#include <qpainter.h>
#include <qrandom.h>

class QwtPlotBoxChart::PrivateData
{
public:
    PrivateData()
        : boxStyle(Rect)
        , whiskerStyle(StandardWhisker)
        , orientation(Qt::Vertical)
        , boxExtent(0.6)
        , minBoxWidth(2.0)
        , maxBoxWidth(-1.0)
        , medianVisible(true)
        , meanVisible(false)
        , outlierJitter(0.0)
        , paintAttributes(ClipBoxes | ClipOutliers)
    {
        pen = QPen(Qt::black, 1.0);
        brush = QBrush(Qt::NoBrush);
        medianPen = QPen(Qt::black, 2.0);
        outlierSymbol = nullptr;
        meanSymbol = nullptr;
    }

    BoxStyle boxStyle;
    WhiskerStyle whiskerStyle;
    Qt::Orientation orientation;
    double boxExtent;
    double minBoxWidth;
    double maxBoxWidth;
    QPen pen;
    QPen medianPen;
    QBrush brush;
    const QwtSymbol* outlierSymbol;
    const QwtSymbol* meanSymbol;
    bool medianVisible;
    bool meanVisible;
    double outlierJitter;
    PaintAttributes paintAttributes;
};

QwtPlotBoxChart::QwtPlotBoxChart(const QString& title)
    : QwtPlotSeriesItem(QwtText(title))
    , m_outlierData(nullptr)
{
    init();
}

QwtPlotBoxChart::QwtPlotBoxChart(const QwtText& title)
    : QwtPlotSeriesItem(title)
    , m_outlierData(nullptr)
{
    init();
}

QwtPlotBoxChart::~QwtPlotBoxChart()
{
    delete m_data;
    delete m_outlierData;
}

void QwtPlotBoxChart::init()
{
    m_data = new PrivateData();

    setItemAttribute(QwtPlotItem::Legend, true);
    setItemAttribute(QwtPlotItem::AutoScale, true);

    setData(new QwtBoxChartData());
    setZ(20.0);
}

int QwtPlotBoxChart::rtti() const
{
    return Rtti_PlotBoxChart;
}

void QwtPlotBoxChart::setPaintAttribute(PaintAttribute attr, bool on)
{
    if (on)
        m_data->paintAttributes |= attr;
    else
        m_data->paintAttributes &= ~attr;
}

bool QwtPlotBoxChart::testPaintAttribute(PaintAttribute attr) const
{
    return (m_data->paintAttributes & attr);
}

void QwtPlotBoxChart::setBoxStyle(BoxStyle style)
{
    if (m_data->boxStyle != style)
    {
        m_data->boxStyle = style;
        legendChanged();
        itemChanged();
    }
}

QwtPlotBoxChart::BoxStyle QwtPlotBoxChart::boxStyle() const
{
    return m_data->boxStyle;
}

void QwtPlotBoxChart::setWhiskerStyle(WhiskerStyle style)
{
    if (m_data->whiskerStyle != style)
    {
        m_data->whiskerStyle = style;
        legendChanged();
        itemChanged();
    }
}

QwtPlotBoxChart::WhiskerStyle QwtPlotBoxChart::whiskerStyle() const
{
    return m_data->whiskerStyle;
}

void QwtPlotBoxChart::setOrientation(Qt::Orientation orient)
{
    if (m_data->orientation != orient)
    {
        m_data->orientation = orient;
        legendChanged();
        itemChanged();
    }
}

Qt::Orientation QwtPlotBoxChart::orientation() const
{
    return m_data->orientation;
}

void QwtPlotBoxChart::setBoxExtent(double extent)
{
    extent = qwtMaxF(0.0, extent);
    if (m_data->boxExtent != extent)
    {
        m_data->boxExtent = extent;
        legendChanged();
        itemChanged();
    }
}

double QwtPlotBoxChart::boxExtent() const
{
    return m_data->boxExtent;
}

void QwtPlotBoxChart::setMinBoxWidth(double pixels)
{
    pixels = qwtMaxF(0.0, pixels);
    if (m_data->minBoxWidth != pixels)
    {
        m_data->minBoxWidth = pixels;
        legendChanged();
        itemChanged();
    }
}

double QwtPlotBoxChart::minBoxWidth() const
{
    return m_data->minBoxWidth;
}

void QwtPlotBoxChart::setMaxBoxWidth(double pixels)
{
    if (m_data->maxBoxWidth != pixels)
    {
        m_data->maxBoxWidth = pixels;
        legendChanged();
        itemChanged();
    }
}

double QwtPlotBoxChart::maxBoxWidth() const
{
    return m_data->maxBoxWidth;
}

void QwtPlotBoxChart::setPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setPen(QPen(color, width, style));
}

void QwtPlotBoxChart::setPen(const QPen& pen)
{
    if (m_data->pen != pen)
    {
        m_data->pen = pen;
        legendChanged();
        itemChanged();
    }
}

const QPen& QwtPlotBoxChart::pen() const
{
    return m_data->pen;
}

void QwtPlotBoxChart::setBrush(const QBrush& brush)
{
    if (m_data->brush != brush)
    {
        m_data->brush = brush;
        legendChanged();
        itemChanged();
    }
}

const QBrush& QwtPlotBoxChart::brush() const
{
    return m_data->brush;
}

void QwtPlotBoxChart::setMedianPen(const QPen& pen)
{
    if (m_data->medianPen != pen)
    {
        m_data->medianPen = pen;
        legendChanged();
        itemChanged();
    }
}

QPen QwtPlotBoxChart::medianPen() const
{
    return m_data->medianPen;
}

void QwtPlotBoxChart::setOutlierSymbol(const QwtSymbol* symbol)
{
    if (m_data->outlierSymbol != symbol)
    {
        delete m_data->outlierSymbol;
        m_data->outlierSymbol = symbol;
        legendChanged();
        itemChanged();
    }
}

const QwtSymbol* QwtPlotBoxChart::outlierSymbol() const
{
    return m_data->outlierSymbol;
}

void QwtPlotBoxChart::setMeanSymbol(const QwtSymbol* symbol)
{
    if (m_data->meanSymbol != symbol)
    {
        delete m_data->meanSymbol;
        m_data->meanSymbol = symbol;
        legendChanged();
        itemChanged();
    }
}

const QwtSymbol* QwtPlotBoxChart::meanSymbol() const
{
    return m_data->meanSymbol;
}

void QwtPlotBoxChart::setMedianVisible(bool visible)
{
    if (m_data->medianVisible != visible)
    {
        m_data->medianVisible = visible;
        itemChanged();
    }
}

bool QwtPlotBoxChart::isMedianVisible() const
{
    return m_data->medianVisible;
}

void QwtPlotBoxChart::setMeanVisible(bool visible)
{
    if (m_data->meanVisible != visible)
    {
        m_data->meanVisible = visible;
        itemChanged();
    }
}

bool QwtPlotBoxChart::isMeanVisible() const
{
    return m_data->meanVisible;
}

void QwtPlotBoxChart::setOutlierJitter(double jitterWidth)
{
    m_data->outlierJitter = qwtMaxF(0.0, jitterWidth);
}

double QwtPlotBoxChart::outlierJitter() const
{
    return m_data->outlierJitter;
}

void QwtPlotBoxChart::setSamples(const QVector<QwtBoxSample>& samples)
{
    setData(new QwtBoxChartData(samples));
}

void QwtPlotBoxChart::setSamples(QwtSeriesData<QwtBoxSample>* data)
{
    setData(data);
}

void QwtPlotBoxChart::setOutliers(const QVector<QwtBoxOutlierSample>& samples)
{
    delete m_outlierData;
    m_outlierData = new QwtBoxOutlierChartData(samples);
    itemChanged();
}

void QwtPlotBoxChart::setOutliers(QwtSeriesData<QwtBoxOutlierSample>* data)
{
    delete m_outlierData;
    m_outlierData = data;
    itemChanged();
}

const QwtSeriesData<QwtBoxOutlierSample>* QwtPlotBoxChart::outlierData() const
{
    return m_outlierData;
}

QRectF QwtPlotBoxChart::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();

    if (rect.isValid())
    {
        const double padding = m_data->boxExtent * 0.5;
        if (m_data->orientation == Qt::Vertical)
        {
            rect.setLeft(rect.left() - padding);
            rect.setRight(rect.right() + padding);
        }
        else
        {
            rect.setTop(rect.top() - padding);
            rect.setBottom(rect.bottom() + padding);
        }
    }

    if (m_outlierData && m_outlierData->size() > 0)
    {
        const QRectF outlierRect = m_outlierData->boundingRect();
        if (outlierRect.isValid())
        {
            rect = rect.united(outlierRect);
        }
    }

    return rect;
}

double QwtPlotBoxChart::scaledBoxWidth(
    const QwtScaleMap& posMap,
    const QwtScaleMap& valueMap,
    const QRectF& canvasRect) const
{
    Q_UNUSED(valueMap);
    Q_UNUSED(canvasRect);

    if (m_data->maxBoxWidth > 0.0 && m_data->minBoxWidth >= m_data->maxBoxWidth)
        return m_data->minBoxWidth;

    const double pos = posMap.transform(posMap.s1() + m_data->boxExtent);
    double width = qAbs(pos - posMap.p1());

    width = qwtMaxF(width, m_data->minBoxWidth);
    if (m_data->maxBoxWidth > 0.0)
        width = qwtMinF(width, m_data->maxBoxWidth);

    return width;
}

void QwtPlotBoxChart::drawSeries(QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    if (from < 0)
        from = 0;

    if (from > to || dataSize() == 0)
        return;

    painter->save();

    const Qt::Orientation orient = m_data->orientation;
    const QwtScaleMap* posMap = (orient == Qt::Vertical) ? &xMap : &yMap;
    const QwtScaleMap* valueMap = (orient == Qt::Vertical) ? &yMap : &xMap;

    const double boxWidth = scaledBoxWidth(xMap, yMap, canvasRect);
    const bool doAlign = QwtPainter::roundingAlignment(painter);

    painter->setPen(m_data->pen);

    for (int i = from; i <= to; i++)
    {
        const QwtBoxSample& sample = this->sample(i);

        double posPixel = posMap->transform(sample.position);
        if (doAlign)
            posPixel = qRound(posPixel);

        if (m_data->boxStyle != NoBox)
        {
            painter->setBrush(m_data->brush);
            drawBox(painter, sample, orient, boxWidth, posPixel, *valueMap);
        }

        if (m_data->whiskerStyle != NoWhiskers)
        {
            drawWhiskers(painter, sample, orient, boxWidth, posPixel, *valueMap);
        }

        if (m_data->medianVisible)
        {
            drawMedian(painter, sample, orient, boxWidth, posPixel, *valueMap);
        }

        if (m_data->meanVisible && m_data->meanSymbol)
        {
            double medianPixel = valueMap->transform(sample.median);
            if (doAlign)
                medianPixel = qRound(medianPixel);

            m_data->meanSymbol->drawSymbol(painter,
                (orient == Qt::Vertical) ? QPointF(posPixel, medianPixel)
                                         : QPointF(medianPixel, posPixel));
        }
    }

    if (m_outlierData && m_outlierData->size() > 0)
    {
        drawOutliers(painter, *posMap, *valueMap, canvasRect, from, to);
    }

    painter->restore();
}

void QwtPlotBoxChart::drawBox(QPainter* painter, const QwtBoxSample& sample,
    Qt::Orientation orient, double boxWidth, double posPixel,
    const QwtScaleMap& valueMap) const
{
    const bool doAlign = QwtPainter::roundingAlignment(painter);

    const double q1Pixel = valueMap.transform(sample.q1);
    const double q3Pixel = valueMap.transform(sample.q3);
    const double medianPixel = valueMap.transform(sample.median);

    if (doAlign)
    {
        const double aligned = qRound(posPixel);
        posPixel = aligned;
    }

    const double halfWidth = boxWidth * 0.5;

    switch (m_data->boxStyle)
    {
        case Rect:
        {
            if (orient == Qt::Vertical)
            {
                QRectF rect(posPixel - halfWidth, q3Pixel,
                           boxWidth, q1Pixel - q3Pixel);
                QwtPainter::drawRect(painter, rect);
            }
            else
            {
                QRectF rect(q1Pixel, posPixel - halfWidth,
                           q3Pixel - q1Pixel, boxWidth);
                QwtPainter::drawRect(painter, rect);
            }
            break;
        }

        case Diamond:
        {
            QPolygonF poly(4);
            if (orient == Qt::Vertical)
            {
                poly[0] = QPointF(posPixel, q3Pixel);
                poly[1] = QPointF(posPixel + halfWidth, medianPixel);
                poly[2] = QPointF(posPixel, q1Pixel);
                poly[3] = QPointF(posPixel - halfWidth, medianPixel);
            }
            else
            {
                poly[0] = QPointF(q3Pixel, posPixel);
                poly[1] = QPointF(medianPixel, posPixel + halfWidth);
                poly[2] = QPointF(q1Pixel, posPixel);
                poly[3] = QPointF(medianPixel, posPixel - halfWidth);
            }
            QwtPainter::drawPolygon(painter, poly);
            break;
        }

        case Notch:
        {
            const double notchWidth = halfWidth * 0.25;
            const double notchOffset = (q3Pixel - q1Pixel) * 0.1;

            QPolygonF poly(10);
            if (orient == Qt::Vertical)
            {
                poly[0] = QPointF(posPixel - halfWidth, q3Pixel);
                poly[1] = QPointF(posPixel - halfWidth, medianPixel - notchOffset);
                poly[2] = QPointF(posPixel - notchWidth, medianPixel);
                poly[3] = QPointF(posPixel - halfWidth, medianPixel + notchOffset);
                poly[4] = QPointF(posPixel - halfWidth, q1Pixel);
                poly[5] = QPointF(posPixel + halfWidth, q1Pixel);
                poly[6] = QPointF(posPixel + halfWidth, medianPixel + notchOffset);
                poly[7] = QPointF(posPixel + notchWidth, medianPixel);
                poly[8] = QPointF(posPixel + halfWidth, medianPixel - notchOffset);
                poly[9] = QPointF(posPixel + halfWidth, q3Pixel);
            }
            else
            {
                poly[0] = QPointF(q1Pixel, posPixel - halfWidth);
                poly[1] = QPointF(medianPixel - notchOffset, posPixel - halfWidth);
                poly[2] = QPointF(medianPixel, posPixel - notchWidth);
                poly[3] = QPointF(medianPixel + notchOffset, posPixel - halfWidth);
                poly[4] = QPointF(q3Pixel, posPixel - halfWidth);
                poly[5] = QPointF(q3Pixel, posPixel + halfWidth);
                poly[6] = QPointF(medianPixel + notchOffset, posPixel + halfWidth);
                poly[7] = QPointF(medianPixel, posPixel + notchWidth);
                poly[8] = QPointF(medianPixel - notchOffset, posPixel + halfWidth);
                poly[9] = QPointF(q1Pixel, posPixel + halfWidth);
            }
            QwtPainter::drawPolygon(painter, poly);
            break;
        }

        default:
            break;
    }
}

void QwtPlotBoxChart::drawWhiskers(QPainter* painter, const QwtBoxSample& sample,
    Qt::Orientation orient, double boxWidth, double posPixel,
    const QwtScaleMap& valueMap) const
{
    const bool doAlign = QwtPainter::roundingAlignment(painter);

    const double whiskerLowerPixel = valueMap.transform(sample.whiskerLower);
    const double whiskerUpperPixel = valueMap.transform(sample.whiskerUpper);
    const double q1Pixel = valueMap.transform(sample.q1);
    const double q3Pixel = valueMap.transform(sample.q3);

    if (doAlign)
    {
    }

    const double capWidth = boxWidth * 0.3;

    QPen whiskerPen = m_data->pen;
    whiskerPen.setCapStyle(Qt::FlatCap);
    painter->setPen(whiskerPen);

    switch (m_data->whiskerStyle)
    {
        case StandardWhisker:
        {
            if (orient == Qt::Vertical)
            {
                QwtPainter::drawLine(painter, posPixel, whiskerLowerPixel,
                                     posPixel, q1Pixel);
                QwtPainter::drawLine(painter, posPixel - capWidth * 0.5, whiskerLowerPixel,
                                     posPixel + capWidth * 0.5, whiskerLowerPixel);

                QwtPainter::drawLine(painter, posPixel, q3Pixel,
                                     posPixel, whiskerUpperPixel);
                QwtPainter::drawLine(painter, posPixel - capWidth * 0.5, whiskerUpperPixel,
                                     posPixel + capWidth * 0.5, whiskerUpperPixel);
            }
            else
            {
                QwtPainter::drawLine(painter, whiskerLowerPixel, posPixel,
                                     q1Pixel, posPixel);
                QwtPainter::drawLine(painter, whiskerLowerPixel, posPixel - capWidth * 0.5,
                                     whiskerLowerPixel, posPixel + capWidth * 0.5);

                QwtPainter::drawLine(painter, q3Pixel, posPixel,
                                     whiskerUpperPixel, posPixel);
                QwtPainter::drawLine(painter, whiskerUpperPixel, posPixel - capWidth * 0.5,
                                     whiskerUpperPixel, posPixel + capWidth * 0.5);
            }
            break;
        }

        case MinMaxLine:
        {
            if (orient == Qt::Vertical)
            {
                QwtPainter::drawLine(painter, posPixel, whiskerLowerPixel,
                                     posPixel, whiskerUpperPixel);
            }
            else
            {
                QwtPainter::drawLine(painter, whiskerLowerPixel, posPixel,
                                     whiskerUpperPixel, posPixel);
            }
            break;
        }

        default:
            break;
    }
}

void QwtPlotBoxChart::drawMedian(QPainter* painter, const QwtBoxSample& sample,
    Qt::Orientation orient, double boxWidth, double posPixel,
    const QwtScaleMap& valueMap) const
{
    const double medianPixel = valueMap.transform(sample.median);
    const bool doAlign = QwtPainter::roundingAlignment(painter);

    if (doAlign)
    {
    }

    const double halfWidth = boxWidth * 0.5;

    QPen medPen = m_data->medianPen;
    medPen.setCapStyle(Qt::FlatCap);
    painter->setPen(medPen);

    double lineHalfWidth = halfWidth;
    if (m_data->boxStyle == Diamond)
        lineHalfWidth *= 0.5;
    else if (m_data->boxStyle == Notch)
        lineHalfWidth *= 0.8;

    if (orient == Qt::Vertical)
    {
        QwtPainter::drawLine(painter, posPixel - lineHalfWidth, medianPixel,
                             posPixel + lineHalfWidth, medianPixel);
    }
    else
    {
        QwtPainter::drawLine(painter, medianPixel, posPixel - lineHalfWidth,
                             medianPixel, posPixel + lineHalfWidth);
    }
}

void QwtPlotBoxChart::drawOutliers(QPainter* painter,
    const QwtScaleMap& posMap, const QwtScaleMap& valueMap,
    const QRectF& canvasRect, int from, int to) const
{
    if (!m_outlierData || m_outlierData->size() == 0)
        return;

    const QwtSymbol* symbol = m_data->outlierSymbol;
    if (!symbol)
    {
        static QwtSymbol defaultSymbol(QwtSymbol::XCross, QBrush(), QPen(Qt::black), QSize(8, 8));
        symbol = &defaultSymbol;
    }

    const Qt::Orientation orient = m_data->orientation;
    const bool doAlign = QwtPainter::roundingAlignment(painter);
    const double jitter = m_data->outlierJitter;

    QRandomGenerator* rng = nullptr;
    if (jitter > 0)
        rng = QRandomGenerator::global();

    for (size_t i = 0; i < m_outlierData->size(); ++i)
    {
        const QwtBoxOutlierSample& outlierSample = m_outlierData->sample(i);

        double basePosPixel = posMap.transform(outlierSample.boxPosition);
        if (doAlign)
            basePosPixel = qRound(basePosPixel);

        for (int j = 0; j < outlierSample.count(); ++j)
        {
            double valuePixel = valueMap.transform(outlierSample.values[j]);
            if (doAlign)
                valuePixel = qRound(valuePixel);

            double posPixel = basePosPixel;
            if (jitter > 0 && rng)
            {
                const double offset = rng->bounded(jitter) - jitter * 0.5;
                posPixel += offset;
            }

            QPointF point = (orient == Qt::Vertical)
                ? QPointF(posPixel, valuePixel)
                : QPointF(valuePixel, posPixel);

            if (m_data->paintAttributes & ClipOutliers)
            {
                if (!canvasRect.contains(point))
                    continue;
            }

            symbol->drawSymbol(painter, point);
        }
    }
}

void QwtPlotBoxChart::drawOutlierSymbol(QPainter* painter, double posPixel, double valuePixel,
    const QwtSymbol& symbol) const
{
    const Qt::Orientation orient = m_data->orientation;
    QPointF point = (orient == Qt::Vertical)
        ? QPointF(posPixel, valuePixel)
        : QPointF(valuePixel, posPixel);
    symbol.drawSymbol(painter, point);
}

QwtGraphic QwtPlotBoxChart::legendIcon(int index, const QSizeF& size) const
{
    Q_UNUSED(index);

    QwtGraphic graphic;
    graphic.setDefaultSize(size);

    QPainter painter(&graphic);

    const QRectF rect(0, 0, size.width(), size.height());

    const double centerX = rect.center().x();
    const double centerY = rect.center().y();
    const double boxHeight = rect.height() * 0.4;
    const double boxWidth = rect.width() * 0.3;

    painter.setPen(m_data->pen);
    painter.setBrush(m_data->brush);

    if (m_data->boxStyle != NoBox)
    {
        QRectF boxRect(centerX - boxWidth * 0.5, centerY - boxHeight * 0.5,
                       boxWidth, boxHeight);
        painter.drawRect(boxRect);
    }

    if (m_data->whiskerStyle != NoWhiskers)
    {
        const double whiskerLen = rect.height() * 0.2;
        painter.drawLine(centerX, centerY - boxHeight * 0.5,
                         centerX, centerY - boxHeight * 0.5 - whiskerLen);
        painter.drawLine(centerX - boxWidth * 0.15, centerY - boxHeight * 0.5 - whiskerLen,
                         centerX + boxWidth * 0.15, centerY - boxHeight * 0.5 - whiskerLen);
        painter.drawLine(centerX, centerY + boxHeight * 0.5,
                         centerX, centerY + boxHeight * 0.5 + whiskerLen);
        painter.drawLine(centerX - boxWidth * 0.15, centerY + boxHeight * 0.5 + whiskerLen,
                         centerX + boxWidth * 0.15, centerY + boxHeight * 0.5 + whiskerLen);
    }

    if (m_data->medianVisible)
    {
        painter.setPen(m_data->medianPen);
        painter.drawLine(centerX - boxWidth * 0.5, centerY,
                         centerX + boxWidth * 0.5, centerY);
    }

    painter.end();

    return graphic;
}