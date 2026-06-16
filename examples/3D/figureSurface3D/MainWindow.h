#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include <QVector>

class QAction;
class QActionGroup;
class QLabel;
class QTimer;
class QwtFigure;
class QwtPlot;
class QwtPlotCurve;
class DynamicSurfacePlot;

// Main window combining a 3D dynamic surface with 2D cross-section and time-series plots
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private Q_SLOTS:
    void onAnimationToggled(bool on);
    void onAutoRotateToggled(bool on);
    void onTimerTick();
    void onPlotStyleTriggered(QAction* action);
    void onFloorStyleTriggered(QAction* action);
    void onCoordStyleTriggered(QAction* action);
    void onLightingToggled(bool on);
    void onLegendToggled(bool on);
    void onOrthoToggled(bool on);
    void onResetView();
    void onSaveFigure();

    // Status bar updates from 3D plot signals
    void showRotation(double x, double y, double z);
    void showZoom(double z);

private:
    void createWidgets();
    void createToolBar();
    QActionGroup* createPlotStyleGroup();
    QActionGroup* createFloorStyleGroup();
    QActionGroup* createCoordStyleGroup();
    void update2DPlots();

    QwtFigure* m_figure = nullptr;
    DynamicSurfacePlot* m_surfacePlot = nullptr;
    QwtPlot* m_crossSectionPlot = nullptr;
    QwtPlot* m_timeSeriesPlot = nullptr;
    QwtPlotCurve* m_crossSectionCurve = nullptr;
    QwtPlotCurve* m_timeSeriesCurve = nullptr;

    QTimer* m_animTimer = nullptr;
    QTimer* m_rotateTimer = nullptr;

    // Time-series ring buffer
    static constexpr int TIME_SERIES_MAX = 200;
    QVector<double> m_timeData;
    QVector<double> m_zData;

    // Status bar labels
    QLabel* m_rotationLabel = nullptr;
    QLabel* m_zoomLabel = nullptr;
    QLabel* m_timeLabel = nullptr;
};

#endif // MAIN_WINDOW_H
