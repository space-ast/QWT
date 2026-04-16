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

#ifndef QWT_COLOR_MAP_H
#define QWT_COLOR_MAP_H

#include "qwt_global.h"
#include <qcolor.h>

class QwtInterval;

#if QT_VERSION < 0x060000
template< typename T >
class QVector;
#endif

/**
 * \if ENGLISH
 * @brief QwtColorMap is used to map values into colors.
 * @details For displaying 3D data on a 2D plane the 3rd dimension is often
 *          displayed using colors, like f.e in a spectrogram.
 *          Each color map is optimized to return colors for only one of the
 *          following image formats:
 *          - QImage::Format_Indexed8
 *          - QImage::Format_ARGB32
 * @sa QwtPlotSpectrogram, QwtScaleWidget
 * \endif
 * \if CHINESE
 * @brief QwtColorMap 用于将数值映射为颜色。
 * @details 在二维平面上显示三维数据时，第三维通常使用颜色来表示，
 *          例如在光谱图中。每个颜色映射针对以下图像格式之一进行了优化：
 *          - QImage::Format_Indexed8
 *          - QImage::Format_ARGB32
 * @sa QwtPlotSpectrogram, QwtScaleWidget
 * \endif
 */

class QWT_EXPORT QwtColorMap
{
public:
    /*!
        Format for color mapping
        \sa rgb(), colorIndex(), colorTable()
     */

    enum Format
    {
        //! The map is intended to map into RGB values.
        RGB,

        /*!
           Map values into 8 bit values, that
           are used as indexes into the color table.

           Indexed color maps are used to generate QImage::Format_Indexed8
           images. The calculation of the color index is usually faster
           and the resulting image has a lower memory footprint.

           \sa colorIndex(), colorTable()
         */
        Indexed
    };

    explicit QwtColorMap(Format = QwtColorMap::RGB);
    virtual ~QwtColorMap();

    void setFormat(Format);
    Format format() const;

    /// Map a value of a given interval into a RGB value.
    virtual QRgb rgb(const QwtInterval& interval, double value) const = 0;

    virtual uint colorIndex(int numColors, const QwtInterval& interval, double value) const;

    QColor color(const QwtInterval&, double value) const;
    virtual QVector< QRgb > colorTable(int numColors) const;
    virtual QVector< QRgb > colorTable256() const;

private:
    Q_DISABLE_COPY(QwtColorMap)

    Format m_format;
};

/**
 * \if ENGLISH
 * @brief QwtLinearColorMap builds a color map from color stops.
 * @details A color stop is a color at a specific position. The valid
 *          range for the positions is [0.0, 1.0]. When mapping a value
 *          into a color it is translated into this interval according to mode().
 * \endif
 * \if CHINESE
 * @brief QwtLinearColorMap 从颜色停止点构建颜色映射。
 * @details 颜色停止点是指特定位置的颜色。位置的有效范围为 [0.0, 1.0]。
 *          将数值映射为颜色时，根据 mode() 设置将其转换到此区间。
 * \endif
 */
class QWT_EXPORT QwtLinearColorMap : public QwtColorMap
{
public:
    /*!
       Mode of color map
       \sa setMode(), mode()
     */
    enum Mode
    {
        //! Return the color from the next lower color stop
        FixedColors,

        //! Interpolating the colors of the adjacent stops.
        ScaledColors
    };

    explicit QwtLinearColorMap(QwtColorMap::Format = QwtColorMap::RGB);

    QwtLinearColorMap(const QColor& from, const QColor& to, QwtColorMap::Format = QwtColorMap::RGB);

    virtual ~QwtLinearColorMap();

    void setMode(Mode);
    Mode mode() const;

    void setColorInterval(const QColor& color1, const QColor& color2);
    void addColorStop(double value, const QColor&);
    QVector< double > stopPos() const;
    QVector< QColor > stopColors() const;
    QColor color1() const;
    QColor color2() const;

    virtual QRgb rgb(const QwtInterval&, double value) const override;

    virtual uint colorIndex(int numColors, const QwtInterval&, double value) const override;

    class ColorStops;

private:
    class PrivateData;
    PrivateData* m_data;
};

/**
 * \if ENGLISH
 * @brief QwtAlphaColorMap varies the alpha value of a color.
 * \endif
 * \if CHINESE
 * @brief QwtAlphaColorMap 改变颜色的透明度值。
 * \endif
 */
class QWT_EXPORT QwtAlphaColorMap : public QwtColorMap
{
public:
    explicit QwtAlphaColorMap(const QColor& = QColor(Qt::gray));
    virtual ~QwtAlphaColorMap();

    void setAlphaInterval(int alpha1, int alpha2);

    int alpha1() const;
    int alpha2() const;

    void setColor(const QColor&);
    QColor color() const;

    virtual QRgb rgb(const QwtInterval&, double value) const override;

private:
    class PrivateData;
    PrivateData* m_data;
};

/**
 * \if ENGLISH
 * @brief QwtHueColorMap varies the hue value of the HSV color model.
 * @details QwtHueColorMap can be used to set up a color map easily, that runs cyclic over
 *          all colors. Each cycle has 360 different steps.
 *          The values for value and saturation are in the range of 0 to 255 and doesn't
 *          depend on the data value to be mapped.
 * @sa QwtSaturationValueColorMap
 * \endif
 * \if CHINESE
 * @brief QwtHueColorMap 改变 HSV 颜色模型中的色调值。
 * @details QwtHueColorMap 可以轻松设置循环遍历所有颜色的颜色映射。
 *          每个周期有 360 个不同的步进。
 *          value 和 saturation 的值范围为 0 到 255，不依赖于要映射的数据值。
 * @sa QwtSaturationValueColorMap
 * \endif
 */
class QWT_EXPORT QwtHueColorMap : public QwtColorMap
{
public:
    explicit QwtHueColorMap(QwtColorMap::Format = QwtColorMap::RGB);
    virtual ~QwtHueColorMap();

    void setHueInterval(int hue1, int hue2);  // direction ?
    void setSaturation(int saturation);
    void setValue(int value);
    void setAlpha(int alpha);

    int hue1() const;
    int hue2() const;
    int saturation() const;
    int value() const;
    int alpha() const;

    virtual QRgb rgb(const QwtInterval&, double value) const override;

private:
    class PrivateData;
    PrivateData* m_data;
};

/**
 * \if ENGLISH
 * @brief QwtSaturationValueColorMap varies the saturation and/or value for a given hue in the HSV color model.
 * @details Value and saturation are in the range of 0 to 255 while hue is in the range of 0 to 359.
 * @sa QwtHueColorMap
 * \endif
 * \if CHINESE
 * @brief QwtSaturationValueColorMap 改变 HSV 颜色模型中给定色调的饱和度和/或明度值。
 * @details Value 和 saturation 的值范围为 0 到 255，而 hue 的范围为 0 到 359。
 * @sa QwtHueColorMap
 * \endif
 */
class QWT_EXPORT QwtSaturationValueColorMap : public QwtColorMap
{
public:
    QwtSaturationValueColorMap();
    virtual ~QwtSaturationValueColorMap();

    void setHue(int hue);
    void setSaturationInterval(int sat1, int sat2);
    void setValueInterval(int value1, int value2);
    void setAlpha(int alpha);

    int hue() const;
    int saturation1() const;
    int saturation2() const;
    int value1() const;
    int value2() const;
    int alpha() const;

    virtual QRgb rgb(const QwtInterval&, double value) const override;

private:
    class PrivateData;
    PrivateData* m_data;
};

// Map a value into a color.
    inline QColor QwtColorMap::color(const QwtInterval& interval, double value) const
{
    return QColor::fromRgba(rgb(interval, value));
}

// Return the intended format of the color map.
    inline QwtColorMap::Format QwtColorMap::format() const
{
    return m_format;
}

#endif
