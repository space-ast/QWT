/**
 * Legend Icon Debug Test
 *
 * This program tests whether QwtGraphic correctly records and replays painter
 * commands on both Qt5 and Qt6.  It mimics exactly what QwtPlotCurve::legendIcon()
 * does and reports the graphic state at every step.
 *
 * The key symptom: on Qt6, legend icons are blank because
 *   QwtNullPaintDevice::metric() returned 0 for PdmDevicePixelRatio /
 *   PdmDevicePixelRatioScaled.  Qt6 used this to set up a scale(0,0) initial
 *   transform, making every drawn path map to a zero-size bounding rect,
 *   leaving QwtGraphic::isEmpty() == true.
 *
 * Build with -DQWT_GRAPHIC_DEBUG (already set in CMakeLists.txt) to see the
 * per-path transform info from qwt_graphic.cpp.
 */

#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QPainter>

#include "qwt_graphic.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_legenditem.h"
#include "qwt_legend.h"

// --------------------------------------------------------------------------
// Helper: dump graphic state to qDebug
// --------------------------------------------------------------------------
static void dumpGraphic(const char* label, const QwtGraphic& g)
{
    qDebug() << "======" << label << "======";
    qDebug() << "  isNull()       :" << g.isNull();
    qDebug() << "  isEmpty()      :" << g.isEmpty();
    qDebug() << "  commands count :" << g.commands().size();
    qDebug() << "  defaultSize()  :" << g.defaultSize();
    qDebug() << "  boundingRect() :" << g.boundingRect();
    qDebug() << "  controlPointRect():" << g.controlPointRect();
}

// --------------------------------------------------------------------------
// Step 1: raw QwtGraphic test (no QwtPlot involved)
// --------------------------------------------------------------------------
static void testRawGraphic()
{
    qDebug() << "\n========== testRawGraphic ==========";
    qDebug() << "Qt version:" << QT_VERSION_STR;

    QSizeF iconSize(8, 8);

    QwtGraphic graphic;
    graphic.setDefaultSize(iconSize);
    graphic.setRenderHint(QwtGraphic::RenderPensUnscaled, true);

    {
        QPainter painter(&graphic);
        qDebug() << "  painter.device()->devicePixelRatioF() =" << painter.device()->devicePixelRatioF();
        qDebug() << "  painter.transform() (initial) =" << painter.transform();

        // Simulate what QwtPlotCurve::legendIcon() does
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Fill with solid color (LegendShowBrush path)
        QBrush brush(Qt::blue);
        QRectF r(0, 0, iconSize.width(), iconSize.height());
        painter.fillRect(r, brush);

        // Draw a line (LegendShowLine path)
        QPen pen(Qt::red, 1);
        pen.setCapStyle(Qt::FlatCap);
        painter.setPen(pen);
        const double y = 0.5 * iconSize.height();
        painter.drawLine(QLineF(0.0, y, iconSize.width(), y));

        painter.end();
    }

    dumpGraphic("after recording", graphic);

    // Try rendering to a QImage
    QImage img(20, 20, QImage::Format_ARGB32);
    img.fill(Qt::white);
    {
        QPainter p(&img);
        graphic.render(&p, QRectF(0, 0, 20, 20), Qt::KeepAspectRatio);
    }

    // Check if image has any non-white pixel
    bool hasContent = false;
    for (int y = 0; y < img.height() && !hasContent; ++y)
        for (int x = 0; x < img.width() && !hasContent; ++x)
            if (img.pixel(x, y) != qRgb(255, 255, 255))
                hasContent = true;

    qDebug() << "  Rendered image has non-white pixels:" << hasContent;
    if (!hasContent)
        qDebug() << "  *** BUG: icon is blank! Rendering produced no output. ***";
    else
        qDebug() << "  OK: icon rendered correctly.";
}

// --------------------------------------------------------------------------
// Step 2: full QwtPlot + QwtPlotLegendItem test (visual window)
// --------------------------------------------------------------------------
static void testWithPlot(QApplication& app)
{
    Q_UNUSED(app);

    qDebug() << "\n========== testWithPlot ==========";

    QwtPlot plot;
    plot.setTitle("Legend Icon Test");
    plot.resize(500, 400);

    // Attach an in-canvas legend
    QwtPlotLegendItem* legendItem = new QwtPlotLegendItem();
    legendItem->attach(&plot);
    legendItem->setMaxColumns(1);
    legendItem->setBorderRadius(4);
    legendItem->setMargin(4);
    legendItem->setSpacing(2);
    legendItem->setItemMargin(0);
    legendItem->setFont(QFont("Sans", 9));
    QColor c(Qt::black);
    c.setAlpha(200);
    legendItem->setBackgroundBrush(QBrush(c));
    legendItem->setTextPen(QPen(Qt::white));

    // Add a simple curve
    QwtPlotCurve* curve = new QwtPlotCurve("Test Curve");
    curve->setPen(Qt::cyan, 2);
    QVector< QPointF > pts;
    for (int i = 0; i <= 10; ++i)
        pts << QPointF(i, i * i);
    curve->setSamples(pts);
    curve->attach(&plot);

    plot.replot();
    plot.show();

    qDebug() << "  Plot shown. Check if legend icon (colored line) is visible.";
    qDebug() << "  Close the window to exit.";
}

// --------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Step 1: non-visual, pure logic test
    testRawGraphic();

    // Step 2: visual test – shows a plot window
    testWithPlot(app);

    return app.exec();
}
