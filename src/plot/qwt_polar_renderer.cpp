/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_renderer.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_layout.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_text_label.h"
#include "qwt_text.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qfiledialog.h>
#include <qimagewriter.h>
#include <qfileinfo.h>
#include <qmath.h>

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

class QwtPolarRenderer::PrivateData
{
public:
    PrivateData() : plot(nullptr)
    {
    }

    QwtPolarPlot* plot;
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
QwtPolarRenderer::QwtPolarRenderer(QObject* parent) : QObject(parent)
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
QwtPolarRenderer::~QwtPolarRenderer()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Render a polar plot to a file
 * @details The format of the document will be autodetected from the suffix of the filename.
 * @param[in] plot Plot widget
 * @param[in] fileName Path of the file, where the document will be stored
 * @param[in] sizeMM Size for the document in millimeters
 * @param[in] resolution Resolution in dots per Inch (dpi)
 * \endif
 *
 * \if CHINESE
 * @brief 将极坐标图渲染到文件
 * @details 文档格式将从文件名的后缀自动检测。
 * @param[in] plot 绘图控件
 * @param[in] fileName 文档存储的文件路径
 * @param[in] sizeMM 文档大小（毫米）
 * @param[in] resolution 分辨率（每英寸点数 dpi）
 * \endif
 */
void QwtPolarRenderer::renderDocument(QwtPolarPlot* plot, const QString& fileName, const QSizeF& sizeMM, int resolution)
{
    renderDocument(plot, fileName, QFileInfo(fileName).suffix(), sizeMM, resolution);
}

/**
 * \if ENGLISH
 * @brief Render a plot to a file with specified format
 * @details Supported formats are: pdf, ps, svg, and all image formats supported by Qt.
 * @param[in] plot Plot widget
 * @param[in] fileName Path of the file, where the document will be stored
 * @param[in] format Format for the document
 * @param[in] sizeMM Size for the document in millimeters
 * @param[in] resolution Resolution in dots per Inch (dpi)
 * @sa renderTo(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到指定格式的文件
 * @details 支持的格式：pdf、ps、svg 以及 Qt 支持的所有图像格式。
 * @param[in] plot 绘图控件
 * @param[in] fileName 文档存储的文件路径
 * @param[in] format 文档格式
 * @param[in] sizeMM 文档大小（毫米）
 * @param[in] resolution 分辨率（每英寸点数 dpi）
 * @sa renderTo(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 */
void QwtPolarRenderer::renderDocument(QwtPolarPlot* plot,
                                      const QString& fileName,
                                      const QString& format,
                                      const QSizeF& sizeMM,
                                      int resolution)
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
    if (format == "pdf") {
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
    } else if (format == "ps") {
#if QWT_FORMAT_POSTSCRIPT
        QPrinter printer;
        printer.setColorMode(QPrinter::Color);
        printer.setFullPage(true);
        printer.setPaperSize(sizeMM, QPrinter::Millimeter);
        printer.setDocName(title);
        printer.setOutputFileName(fileName);
        printer.setOutputFormat(QPrinter::PostScriptFormat);
        printer.setResolution(resolution);

        QPainter painter(&printer);
        render(plot, &painter, documentRect);
#endif
    } else if (format == "svg") {
#ifdef QWT_FORMAT_SVG
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
 * @details This function renders the contents of a QwtPolarPlot instance to
 *          QPaintDevice object. The target rectangle is derived from its device metrics.
 * @param[in] plot Plot to be rendered
 * @param[in] paintDevice Device to paint on, e.g. a QImage
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到 QPaintDevice
 * @details 此函数将 QwtPolarPlot 实例的内容渲染到 QPaintDevice 对象。
 *          目标矩形从其设备度量中派生。
 * @param[in] plot 要渲染的绘图
 * @param[in] paintDevice 用于绘制的设备，例如 QImage
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 */
void QwtPolarRenderer::renderTo(QwtPolarPlot* plot, QPaintDevice& paintDevice) const
{
    int w = paintDevice.width();
    int h = paintDevice.height();

    QPainter p(&paintDevice);
    render(plot, &p, QRectF(0, 0, w, h));
}

/**
 * \if ENGLISH
 * @brief Render the plot to a QPrinter
 * @details This function renders the contents of a QwtPolarPlot instance to
 *          QPaintDevice object. The size is derived from the printer metrics.
 * @param[in] plot Plot to be rendered
 * @param[in] printer Printer to paint on
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到打印机
 * @details 此函数将 QwtPolarPlot 实例的内容渲染到 QPaintDevice 对象。
 *          大小从打印机度量中派生。
 * @param[in] plot 要渲染的绘图
 * @param[in] printer 用于绘制的打印机
 * @sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
 * \endif
 */
#ifndef QT_NO_PRINTER
void QwtPolarRenderer::renderTo(QwtPolarPlot* plot, QPrinter& printer) const
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

#ifdef QWT_FORMAT_SVG
/**
 * \if ENGLISH
 * @brief Render the plot to a QSvgGenerator
 * @details If the generator has a view box, the plot will be rendered into it.
 *          If it has no viewBox but a valid size the target coordinates will be
 *          (0, 0, generator.width(), generator.height()). Otherwise the target
 *          rectangle will be QRectF(0, 0, 800, 600).
 * @param[in] plot Plot to be rendered
 * @param[in] generator SVG generator
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到 SVG 生成器
 * @details 如果生成器有视图框，绘图将渲染到其中。如果没有 viewBox 但有有效大小，
 *          目标坐标将是 (0, 0, generator.width(), generator.height())。
 *          否则目标矩形将是 QRectF(0, 0, 800, 600)。
 * @param[in] plot 要渲染的绘图
 * @param[in] generator SVG 生成器
 * \endif
 */
void QwtPolarRenderer::renderTo(QwtPolarPlot* plot, QSvgGenerator& generator) const
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
 * @brief Render the plot to a given rectangle
 * @details Renders the plot to the specified rectangle on the given painter.
 * @param[in] plot Plot widget to be rendered
 * @param[in] painter Painter
 * @param[in] plotRect Bounding rectangle for the plot
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图渲染到指定矩形
 * @details 将绘图渲染到指定绘图设备上的指定矩形区域。
 * @param[in] plot 要渲染的绘图控件
 * @param[in] painter 绘制器
 * @param[in] plotRect 绘图的边界矩形
 * \endif
 */
void QwtPolarRenderer::render(QwtPolarPlot* plot, QPainter* painter, const QRectF& plotRect) const
{
    if (plot == nullptr || painter == nullptr || !painter->isActive() || !plotRect.isValid() || plot->size().isNull()) {
        return;
    }

    m_data->plot = plot;

    /*
       The layout engine uses the same methods as they are used
       by the Qt layout system. Therefore we need to calculate the
       layout in screen coordinates and paint with a scaled painter.
     */
    QTransform transform;
    transform.scale(double(painter->device()->logicalDpiX()) / plot->logicalDpiX(),
                    double(painter->device()->logicalDpiY()) / plot->logicalDpiY());

    const QRectF layoutRect = transform.inverted().mapRect(plotRect);

    QwtPolarLayout* layout = plot->plotLayout();

    // All paint operations need to be scaled according to
    // the paint device metrics.

    QwtPolarLayout::Options layoutOptions = QwtPolarLayout::IgnoreScrollbars | QwtPolarLayout::IgnoreFrames;

    layout->activate(plot, layoutRect, layoutOptions);

    painter->save();
    painter->setWorldTransform(transform, true);

    painter->save();
    renderTitle(painter, layout->titleRect());
    painter->restore();

    painter->save();
    renderLegend(plot, painter, layout->legendRect());
    painter->restore();

    const QRectF canvasRect = layout->canvasRect();

    painter->save();
    painter->setClipRect(canvasRect);
    plot->drawCanvas(painter, canvasRect);
    painter->restore();

    painter->restore();

    layout->invalidate();

    m_data->plot = nullptr;
}

/**
 * \if ENGLISH
 * @brief Render the title into a given rectangle
 * @param[in] painter Painter
 * @param[in] rect Bounding rectangle
 * \endif
 *
 * \if CHINESE
 * @brief 将标题渲染到指定矩形
 * @param[in] painter 绘制器
 * @param[in] rect 边界矩形
 * \endif
 */
void QwtPolarRenderer::renderTitle(QPainter* painter, const QRectF& rect) const
{
    QwtTextLabel* title = m_data->plot->titleLabel();

    painter->setFont(title->font());

    const QColor color = title->palette().color(QPalette::Active, QPalette::Text);

    painter->setPen(color);
    title->text().draw(painter, rect);
}

/**
 * \if ENGLISH
 * @brief Render the legend into a given rectangle
 * @param[in] plot Plot widget
 * @param[in] painter Painter
 * @param[in] rect Bounding rectangle
 * \endif
 *
 * \if CHINESE
 * @brief 将图例渲染到指定矩形
 * @param[in] plot 绘图控件
 * @param[in] painter 绘制器
 * @param[in] rect 边界矩形
 * \endif
 */
void QwtPolarRenderer::renderLegend(const QwtPolarPlot* plot, QPainter* painter, const QRectF& rect) const
{
    if (plot->legend())
        plot->legend()->renderLegend(painter, rect, true);
}

/**
 * \if ENGLISH
 * @brief Execute a file dialog and render the plot to the selected file
 * @details The document will be rendered in 85 dpi for a size 30x30 cm.
 * @param[in] plot Plot widget
 * @param[in] documentName Default document name
 * @param[in] sizeMM Size for the document in millimeters
 * @param[in] resolution Resolution in dots per Inch (dpi)
 * @return True if export was successful, false otherwise
 * @sa renderDocument()
 * \endif
 *
 * \if CHINESE
 * @brief 执行文件对话框并将绘图渲染到选定文件
 * @details 文档将以85 dpi渲染，大小为30x30厘米。
 * @param[in] plot 绘图控件
 * @param[in] documentName 默认文档名称
 * @param[in] sizeMM 文档大小（毫米）
 * @param[in] resolution 分辨率（每英寸点数 dpi）
 * @return 导出成功返回 true，否则返回 false
 * @sa renderDocument()
 * \endif
 */
bool QwtPolarRenderer::exportTo(QwtPolarPlot* plot, const QString& documentName, const QSizeF& sizeMM, int resolution)
{
    if (plot == nullptr)
        return false;

    QString fileName = documentName;

    // What about translation

#ifndef QT_NO_FILEDIALOG
    const QList< QByteArray > imageFormats = QImageWriter::supportedImageFormats();

    QStringList filter;
#ifndef QT_NO_PRINTER
    filter += QString("PDF ") + tr("Documents") + " (*.pdf)";
#endif
#ifndef QWT_NO_SVG
    filter += QString("SVG ") + tr("Documents") + " (*.svg)";
#endif
#ifndef QT_NO_PRINTER
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
