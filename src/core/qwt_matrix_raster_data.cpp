/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *   Summary of major modifications (see ChangeLog.md for full history):
 *   1. CMake build system & C++11 throughout.
 *   2. Core panner/ zoomer refactored:
 *        - QwtPanner → QwtCachePanner (pixmap-cache version)
 *        - New real-time QwtPlotPanner derived from QwtPicker.
 *   3. Zoomer supports multi-axis.
 *   4. Parasite-plot framework:
 *        - QwtFigure, QwtPlotParasiteLayout, QwtPlotTransparentCanvas,
 *        - QwtPlotScaleEventDispatcher, built-in pan/zoom on axis.
 *   5. New picker: QwtPlotSeriesDataPicker (works with date axis).
 *   6. Raster & color-map extensions:
 *        - QwtGridRasterData (2-D table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 *****************************************************************************/

#include "qwt_matrix_raster_data.h"
#include "qwt_interval.h"

#include <qvector.h>
#include <qnumeric.h>
#include <qrect.h>

static inline double qwtHermiteInterpolate(
    double A, double B, double C, double D, double t )
{
    const double t2 = t * t;
    const double t3 = t2 * t;

    const double a = -A / 2.0 + ( 3.0 * B ) / 2.0 - ( 3.0 * C ) / 2.0 + D / 2.0;
    const double b = A - ( 5.0 * B ) / 2.0 + 2.0 * C - D / 2.0;
    const double c = -A / 2.0 + C / 2.0;
    const double d = B;

    return a * t3 + b * t2 + c * t + d;
}

static inline double qwtBicubicInterpolate(
    double v00, double v10, double v20, double v30,
    double v01, double v11, double v21, double v31,
    double v02, double v12, double v22, double v32,
    double v03, double v13, double v23, double v33,
    double dx, double dy )
{
    const double v0 = qwtHermiteInterpolate( v00, v10, v20, v30, dx );
    const double v1 = qwtHermiteInterpolate( v01, v11, v21, v31, dx );
    const double v2 = qwtHermiteInterpolate( v02, v12, v22, v32, dx );
    const double v3 = qwtHermiteInterpolate( v03, v13, v23, v33, dx );

    return qwtHermiteInterpolate( v0, v1, v2, v3, dy );
}

class QwtMatrixRasterData::PrivateData
{
  public:
    QWT_DECLARE_PUBLIC(QwtMatrixRasterData)

    PrivateData(QwtMatrixRasterData* p)
        : q_ptr(p)
        , resampleMode( QwtMatrixRasterData::NearestNeighbour )
        , numColumns(0)
    {
    }

    inline double value(int row, int col) const
    {
        return values.data()[ row * numColumns + col ];
    }

    QwtInterval intervals[3];
    QwtMatrixRasterData::ResampleMode resampleMode;

    QVector< double > values;
    int numColumns;
    int numRows;

    double dx;
    double dy;
};

/**
 * @brief Constructor.
 */
QwtMatrixRasterData::QwtMatrixRasterData() : QWT_PIMPL_CONSTRUCT
{
    update();
}

/**
 * @brief Destructor
 */
QwtMatrixRasterData::~QwtMatrixRasterData()
{
}

/**
 * @brief Set the resampling algorithm.
 * @param[in] mode Resampling mode.
 * @sa resampleMode(), value()
 */
void QwtMatrixRasterData::setResampleMode( ResampleMode mode )
{
    QWT_D(d);
    d->resampleMode = mode;
}

/**
 * @brief Return the resampling algorithm.
 * @return Resampling algorithm.
 * @sa setResampleMode(), value()
 */
QwtMatrixRasterData::ResampleMode QwtMatrixRasterData::resampleMode() const
{
    QWT_DC(d);
    return d->resampleMode;
}

/**
 * @brief Assign the bounding interval for an axis.
 * @details Setting the bounding intervals for the X/Y axis is mandatory to define the positions
 *          for the values of the value matrix. The interval in Z direction defines the possible range
 *          for the values in the matrix, what is f.e used by QwtPlotSpectrogram to map values to colors.
 *          The Z-interval might be the bounding interval of the values in the matrix, but usually it isn't.
 *          (f.e a interval of 0.0-100.0 for values in percentage).
 * @param[in] axis X, Y or Z axis.
 * @param[in] interval Interval.
 * @sa QwtRasterData::interval(), setValueMatrix()
 */
void QwtMatrixRasterData::setInterval(
    Qt::Axis axis, const QwtInterval& interval )
{
    QWT_D(d);
    if ( axis >= 0 && axis <= 2 )
    {
        d->intervals[axis] = interval;
        update();
    }
}

/**
 * @brief Return bounding interval for an axis.
 * @param[in] axis Axis to query.
 * @return Bounding interval for the axis.
 * @sa setInterval()
 */
QwtInterval QwtMatrixRasterData::interval( Qt::Axis axis ) const
{
    QWT_DC(d);
    if ( axis >= 0 && axis <= 2 )
        return d->intervals[ axis ];

    return QwtInterval();
}

/**
 * @brief Assign a value matrix.
 * @details The positions of the values are calculated by dividing the bounding rectangle
 *          of the X/Y intervals into equidistant rectangles (pixels).
 *          Each value corresponds to the center of a pixel.
 * @param[in] values Vector of values.
 * @param[in] numColumns Number of columns.
 * @sa valueMatrix(), numColumns(), numRows(), setInterval()
 */
void QwtMatrixRasterData::setValueMatrix(
    const QVector< double >& values, int numColumns )
{
    QWT_D(d);
    d->values = values;
    d->numColumns = qMax( numColumns, 0 );
    update();
}

/**
 * @brief Return value matrix.
 * @return Value matrix.
 * @sa setValueMatrix(), numColumns(), numRows(), setInterval()
 */
const QVector< double > QwtMatrixRasterData::valueMatrix() const
{
    QWT_DC(d);
    return d->values;
}

/**
 * @brief Change a single value in the matrix.
 * @param[in] row Row index.
 * @param[in] col Column index.
 * @param[in] value New value.
 * @sa value(), setValueMatrix()
 */
void QwtMatrixRasterData::setValue( int row, int col, double value )
{
    QWT_D(d);
    if ( row >= 0 && row < d->numRows &&
        col >= 0 && col < d->numColumns )
    {
        const int index = row * d->numColumns + col;
        d->values.data()[ index ] = value;
    }
}

/**
 * @brief Return number of columns of the value matrix.
 * @return Number of columns.
 * @sa valueMatrix(), numRows(), setValueMatrix()
 */
int QwtMatrixRasterData::numColumns() const
{
    QWT_DC(d);
    return d->numColumns;
}

/**
 * @brief Return number of rows of the value matrix.
 * @return Number of rows.
 * @sa valueMatrix(), numColumns(), setValueMatrix()
 */
int QwtMatrixRasterData::numRows() const
{
    QWT_DC(d);
    return d->numRows;
}

/**
 * @brief Calculate the pixel hint.
 * @details pixelHint() returns the geometry of a pixel, that can be used to calculate
 *          the resolution and alignment of the plot item, that is representing the data.
 *          - NearestNeighbour: pixelHint() returns the surrounding pixel of the top left value in the matrix.
 *          - BilinearInterpolation: Returns an empty rectangle recommending to render in target device resolution.
 * @param[in] area Requested area, ignored.
 * @return Calculated hint.
 * @sa ResampleMode, setMatrix(), setInterval()
 */
QRectF QwtMatrixRasterData::pixelHint( const QRectF& area ) const
{
    QWT_DC(d);
    Q_UNUSED( area )

    QRectF rect;
    if ( d->resampleMode == NearestNeighbour )
    {
        const QwtInterval intervalX = interval( Qt::XAxis );
        const QwtInterval intervalY = interval( Qt::YAxis );
        if ( intervalX.isValid() && intervalY.isValid() )
        {
            rect = QRectF( intervalX.minValue(), intervalY.minValue(),
                d->dx, d->dy );
        }
    }

    return rect;
}

/**
 * @brief Return the value at a raster position.
 * @param[in] x X value in plot coordinates.
 * @param[in] y Y value in plot coordinates.
 * @return Value at the position.
 * @sa ResampleMode
 */
double QwtMatrixRasterData::value( double x, double y ) const
{
    QWT_DC(d);
    const QwtInterval xInterval = interval( Qt::XAxis );
    const QwtInterval yInterval = interval( Qt::YAxis );

    if ( !( xInterval.contains(x) && yInterval.contains(y) ) )
        return qQNaN();

    double value;

    switch( d->resampleMode )
    {
        case BicubicInterpolation:
        {
            const double colF = ( x - xInterval.minValue() ) / d->dx;
            const double rowF = ( y - yInterval.minValue() ) / d->dy;

            const int col = qRound( colF );
            const int row = qRound( rowF );

            int col0 = col - 2;
            int col1 = col - 1;
            int col2 = col;
            int col3 = col + 1;

            if ( col1 < 0 )
                col1 = col2;

            if ( col0 < 0 )
                col0 = col1;

            if ( col2 >= d->numColumns )
                col2 = col1;

            if ( col3 >= d->numColumns )
                col3 = col2;

            int row0 = row - 2;
            int row1 = row - 1;
            int row2 = row;
            int row3 = row + 1;

            if ( row1 < 0 )
                row1 = row2;

            if ( row0 < 0 )
                row0 = row1;

            if ( row2 >= d->numRows )
                row2 = row1;

            if ( row3 >= d->numRows )
                row3 = row2;

            // First row
            const double v00 = d->value( row0, col0 );
            const double v10 = d->value( row0, col1 );
            const double v20 = d->value( row0, col2 );
            const double v30 = d->value( row0, col3 );

            // Second row
            const double v01 = d->value( row1, col0 );
            const double v11 = d->value( row1, col1 );
            const double v21 = d->value( row1, col2 );
            const double v31 = d->value( row1, col3 );

            // Third row
            const double v02 = d->value( row2, col0 );
            const double v12 = d->value( row2, col1 );
            const double v22 = d->value( row2, col2 );
            const double v32 = d->value( row2, col3 );

            // Fourth row
            const double v03 = d->value( row3, col0 );
            const double v13 = d->value( row3, col1 );
            const double v23 = d->value( row3, col2 );
            const double v33 = d->value( row3, col3 );

            value = qwtBicubicInterpolate(
                v00, v10, v20, v30, v01, v11, v21, v31,
                v02, v12, v22, v32, v03, v13, v23, v33,
                colF - col + 0.5, rowF - row + 0.5 );

            break;
        }
        case BilinearInterpolation:
        {
            int col1 = qRound( ( x - xInterval.minValue() ) / d->dx ) - 1;
            int row1 = qRound( ( y - yInterval.minValue() ) / d->dy ) - 1;
            int col2 = col1 + 1;
            int row2 = row1 + 1;

            if ( col1 < 0 )
                col1 = col2;
            else if ( col2 >= d->numColumns )
                col2 = col1;

            if ( row1 < 0 )
                row1 = row2;
            else if ( row2 >= d->numRows )
                row2 = row1;

            const double v11 = d->value( row1, col1 );
            const double v21 = d->value( row1, col2 );
            const double v12 = d->value( row2, col1 );
            const double v22 = d->value( row2, col2 );

            const double x2 = xInterval.minValue() + ( col2 + 0.5 ) * d->dx;
            const double y2 = yInterval.minValue() + ( row2 + 0.5 ) * d->dy;

            const double rx = ( x2 - x ) / d->dx;
            const double ry = ( y2 - y ) / d->dy;

            const double vr1 = rx * v11 + ( 1.0 - rx ) * v21;
            const double vr2 = rx * v12 + ( 1.0 - rx ) * v22;

            value = ry * vr1 + ( 1.0 - ry ) * vr2;

            break;
        }
        case NearestNeighbour:
        default:
        {
            int row = int( ( y - yInterval.minValue() ) / d->dy );
            int col = int( ( x - xInterval.minValue() ) / d->dx );

            // In case of intervals, where the maximum is included
            // we get out of bound for row/col, when the value for the
            // maximum is requested. Instead we return the value
            // from the last row/col

            if ( row >= d->numRows )
                row = d->numRows - 1;

            if ( col >= d->numColumns )
                col = d->numColumns - 1;

            value = d->value( row, col );
        }
    }

    return value;
}

void QwtMatrixRasterData::update()
{
    QWT_D(d);
    d->numRows = 0;
    d->dx = 0.0;
    d->dy = 0.0;

    if ( d->numColumns > 0 )
    {
        d->numRows = d->values.size() / d->numColumns;

        const QwtInterval xInterval = interval( Qt::XAxis );
        const QwtInterval yInterval = interval( Qt::YAxis );
        if ( xInterval.isValid() )
            d->dx = xInterval.width() / d->numColumns;
        if ( yInterval.isValid() )
            d->dy = yInterval.width() / d->numRows;
    }
}
