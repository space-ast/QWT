/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_polar_layout.h"
#include "qwt_painter.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_text_label.h"
#include "qwt_round_scale_draw.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_math.h"
#include <qpointer.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qevent.h>

namespace
{
class QwtPolarPlotScaleData
{
public:
    QwtPolarPlotScaleData() : isValid(false), scaleEngine(nullptr)
    {
    }

    ~QwtPolarPlotScaleData()
    {
        delete scaleEngine;
    }

    bool doAutoScale;

    double minValue;
    double maxValue;
    double stepSize;

    int maxMajor;
    int maxMinor;

    bool isValid;

    QwtScaleDiv scaleDiv;
    QwtScaleEngine* scaleEngine;
};
}

class QwtPolarPlot::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPolarPlot)
public:
    PrivateData(QwtPolarPlot* p) : q_ptr(p)
    {
    }

    QBrush canvasBrush;

    bool autoReplot;

    QwtPointPolar zoomPos;
    double zoomFactor;

    QwtPolarPlotScaleData scaleData[ QwtPolar::ScaleCount ];
    QPointer< QwtTextLabel > titleLabel;
    QPointer< QwtPolarCanvas > canvas;
    QPointer< QwtAbstractLegend > legend;
    double azimuthOrigin;

    QwtPolarLayout* layout;
};

/**
 * @brief Constructor
 * @param parent Parent widget
 */
QwtPolarPlot::QwtPolarPlot(QWidget* parent) : QFrame(parent)
{
    initPlot(QwtText());
}

/**
 * @brief Constructor with title
 * @param title Title text
 * @param parent Parent widget
 */
QwtPolarPlot::QwtPolarPlot(const QwtText& title, QWidget* parent) : QFrame(parent)
{
    initPlot(title);
}

/**
 * @brief Destructor
 */
QwtPolarPlot::~QwtPolarPlot()
{
    QWT_D(d);

    detachItems(QwtPolarItem::Rtti_PolarItem, autoDelete());

    delete d->layout;
}

/**
 * @brief Change the plot's title
 * @param title New title
 */
void QwtPolarPlot::setTitle(const QString& title)
{
    QWT_D(d);

    if (title != d->titleLabel->text().text()) {
        d->titleLabel->setText(title);
        if (!title.isEmpty())
            d->titleLabel->show();
        else
            d->titleLabel->hide();
    }
}

/**
 * @brief Change the plot's title
 * @param title New title
 */
void QwtPolarPlot::setTitle(const QwtText& title)
{
    QWT_D(d);

    if (title != d->titleLabel->text()) {
        d->titleLabel->setText(title);
        if (!title.isEmpty())
            d->titleLabel->show();
        else
            d->titleLabel->hide();
    }
}

/**
 * @brief Get the plot's title
 * @return Title text
 */
QwtText QwtPolarPlot::title() const
{
    QWT_DC(d);
    return d->titleLabel->text();
}

/**
 * @brief Get the title label widget
 * @return Title label widget
 */
QwtTextLabel* QwtPolarPlot::titleLabel()
{
    QWT_D(d);
    return d->titleLabel;
}

/**
 * @brief Get the title label widget (const version)
 * @return Title label widget
 */
const QwtTextLabel* QwtPolarPlot::titleLabel() const
{
    QWT_DC(d);
    return d->titleLabel;
}

/**
 * @brief Insert a legend
 * @details If the position legend is \c QwtPolarPlot::LeftLegend or \c QwtPolarPlot::RightLegend
 *          the legend will be organized in one column from top to down.
 *          Otherwise the legend items will be placed in a table
 *          with a best fit number of columns from left to right.
 *          If pos != QwtPolarPlot::ExternalLegend the plot widget will become
 *          parent of the legend. It will be deleted when the plot is deleted,
 *          or another legend is set with insertLegend().
 * @param legend Legend widget
 * @param pos The legend's position. For top/left position the number
 *            of columns will be limited to 1, otherwise it will be set to unlimited.
 * @param ratio Ratio between legend and the bounding rect of title, canvas and axes.
 *               The legend will be shrunk if it would need more space than the given ratio.
 *               The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0 it will be reset to the default ratio.
 *               The default vertical/horizontal ratio is 0.33/0.5.
 * @sa legend(), QwtPolarLayout::legendPosition(), QwtPolarLayout::setLegendPosition()
 */
void QwtPolarPlot::insertLegend(QwtAbstractLegend* legend, QwtPolarPlot::LegendPosition pos, double ratio)
{
    QWT_D(d);

    d->layout->setLegendPosition(pos, ratio);

    if (legend != d->legend) {
        if (d->legend && d->legend->parent() == this)
            delete d->legend;

        d->legend = legend;

        if (d->legend) {
            QwtLegend* lgd = qobject_cast< QwtLegend* >(legend);
            if (lgd) {
                connect(this, &QwtPolarPlot::legendDataChanged, lgd, &QwtLegend::updateLegend);
            }

            if (d->legend->parent() != this)
                d->legend->setParent(this);

            updateLegend();

            if (lgd) {
                switch (d->layout->legendPosition()) {
                case LeftLegend:
                case RightLegend: {
                    if (lgd->maxColumns() == 0)
                        lgd->setMaxColumns(1);  // 1 column: align vertical
                    break;
                }
                case TopLegend:
                case BottomLegend: {
                    lgd->setMaxColumns(0);  // unlimited
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    updateLayout();
}

/**
 * @brief Emit legendDataChanged() for all plot items
 * @sa QwtPlotItem::legendData(), legendDataChanged()
 */
void QwtPolarPlot::updateLegend()
{
    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        updateLegend(*it);
    }
}

/**
 * @brief Emit legendDataChanged() for a plot item
 * @param plotItem Plot item
 * @sa QwtPlotItem::legendData(), legendDataChanged()
 */
void QwtPolarPlot::updateLegend(const QwtPolarItem* plotItem)
{
    if (plotItem == nullptr)
        return;

    QList< QwtLegendData > legendData;

    if (plotItem->testItemAttribute(QwtPolarItem::Legend))
        legendData = plotItem->legendData();

    const QVariant itemInfo = itemToInfo(const_cast< QwtPolarItem* >(plotItem));
    Q_EMIT legendDataChanged(itemInfo, legendData);
}

/**
 * @brief Get the plot's legend
 * @return Legend widget
 * @sa insertLegend()
 */
QwtAbstractLegend* QwtPolarPlot::legend()
{
    QWT_D(d);
    return d->legend;
}

/**
 * @brief Get the plot's legend (const version)
 * @return Legend widget
 * @sa insertLegend()
 */
const QwtAbstractLegend* QwtPolarPlot::legend() const
{
    QWT_DC(d);
    return d->legend;
}

/**
 * @brief Set the background of the plot area
 * @details The plot area is the circle around the pole. Its radius is defined by the radial scale.
 * @param brush Background brush
 * @sa plotBackground(), plotArea()
 */
void QwtPolarPlot::setPlotBackground(const QBrush& brush)
{
    QWT_D(d);

    if (brush != d->canvasBrush) {
        d->canvasBrush = brush;
        autoRefresh();
    }
}

/**
 * @brief Get the plot background brush
 * @return Background brush
 * @sa plotBackground(), plotArea()
 */
const QBrush& QwtPolarPlot::plotBackground() const
{
    QWT_DC(d);
    return d->canvasBrush;
}

/**
 * @brief Set the background color
 * @param c Background color
 * @sa backgroundColor()
 */
void QwtPolarPlot::setBackgroundColor(const QColor& c)
{
    QPalette p = palette();
    p.setColor(backgroundRole(), c);
    setPalette(p);

    setAutoFillBackground(true);
}

/**
 * @brief Get the background color
 * @return Background color
 * @sa setBackgroundColor()
 */
QColor QwtPolarPlot::backgroundColor() const
{
    return palette().color(backgroundRole());
}

/**
 * @brief Set or reset the autoReplot option
 * @details If the autoReplot option is set, the plot will be updated implicitly by manipulating member functions.
 *          Since this may be time-consuming, it is recommended to leave this option switched off and call replot()
 *          explicitly if necessary.
 *          The autoReplot option is set to false by default, which means that the user has to call replot()
 *          in order to make changes visible.
 * @param enable \c true or \c false. Defaults to \c true.
 * @sa replot()
 */
void QwtPolarPlot::setAutoReplot(bool enable)
{
    QWT_D(d);
    d->autoReplot = enable;
}

/**
 * @brief Check if autoReplot option is set
 * @return \c true if autoReplot option is set
 * @sa setAutoReplot()
 */
bool QwtPolarPlot::autoReplot() const
{
    QWT_DC(d);
    return d->autoReplot;
}

/**
 * @brief Enable autoscaling
 * @details This member function is used to switch back to autoscaling mode after a fixed scale has been set.
 *          Autoscaling calculates a useful scale division from the bounding interval of all plot items
 *          with the QwtPolarItem::AutoScale attribute.
 *          Autoscaling is only supported for the radial scale and enabled as default.
 * @param scaleId Scale index
 * @sa hasAutoScale(), setScale(), setScaleDiv(), QwtPolarItem::boundingInterval()
 */
void QwtPolarPlot::setAutoScale(int scaleId)
{
    QWT_D(d);

    if (scaleId != QwtPolar::ScaleRadius)
        return;

    QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];
    if (!scaleData.doAutoScale) {
        scaleData.doAutoScale = true;
        autoRefresh();
    }
}

/**
 * @brief Check if autoscaling is enabled
 * @param scaleId Scale index
 * @return \c true if autoscaling is enabled
 * @sa setAutoScale()
 */
bool QwtPolarPlot::hasAutoScale(int scaleId) const
{
    QWT_DC(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return false;

    return d->scaleData[ scaleId ].doAutoScale;
}

/**
 * @brief Set the maximum number of minor scale intervals for a specified scale
 * @param scaleId Scale index
 * @param maxMinor Maximum number of minor steps
 * @sa scaleMaxMajor()
 */
void QwtPolarPlot::setScaleMaxMinor(int scaleId, int maxMinor)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    maxMinor = qBound(0, maxMinor, 100);

    QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];

    if (maxMinor != scaleData.maxMinor) {
        scaleData.maxMinor = maxMinor;
        scaleData.isValid  = false;
        autoRefresh();
    }
}

/**
 * @brief Get the maximum number of minor ticks for a specified axis
 * @param scaleId Scale index
 * @return Maximum number of minor ticks
 * @sa setScaleMaxMinor()
 */
int QwtPolarPlot::scaleMaxMinor(int scaleId) const
{
    QWT_DC(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return 0;

    return d->scaleData[ scaleId ].maxMinor;
}

/**
 * @brief Set the maximum number of major scale intervals for a specified scale
 * @param scaleId Scale index
 * @param maxMajor Maximum number of major steps
 * @sa scaleMaxMajor()
 */
void QwtPolarPlot::setScaleMaxMajor(int scaleId, int maxMajor)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    maxMajor = qBound(1, maxMajor, 10000);

    QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];
    if (maxMajor != scaleData.maxMinor) {
        scaleData.maxMajor = maxMajor;
        scaleData.isValid  = false;
        autoRefresh();
    }
}

/**
 * @brief Get the maximum number of major ticks for a specified axis
 * @param scaleId Scale index
 * @return Maximum number of major ticks
 * @sa setScaleMaxMajor()
 */
int QwtPolarPlot::scaleMaxMajor(int scaleId) const
{
    QWT_DC(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return 0;

    return d->scaleData[ scaleId ].maxMajor;
}

/**
 * @brief Change the scale engine for an axis
 * @param scaleId Scale index
 * @param scaleEngine Scale engine
 * @sa axisScaleEngine()
 */
void QwtPolarPlot::setScaleEngine(int scaleId, QwtScaleEngine* scaleEngine)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];
    if (scaleEngine == nullptr || scaleEngine == scaleData.scaleEngine)
        return;

    delete scaleData.scaleEngine;
    scaleData.scaleEngine = scaleEngine;

    scaleData.isValid = false;

    autoRefresh();
}

/**
 * @brief Get scale engine for a specific scale
 * @param scaleId Scale index
 * @return Scale engine
 * @sa setScaleEngine()
 */
QwtScaleEngine* QwtPolarPlot::scaleEngine(int scaleId)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return d->scaleData[ scaleId ].scaleEngine;
}

/**
 * @brief Get scale engine for a specific scale (const version)
 * @param scaleId Scale index
 * @return Scale engine
 * @sa setScaleEngine()
 */
const QwtScaleEngine* QwtPolarPlot::scaleEngine(int scaleId) const
{
    QWT_DC(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return d->scaleData[ scaleId ].scaleEngine;
}

/**
 * @brief Disable autoscaling and specify a fixed scale for a selected scale
 * @param scaleId Scale index
 * @param min Minimum value of the scale
 * @param max Maximum value of the scale
 * @param stepSize Major step size. If <code>step == 0</code>, the step size is calculated automatically using the maxMajor setting.
 * @sa setScaleMaxMajor(), setAutoScale()
 */
void QwtPolarPlot::setScale(int scaleId, double min, double max, double stepSize)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];

    scaleData.isValid = false;

    scaleData.minValue    = min;
    scaleData.maxValue    = max;
    scaleData.stepSize    = stepSize;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/**
 * @brief Disable autoscaling and specify a fixed scale for a selected scale
 * @param scaleId Scale index
 * @param scaleDiv Scale division
 * @sa setScale(), setAutoScale()
 */
void QwtPolarPlot::setScaleDiv(int scaleId, const QwtScaleDiv& scaleDiv)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];

    scaleData.scaleDiv    = scaleDiv;
    scaleData.isValid     = true;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/**
 * @brief Return the scale division of a specified scale
 * @details scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound() are the current limits of the scale.
 * @param scaleId Scale index
 * @return Scale division
 * @sa QwtScaleDiv, setScaleDiv(), setScale()
 */
const QwtScaleDiv* QwtPolarPlot::scaleDiv(int scaleId) const
{
    QWT_DC(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return &d->scaleData[ scaleId ].scaleDiv;
}

/**
 * @brief Return the scale division of a specified scale
 * @details scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound() are the current limits of the scale.
 * @param scaleId Scale index
 * @return Scale division
 * @sa QwtScaleDiv, setScaleDiv(), setScale()
 */
QwtScaleDiv* QwtPolarPlot::scaleDiv(int scaleId)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return nullptr;

    return &d->scaleData[ scaleId ].scaleDiv;
}

/**
 * @brief Change the origin of the azimuth scale
 * @details The azimuth origin is the angle where the azimuth scale shows the value 0.0. The default origin is 0.0.
 * @param origin New origin
 * @sa azimuthOrigin()
 */
void QwtPolarPlot::setAzimuthOrigin(double origin)
{
    QWT_D(d);

    origin = ::fmod(origin, 2 * M_PI);
    if (origin != d->azimuthOrigin) {
        d->azimuthOrigin = origin;
        autoRefresh();
    }
}

/**
 * @brief Get the origin of the azimuth scale
 * @details The azimuth origin is the angle where the azimuth scale shows the value 0.0.
 * @return Origin of the azimuth scale
 * @sa setAzimuthOrigin()
 */
double QwtPolarPlot::azimuthOrigin() const
{
    QWT_DC(d);
    return d->azimuthOrigin;
}

/**
 * @brief Translate and in/decrease the zoom factor
 * @details In zoom mode the zoom position is in the center of the canvas.
 *          The radius of the circle depends on the size of the plot canvas, that is divided by the zoom factor.
 *          Thus a factor < 1.0 zooms in.
 *          Setting an invalid zoom position disables zooming.
 * @param zoomPos Center of the translation
 * @param zoomFactor Zoom factor
 * @sa unzoom(), zoomPos(), zoomFactor()
 */
void QwtPolarPlot::zoom(const QwtPointPolar& zoomPos, double zoomFactor)
{
    QWT_D(d);

    zoomFactor = qAbs(zoomFactor);
    if (zoomPos != d->zoomPos || zoomFactor != d->zoomFactor) {
        d->zoomPos    = zoomPos;
        d->zoomFactor = zoomFactor;
        updateLayout();
        autoRefresh();
    }
}

/**
 * @brief Unzoom the plot
 * @sa zoom()
 */
void QwtPolarPlot::unzoom()
{
    QWT_D(d);

    if (d->zoomFactor != 1.0 || d->zoomPos.isValid()) {
        d->zoomFactor = 1.0;
        d->zoomPos    = QwtPointPolar();
        autoRefresh();
    }
}

/**
 * @brief Get the zoom position
 * @return Zoom position
 * @sa zoom(), zoomFactor()
 */
QwtPointPolar QwtPolarPlot::zoomPos() const
{
    QWT_DC(d);
    return d->zoomPos;
}

/**
 * @brief Get the zoom factor
 * @return Zoom factor
 * @sa zoom(), zoomPos()
 */
double QwtPolarPlot::zoomFactor() const
{
    QWT_DC(d);
    return d->zoomFactor;
}

/**
 * @brief Build a scale map
 * @details The azimuth map translates between the scale values and angles from [0.0, 2 * PI[.
 *          The radial map translates scale values into the distance from the pole.
 *          The radial map is calculated from the current geometry of the canvas.
 * @param scaleId Scale index
 * @return Map for the scale on the canvas. With this map pixel coordinates can be translated to plot coordinates and vice versa.
 * @sa QwtScaleMap, transform(), invTransform()
 */
QwtScaleMap QwtPolarPlot::scaleMap(int scaleId) const
{
    const QRectF pr = plotRect();
    return scaleMap(scaleId, pr.width() / 2.0);
}

/**
 * @brief Build a scale map with specified radius
 * @details The azimuth map translates between the scale values and angles from [0.0, 2 * PI[.
 *          The radial map translates scale values into the distance from the pole.
 * @param scaleId Scale index
 * @param radius Radius of the plot area in pixels
 * @return Map for the scale on the canvas. With this map pixel coordinates can be translated to plot coordinates and vice versa.
 * @sa QwtScaleMap, transform(), invTransform()
 */
QwtScaleMap QwtPolarPlot::scaleMap(int scaleId, const double radius) const
{
    QWT_DC(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return QwtScaleMap();

    QwtScaleMap map;
    map.setTransformation(scaleEngine(scaleId)->transformation());

    const QwtScaleDiv* sd = scaleDiv(scaleId);
    map.setScaleInterval(sd->lowerBound(), sd->upperBound());

    if (scaleId == QwtPolar::Azimuth) {
        map.setPaintInterval(d->azimuthOrigin, d->azimuthOrigin + 2 * M_PI);
    } else {
        map.setPaintInterval(0.0, radius);
    }

    return map;
}

/**
 * @brief Qt event handler
 * @details Handles QEvent::LayoutRequest and QEvent::PolishRequest
 * @param e Qt Event
 * @return True when the event was processed
 */
bool QwtPolarPlot::event(QEvent* e)
{
    bool ok = QWidget::event(e);
    switch (e->type()) {
    case QEvent::LayoutRequest: {
        updateLayout();
        break;
    }
    case QEvent::PolishRequest: {
        updateLayout();
        replot();
        break;
    }
    default:;
    }
    return ok;
}

/**
 * @brief Resize and update internal layout
 * @param e Resize event
 */
void QwtPolarPlot::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);
    updateLayout();
}

void QwtPolarPlot::initPlot(const QwtText& title)
{
    QWT_PIMPL_CONSTRUCT_INIT();
    QWT_D(d);

    d->layout = new QwtPolarLayout;

    QwtText text(title);
    text.setRenderFlags(Qt::AlignCenter | Qt::TextWordWrap);

    d->titleLabel = new QwtTextLabel(text, this);
    d->titleLabel->setFont(QFont(fontInfo().family(), 14, QFont::Bold));
    if (!text.isEmpty())
        d->titleLabel->show();
    else
        d->titleLabel->hide();

    d->canvas = new QwtPolarCanvas(this);

    d->autoReplot  = false;
    d->canvasBrush = QBrush(Qt::white);

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++) {
        QwtPolarPlotScaleData& scaleData = d->scaleData[ scaleId ];

        if (scaleId == QwtPolar::Azimuth) {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 360.0;
            scaleData.stepSize = 30.0;
        } else {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 1000.0;
            scaleData.stepSize = 0.0;
        }

        scaleData.doAutoScale = true;

        scaleData.maxMinor = 5;
        scaleData.maxMajor = 8;

        scaleData.isValid = false;

        scaleData.scaleEngine = new QwtLinearScaleEngine;
    }
    d->zoomFactor    = 1.0;
    d->azimuthOrigin = 0.0;

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++)
        updateScale(scaleId);
}

/**
 * @brief Replots the plot if autoReplot() is \c true
 */
void QwtPolarPlot::autoRefresh()
{
    QWT_D(d);

    if (d->autoReplot)
        replot();
}

/**
 * @brief Rebuild the layout
 */
void QwtPolarPlot::updateLayout()
{
    QWT_D(d);

    d->layout->activate(this, contentsRect());

    // resize and show the visible widgets
    if (d->titleLabel) {
        if (!d->titleLabel->text().isEmpty()) {
            d->titleLabel->setGeometry(d->layout->titleRect().toRect());
            if (!d->titleLabel->isVisible())
                d->titleLabel->show();
        } else
            d->titleLabel->hide();
    }

    if (d->legend) {
        if (d->legend->isEmpty()) {
            d->legend->hide();
        } else {
            const QRectF legendRect = d->layout->legendRect();
            d->legend->setGeometry(legendRect.toRect());
            d->legend->show();
        }
    }

    d->canvas->setGeometry(d->layout->canvasRect().toRect());
    Q_EMIT layoutChanged();
}

/**
 * @brief Redraw the plot
 * @details If the autoReplot option is not set (which is the default) or if any curves are attached to raw data,
 *          the plot has to be refreshed explicitly in order to make changes visible.
 * @warning Calls canvas()->repaint, take care of infinite recursions
 * @sa setAutoReplot()
 */
void QwtPolarPlot::replot()
{
    QWT_D(d);

    bool doAutoReplot = autoReplot();
    setAutoReplot(false);

    for (int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++)
        updateScale(scaleId);

    d->canvas->invalidateBackingStore();
    d->canvas->repaint();

    setAutoReplot(doAutoReplot);
}

/**
 * @brief Get the plot's canvas
 * @return Canvas widget
 */
QwtPolarCanvas* QwtPolarPlot::canvas()
{
    QWT_D(d);
    return d->canvas;
}

/**
 * @brief Get the plot's canvas (const version)
 * @return Canvas widget
 */
const QwtPolarCanvas* QwtPolarPlot::canvas() const
{
    QWT_DC(d);
    return d->canvas;
}

/**
 * @brief Redraw the canvas
 * @param painter Painter used for drawing
 * @param canvasRect Contents rect of the canvas
 */
void QwtPolarPlot::drawCanvas(QPainter* painter, const QRectF& canvasRect) const
{
    QWT_DC(d);

    const QRectF cr = canvasRect;
    const QRectF pr = plotRect(cr);

    const double radius = pr.width() / 2.0;

    if (d->canvasBrush.style() != Qt::NoBrush) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(d->canvasBrush);

        if (qwtDistance(pr.center(), cr.topLeft()) < radius && qwtDistance(pr.center(), cr.topRight()) < radius
            && qwtDistance(pr.center(), cr.bottomRight()) < radius && qwtDistance(pr.center(), cr.bottomLeft()) < radius) {
            QwtPainter::drawRect(painter, cr);
        } else {
            painter->setRenderHint(QPainter::Antialiasing, true);
            QwtPainter::drawEllipse(painter, pr);
        }
        painter->restore();
    }

    drawItems(painter, scaleMap(QwtPolar::Azimuth, radius), scaleMap(QwtPolar::Radius, radius), pr.center(), radius, canvasRect);
}

/**
 * @brief Redraw the canvas items
 * @param painter Painter used for drawing
 * @param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param radialMap Maps radius values into painter coordinates
 * @param pole Position of the pole in painter coordinates
 * @param radius Radius of the complete plot area in painter coordinates
 * @param canvasRect Contents rect of the canvas in painter coordinates
 */
void QwtPolarPlot::drawItems(QPainter* painter,
                             const QwtScaleMap& azimuthMap,
                             const QwtScaleMap& radialMap,
                             const QPointF& pole,
                             double radius,
                             const QRectF& canvasRect) const
{
    const QRectF pr = plotRect(canvasRect);

    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        QwtPolarItem* item = *it;
        if (item && item->isVisible()) {
            painter->save();

            // Unfortunately circular clipping slows down
            // painting a lot. So we better try to avoid it.

            bool doClipping = false;
            if (item->rtti() != QwtPolarItem::Rtti_PolarGrid) {
                const QwtInterval intv = item->boundingInterval(QwtPolar::Radius);

                if (!intv.isValid())
                    doClipping = true;
                else {
                    if (radialMap.s1() < radialMap.s2())
                        doClipping = intv.maxValue() > radialMap.s2();
                    else
                        doClipping = intv.minValue() < radialMap.s2();
                }
            }

            if (doClipping) {
                const int margin = item->marginHint();

                const QRectF clipRect = pr.adjusted(-margin, -margin, margin, margin);
                if (!clipRect.contains(canvasRect)) {
                    QRegion clipRegion(clipRect.toRect(), QRegion::Ellipse);
                    painter->setClipRegion(clipRegion, Qt::IntersectClip);
                }
            }

            painter->setRenderHint(QPainter::Antialiasing, item->testRenderHint(QwtPolarItem::RenderAntialiased));

            item->draw(painter, azimuthMap, radialMap, pole, radius, canvasRect);

            painter->restore();
        }
    }
}

/**
 * @brief Rebuild the scale
 * @param scaleId Scale index
 */
void QwtPolarPlot::updateScale(int scaleId)
{
    QWT_D(d);

    if (scaleId < 0 || scaleId >= QwtPolar::ScaleCount)
        return;

    QwtPolarPlotScaleData& sd = d->scaleData[ scaleId ];

    double minValue = sd.minValue;
    double maxValue = sd.maxValue;
    double stepSize = sd.stepSize;

    if (scaleId == QwtPolar::ScaleRadius && sd.doAutoScale) {
        QwtInterval interval;

        const QwtPolarItemList& itmList = itemList();
        for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
            const QwtPolarItem* item = *it;
            if (item->testItemAttribute(QwtPolarItem::AutoScale))
                interval |= item->boundingInterval(scaleId);
        }

        minValue = interval.minValue();
        maxValue = interval.maxValue();

        sd.scaleEngine->autoScale(sd.maxMajor, minValue, maxValue, stepSize);
        sd.isValid = false;
    }

    if (!sd.isValid) {
        sd.scaleDiv = sd.scaleEngine->divideScale(minValue, maxValue, sd.maxMajor, sd.maxMinor, stepSize);
        sd.isValid  = true;
    }

    const QwtInterval interval = visibleInterval();

    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        QwtPolarItem* item = *it;
        item->updateScaleDiv(*scaleDiv(QwtPolar::Azimuth), *scaleDiv(QwtPolar::Radius), interval);
    }
}

/**
 * @brief Get the maximum of all item margin hints
 * @return Maximum margin hint
 * @sa QwtPolarItem::marginHint()
 */
int QwtPolarPlot::plotMarginHint() const
{
    int margin                      = 0;
    const QwtPolarItemList& itmList = itemList();
    for (QwtPolarItemIterator it = itmList.begin(); it != itmList.end(); ++it) {
        QwtPolarItem* item = *it;
        if (item && item->isVisible()) {
            const int hint = item->marginHint();
            if (hint > margin)
                margin = hint;
        }
    }
    return margin;
}

/**
 * @brief Get the bounding rect of the plot area
 * @details The plot area depends on the size of the canvas and the zoom parameters.
 * @return Bounding rect of the plot area
 */
QRectF QwtPolarPlot::plotRect() const
{
    return plotRect(canvas()->contentsRect());
}

/**
 * @brief Calculate the bounding rect of the plot area
 * @details The plot area depends on the zoom parameters.
 * @param canvasRect Rectangle of the canvas
 * @return Rectangle for displaying 100% of the plot
 */
QRectF QwtPolarPlot::plotRect(const QRectF& canvasRect) const
{
    QWT_DC(d);

    const QwtScaleDiv* sd    = scaleDiv(QwtPolar::Radius);
    const QwtScaleEngine* se = scaleEngine(QwtPolar::Radius);

    const int margin = plotMarginHint();
    const QRectF cr  = canvasRect;
    const int radius = qMin(cr.width(), cr.height()) / 2 - margin;

    QwtScaleMap map;
    map.setTransformation(se->transformation());
    map.setPaintInterval(0.0, radius / d->zoomFactor);
    map.setScaleInterval(sd->lowerBound(), sd->upperBound());

    double v = map.s1();
    if (map.s1() <= map.s2())
        v += d->zoomPos.radius();
    else
        v -= d->zoomPos.radius();
    v = map.transform(v);

    const QPointF off = QwtPointPolar(d->zoomPos.azimuth(), v).toPoint();

    QPointF center(cr.center().x(), cr.top() + margin + radius);
    center -= QPointF(off.x(), -off.y());

    QRectF rect(0, 0, 2 * map.p2(), 2 * map.p2());
    rect.moveCenter(center);

    return rect;
}

/**
 * @brief Get the bounding interval of the radial scale visible on canvas
 * @return Bounding interval of the radial scale that is visible on the canvas
 */
QwtInterval QwtPolarPlot::visibleInterval() const
{
    const QwtScaleDiv* sd = scaleDiv(QwtPolar::Radius);

    const QRectF cRect = canvas()->contentsRect();
    const QRectF pRect = plotRect(cRect);
    if (cRect.contains(pRect) || !cRect.intersects(pRect)) {
        return QwtInterval(sd->lowerBound(), sd->upperBound());
    }

    const QPointF pole     = pRect.center();
    const QRectF scaleRect = pRect & cRect;

    const QwtScaleMap map = scaleMap(QwtPolar::Radius);

    double dmin = 0.0;
    double dmax = 0.0;
    if (scaleRect.contains(pole)) {
        dmin = 0.0;

        QPointF corners[ 4 ];
        corners[ 0 ] = scaleRect.bottomRight();
        corners[ 1 ] = scaleRect.topRight();
        corners[ 2 ] = scaleRect.topLeft();
        corners[ 3 ] = scaleRect.bottomLeft();

        dmax = 0.0;
        for (int i = 0; i < 4; i++) {
            const double dist = qwtDistance(pole, corners[ i ]);
            if (dist > dmax)
                dmax = dist;
        }
    } else {
        if (pole.x() < scaleRect.left()) {
            if (pole.y() < scaleRect.top()) {
                dmin = qwtDistance(pole, scaleRect.topLeft());
                dmax = qwtDistance(pole, scaleRect.bottomRight());
            } else if (pole.y() > scaleRect.bottom()) {
                dmin = qwtDistance(pole, scaleRect.bottomLeft());
                dmax = qwtDistance(pole, scaleRect.topRight());
            } else {
                dmin = scaleRect.left() - pole.x();
                dmax = qMax(qwtDistance(pole, scaleRect.bottomRight()), qwtDistance(pole, scaleRect.topRight()));
            }
        } else if (pole.x() > scaleRect.right()) {
            if (pole.y() < scaleRect.top()) {
                dmin = qwtDistance(pole, scaleRect.topRight());
                dmax = qwtDistance(pole, scaleRect.bottomLeft());
            } else if (pole.y() > scaleRect.bottom()) {
                dmin = qwtDistance(pole, scaleRect.bottomRight());
                dmax = qwtDistance(pole, scaleRect.topLeft());
            } else {
                dmin = pole.x() - scaleRect.right();
                dmax = qMax(qwtDistance(pole, scaleRect.bottomLeft()), qwtDistance(pole, scaleRect.topLeft()));
            }
        } else if (pole.y() < scaleRect.top()) {
            dmin = scaleRect.top() - pole.y();
            dmax = qMax(qwtDistance(pole, scaleRect.bottomLeft()), qwtDistance(pole, scaleRect.bottomRight()));
        } else if (pole.y() > scaleRect.bottom()) {
            dmin = pole.y() - scaleRect.bottom();
            dmax = qMax(qwtDistance(pole, scaleRect.topLeft()), qwtDistance(pole, scaleRect.topRight()));
        }
    }

    const double radius = pRect.width() / 2.0;
    if (dmax > radius)
        dmax = radius;

    QwtInterval interval;
    interval.setMinValue(map.invTransform(dmin));
    interval.setMaxValue(map.invTransform(dmax));

    return interval;
}

/**
 * @brief Get the layout responsible for geometry of plot components
 * @return Plot layout
 */
QwtPolarLayout* QwtPolarPlot::plotLayout()
{
    QWT_D(d);
    return d->layout;
}

/**
 * @brief Get the layout responsible for geometry of plot components (const version)
 * @return Plot layout
 */
const QwtPolarLayout* QwtPolarPlot::plotLayout() const
{
    QWT_DC(d);
    return d->layout;
}

/**
 * @brief Attach/Detach a plot item
 * @param plotItem Plot item
 * @param on When true attach the item, otherwise detach it
 */
void QwtPolarPlot::attachItem(QwtPolarItem* plotItem, bool on)
{
    if (on)
        insertItem(plotItem);
    else
        removeItem(plotItem);

    Q_EMIT itemAttached(plotItem, on);

    if (plotItem->testItemAttribute(QwtPolarItem::Legend)) {
        // the item wants to be represented on the legend

        if (on) {
            updateLegend(plotItem);
        } else {
            const QVariant itemInfo = itemToInfo(plotItem);
            Q_EMIT legendDataChanged(itemInfo, QList< QwtLegendData >());
        }
    }

    if (autoReplot())
        update();
}

/**
 * @brief Build an information object to identify a plot item on the legend
 * @details The default implementation simply wraps the plot item into a QVariant object.
 *          When overloading itemToInfo() usually infoToItem() needs to be reimplemented too.
 * @param plotItem Plot item
 * @return QVariant containing the plot item information
 * @sa infoToItem()
 */
QVariant QwtPolarPlot::itemToInfo(QwtPolarItem* plotItem) const
{
    return QVariant::fromValue(plotItem);
}

/**
 * @brief Identify the plot item according to an item info object
 * @details The default implementation simply tries to unwrap a QwtPolarItem pointer.
 * @param itemInfo Item info object generated from itemToInfo()
 * @return A plot item when successful, otherwise nullptr
 * @sa itemToInfo()
 */
QwtPolarItem* QwtPolarPlot::infoToItem(const QVariant& itemInfo) const
{
    if (itemInfo.canConvert< QwtPolarItem* >())
        return qvariant_cast< QwtPolarItem* >(itemInfo);

    return nullptr;
}
