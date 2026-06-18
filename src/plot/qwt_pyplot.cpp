/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_pyplot.h"

#include <qmath.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <qpoint.h>
#include <qrect.h>

#include <algorithm>
#include <cmath>

#include "qwt_figure.h"
#include "qwt_plot.h"
#include "qwt_plot_item.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_barchart.h"
#include "qwt_plot_histogram.h"
#include "qwt_plot_boxchart.h"
#include "qwt_plot_intervalcurve.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_vectorfield.h"
#include "qwt_plot_tradingcurve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_plot_arrowmarker.h"
#include "qwt_plot_legenditem.h"
#include "qwt_plot_factory.h"
#include "qwt_plot_styling.h"
#include "qwt_matrix_raster_data.h"
#include "qwt_color_map.h"
#include "qwt_interval.h"
#include "qwt_samples.h"
#include "qwt_symbol.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_canvas_zoomer.h"

// ============================================================================
// QwtFormatString implementation
// ============================================================================

/**
 * @brief Parse a matplotlib-style format string
 * @param fmt Format string (e.g. "r-o", "b--", "g^:", "ko")
 * @return Parsed format structure
 *
 * @details Supported components:
 *   - Colors: b(blue), g(green), r(red), c(cyan), m(magenta), y(yellow), k(black), w(white)
 *   - Markers: o(circle), s(square), ^(triangle up), v(triangle down), D(diamond),
 *              x(cross), +(plus), *(star), .(dot)
 *   - Line styles: -(solid), --(dash), -.(dashdot), :(dot)
 */
QwtFormatString QwtFormatString::parse(const QString& fmt)
{
    QwtFormatString result;
    if (fmt.isEmpty()) {
        return result;
    }

    QString s = fmt.trimmed();
    int i     = 0;

    // Parse color (single character at start)
    if (i < s.length()) {
        QChar ch = s[ i ];
        QColor c;
        switch (ch.toLatin1()) {
        case 'b':
            c = QColor(Qt::blue);
            break;
        case 'g':
            c = QColor(Qt::green);
            break;
        case 'r':
            c = QColor(Qt::red);
            break;
        case 'c':
            c = QColor(Qt::cyan);
            break;
        case 'm':
            c = QColor(Qt::magenta);
            break;
        case 'y':
            c = QColor(Qt::yellow);
            break;
        case 'k':
            c = QColor(Qt::black);
            break;
        case 'w':
            c = QColor(Qt::white);
            break;
        default:
            break;
        }
        if (c.isValid()) {
            result.color    = c;
            result.hasColor = true;
            i++;
        }
    }

    // Parse line style (must check before marker since '-' can be ambiguous)
    if (i < s.length()) {
        if (s.mid(i).startsWith("--")) {
            result.lineStyle    = Qt::DashLine;
            result.hasLineStyle = true;
            i += 2;
        } else if (s.mid(i).startsWith("-.")) {
            result.lineStyle    = Qt::DashDotLine;
            result.hasLineStyle = true;
            i += 2;
        } else if (i < s.length() && s[ i ] == '-') {
            // Single dash - could be solid line or part of marker context
            // Check if next char is a marker or end of string
            if (i + 1 >= s.length()) {
                result.lineStyle    = Qt::SolidLine;
                result.hasLineStyle = true;
                i++;
            } else {
                QChar next = s[ i + 1 ];
                if (next == '-' || next == '.') {
                    // Already handled above, shouldn't reach here
                    i++;
                } else {
                    result.lineStyle    = Qt::SolidLine;
                    result.hasLineStyle = true;
                    i++;
                }
            }
        } else if (i < s.length() && s[ i ] == ':') {
            result.lineStyle    = Qt::DotLine;
            result.hasLineStyle = true;
            i++;
        }
    }

    // Parse marker
    if (i < s.length()) {
        QChar ch                = s[ i ];
        QwtSymbol::Style marker = QwtSymbol::NoSymbol;
        switch (ch.toLatin1()) {
        case 'o':
            marker = QwtSymbol::Ellipse;
            break;
        case 's':
            marker = QwtSymbol::Rect;
            break;
        case '^':
            marker = QwtSymbol::Triangle;
            break;
        case 'v':
            marker = QwtSymbol::DTriangle;
            break;
        case 'D':
        case 'd':
            marker = QwtSymbol::Diamond;
            break;
        case 'x':
            marker = QwtSymbol::XCross;
            break;
        case '+':
            marker = QwtSymbol::Cross;
            break;
        case '*':
            marker = QwtSymbol::Star1;
            break;
        case '.':
            marker = QwtSymbol::Ellipse;
            break;
        default:
            break;
        }
        if (marker != QwtSymbol::NoSymbol) {
            result.marker    = marker;
            result.hasMarker = true;
            i++;
        }
    }

    // If only marker specified without line style, suppress the line
    // (matplotlib behavior: "ro" = red circles, no line)
    if (result.hasMarker && !result.hasLineStyle) {
        result.noLine = true;
    }

    return result;
}

// ============================================================================
// QwtPyPlot::PrivateData
// ============================================================================

class QwtPyPlot::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPyPlot)

public:
    PrivateData(QwtPyPlot* p);

    QwtFigure* figure { nullptr };
    QwtPlot* currentAxes { nullptr };
    QwtPlotPanner* panner { nullptr };
    QwtPlotCanvasZoomer* zoomer { nullptr };
};

QwtPyPlot::PrivateData::PrivateData(QwtPyPlot* p) : q_ptr(p)
{
}

// ============================================================================
// QwtPyPlot implementation
// ============================================================================

/**
 * @brief Construct a QwtPyPlot from a QwtFigure
 * @param figure The figure to operate on
 * @param parent Parent QObject
 */
QwtPyPlot::QwtPyPlot(QwtFigure* figure, QObject* parent) : QObject(parent), QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    d->figure = figure;
    if (figure) {
        QList< QwtPlot* > axes = figure->allAxes();
        if (!axes.isEmpty()) {
            d->currentAxes = figure->gca();
            if (!d->currentAxes) {
                d->currentAxes = axes.first();
            }
        }
    }
}

/**
 * @brief Construct a QwtPyPlot from a single QwtPlot
 * @param plot The plot to operate on
 * @param parent Parent QObject
 */
QwtPyPlot::QwtPyPlot(QwtPlot* plot, QObject* parent) : QObject(parent), QWT_PIMPL_CONSTRUCT
{
    QWT_D(d);
    d->figure      = nullptr;
    d->currentAxes = plot;
}

QwtPyPlot::~QwtPyPlot() = default;

// ---- State management ----

QwtFigure* QwtPyPlot::gcf() const
{
    QWT_DC(d);
    return d->figure;
}

QwtPlot* QwtPyPlot::gca() const
{
    QWT_DC(d);
    if (d->currentAxes) {
        return d->currentAxes;
    }
    if (d->figure) {
        return const_cast< QwtPyPlot* >(this)->subplot(1, 1, 1);
    }
    return nullptr;
}

void QwtPyPlot::sca(QwtPlot* plot)
{
    QWT_D(d);
    d->currentAxes = plot;
    if (d->figure && plot) {
        d->figure->setCurrentAxes(plot);
    }
}

// ---- Figure operations ----

/**
 * @brief Create a subplot in a grid layout
 * @param rows Number of rows in the grid
 * @param cols Number of columns in the grid
 * @param index 1-based index of the subplot (row-major order)
 * @return The newly created QwtPlot
 */
QwtPlot* QwtPyPlot::subplot(int rows, int cols, int index)
{
    QWT_D(d);
    if (!d->figure) {
        qWarning() << "QwtPyPlot::subplot: no figure available";
        return nullptr;
    }
    if (index < 1 || index > rows * cols) {
        qWarning() << "QwtPyPlot::subplot: index out of range";
        return nullptr;
    }

    int row = (index - 1) / cols;
    int col = (index - 1) % cols;

    auto* plot = new QwtPlot;
    d->figure->addGridAxes(plot, rows, cols, row, col);
    d->figure->setCurrentAxes(plot);
    d->currentAxes = plot;
    return plot;
}

QwtPlot* QwtPyPlot::addAxes(const QRectF& rect)
{
    QWT_D(d);
    if (!d->figure) {
        qWarning() << "QwtPyPlot::addAxes: no figure available";
        return nullptr;
    }
    auto* plot = new QwtPlot;
    d->figure->addAxes(plot, rect);
    d->figure->setCurrentAxes(plot);
    d->currentAxes = plot;
    return plot;
}

QwtPlot* QwtPyPlot::twinx(QwtPlot* host)
{
    QWT_D(d);
    if (!d->figure) {
        qWarning() << "QwtPyPlot::twinx: no figure available";
        return nullptr;
    }
    QwtPlot* h = host ? host : d->currentAxes;
    if (!h) {
        qWarning() << "QwtPyPlot::twinx: no host plot available";
        return nullptr;
    }
    QwtPlot* parasite = d->figure->createParasiteAxes(h, QwtAxis::YRight);
    if (parasite) {
        parasite->setParasiteShareAxis(QwtAxis::XBottom, true);
        d->currentAxes = parasite;
    }
    return parasite;
}

QwtPlot* QwtPyPlot::twiny(QwtPlot* host)
{
    QWT_D(d);
    if (!d->figure) {
        qWarning() << "QwtPyPlot::twiny: no figure available";
        return nullptr;
    }
    QwtPlot* h = host ? host : d->currentAxes;
    if (!h) {
        qWarning() << "QwtPyPlot::twiny: no host plot available";
        return nullptr;
    }
    QwtPlot* parasite = d->figure->createParasiteAxes(h, QwtAxis::XTop);
    if (parasite) {
        parasite->setParasiteShareAxis(QwtAxis::YLeft, true);
        d->currentAxes = parasite;
    }
    return parasite;
}

void QwtPyPlot::tightLayout()
{
    QWT_D(d);
    if (!d->figure) {
        return;
    }
    // Align Y-axes across all plots
    QList< QwtPlot* > axes = d->figure->allAxes();
    if (axes.size() > 1) {
        d->figure->addAxisAlignment(axes, QwtAxis::YLeft);
        d->figure->applyAllAxisAlignments(true);
    }
}

// ---- Plotting methods ----

QwtPlotCurve* QwtPyPlot::plot(const QVector< double >& y, const QString& fmt, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* curve = QwtPlotFactory::createCurve(ax, label, y);
    if (!fmt.isEmpty()) {
        applyFormat(curve, QwtFormatString::parse(fmt));
    }
    ax->replot();
    return curve;
}

QwtPlotCurve* QwtPyPlot::plot(const QVector< double >& x, const QVector< double >& y, const QString& fmt, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* curve = QwtPlotFactory::createCurve(ax, label, x, y);
    if (!fmt.isEmpty()) {
        applyFormat(curve, QwtFormatString::parse(fmt));
    }
    ax->replot();
    return curve;
}

QwtPlotCurve* QwtPyPlot::plot(const QVector< QPointF >& data, const QString& fmt, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* curve = QwtPlotFactory::createCurve(ax, label, data);
    if (!fmt.isEmpty()) {
        applyFormat(curve, QwtFormatString::parse(fmt));
    }
    ax->replot();
    return curve;
}

/**
 * @brief Create a scatter plot (markers only, no lines)
 * @param x X coordinates
 * @param y Y coordinates
 * @param size Marker size in points
 * @param color Color name (e.g. "r", "blue", "#ff0000")
 * @param label Legend label
 * @return The created curve
 */
QwtPlotCurve* QwtPyPlot::scatter(const QVector< double >& x,
                                 const QVector< double >& y,
                                 double size,
                                 const QString& color,
                                 const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* curve = QwtPlotFactory::createCurve(ax, label, x, y);
    curve->setStyle(QwtPlotCurve::NoCurve);

    QColor c = color.isEmpty() ? QColor(Qt::blue) : namedColor(color);
    int sz   = qMax(4, static_cast< int >(size / 3.0));
    QwtPlotStyling::setSymbol(curve, QwtSymbol::Ellipse, c, QSize(sz, sz));

    ax->replot();
    return curve;
}

QwtPlotBarChart* QwtPyPlot::bar(const QVector< double >& values, const QString& color, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* chart = QwtPlotFactory::createBarChart(ax, label, values);
    if (!color.isEmpty()) {
        QColor c = namedColor(color);
        chart->setBrush(QBrush(c));
        chart->setPen(QPen(c.darker(120), 1.0));
    }
    ax->replot();
    return chart;
}

QwtPlotBarChart* QwtPyPlot::bar(const QVector< double >& x,
                                const QVector< double >& values,
                                double width,
                                const QString& color,
                                const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    // Build QPointF data from x and values
    QVector< QPointF > data;
    data.reserve(qMin(x.size(), values.size()));
    for (int i = 0; i < qMin(x.size(), values.size()); i++) {
        data.append(QPointF(x[ i ], values[ i ]));
    }

    auto* chart = QwtPlotFactory::createBarChart(ax, label, data);
    Q_UNUSED(width)  // Width control would require custom symbol
    if (!color.isEmpty()) {
        QColor c = namedColor(color);
        chart->setBrush(QBrush(c));
        chart->setPen(QPen(c.darker(120), 1.0));
    }
    ax->replot();
    return chart;
}

/**
 * @brief Create a histogram from raw data with automatic binning
 * @param data Raw data values
 * @param bins Number of bins
 * @param color Bar color
 * @param label Legend label
 * @return The created histogram
 */
QwtPlotHistogram* QwtPyPlot::hist(const QVector< double >& data, int bins, const QString& color, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax || data.isEmpty())
        return nullptr;

    // Compute bin edges
    double minVal = *std::min_element(data.constBegin(), data.constEnd());
    double maxVal = *std::max_element(data.constBegin(), data.constEnd());
    if (qFuzzyCompare(minVal, maxVal)) {
        minVal -= 0.5;
        maxVal += 0.5;
    }
    double binWidth = (maxVal - minVal) / bins;

    // Count values in each bin
    QVector< double > counts(bins, 0.0);
    for (double v : data) {
        int idx = static_cast< int >((v - minVal) / binWidth);
        if (idx >= bins)
            idx = bins - 1;
        if (idx < 0)
            idx = 0;
        counts[ idx ] += 1.0;
    }

    // Create interval samples
    QVector< QwtIntervalSample > samples;
    samples.reserve(bins);
    for (int i = 0; i < bins; i++) {
        double left  = minVal + i * binWidth;
        double right = left + binWidth;
        samples.append(QwtIntervalSample(counts[ i ], left, right));
    }

    auto* hist = QwtPlotFactory::createHistogram(ax, label, samples);
    if (!color.isEmpty()) {
        QColor c = namedColor(color);
        hist->setBrush(QBrush(c));
        hist->setPen(QPen(c.darker(120), 1.0));
    }
    ax->replot();
    return hist;
}

QwtPlotBoxChart* QwtPyPlot::boxplot(const QVector< QwtBoxSample >& data, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* chart = QwtPlotFactory::createBoxChart(ax, label, data);
    ax->replot();
    return chart;
}

/**
 * @brief Fill the area between two curves
 * @param x X coordinates
 * @param y1 Upper curve values
 * @param y2 Lower curve values
 * @param color Fill color
 * @param alpha Fill opacity (0.0-1.0)
 * @return The created interval curve
 */
QwtPlotIntervalCurve* QwtPyPlot::fillBetween(const QVector< double >& x,
                                             const QVector< double >& y1,
                                             const QVector< double >& y2,
                                             const QString& color,
                                             double alpha)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    int n = static_cast< int >(std::min(
        { static_cast< size_t >(x.size()), static_cast< size_t >(y1.size()), static_cast< size_t >(y2.size()) }));
    QVector< QwtIntervalSample > samples;
    samples.reserve(n);
    for (int i = 0; i < n; i++) {
        double lo = qMin(y1[ i ], y2[ i ]);
        double hi = qMax(y1[ i ], y2[ i ]);
        samples.append(QwtIntervalSample(0.0, lo, hi));
    }

    auto* curve = QwtPlotFactory::createIntervalCurve(ax, QString(), samples);
    curve->setStyle(QwtPlotIntervalCurve::Tube);

    QColor c = color.isEmpty() ? QColor(Qt::blue) : namedColor(color);
    c.setAlphaF(alpha);
    curve->setBrush(QBrush(c));
    curve->setPen(QPen(Qt::NoPen));

    ax->replot();
    return curve;
}

/**
 * @brief Create error bars with symmetric y-error
 * @param x X coordinates
 * @param y Y values (center)
 * @param yerr Error values (symmetric)
 * @param fmt Format string for the center line
 * @param label Legend label
 * @return The created interval curve representing error bars
 */
QwtPlotIntervalCurve* QwtPyPlot::errorbar(const QVector< double >& x,
                                          const QVector< double >& y,
                                          const QVector< double >& yerr,
                                          const QString& fmt,
                                          const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    int n = static_cast< int >(std::min(
        { static_cast< size_t >(x.size()), static_cast< size_t >(y.size()), static_cast< size_t >(yerr.size()) }));
    QVector< QwtIntervalSample > samples;
    samples.reserve(n);
    for (int i = 0; i < n; i++) {
        samples.append(QwtIntervalSample(x[ i ], y[ i ] - yerr[ i ], y[ i ] + yerr[ i ]));
    }

    auto* curve = QwtPlotFactory::createIntervalCurve(ax, label, samples);
    curve->setOrientation(Qt::Horizontal);  // X is position, Y is interval
    curve->setStyle(QwtPlotIntervalCurve::Tube);

    QColor c = Qt::black;
    if (!fmt.isEmpty()) {
        auto f = QwtFormatString::parse(fmt);
        if (f.hasColor)
            c = f.color;
    }
    curve->setPen(QPen(c, 1.0));
    curve->setBrush(QBrush());

    ax->replot();
    return curve;
}

/**
 * @brief Display a 2D matrix as a color-mapped image
 * @param data 2D matrix (rows x cols)
 * @param cmap Color map name ("viridis", "hot", "cool", "jet", "gray")
 * @param vmin Minimum value for color scaling (0 = auto)
 * @param vmax Maximum value for color scaling (0 = auto)
 * @return The created spectrogram
 */
QwtPlotSpectrogram* QwtPyPlot::imshow(const QVector< QVector< double > >& data, const QString& cmap, double vmin, double vmax)
{
    QwtPlot* ax = gca();
    if (!ax || data.isEmpty() || data.first().isEmpty())
        return nullptr;

    int rows = data.size();
    int cols = data.first().size();

    // Flatten to 1D
    QVector< double > flat;
    flat.reserve(rows * cols);
    double minVal = std::numeric_limits< double >::max();
    double maxVal = std::numeric_limits< double >::lowest();
    for (const auto& row : data) {
        for (double v : row) {
            flat.append(v);
            if (v < minVal)
                minVal = v;
            if (v > maxVal)
                maxVal = v;
        }
    }

    if (vmin != 0.0 || vmax != 0.0) {
        minVal = vmin;
        maxVal = vmax;
    }

    auto* rasterData = new QwtMatrixRasterData();
    rasterData->setValueMatrix(flat, cols);
    rasterData->setInterval(Qt::XAxis, QwtInterval(0, cols));
    rasterData->setInterval(Qt::YAxis, QwtInterval(0, rows));
    rasterData->setInterval(Qt::ZAxis, QwtInterval(minVal, maxVal));

    auto* spectro = new QwtPlotSpectrogram();
    spectro->setData(rasterData);
    spectro->setDisplayMode(QwtPlotSpectrogram::ImageMode);

    QwtLinearColorMap* colorMap = createColorMap(cmap);
    if (colorMap) {
        spectro->setColorMap(colorMap);
    }

    spectro->attach(ax);
    ax->setAxisScale(QwtAxis::XBottom, 0, cols);
    ax->setAxisScale(QwtAxis::YLeft, 0, rows);
    ax->replot();
    return spectro;
}

/**
 * @brief Draw contour lines from a 2D matrix
 * @param data 2D matrix (rows x cols)
 * @param levels Contour levels (empty = auto-generate 10 levels)
 * @param cmap Color map name
 * @return The created spectrogram with contour mode
 */
QwtPlotSpectrogram*
QwtPyPlot::contour(const QVector< QVector< double > >& data, const QList< double >& levels, const QString& cmap)
{
    QwtPlot* ax = gca();
    if (!ax || data.isEmpty() || data.first().isEmpty())
        return nullptr;

    int rows = data.size();
    int cols = data.first().size();

    QVector< double > flat;
    flat.reserve(rows * cols);
    double minVal = std::numeric_limits< double >::max();
    double maxVal = std::numeric_limits< double >::lowest();
    for (const auto& row : data) {
        for (double v : row) {
            flat.append(v);
            if (v < minVal)
                minVal = v;
            if (v > maxVal)
                maxVal = v;
        }
    }

    auto* rasterData = new QwtMatrixRasterData();
    rasterData->setValueMatrix(flat, cols);
    rasterData->setInterval(Qt::XAxis, QwtInterval(0, cols));
    rasterData->setInterval(Qt::YAxis, QwtInterval(0, rows));
    rasterData->setInterval(Qt::ZAxis, QwtInterval(minVal, maxVal));

    auto* spectro = new QwtPlotSpectrogram();
    spectro->setData(rasterData);
    spectro->setDisplayMode(QwtPlotSpectrogram::ContourMode);

    QList< double > contourLevels = levels;
    if (contourLevels.isEmpty()) {
        // Auto-generate 10 levels
        double step = (maxVal - minVal) / 11.0;
        for (int i = 1; i <= 10; i++) {
            contourLevels.append(minVal + i * step);
        }
    }
    spectro->setContourLevels(contourLevels);

    QwtLinearColorMap* colorMap = createColorMap(cmap);
    if (colorMap) {
        spectro->setColorMap(colorMap);
    }

    spectro->attach(ax);
    ax->setAxisScale(QwtAxis::XBottom, 0, cols);
    ax->setAxisScale(QwtAxis::YLeft, 0, rows);
    ax->replot();
    return spectro;
}

/**
 * @brief Create a quiver (vector field) plot
 * @param x X positions
 * @param y Y positions
 * @param u X components of vectors
 * @param v Y components of vectors
 * @param color Arrow color
 * @return The created vector field
 */
QwtPlotVectorField* QwtPyPlot::quiver(const QVector< double >& x,
                                      const QVector< double >& y,
                                      const QVector< double >& u,
                                      const QVector< double >& v,
                                      const QString& color)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    int n = static_cast< int >(std::min({ static_cast< size_t >(x.size()),
                                          static_cast< size_t >(y.size()),
                                          static_cast< size_t >(u.size()),
                                          static_cast< size_t >(v.size()) }));
    QVector< QwtVectorFieldSample > samples;
    samples.reserve(n);
    for (int i = 0; i < n; i++) {
        samples.append(QwtVectorFieldSample(x[ i ], y[ i ], u[ i ], v[ i ]));
    }

    auto* field = QwtPlotFactory::createVectorField(ax, QString(), samples);
    if (!color.isEmpty()) {
        QColor c = namedColor(color);
        field->setPen(QPen(c, 1.0));
    }
    ax->replot();
    return field;
}

QwtPlotTradingCurve* QwtPyPlot::candlestick(const QVector< QwtOHLCSample >& data, const QString& label)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* curve = QwtPlotFactory::createTradingCurve(ax, label, data);
    ax->replot();
    return curve;
}

// ---- Auxiliary elements ----

QwtPlotGrid* QwtPyPlot::grid(bool show, bool minor)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    // Find existing grid to avoid duplicates
    QwtPlotGrid* g = nullptr;
    auto items     = ax->itemList(QwtPlotItem::Rtti_PlotGrid);
    if (!items.isEmpty()) {
        g = static_cast< QwtPlotGrid* >(items.first());
    }

    if (!g) {
        g = QwtPlotFactory::createGrid(ax, minor);
    } else {
        // Existing grid found — update minor grid settings
        g->enableXMin(minor);
        g->enableYMin(minor);
    }
    g->setVisible(show);
    ax->replot();
    return g;
}

QwtPlotMarker* QwtPyPlot::axhline(double y, const QString& fmt)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* marker = QwtPlotFactory::createHLine(ax, y);
    if (!fmt.isEmpty()) {
        applyFormat(marker, QwtFormatString::parse(fmt));
    }
    ax->replot();
    return marker;
}

QwtPlotMarker* QwtPyPlot::axvline(double x, const QString& fmt)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* marker = QwtPlotFactory::createVLine(ax, x);
    if (!fmt.isEmpty()) {
        applyFormat(marker, QwtFormatString::parse(fmt));
    }
    ax->replot();
    return marker;
}

QwtPlotZoneItem* QwtPyPlot::axhspan(double y1, double y2, const QString& color, double alpha)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    QColor c = color.isEmpty() ? QColor(0, 0, 255) : namedColor(color);
    c.setAlphaF(alpha);

    auto* zone = QwtPlotFactory::createZone(ax, QwtInterval(qMin(y1, y2), qMax(y1, y2)), Qt::Horizontal, QBrush(c));
    ax->replot();
    return zone;
}

QwtPlotZoneItem* QwtPyPlot::axvspan(double x1, double x2, const QString& color, double alpha)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    QColor c = color.isEmpty() ? QColor(0, 0, 255) : namedColor(color);
    c.setAlphaF(alpha);

    auto* zone = QwtPlotFactory::createZone(ax, QwtInterval(qMin(x1, x2), qMax(x1, x2)), Qt::Vertical, QBrush(c));
    ax->replot();
    return zone;
}

QwtPlotArrowMarker* QwtPyPlot::annotate(const QString& text, const QPointF& xy, const QPointF& xytext)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    auto* arrow = QwtPlotFactory::createArrowMarker(ax, xytext, xy);
    Q_UNUSED(text)  // Text label would require additional marker setup
    ax->replot();
    return arrow;
}

QwtPlotLegendItem* QwtPyPlot::legend(const QString& loc)
{
    QwtPlot* ax = gca();
    if (!ax)
        return nullptr;

    Q_UNUSED(loc)  // Location handling could be enhanced later

    // Find existing legend to avoid duplicates
    auto items = ax->itemList(QwtPlotItem::Rtti_PlotLegend);
    if (!items.isEmpty()) {
        return static_cast< QwtPlotLegendItem* >(items.first());
    }

    auto* leg = QwtPlotFactory::createLegend(ax);
    ax->replot();
    return leg;
}

// ---- Axis configuration ----

void QwtPyPlot::setTitle(const QString& title)
{
    QwtPlot* ax = gca();
    if (ax) {
        ax->setTitle(title);
    }
}

void QwtPyPlot::setXLabel(const QString& label)
{
    QwtPlot* ax = gca();
    if (ax) {
        ax->setAxisTitle(QwtAxis::XBottom, label);
    }
}

void QwtPyPlot::setYLabel(const QString& label)
{
    QwtPlot* ax = gca();
    if (ax) {
        ax->setAxisTitle(QwtAxis::YLeft, label);
    }
}

void QwtPyPlot::setXLim(double min, double max)
{
    QwtPlot* ax = gca();
    if (ax) {
        ax->setAxisScale(QwtAxis::XBottom, min, max);
        ax->replot();
    }
}

void QwtPyPlot::setYLim(double min, double max)
{
    QwtPlot* ax = gca();
    if (ax) {
        ax->setAxisScale(QwtAxis::YLeft, min, max);
        ax->replot();
    }
}

void QwtPyPlot::setXScale(const QString& scale)
{
    QwtPlot* ax = gca();
    if (!ax)
        return;

    if (scale.compare("log", Qt::CaseInsensitive) == 0) {
        ax->setAxisToLogScale(QwtAxis::XBottom);
    } else {
        ax->setAxisToLinearScale(QwtAxis::XBottom);
    }
    ax->replot();
}

void QwtPyPlot::setYScale(const QString& scale)
{
    QwtPlot* ax = gca();
    if (!ax)
        return;

    if (scale.compare("log", Qt::CaseInsensitive) == 0) {
        ax->setAxisToLogScale(QwtAxis::YLeft);
    } else {
        ax->setAxisToLinearScale(QwtAxis::YLeft);
    }
    ax->replot();
}

void QwtPyPlot::setXTicks(const QVector< double >& ticks, const QStringList& labels)
{
    QwtPlot* ax = gca();
    if (!ax || ticks.isEmpty())
        return;

    QwtScaleDiv div;
    div.setInterval(ticks.first(), ticks.last());
    div.setTicks(QwtScaleDiv::MajorTick, ticks.toList());
    ax->setAxisScaleDiv(QwtAxis::XBottom, div);

    Q_UNUSED(labels)  // Custom labels would require a custom QwtScaleDraw
    ax->replot();
}

void QwtPyPlot::setYTicks(const QVector< double >& ticks, const QStringList& labels)
{
    QwtPlot* ax = gca();
    if (!ax || ticks.isEmpty())
        return;

    QwtScaleDiv div;
    div.setInterval(ticks.first(), ticks.last());
    div.setTicks(QwtScaleDiv::MajorTick, ticks.toList());
    ax->setAxisScaleDiv(QwtAxis::YLeft, div);

    Q_UNUSED(labels)  // Custom labels would require a custom QwtScaleDraw
    ax->replot();
}

void QwtPyPlot::invertXAxis()
{
    QwtPlot* ax = gca();
    if (!ax)
        return;

    QwtInterval interval = ax->axisInterval(QwtAxis::XBottom);
    ax->setAxisScale(QwtAxis::XBottom, interval.maxValue(), interval.minValue());
    ax->replot();
}

void QwtPyPlot::invertYAxis()
{
    QwtPlot* ax = gca();
    if (!ax)
        return;

    QwtInterval interval = ax->axisInterval(QwtAxis::YLeft);
    ax->setAxisScale(QwtAxis::YLeft, interval.maxValue(), interval.minValue());
    ax->replot();
}

// ---- Appearance ----

void QwtPyPlot::setFaceColor(const QString& color)
{
    QWT_D(d);
    if (!d->figure)
        return;

    QColor c = namedColor(color);
    d->figure->setFaceColor(c);
}

void QwtPyPlot::setAxesColor(const QString& color)
{
    QwtPlot* ax = gca();
    if (!ax)
        return;

    QColor c = namedColor(color);
    ax->setCanvasBackground(QBrush(c));
    ax->replot();
}

void QwtPyPlot::colorbar(QwtPlotSpectrogram* spectro)
{
    QwtPlot* ax = gca();
    if (!ax)
        return;

    Q_UNUSED(spectro)  // Colorbar implementation would require right-axis setup
    // Basic implementation: enable color bar on the right axis
    QwtScaleWidget* scaleWidget = ax->axisWidget(QwtAxis::YRight);
    if (scaleWidget && spectro) {
        const QwtColorMap* cmap = spectro->colorMap();
        if (cmap && spectro->data()) {
            QwtInterval interval = spectro->data()->interval(Qt::ZAxis);
            scaleWidget->setColorBarEnabled(true);
            scaleWidget->setColorMap(interval, const_cast< QwtColorMap* >(cmap));
            ax->setAxisVisible(QwtAxis::YRight, true);
            ax->replot();
        }
    }
}

// ---- Output ----

bool QwtPyPlot::savefig(const QString& filename, int dpi)
{
    QWT_DC(d);
    if (d->figure) {
        return d->figure->saveFig(filename, dpi);
    }
    qWarning() << "QwtPyPlot::savefig: no figure available for saving";
    return false;
}

void QwtPyPlot::show()
{
    QWT_D(d);
    if (d->figure) {
        d->figure->show();
    } else if (d->currentAxes) {
        d->currentAxes->show();
    }
}

// ---- Interaction ----

void QwtPyPlot::enablePan(bool enable)
{
    QWT_D(d);
    QwtPlot* ax = gca();
    if (!ax)
        return;

    if (enable && !d->panner) {
        // Check if a panner already exists on the canvas (possibly created by another QwtPyPlot instance)
        d->panner = ax->canvas()->findChild< QwtPlotPanner* >();
    }
    if (enable && !d->panner) {
        d->panner = new QwtPlotPanner(ax->canvas());
    }
    if (d->panner) {
        d->panner->setEnabled(enable);
    }
}

void QwtPyPlot::enableZoom(bool enable)
{
    QWT_D(d);
    QwtPlot* ax = gca();
    if (!ax)
        return;

    if (enable && !d->zoomer) {
        // Check if a zoomer already exists on the canvas (possibly created by another QwtPyPlot instance)
        d->zoomer = ax->canvas()->findChild< QwtPlotCanvasZoomer* >();
    }
    if (enable && !d->zoomer) {
        d->zoomer = new QwtPlotCanvasZoomer(ax->canvas());
    }
    if (d->zoomer) {
        d->zoomer->setEnabled(enable);
    }
}

// ---- Private helpers ----

void QwtPyPlot::applyFormat(QwtPlotCurve* curve, const QwtFormatString& f)
{
    if (!curve)
        return;

    QColor c = f.hasColor ? f.color : curve->pen().color();

    if (f.noLine) {
        curve->setStyle(QwtPlotCurve::NoCurve);
    } else {
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setPen(QPen(c, 1.5, f.hasLineStyle ? f.lineStyle : Qt::SolidLine));
    }

    if (f.hasMarker) {
        int sz = (f.marker == QwtSymbol::Ellipse && f.noLine) ? 4 : 6;
        QwtPlotStyling::setSymbol(curve, f.marker, c, QSize(sz, sz));
    }
}

void QwtPyPlot::applyFormat(QwtPlotMarker* marker, const QwtFormatString& f)
{
    if (!marker)
        return;

    QColor c           = f.hasColor ? f.color : QColor(Qt::gray);
    Qt::PenStyle style = f.hasLineStyle ? f.lineStyle : Qt::DashLine;
    marker->setLinePen(QPen(c, 1.0, style));
}

QColor QwtPyPlot::namedColor(const QString& name)
{
    if (name.isEmpty())
        return QColor();

    // Single-character matplotlib colors
    if (name.length() == 1) {
        switch (name[ 0 ].toLatin1()) {
        case 'b':
            return QColor(Qt::blue);
        case 'g':
            return QColor(Qt::green);
        case 'r':
            return QColor(Qt::red);
        case 'c':
            return QColor(Qt::cyan);
        case 'm':
            return QColor(Qt::magenta);
        case 'y':
            return QColor(Qt::yellow);
        case 'k':
            return QColor(Qt::black);
        case 'w':
            return QColor(Qt::white);
        default:
            break;
        }
    }

    // Try as QColor name (e.g. "blue", "#ff0000")
    QColor c(name);
    if (c.isValid())
        return c;

    return QColor(Qt::blue);  // Default fallback
}

/**
 * @brief Create a color map by name
 * @param name Color map name ("viridis", "hot", "cool", "jet", "gray")
 * @return Newly allocated QwtLinearColorMap, or nullptr if unknown
 */
QwtLinearColorMap* QwtPyPlot::createColorMap(const QString& name)
{
    auto* map = new QwtLinearColorMap(QwtColorMap::RGB);
    map->setMode(QwtLinearColorMap::ScaledColors);

    if (name.compare("viridis", Qt::CaseInsensitive) == 0) {
        map->addColorStop(0.0, QColor(68, 1, 84));
        map->addColorStop(0.25, QColor(59, 82, 139));
        map->addColorStop(0.5, QColor(33, 145, 140));
        map->addColorStop(0.75, QColor(94, 201, 98));
        map->addColorStop(1.0, QColor(253, 231, 37));
    } else if (name.compare("hot", Qt::CaseInsensitive) == 0) {
        map->addColorStop(0.0, QColor(Qt::black));
        map->addColorStop(0.33, QColor(Qt::red));
        map->addColorStop(0.67, QColor(Qt::yellow));
        map->addColorStop(1.0, QColor(Qt::white));
    } else if (name.compare("cool", Qt::CaseInsensitive) == 0) {
        map->setColorInterval(QColor(Qt::cyan), QColor(Qt::magenta));
    } else if (name.compare("jet", Qt::CaseInsensitive) == 0) {
        map->addColorStop(0.0, QColor(0, 0, 128));
        map->addColorStop(0.125, QColor(0, 0, 255));
        map->addColorStop(0.375, QColor(0, 255, 255));
        map->addColorStop(0.625, QColor(255, 255, 0));
        map->addColorStop(0.875, QColor(255, 0, 0));
        map->addColorStop(1.0, QColor(128, 0, 0));
    } else if (name.compare("gray", Qt::CaseInsensitive) == 0 || name.compare("grey", Qt::CaseInsensitive) == 0) {
        map->setColorInterval(QColor(Qt::black), QColor(Qt::white));
    } else {
        // Default to viridis
        map->addColorStop(0.0, QColor(68, 1, 84));
        map->addColorStop(0.25, QColor(59, 82, 139));
        map->addColorStop(0.5, QColor(33, 145, 140));
        map->addColorStop(0.75, QColor(94, 201, 98));
        map->addColorStop(1.0, QColor(253, 231, 37));
    }

    return map;
}
