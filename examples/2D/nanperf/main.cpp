#include <QApplication>
#include "PlotPanel.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PlotPanel panel(NanCase::Middle);
    panel.setCurveData(100000, 0.5);
    panel.setMode(4);  // FilterPointsLTTB (default)
    panel.show();

    return app.exec();
}
