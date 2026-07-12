#ifndef PLOT_PANEL_H
#define PLOT_PANEL_H

#include <QWidget>
#include <QVector>
#include "NanDataGenerator.h"

class QwtPlot;
class QwtPlotCurve;
class QLabel;

/// One grid cell: a QwtPlot with a single curve for a fixed NanCase.
class PlotPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PlotPanel(NanCase cs, QWidget* parent = nullptr);

    /// Generate data for @a numPoints / @a nanFraction and set it on the curve.
    void setCurveData(int numPoints, double nanFraction);
    /// Apply filtering mode @a modeIndex to the curve.
    void setMode(int modeIndex);
    /// Fix axis scales so replot does not autoscale (pure render).
    void setFixedScales(int numPoints);

    /// Set data, apply mode, replot, and show the measured replot time.
    void apply(int numPoints, double nanFraction, int modeIndex);

    QwtPlot* plot() const
    {
        return m_plot;
    }
    QwtPlotCurve* curve() const
    {
        return m_curve;
    }
    NanCase nanCase() const
    {
        return m_case;
    }

    /// Update the small replot-time readout under the plot.
    void showReplotTime(double ms);

private:
    NanCase m_case;
    QwtPlot* m_plot;
    QwtPlotCurve* m_curve;
    QLabel* m_titleLabel;
    QLabel* m_timeLabel;
};

#endif
