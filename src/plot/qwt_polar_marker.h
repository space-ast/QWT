/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_MARKER_H
#define QWT_POLAR_MARKER_H

#include "qwt_global.h"
#include "qwt_polar_item.h"
#include "qwt_point_polar.h"

class QRect;
class QwtText;
class QwtSymbol;

/**
 * \if ENGLISH
 * @brief A class for drawing markers
 * @details A marker can be a a symbol, a label or a combination of them, which can
 *          be drawn around a center point inside a bounding rectangle.
 *
 *          The setSymbol() member assigns a symbol to the marker.
 *          The symbol is drawn at the specified point.
 *
 *          With setLabel(), a label can be assigned to the marker.
 *          The setLabelAlignment() member specifies where the label is
 *          drawn. All the Align*-constants in Qt::AlignmentFlags (see Qt documentation)
 *          are valid. The alignment refers to the center point of
 *          the marker, which means, for example, that the label would be painted
 *          left above the center point if the alignment was set to AlignLeft|AlignTop.
 * \endif
 *
 * \if CHINESE
 * @brief 用于绘制标记的类
 * @details 标记可以是符号、标签或它们的组合，可以在边界矩形内的中心点周围绘制。
 *
 *          setSymbol() 成员为标记分配一个符号。符号在指定的点绘制。
 *
 *          使用 setLabel() 可以为标记分配标签。setLabelAlignment() 成员指定标签的绘制位置。
 *          Qt::AlignmentFlags 中的所有 Align*-常量（参见 Qt 文档）都有效。
 *          对齐方式指的是标记的中心点，例如，如果对齐方式设置为 AlignLeft|AlignTop，
 *          则标签将在中心点的左上方绘制。
 * \endif
 */
class QWT_EXPORT QwtPolarMarker : public QwtPolarItem
{
public:
    /// Constructor
    explicit QwtPolarMarker();
    /// Destructor
    virtual ~QwtPolarMarker();

    /// Get the runtime type information
    virtual int rtti() const override;

    /// Set the position
    void setPosition(const QwtPointPolar&);
    /// Get the position
    QwtPointPolar position() const;

    /// Set the symbol
    void setSymbol(const QwtSymbol* s);
    /// Get the symbol
    const QwtSymbol* symbol() const;

    /// Set the label
    void setLabel(const QwtText&);
    /// Get the label
    QwtText label() const;

    /// Set the label alignment
    void setLabelAlignment(Qt::Alignment);
    /// Get the label alignment
    Qt::Alignment labelAlignment() const;

    /// Draw the marker
    virtual void draw(QPainter* painter,
                      const QwtScaleMap& azimuthMap,
                      const QwtScaleMap& radialMap,
                      const QPointF& pole,
                      double radius,
                      const QRectF& canvasRect) const override;

    /// Get the bounding interval for a scale
    virtual QwtInterval boundingInterval(int scaleId) const override;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
