/*****************************************************************************
 * Qwt Polar Examples - Copyright (C) 2008   Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QWidget>
#include <QString>

class Plot;
class QwtScaleWidget;

class PlotWindow : public QWidget
{
  public:
    PlotWindow( QWidget* = NULL );

    Plot* plot() { return m_plot; }

    void setColorMapPreset( const QString& name );

  private:
    Plot* m_plot;
    QwtScaleWidget* m_colorScale;
};
