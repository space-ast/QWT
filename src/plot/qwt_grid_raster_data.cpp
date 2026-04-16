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
    double dxMin, dyMin;  ///< 把x轴和y轴相邻最小值记录下来，用于pixelHint
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
 * \if ENGLISH
 * @brief Constructor.
 * \endif
 * \if CHINESE
 * @brief 构造函数。
 * \endif
 */
QwtGridRasterData::QwtGridRasterData() : m_data(new QwtGridRasterData::PrivateData())
{
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtGridRasterData::~QwtGridRasterData()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the resample method.
 * @details Sets the resample method to be used when querying values.
 * @param[in] mode The resample method to use.
 * @sa resampleMode(), value()
 * \endif
 * \if CHINESE
 * @brief 设置查询值时使用的插值方法。
 * @details 设置查询值时使用的插值方法。
 * @param[in] mode 要使用的插值方法。
 * @sa resampleMode(), value()
 * \endif
 */
void QwtGridRasterData::setResampleMode(QwtGridRasterData::ResampleMode mode)
{
    m_data->gridData.setResampleMode(PrivateData::resampleModeCast(mode));
}

/**
 * \if ENGLISH
 * @brief Get the current resample method.
 * @return The current resample method.
 * @sa setResampleMode(), value()
 * \endif
 * \if CHINESE
 * @brief 返回当前激活的插值方法。
 * @return 当前插值方法。
 * @sa setResampleMode(), value()
 * \endif
 */
QwtGridRasterData::ResampleMode QwtGridRasterData::resampleMode() const
{
    return PrivateData::resampleModeCast(m_data->gridData.resampleMode());
}

/**
 * \if ENGLISH
 * @brief Return bounding interval for an axis
 * @param[in] axis Axis to query (X, Y, or Z)
 * @return Bounding interval for the axis
 * \endif
 * \if CHINESE
 * @brief 返回轴的边界区间
 * @param[in] axis 要查询的轴（X、Y 或 Z）
 * @return 轴的边界区间
 * \endif
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
 * \if ENGLISH
 * @brief Set new x-axis, y-axis, and data matrix
 * @details The data matrix layout:
 *          - data matrix.size = xAxis.size
 *          - data matrix.at(n).size = yAxis.size
 * @param[in] x The x-axis values
 * @param[in] y The y-axis values
 * @param[in] v The 2D data matrix
 * \endif
 * \if CHINESE
 * @brief 设置新的 x 轴、y 轴和数据矩阵
 * @details 数据矩阵布局：
 *          - 数据矩阵.size = xAxis.size
 *          - 数据矩阵.at(n).size = yAxis.size
 * @param[in] x x 轴值
 * @param[in] y y 轴值
 * @param[in] v 二维数据矩阵
 * \endif
 */
void QwtGridRasterData::setValue(const QVector< double >& x, const QVector< double >& y, const QVector< QVector< double > >& v)
{
    m_data->gridData.setValue(x, y, v);
    const QVector< double >& sortedX = m_data->gridData.xAxis();
    const QVector< double >& sortedY = m_data->gridData.yAxis();
    // 计算dxmin和dymin
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
 * \if ENGLISH
 * @brief Return the value at a raster position
 * @param[in] x X value in plot coordinates
 * @param[in] y Y value in plot coordinates
 * @return Value at the position, or NaN if outside bounds
 * \endif
 * \if CHINESE
 * @brief 返回栅格位置处的数值
 * @param[in] x 绘图坐标系中的 X 值
 * @param[in] y 绘图坐标系中的 Y 值
 * @return 该位置的数值，超出边界返回 NaN
 * \endif
 */
double QwtGridRasterData::value(double x, double y) const
{
    return m_data->gridData.value(x, y);
}

/**
 * \if ENGLISH
 * @brief Calculate the pixel hint
 * @details pixelHint() returns the geometry of a pixel that can be used to calculate
 *          the resolution and alignment of the plot item representing the data.
 *          For NearestNeighbour mode, returns the surrounding pixel.
 *          For other modes, returns an empty rectangle.
 * @param[in] area Requested area (ignored)
 * @return Calculated hint rectangle
 * \endif
 * \if CHINESE
 * @brief 计算像素提示
 * @details pixelHint() 返回像素的几何信息，可用于计算表示数据的绘图项的分辨率和对齐方式。
 *          NearestNeighbour 模式返回周围的像素，其他模式返回空矩形。
 * @param[in] area 请求的区域（忽略）
 * @return 计算的提示矩形
 * \endif
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
 * \if ENGLISH
 * @brief Return the size of x-axis.
 * @return Size of x-axis.
 * \endif
 * \if CHINESE
 * @brief 返回 x 轴的大小。
 * @return x 轴的大小。
 * \endif
 */
int QwtGridRasterData::xSize() const
{
    return m_data->gridData.xSize();
}

/**
 * \if ENGLISH
 * @brief Return the size of y-axis.
 * @return Size of y-axis.
 * \endif
 * \if CHINESE
 * @brief 返回 y 轴的大小。
 * @return y 轴的大小。
 * \endif
 */
int QwtGridRasterData::ySize() const
{
    return m_data->gridData.ySize();
}

/**
 * \if ENGLISH
 * @brief Return the size of the value matrix.
 * @return Pair of (xSize, ySize).
 * \endif
 * \if CHINESE
 * @brief 返回数值矩阵的大小。
 * @return (xSize, ySize) 对。
 * \endif
 */
std::pair< int, int > QwtGridRasterData::valueSize() const
{
    return m_data->gridData.valueSize();
}

/**
 * \if ENGLISH
 * @brief Return the value at specified position in the value matrix.
 * @param[in] xIndex X index.
 * @param[in] yIndex Y index.
 * @return Value at the position.
 * \endif
 * \if CHINESE
 * @brief 返回数值矩阵中指定位置的值。
 * @param[in] xIndex X 索引。
 * @param[in] yIndex Y 索引。
 * @return 该位置的值。
 * \endif
 */
double QwtGridRasterData::atValue(int xIndex, int yIndex) const
{
    return m_data->gridData.atValue(xIndex, yIndex);
}

/**
 * \if ENGLISH
 * @brief Return the x-axis value at specified index.
 * @param[in] xIndex X index.
 * @return X-axis value.
 * \endif
 * \if CHINESE
 * @brief 返回指定索引处的 x 轴值。
 * @param[in] xIndex X 索引。
 * @return x 轴值。
 * \endif
 */
double QwtGridRasterData::atX(int xIndex) const
{
    return m_data->gridData.atX(xIndex);
}

/**
 * \if ENGLISH
 * @brief Return the y-axis value at specified index.
 * @param[in] yIndex Y index.
 * @return Y-axis value.
 * \endif
 * \if CHINESE
 * @brief 返回指定索引处的 y 轴值。
 * @param[in] yIndex Y 索引。
 * @return y 轴值。
 * \endif
 */
double QwtGridRasterData::atY(int yIndex) const
{
    return m_data->gridData.atY(yIndex);
}
