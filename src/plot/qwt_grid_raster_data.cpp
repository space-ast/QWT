/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#include "qwt_grid_raster_data.h"
#include "qwt_interval.h"
#include <QVector>
#include <QRectF>
#include "qwt_grid_data.hpp"
class QwtGridRasterData::PrivateData
{
public:
    using GridData =
        QwtGridData< double, QVector< double >, QVector< double >, QVector< double >, QVector< QVector< double > > >;

public:
    PrivateData() : resampleMode(QwtGridRasterData::NearestNeighbour)
    {
    }

    static QwtGridRasterData::ResampleMode resampleModeCast(GridData::ResampleMode m);
    static GridData::ResampleMode resampleModeCast(QwtGridRasterData::ResampleMode m);

public:
    QwtGridRasterData::ResampleMode resampleMode;
    QwtInterval intervals[ 3 ];
    GridData gridData;
    double dxMin, dyMin;  ///< Store minimum adjacent intervals of x-axis and y-axis for pixelHint
};

QwtGridRasterData::ResampleMode QwtGridRasterData::PrivateData::resampleModeCast(GridData::ResampleMode m)
{
    switch (m) {
    case GridData::NearestNeighbour:
        return QwtGridRasterData::NearestNeighbour;
    case GridData::BilinearInterpolation:
        return QwtGridRasterData::BilinearInterpolation;
    case GridData::BicubicInterpolation:
        return QwtGridRasterData::BicubicInterpolation;
    default:
        break;
    }
    return QwtGridRasterData::NearestNeighbour;
}

QwtGridRasterData::PrivateData::GridData::ResampleMode QwtGridRasterData::PrivateData::resampleModeCast(ResampleMode m)
{
    switch (m) {
    case QwtGridRasterData::NearestNeighbour:
        return GridData::NearestNeighbour;
    case QwtGridRasterData::BilinearInterpolation:
        return GridData::BilinearInterpolation;
    case QwtGridRasterData::BicubicInterpolation:
        return GridData::BicubicInterpolation;
    default:
        break;
    }
    return GridData::NearestNeighbour;
}

/**
 * @brief Constructor.
 */
QwtGridRasterData::QwtGridRasterData() : m_data(new QwtGridRasterData::PrivateData())
{
}

/**
 * @brief Destructor
 */
QwtGridRasterData::~QwtGridRasterData()
{
    delete m_data;
}

/**
 * @brief Set the resample method.
 * @details Sets the resample method to be used when querying values.
 * @param[in] mode The resample method to use.
 * @sa resampleMode(), value()
 */
void QwtGridRasterData::setResampleMode(QwtGridRasterData::ResampleMode mode)
{
    m_data->gridData.setResampleMode(PrivateData::resampleModeCast(mode));
}

/**
 * @brief Get the current resample method.
 * @return The current resample method.
 * @sa setResampleMode(), value()
 */
QwtGridRasterData::ResampleMode QwtGridRasterData::resampleMode() const
{
    return PrivateData::resampleModeCast(m_data->gridData.resampleMode());
}

/**
 * @brief Return bounding interval for an axis
 * @param[in] axis Axis to query (X, Y, or Z)
 * @return Bounding interval for the axis
 */
QwtInterval QwtGridRasterData::interval(Qt::Axis axis) const
{
    switch (axis) {
    case Qt::XAxis:
        return QwtInterval(m_data->gridData.xMin(), m_data->gridData.xMax());
    case Qt::YAxis:
        return QwtInterval(m_data->gridData.yMin(), m_data->gridData.yMax());
    case Qt::ZAxis:
        return QwtInterval(m_data->gridData.dataMin(), m_data->gridData.dataMax());
    default:
        break;
    }
    return QwtInterval(qQNaN(), qQNaN());
}

/**
 * @brief Set new x-axis, y-axis, and data matrix
 * @details The data matrix layout:
 *          - data matrix.size = xAxis.size
 *          - data matrix.at(n).size = yAxis.size
 * @param[in] x The x-axis values
 * @param[in] y The y-axis values
 * @param[in] v The 2D data matrix
 */
void QwtGridRasterData::setValue(const QVector< double >& x, const QVector< double >& y, const QVector< QVector< double > >& v)
{
    m_data->gridData.setValue(x, y, v);
    const QVector< double >& sortedX = m_data->gridData.xAxis();
    const QVector< double >& sortedY = m_data->gridData.yAxis();
    // Calculate dxMin and dyMin
    QVector< double > dx, dy;
    dx.reserve(x.size());
    dy.reserve(y.size());
    for (QVector< double >::size_type i = 1; i < sortedX.size(); ++i) {
        double delta = sortedX[ i ] - sortedX[ i - 1 ];
        dx.push_back(delta);
    }
    for (QVector< double >::size_type i = 1; i < sortedY.size(); ++i) {
        double delta = sortedY[ i ] - sortedY[ i - 1 ];
        dy.push_back(delta);
    }
    if (dx.empty()) {
        m_data->dxMin = 0;
    } else {
        m_data->dxMin = *std::min_element(dx.begin(), dx.end());
    }

    if (dy.empty()) {
        m_data->dyMin = 0;
    } else {
        m_data->dyMin = *std::max_element(dy.begin(), dy.end());
    }
}

/**
 * @brief Return the value at a raster position
 * @param[in] x X value in plot coordinates
 * @param[in] y Y value in plot coordinates
 * @return Value at the position, or NaN if outside bounds
 */
double QwtGridRasterData::value(double x, double y) const
{
    return m_data->gridData.value(x, y);
}

/**
 * @brief Calculate the pixel hint
 * @details pixelHint() returns the geometry of a pixel that can be used to calculate
 *          the resolution and alignment of the plot item representing the data.
 *          For NearestNeighbour mode, returns the surrounding pixel.
 *          For other modes, returns an empty rectangle.
 * @param[in] area Requested area (ignored)
 * @return Calculated hint rectangle
 */
QRectF QwtGridRasterData::pixelHint(const QRectF& area) const
{
    Q_UNUSED(area)

    QRectF rect;
    if (resampleMode() == NearestNeighbour) {
        const QwtInterval intervalX = interval(Qt::XAxis);
        const QwtInterval intervalY = interval(Qt::YAxis);
        if (intervalX.isValid() && intervalY.isValid()) {
            rect = QRectF(intervalX.minValue(), intervalY.minValue(), m_data->dxMin, m_data->dyMin);
        }
    }

    return rect;
}

/**
 * @brief Return the size of x-axis.
 * @return Size of x-axis.
 */
int QwtGridRasterData::xSize() const
{
    return m_data->gridData.xSize();
}

/**
 * @brief Return the size of y-axis.
 * @return Size of y-axis.
 */
int QwtGridRasterData::ySize() const
{
    return m_data->gridData.ySize();
}

/**
 * @brief Return the size of the value matrix.
 * @return Pair of (xSize, ySize).
 */
std::pair< int, int > QwtGridRasterData::valueSize() const
{
    return m_data->gridData.valueSize();
}

/**
 * @brief Return the value at specified position in the value matrix.
 * @param[in] xIndex X index.
 * @param[in] yIndex Y index.
 * @return Value at the position.
 */
double QwtGridRasterData::atValue(int xIndex, int yIndex) const
{
    return m_data->gridData.atValue(xIndex, yIndex);
}

/**
 * @brief Return the x-axis value at specified index.
 * @param[in] xIndex X index.
 * @return X-axis value.
 */
double QwtGridRasterData::atX(int xIndex) const
{
    return m_data->gridData.atX(xIndex);
}

/**
 * @brief Return the y-axis value at specified index.
 * @param[in] yIndex Y index.
 * @return Y-axis value.
 */
double QwtGridRasterData::atY(int yIndex) const
{
    return m_data->gridData.atY(yIndex);
}
