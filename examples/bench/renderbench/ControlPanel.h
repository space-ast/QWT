/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QWidget>

class QSpinBox;
class QComboBox;
class QPushButton;
class QProgressBar;
class QLabel;

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    ControlPanel(QWidget* parent = nullptr);

    int numPoints() const;
    int frameCount() const;
    int waveType() const;
    int methodIndex() const;

    void setProgress(int current, int total);
    void setStatus(const QString& text);
    void setRunning(bool running);

Q_SIGNALS:
    void runRequested();
    void runAllRequested();

private:
    QSpinBox* m_numPoints;
    QSpinBox* m_frameCount;
    QComboBox* m_waveType;
    QComboBox* m_method;
    QPushButton* m_runBtn;
    QPushButton* m_runAllBtn;
    QProgressBar* m_progress;
    QLabel* m_status;
};
