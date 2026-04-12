/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class QwtPlot;
class QwtPlotCurve;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onYLeftToggled(bool checked);
    void onYRightToggled(bool checked);
    void onXTopToggled(bool checked);
    void onXBottomToggled(bool checked);
    void onAllInsideClicked();
    void onAllOutsideClicked();

private:
    void setupPlot();
    QWidget* createControlPanel();

    QwtPlot* m_plot;
    QwtPlotCurve* m_curve;

    QCheckBox* m_chkYLeft;
    QCheckBox* m_chkYRight;
    QCheckBox* m_chkXTop;
    QCheckBox* m_chkXBottom;
};

#endif // MAIN_WINDOW_H