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

#include "qwt_plot_arrowmarker.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qmath.h>

// Endpoint parameters class for managing arrow head and tail properties
class EndpointParams
{
public:
    EndpointParams() : style(QwtPlotArrowMarker::NoEndpoint), size(8.0, 8.0), pathValid(false)
    {
        pen   = QPen(Qt::black, 1.0);
        brush = Qt::transparent;
    }

    EndpointParams(QwtPlotArrowMarker::EndpointStyle s,
                   const QSizeF& sz,
                   const QPen& p   = QPen(Qt::black, 1.0),
                   const QBrush& b = Qt::transparent)
        : style(s), size(sz), pen(p), brush(b), pathValid(false)
    {
    }

    // Getters
    QwtPlotArrowMarker::EndpointStyle getStyle() const
    {
        return style;
    }
    const QSizeF& getSize() const
    {
        return size;
    }
    const QPen& getPen() const
    {
        return pen;
    }
    const QBrush& getBrush() const
    {
        return brush;
    }
    const QPainterPath& getCustomPath() const
    {
        return customPath;
    }
    const QPainterPath& getCachedPath() const
    {
        return cachedPath;
    }
    bool isPathValid() const
    {
        return pathValid;
    }

    // Setters that invalidate cache
    void setStyle(QwtPlotArrowMarker::EndpointStyle s)
    {
        if (style != s) {
            style = s;
            invalidatePath();
        }
    }

    void setSize(const QSizeF& sz)
    {
        if (size != sz) {
            size = sz;
            invalidatePath();
        }
    }

    void setPen(const QPen& p)
    {
        pen = p;
    }
    void setBrush(const QBrush& b)
    {
        brush = b;
    }

    void setCustomPath(const QPainterPath& path)
    {
        if (customPath != path) {
            customPath = path;
            invalidatePath();
        }
    }

    // Cache management
    void invalidatePath()
    {
        pathValid = false;
    }

    void updatePath()
    {
        if (!pathValid) {
            cachedPath = createEndpointPath(style, size, customPath);
            pathValid  = true;
        }
    }

    // Create endpoint path based on style and size
    static QPainterPath
    createEndpointPath(QwtPlotArrowMarker::EndpointStyle style, const QSizeF& size, const QPainterPath& customPath)
    {
        QPainterPath path;

        switch (style) {
        case QwtPlotArrowMarker::ArrowHead:
            //! *1                          -----
            //!     *                        |
            //!         *                   |
            //!    (0,0)     x（0，0）     height
            //!         *                   |
            //!     *                        |
            //! *2                          ------
            //!
            //! |    width     |
            path.moveTo(0, 0);                               // x
            path.lineTo(-size.width(), size.height() / 2);   // 1
            path.moveTo(0, 0);                               // x
            path.lineTo(-size.width(), -size.height() / 2);  // 2
            break;

        case QwtPlotArrowMarker::Circle:
            //!    * *            ---
            //!  *     *           |
            //! *        x(0,0)    height
            //!  *      *          |
            //!    * *            ---
            //! |  width |
            path.addEllipse(QPointF(-size.width() / 2, 0), size.width() / 2, size.height() / 2);
            break;
        case QwtPlotArrowMarker::Square:
            //! *  *  *  *  *          ---
            //! *           *          |
            //! *           x(0,0)    height
            //! *           *          |
            //! *  *  *  *  *          ---
            //! |  width |
            path.addRect(-size.width(), -size.height() / 2, size.width(), size.height());
            break;

        case QwtPlotArrowMarker::Diamond:
            //!      *1
            //!    *   *
            //!  *        *
            //! 2*        x(0,0)    height
            //!  *        *
            //!    *   *
            //!      *3
            //! |  width |
            path.moveTo(0, 0);                                   // x
            path.lineTo(-size.width() / 2, size.height() / 2);   // 1
            path.lineTo(-size.width(), 0);                       // 2
            path.lineTo(-size.width() / 2, -size.height() / 2);  // 3
            path.closeSubpath();
            break;

        case QwtPlotArrowMarker::Triangle:
            //! *1                          -----
            //! *   *                        |
            //! *        *                   |
            //! *   (0,0)     x（0，0）     height
            //! *        *                   |
            //! *   *                        |
            //! *2                          ------
            //!
            //! |    width     |
            path.moveTo(0, 0);                               // x
            path.lineTo(-size.width(), size.height() / 2);   // 1
            path.lineTo(-size.width(), -size.height() / 2);  // 2
            path.closeSubpath();
            break;

        case QwtPlotArrowMarker::CustomPath:
            if (!customPath.isEmpty()) {
                path = customPath;
                // 原点位于路径的最右侧的中间
                QRectF bounds = path.boundingRect();
                if (!bounds.isEmpty()) {
                    QTransform transform;
                    transform.translate(-bounds.right(), -bounds.center().y());  // 将路径的中最右端移动到(0,0)
                    path = transform.map(path);
                }
            }
            break;

        case QwtPlotArrowMarker::NoEndpoint:
        default:
            break;
        }

        return path;
    }

private:
    QwtPlotArrowMarker::EndpointStyle style;
    QSizeF size;
    QPen pen;
    QBrush brush;
    QPainterPath customPath;

    // Cached path for performance optimization
    QPainterPath cachedPath;
    bool pathValid;
};

class QwtPlotArrowMarker::PrivateData
{
public:
    PrivateData()
        : startPoint(0.0, 0.0)
        , endPoint(0.0, 0.0)
        , length(50.0)
        , angle(45.0)
        , positionMode(QwtPlotArrowMarker::ExplicitPoints)
        , headParams(QwtPlotArrowMarker::ArrowHead, QSizeF(10.0, 10.0), QPen(Qt::black, 1.0), Qt::red)
        , tailParams(QwtPlotArrowMarker::NoEndpoint, QSizeF(8.0, 8.0), QPen(Qt::black, 1.0), Qt::blue)
    {
        linePen = QPen(Qt::black, 1.0);
    }

    QPointF startPoint;
    QPointF endPoint;
    double length;
    double angle;
    PositionMode positionMode;

    QPen linePen;

    // Endpoint parameters
    EndpointParams headParams;
    EndpointParams tailParams;

    // Convenience methods for backward compatibility
    void invalidateHeadPath()
    {
        headParams.invalidatePath();
    }
    void invalidateTailPath()
    {
        tailParams.invalidatePath();
    }
    void invalidateAllPaths()
    {
        headParams.invalidatePath();
        tailParams.invalidatePath();
    }

    void updateHeadPath()
    {
        headParams.updatePath();
    }
    void updateTailPath()
    {
        tailParams.updatePath();
    }
};

/**
 * \if ENGLISH
 * @brief Default constructor
 * @details Creates a new QwtPlotArrowMarker with default settings.
 *          The arrow has a length of 50 pixels, angle of 45 degrees,
 *          arrow head style, and default colors.
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数
 * @details 创建一个具有默认设置的新QwtPlotArrowMarker。
 *          箭头长度为50像素，角度为45度，箭头头部样式，使用默认颜色。
 * \endif
 */
QwtPlotArrowMarker::QwtPlotArrowMarker()
{
    m_data = new PrivateData;
    setZ(30.0);
    setItemAttribute(QwtPlotItem::AutoScale, false);
}

/**
 * \if ENGLISH
 * @brief Constructor with title
 * @param[in] title Title of the marker
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数（带标题）
 * @param[in] title 标记的标题
 * \endif
 */
QwtPlotArrowMarker::QwtPlotArrowMarker(const QString& title) : QwtPlotItem(QwtText(title))
{
    m_data = new PrivateData;
    setZ(30.0);
    setItemAttribute(QwtPlotItem::AutoScale, false);
}

/**
 * \if ENGLISH
 * @brief Constructor with QwtText title
 * @param[in] title Title of the marker
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数（带 QwtText 标题）
 * @param[in] title 标记的标题
 * \endif
 */
QwtPlotArrowMarker::QwtPlotArrowMarker(const QwtText& title) : QwtPlotItem(title)
{
    m_data = new PrivateData;
    setZ(30.0);
    setItemAttribute(QwtPlotItem::AutoScale, false);
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
QwtPlotArrowMarker::~QwtPlotArrowMarker()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return The RTTI value for QwtPlotArrowMarker (QwtPlotItem::Rtti_PlotArrowMarker)
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPlotArrowMarker的RTTI值 (QwtPlotItem::Rtti_PlotArrowMarker)
 * \endif
 */
int QwtPlotArrowMarker::rtti() const
{
    return QwtPlotItem::Rtti_PlotArrowMarker;
}

// Position and geometry methods

/**
 * \if ENGLISH
 * @brief Get the start point of the arrow
 * @return The start point in plot coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 获取箭头的起点
 * @return 绘图坐标中的起点
 * \endif
 */
QPointF QwtPlotArrowMarker::startPoint() const
{
    return m_data->startPoint;
}

/**
 * \if ENGLISH
 * @brief Get the end point of the arrow
 * @return End point in plot coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 获取箭头的终点
 * @return 绘图坐标中的终点
 * \endif
 */
QPointF QwtPlotArrowMarker::endPoint() const
{
    if (m_data->positionMode == ExplicitPoints)
        return m_data->endPoint;
    else
        return calculateEndPoint();
}

/**
 * \if ENGLISH
 * @brief Set the start point in plot coordinates
 * @param[in] point Start point in plot coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 设置起点（绘图坐标）
 * @param[in] point 绘图坐标中的起点
 * \endif
 */
void QwtPlotArrowMarker::setStartPoint(const QPointF& point)
{
    if (m_data->startPoint != point) {
        m_data->startPoint = point;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set the end point in plot coordinates
 * @param[in] point End point in plot coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 设置终点（绘图坐标）
 * @param[in] point 绘图坐标中的终点
 * \endif
 */
void QwtPlotArrowMarker::setEndPoint(const QPointF& point)
{
    if (m_data->endPoint != point) {
        m_data->endPoint = point;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set both start and end points in plot coordinates
 * @param[in] start Start point in plot coordinates
 * @param[in] end End point in plot coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 设置起点和终点（绘图坐标）
 * @param[in] start 绘图坐标中的起点
 * @param[in] end 绘图坐标中的终点
 * \endif
 */
void QwtPlotArrowMarker::setPoints(const QPointF& start, const QPointF& end)
{
    bool changed = false;

    if (m_data->startPoint != start) {
        m_data->startPoint = start;
        changed            = true;
    }

    if (m_data->endPoint != end) {
        m_data->endPoint = end;
        changed          = true;
    }

    if (changed) {
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the arrow length in pixels
 * @return Arrow length in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 获取箭头长度（像素）
 * @return 箭头长度（像素）
 * \endif
 */
double QwtPlotArrowMarker::length() const
{
    return m_data->length;
}

/**
 * \if ENGLISH
 * @brief Set the arrow length in pixels
 * @param[in] length The arrow length in pixels (must be non-negative)
 * @details The arrow length is used when position mode is StartLengthAngle.
 *          Invalid values (NaN, Infinity) are rejected with a warning.
 * \endif
 *
 * \if CHINESE
 * @brief 设置箭头长度（像素）
 * @param[in] length 箭头长度（像素，必须为非负数）
 * @details 当定位模式为StartLengthAngle时使用箭头长度。
 *          无效值（NaN、Infinity）会被拒绝并发出警告。
 * \endif
 */
void QwtPlotArrowMarker::setLength(double length)
{
    // Validate input
    if (qIsNaN(length) || qIsInf(length)) {
        qWarning("QwtPlotArrowMarker::setLength: Invalid length value");
        return;
    }

    if (length < 0.0)
        length = 0.0;

    if (m_data->length != length) {
        m_data->length = length;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the rotation angle in degrees
 * @return Rotation angle in degrees
 * \endif
 *
 * \if CHINESE
 * @brief 获取旋转角度（度）
 * @return 旋转角度（度）
 * \endif
 */
double QwtPlotArrowMarker::angle() const
{
    return m_data->angle;
}

/**
 * \if ENGLISH
 * @brief Set the rotation angle in degrees
 * @param[in] angle Rotation angle in degrees
 * \endif
 *
 * \if CHINESE
 * @brief 设置旋转角度（度）
 * @param[in] angle 旋转角度（度）
 * \endif
 */
void QwtPlotArrowMarker::setAngle(double angle)
{
    // Validate input
    if (qIsNaN(angle) || qIsInf(angle)) {
        qWarning("QwtPlotArrowMarker::setAngle: Invalid angle value");
        return;
    }

    // Normalize angle to 0-360 range
    angle = fmod(angle, 360.0);
    if (angle < 0.0)
        angle += 360.0;

    if (m_data->angle != angle) {
        m_data->angle = angle;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the positioning mode
 * @return Positioning mode
 * \endif
 *
 * \if CHINESE
 * @brief 获取定位模式
 * @return 定位模式
 * \endif
 */
QwtPlotArrowMarker::PositionMode QwtPlotArrowMarker::positionMode() const
{
    return m_data->positionMode;
}

/**
 * \if ENGLISH
 * @brief Set the positioning mode
 * @param[in] mode Positioning mode
 * \endif
 *
 * \if CHINESE
 * @brief 设置定位模式
 * @param[in] mode 定位模式
 * \endif
 */
void QwtPlotArrowMarker::setPositionMode(PositionMode mode)
{
    if (m_data->positionMode != mode) {
        m_data->positionMode = mode;
        itemChanged();
    }
}

// Style and appearance methods

/**
 * \if ENGLISH
 * @brief Get the arrow line pen
 * @return Arrow line pen
 * \endif
 *
 * \if CHINESE
 * @brief 获取箭头线条画笔
 * @return 箭头线条画笔
 * \endif
 */
const QPen& QwtPlotArrowMarker::linePen() const
{
    return m_data->linePen;
}

/**
 * \if ENGLISH
 * @brief Set the arrow line pen
 * @param[in] pen Arrow line pen
 * \endif
 *
 * \if CHINESE
 * @brief 设置箭头线条画笔
 * @param[in] pen 箭头线条画笔
 * \endif
 */
void QwtPlotArrowMarker::setLinePen(const QPen& pen)
{
    if (m_data->linePen != pen) {
        m_data->linePen = pen;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Convenience method to set line color and width
 * @param[in] color Line color
 * @param[in] width Line width
 * @param[in] style Line style
 * \endif
 *
 * \if CHINESE
 * @brief 设置线条颜色和宽度的便捷方法
 * @param[in] color 线条颜色
 * @param[in] width 线条宽度
 * @param[in] style 线条样式
 * \endif
 */
void QwtPlotArrowMarker::setLinePen(const QColor& color, qreal width, Qt::PenStyle style)
{
    setLinePen(QPen(color, width, style));
}

/**
 * \if ENGLISH
 * @brief Get the head style
 * @return Head style
 * \endif
 *
 * \if CHINESE
 * @brief 获取头部样式
 * @return 头部样式
 * \endif
 */
QwtPlotArrowMarker::EndpointStyle QwtPlotArrowMarker::headStyle() const
{
    return m_data->headParams.getStyle();
}

/**
 * \if ENGLISH
 * @brief Set the head style
 * @param[in] style Head style
 * \endif
 *
 * \if CHINESE
 * @brief 设置头部样式
 * @param[in] style 头部样式
 * \endif
 */
void QwtPlotArrowMarker::setHeadStyle(EndpointStyle style)
{
    if (m_data->headParams.getStyle() != style) {
        m_data->headParams.setStyle(style);
        m_data->invalidateHeadPath();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the tail style
 * @return Tail style
 * \endif
 *
 * \if CHINESE
 * @brief 获取尾部样式
 * @return 尾部样式
 * \endif
 */
QwtPlotArrowMarker::EndpointStyle QwtPlotArrowMarker::tailStyle() const
{
    return m_data->tailParams.getStyle();
}

/**
 * \if ENGLISH
 * @brief Set the tail style
 * @param[in] style Tail style
 * \endif
 *
 * \if CHINESE
 * @brief 设置尾部样式
 * @param[in] style 尾部样式
 * \endif
 */
void QwtPlotArrowMarker::setTailStyle(EndpointStyle style)
{
    if (m_data->tailParams.getStyle() != style) {
        m_data->tailParams.setStyle(style);
        m_data->invalidateTailPath();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the head size in pixels
 * @return Head size in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 获取头部大小（像素）
 * @return 头部大小（像素）
 * \endif
 */
QSizeF QwtPlotArrowMarker::headSize() const
{
    return m_data->headParams.getSize();
}

/**
 * \if ENGLISH
 * @brief Set the head size in pixels
 * @param[in] size Head size in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 设置头部大小（像素）
 * @param[in] size 头部大小（像素）
 * \endif
 */
void QwtPlotArrowMarker::setHeadSize(const QSizeF& size)
{
    QSizeF newSize = size;
    if (newSize.width() < 0.0)
        newSize.setWidth(0.0);
    if (newSize.height() < 0.0)
        newSize.setHeight(0.0);

    if (m_data->headParams.getSize() != newSize) {
        m_data->headParams.setSize(newSize);
        m_data->invalidateHeadPath();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Convenience method to set head size with equal width and height
 * @param[in] size Head size (both width and height)
 * \endif
 *
 * \if CHINESE
 * @brief 设置头部大小的便捷方法（宽度和高度相等）
 * @param[in] size 头部大小（宽度和高度）
 * \endif
 */
void QwtPlotArrowMarker::setHeadSize(qreal size)
{
    setHeadSize(QSizeF(size, size));
}

/**
 * \if ENGLISH
 * @brief Get the tail size in pixels
 * @return Tail size in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 获取尾部大小（像素）
 * @return 尾部大小（像素）
 * \endif
 */
QSizeF QwtPlotArrowMarker::tailSize() const
{
    return m_data->tailParams.getSize();
}

/**
 * \if ENGLISH
 * @brief Set the tail size in pixels
 * @param[in] size Tail size in pixels
 * \endif
 *
 * \if CHINESE
 * @brief 设置尾部大小（像素）
 * @param[in] size 尾部大小（像素）
 * \endif
 */
void QwtPlotArrowMarker::setTailSize(const QSizeF& size)
{
    QSizeF newSize = size;
    if (newSize.width() < 0.0)
        newSize.setWidth(0.0);
    if (newSize.height() < 0.0)
        newSize.setHeight(0.0);

    if (m_data->tailParams.getSize() != newSize) {
        m_data->tailParams.setSize(newSize);
        m_data->invalidateTailPath();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Convenience method to set tail size with equal width and height
 * @param[in] size Tail size (both width and height)
 * \endif
 *
 * \if CHINESE
 * @brief 设置尾部大小的便捷方法（宽度和高度相等）
 * @param[in] size 尾部大小（宽度和高度）
 * \endif
 */
void QwtPlotArrowMarker::setTailSize(qreal size)
{
    setTailSize(QSizeF(size, size));
}

/**
 * \if ENGLISH
 * @brief Get the head brush
 * @return Head brush
 * \endif
 *
 * \if CHINESE
 * @brief 获取头部画刷
 * @return 头部画刷
 * \endif
 */
const QBrush& QwtPlotArrowMarker::headBrush() const
{
    return m_data->headParams.getBrush();
}

/**
 * \if ENGLISH
 * @brief Set the head brush
 * @param[in] brush Head brush
 * \endif
 *
 * \if CHINESE
 * @brief 设置头部画刷
 * @param[in] brush 头部画刷
 * \endif
 */
void QwtPlotArrowMarker::setHeadBrush(const QBrush& brush)
{
    if (m_data->headParams.getBrush() != brush) {
        m_data->headParams.setBrush(brush);
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the tail brush
 * @return Tail brush
 * \endif
 *
 * \if CHINESE
 * @brief 获取尾部画刷
 * @return 尾部画刷
 * \endif
 */
const QBrush& QwtPlotArrowMarker::tailBrush() const
{
    return m_data->tailParams.getBrush();
}

/**
 * \if ENGLISH
 * @brief Set the tail brush
 * @param[in] brush Tail brush
 * \endif
 *
 * \if CHINESE
 * @brief 设置尾部画刷
 * @param[in] brush 尾部画刷
 * \endif
 */
void QwtPlotArrowMarker::setTailBrush(const QBrush& brush)
{
    if (m_data->tailParams.getBrush() != brush) {
        m_data->tailParams.setBrush(brush);
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the head pen
 * @return Head pen
 * \endif
 *
 * \if CHINESE
 * @brief 获取头部画笔
 * @return 头部画笔
 * \endif
 */
const QPen& QwtPlotArrowMarker::headPen() const
{
    return m_data->headParams.getPen();
}

/**
 * \if ENGLISH
 * @brief Set the head pen
 * @param[in] pen Head pen
 * \endif
 *
 * \if CHINESE
 * @brief 设置头部画笔
 * @param[in] pen 头部画笔
 * \endif
 */
void QwtPlotArrowMarker::setHeadPen(const QPen& pen)
{
    if (m_data->headParams.getPen() != pen) {
        m_data->headParams.setPen(pen);
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the tail pen
 * @return Tail pen
 * \endif
 *
 * \if CHINESE
 * @brief 获取尾部画笔
 * @return 尾部画笔
 * \endif
 */
const QPen& QwtPlotArrowMarker::tailPen() const
{
    return m_data->tailParams.getPen();
}

/**
 * \if ENGLISH
 * @brief Set the tail pen
 * @param[in] pen Tail pen
 * \endif
 *
 * \if CHINESE
 * @brief 设置尾部画笔
 * @param[in] pen 尾部画笔
 * \endif
 */
void QwtPlotArrowMarker::setTailPen(const QPen& pen)
{
    if (m_data->tailParams.getPen() != pen) {
        m_data->tailParams.setPen(pen);
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Set a custom path for head endpoint
 * @param[in] path Custom QPainterPath for head
 * \endif
 *
 * \if CHINESE
 * @brief 设置头部端点的自定义路径
 * @param[in] path 头部的自定义 QPainterPath
 * \endif
 */
void QwtPlotArrowMarker::setHeadCustomPath(const QPainterPath& path)
{
    if (m_data->headParams.getCustomPath() != path) {
        m_data->headParams.setCustomPath(path);
        m_data->invalidateHeadPath();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the custom head path
 * @return Custom head path
 * \endif
 *
 * \if CHINESE
 * @brief 获取自定义头部路径
 * @return 自定义头部路径
 * \endif
 */
QPainterPath QwtPlotArrowMarker::headCustomPath() const
{
    return m_data->headParams.getCustomPath();
}

/**
 * \if ENGLISH
 * @brief Set a custom path for tail endpoint
 * @param[in] path Custom QPainterPath for tail
 * \endif
 *
 * \if CHINESE
 * @brief 设置尾部端点的自定义路径
 * @param[in] path 尾部的自定义 QPainterPath
 * \endif
 */
void QwtPlotArrowMarker::setTailCustomPath(const QPainterPath& path)
{
    if (m_data->tailParams.getCustomPath() != path) {
        m_data->tailParams.setCustomPath(path);
        m_data->invalidateTailPath();
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the custom tail path
 * @return Custom tail path
 * \endif
 *
 * \if CHINESE
 * @brief 获取自定义尾部路径
 * @return 自定义尾部路径
 * \endif
 */
QPainterPath QwtPlotArrowMarker::tailCustomPath() const
{
    return m_data->tailParams.getCustomPath();
}

// Drawing methods

/**
 * \if ENGLISH
 * @brief Draw the arrow marker on the plot
 * @param[in] painter The painter to use for drawing
 * @param[in] xMap X-axis scale map for coordinate transformation
 * @param[in] yMap Y-axis scale map for coordinate transformation
 * @param[in] canvasRect The canvas rectangle in painter coordinates
 * @details This method draws the arrow marker including the line,
 *          head, and tail endpoints. It handles both positioning modes
 *          (ExplicitPoints and StartLengthAngle) and applies proper
 *          rotation to endpoints based on arrow direction.
 * \endif
 *
 * \if CHINESE
 * @brief 在绘图上绘制箭头标记
 * @param[in] painter 用于绘制的画笔
 * @param[in] xMap X轴比例映射用于坐标转换
 * @param[in] yMap Y轴比例映射用于坐标转换
 * @param[in] canvasRect 画布矩形（画笔坐标）
 * @details 此方法绘制箭头标记，包括线条、头部和尾部端点。
 *          它处理两种定位模式（ExplicitPoints和StartLengthAngle），
 *          并根据箭头方向对端点应用适当的旋转。
 * \endif
 */
void QwtPlotArrowMarker::draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const
{
    if (!painter || !painter->isActive())
        return;

    // Convert start point to canvas coordinates
    QPointF canvasStart = toCanvasPoint(m_data->startPoint, xMap, yMap);

    // Calculate or get end point
    QPointF canvasEnd;
    if (m_data->positionMode == ExplicitPoints) {
        canvasEnd = toCanvasPoint(m_data->endPoint, xMap, yMap);
    } else {
        // For StartLengthAngle mode, we need to calculate end point in canvas coordinates
        // Since length is in pixels, we can calculate directly in canvas space
        // 由于长度是像素值，我们可以直接在画布空间中计算
        double angleRad = qDegreesToRadians(m_data->angle);
        canvasEnd =
            canvasStart
            + QPointF(m_data->length * qCos(angleRad),
                      -m_data->length
                          * qSin(angleRad)  // Negative because y increases downward/由于y轴在画布中增加方向，所以需要取负
            );
    }

    // Draw arrow line
    drawArrowLine(painter, canvasStart, canvasEnd);

    // Calculate arrow direction for head rotation
    QPointF direction = canvasEnd - canvasStart;
    double lineAngle  = qRadiansToDegrees(qAtan2(direction.y(), direction.x()));  // qAtan2返回弧度，范围 [-π, π]

    // Draw tail (at start point)
    if (m_data->tailParams.getStyle() != NoEndpoint) {
        // Update cached path if needed
        m_data->updateTailPath();
        drawCachedEndpoint(painter,
                           canvasEnd,
                           m_data->tailParams.getCachedPath(),
                           m_data->tailParams.getSize(),
                           m_data->tailParams.getPen(),
                           m_data->tailParams.getBrush(),
                           lineAngle);
    }

    // Draw head (at end point)
    if (m_data->headParams.getStyle() != NoEndpoint) {
        // Update cached path if needed
        m_data->updateHeadPath();
        drawCachedEndpoint(painter,
                           canvasStart,
                           m_data->headParams.getCachedPath(),
                           m_data->headParams.getSize(),
                           m_data->headParams.getPen(),
                           m_data->headParams.getBrush(),
                           lineAngle + 180.0);
    }
}

/**
 * \if ENGLISH
 * @brief Get the bounding rectangle
 * @return Bounding rectangle
 * \endif
 *
 * \if CHINESE
 * @brief 获取边界矩形
 * @return 边界矩形
 * \endif
 */
QRectF QwtPlotArrowMarker::boundingRect() const
{
    QPointF endPoint = (m_data->positionMode == ExplicitPoints) ? m_data->endPoint : calculateEndPoint();

    QRectF rect(m_data->startPoint, endPoint);
    rect = rect.normalized();

    // Add some margin for endpoint sizes
    double margin = qMax(m_data->headParams.getSize().width(), m_data->headParams.getSize().height());
    margin        = qMax(margin, qMax(m_data->tailParams.getSize().width(), m_data->tailParams.getSize().height()));

    if (margin > 0.0) {
        rect.adjust(-margin, -margin, margin, margin);
    }

    return rect;
}

/**
 * \if ENGLISH
 * @brief Get the legend icon
 * @param[in] index Index of the legend entry
 * @param[in] size Size of the icon
 * @return Legend icon graphic
 * \endif
 *
 * \if CHINESE
 * @brief 获取图例图标
 * @param[in] index 图例条目的索引
 * @param[in] size 图标大小
 * @return 图例图标图形
 * \endif
 */
QwtGraphic QwtPlotArrowMarker::legendIcon(int index, const QSizeF& size) const
{
    Q_UNUSED(index);

    QwtGraphic icon;
    icon.setDefaultSize(size);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing, testRenderHint(QwtPlotItem::RenderAntialiased));

    // Draw a sample arrow in the legend
    QRectF rect(QPointF(0, 0), size);
    rect.adjust(1, 1, -1, -1);  // Add some margin

    QPointF start(rect.left(), rect.center().y());
    QPointF end(rect.right(), rect.center().y());

    // Draw line
    painter.setPen(m_data->linePen);
    painter.drawLine(start, end);

    // Draw head
    if (m_data->headParams.getStyle() != NoEndpoint) {
        QSizeF headSize = m_data->headParams.getSize();
        headSize        = headSize.scaled(rect.height() * 0.6, rect.height() * 0.6, Qt::KeepAspectRatio);

        // Update cached path if needed
        m_data->updateHeadPath();
        drawCachedEndpoint(&painter,
                           end,
                           m_data->headParams.getCachedPath(),
                           headSize,
                           m_data->headParams.getPen(),
                           m_data->headParams.getBrush(),
                           0.0);
    }

    return icon;
}

// Protected methods

//! Draw the arrow line
void QwtPlotArrowMarker::drawArrowLine(QPainter* painter, const QPointF& canvasStart, const QPointF& canvasEnd) const
{
    if (!painter || !painter->isActive())
        return;

    // Check if points are valid
    if (qIsNaN(canvasStart.x()) || qIsNaN(canvasStart.y()) || qIsNaN(canvasEnd.x()) || qIsNaN(canvasEnd.y())) {
        qWarning("QwtPlotArrowMarker::drawArrowLine: Invalid canvas coordinates");
        return;
    }

    if (canvasStart == canvasEnd)
        return;

    painter->save();
    painter->setPen(m_data->linePen);
    painter->drawLine(canvasStart, canvasEnd);
    painter->restore();
}

//! Convert plot coordinates to canvas coordinates
QPointF QwtPlotArrowMarker::toCanvasPoint(const QPointF& plotPoint, const QwtScaleMap& xMap, const QwtScaleMap& yMap) const
{
    return QPointF(xMap.transform(plotPoint.x()), yMap.transform(plotPoint.y()));
}

//! Calculate end point based on start point, length, and angle
QPointF QwtPlotArrowMarker::calculateEndPoint() const
{
    double angleRad = qDegreesToRadians(m_data->angle);
    return m_data->startPoint + QPointF(m_data->length * qCos(angleRad), m_data->length * qSin(angleRad));
}

/**
 * \if ENGLISH
 * @brief Draw a cached endpoint path
 * @param[in] painter The painter to use for drawing
 * @param[in] position The position to draw the endpoint at (canvas coordinates)
 * @param[in] cachedPath The pre-cached QPainterPath for the endpoint
 * @param[in] size The size of the endpoint in pixels
 * @param[in] pen The pen to use for drawing the endpoint outline
 * @param[in] brush The brush to use for filling the endpoint
 * @param[in] rotation The rotation angle in degrees (0 = positive X direction)
 * @details This method draws a cached endpoint path with proper scaling,
 *          positioning, and rotation. It is optimized for performance by
 *          reusing pre-computed QPainterPath objects.
 * \endif
 *
 * \if CHINESE
 * @brief 绘制缓存的端点路径
 * @param[in] painter 用于绘制的画笔
 * @param[in] position 绘制端点的位置（画布坐标）
 * @param[in] cachedPath 预缓存的端点QPainterPath
 * @param[in] size 端点大小（像素）
 * @param[in] pen 用于绘制端点轮廓的画笔
 * @param[in] brush 用于填充端点的画刷
 * @param[in] rotation 旋转角度（度，0 = 正X方向）
 * @details 此方法绘制缓存的端点路径，具有适当的缩放、定位和旋转。
 *          通过重用预计算的QPainterPath对象来优化性能。
 * \endif
 */
void QwtPlotArrowMarker::drawCachedEndpoint(QPainter* painter,
                                            const QPointF& position,
                                            const QPainterPath& cachedPath,
                                            const QSizeF& size,
                                            const QPen& pen,
                                            const QBrush& brush,
                                            double rotation) const
{
    if (!painter || cachedPath.isEmpty() || size.isEmpty())
        return;

    painter->save();

    // Set up pen and brush
    painter->setPen(pen);
    painter->setBrush(brush);

    // Apply rotation if needed
    if (!qFuzzyIsNull(rotation)) {
        painter->translate(position);
        painter->rotate(rotation);
    }

    // Scale and position the cached path
    QPainterPath scaledPath = cachedPath;

    // Get the bounding rectangle of the cached path
    QRectF pathBounds = cachedPath.boundingRect();
    if (!pathBounds.isEmpty()) {
        // Calculate scaling factors to fit the specified size / 计算缩放因子以适应指定大小
        // The cached path is centered at (0, 0) and normalized to unit size / 缓存路径以(0, 0)为中心，归一化到单位大小
        qreal scaleX = size.width() / (pathBounds.width() > 0 ? pathBounds.width() : 1.0);
        qreal scaleY = size.height() / (pathBounds.height() > 0 ? pathBounds.height() : 1.0);

        QTransform transform;
        transform.scale(scaleX, scaleY);

        scaledPath = transform.map(scaledPath);
        painter->drawPath(scaledPath);
    }

    painter->restore();
}