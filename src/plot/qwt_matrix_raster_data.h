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

#ifndef QWT_MATRIX_RASTER_DATA_H
#define QWT_MATRIX_RASTER_DATA_H

#include "qwt_global.h"
#include "qwt_raster_data.h"

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/**
 * \if ENGLISH
 * @brief A class representing a matrix of values as raster data.
 * @details QwtMatrixRasterData implements an interface for a matrix of equidistant values,
 *          that can be used by a QwtPlotRasterItem.
 *          It implements a couple of resampling algorithms, to provide values for positions,
 *          that or not on the value matrix.
 * \endif
 * \if CHINESE
 * @brief 将数值矩阵表示为栅格数据的类。
 * @details QwtMatrixRasterData 为等距数值矩阵实现了接口，
 *          可用于 QwtPlotRasterItem。它实现了多种重采样算法，
 *          为不在数值矩阵上的位置提供数值。
 * \endif
 */
class QWT_EXPORT QwtMatrixRasterData : public QwtRasterData
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

    // Constructor
    QwtMatrixRasterData();
    // Destructor
    virtual ~QwtMatrixRasterData();

    // Set the resampling algorithm
    void setResampleMode(ResampleMode mode);
    // Return the resampling algorithm
    ResampleMode resampleMode() const;

    // Assign the bounding interval for an axis
    void setInterval( Qt::Axis, const QwtInterval& );
    // Return bounding interval for an axis
    virtual QwtInterval interval( Qt::Axis axis) const override final;

    // Assign a value matrix
    void setValueMatrix( const QVector< double >& values, int numColumns );
    // Return value matrix
    const QVector< double > valueMatrix() const;

    // Change a single value in the matrix
    void setValue( int row, int col, double value );

    // Return number of columns of the value matrix
    int numColumns() const;
    // Return number of rows of the value matrix
    int numRows() const;

    // Calculate the pixel hint
    virtual QRectF pixelHint( const QRectF& ) const override;

    // Return the value at a raster position
    virtual double value( double x, double y ) const override;

  private:
    void update();

    class PrivateData;
    PrivateData* m_data;
};

#endif
