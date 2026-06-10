#ifndef TEST_FLAT_STYLE_H
#define TEST_FLAT_STYLE_H

#include <QObject>

class TestFlatStyle : public QObject
{
    Q_OBJECT

private slots:
    void testSliderDefaultFlatStyle();
    void testSliderSetFlatStyle();
    void testThermoDefaultFlatStyle();
    void testThermoSetFlatStyle();
    void testWheelDefaultFlatStyle();
    void testWheelSetFlatStyle();
    void testKnobDefaultStyle();
    void testDialNeedleDefaultFlatStyle();
    void testDialNeedleSetFlatStyle();
    void testCompassRoseDefaultFlatStyle();
    void testCompassRoseSetFlatStyle();
    void testGLCanvasDefaultFrameStyle();
};

#endif
