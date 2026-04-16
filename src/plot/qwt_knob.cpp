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

#include "qwt_knob.h"
#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_utils.h"

#include <qpainter.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qmargins.h>
#include <qmath.h>

static QSize qwtKnobSizeHint(const QwtKnob* knob, int min)
{
    int knobWidth = knob->knobWidth();
    if (knobWidth <= 0)
        knobWidth = qMax(3 * knob->markerSize(), min);

    // Add the scale radial thickness to the knobWidth
    const int extent = qwtCeil(knob->scaleDraw()->extent(knob->font()));
    const int d      = 2 * (extent + 4) + knobWidth;

    const QMargins m = knob->contentsMargins();
    return QSize(d + m.left() + m.right(), d + m.top() + m.bottom());
}

static inline double qwtToScaleAngle(double angle)
{
    // the map is counter clockwise with the origin
    // at 90° using angles from -180° -> 180°

    double a = 90.0 - angle;
    if (a <= -180.0)
        a += 360.0;
    else if (a >= 180.0)
        a -= 360.0;

    return a;
}

static double qwtToDegrees(double value)
{
    return qwtNormalizeDegrees(90.0 - value);
}

class QwtKnob::PrivateData
{
public:
    PrivateData()
        : knobStyle(QwtKnob::Raised)
        , markerStyle(QwtKnob::Notch)
        , borderWidth(2)
        , borderDist(4)
        , scaleDist(4)
        , maxScaleTicks(11)
        , knobWidth(0)
        , alignment(Qt::AlignCenter)
        , markerSize(8)
        , totalAngle(270.0)
        , mouseOffset(0.0)
    {
    }

    QwtKnob::KnobStyle knobStyle;
    QwtKnob::MarkerStyle markerStyle;

    int borderWidth;
    int borderDist;
    int scaleDist;
    int maxScaleTicks;
    int knobWidth;
    Qt::Alignment alignment;
    int markerSize;

    double totalAngle;

    double mouseOffset;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Construct a knob with an angle of 270°. The style is
 *          QwtKnob::Raised and the marker style is QwtKnob::Notch.
 *          The width of the knob is set to 50 pixels.
 * @param[in] parent Parent widget
 * \sa setTotalAngle()
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @details 构造一个角度为 270° 的旋钮。样式为 QwtKnob::Raised，
 *          标记样式为 QwtKnob::Notch。旋钮宽度设置为 50 像素。
 * @param[in] parent 父控件
 * \sa setTotalAngle()
 * \endif
 */
QwtKnob::QwtKnob(QWidget* parent) : QwtAbstractSlider(parent)
{
    m_data = new PrivateData;

    setScaleDraw(new QwtRoundScaleDraw());

    setTotalAngle(270.0);

    setScale(0.0, 10.0);
    setValue(0.0);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

//! Destructor
QwtKnob::~QwtKnob()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Set the knob type
 * @param[in] knobStyle Knob type
 * \sa knobStyle(), setBorderWidth()
 * \endif
 * \if CHINESE
 * @brief 设置旋钮类型
 * @param[in] knobStyle 旋钮类型
 * \sa knobStyle(), setBorderWidth()
 * \endif
 */
void QwtKnob::setKnobStyle(KnobStyle knobStyle)
{
    if (m_data->knobStyle != knobStyle) {
        m_data->knobStyle = knobStyle;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return the knob style
 * @return Knob type
 * \sa setKnobStyle(), setBorderWidth()
 * \endif
 * \if CHINESE
 * @brief 返回旋钮样式
 * @return 旋钮类型
 * \sa setKnobStyle(), setBorderWidth()
 * \endif
 */
QwtKnob::KnobStyle QwtKnob::knobStyle() const
{
    return m_data->knobStyle;
}

/**
 * \if ENGLISH
 * @brief Set the marker type of the knob
 * @param[in] markerStyle Marker type
 * \sa markerStyle(), setMarkerSize()
 * \endif
 * \if CHINESE
 * @brief 设置旋钮的标记类型
 * @param[in] markerStyle 标记类型
 * \sa markerStyle(), setMarkerSize()
 * \endif
 */
void QwtKnob::setMarkerStyle(MarkerStyle markerStyle)
{
    if (m_data->markerStyle != markerStyle) {
        m_data->markerStyle = markerStyle;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return the marker style
 * @return Marker type of the knob
 * \sa setMarkerStyle(), setMarkerSize()
 * \endif
 * \if CHINESE
 * @brief 返回标记样式
 * @return 旋钮的标记类型
 * \sa setMarkerStyle(), setMarkerSize()
 * \endif
 */
QwtKnob::MarkerStyle QwtKnob::markerStyle() const
{
    return m_data->markerStyle;
}

/**
 * \if ENGLISH
 * @brief Set the total angle by which the knob can be turned
 * @details The angle has to be between [10, 360] degrees.
 *          Angles above 360 (so that the knob can be turned several times around its axis)
 *          have to be set using setNumTurns().
 *          The default angle is 270 degrees.
 * @param[in] angle Angle in degrees
 * \sa totalAngle(), setNumTurns()
 * \endif
 * \if CHINESE
 * @brief 设置旋钮可转动的总角度
 * @details 角度必须在 [10, 360] 度之间。
 *          超过 360 度的角度（使旋钮可绕轴转动多圈）需使用 setNumTurns() 设置。
 *          默认角度为 270 度。
 * @param[in] angle 角度（度）
 * \sa totalAngle(), setNumTurns()
 * \endif
 */
void QwtKnob::setTotalAngle(double angle)
{
    angle = qBound(10.0, angle, 360.0);

    if (angle != m_data->totalAngle) {
        m_data->totalAngle = angle;

        scaleDraw()->setAngleRange(-0.5 * m_data->totalAngle, 0.5 * m_data->totalAngle);

        updateGeometry();
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return the total angle
 * @return Total angle in degrees
 * \sa setTotalAngle(), setNumTurns(), numTurns()
 * \endif
 * \if CHINESE
 * @brief 返回总角度
 * @return 总角度（度）
 * \sa setTotalAngle(), setNumTurns(), numTurns()
 * \endif
 */
double QwtKnob::totalAngle() const
{
    return m_data->totalAngle;
}

/**
 * \if ENGLISH
 * @brief Set the number of turns
 * @details When numTurns > 1 the knob can be turned several times around its axis.
 *          Otherwise the total angle is floored to 360°.
 * @param[in] numTurns Number of turns
 * \sa numTurns(), totalAngle(), setTotalAngle()
 * \endif
 * \if CHINESE
 * @brief 设置圈数
 * @details 当 numTurns > 1 时，旋钮可绕轴转动多圈。否则总角度限制为 360°。
 * @param[in] numTurns 圈数
 * \sa numTurns(), totalAngle(), setTotalAngle()
 * \endif
 */
void QwtKnob::setNumTurns(int numTurns)
{
    numTurns = qMax(numTurns, 1);

    if (numTurns == 1 && m_data->totalAngle <= 360.0)
        return;

    const double angle = numTurns * 360.0;
    if (angle != m_data->totalAngle) {
        m_data->totalAngle = angle;

        scaleDraw()->setAngleRange(-0.5 * m_data->totalAngle, 0.5 * m_data->totalAngle);

        updateGeometry();
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return the number of turns
 * @return Number of turns. When the total angle is below 360°, numTurns() is ceiled to 1.
 * \sa setNumTurns(), setTotalAngle(), totalAngle()
 * \endif
 * \if CHINESE
 * @brief 返回圈数
 * @return 圈数。当总角度低于 360° 时，numTurns() 向上取整为 1。
 * \sa setNumTurns(), setTotalAngle(), totalAngle()
 * \endif
 */
int QwtKnob::numTurns() const
{
    return qwtCeil(m_data->totalAngle / 360.0);
}

/**
 * \if ENGLISH
 * @brief Change the scale draw of the knob
 * @details For changing the labels of the scales, it is necessary to derive
 *          from QwtRoundScaleDraw and overload QwtRoundScaleDraw::label().
 * @param[in] scaleDraw Scale draw object
 * \sa scaleDraw()
 * \endif
 * \if CHINESE
 * @brief 更改旋钮的刻度绘制器
 * @details 要更改刻度标签，需要从 QwtRoundScaleDraw 派生并重载 QwtRoundScaleDraw::label()。
 * @param[in] scaleDraw 刻度绘制对象
 * \sa scaleDraw()
 * \endif
 */
void QwtKnob::setScaleDraw(QwtRoundScaleDraw* scaleDraw)
{
    setAbstractScaleDraw(scaleDraw);
    setTotalAngle(m_data->totalAngle);

    updateGeometry();
    update();
}

/**
 * \if ENGLISH
 * @brief Return the scale draw of the knob (const version)
 * @return The scale draw
 * \sa setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回旋钮的刻度绘制器 (const 版本)
 * @return 刻度绘制器
 * \sa setScaleDraw()
 * \endif
 */
const QwtRoundScaleDraw* QwtKnob::scaleDraw() const
{
    return static_cast< const QwtRoundScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Return the scale draw of the knob (non-const version)
 * @return The scale draw
 * \sa setScaleDraw()
 * \endif
 * \if CHINESE
 * @brief 返回旋钮的刻度绘制器 (非 const 版本)
 * @return 刻度绘制器
 * \sa setScaleDraw()
 * \endif
 */
QwtRoundScaleDraw* QwtKnob::scaleDraw()
{
    return static_cast< QwtRoundScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Calculate the bounding rectangle of the knob without the scale
 * @return Bounding rectangle of the knob
 * \sa knobWidth(), alignment(), QWidget::contentsRect()
 * \endif
 * \if CHINESE
 * @brief 计算不含刻度的旋钮边界矩形
 * @return 旋钮的边界矩形
 * \sa knobWidth(), alignment(), QWidget::contentsRect()
 * \endif
 */
QRect QwtKnob::knobRect() const
{
    const QRect cr = contentsRect();

    const int extent = qwtCeil(scaleDraw()->extent(font()));
    const int d      = extent + m_data->scaleDist;

    int w = m_data->knobWidth;
    if (w <= 0) {
        const int dim = qMin(cr.width(), cr.height());

        w = dim - 2 * (d);
        w = qMax(0, w);
    }

    QRect r(0, 0, w, w);

    if (m_data->alignment & Qt::AlignLeft) {
        r.moveLeft(cr.left() + d);
    } else if (m_data->alignment & Qt::AlignRight) {
        r.moveRight(cr.right() - d);
    } else {
        r.moveCenter(QPoint(cr.center().x(), r.center().y()));
    }

    if (m_data->alignment & Qt::AlignTop) {
        r.moveTop(cr.top() + d);
    } else if (m_data->alignment & Qt::AlignBottom) {
        r.moveBottom(cr.bottom() - d);
    } else {
        r.moveCenter(QPoint(r.center().x(), cr.center().y()));
    }

    return r;
}

/*!
   \brief Determine what to do when the user presses a mouse button.

   \param pos Mouse position

   \retval True, when pos is inside the circle of the knob.
   \sa scrolledTo()
 */
bool QwtKnob::isScrollPosition(const QPoint& pos) const
{
    const QRect kr = knobRect();

    const QRegion region(kr, QRegion::Ellipse);
    if (region.contains(pos) && (pos != kr.center())) {
        const double angle      = QLineF(kr.center(), pos).angle();
        const double valueAngle = qwtToDegrees(scaleMap().transform(value()));

        m_data->mouseOffset = qwtNormalizeDegrees(angle - valueAngle);

        return true;
    }

    return false;
}

/*!
   \brief Determine the value for a new position of the mouse

   \param pos Mouse position

   \return Value for the mouse position
   \sa isScrollPosition()
 */
double QwtKnob::scrolledTo(const QPoint& pos) const
{
    double angle = QLineF(rect().center(), pos).angle();
    angle        = qwtNormalizeDegrees(angle - m_data->mouseOffset);

    if (scaleMap().pDist() > 360.0) {
        angle = qwtToDegrees(angle);

        const double v = scaleMap().transform(value());

        int numTurns = qwtFloor((v - scaleMap().p1()) / 360.0);

        double valueAngle = qwtNormalizeDegrees(v);
        if (qAbs(valueAngle - angle) > 180.0) {
            numTurns += (angle > valueAngle) ? -1 : 1;
        }

        angle += scaleMap().p1() + numTurns * 360.0;

        if (!wrapping()) {
            const double boundedAngle = qBound(scaleMap().p1(), angle, scaleMap().p2());

            m_data->mouseOffset += (boundedAngle - angle);
            angle = boundedAngle;
        }
    } else {
        angle = qwtToScaleAngle(angle);

        double boundedAngle = qBound(scaleMap().p1(), angle, scaleMap().p2());

        if (!wrapping()) {
            const double currentAngle = scaleMap().transform(value());

            if ((currentAngle > 90.0) && (boundedAngle < -90.0))
                boundedAngle = scaleMap().p2();
            else if ((currentAngle < -90.0) && (boundedAngle > 90.0))
                boundedAngle = scaleMap().p1();

            m_data->mouseOffset += (boundedAngle - angle);
        }

        angle = boundedAngle;
    }

    return scaleMap().invTransform(angle);
}

/*!
   Handle QEvent::StyleChange and QEvent::FontChange;
   \param event Change event
 */
void QwtKnob::changeEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::StyleChange:
    case QEvent::FontChange: {
        updateGeometry();
        update();
        break;
    }
    default:
        break;
    }
}

/*!
   Repaint the knob
   \param event Paint event
 */
void QwtKnob::paintEvent(QPaintEvent* event)
{
    const QRectF knobRect = this->knobRect();

    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::Antialiasing, true);

    if (!knobRect.contains(event->region().boundingRect())) {
        scaleDraw()->setRadius(0.5 * knobRect.width() + m_data->scaleDist);
        scaleDraw()->moveCenter(knobRect.center());

        scaleDraw()->draw(&painter, palette());
    }

    drawKnob(&painter, knobRect);

    drawMarker(&painter, knobRect, qwtNormalizeDegrees(scaleMap().transform(value())));

    painter.setRenderHint(QPainter::Antialiasing, false);

    if (hasFocus())
        drawFocusIndicator(&painter);
}

/*!
   \brief Draw the knob

   \param painter painter
   \param knobRect Bounding rectangle of the knob (without scale)
 */
void QwtKnob::drawKnob(QPainter* painter, const QRectF& knobRect) const
{
    double dim = qMin(knobRect.width(), knobRect.height());
    dim -= m_data->borderWidth * 0.5;

    QRectF aRect(0, 0, dim, dim);
    aRect.moveCenter(knobRect.center());

    QPen pen(Qt::NoPen);
    if (m_data->borderWidth > 0) {
        QColor c1 = palette().color(QPalette::Light);
        QColor c2 = palette().color(QPalette::Dark);

        QLinearGradient gradient(aRect.topLeft(), aRect.bottomRight());
        gradient.setColorAt(0.0, c1);
        gradient.setColorAt(0.3, c1);
        gradient.setColorAt(0.7, c2);
        gradient.setColorAt(1.0, c2);

        pen = QPen(gradient, m_data->borderWidth);
    }

    QBrush brush;
    switch (m_data->knobStyle) {
    case QwtKnob::Raised: {
        double off = 0.3 * knobRect.width();
        QRadialGradient gradient(knobRect.center(), knobRect.width(), knobRect.topLeft() + QPointF(off, off));

        gradient.setColorAt(0.0, palette().color(QPalette::Midlight));
        gradient.setColorAt(1.0, palette().color(QPalette::Button));

        brush = QBrush(gradient);

        break;
    }
    case QwtKnob::Styled: {
        QRadialGradient gradient(knobRect.center().x() - knobRect.width() / 3,
                                 knobRect.center().y() - knobRect.height() / 2,
                                 knobRect.width() * 1.3,
                                 knobRect.center().x(),
                                 knobRect.center().y() - knobRect.height() / 2);

        const QColor c = palette().color(QPalette::Button);
        gradient.setColorAt(0, c.lighter(110));
        gradient.setColorAt(0.5, c);
        gradient.setColorAt(0.501, c.darker(102));
        gradient.setColorAt(1, c.darker(115));

        brush = QBrush(gradient);

        break;
    }
    case QwtKnob::Sunken: {
        QLinearGradient gradient(knobRect.topLeft(), knobRect.bottomRight());
        gradient.setColorAt(0.0, palette().color(QPalette::Mid));
        gradient.setColorAt(0.5, palette().color(QPalette::Button));
        gradient.setColorAt(1.0, palette().color(QPalette::Midlight));
        brush = QBrush(gradient);

        break;
    }
    case QwtKnob::Flat:
    default:
        brush = palette().brush(QPalette::Button);
    }

    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawEllipse(aRect);
}

/*!
   \brief Draw the marker at the knob's front

   \param painter Painter
   \param rect Bounding rectangle of the knob without scale
   \param angle Angle of the marker in degrees
               ( clockwise, 0 at the 12 o'clock position )
 */
void QwtKnob::drawMarker(QPainter* painter, const QRectF& rect, double angle) const
{
    if (m_data->markerStyle == NoMarker || !isValid())
        return;

    const double radians = qwtRadians(angle);
    const double sinA    = -qFastSin(radians);
    const double cosA    = qFastCos(radians);

    const double xm     = rect.center().x();
    const double ym     = rect.center().y();
    const double margin = 4.0;

    double radius = 0.5 * (rect.width() - m_data->borderWidth) - margin;
    if (radius < 1.0)
        radius = 1.0;

    double markerSize = m_data->markerSize;
    if (markerSize <= 0)
        markerSize = qRound(0.4 * radius);

    switch (m_data->markerStyle) {
    case Notch:
    case Nub: {
        const double dotWidth = qwtMinF(markerSize, radius);

        const double dotCenterDist = radius - 0.5 * dotWidth;
        if (dotCenterDist > 0.0) {
            const QPointF center(xm - sinA * dotCenterDist, ym - cosA * dotCenterDist);

            QRectF ellipse(0.0, 0.0, dotWidth, dotWidth);
            ellipse.moveCenter(center);

            QColor c1 = palette().color(QPalette::Light);
            QColor c2 = palette().color(QPalette::Mid);

            if (m_data->markerStyle == Notch)
                qSwap(c1, c2);

            QLinearGradient gradient(ellipse.topLeft(), ellipse.bottomRight());
            gradient.setColorAt(0.0, c1);
            gradient.setColorAt(1.0, c2);

            painter->setPen(Qt::NoPen);
            painter->setBrush(gradient);

            painter->drawEllipse(ellipse);
        }
        break;
    }
    case Dot: {
        const double dotWidth = qwtMinF(markerSize, radius);

        const double dotCenterDist = radius - 0.5 * dotWidth;
        if (dotCenterDist > 0.0) {
            const QPointF center(xm - sinA * dotCenterDist, ym - cosA * dotCenterDist);

            QRectF ellipse(0.0, 0.0, dotWidth, dotWidth);
            ellipse.moveCenter(center);

            painter->setPen(Qt::NoPen);
            painter->setBrush(palette().color(QPalette::ButtonText));
            painter->drawEllipse(ellipse);
        }

        break;
    }
    case Tick: {
        const double rb = qwtMaxF(radius - markerSize, 1.0);
        const double re = radius;

        const QLineF line(xm - sinA * rb, ym - cosA * rb, xm - sinA * re, ym - cosA * re);

        QPen pen(palette().color(QPalette::ButtonText), 0);
        pen.setCapStyle(Qt::FlatCap);
        painter->setPen(pen);
        painter->drawLine(line);

        break;
    }
    case Triangle: {
        const double rb = qwtMaxF(radius - markerSize, 1.0);
        const double re = radius;

        painter->translate(rect.center());
        painter->rotate(angle - 90.0);

        QPolygonF polygon;
        polygon += QPointF(re, 0.0);
        polygon += QPointF(rb, 0.5 * (re - rb));
        polygon += QPointF(rb, -0.5 * (re - rb));

        painter->setPen(Qt::NoPen);
        painter->setBrush(palette().color(QPalette::ButtonText));
        painter->drawPolygon(polygon);

        painter->resetTransform();

        break;
    }
    default:
        break;
    }
}

/*!
   Draw the focus indicator
   \param painter Painter
 */
void QwtKnob::drawFocusIndicator(QPainter* painter) const
{
    const QRect cr = contentsRect();

    int w = m_data->knobWidth;
    if (w <= 0) {
        w = qMin(cr.width(), cr.height());
    } else {
        const int extent = qCeil(scaleDraw()->extent(font()));
        w += 2 * (extent + m_data->scaleDist);
    }

    QRect focusRect(0, 0, w, w);
    focusRect.moveCenter(cr.center());

    QwtPainter::drawFocusRect(painter, this, focusRect);
}

/**
 * \if ENGLISH
 * @brief Set the alignment of the knob
 * @details Similar to QLabel::alignment(), the flags decide how to align the knob inside of contentsRect().
 *          The default setting is Qt::AlignCenter.
 * @param[in] alignment Or'd alignment flags
 * \sa alignment(), setKnobWidth(), knobRect()
 * \endif
 * \if CHINESE
 * @brief 设置旋钮的对齐方式
 * @details 类似于 QLabel::alignment()，标志决定如何将旋钮在 contentsRect() 内对齐。
 *          默认设置为 Qt::AlignCenter。
 * @param[in] alignment 对齐标志的组合
 * \sa alignment(), setKnobWidth(), knobRect()
 * \endif
 */
void QwtKnob::setAlignment(Qt::Alignment alignment)
{
    if (m_data->alignment != alignment) {
        m_data->alignment = alignment;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return alignment of the knob inside contentsRect()
 * @return Alignment flags
 * \sa setAlignment(), knobWidth(), knobRect()
 * \endif
 * \if CHINESE
 * @brief 返回旋钮在 contentsRect() 内的对齐方式
 * @return 对齐标志
 * \sa setAlignment(), knobWidth(), knobRect()
 * \endif
 */
Qt::Alignment QwtKnob::alignment() const
{
    return m_data->alignment;
}

/**
 * \if ENGLISH
 * @brief Change the knob's width
 * @details Setting a fixed value for the diameter of the knob is helpful for aligning several knobs in a row.
 * @param[in] width New width
 * \sa knobWidth(), setAlignment()
 * \note Modifies the sizePolicy()
 * \endif
 * \if CHINESE
 * @brief 更改旋钮宽度
 * @details 为旋钮直径设置固定值有助于将多个旋钮在一行中对齐。
 * @param[in] width 新宽度
 * \sa knobWidth(), setAlignment()
 * \note 会修改 sizePolicy()
 * \endif
 */
void QwtKnob::setKnobWidth(int width)
{
    width = qMax(width, 0);

    if (width != m_data->knobWidth) {
        QSizePolicy::Policy policy;
        if (width > 0)
            policy = QSizePolicy::Minimum;
        else
            policy = QSizePolicy::MinimumExpanding;

        setSizePolicy(policy, policy);

        m_data->knobWidth = width;

        updateGeometry();
        update();
    }
}

//! Return the width of the knob
int QwtKnob::knobWidth() const
{
    return m_data->knobWidth;
}

/**
 * \if ENGLISH
 * @brief Set the knob's border width
 * @param[in] borderWidth New border width
 * \endif
 * \if CHINESE
 * @brief 设置旋钮边框宽度
 * @param[in] borderWidth 新边框宽度
 * \endif
 */
void QwtKnob::setBorderWidth(int borderWidth)
{
    m_data->borderWidth = qMax(borderWidth, 0);

    updateGeometry();
    update();
}

//! Return the border width
int QwtKnob::borderWidth() const
{
    return m_data->borderWidth;
}

/**
 * \if ENGLISH
 * @brief Set the size of the marker
 * @details When setting a size <= 0 the marker will automatically be scaled to 40% of the radius of the knob.
 * @param[in] size Marker size
 * \sa markerSize(), markerStyle()
 * \endif
 * \if CHINESE
 * @brief 设置标记大小
 * @details 当设置大小 <= 0 时，标记将自动缩放为旋钮半径的 40%。
 * @param[in] size 标记大小
 * \sa markerSize(), markerStyle()
 * \endif
 */
void QwtKnob::setMarkerSize(int size)
{
    if (m_data->markerSize != size) {
        m_data->markerSize = size;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return the marker size
 * @return Marker size
 * \sa setMarkerSize()
 * \endif
 * \if CHINESE
 * @brief 返回标记大小
 * @return 标记大小
 * \sa setMarkerSize()
 * \endif
 */
int QwtKnob::markerSize() const
{
    return m_data->markerSize;
}

/**
 * \if ENGLISH
 * @brief Return the size hint
 * @return Size hint
 * \endif
 * \if CHINESE
 * @brief 返回大小提示
 * @return 大小提示
 * \endif
 */
QSize QwtKnob::sizeHint() const
{
    const QSize hint = qwtKnobSizeHint(this, 50);
    return qwtExpandedToGlobalStrut(hint);
}

/**
 * \if ENGLISH
 * @brief Return the minimum size hint
 * @return Minimum size hint
 * \sa sizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回最小大小提示
 * @return 最小大小提示
 * \sa sizeHint()
 * \endif
 */
QSize QwtKnob::minimumSizeHint() const
{
    return qwtKnobSizeHint(this, 20);
}
