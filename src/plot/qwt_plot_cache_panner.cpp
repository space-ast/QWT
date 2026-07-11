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
 *****************************************************************************/

#include "qwt_plot_cache_panner.h"
#include "qwt_scale_div.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"

#include <qbitmap.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qpainterpath.h>

static QBitmap qwtBorderMask(const QWidget* canvas, const QSize& size)
{
#if QT_VERSION >= 0x050000
    const qreal pixelRatio = QwtPainter::devicePixelRatio(canvas);
#endif

    const QRect r(0, 0, size.width(), size.height());

    QPainterPath borderPath;

    (void)QMetaObject::invokeMethod(const_cast< QWidget* >(canvas),
                                    "borderPath",
                                    Qt::DirectConnection,
                                    Q_RETURN_ARG(QPainterPath, borderPath),
                                    Q_ARG(QRect, r));

    if (borderPath.isEmpty()) {
        if (canvas->contentsRect() == canvas->rect())
            return QBitmap();

#if QT_VERSION >= 0x050000
        QBitmap mask(size * pixelRatio);
        mask.setDevicePixelRatio(pixelRatio);
#else
        QBitmap mask(size);
#endif
        mask.fill(Qt::color0);

        QPainter painter(&mask);
        painter.fillRect(canvas->contentsRect(), Qt::color1);

        return mask;
    }

#if QT_VERSION >= 0x050000
    QImage image(size * pixelRatio, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(pixelRatio);
#else
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
#endif
    image.fill(Qt::color0);

    QPainter painter(&image);
    painter.setClipPath(borderPath);
    painter.fillRect(r, Qt::color1);

    // now erase the frame

    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);

    if (canvas->testAttribute(Qt::WA_StyledBackground)) {
        QStyleOptionFrame opt;
        opt.initFrom(canvas);
        opt.rect = r;
        canvas->style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, canvas);
    } else {
        const QVariant borderRadius = canvas->property("borderRadius");
        const QVariant frameWidth   = canvas->property("frameWidth");

        if (borderRadius.canConvert< double >() && frameWidth.canConvert< int >()) {
            const double br = borderRadius.value< double >();
            const int fw    = frameWidth.value< int >();

            if (br > 0.0 && fw > 0) {
                painter.setPen(QPen(Qt::color1, fw));
                painter.setBrush(Qt::NoBrush);
                painter.setRenderHint(QPainter::Antialiasing, true);

                painter.drawPath(borderPath);
            }
        }
    }

    painter.end();

    const QImage mask = image.createMaskFromColor(QColor(Qt::color1).rgb(), Qt::MaskOutColor);

    return QBitmap::fromImage(mask);
}

class QwtPlotCachePanner::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotCachePanner)
public:
    PrivateData(QwtPlotCachePanner* p) : q_ptr(p)
    {
        for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
            isAxisEnabled[ axis ] = true;
    }

    bool isAxisEnabled[ QwtAxis::AxisPositions ];
};

/*!
   @brief Constructs a panner for the canvas of a QwtPlot
   @details Creates a QwtPlotCachePanner object attached to the given canvas.
            The panner is enabled for all axes by default.

   @param canvas Plot canvas to pan, also becomes the parent object
   @sa setAxisEnabled()
 */
QwtPlotCachePanner::QwtPlotCachePanner(QWidget* canvas) : QwtCachePanner(canvas), QWT_PIMPL_CONSTRUCT
{
    connect(this, &QwtPlotCachePanner::panned, this, &QwtPlotCachePanner::moveCanvas);
    // connect(this, SIGNAL(panned(int, int)), SLOT(moveCanvas(int, int)));
}

/*!
   @brief Destructor
   @details Releases all resources held by the panner.
 */
QwtPlotCachePanner::~QwtPlotCachePanner()
{
}

/*!
   @brief Enables or disables an axis for panning
   @details Axes that are enabled will be synchronized to the
            result of panning. All other axes will remain unchanged.

   @param axisId Axis identifier
   @param on True to enable, false to disable
   @sa isAxisEnabled(), moveCanvas()
 */
void QwtPlotCachePanner::setAxisEnabled(QwtAxisId axisId, bool on)
{
    QWT_D(d);
    if (QwtAxis::isValid(axisId))
        d->isAxisEnabled[ axisId ] = on;
}

/*!
   @brief Tests if an axis is enabled for panning
   @param axisId Axis identifier to test
   @return True if the axis is enabled, false otherwise
   @sa setAxisEnabled(), moveCanvas()
 */
bool QwtPlotCachePanner::isAxisEnabled(QwtAxisId axisId) const
{
    QWT_DC(d);
    if (QwtAxis::isValid(axisId))
        return d->isAxisEnabled[ axisId ];

    return true;
}

/*!
   @brief Returns the observed plot canvas
   @return Pointer to the canvas widget
 */
QWidget* QwtPlotCachePanner::canvas()
{
    return parentWidget();
}

/*!
   @brief Returns the observed plot canvas (const version)
   @return Const pointer to the canvas widget
 */
const QWidget* QwtPlotCachePanner::canvas() const
{
    return parentWidget();
}

/*!
   @brief Returns the plot widget containing the observed canvas
   @return Pointer to the QwtPlot widget, or nullptr if not found
 */
QwtPlot* QwtPlotCachePanner::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/*!
   @brief Returns the plot widget containing the observed canvas (const version)
   @return Const pointer to the QwtPlot widget, or nullptr if not found
 */
const QwtPlot* QwtPlotCachePanner::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
   @brief Adjusts the enabled axes according to the pixel offset
   @details Moves the canvas by the specified pixel offset and updates
            the axis scales accordingly. Only enabled axes are affected.

   @param dx Pixel offset in x direction
   @param dy Pixel offset in y direction
   @sa QwtPanner::panned(), setAxisEnabled()
 */
void QwtPlotCachePanner::moveCanvas(int dx, int dy)
{
    QWT_D(d);
    if (dx == 0 && dy == 0)
        return;

    QwtPlot* plot = this->plot();
    if (plot == nullptr)
        return;

    plot->saveAutoReplotState();
    plot->setAutoReplot(false);

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        {
            const QwtAxisId axisId(axisPos);

            if (!d->isAxisEnabled[ axisId ])
                continue;

            const QwtScaleMap map = plot->canvasMap(axisId);

            const double p1 = map.transform(plot->axisScaleDiv(axisId).lowerBound());
            const double p2 = map.transform(plot->axisScaleDiv(axisId).upperBound());

            double d1, d2;
            if (QwtAxis::isXAxis(axisPos)) {
                d1 = map.invTransform(p1 - dx);
                d2 = map.invTransform(p2 - dx);
            } else {
                d1 = map.invTransform(p1 - dy);
                d2 = map.invTransform(p2 - dy);
            }

            plot->setAxisScale(axisId, d1, d2);
        }
    }

    plot->restoreAutoReplotState();
    plot->replot();
}

/*!
   Calculate a mask from the border path of the canvas

   @return Mask as bitmap
   @sa QwtPlotCanvas::borderPath()
 */
QBitmap QwtPlotCachePanner::contentsMask() const
{
    if (canvas())
        return qwtBorderMask(canvas(), size());

    return QwtCachePanner::contentsMask();
}

/*!
   @return Pixmap with the content of the canvas
 */
QPixmap QwtPlotCachePanner::grab() const
{
    const QWidget* cv = canvas();
    if (cv && (cv->inherits("QGLWidget") || cv->inherits("QOpenGLWidget"))) {
        // we can't grab from a QGLWidget/QOpenGLWidget

        QPixmap pm(cv->size());
        QwtPainter::fillPixmap(cv, pm);

        QPainter painter(&pm);
        const_cast< QwtPlot* >(plot())->drawCanvas(&painter);

        return pm;
    }

    return QwtCachePanner::grab();
}
