/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "BoxChart.h"

#include <QwtPlotRenderer>
#include <QwtPlotBoxChart>
#include <qwt_box_statistics.h>
#include <QwtPlotLayout>
#include <QwtLegend>
#include <QwtScaleDraw>
#include <QwtSymbol>
#include <QwtSamples>
#include <QwtText>
#include <QwtMath>

BoxChart::BoxChart( QWidget* parent )
    : QwtPlot( parent )
    , m_boxChartItem1( NULL )
    , m_boxChartItem2( NULL )
{
    setAutoFillBackground( true );
    setPalette( Qt::white );
    canvas()->setPalette( QColor( "LemonChiffon" ) );

    setTitle( "Box Chart Example" );
    setAxisTitle( QwtAxis::YLeft, "Value" );
    setAxisTitle( QwtAxis::XBottom, "Sample" );

    insertLegend( new QwtLegend() );

    populate();
    setOrientation( 0 );
    setAutoReplot( true );
}

void BoxChart::populate()
{
    m_boxChartItem1 = new QwtPlotBoxChart( "Pre-computed (Manual)" );
    m_boxChartItem1->attach( this );

    QVector< QwtBoxSample > samples1;
    samples1 += QwtBoxSample( 1.0, 10.0, 20.0, 35.0, 50.0, 60.0 );
    samples1 += QwtBoxSample( 2.0, 5.0, 15.0, 30.0, 45.0, 55.0 );
    samples1 += QwtBoxSample( 3.0, 8.0, 25.0, 40.0, 55.0, 70.0 );
    samples1 += QwtBoxSample( 4.0, 12.0, 18.0, 28.0, 42.0, 58.0 );
    m_boxChartItem1->setSamples( samples1 );
    m_boxChartItem1->setBrush( QColor( 100, 150, 200, 150 ) );
    m_boxChartItem1->setPen( QPen( Qt::darkBlue, 2.0 ) );
    m_boxChartItem1->setBoxExtent( 0.35 );

    QwtSymbol* outlierSymbol1 = new QwtSymbol( QwtSymbol::Diamond );
    outlierSymbol1->setSize( 8, 8 );
    outlierSymbol1->setBrush( Qt::red );
    outlierSymbol1->setPen( QPen( Qt::darkRed, 1.0 ) );
    m_boxChartItem1->setOutlierSymbol( outlierSymbol1 );
    m_boxChartItem1->setOutlierJitter( 0.1 );

    m_boxChartItem2 = new QwtPlotBoxChart( "Calculated from Raw Data" );
    m_boxChartItem2->attach( this );

    QVector< QwtBoxSample > samples2;
    QVector< QwtBoxOutlierSample > outlierSamples;

    QVector< double > rawData1;
    for ( int i = 0; i < 100; i++ )
        rawData1 += 50.0 + ( qwtRand() % 50 ) - 25.0;
    rawData1 += 5.0;
    rawData1 += 100.0;

    QwtBoxSample sample1;
    QwtBoxOutlierSample outlier1;
    QwtBoxStatisticsCalculator::calculateFull( 1.5, rawData1, sample1, outlier1 );
    samples2 += sample1;
    outlierSamples += outlier1;

    QVector< double > rawData2;
    for ( int i = 0; i < 80; i++ )
        rawData2 += 70.0 + ( qwtRand() % 40 ) - 20.0;
    rawData2 += 10.0;
    rawData2 += 120.0;

    QwtBoxSample sample2;
    QwtBoxOutlierSample outlier2;
    QwtBoxStatisticsCalculator::calculateFull( 2.5, rawData2, sample2, outlier2 );
    samples2 += sample2;
    outlierSamples += outlier2;

    QVector< double > rawData3;
    for ( int i = 0; i < 60; i++ )
        rawData3 += 40.0 + ( qwtRand() % 60 ) - 30.0;

    QwtBoxSample sample3;
    QwtBoxOutlierSample outlier3;
    QwtBoxStatisticsCalculator::calculateFull( 3.5, rawData3, sample3, outlier3 );
    samples2 += sample3;
    outlierSamples += outlier3;

    m_boxChartItem2->setSamples( samples2 );
    m_boxChartItem2->setOutliers( outlierSamples );
    m_boxChartItem2->setBrush( QColor( 200, 100, 100, 150 ) );
    m_boxChartItem2->setPen( QPen( Qt::darkRed, 2.0 ) );
    m_boxChartItem2->setBoxExtent( 0.35 );

    QwtSymbol* outlierSymbol2 = new QwtSymbol( QwtSymbol::Ellipse );
    outlierSymbol2->setSize( 6, 6 );
    outlierSymbol2->setBrush( Qt::blue );
    outlierSymbol2->setPen( QPen( Qt::darkBlue, 1.0 ) );
    m_boxChartItem2->setOutlierSymbol( outlierSymbol2 );
    m_boxChartItem2->setOutlierJitter( 0.08 );
}

void BoxChart::setBoxStyle( int style )
{
    if ( m_boxChartItem1 == NULL || m_boxChartItem2 == NULL )
        return;

    QwtPlotBoxChart::BoxStyle boxStyle = static_cast< QwtPlotBoxChart::BoxStyle >( style + 1 );
    m_boxChartItem1->setBoxStyle( boxStyle );
    m_boxChartItem2->setBoxStyle( boxStyle );
}

void BoxChart::setOrientation( int orientation )
{
    if ( m_boxChartItem1 == NULL || m_boxChartItem2 == NULL )
        return;

    int axis1, axis2;

    if ( orientation == 0 )
    {
        axis1 = QwtAxis::XBottom;
        axis2 = QwtAxis::YLeft;

        m_boxChartItem1->setOrientation( Qt::Vertical );
        m_boxChartItem2->setOrientation( Qt::Vertical );
    }
    else
    {
        axis1 = QwtAxis::YLeft;
        axis2 = QwtAxis::XBottom;

        m_boxChartItem1->setOrientation( Qt::Horizontal );
        m_boxChartItem2->setOrientation( Qt::Horizontal );
    }

    setAxisAutoScale( axis1 );
    setAxisAutoScale( axis2 );

    QwtScaleDraw* scaleDraw1 = axisScaleDraw( axis1 );
    scaleDraw1->enableComponent( QwtScaleDraw::Backbone, true );
    scaleDraw1->enableComponent( QwtScaleDraw::Ticks, true );

    QwtScaleDraw* scaleDraw2 = axisScaleDraw( axis2 );
    scaleDraw2->enableComponent( QwtScaleDraw::Backbone, true );
    scaleDraw2->enableComponent( QwtScaleDraw::Ticks, true );

    plotLayout()->setAlignCanvasToScale( axis1, false );
    plotLayout()->setAlignCanvasToScale( axis2, false );

    plotLayout()->setCanvasMargin( 0 );
    updateCanvasMargins();
    replot();
}

void BoxChart::exportChart()
{
    QwtPlotRenderer renderer;
    renderer.exportTo( this, "boxchart.pdf" );
}

#include "moc_BoxChart.cpp"