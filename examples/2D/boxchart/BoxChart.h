/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QwtPlot>

class QwtPlotBoxChart;

class BoxChart : public QwtPlot
{
    Q_OBJECT

  public:
    BoxChart( QWidget* = NULL );

  public Q_SLOTS:
    void setBoxStyle( int style );
    void setOrientation( int orientation );
    void exportChart();

  private:
    void populate();

    QwtPlotBoxChart* m_boxChartItem1;
    QwtPlotBoxChart* m_boxChartItem2;
};