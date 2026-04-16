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

#ifndef QWT_POINT_MAPPER_H
#define QWT_POINT_MAPPER_H

#include "qwt_global.h"

class QwtScaleMap;
template< typename T >
class QwtSeriesData;
class QPolygonF;
class QPointF;
class QRectF;
class QPolygon;
class QPen;
class QImage;

/**
 * \if ENGLISH
 * @brief A helper class for translating a series of points
 *
 * @details QwtPointMapper is a collection of methods and optimizations
 *          for translating a series of points into paint device coordinates.
 *          It is used by QwtPlotCurve but might also be useful for
 *          similar plot items displaying a QwtSeriesData<QPointF>.
 * \endif
 *
 * \if CHINESE
 * @brief 用于转换点序列的辅助类
 *
 * @details QwtPointMapper 是一组方法和优化的集合，
 *          用于将点序列转换为绘图设备坐标。
 *          它被 QwtPlotCurve 使用，但对于类似的显示 QwtSeriesData<QPointF> 的绘图项也可能有用。
 * \endif
 */
class QWT_EXPORT QwtPointMapper
{
public:
    /**
     * \if ENGLISH
     * @brief Flags affecting the transformation process
     * @sa setFlag(), setFlags()
     * \endif
     *
     * \if CHINESE
     * @brief 影响转换过程的标志
     * @sa setFlag(), setFlags()
     * \endif
     */
    enum TransformationFlag
    {
        //! \if ENGLISH Round points to integer values \endif \if CHINESE 将点舍入为整数值 \endif
        RoundPoints = 0x01,

        //! \if ENGLISH Try to remove points, that are translated to the same position \endif \if CHINESE 尝试删除被转换到相同位置的点 \endif
        WeedOutPoints = 0x02,

        /**
         * \if ENGLISH
         * @brief An even more aggressive weeding algorithm
         *
         * @details An even more aggressive weeding algorithm, that can be used in toPolygon().
         *          A consecutive chunk of points being mapped to the same x coordinate is reduced to 4 points:
         *          - first point
         *          - point with the minimum y coordinate
         *          - point with the maximum y coordinate
         *          - last point
         *
         *          In the worst case (first and last points are never one of the extremes)
         *          the number of points will be 4 times the width.
         *          As the algorithm is fast it can be used inside of a polyline render cycle.
         * \endif
         *
         * \if CHINESE
         * @brief 更激进的剔除算法
         *
         * @details 更激进的剔除算法，可用于 toPolygon()。
         *          映射到相同 x 坐标的连续点块被减少为 4 个点：
         *          - 第一个点
         *          - y 坐标最小的点
         *          - y 坐标最大的点
         *          - 最后一个点
         *
         *          在最坏情况下（第一个和最后一个点永远不是极值之一）
         *          点数将是宽度的 4 倍。
         *          由于算法速度快，可以在折线渲染周期内使用。
         * \endif
         */
        WeedOutIntermediatePoints = 0x04
    };

    Q_DECLARE_FLAGS(TransformationFlags, TransformationFlag)

    // Constructor
    QwtPointMapper();
    // Destructor
    ~QwtPointMapper();

    // Set the transformation flags
    void setFlags(TransformationFlags);
    // Get the transformation flags
    TransformationFlags flags() const;

    // Set or clear a transformation flag
    void setFlag(TransformationFlag, bool on = true);
    // Test if a transformation flag is set
    bool testFlag(TransformationFlag) const;

    // Set the bounding rectangle for mapping
    void setBoundingRect(const QRectF&);
    // Get the bounding rectangle
    QRectF boundingRect() const;

    // Translate a series of points into a QPolygonF
    QPolygonF
    toPolygonF(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QwtSeriesData< QPointF >* series, int from, int to) const;

    // Translate a series of points into a QPolygon
    QPolygon
    toPolygon(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QwtSeriesData< QPointF >* series, int from, int to) const;

    // Translate a series of points into a QPolygon (scattered points)
    QPolygon
    toPoints(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QwtSeriesData< QPointF >* series, int from, int to) const;

    // Translate a series of points into a QPolygonF (scattered points)
    QPolygonF
    toPointsF(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QwtSeriesData< QPointF >* series, int from, int to) const;

    // Translate a series into a QImage
    QImage toImage(const QwtScaleMap& xMap,
                   const QwtScaleMap& yMap,
                   const QwtSeriesData< QPointF >* series,
                   int from,
                   int to,
                   const QPen&,
                   bool antialiased,
                   uint numThreads) const;

private:
    Q_DISABLE_COPY(QwtPointMapper)

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPointMapper::TransformationFlags)

#endif
