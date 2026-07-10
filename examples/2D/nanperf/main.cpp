#include <QApplication>
#include <QwtPlot>
#include <QwtPlotGrid>
#include <QwtPlotCurve>

#include <cmath>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QwtPlot plot;
    plot.setTitle("nanperf scaffold");
    plot.setCanvasBackground(Qt::white);

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->attach(&plot);

    QwtPlotCurve* curve = new QwtPlotCurve("scaffold");
    curve->setPen(Qt::blue, 2);
    QVector<double> x, y;
    for (int i = 0; i < 10; ++i)
    {
        x << double(i);
        y << std::sin(double(i));
    }
    curve->setSamples(x, y);
    curve->attach(&plot);

    plot.resize(600, 400);
    plot.show();

    return app.exec();
}
