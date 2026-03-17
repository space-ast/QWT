/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_GRID_H
#define QWT_POLAR_GRID_H

#include "qwt_global.h"
#include "qwt_polar.h"
#include "qwt_polar_item.h"
#include "qwt_polar_plot.h"

class QPainter;
class QPen;
class QwtScaleMap;
class QwtScaleDiv;
class QwtRoundScaleDraw;
class QwtScaleDraw;

/**
 * \if ENGLISH
 * @brief An item which draws scales and grid lines on a polar plot
 * @details The QwtPolarGrid class can be used to draw a coordinate grid.
 *          A coordinate grid consists of major and minor gridlines.
 *          The locations of the gridlines are determined by the azimuth and radial
 *          scale divisions.
 *
 *          QwtPolarGrid is also responsible for drawing the axis representing the
 *          scales. It is possible to display 4 radial and one azimuth axis.
 *
 *          Whenever the scale divisions of the plot widget changes the grid
 *          is synchronized by updateScaleDiv().
 *
 * @sa QwtPolarPlot, QwtPolar::Axis
 * \endif
 *
 * \if CHINESE
 * @brief 在极坐标图上绘制刻度和网格线的绘图项
 * @details QwtPolarGrid 类可用于绘制坐标网格。坐标网格由主次网格线组成。
 *          网格线的位置由方位角和径向刻度分度决定。
 *
 *          QwtPolarGrid 还负责绘制代表刻度的轴。可以显示 4 个径向轴和 1 个方位角轴。
 *
 *          当绘图控件的刻度分度发生变化时，网格会通过 updateScaleDiv() 进行同步。
 *
 * @sa QwtPolarPlot, QwtPolar::Axis
 * \endif
 */
class QWT_EXPORT QwtPolarGrid : public QwtPolarItem
{
public:
    /**
     * \if ENGLISH
     * @brief Display flags to avoid conflicts when painting scales and grid lines
     * @details The default setting enables all flags.
     * @sa setDisplayFlag(), testDisplayFlag()
     * \endif
     *
     * \if CHINESE
     * @brief 绘制刻度和网格线时避免冲突的显示标志
     * @details 默认设置启用所有标志。
     * @sa setDisplayFlag(), testDisplayFlag()
     * \endif
     */
    enum DisplayFlag
    {
        /**
         * \if ENGLISH
         * Try to avoid situations, where the label of the origin is
         * painted over another axis.
         * \endif
         *
         * \if CHINESE
         * 尽量避免原点的标签绘制在另一个轴上的情况。
         * \endif
         */
        SmartOriginLabel = 1,

        /**
         * \if ENGLISH
         * Often the outermost tick of the radial scale is close to the
         * canvas border. With HideMaxRadiusLabel enabled it is not painted.
         * \endif
         *
         * \if CHINESE
         * 径向刻度的最外刻度通常靠近画布边界。启用 HideMaxRadiusLabel 后，它将不被绘制。
         * \endif
         */
        HideMaxRadiusLabel = 2,

        /**
         * \if ENGLISH
         * The tick labels of the radial scales might be hard to read, when
         * they are painted on top of the radial grid lines ( or on top
         * of a curve/spectrogram ). When ClipAxisBackground the bounding rect
         * of each label is added to the clip region.
         * \endif
         *
         * \if CHINESE
         * 当径向刻度的刻度标签绘制在径向网格线（或曲线/光谱图）上时，可能难以阅读。
         * 使用 ClipAxisBackground 时，每个标签的边界矩形将添加到裁剪区域。
         * \endif
         */
        ClipAxisBackground = 4,

        /**
         * \if ENGLISH
         * Don't paint the backbone of the radial axes, when they are very close
         * to a line of the azimuth grid.
         * \endif
         *
         * \if CHINESE
         * 当径向轴的骨干非常靠近方位角网格线时，不绘制它。
         * \endif
         */
        SmartScaleDraw = 8,

        /**
         * \if ENGLISH
         * All grid lines are clipped against the plot area before being painted.
         * When the plot is zoomed in this will have an significant impact
         * on the performance of the painting code.
         * \endif
         *
         * \if CHINESE
         * 所有网格线在绘制之前都针对绘图区域进行裁剪。
         * 当绘图放大时，这将对绘制代码的性能产生显著影响。
         * \endif
         */
        ClipGridLines = 16
    };

    Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag)

    /**
     * \if ENGLISH
     * @brief Grid attributes
     * @sa setGridAttributes(), testGridAttributes()
     * \endif
     *
     * \if CHINESE
     * @brief 网格属性
     * @sa setGridAttributes(), testGridAttributes()
     * \endif
     */
    enum GridAttribute
    {
        /**
         * \if ENGLISH
         * When AutoScaling is enabled, the radial axes will be adjusted
         * to the interval, that is currently visible on the canvas plot.
         * \endif
         *
         * \if CHINESE
         * 启用自动缩放时，径向轴将调整为当前在画布绘图上可见的区间。
         * \endif
         */
        AutoScaling = 0x01
    };

    Q_DECLARE_FLAGS(GridAttributes, GridAttribute)

    /// Constructor
    explicit QwtPolarGrid();
    /// Destructor
    virtual ~QwtPolarGrid();

    /// Get the runtime type information
    virtual int rtti() const override;

    /// Set a display flag
    void setDisplayFlag(DisplayFlag, bool on = true);
    /// Test a display flag
    bool testDisplayFlag(DisplayFlag) const;

    /// Set a grid attribute
    void setGridAttribute(GridAttribute, bool on = true);
    /// Test a grid attribute
    bool testGridAttribute(GridAttribute) const;

    /// Show/hide the grid for a scale
    void showGrid(int scaleId, bool show = true);
    /// Check if the grid is visible for a scale
    bool isGridVisible(int scaleId) const;

    /// Show/hide the minor grid for a scale
    void showMinorGrid(int scaleId, bool show = true);
    /// Check if the minor grid is visible for a scale
    bool isMinorGridVisible(int scaleId) const;

    /// Show/hide an axis
    void showAxis(int axisId, bool show = true);
    /// Check if an axis is visible
    bool isAxisVisible(int axisId) const;

    /// Set the pen
    void setPen(const QPen& p);
    /// Set the font
    void setFont(const QFont&);

    /// Set the major grid pen
    void setMajorGridPen(const QPen& p);
    /// Set the major grid pen for a scale
    void setMajorGridPen(int scaleId, const QPen& p);
    /// Get the major grid pen for a scale
    QPen majorGridPen(int scaleId) const;

    /// Set the minor grid pen
    void setMinorGridPen(const QPen& p);
    /// Set the minor grid pen for a scale
    void setMinorGridPen(int scaleId, const QPen& p);
    /// Get the minor grid pen for a scale
    QPen minorGridPen(int scaleId) const;

    /// Set the axis pen
    void setAxisPen(int axisId, const QPen& p);
    /// Get the axis pen
    QPen axisPen(int axisId) const;

    /// Set the axis font
    void setAxisFont(int axisId, const QFont& p);
    /// Get the axis font
    QFont axisFont(int axisId) const;

    /// Set the scale draw for an axis
    void setScaleDraw(int axisId, QwtScaleDraw*);
    /// Get the scale draw for an axis (const version)
    const QwtScaleDraw* scaleDraw(int axisId) const;
    /// Get the scale draw for an axis
    QwtScaleDraw* scaleDraw(int axisId);

    /// Set the azimuth scale draw
    void setAzimuthScaleDraw(QwtRoundScaleDraw*);
    /// Get the azimuth scale draw (const version)
    const QwtRoundScaleDraw* azimuthScaleDraw() const;
    /// Get the azimuth scale draw
    QwtRoundScaleDraw* azimuthScaleDraw();

    /// Draw the grid
    virtual void draw(QPainter* p,
                      const QwtScaleMap& azimuthMap,
                      const QwtScaleMap& radialMap,
                      const QPointF& pole,
                      double radius,
                      const QRectF& rect) const override;

    /// Update the scale division
    virtual void updateScaleDiv(const QwtScaleDiv& azimuthMap, const QwtScaleDiv& radialMap, const QwtInterval&) override;

    /// Get the margin hint
    virtual int marginHint() const override;

protected:
    /// Draw the rays
    void drawRays(QPainter*,
                  const QRectF&,
                  const QPointF& pole,
                  double radius,
                  const QwtScaleMap& azimuthMap,
                  const QList< double >&) const;
    /// Draw the circles
    void drawCircles(QPainter*, const QRectF&, const QPointF& pole, const QwtScaleMap& radialMap, const QList< double >&) const;

    /// Draw an axis
    void drawAxis(QPainter*, int axisId) const;

private:
    void updateScaleDraws(const QwtScaleMap& azimuthMap,
                          const QwtScaleMap& radialMap,
                          const QPointF& pole,
                          const double radius) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPolarGrid::DisplayFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPolarGrid::GridAttributes)

#endif
