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

#include "qwt_plot_renderer.h"
#include "qwt_plot.h"
#include "qwt_painter.h"
#include "qwt_plot_layout.h"
#include "qwt_abstract_legend.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qtransform.h>
#include <qprinter.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qimagewriter.h>
#include <qvariant.h>
#include <qmargins.h>

#ifndef QWT_NO_SVG
#ifdef QT_SVG_LIB
#define QWT_FORMAT_SVG 1
#endif
#endif

#ifndef QT_NO_PRINTER
#define QWT_FORMAT_PDF 1
#endif

#ifndef QT_NO_PDF

// QPdfWriter::setResolution() has been introduced with
// Qt 5.3. Guess it is o.k. to stay with QPrinter for older
// versions.

#if QT_VERSION >= 0x050300

#ifndef QWT_FORMAT_PDF
#define QWT_FORMAT_PDF 1
#endif

#define QWT_PDF_WRITER 1

#endif
#endif

#ifndef QT_NO_PRINTER
// postscript support has been dropped in Qt5
#if QT_VERSION < 0x050000
#define QWT_FORMAT_POSTSCRIPT 1
#endif
#endif

#if QWT_FORMAT_SVG
#include <qsvggenerator.h>
#endif

#if QWT_PDF_WRITER
#include <qpdfwriter.h>
#endif

static qreal qwtScalePenWidth(const QwtPlot* plot)
{
    qreal pw = 0.0;

    for (int axisId = 0; axisId < QwtAxis::AxisPositions; axisId++) {
        if (plot->isAxisVisible(axisId))
            pw = qMax(pw, plot->axisScaleDraw(axisId)->penWidthF());
    }

    return pw;
}

static QColor qwtScalePenColor(const QwtPlot* plot)
{
    const QPalette pal = plot->axisWidget(QwtAxis::YLeft)->palette();
    return pal.color(QPalette::WindowText);
}

static QPainterPath qwtCanvasClip(const QWidget* canvas, const QRectF& canvasRect)
{
    // The clip region is calculated in integers
    // To avoid too much rounding errors better
    // calculate it in target device resolution

    int x1 = qwtCeil(canvasRect.left());
    int x2 = qwtFloor(canvasRect.right());
    int y1 = qwtCeil(canvasRect.top());
    int y2 = qwtFloor(canvasRect.bottom());

    const QRect r(x1, y1, x2 - x1 - 1, y2 - y1 - 1);

    QPainterPath clipPath;

    (void)QMetaObject::invokeMethod(const_cast< QWidget* >(canvas),
                                    "borderPath",
                                    Qt::DirectConnection,
                                    Q_RETURN_ARG(QPainterPath, clipPath),
                                    Q_ARG(QRect, r));

    return clipPath;
}

static inline QFont qwtResolvedFont(const QWidget* widget)
{
    QFont font = widget->font();
#if QT_VERSION >= 0x060000
    font.setResolveMask(QFont::AllPropertiesResolved);
#else
    font.resolve(QFont::AllPropertiesResolved);
#endif

    return font;
}

class QwtPlotRenderer::PrivateData
{
public:
    PrivateData() : discardFlags(QwtPlotRenderer::DiscardNone), layoutFlags(QwtPlotRenderer::DefaultLayout)
    {
    }

    QwtPlotRenderer::DiscardFlags discardFlags;
    QwtPlotRenderer::LayoutFlags layoutFlags;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] parent Parent object
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] parent 父对象
 * \endif
 */
QwtPlotRenderer::QwtPlotRenderer(QObject* parent) : QObject(parent)
{
    m_data = new PrivateData;
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
QwtPlotRenderer::~QwtPlotRenderer()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Change a flag, indicating what to discard from rendering
 * @param[in] flag Flag to change
 * @param[in] on On/Off
 * @sa DiscardFlag, testDiscardFlag(), setDiscardFlags(), discardFlags()
 * \endif
 *
 * \if CHINESE
 * @brief 更改指示渲染时丢弃内容的标志
 * @param[in] flag 要更改的标志
 * @param[in] on 开启或关闭
 * @sa DiscardFlag, testDiscardFlag(), setDiscardFlags(), discardFlags()
 * \endif
 */
void QwtPlotRenderer::setDiscardFlag(DiscardFlag flag, bool on)
{
    if (on)
        m_data->discardFlags |= flag;
    else
        m_data->discardFlags &= ~flag;
}

/**
 * \if ENGLISH
 * @brief Test if a discard flag is enabled
 * @param[in] flag Flag to be tested
 * @return True if flag is enabled
 * @sa DiscardFlag, setDiscardFlag(), setDiscardFlags(), discardFlags()
 * \endif
 *
 * \if CHINESE
 * @brief 测试丢弃标志是否启用
 * @param[in] flag 要测试的标志
 * @return 如果标志启用则返回 true
 * @sa DiscardFlag, setDiscardFlag(), setDiscardFlags(), discardFlags()
 * \endif
 */
bool QwtPlotRenderer::testDiscardFlag(DiscardFlag flag) const
{
    return m_data->discardFlags & flag;
}

/**
 * \if ENGLISH
 * @brief Set the flags, indicating what to discard from rendering
 * @param[in] flags Flags to set
 * @sa DiscardFlag, setDiscardFlag(), testDiscardFlag(), discardFlags()
 * \endif
 *
 * \if CHINESE
 * @brief 设置指示渲染时丢弃内容的标志
 * @param[in] flags 要设置的标志
 * @sa DiscardFlag, setDiscardFlag(), testDiscardFlag(), discardFlags()
 * \endif
 */
void QwtPlotRenderer::setDiscardFlags(DiscardFlags flags)
{
    m_data->discardFlags = flags;
}

/**
 * \if ENGLISH
 * @brief Get the flags indicating what to discard from rendering
 * @return Flags indicating what to discard from rendering
 * @sa DiscardFlag, setDiscardFlags(), setDiscardFlag(), testDiscardFlag()
 * \endif
 *
 * \if CHINESE
 * @brief 获取指示渲染时丢弃内容的标志
 * @return 指示渲染时丢弃内容的标志
 * @sa DiscardFlag, setDiscardFlags(), setDiscardFlag(), testDiscardFlag()
 * \endif
 */
QwtPlotRenderer::DiscardFlags QwtPlotRenderer::discardFlags() const
{
    return m_data->discardFlags;
}

/**
 * \if ENGLISH
 * @brief Change a layout flag
 * @param[in] flag Flag to change
 * @param[in] on On/Off
 * @sa LayoutFlag, testLayoutFlag(), setLayoutFlags(), layoutFlags()
 * \endif
 *
 * \if CHINESE
 * @brief 更改布局标志
 * @param[in] flag 要更改的标志
 * @param[in] on 开启或关闭
 * @sa LayoutFlag, testLayoutFlag(), setLayoutFlags(), layoutFlags()
 * \endif
 */
void QwtPlotRenderer::setLayoutFlag(LayoutFlag flag, bool on)
{
    if (on)
        m_data->layoutFlags |= flag;
    else
        m_data->layoutFlags &= ~flag;
}

/**
 * \if ENGLISH
 * @brief Test if a layout flag is enabled
 * @param[in] flag Flag to be tested
 * @return True if flag is enabled
 * @sa LayoutFlag, setLayoutFlag(), setLayoutFlags(), layoutFlags()
 * \endif
 *
 * \if CHINESE
 * @brief 测试布局标志是否启用
 * @param[in] flag 要测试的标志
 * @return 如果标志启用则返回 true
 * @sa LayoutFlag, setLayoutFlag(), setLayoutFlags(), layoutFlags()
 * \endif
 */
bool QwtPlotRenderer::testLayoutFlag(LayoutFlag flag) const
{
    return m_data->layoutFlags & flag;
}

/**
 * \if ENGLISH
 * @brief Set the layout flags
 * @param[in] flags Flags to set
 * @sa LayoutFlag, setLayoutFlag(), testLayoutFlag(), layoutFlags()
 * \endif
 *
 * \if CHINESE
 * @brief 设置布局标志
 * @param[in] flags 要设置的标志
 * @sa LayoutFlag, setLayoutFlag(), testLayoutFlag(), layoutFlags()
 * \endif
 */
void QwtPlotRenderer::setLayoutFlags(LayoutFlags flags)
{
    m_data->layoutFlags = flags;
}

/**
 * \if ENGLISH
 * @brief Get the layout flags
 * @return Layout flags
 * @sa LayoutFlag, setLayoutFlags(), setLayoutFlag(), testLayoutFlag()
 * \endif
 *
 * \if CHINESE
 * @brief 获取布局标志
 * @return 布局标志
 * @sa LayoutFlag, setLayoutFlags(), setLayoutFlag(), testLayoutFlag()
 * \endif
 */
QwtPlotRenderer::LayoutFlags QwtPlotRenderer::layoutFlags() const
{
    return m_data->layoutFlags;
}

/**
 * \if ENGLISH
 * @brief Render a plot to a file
 * @details The format of the document will be auto-detected from the suffix of the file name.
 * @param[in] plot Plot widget
 * @param[in] fileName Path of the file where the document will be stored
 * @param[in] sizeMM Size for the document in millimeters
 * @param[in] resolution Resolution in dots per Inch (dpi)
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到文件
 * @details 文档格式将从文件名后缀自动检测。
 * @param[in] plot 绘图控件
 * @param[in] fileName 存储文档的文件路径
 * @param[in] sizeMM 文档尺寸（毫米）
 * @param[in] resolution 分辨率（点/英寸）
 * \endif
 */
void QwtPlotRenderer::renderDocument(QwtPlot* plot, const QString& fileName, const QSizeF& sizeMM, int resolution)
{
    renderDocument(plot, fileName, QFileInfo(fileName).suffix(), sizeMM, resolution);
}

/**
 * \if ENGLISH
 * @brief Render a plot to a file with specified format
 * @details Supported formats are:
 *          - pdf: Portable Document Format PDF
 *          - ps: PostScript
 *          - svg: Scalable Vector Graphics SVG
 *          - all image formats supported by Qt (see QImageWriter::supportedImageFormats())
 *          Scalable vector graphic formats like PDF or SVG are superior to raster graphics formats.
 * @param[in] plot Plot widget
 * @param[in] fileName Path of the file where the document will be stored
 * @param[in] format Format for the document
 * @param[in] sizeMM Size for the document in millimeters
 * @param[in] resolution Resolution in dots per Inch (dpi)
 * @sa renderTo(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到指定格式的文件
 * @details 支持的格式有：
 *          - pdf：便携式文档格式 PDF
 *          - ps：PostScript
 *          - svg：可缩放矢量图形 SVG
 *          - Qt 支持的所有图像格式（参见 QImageWriter::supportedImageFormats()）
 *          像 PDF 或 SVG 这样的可缩放矢量图形格式优于栅格图形格式。
 * @param[in] plot 绘图控件
 * @param[in] fileName 存储文档的文件路径
 * @param[in] format 文档格式
 * @param[in] sizeMM 文档尺寸（毫米）
 * @param[in] resolution 分辨率（点/英寸）
 * @sa renderTo(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 */
void QwtPlotRenderer::renderDocument(QwtPlot* plot, const QString& fileName, const QString& format, const QSizeF& sizeMM, int resolution)
{
    if (plot == nullptr || sizeMM.isEmpty() || resolution <= 0)
        return;

    QString title = plot->title().text();
    if (title.isEmpty())
        title = "Plot Document";

    const double mmToInch = 1.0 / 25.4;
    const QSizeF size     = sizeMM * mmToInch * resolution;

    const QRectF documentRect(0.0, 0.0, size.width(), size.height());

    const QString fmt = format.toLower();
    if (fmt == QLatin1String("pdf")) {
#if QWT_FORMAT_PDF

#if QWT_PDF_WRITER
        QPdfWriter pdfWriter(fileName);
        pdfWriter.setPageSize(QPageSize(sizeMM, QPageSize::Millimeter));
        pdfWriter.setTitle(title);
        pdfWriter.setPageMargins(QMarginsF());
        pdfWriter.setResolution(resolution);

        QPainter painter(&pdfWriter);
        render(plot, &painter, documentRect);
#else
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setColorMode(QPrinter::Color);
        printer.setFullPage(true);
        printer.setPaperSize(sizeMM, QPrinter::Millimeter);
        printer.setDocName(title);
        printer.setOutputFileName(fileName);
        printer.setResolution(resolution);

        QPainter painter(&printer);
        render(plot, &painter, documentRect);
#endif
#endif
    } else if (fmt == QLatin1String("ps")) {
#if QWT_FORMAT_POSTSCRIPT
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PostScriptFormat);
        printer.setColorMode(QPrinter::Color);
        printer.setFullPage(true);
        printer.setPaperSize(sizeMM, QPrinter::Millimeter);
        printer.setDocName(title);
        printer.setOutputFileName(fileName);
        printer.setResolution(resolution);

        QPainter painter(&printer);
        render(plot, &painter, documentRect);
#endif
    } else if (fmt == QLatin1String("svg")) {
#if QWT_FORMAT_SVG
        QSvgGenerator generator;
        generator.setTitle(title);
        generator.setFileName(fileName);
        generator.setResolution(resolution);
        generator.setViewBox(documentRect);

        QPainter painter(&generator);
        render(plot, &painter, documentRect);
#endif
    } else {
        if (QImageWriter::supportedImageFormats().indexOf(format.toLatin1()) >= 0) {
            const QRect imageRect  = documentRect.toRect();
            const int dotsPerMeter = qRound(resolution * mmToInch * 1000.0);

            QImage image(imageRect.size(), QImage::Format_ARGB32);
            image.setDotsPerMeterX(dotsPerMeter);
            image.setDotsPerMeterY(dotsPerMeter);
            image.fill(QColor(Qt::white).rgb());

            QPainter painter(&image);
            render(plot, &painter, imageRect);
            painter.end();

            image.save(fileName, format.toLatin1());
        }
    }
}

/**
 * \if ENGLISH
 * @brief Render the plot to a QPaintDevice
 * @details This function renders the contents of a QwtPlot instance to QPaintDevice object.
 *          The target rectangle is derived from its device metrics.
 * @param[in] plot Plot to be rendered
 * @param[in] paintDevice Device to paint on, f.e. a QImage
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到 QPaintDevice
 * @details 此函数将 QwtPlot 实例的内容渲染到 QPaintDevice 对象。
 *          目标矩形从其设备指标派生。
 * @param[in] plot 要渲染的绘图
 * @param[in] paintDevice 绘制设备，例如 QImage
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 */

void QwtPlotRenderer::renderTo(QwtPlot* plot, QPaintDevice& paintDevice) const
{
    int w = paintDevice.width();
    int h = paintDevice.height();

    QPainter p(&paintDevice);
    render(plot, &p, QRectF(0, 0, w, h));
}

/**
 * \if ENGLISH
 * @brief Render the plot to a QPrinter
 * @details This function renders the contents of a QwtPlot instance to QPaintDevice object.
 *          The size is derived from the printer metrics.
 * @param[in] plot Plot to be rendered
 * @param[in] printer Printer to paint on
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到 QPrinter
 * @details 此函数将 QwtPlot 实例的内容渲染到 QPaintDevice 对象。
 *          尺寸从打印机指标派生。
 * @param[in] plot 要渲染的绘图
 * @param[in] printer 绘制打印机
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 */

#ifndef QT_NO_PRINTER

void QwtPlotRenderer::renderTo(QwtPlot* plot, QPrinter& printer) const
{
    int w = printer.width();
    int h = printer.height();

    QRectF rect(0, 0, w, h);
    double aspect = rect.width() / rect.height();
    if ((aspect < 1.0))
        rect.setHeight(aspect * rect.width());

    QPainter p(&printer);
    render(plot, &p, rect);
}

#endif

#if QWT_FORMAT_SVG

/**
 * \if ENGLISH
 * @brief Render the plot to a QSvgGenerator
 * @details If the generator has a view box, the plot will be rendered into it.
 *          If it has no viewBox but a valid size, the target coordinates will be
 *          (0, 0, generator.width(), generator.height()). Otherwise the target
 *          rectangle will be QRectF(0, 0, 800, 600).
 * @param[in] plot Plot to be rendered
 * @param[in] generator SVG generator
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到 QSvgGenerator
 * @details 如果生成器有视图框，绘图将渲染到其中。
 *          如果没有 viewBox 但有有效尺寸，目标坐标将是
 *          (0, 0, generator.width(), generator.height())。
 *          否则目标矩形将是 QRectF(0, 0, 800, 600)。
 * @param[in] plot 要渲染的绘图
 * @param[in] generator SVG 生成器
 * \endif
 */
void QwtPlotRenderer::renderTo(QwtPlot* plot, QSvgGenerator& generator) const
{
    QRectF rect = generator.viewBoxF();
    if (rect.isEmpty())
        rect.setRect(0, 0, generator.width(), generator.height());

    if (rect.isEmpty())
        rect.setRect(0, 0, 800, 600);  // something

    QPainter p(&generator);
    render(plot, &p, rect);
}

#endif

/**
 * \if ENGLISH
 * @brief Paint the contents of a QwtPlot instance into a given rectangle
 * @param[in] plot Plot to be rendered
 * @param[in] painter Painter
 * @param[in] plotRect Bounding rectangle
 * @sa renderDocument(), renderTo(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将 QwtPlot 实例的内容绘制到给定矩形中
 * @param[in] plot 要渲染的绘图
 * @param[in] painter 绘制器
 * @param[in] plotRect 边界矩形
 * @sa renderDocument(), renderTo(), QwtPainter::setRoundingAlignment()
 * \endif
 */
void QwtPlotRenderer::render(QwtPlot* plot, QPainter* painter, const QRectF& plotRect) const
{
    if (painter == 0 || !painter->isActive() || !plotRect.isValid() || plot->size().isNull()) {
        return;
    }

    if (!(m_data->discardFlags & DiscardBackground))
        QwtPainter::drawBackgound(painter, plotRect, plot);

    /*
       The layout engine uses the same methods as they are used
       by the Qt layout system. Therefore we need to calculate the
       layout in screen coordinates and paint with a scaled painter.
     */
    QTransform transform;
    transform.scale(double(painter->device()->logicalDpiX()) / plot->logicalDpiX(),
                    double(painter->device()->logicalDpiY()) / plot->logicalDpiY());

    QRectF layoutRect = transform.inverted().mapRect(plotRect);

    if (!(m_data->discardFlags & DiscardBackground)) {
        // subtract the contents margins

        const QMargins m = plot->contentsMargins();
        layoutRect.adjust(m.left(), m.top(), -m.right(), -m.bottom());
    }

    QwtPlotLayout* layout = plot->plotLayout();

    int baseLineDists[ QwtAxis::AxisPositions ];
    int canvasMargins[ QwtAxis::AxisPositions ];

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        canvasMargins[ axisPos ] = layout->canvasMargin(axisPos);

        if (m_data->layoutFlags & FrameWithScales) {
            const QwtAxisId axisId(axisPos);

            QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
            if (scaleWidget) {
                baseLineDists[ axisPos ] = scaleWidget->margin();
                scaleWidget->setMargin(0);
            }

            if (!plot->isAxisVisible(axisId)) {
                // When we have a scale the frame is painted on
                // the position of the backbone - otherwise we
                // need to introduce a margin around the canvas

                const qreal fw = qwtScalePenWidth(plot);

                switch (axisPos) {
                case QwtAxis::YLeft:
                    layoutRect.adjust(fw, 0, 0, 0);
                    break;

                case QwtAxis::YRight:
                    layoutRect.adjust(0, 0, -fw, 0);
                    break;

                case QwtAxis::XTop:
                    layoutRect.adjust(0, fw, 0, 0);
                    break;

                case QwtAxis::XBottom:
                    layoutRect.adjust(0, 0, 0, -fw);
                    break;

                default:;
                }
            }
        }
    }

    // Calculate the layout for the document.

    QwtPlotLayout::Options layoutOptions = QwtPlotLayout::IgnoreScrollbars;

    if ((m_data->layoutFlags & FrameWithScales) || (m_data->discardFlags & DiscardCanvasFrame)) {
        layoutOptions |= QwtPlotLayout::IgnoreFrames;
    }

    if (m_data->discardFlags & DiscardLegend)
        layoutOptions |= QwtPlotLayout::IgnoreLegend;

    if (m_data->discardFlags & DiscardTitle)
        layoutOptions |= QwtPlotLayout::IgnoreTitle;

    if (m_data->discardFlags & DiscardFooter)
        layoutOptions |= QwtPlotLayout::IgnoreFooter;

    layout->activate(plot, layoutRect, layoutOptions);

    // canvas

    QwtScaleMap maps[ QwtAxis::AxisPositions ];
    buildCanvasMaps(plot, layout->canvasRect(), maps);
    if (updateCanvasMargins(plot, layout->canvasRect(), maps)) {
        // recalculate maps and layout, when the margins
        // have been changed

        layout->activate(plot, layoutRect, layoutOptions);
        buildCanvasMaps(plot, layout->canvasRect(), maps);
    }

    // now start painting

    painter->save();
    painter->setWorldTransform(transform, true);

    renderCanvas(plot, painter, layout->canvasRect(), maps);

    if (!(m_data->discardFlags & DiscardTitle) && (!plot->titleLabel()->text().isEmpty())) {
        renderTitle(plot, painter, layout->titleRect());
    }

    if (!(m_data->discardFlags & DiscardFooter) && (!plot->footerLabel()->text().isEmpty())) {
        renderFooter(plot, painter, layout->footerRect());
    }

    if (!(m_data->discardFlags & DiscardLegend) && plot->legend() && !plot->legend()->isEmpty()) {
        renderLegend(plot, painter, layout->legendRect());
    }

    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        {
            const QwtAxisId axisId(axisPos);

            QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
            if (scaleWidget) {
                int baseDist = scaleWidget->margin();

                int startDist, endDist;
                scaleWidget->getBorderDistHint(startDist, endDist);

                renderScale(plot, painter, axisId, startDist, endDist, baseDist, layout->scaleRect(axisId));
            }
        }
    }

    painter->restore();

    // restore all setting to their original attributes.
    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        if (m_data->layoutFlags & FrameWithScales) {
            const QwtAxisId axisId(axisPos);

            QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
            if (scaleWidget)
                scaleWidget->setMargin(baseLineDists[ axisPos ]);
        }

        layout->setCanvasMargin(canvasMargins[ axisPos ]);
    }

    layout->invalidate();
}

/**
 * \if ENGLISH
 * @brief Render the title into a given rectangle
 * @param[in] plot Plot widget
 * @param[in] painter Painter
 * @param[in] titleRect Bounding rectangle for the title
 * \endif
 *
 * \if CHINESE
 * @brief 将标题渲染到给定矩形中
 * @param[in] plot 绘图控件
 * @param[in] painter 绘制器
 * @param[in] titleRect 标题的边界矩形
 * \endif
 */
void QwtPlotRenderer::renderTitle(const QwtPlot* plot, QPainter* painter, const QRectF& titleRect) const
{
    painter->setFont(qwtResolvedFont(plot->titleLabel()));

    const QColor color = plot->titleLabel()->palette().color(QPalette::Active, QPalette::Text);

    painter->setPen(color);
    plot->titleLabel()->text().draw(painter, titleRect);
}

/**
 * \if ENGLISH
 * @brief Render the footer into a given rectangle
 * @param[in] plot Plot widget
 * @param[in] painter Painter
 * @param[in] footerRect Bounding rectangle for the footer
 * \endif
 *
 * \if CHINESE
 * @brief 将页脚渲染到给定矩形中
 * @param[in] plot 绘图控件
 * @param[in] painter 绘制器
 * @param[in] footerRect 页脚的边界矩形
 * \endif
 */
void QwtPlotRenderer::renderFooter(const QwtPlot* plot, QPainter* painter, const QRectF& footerRect) const
{
    painter->setFont(qwtResolvedFont(plot->footerLabel()));

    const QColor color = plot->footerLabel()->palette().color(QPalette::Active, QPalette::Text);

    painter->setPen(color);
    plot->footerLabel()->text().draw(painter, footerRect);
}

/**
 * \if ENGLISH
 * @brief Render the legend into a given rectangle
 * @param[in] plot Plot widget
 * @param[in] painter Painter
 * @param[in] legendRect Bounding rectangle for the legend
 * \endif
 *
 * \if CHINESE
 * @brief 将图例渲染到给定矩形中
 * @param[in] plot 绘图控件
 * @param[in] painter 绘制器
 * @param[in] legendRect 图例的边界矩形
 * \endif
 */
void QwtPlotRenderer::renderLegend(const QwtPlot* plot, QPainter* painter, const QRectF& legendRect) const
{
    if (plot->legend()) {
        bool fillBackground = !(m_data->discardFlags & DiscardBackground);
        plot->legend()->renderLegend(painter, legendRect, fillBackground);
    }
}

/**
 * \if ENGLISH
 * @brief Paint a scale into a given rectangle
 * @details Render the scale into a given rectangle.
 * @param[in] plot Plot widget
 * @param[in] painter Painter
 * @param[in] axisId Axis identifier
 * @param[in] startDist Start border distance
 * @param[in] endDist End border distance
 * @param[in] baseDist Base distance
 * @param[in] scaleRect Bounding rectangle for the scale
 * \endif
 *
 * \if CHINESE
 * @brief 将比例尺绘制到给定矩形中
 * @details 将比例尺渲染到给定矩形。
 * @param[in] plot 绘图控件
 * @param[in] painter 绘制器
 * @param[in] axisId 坐标轴标识
 * @param[in] startDist 起始边距离
 * @param[in] endDist 结束边距离
 * @param[in] baseDist 基础距离
 * @param[in] scaleRect 比例尺的边界矩形
 * \endif
 */
void QwtPlotRenderer::renderScale(const QwtPlot* plot,
                                  QPainter* painter,
                                  QwtAxisId axisId,
                                  int startDist,
                                  int endDist,
                                  int baseDist,
                                  const QRectF& scaleRect) const
{
    if (!plot->isAxisVisible(axisId))
        return;

    const QwtScaleWidget* scaleWidget = plot->axisWidget(axisId);
    if (scaleWidget->isColorBarEnabled() && scaleWidget->colorBarWidth() > 0) {
        scaleWidget->drawColorBar(painter, scaleWidget->colorBarRect(scaleRect));
        baseDist += scaleWidget->colorBarWidth() + scaleWidget->spacing();
    }

    painter->save();

    QwtScaleDraw::Alignment align;
    double x, y, w;

    qreal off = 0.0;
    if (m_data->layoutFlags & FrameWithScales)
        off = qwtScalePenWidth(plot);

    switch (axisId) {
    case QwtAxis::YLeft: {
        x     = scaleRect.right() - 1.0 - baseDist - off;
        y     = scaleRect.y() + startDist;
        w     = scaleRect.height() - startDist - endDist;
        align = QwtScaleDraw::LeftScale;
        break;
    }
    case QwtAxis::YRight: {
        x     = scaleRect.left() + baseDist + off;
        y     = scaleRect.y() + startDist;
        w     = scaleRect.height() - startDist - endDist;
        align = QwtScaleDraw::RightScale;
        break;
    }
    case QwtAxis::XTop: {
        x     = scaleRect.left() + startDist;
        y     = scaleRect.bottom() - 1.0 - baseDist - off;
        w     = scaleRect.width() - startDist - endDist;
        align = QwtScaleDraw::TopScale;
        break;
    }
    case QwtAxis::XBottom: {
        x     = scaleRect.left() + startDist;
        y     = scaleRect.top() + baseDist + off;
        w     = scaleRect.width() - startDist - endDist;
        align = QwtScaleDraw::BottomScale;
        break;
    }
    default:
        return;
    }

    scaleWidget->drawTitle(painter, align, scaleRect);

    painter->setFont(qwtResolvedFont(scaleWidget));

    QwtScaleDraw* sd      = const_cast< QwtScaleDraw* >(scaleWidget->scaleDraw());
    const QPointF sdPos   = sd->pos();
    const double sdLength = sd->length();

    const bool hasBackbone = sd->hasComponent(QwtAbstractScaleDraw::Backbone);

    if (m_data->layoutFlags & FrameWithScales)
        sd->enableComponent(QwtAbstractScaleDraw::Backbone, false);

    sd->move(x, y);
    sd->setLength(w);

    QPalette palette = scaleWidget->palette();
    palette.setCurrentColorGroup(QPalette::Active);
    sd->draw(painter, palette);

    // reset previous values
    sd->move(sdPos);
    sd->setLength(sdLength);
    sd->enableComponent(QwtAbstractScaleDraw::Backbone, hasBackbone);

    painter->restore();
}

/**
 * \if ENGLISH
 * @brief Render the canvas into a given rectangle
 * @param[in] plot Plot widget
 * @param[in] painter Painter
 * @param[in] canvasRect Canvas rectangle
 * @param[in] maps Maps mapping between plot and paint device coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 将画布渲染到给定矩形中
 * @param[in] plot 绘图控件
 * @param[in] painter 绘制器
 * @param[in] canvasRect 画布矩形
 * @param[in] maps 绘图与绘制设备坐标之间的映射
 * \endif
 */
void QwtPlotRenderer::renderCanvas(const QwtPlot* plot, QPainter* painter, const QRectF& canvasRect, const QwtScaleMap* maps) const
{
    const QWidget* canvas = plot->canvas();

    QRectF r = canvasRect.adjusted(0.0, 0.0, -1.0, -1.0);

    if (m_data->layoutFlags & FrameWithScales) {
        painter->save();

        QPen pen;
        pen.setColor(qwtScalePenColor(plot));
        pen.setWidth(qwtScalePenWidth(plot));
        pen.setJoinStyle(Qt::MiterJoin);

        painter->setPen(pen);

        const qreal pw2 = 0.5 * pen.widthF();
        r.adjust(-pw2, -pw2, pw2, pw2);

        if (!(m_data->discardFlags & DiscardCanvasBackground)) {
            const QBrush bgBrush = canvas->palette().brush(plot->backgroundRole());
            painter->setBrush(bgBrush);
        }

        QwtPainter::drawRect(painter, r);

        painter->restore();
        painter->save();

        painter->setClipRect(canvasRect);
        plot->drawItems(painter, canvasRect, maps);

        painter->restore();
    } else if (canvas->testAttribute(Qt::WA_StyledBackground)) {
        QPainterPath clipPath;

        painter->save();

        if (!(m_data->discardFlags & DiscardCanvasBackground)) {
            QwtPainter::drawBackgound(painter, r, canvas);
            clipPath = qwtCanvasClip(canvas, canvasRect);
        }

        painter->restore();
        painter->save();

        if (clipPath.isEmpty())
            painter->setClipRect(canvasRect);
        else
            painter->setClipPath(clipPath);

        plot->drawItems(painter, canvasRect, maps);

        painter->restore();
    } else {
        QPainterPath clipPath;

        double frameWidth = 0.0;

        if (!(m_data->discardFlags & DiscardCanvasFrame)) {
            const QVariant fw = canvas->property("frameWidth");
            if (fw.canConvert< double >())
                frameWidth = fw.value< double >();

            clipPath = qwtCanvasClip(canvas, canvasRect);
        }

        QRectF innerRect = canvasRect.adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);

        painter->save();

        if (clipPath.isEmpty()) {
            painter->setClipRect(innerRect);
        } else {
            painter->setClipPath(clipPath);
        }

        if (!(m_data->discardFlags & DiscardCanvasBackground)) {
            QwtPainter::drawBackgound(painter, innerRect, canvas);
        }

        plot->drawItems(painter, innerRect, maps);

        painter->restore();

        if (frameWidth > 0) {
            painter->save();

            const int frameStyle = canvas->property("frameShadow").toInt() | canvas->property("frameShape").toInt();

            const QVariant borderRadius = canvas->property("borderRadius");
            if (borderRadius.canConvert< double >() && borderRadius.value< double >() > 0.0) {
                const double radius = borderRadius.value< double >();

                QwtPainter::drawRoundedFrame(painter, canvasRect, radius, radius, canvas->palette(), frameWidth, frameStyle);
            } else {
                const int midLineWidth = canvas->property("midLineWidth").toInt();

                QwtPainter::drawFrame(painter,
                                      canvasRect,
                                      canvas->palette(),
                                      canvas->foregroundRole(),
                                      frameWidth,
                                      midLineWidth,
                                      frameStyle);
            }
            painter->restore();
        }
    }
}

/*!
   Calculated the scale maps for rendering the canvas

   \param plot Plot widget
   \param canvasRect Target rectangle
   \param maps Scale maps to be calculated
 */
void QwtPlotRenderer::buildCanvasMaps(const QwtPlot* plot, const QRectF& canvasRect, QwtScaleMap maps[]) const
{
    for (int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++) {
        {
            const QwtAxisId axisId(axisPos);

            QwtScaleMap& scaleMap = maps[ axisId ];

            scaleMap.setTransformation(plot->axisScaleEngine(axisId)->transformation());

            const QwtScaleDiv& scaleDiv = plot->axisScaleDiv(axisId);
            scaleMap.setScaleInterval(scaleDiv.lowerBound(), scaleDiv.upperBound());

            double from, to;
            if (plot->isAxisVisible(axisId)) {
                const int sDist        = plot->axisWidget(axisId)->startBorderDist();
                const int eDist        = plot->axisWidget(axisId)->endBorderDist();
                const QRectF scaleRect = plot->plotLayout()->scaleRect(axisId);

                if (QwtAxis::isXAxis(axisPos)) {
                    from = scaleRect.left() + sDist;
                    to   = scaleRect.right() - eDist;
                } else {
                    from = scaleRect.bottom() - eDist;
                    to   = scaleRect.top() + sDist;
                }
            } else {
                int margin = 0;
                if (!plot->plotLayout()->alignCanvasToScale(axisPos))
                    margin = plot->plotLayout()->canvasMargin(axisPos);

                if (QwtAxis::isYAxis(axisPos)) {
                    from = canvasRect.bottom() - margin;
                    to   = canvasRect.top() + margin;
                } else {
                    from = canvasRect.left() + margin;
                    to   = canvasRect.right() - margin;
                }
            }
            scaleMap.setPaintInterval(from, to);
        }
    }
}

bool QwtPlotRenderer::updateCanvasMargins(QwtPlot* plot, const QRectF& canvasRect, const QwtScaleMap maps[]) const
{
    using namespace QwtAxis;

    double margins[ AxisPositions ];
    plot->getCanvasMarginsHint(maps, canvasRect, margins[ YLeft ], margins[ XTop ], margins[ YRight ], margins[ XBottom ]);

    bool marginsChanged = false;
    for (int axisId = 0; axisId < AxisPositions; axisId++) {
        if (margins[ axisId ] >= 0.0) {
            const int m = qwtCeil(margins[ axisId ]);
            plot->plotLayout()->setCanvasMargin(m, axisId);
            marginsChanged = true;
        }
    }

    return marginsChanged;
}

/**
 * \if ENGLISH
 * @brief Execute a file dialog and render the plot to the selected file
 * @param[in] plot Plot widget
 * @param[in] documentName Default document name
 * @param[in] sizeMM Size for the document in millimeters
 * @param[in] resolution Resolution in dots per Inch (dpi)
 * @return True when exporting was successful
 * @sa renderDocument()
 * \endif
 *
 * \if CHINESE
 * @brief 执行文件对话框并将绘图渲染到选定文件
 * @param[in] plot 绘图控件
 * @param[in] documentName 默认文档名称
 * @param[in] sizeMM 文档尺寸（毫米）
 * @param[in] resolution 分辨率（点/英寸）
 * @return 导出成功返回 true
 * @sa renderDocument()
 * \endif
 */
bool QwtPlotRenderer::exportTo(QwtPlot* plot, const QString& documentName, const QSizeF& sizeMM, int resolution)
{
    if (plot == nullptr)
        return false;

    QString fileName = documentName;

    // What about translation

#ifndef QT_NO_FILEDIALOG
    const QList< QByteArray > imageFormats = QImageWriter::supportedImageFormats();

    QStringList filter;
#if QWT_FORMAT_PDF
    filter += QString("PDF ") + tr("Documents") + " (*.pdf)";
#endif
#if QWT_FORMAT_SVG
    filter += QString("SVG ") + tr("Documents") + " (*.svg)";
#endif
#if QWT_FORMAT_POSTSCRIPT
    filter += QString("Postscript ") + tr("Documents") + " (*.ps)";
#endif

    if (imageFormats.size() > 0) {
        QString imageFilter(tr("Images"));
        imageFilter += " (";
        for (int i = 0; i < imageFormats.size(); i++) {
            if (i > 0)
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[ i ];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(nullptr,
                                            tr("Export File Name"),
                                            fileName,
                                            filter.join(";;"),
                                            nullptr,
                                            QFileDialog::DontConfirmOverwrite);
#endif
    if (fileName.isEmpty())
        return false;

    renderDocument(plot, fileName, sizeMM, resolution);

    return true;
}
