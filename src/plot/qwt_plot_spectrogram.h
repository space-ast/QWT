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

    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    explicit QwtPlotSpectrogram(const QString& title = QString());
    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
    virtual ~QwtPlotSpectrogram();

    /**
     * \if ENGLISH
     * @brief Set a display mode
     * \endif
     */
    void setDisplayMode(DisplayMode, bool on = true);
    /**
     * \if ENGLISH
     * @brief Test a display mode
     * \endif
     */
    bool testDisplayMode(DisplayMode) const;

    /**
     * \if ENGLISH
     * @brief Set the raster data
     * \endif
     */
    void setData(QwtRasterData* data);
    /**
     * \if ENGLISH
     * @brief Get the raster data
     * \endif
     */
    const QwtRasterData* data() const;
    /**
     * \if ENGLISH
     * @brief Get the raster data
     * \endif
     */
    QwtRasterData* data();

    /**
     * \if ENGLISH
     * @brief Set the color map
     * \endif
     */
    void setColorMap(QwtColorMap*);
    /**
     * \if ENGLISH
     * @brief Get the color map
     * \endif
     */
    const QwtColorMap* colorMap() const;

    /**
     * \if ENGLISH
     * @brief Set the color table size
     * \endif
     */
    void setColorTableSize(int numColors);
    /**
     * \if ENGLISH
     * @brief Get the color table size
     * \endif
     */
    int colorTableSize() const;

    /**
     * \if ENGLISH
     * @brief Get the interval for an axis
     * \endif
     */
    virtual QwtInterval interval(Qt::Axis) const override;
    /**
     * \if ENGLISH
     * @brief Get the pixel hint
     * \endif
     */
    virtual QRectF pixelHint(const QRectF&) const override;

    /**
     * \if ENGLISH
     * @brief Set the default contour pen
     * \endif
     */
    void setDefaultContourPen(const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine);
    /**
     * \if ENGLISH
     * @brief Set the default contour pen
     * \endif
     */
    void setDefaultContourPen(const QPen&);
    /**
     * \if ENGLISH
     * @brief Get the default contour pen
     * \endif
     */
    QPen defaultContourPen() const;

    /**
     * \if ENGLISH
     * @brief Get the contour pen for a specific level
     * \endif
     */
    virtual QPen contourPen(double level) const;

    /**
     * \if ENGLISH
     * @brief Set a conrec flag
     * \endif
     */
    void setConrecFlag(QwtRasterData::ConrecFlag, bool on);
    /**
     * \if ENGLISH
     * @brief Test a conrec flag
     * \endif
     */
    bool testConrecFlag(QwtRasterData::ConrecFlag) const;

    /**
     * \if ENGLISH
     * @brief Set the contour levels
     * \endif
     */
    void setContourLevels(const QList< double >&);
    /**
     * \if ENGLISH
     * @brief Get the contour levels
     * \endif
     */
    QList< double > contourLevels() const;

    /**
     * \if ENGLISH
     * @brief Get the runtime type information
     * \endif
     */
    virtual int rtti() const override;

    /**
     * \if ENGLISH
     * @brief Draw the spectrogram
     * \endif
     */
    virtual void draw(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;

protected:
    /**
     * \if ENGLISH
     * @brief Render the image
     * \endif
     */
    virtual QImage renderImage(const QwtScaleMap& xMap,
                               const QwtScaleMap& yMap,
                               const QRectF& area,
                               const QSize& imageSize) const override;

    /**
     * \if ENGLISH
     * @brief Calculate the contour raster size
     * \endif
     */
    virtual QSize contourRasterSize(const QRectF&, const QRect&) const;

    /**
     * \if ENGLISH
     * @brief Render the contour lines
     * \endif
     */
    virtual QwtRasterData::ContourLines renderContourLines(const QRectF& rect, const QSize& raster) const;

    /**
     * \if ENGLISH
     * @brief Draw the contour lines
     * \endif
     */
    virtual void
    drawContourLines(QPainter*, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QwtRasterData::ContourLines&) const;

    /**
     * \if ENGLISH
     * @brief Render a tile
     * \endif
     */
    void renderTile(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRect& tile, QImage*) const;

private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotSpectrogram::DisplayModes)

#endif
