#ifndef FILTER_MODES_H
#define FILTER_MODES_H

#include <QVector>
#include <QString>
#include <qwt_plot_curve.h>

/// Definition of one point-filtering paint mode.
struct FilterMode
{
    QString label;
    QwtPlotCurve::PaintAttribute attr;
    bool clipOnly;  ///< true => ClipPolygons only, no downsample filter
};

/// The five filtering modes compared by the benchmark.
inline QVector< FilterMode > filterModes()
{
    return {
        { QStringLiteral("ClipPolygons"), QwtPlotCurve::ClipPolygons, true },
        { QStringLiteral("FilterPoints"), QwtPlotCurve::FilterPoints, false },
        { QStringLiteral("FilterPointsAggressive"), QwtPlotCurve::FilterPointsAggressive, false },
        { QStringLiteral("FilterPointsPixel"), QwtPlotCurve::FilterPointsPixel, false },
        { QStringLiteral("FilterPointsLTTB"), QwtPlotCurve::FilterPointsLTTB, false },
    };
}

/// Clear all filter paint attributes, keep ClipPolygons on (library default),
/// then enable the downsample filter of mode @a modeIndex (unless clipOnly).
inline void applyMode(QwtPlotCurve* curve, int modeIndex)
{
    curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
    curve->setPaintAttribute(QwtPlotCurve::FilterPoints, false);
    curve->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive, false);
    curve->setPaintAttribute(QwtPlotCurve::FilterPointsPixel, false);
    curve->setPaintAttribute(QwtPlotCurve::FilterPointsLTTB, false);

    curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);

    const auto modes = filterModes();
    const int idx    = (modeIndex >= 0 && modeIndex < modes.size()) ? modeIndex : modes.size() - 1;
    if (!modes[ idx ].clipOnly)
        curve->setPaintAttribute(modes[ idx ].attr, true);
}

#endif
