#ifndef QWT_GRID_RASTER_DATA_H
#define QWT_GRID_RASTER_DATA_H
#include "qwt_global.h"
#include "qwt_raster_data.h"
#if QT_VERSION < 0x060000
template< typename T >
class QVector;
#endif

/**
 * \if ENGLISH
 * @brief A class that encapsulates grid data and provides interpolation methods.
 * @details This class inherits from QwtRasterData and is used to represent 2D grid data.
 *          It supports various interpolation methods such as nearest neighbor and bilinear interpolation.
 * \endif
 * \if CHINESE
 * @brief 封装网格数据并提供插值方法的类。
 * @details 此类继承自 QwtRasterData，用于表示二维网格数据。
 *          它支持多种插值方法，例如最近邻插值和双线性插值。
 * \endif
 */
class QWT_EXPORT QwtGridRasterData : public QwtRasterData
{
public:
    /**
     * \if ENGLISH
     * @brief Resampling algorithm
     * @details The default setting is NearestNeighbour.
     * \endif
     * 
     * \if CHINESE
     * @brief 重采样算法
     * @details 默认设置为 NearestNeighbour。
     * \endif
     */
    enum ResampleMode
    {
        /**
         * \if ENGLISH
         * Return the value from the matrix that is nearest to the requested position.
         * \endif
         * 
         * \if CHINESE
         * 返回矩阵中距离请求位置最近的值。
         * \endif
         */
        NearestNeighbour,

        /**
         * \if ENGLISH
         * Interpolate the value from the distances and values of the 4 surrounding values in the matrix.
         * \endif
         * 
         * \if CHINESE
         * 从矩阵中 4 个相邻值的距离和值进行插值。
         * \endif
         */
        BilinearInterpolation,

        /**
         * \if ENGLISH
         * Interpolate the value from the 16 surrounding values in the matrix using hermite bicubic interpolation.
         * \endif
         * 
         * \if CHINESE
         * 使用 Hermite 双三次插值从矩阵中 16 个相邻值进行插值。
         * \endif
         */
        BicubicInterpolation
    };

public:
    // Constructor
    QwtGridRasterData();
    // Destructor
    virtual ~QwtGridRasterData();

    // Set the resampling algorithm
    void setResampleMode(ResampleMode mode);
    // Return the resampling algorithm
    ResampleMode resampleMode() const;

    // Return bounding interval for an axis
    virtual QwtInterval interval(Qt::Axis axis) const override final;

    // Set new x-axis, y-axis, and data matrix
    void setValue(const QVector< double >& x, const QVector< double >& y, const QVector< QVector< double > >& v);
    // Return the value at a raster position
    virtual double value(double x, double y) const override;

    // Calculate the pixel hint
    virtual QRectF pixelHint(const QRectF&) const override;

    // Return the size of x-axis
    int xSize() const;
    // Return the size of y-axis
    int ySize() const;
    // Return the size of the value matrix
    std::pair< int, int > valueSize() const;

    // Return the value at specified position in the value matrix
    double atValue(int xIndex, int yIndex) const;

    // Return the x-axis value at specified index
    double atX(int xIndex) const;
    // Return the y-axis value at specified index
    double atY(int yIndex) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif  // QWTGRIDRASTERDATA_H
