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
public:
    PrivateData()
    {
        for (int axis = 0; axis < QwtAxis::AxisPositions; axis++)
            isAxisEnabled[ axis ] = true;
    }

    bool isAxisEnabled[ QwtAxis::AxisPositions ];
};

/*!
\if ENGLISH
   @brief Constructs a panner for the canvas of a QwtPlot
   @details Creates a QwtPlotCachePanner object attached to the given canvas.
            The panner is enabled for all axes by default.
   
   @param canvas Plot canvas to pan, also becomes the parent object
   @sa setAxisEnabled()
\endif

\if CHINESE
   @brief 构造绑定为绘图部件画布的平移器
   @details 创建一个绑定到指定画布的 QwtPlotCachePanner 对象。
            默认情况下，所有坐标轴都启用平移功能。
   
   @param canvas 要平移的绘图画布，同时作为父对象
   @sa setAxisEnabled()
\endif
 */
QwtPlotCachePanner::QwtPlotCachePanner(QWidget* canvas) : QwtCachePanner(canvas)
{
    m_data = new PrivateData();
    connect(this, &QwtPlotCachePanner::panned, this, &QwtPlotCachePanner::moveCanvas);
    // connect(this, SIGNAL(panned(int, int)), SLOT(moveCanvas(int, int)));
}

/*!
\if ENGLISH
   @brief Destructor
   @details Releases all resources held by the panner.
\endif

\if CHINESE
   @brief 析构函数
   @details 释放平移器持有的所有资源。
\endif
 */
QwtPlotCachePanner::~QwtPlotCachePanner()
{
    delete m_data;
}

/*!
\if ENGLISH
   @brief Enables or disables an axis for panning
   @details Axes that are enabled will be synchronized to the
            result of panning. All other axes will remain unchanged.
   
   @param axisId Axis identifier
   @param on True to enable, false to disable
   @sa isAxisEnabled(), moveCanvas()
\endif

\if CHINESE
   @brief 启用或禁用坐标轴的平移功能
   @details 启用的坐标轴会在平移操作后同步更新，
            其他未启用的坐标轴将保持不变。
   
   @param axisId 坐标轴标识符
   @param on true 为启用，false 为禁用
   @sa isAxisEnabled(), moveCanvas()
\endif
 */
void QwtPlotCachePanner::setAxisEnabled(QwtAxisId axisId, bool on)
{
    if (QwtAxis::isValid(axisId))
        m_data->isAxisEnabled[ axisId ] = on;
}

/*!
\if ENGLISH
   @brief Tests if an axis is enabled for panning
   @param axisId Axis identifier to test
   @return True if the axis is enabled, false otherwise
   @sa setAxisEnabled(), moveCanvas()
\endif

\if CHINESE
   @brief 测试坐标轴是否启用平移功能
   @param axisId 要测试的坐标轴标识符
   @return 如果坐标轴已启用则返回 true，否则返回 false
   @sa setAxisEnabled(), moveCanvas()
\endif
 */
bool QwtPlotCachePanner::isAxisEnabled(QwtAxisId axisId) const
{
    if (QwtAxis::isValid(axisId))
        return m_data->isAxisEnabled[ axisId ];

    return true;
}

/*!
\if ENGLISH
   @brief Returns the observed plot canvas
   @return Pointer to the canvas widget
\endif

\if CHINESE
   @brief 返回被观察的绘图画布
   @return 画布部件的指针
\endif
 */
QWidget* QwtPlotCachePanner::canvas()
{
    return parentWidget();
}

/*!
\if ENGLISH
   @brief Returns the observed plot canvas (const version)
   @return Const pointer to the canvas widget
\endif

\if CHINESE
   @brief 返回被观察的绘图画布（常量版本）
   @return 画布部件的常量指针
\endif
 */
const QWidget* QwtPlotCachePanner::canvas() const
{
    return parentWidget();
}

/*!
\if ENGLISH
   @brief Returns the plot widget containing the observed canvas
   @return Pointer to the QwtPlot widget, or nullptr if not found
\endif

\if CHINESE
   @brief 返回包含被观察画布的绘图部件
   @return QwtPlot 部件的指针，如果未找到则返回 nullptr
\endif
 */
QwtPlot* QwtPlotCachePanner::plot()
{
    QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >(w);
}

/*!
\if ENGLISH
   @brief Returns the plot widget containing the observed canvas (const version)
   @return Const pointer to the QwtPlot widget, or nullptr if not found
\endif

\if CHINESE
   @brief 返回包含被观察画布的绘图部件（常量版本）
   @return QwtPlot 部件的常量指针，如果未找到则返回 nullptr
\endif
 */
const QwtPlot* QwtPlotCachePanner::plot() const
{
    const QWidget* w = canvas();
    if (w)
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >(w);
}

/*!
\if ENGLISH
   @brief Adjusts the enabled axes according to the pixel offset
   @details Moves the canvas by the specified pixel offset and updates
            the axis scales accordingly. Only enabled axes are affected.
   
   @param dx Pixel offset in x direction
   @param dy Pixel offset in y direction
   @sa QwtPanner::panned(), setAxisEnabled()
\endif

\if CHINESE
   @brief 根据像素偏移量调整已启用的坐标轴
   @details 按指定的像素偏移量移动画布，并相应地更新坐标轴比例尺。
            只有已启用的坐标轴会受到影响。
   
   @param dx X 方向的像素偏移量
   @param dy Y 方向的像素偏移量
   @sa QwtPanner::panned(), setAxisEnabled()
\endif
 */
void QwtPlotCachePanner::moveCanvas(int dx, int dy)
{
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

            if (!m_data->isAxisEnabled[ axisId ])
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

   \return Mask as bitmap
   \sa QwtPlotCanvas::borderPath()
 */
QBitmap QwtPlotCachePanner::contentsMask() const
{
    if (canvas())
        return qwtBorderMask(canvas(), size());

    return QwtCachePanner::contentsMask();
}

/*!
   \return Pixmap with the content of the canvas
 */
QPixmap QwtPlotCachePanner::grab() const
{
    const QWidget* cv = canvas();
    if (cv && cv->inherits("QGLWidget")) {
        // we can't grab from a QGLWidget

        QPixmap pm(cv->size());
        QwtPainter::fillPixmap(cv, pm);

        QPainter painter(&pm);
        const_cast< QwtPlot* >(plot())->drawCanvas(&painter);

        return pm;
    }

    return QwtCachePanner::grab();
}
