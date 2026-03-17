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

#ifndef QWT_PLOT_SPECTROGRAM_H
#define QWT_PLOT_SPECTROGRAM_H

#include "qwt_global.h"
#include "qwt_raster_data.h"
#include "qwt_plot_rasteritem.h"

class QwtColorMap;
template< typename T >
class QList;

/**
 * \if ENGLISH
 * @brief A plot item, which displays a spectrogram
 * @details A spectrogram displays 3-dimensional data, where the 3rd dimension
 *          ( the intensity ) is displayed using colors. The colors are calculated
 *          from the values using a color map.
 * 
 *          On multi-core systems the performance of the image composition
 *          can often be improved by dividing the area into tiles - each of them
 *          rendered in a different thread ( see QwtPlotItem::setRenderThreadCount() ).
 * 
 *          In ContourMode contour lines are painted for the contour levels.
 * 
 * @sa QwtRasterData, QwtColorMap, QwtPlotItem::setRenderThreadCount()
 * \endif
 * 
 * \if CHINESE
 * @brief 显示频谱图的绘图项
 * @details 频谱图显示三维数据，其中第三维（强度）使用颜色显示。颜色是通过颜色映射从值计算得到的。
 * 
 *          在多核系统上，通过将区域划分为多个瓦片（每个瓦片在不同的线程中渲染），
 *          通常可以提高图像合成的性能（参见 QwtPlotItem::setRenderThreadCount()）。
 * 
 *          在 ContourMode 中，会为等高线级别绘制等高线。
 * 
 * @sa QwtRasterData, QwtColorMap, QwtPlotItem::setRenderThreadCount()
 * \endif
 */

class QWT_EXPORT QwtPlotSpectrogram : public QwtPlotRasterItem
{
public:
    /**
     * \if ENGLISH
     * @brief Display modes
     * @details The display mode controls how the raster data will be represented.
     * @sa setDisplayMode(), testDisplayMode()
     * \endif
     * 
     * \if CHINESE
     * @brief 显示模式
     * @details 显示模式控制栅格数据的表示方式。
     * @sa setDisplayMode(), testDisplayMode()
     * \endif
     */

    enum DisplayMode
    {
        /// The values are mapped to colors using a color map.
        ImageMode = 0x01,

        /// The data is displayed using contour lines
        ContourMode = 0x02
    };

    Q_DECLARE_FLAGS(DisplayModes, DisplayMode)

    /// Constructor
    explicit QwtPlotSpectrogram(const QString& title = QString());
    /// Destructor
    virtual ~QwtPlotSpectrogram();

    /// Set a display mode
    void setDisplayMode(DisplayMode, bool on = true);
    /// Test a display mode
    bool testDisplayMode(DisplayMode) const;

    /// Set the raster data
    void setData(QwtRasterData* data);
    /// Get the raster data
    const QwtRasterData* data() const;
    /// Get the raster data
    QwtRasterData* data();

    /// Set the color map
    void setColorMap(QwtColorMap*);
    /// Get the color map
    const QwtColorMap* colorMap() const;

    /// Set the color table size
    void setColorTableSize(int numColors);
    /// Get the color table size
    int colorTableSize() const;

    /// Get the interval for an axis
    virtual QwtInterval interval(Qt::Axis) const override;
    /// Get the pixel hint
    virtual QRectF pixelHint(const QRectF&) const override;

    /// Set the default contour pen
    void setDefaultContourPen(const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine);
    /// Set the default contour pen
    void setDefaultContourPen(const QPen&);
    /// Get the default contour pen
    QPen defaultContourPen() const;

    /// Get the contour pen for a specific level
    virtual QPen contourPen(double level) const;

    /// Set a conrec flag
    void setConrecFlag(QwtRasterData::ConrecFlag, bool on);
    /// Test a conrec flag
    bool testConrecFlag(QwtRasterData::ConrecFlag) const;

    /// Set the contour levels
    void setContourLevels(const QList< double >&);
    /// Get the contour levels
    QList< double > contourLevels() const;

    /// Get the runtime type information
    virtual int rtti() const override;

    /// Draw the spectrogram
    virtual void draw(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;

protected:
    /// Render the image
    virtual QImage renderImage(const QwtScaleMap& xMap,
                               const QwtScaleMap& yMap,
                               const QRectF& area,
                               const QSize& imageSize) const override;

    /// Calculate the contour raster size
    virtual QSize contourRasterSize(const QRectF&, const QRect&) const;

    /// Render the contour lines
    virtual QwtRasterData::ContourLines renderContourLines(const QRectF& rect, const QSize& raster) const;

    /// Draw the contour lines
    virtual void
    drawContourLines(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QwtRasterData::ContourLines&) const;

    /// Render a tile
    void renderTile(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRect& tile, QImage*) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotSpectrogram::DisplayModes)

#endif
