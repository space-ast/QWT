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

#include "qwt_plot_spectrogram.h"
#include "qwt_painter.h"
#include "qwt_interval.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_math.h"

#include <qimage.h>
#include <qpen.h>
#include <qpainter.h>
#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>

#define DEBUG_RENDER 0

#if DEBUG_RENDER
#include <qelapsedtimer.h>
#endif

#include <algorithm>

static inline bool qwtIsNaN(double d)
{
    // qt_is_nan is private header and qIsNaN is not inlined
    // so we need these code here too

    const uchar* ch = (const uchar*)&d;
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return (ch[ 0 ] & 0x7f) == 0x7f && ch[ 1 ] > 0xf0;
    } else {
        return (ch[ 7 ] & 0x7f) == 0x7f && ch[ 6 ] > 0xf0;
    }
}

class QwtPlotSpectrogram::PrivateData
{
public:
    PrivateData() : data(nullptr), colorTableSize(0)
    {
        colorMap    = new QwtLinearColorMap();
        displayMode = ImageMode;

        conrecFlags = QwtRasterData::IgnoreAllVerticesOnLevel;
#if 0
        conrecFlags |= QwtRasterData::IgnoreOutOfRange;
#endif
    }

    ~PrivateData()
    {
        delete data;
        delete colorMap;
    }

    void updateColorTable()
    {
        if (colorMap->format() == QwtColorMap::Indexed) {
            colorTable = colorMap->colorTable256();
        } else {
            if (colorTableSize == 0)
                colorTable.clear();
            else
                colorTable = colorMap->colorTable(colorTableSize);
        }
    }

    QwtRasterData* data;
    QwtColorMap* colorMap;
    DisplayModes displayMode;

    QList< double > contourLevels;
    QPen defaultContourPen;
    QwtRasterData::ConrecFlags conrecFlags;

    int colorTableSize;
    QVector< QRgb > colorTable;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Sets the following item attributes:
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 *          
 *          The z value is initialized by 8.0.
 * @param[in] title Title
 * @sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 * \endif
 * 
 * \if CHINESE
 * @brief 构造函数
 * @details 设置以下项属性：
 *          - QwtPlotItem::AutoScale: true
 *          - QwtPlotItem::Legend: false
 *          
 *          z 值初始化为 8.0。
 * @param[in] title 标题
 * @sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 * \endif
 */
QwtPlotSpectrogram::QwtPlotSpectrogram(const QString& title) : QwtPlotRasterItem(title)
{
    m_data = new PrivateData();

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, false);

    setZ(8.0);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * 
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtPlotSpectrogram::~QwtPlotSpectrogram()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPlotItem::Rtti_PlotSpectrogram
 * \endif
 * 
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotItem::Rtti_PlotSpectrogram
 * \endif
 */
int QwtPlotSpectrogram::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectrogram;
}

/**
 * \if ENGLISH
 * @brief Set a display mode
 * @details The display mode controls how the raster data will be represented.
 *          The default setting enables ImageMode.
 * @param[in] mode Display mode
 * @param[in] on On/Off
 * @sa DisplayMode, testDisplayMode()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置显示模式
 * @details 显示模式控制栅格数据的表示方式。默认设置启用 ImageMode。
 * @param[in] mode 显示模式
 * @param[in] on 开启/关闭
 * @sa DisplayMode, testDisplayMode()
 * \endif
 */
void QwtPlotSpectrogram::setDisplayMode(DisplayMode mode, bool on)
{
    if (on != bool(mode & m_data->displayMode)) {
        if (on)
            m_data->displayMode |= mode;
        else
            m_data->displayMode &= ~mode;
    }

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Test a display mode
 * @details The display mode controls how the raster data will be represented.
 * @param[in] mode Display mode
 * @return true if mode is enabled
 * \endif
 * 
 * \if CHINESE
 * @brief 测试显示模式
 * @details 显示模式控制栅格数据的表示方式。
 * @param[in] mode 显示模式
 * @return 如果模式启用则返回 true
 * \endif
 */
bool QwtPlotSpectrogram::testDisplayMode(DisplayMode mode) const
{
    return (m_data->displayMode & mode);
}

/**
 * \if ENGLISH
 * @brief Change the color map
 * @details Often it is useful to display the mapping between intensities and
 *          colors as an additional plot axis, showing a color bar.
 * @param[in] colorMap Color map
 * @sa colorMap(), QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * \endif
 * 
 * \if CHINESE
 * @brief 更改颜色映射
 * @details 通常将强度和颜色之间的映射显示为额外的绘图轴（显示颜色条）是有用的。
 * @param[in] colorMap 颜色映射
 * @sa colorMap(), QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 * \endif
 */
void QwtPlotSpectrogram::setColorMap(QwtColorMap* colorMap)
{
    if (colorMap == nullptr)
        return;

    if (colorMap != m_data->colorMap) {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }

    m_data->updateColorTable();

    invalidateCache();

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the color map used for mapping intensity values to colors
 * @return Color map used for mapping the intensity values to colors
 * @sa setColorMap()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取用于将强度值映射到颜色的颜色映射
 * @return 用于将强度值映射到颜色的颜色映射
 * @sa setColorMap()
 * \endif
 */
const QwtColorMap* QwtPlotSpectrogram::colorMap() const
{
    return m_data->colorMap;
}

/**
 * \if ENGLISH
 * @brief Limit the number of colors being used by the color map
 * @details When using a color table the mapping from the value into a color
 *          is usually faster as it can be done by simple lookups into a
 *          precalculated color table.
 *          
 *          Setting a table size > 0 enables using a color table, while setting
 *          the size to 0 disables it. The default size = 0, and no color table is used.
 * @param[in] numColors Number of colors. 0 means not using a color table
 * @note The colorTableSize has no effect when using a color table
 *       of QwtColorMap::Indexed, where the size is always 256.
 * @sa QwtColorMap::colorTable(), colorTableSize()
 * \endif
 * 
 * \if CHINESE
 * @brief 限制颜色映射使用的颜色数量
 * @details 使用颜色表时，将值映射到颜色通常更快，因为可以通过简单查找
 *          预计算的颜色表来完成。
 *          
 *          设置表大小 > 0 启用颜色表，设置为 0 则禁用。默认大小 = 0，不使用颜色表。
 * @param[in] numColors 颜色数量。0 表示不使用颜色表
 * @note 当使用 QwtColorMap::Indexed 的颜色表时，colorTableSize 无效，
 *       其中大小始终为 256。
 * @sa QwtColorMap::colorTable(), colorTableSize()
 * \endif
 */
void QwtPlotSpectrogram::setColorTableSize(int numColors)
{
    numColors = qMax(numColors, 0);
    if (numColors != m_data->colorTableSize) {
        m_data->colorTableSize = numColors;
        m_data->updateColorTable();
        invalidateCache();
    }
}

/**
 * \if ENGLISH
 * @brief Get the color table size
 * @return Size of the color table, 0 means not using a color table
 * @sa QwtColorMap::colorTable(), setColorTableSize()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取颜色表大小
 * @return 颜色表大小，0 表示不使用颜色表
 * @sa QwtColorMap::colorTable(), setColorTableSize()
 * \endif
 */
int QwtPlotSpectrogram::colorTableSize() const
{
    return m_data->colorTableSize;
}

/**
 * \if ENGLISH
 * @brief Build and assign the default pen for the contour lines
 * @details In Qt5 the default pen width is 1.0 (0.0 in Qt4) what makes it
 *          non cosmetic (see QPen::isCosmetic()). This method has been introduced
 *          to hide this incompatibility.
 * @param[in] color Pen color
 * @param[in] width Pen width
 * @param[in] style Pen style
 * @sa defaultContourPen(), contourPen()
 * \endif
 * 
 * \if CHINESE
 * @brief 构建并分配等高线的默认画笔
 * @details 在 Qt5 中，默认画笔宽度是 1.0（Qt4 中是 0.0），这使其
 *          成为非装饰性画笔（参见 QPen::isCosmetic()）。此方法被引入
 *          以隐藏这种不兼容性。
 * @param[in] color 画笔颜色
 * @param[in] width 画笔宽度
 * @param[in] style 画笔样式
 * @sa defaultContourPen(), contourPen()
 * \endif
 */
void QwtPlotSpectrogram::setDefaultContourPen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setDefaultContourPen(QPen(color, width, style));
}

/**
 * \if ENGLISH
 * @brief Set the default pen for the contour lines
 * @details If the spectrogram has a valid default contour pen
 *          a contour line is painted using the default contour pen.
 *          Otherwise (pen.style() == Qt::NoPen) the pen is calculated
 *          for each contour level using contourPen().
 * @param[in] pen New default pen
 * @sa defaultContourPen(), contourPen()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置等高线的默认画笔
 * @details 如果频谱图有有效的默认等高线画笔，
 *          等高线将使用默认等高线画笔绘制。
 *          否则（pen.style() == Qt::NoPen），使用 contourPen()
 *          为每个等高线级别计算画笔。
 * @param[in] pen 新的默认画笔
 * @sa defaultContourPen(), contourPen()
 * \endif
 */
void QwtPlotSpectrogram::setDefaultContourPen(const QPen& pen)
{
    if (pen != m_data->defaultContourPen) {
        m_data->defaultContourPen = pen;

        legendChanged();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the default contour pen
 * @return Default contour pen
 * @sa setDefaultContourPen()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取默认等高线画笔
 * @return 默认等高线画笔
 * @sa setDefaultContourPen()
 * \endif
 */
QPen QwtPlotSpectrogram::defaultContourPen() const
{
    return m_data->defaultContourPen;
}

/**
 * \if ENGLISH
 * @brief Calculate the pen for a contour line
 * @details The color of the pen is the color for level calculated by the color map.
 * @param[in] level Contour level
 * @return Pen for the contour line
 * @note contourPen is only used if defaultContourPen().style() == Qt::NoPen
 * @sa setDefaultContourPen(), setColorMap(), setContourLevels()
 * \endif
 * 
 * \if CHINESE
 * @brief 计算等高线的画笔
 * @details 画笔的颜色是由颜色映射计算的级别颜色。
 * @param[in] level 等高线级别
 * @return 等高线的画笔
 * @note contourPen 仅在 defaultContourPen().style() == Qt::NoPen 时使用
 * @sa setDefaultContourPen(), setColorMap(), setContourLevels()
 * \endif
 */
QPen QwtPlotSpectrogram::contourPen(double level) const
{
    if (m_data->data == nullptr || m_data->colorMap == nullptr)
        return QPen();

    const QwtInterval intensityRange = m_data->data->interval(Qt::ZAxis);
    const QColor c(m_data->colorMap->rgb(intensityRange, level));

    return QPen(c);
}

/**
 * \if ENGLISH
 * @brief Modify an attribute of the CONREC algorithm
 * @details Used to calculate the contour lines.
 * @param[in] flag CONREC flag
 * @param[in] on On/Off
 * @sa testConrecFlag(), renderContourLines(), QwtRasterData::contourLines()
 * \endif
 * 
 * \if CHINESE
 * @brief 修改 CONREC 算法的属性
 * @details 用于计算等高线。
 * @param[in] flag CONREC 标志
 * @param[in] on 开启/关闭
 * @sa testConrecFlag(), renderContourLines(), QwtRasterData::contourLines()
 * \endif
 */
void QwtPlotSpectrogram::setConrecFlag(QwtRasterData::ConrecFlag flag, bool on)
{
    if (bool(m_data->conrecFlags & flag) == on)
        return;

    if (on)
        m_data->conrecFlags |= flag;
    else
        m_data->conrecFlags &= ~flag;

    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Test an attribute of the CONREC algorithm
 * @details Used to calculate the contour lines.
 *          The default setting enables QwtRasterData::IgnoreAllVerticesOnLevel.
 * @param[in] flag CONREC flag
 * @return true if enabled
 * @sa setConrecFlag(), renderContourLines(), QwtRasterData::contourLines()
 * \endif
 * 
 * \if CHINESE
 * @brief 测试 CONREC 算法的属性
 * @details 用于计算等高线。
 *          默认设置启用 QwtRasterData::IgnoreAllVerticesOnLevel。
 * @param[in] flag CONREC 标志
 * @return 如果启用则返回 true
 * @sa setConrecFlag(), renderContourLines(), QwtRasterData::contourLines()
 * \endif
 */
bool QwtPlotSpectrogram::testConrecFlag(QwtRasterData::ConrecFlag flag) const
{
    return m_data->conrecFlags & flag;
}

/**
 * \if ENGLISH
 * @brief Set the levels of the contour lines
 * @param[in] levels Values of the contour levels
 * @sa contourLevels(), renderContourLines(), QwtRasterData::contourLines()
 * @note contourLevels returns the same levels but sorted.
 * \endif
 * 
 * \if CHINESE
 * @brief 设置等高线的级别
 * @param[in] levels 等高线级别的值
 * @sa contourLevels(), renderContourLines(), QwtRasterData::contourLines()
 * @note contourLevels 返回相同的级别但已排序。
 * \endif
 */
void QwtPlotSpectrogram::setContourLevels(const QList< double >& levels)
{
    m_data->contourLevels = levels;
    std::sort(m_data->contourLevels.begin(), m_data->contourLevels.end());

    legendChanged();
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the levels of the contour lines
 * @return Levels of the contour lines. The levels are sorted in increasing order.
 * @sa setContourLevels(), renderContourLines(), QwtRasterData::contourLines()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取等高线的级别
 * @return 等高线的级别。级别按递增顺序排序。
 * @sa setContourLevels(), renderContourLines(), QwtRasterData::contourLines()
 * \endif
 */
QList< double > QwtPlotSpectrogram::contourLevels() const
{
    return m_data->contourLevels;
}

/**
 * \if ENGLISH
 * @brief Set the data to be displayed
 * @details The ownership of the data is managed by QwtPlotSpectrogram.
 * @param[in] data Spectrogram data
 * @sa data()
 * \endif
 * 
 * \if CHINESE
 * @brief 设置要显示的数据
 * @details 数据的所有权由 QwtPlotSpectrogram 管理。
 * @param[in] data 频谱图数据
 * @sa data()
 * \endif
 */
void QwtPlotSpectrogram::setData(QwtRasterData* data)
{
    if (data != m_data->data) {
        delete m_data->data;
        m_data->data = data;

        invalidateCache();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the spectrogram data (const version)
 * @return Spectrogram data
 * @sa setData()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取频谱图数据（常量版本）
 * @return 频谱图数据
 * @sa setData()
 * \endif
 */
const QwtRasterData* QwtPlotSpectrogram::data() const
{
    return m_data->data;
}

/**
 * \if ENGLISH
 * @brief Get the spectrogram data
 * @return Spectrogram data
 * @sa setData()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取频谱图数据
 * @return 频谱图数据
 * @sa setData()
 * \endif
 */
QwtRasterData* QwtPlotSpectrogram::data()
{
    return m_data->data;
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval for an axis
 * @details The default implementation returns the interval of the
 *          associated raster data object.
 * @param[in] axis X, Y, or Z axis
 * @return Bounding interval for the axis
 * @sa QwtRasterData::interval()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取轴的边界区间
 * @details 默认实现返回关联栅格数据对象的区间。
 * @param[in] axis X、Y 或 Z 轴
 * @return 轴的边界区间
 * @sa QwtRasterData::interval()
 * \endif
 */
QwtInterval QwtPlotSpectrogram::interval(Qt::Axis axis) const
{
    if (m_data->data == nullptr)
        return QwtInterval();

    return m_data->data->interval(axis);
}

/**
 * \if ENGLISH
 * @brief Get the pixel hint
 * @details The geometry of a pixel is used to calculated the resolution and
 *          alignment of the rendered image.
 *          The default implementation returns data()->pixelHint( rect ).
 * @param[in] area In most implementations the resolution of the data doesn't
 *                 depend on the requested area.
 * @return Bounding rectangle of a pixel
 * @sa QwtPlotRasterItem::pixelHint(), QwtRasterData::pixelHint(), render(), renderImage()
 * \endif
 * 
 * \if CHINESE
 * @brief 获取像素提示
 * @details 像素的几何形状用于计算渲染图像的分辨率和对齐。
 *          默认实现返回 data()->pixelHint( rect )。
 * @param[in] area 在大多数实现中，数据的分辨率不依赖于请求的区域。
 * @return 像素的边界矩形
 * @sa QwtPlotRasterItem::pixelHint(), QwtRasterData::pixelHint(), render(), renderImage()
 * \endif
 */
QRectF QwtPlotSpectrogram::pixelHint(const QRectF& area) const
{
    if (m_data->data == nullptr)
        return QRectF();

    return m_data->data->pixelHint(area);
}

/*!
   \brief Render an image from data and color map.

   For each pixel of area the value is mapped into a color.

   \param xMap X-Scale Map
   \param yMap Y-Scale Map
   \param area Requested area for the image in scale coordinates
   \param imageSize Size of the requested image

   \return A QImage::Format_Indexed8 or QImage::Format_ARGB32 depending
           on the color map.

   \sa QwtRasterData::value(), QwtColorMap::rgb(),
       QwtColorMap::colorIndex()
 */
QImage QwtPlotSpectrogram::renderImage(const QwtScaleMap& xMap,
                                       const QwtScaleMap& yMap,
                                       const QRectF& area,
                                       const QSize& imageSize) const
{
    if (imageSize.isEmpty() || m_data->data == nullptr || m_data->colorMap == nullptr) {
        return QImage();
    }

    const QwtInterval intensityRange = m_data->data->interval(Qt::ZAxis);
    if (!intensityRange.isValid())
        return QImage();

    const QImage::Format format = (m_data->colorMap->format() == QwtColorMap::RGB) ? QImage::Format_ARGB32
                                                                                   : QImage::Format_Indexed8;

    QImage image(imageSize, format);

    if (m_data->colorMap->format() == QwtColorMap::Indexed)
        image.setColorTable(m_data->colorMap->colorTable256());

    m_data->data->initRaster(area, image.size());

#if DEBUG_RENDER
    QElapsedTimer time;
    time.start();
#endif

#if !defined(QT_NO_QFUTURE)
    uint numThreads = renderThreadCount();

    if (numThreads <= 0)
        numThreads = QThread::idealThreadCount();

    if (numThreads <= 0)
        numThreads = 1;

    const int numRows = imageSize.height() / numThreads;

    QVector< QFuture< void > > futures;
    futures.reserve(numThreads - 1);

    for (uint i = 0; i < numThreads; i++) {
        QRect tile(0, i * numRows, image.width(), numRows);
        if (i == numThreads - 1) {
            tile.setHeight(image.height() - i * numRows);
            renderTile(xMap, yMap, tile, &image);
        } else {
            futures += QtConcurrent::run(
#if QT_VERSION >= 0x060000
                &QwtPlotSpectrogram::renderTile,
                this,
#else
                this,
                &QwtPlotSpectrogram::renderTile,
#endif
                xMap,
                yMap,
                tile,
                &image);
        }
    }

    for (int i = 0; i < futures.size(); i++)
        futures[ i ].waitForFinished();

#else
    const QRect tile(0, 0, image.width(), image.height());
    renderTile(xMap, yMap, tile, &image);
#endif

#if DEBUG_RENDER
    const qint64 elapsed = time.elapsed();
    qDebug() << "renderImage" << imageSize << elapsed;
#endif

    m_data->data->discardRaster();

    return image;
}

/*!
    \brief Render a tile of an image.

    Rendering in tiles can be used to composite an image in parallel
    threads.

    \param xMap X-Scale Map
    \param yMap Y-Scale Map
    \param tile Geometry of the tile in image coordinates
    \param image Image to be rendered
 */
void QwtPlotSpectrogram::renderTile(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRect& tile, QImage* image) const
{
    const QwtInterval range = m_data->data->interval(Qt::ZAxis);
    if (range.width() <= 0.0)
        return;

    const bool hasGaps = !m_data->data->testAttribute(QwtRasterData::WithoutGaps);

    if (m_data->colorMap->format() == QwtColorMap::RGB) {
        const int numColors         = m_data->colorTable.size();
        const QRgb* rgbTable        = m_data->colorTable.constData();
        const QwtColorMap* colorMap = m_data->colorMap;

        for (int y = tile.top(); y <= tile.bottom(); y++) {
            const double ty = yMap.invTransform(y);

            QRgb* line = reinterpret_cast< QRgb* >(image->scanLine(y));
            line += tile.left();

            for (int x = tile.left(); x <= tile.right(); x++) {
                const double tx = xMap.invTransform(x);

                const double value = m_data->data->value(tx, ty);

                if (hasGaps && qwtIsNaN(value)) {
                    *line++ = 0u;
                } else if (numColors == 0) {
                    *line++ = colorMap->rgb(range, value);
                } else {
                    const uint index = colorMap->colorIndex(numColors, range, value);
                    *line++          = rgbTable[ index ];
                }
            }
        }
    } else if (m_data->colorMap->format() == QwtColorMap::Indexed) {
        for (int y = tile.top(); y <= tile.bottom(); y++) {
            const double ty = yMap.invTransform(y);

            unsigned char* line = image->scanLine(y);
            line += tile.left();

            for (int x = tile.left(); x <= tile.right(); x++) {
                const double tx = xMap.invTransform(x);

                const double value = m_data->data->value(tx, ty);

                if (hasGaps && qwtIsNaN(value)) {
                    *line++ = 0;
                } else {
                    const uint index = m_data->colorMap->colorIndex(256, range, value);
                    *line++          = static_cast< unsigned char >(index);
                }
            }
        }
    }
}

/*!
   \brief Return the raster to be used by the CONREC contour algorithm.

   A larger size will improve the precision of the CONREC algorithm,
   but will slow down the time that is needed to calculate the lines.

   The default implementation returns rect.size() / 2 bounded to
   the resolution depending on pixelSize().

   \param area Rectangle, where to calculate the contour lines
   \param rect Rectangle in pixel coordinates, where to paint the contour lines
   \return Raster to be used by the CONREC contour algorithm.

   \note The size will be bounded to rect.size().

   \sa drawContourLines(), QwtRasterData::contourLines()
 */
QSize QwtPlotSpectrogram::contourRasterSize(const QRectF& area, const QRect& rect) const
{
    QSize raster = rect.size() / 2;

    const QRectF pixelRect = pixelHint(area);
    if (!pixelRect.isEmpty()) {
        const QSize res(qwtCeil(rect.width() / pixelRect.width()), qwtCeil(rect.height() / pixelRect.height()));
        raster = raster.boundedTo(res);
    }

    return raster;
}

/*!
   Calculate contour lines

   \param rect Rectangle, where to calculate the contour lines
   \param raster Raster, used by the CONREC algorithm
   \return Calculated contour lines

   \sa contourLevels(), setConrecFlag(),
       QwtRasterData::contourLines()
 */
QwtRasterData::ContourLines QwtPlotSpectrogram::renderContourLines(const QRectF& rect, const QSize& raster) const
{
    if (m_data->data == nullptr)
        return QwtRasterData::ContourLines();

    return m_data->data->contourLines(rect, raster, m_data->contourLevels, m_data->conrecFlags);
}

/*!
   Paint the contour lines

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param contourLines Contour lines

   \sa renderContourLines(), defaultContourPen(), contourPen()
 */
void QwtPlotSpectrogram::drawContourLines(QPainter* painter,
                                          const QwtScaleMap& xMap,
                                          const QwtScaleMap& yMap,
                                          const QwtRasterData::ContourLines& contourLines) const
{
    if (m_data->data == nullptr)
        return;

    const int numLevels = m_data->contourLevels.size();
    for (int l = 0; l < numLevels; l++) {
        const double level = m_data->contourLevels[ l ];

        QPen pen = defaultContourPen();
        if (pen.style() == Qt::NoPen)
            pen = contourPen(level);

        if (pen.style() == Qt::NoPen)
            continue;

        painter->setPen(pen);

        const QPolygonF& lines = contourLines[ level ];
        for (int i = 0; i < lines.size(); i += 2) {
            const QPointF p1(xMap.transform(lines[ i ].x()), yMap.transform(lines[ i ].y()));
            const QPointF p2(xMap.transform(lines[ i + 1 ].x()), yMap.transform(lines[ i + 1 ].y()));

            QwtPainter::drawLine(painter, p1, p2);
        }
    }
}

/**
 * \if ENGLISH
 * @brief Draw the spectrogram
 * @param[in] painter Painter
 * @param[in] xMap Maps x-values into pixel coordinates
 * @param[in] yMap Maps y-values into pixel coordinates
 * @param[in] canvasRect Contents rectangle of the canvas in painter coordinates
 * @sa setDisplayMode(), renderImage(), QwtPlotRasterItem::draw(), drawContourLines()
 * \endif
 * 
 * \if CHINESE
 * @brief 绘制频谱图
 * @param[in] painter 绘制器
 * @param[in] xMap 将 x 值映射到像素坐标
 * @param[in] yMap 将 y 值映射到像素坐标
 * @param[in] canvasRect 绘制器坐标中画布的内容矩形
 * @sa setDisplayMode(), renderImage(), QwtPlotRasterItem::draw(), drawContourLines()
 * \endif
 */
void QwtPlotSpectrogram::draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const
{
    if (m_data->displayMode & ImageMode)
        QwtPlotRasterItem::draw(painter, xMap, yMap, canvasRect);

    if (m_data->displayMode & ContourMode) {
        // Add some pixels at the borders
        const int margin = 2;
        QRectF rasterRect(canvasRect.x() - margin,
                          canvasRect.y() - margin,
                          canvasRect.width() + 2 * margin,
                          canvasRect.height() + 2 * margin);

        QRectF area = QwtScaleMap::invTransform(xMap, yMap, rasterRect);

        const QRectF br = boundingRect();
        if (br.isValid()) {
            area &= br;
            if (area.isEmpty())
                return;

            rasterRect = QwtScaleMap::transform(xMap, yMap, area);
        }

        QSize raster = contourRasterSize(area, rasterRect.toRect());
        raster       = raster.boundedTo(rasterRect.toRect().size());
        if (raster.isValid()) {
            const QwtRasterData::ContourLines lines = renderContourLines(area, raster);

            drawContourLines(painter, xMap, yMap, lines);
        }
    }
}
