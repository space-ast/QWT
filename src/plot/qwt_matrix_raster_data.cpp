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
    PrivateData()
        : resampleMode( QwtMatrixRasterData::NearestNeighbour )
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
 * \if ENGLISH
 * @brief Constructor.
 * \endif
 * \if CHINESE
 * @brief 构造函数。
 * \endif
 */
QwtMatrixRasterData::QwtMatrixRasterData()
{
    m_data = new PrivateData();
    update();
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtMatrixRasterData::~QwtMatrixRasterData()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the resampling algorithm.
 * @param[in] mode Resampling mode.
 * @sa resampleMode(), value()
 * \endif
 * \if CHINESE
 * @brief 设置重采样算法。
 * @param[in] mode 重采样模式。
 * @sa resampleMode(), value()
 * \endif
 */
void QwtMatrixRasterData::setResampleMode( ResampleMode mode )
{
    m_data->resampleMode = mode;
}

/**
 * \if ENGLISH
 * @brief Return the resampling algorithm.
 * @return Resampling algorithm.
 * @sa setResampleMode(), value()
 * \endif
 * \if CHINESE
 * @brief 返回重采样算法。
 * @return 重采样算法。
 * @sa setResampleMode(), value()
 * \endif
 */
QwtMatrixRasterData::ResampleMode QwtMatrixRasterData::resampleMode() const
{
    return m_data->resampleMode;
}

/**
 * \if ENGLISH
 * @brief Assign the bounding interval for an axis.
 * @details Setting the bounding intervals for the X/Y axis is mandatory to define the positions
 *          for the values of the value matrix. The interval in Z direction defines the possible range
 *          for the values in the matrix, what is f.e used by QwtPlotSpectrogram to map values to colors.
 *          The Z-interval might be the bounding interval of the values in the matrix, but usually it isn't.
 *          (f.e a interval of 0.0-100.0 for values in percentage).
 * @param[in] axis X, Y or Z axis.
 * @param[in] interval Interval.
 * @sa QwtRasterData::interval(), setValueMatrix()
 * \endif
 * \if CHINESE
 * @brief 为轴分配边界区间。
 * @details 设置 X/Y 轴的边界区间是必须的，用于定义数值矩阵中各值的位置。
 *          Z 方向的区间定义了矩阵中数值的可能范围，例如用于 QwtPlotSpectrogram 将数值映射为颜色。
 *          Z 区间可以是矩阵中数值的边界区间，但通常不是。
 *          （例如百分比值使用 0.0-100.0 区间）。
 * @param[in] axis X、Y 或 Z 轴。
 * @param[in] interval 区间。
 * @sa QwtRasterData::interval(), setValueMatrix()
 * \endif
 */
void QwtMatrixRasterData::setInterval(
    Qt::Axis axis, const QwtInterval& interval )
{
    if ( axis >= 0 && axis <= 2 )
    {
        m_data->intervals[axis] = interval;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return bounding interval for an axis.
 * @param[in] axis Axis to query.
 * @return Bounding interval for the axis.
 * @sa setInterval()
 * \endif
 * \if CHINESE
 * @brief 返回轴的边界区间。
 * @param[in] axis 要查询的轴。
 * @return 轴的边界区间。
 * @sa setInterval()
 * \endif
 */
QwtInterval QwtMatrixRasterData::interval( Qt::Axis axis ) const
{
    if ( axis >= 0 && axis <= 2 )
        return m_data->intervals[ axis ];

    return QwtInterval();
}

/**
 * \if ENGLISH
 * @brief Assign a value matrix.
 * @details The positions of the values are calculated by dividing the bounding rectangle
 *          of the X/Y intervals into equidistant rectangles (pixels).
 *          Each value corresponds to the center of a pixel.
 * @param[in] values Vector of values.
 * @param[in] numColumns Number of columns.
 * @sa valueMatrix(), numColumns(), numRows(), setInterval()
 * \endif
 * \if CHINESE
 * @brief 分配数值矩阵。
 * @details 数值的位置通过将 X/Y 区间的边界矩形划分为等距矩形（像素）来计算。
 *          每个数值对应于像素的中心。
 * @param[in] values 数值向量。
 * @param[in] numColumns 列数。
 * @sa valueMatrix(), numColumns(), numRows(), setInterval()
 * \endif
 */
void QwtMatrixRasterData::setValueMatrix(
    const QVector< double >& values, int numColumns )
{
    m_data->values = values;
    m_data->numColumns = qMax( numColumns, 0 );
    update();
}

/**
 * \if ENGLISH
 * @brief Return value matrix.
 * @return Value matrix.
 * @sa setValueMatrix(), numColumns(), numRows(), setInterval()
 * \endif
 * \if CHINESE
 * @brief 返回数值矩阵。
 * @return 数值矩阵。
 * @sa setValueMatrix(), numColumns(), numRows(), setInterval()
 * \endif
 */
const QVector< double > QwtMatrixRasterData::valueMatrix() const
{
    return m_data->values;
}

/**
 * \if ENGLISH
 * @brief Change a single value in the matrix.
 * @param[in] row Row index.
 * @param[in] col Column index.
 * @param[in] value New value.
 * @sa value(), setValueMatrix()
 * \endif
 * \if CHINESE
 * @brief 更改矩阵中的单个值。
 * @param[in] row 行索引。
 * @param[in] col 列索引。
 * @param[in] value 新值。
 * @sa value(), setValueMatrix()
 * \endif
 */
void QwtMatrixRasterData::setValue( int row, int col, double value )
{
    if ( row >= 0 && row < m_data->numRows &&
        col >= 0 && col < m_data->numColumns )
    {
        const int index = row * m_data->numColumns + col;
        m_data->values.data()[ index ] = value;
    }
}

/**
 * \if ENGLISH
 * @brief Return number of columns of the value matrix.
 * @return Number of columns.
 * @sa valueMatrix(), numRows(), setValueMatrix()
 * \endif
 * \if CHINESE
 * @brief 返回数值矩阵的列数。
 * @return 列数。
 * @sa valueMatrix(), numRows(), setValueMatrix()
 * \endif
 */
int QwtMatrixRasterData::numColumns() const
{
    return m_data->numColumns;
}

/**
 * \if ENGLISH
 * @brief Return number of rows of the value matrix.
 * @return Number of rows.
 * @sa valueMatrix(), numColumns(), setValueMatrix()
 * \endif
 * \if CHINESE
 * @brief 返回数值矩阵的行数。
 * @return 行数。
 * @sa valueMatrix(), numColumns(), setValueMatrix()
 * \endif
 */
int QwtMatrixRasterData::numRows() const
{
    return m_data->numRows;
}

/**
 * \if ENGLISH
 * @brief Calculate the pixel hint.
 * @details pixelHint() returns the geometry of a pixel, that can be used to calculate
 *          the resolution and alignment of the plot item, that is representing the data.
 *          - NearestNeighbour: pixelHint() returns the surrounding pixel of the top left value in the matrix.
 *          - BilinearInterpolation: Returns an empty rectangle recommending to render in target device resolution.
 * @param[in] area Requested area, ignored.
 * @return Calculated hint.
 * @sa ResampleMode, setMatrix(), setInterval()
 * \endif
 * \if CHINESE
 * @brief 计算像素提示。
 * @details pixelHint() 返回像素的几何信息，可用于计算表示数据的绘图项的分辨率和对齐方式。
 *          - NearestNeighbour：返回矩阵中左上角数值周围的像素。
 *          - BilinearInterpolation：返回空矩形，建议在目标设备分辨率下渲染。
 * @param[in] area 请求的区域，忽略。
 * @return 计算的提示。
 * @sa ResampleMode, setMatrix(), setInterval()
 * \endif
 */
QRectF QwtMatrixRasterData::pixelHint( const QRectF& area ) const
{
    Q_UNUSED( area )

    QRectF rect;
    if ( m_data->resampleMode == NearestNeighbour )
    {
        const QwtInterval intervalX = interval( Qt::XAxis );
        const QwtInterval intervalY = interval( Qt::YAxis );
        if ( intervalX.isValid() && intervalY.isValid() )
        {
            rect = QRectF( intervalX.minValue(), intervalY.minValue(),
                m_data->dx, m_data->dy );
        }
    }

    return rect;
}

/**
 * \if ENGLISH
 * @brief Return the value at a raster position.
 * @param[in] x X value in plot coordinates.
 * @param[in] y Y value in plot coordinates.
 * @return Value at the position.
 * @sa ResampleMode
 * \endif
 * \if CHINESE
 * @brief 返回栅格位置处的数值。
 * @param[in] x 绘图坐标系中的 X 值。
 * @param[in] y 绘图坐标系中的 Y 值。
 * @return 该位置的数值。
 * @sa ResampleMode
 * \endif
 */
double QwtMatrixRasterData::value( double x, double y ) const
{
    const QwtInterval xInterval = interval( Qt::XAxis );
    const QwtInterval yInterval = interval( Qt::YAxis );

    if ( !( xInterval.contains(x) && yInterval.contains(y) ) )
        return qQNaN();

    double value;

    switch( m_data->resampleMode )
    {
        case BicubicInterpolation:
        {
            const double colF = ( x - xInterval.minValue() ) / m_data->dx;
            const double rowF = ( y - yInterval.minValue() ) / m_data->dy;

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

            if ( col2 >= m_data->numColumns )
                col2 = col1;

            if ( col3 >= m_data->numColumns )
                col3 = col2;

            int row0 = row - 2;
            int row1 = row - 1;
            int row2 = row;
            int row3 = row + 1;

            if ( row1 < 0 )
                row1 = row2;

            if ( row0 < 0 )
                row0 = row1;

            if ( row2 >= m_data->numRows )
                row2 = row1;

            if ( row3 >= m_data->numRows )
                row3 = row2;

            // First row
            const double v00 = m_data->value( row0, col0 );
            const double v10 = m_data->value( row0, col1 );
            const double v20 = m_data->value( row0, col2 );
            const double v30 = m_data->value( row0, col3 );

            // Second row
            const double v01 = m_data->value( row1, col0 );
            const double v11 = m_data->value( row1, col1 );
            const double v21 = m_data->value( row1, col2 );
            const double v31 = m_data->value( row1, col3 );

            // Third row
            const double v02 = m_data->value( row2, col0 );
            const double v12 = m_data->value( row2, col1 );
            const double v22 = m_data->value( row2, col2 );
            const double v32 = m_data->value( row2, col3 );

            // Fourth row
            const double v03 = m_data->value( row3, col0 );
            const double v13 = m_data->value( row3, col1 );
            const double v23 = m_data->value( row3, col2 );
            const double v33 = m_data->value( row3, col3 );

            value = qwtBicubicInterpolate(
                v00, v10, v20, v30, v01, v11, v21, v31,
                v02, v12, v22, v32, v03, v13, v23, v33,
                colF - col + 0.5, rowF - row + 0.5 );

            break;
        }
        case BilinearInterpolation:
        {
            int col1 = qRound( ( x - xInterval.minValue() ) / m_data->dx ) - 1;
            int row1 = qRound( ( y - yInterval.minValue() ) / m_data->dy ) - 1;
            int col2 = col1 + 1;
            int row2 = row1 + 1;

            if ( col1 < 0 )
                col1 = col2;
            else if ( col2 >= m_data->numColumns )
                col2 = col1;

            if ( row1 < 0 )
                row1 = row2;
            else if ( row2 >= m_data->numRows )
                row2 = row1;

            const double v11 = m_data->value( row1, col1 );
            const double v21 = m_data->value( row1, col2 );
            const double v12 = m_data->value( row2, col1 );
            const double v22 = m_data->value( row2, col2 );

            const double x2 = xInterval.minValue() + ( col2 + 0.5 ) * m_data->dx;
            const double y2 = yInterval.minValue() + ( row2 + 0.5 ) * m_data->dy;

            const double rx = ( x2 - x ) / m_data->dx;
            const double ry = ( y2 - y ) / m_data->dy;

            const double vr1 = rx * v11 + ( 1.0 - rx ) * v21;
            const double vr2 = rx * v12 + ( 1.0 - rx ) * v22;

            value = ry * vr1 + ( 1.0 - ry ) * vr2;

            break;
        }
        case NearestNeighbour:
        default:
        {
            int row = int( ( y - yInterval.minValue() ) / m_data->dy );
            int col = int( ( x - xInterval.minValue() ) / m_data->dx );

            // In case of intervals, where the maximum is included
            // we get out of bound for row/col, when the value for the
            // maximum is requested. Instead we return the value
            // from the last row/col

            if ( row >= m_data->numRows )
                row = m_data->numRows - 1;

            if ( col >= m_data->numColumns )
                col = m_data->numColumns - 1;

            value = m_data->value( row, col );
        }
    }

    return value;
}

void QwtMatrixRasterData::update()
{
    m_data->numRows = 0;
    m_data->dx = 0.0;
    m_data->dy = 0.0;

    if ( m_data->numColumns > 0 )
    {
        m_data->numRows = m_data->values.size() / m_data->numColumns;

        const QwtInterval xInterval = interval( Qt::XAxis );
        const QwtInterval yInterval = interval( Qt::YAxis );
        if ( xInterval.isValid() )
            m_data->dx = xInterval.width() / m_data->numColumns;
        if ( yInterval.isValid() )
            m_data->dy = yInterval.width() / m_data->numRows;
    }
}
