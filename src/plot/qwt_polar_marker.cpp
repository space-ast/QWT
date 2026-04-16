/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_marker.h"
#include "qwt_polar.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <qpainter.h>

static const int cs_polarMarker_labelDist = 2;

class QwtPolarMarker::PrivateData
{
public:
    PrivateData() : align(Qt::AlignCenter)
    {
        symbol = new QwtSymbol();
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtText label;
    Qt::Alignment align;
    QPen pen;
    const QwtSymbol* symbol;

    QwtPointPolar pos;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Sets alignment to Qt::AlignCenter, and style to NoLine.
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @details 将对齐方式设置为 Qt::AlignCenter，样式设置为 NoLine。
 * \endif
 */
QwtPolarMarker::QwtPolarMarker() : QwtPolarItem(QwtText("Marker"))
{
    m_data = new PrivateData;

    setItemAttribute(QwtPolarItem::AutoScale);
    setZ(30.0);
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
QwtPolarMarker::~QwtPolarMarker()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Get the runtime type information
 * @return QwtPolarItem::Rtti_PolarMarker
 * \endif
 *
 * \if CHINESE
 * @brief 获取运行时类型信息
 * @return QwtPolarItem::Rtti_PolarMarker
 * \endif
 */
int QwtPolarMarker::rtti() const
{
    return QwtPolarItem::Rtti_PolarMarker;
}

/**
 * \if ENGLISH
 * @brief Get the position of the marker
 * @return Position of the marker
 * @sa setPosition()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标记的位置
 * @return 标记的位置
 * @sa setPosition()
 * \endif
 */
QwtPointPolar QwtPolarMarker::position() const
{
    return m_data->pos;
}

/**
 * \if ENGLISH
 * @brief Set the position of the marker
 * @param[in] pos New position of the marker
 * @sa position()
 * \endif
 *
 * \if CHINESE
 * @brief 设置标记的位置
 * @param[in] pos 标记的新位置
 * @sa position()
 * \endif
 */
void QwtPolarMarker::setPosition(const QwtPointPolar& pos)
{
    if (m_data->pos != pos) {
        m_data->pos = pos;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Draw the marker
 * @param[in] painter Painter
 * @param[in] azimuthMap Maps azimuth values to values related to 0.0, M_2PI
 * @param[in] radialMap Maps radius values into painter coordinates
 * @param[in] pole Position of the pole in painter coordinates
 * @param[in] radius Radius of the complete plot area in painter coordinates
 * @param[in] canvasRect Contents rect of the canvas in painter coordinates
 * \endif
 *
 * \if CHINESE
 * @brief 绘制标记
 * @param[in] painter 绘图器
 * @param[in] azimuthMap 将方位角值映射到与 0.0, M_2PI 相关的值
 * @param[in] radialMap 将半径值映射到绘图器坐标
 * @param[in] pole 绘图器坐标中极点的位置
 * @param[in] radius 绘图器坐标中完整绘图区域的半径
 * @param[in] canvasRect 绘图器坐标中画布的内容矩形
 * \endif
 */
void QwtPolarMarker::draw(QPainter* painter,
                          const QwtScaleMap& azimuthMap,
                          const QwtScaleMap& radialMap,
                          const QPointF& pole,
                          double radius,
                          const QRectF& canvasRect) const
{
    Q_UNUSED(radius);
    Q_UNUSED(canvasRect);

    const double r = radialMap.transform(m_data->pos.radius());
    const double a = azimuthMap.transform(m_data->pos.azimuth());

    const QPointF pos = qwtPolar2Pos(pole, r, a);

    // draw symbol
    QSize sSym(0, 0);
    if (m_data->symbol->style() != QwtSymbol::NoSymbol) {
        sSym = m_data->symbol->size();
        m_data->symbol->drawSymbol(painter, pos);
    }

    // draw label
    if (!m_data->label.isEmpty()) {
        int xlw = qMax(int(m_data->pen.width()), 1);
        int ylw = xlw;

        int xlw1 = qMax((xlw + 1) / 2, (sSym.width() + 1) / 2) + cs_polarMarker_labelDist;
        xlw      = qMax(xlw / 2, (sSym.width() + 1) / 2) + cs_polarMarker_labelDist;
        int ylw1 = qMax((ylw + 1) / 2, (sSym.height() + 1) / 2) + cs_polarMarker_labelDist;
        ylw      = qMax(ylw / 2, (sSym.height() + 1) / 2) + cs_polarMarker_labelDist;

        QRect tr(QPoint(0, 0), m_data->label.textSize(painter->font()).toSize());
        tr.moveCenter(QPoint(0, 0));

        int dx = pos.x();
        int dy = pos.y();

        if (m_data->align & Qt::AlignTop)
            dy += tr.y() - ylw1;
        else if (m_data->align & Qt::AlignBottom)
            dy -= tr.y() - ylw1;

        if (m_data->align & Qt::AlignLeft)
            dx += tr.x() - xlw1;
        else if (m_data->align & Qt::AlignRight)
            dx -= tr.x() - xlw1;

        tr.translate(dx, dy);
        m_data->label.draw(painter, tr);
    }
}

/**
 * \if ENGLISH
 * @brief Assign a symbol
 * @param[in] symbol New symbol (ownership is transferred)
 * @sa symbol(), QwtSymbol
 * \endif
 *
 * \if CHINESE
 * @brief 分配符号
 * @param[in] symbol 新符号（所有权转移）
 * @sa symbol(), QwtSymbol
 * \endif
 */
void QwtPolarMarker::setSymbol(const QwtSymbol* symbol)
{
    if (m_data->symbol != symbol) {
        delete m_data->symbol;
        m_data->symbol = symbol;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the current symbol
 * @return The current symbol
 * @sa setSymbol(), QwtSymbol
 * \endif
 *
 * \if CHINESE
 * @brief 获取当前符号
 * @return 当前符号
 * @sa setSymbol(), QwtSymbol
 * \endif
 */
const QwtSymbol* QwtPolarMarker::symbol() const
{
    return m_data->symbol;
}

/**
 * \if ENGLISH
 * @brief Set the label text
 * @param[in] label Label text
 * @sa label()
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签文本
 * @param[in] label 标签文本
 * @sa label()
 * \endif
 */
void QwtPolarMarker::setLabel(const QwtText& label)
{
    if (label != m_data->label) {
        m_data->label = label;
        itemChanged();
    }
}

/**
 * \if ENGLISH
 * @brief Get the label text
 * @return The label text
 * @sa setLabel()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标签文本
 * @return 标签文本
 * @sa setLabel()
 * \endif
 */
QwtText QwtPolarMarker::label() const
{
    return m_data->label;
}

/**
 * \if ENGLISH
 * @brief Set the alignment of the label
 * @param[in] align Alignment. A combination of AlignTop, AlignBottom, AlignLeft, AlignRight, AlignCenter, AlignHCenter, AlignVCenter.
 * @details The alignment determines where the label is drawn relative to the marker's position.
 * @sa labelAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签的对齐方式
 * @param[in] align 对齐方式。AlignTop、AlignBottom、AlignLeft、AlignRight、AlignCenter、AlignHCenter、AlignVCenter 的组合。
 * @details 对齐方式决定标签相对于标记位置的绘制位置。
 * @sa labelAlignment()
 * \endif
 */
void QwtPolarMarker::setLabelAlignment(Qt::Alignment align)
{
    if (align == m_data->align)
        return;

    m_data->align = align;
    itemChanged();
}

/**
 * \if ENGLISH
 * @brief Get the label alignment
 * @return The label alignment
 * @sa setLabelAlignment()
 * \endif
 *
 * \if CHINESE
 * @brief 获取标签对齐方式
 * @return 标签对齐方式
 * @sa setLabelAlignment()
 * \endif
 */
Qt::Alignment QwtPolarMarker::labelAlignment() const
{
    return m_data->align;
}

/**
 * \if ENGLISH
 * @brief Get the bounding interval necessary to display the item
 * @param[in] scaleId Scale index
 * @return Bounding interval (equals position)
 * @details This interval can be useful for operations like clipping or autoscaling.
 * @sa position()
 * \endif
 *
 * \if CHINESE
 * @brief 获取显示项所需的边界区间
 * @param[in] scaleId 刻度索引
 * @return 边界区间（等于位置）
 * @details 此区间可用于裁剪或自动缩放等操作。
 * @sa position()
 * \endif
 */
QwtInterval QwtPolarMarker::boundingInterval(int scaleId) const
{
    const double v = (scaleId == QwtPolar::ScaleRadius) ? m_data->pos.radius() : m_data->pos.azimuth();

    return QwtInterval(v, v);
}
