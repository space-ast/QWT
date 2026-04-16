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

#include "qwt_dial.h"
#include "qwt_dial_needle.h"
#include "qwt_math.h"
#include "qwt_scale_map.h"
#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include "qwt_utils.h"

#include <qpainter.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qstyle.h>
#include <qstyleoption.h>

static inline double qwtAngleDist(double a1, double a2)
{
    double dist = qAbs(a2 - a1);
    if (dist > 360.0)
        dist -= 360.0;

    return dist;
}

static inline bool qwtIsOnArc(double angle, double min, double max)
{
    if (min < max) {
        return (angle >= min) && (angle <= max);
    } else {
        return (angle >= min) || (angle <= max);
    }
}

static inline double qwtBoundedAngle(double min, double angle, double max)
{
    double from = qwtNormalizeDegrees(min);
    double to   = qwtNormalizeDegrees(max);

    double a;

    if (qwtIsOnArc(angle, from, to)) {
        a = angle;
        if (a < min)
            a += 360.0;
    } else {
        if (qwtAngleDist(angle, from) < qwtAngleDist(angle, to)) {
            a = min;
        } else {
            a = max;
        }
    }

    return a;
}

class QwtDial::PrivateData
{
public:
    PrivateData()
        : frameShadow(Sunken)
        , lineWidth(0)
        , mode(RotateNeedle)
        , origin(90.0)
        , minScaleArc(0.0)
        , maxScaleArc(0.0)
        , needle(nullptr)
        , arcOffset(0.0)
        , mouseOffset(0.0)
    {
    }

    ~PrivateData()
    {
        delete needle;
    }
    Shadow frameShadow;
    int lineWidth;

    QwtDial::Mode mode;

    double origin;
    double minScaleArc;
    double maxScaleArc;

    QwtDialNeedle* needle;

    double arcOffset;
    double mouseOffset;

    QPixmap pixmapCache;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @details Create a dial widget with no needle. The scale is initialized
 *          to [ 0.0, 360.0 ] and 360 steps ( QwtAbstractSlider::setTotalSteps() ).
 *          The origin of the scale is at 90°,
 *          The value is set to 0.0.
 *          The default mode is QwtDial::RotateNeedle.
 * @param parent Parent widget
 * \endif
 * \if CHINESE
 * @brief 构造函数
 * @details 创建一个无指针的表盘控件。刻度初始化为 [ 0.0, 360.0 ] 和 360 步
 *          ( QwtAbstractSlider::setTotalSteps() )。
 *          刻度的原点在 90°，值设置为 0.0。
 *          默认模式是 QwtDial::RotateNeedle。
 * @param parent 父控件
 * \endif
 */
QwtDial::QwtDial(QWidget* parent) : QwtAbstractSlider(parent)
{
    m_data = new PrivateData;

    setFocusPolicy(Qt::TabFocus);

    QPalette p = palette();
    for (int i = 0; i < QPalette::NColorGroups; i++) {
        const QPalette::ColorGroup colorGroup = static_cast< QPalette::ColorGroup >(i);

        // Base: background color of the circle inside the frame.
        // WindowText: background color of the circle inside the scale

        p.setColor(colorGroup, QPalette::WindowText, p.color(colorGroup, QPalette::Base));
    }
    setPalette(p);

    QwtRoundScaleDraw* scaleDraw = new QwtRoundScaleDraw();
    scaleDraw->setRadius(0);

    setScaleDraw(scaleDraw);

    setScaleArc(0.0, 360.0);  // scale as a full circle

    setScaleMaxMajor(10);
    setScaleMaxMinor(5);

    setValue(0.0);
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
QwtDial::~QwtDial()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Sets the frame shadow value from the frame style
 * @param shadow Frame shadow
 * \sa setLineWidth(), QFrame::setFrameShadow()
 * \endif
 * \if CHINESE
 * @brief 从框架样式设置框架阴影值
 * @param shadow 框架阴影
 * \sa setLineWidth(), QFrame::setFrameShadow()
 * \endif
 */
void QwtDial::setFrameShadow(Shadow shadow)
{
    if (shadow != m_data->frameShadow) {
        invalidateCache();

        m_data->frameShadow = shadow;
        if (lineWidth() > 0)
            update();
    }
}

/**
 * \if ENGLISH
 * @brief Return frame shadow
 * \sa setFrameShadow(), lineWidth(), QFrame::frameShadow()
 * \endif
 * \if CHINESE
 * @brief 返回框架阴影
 * \sa setFrameShadow(), lineWidth(), QFrame::frameShadow()
 * \endif
 */
QwtDial::Shadow QwtDial::frameShadow() const
{
    return m_data->frameShadow;
}

/**
 * \if ENGLISH
 * @brief Sets the line width of the frame
 * @param lineWidth Line width
 * \sa setFrameShadow()
 * \endif
 * \if CHINESE
 * @brief 设置框架的线宽
 * @param lineWidth 线宽
 * \sa setFrameShadow()
 * \endif
 */
void QwtDial::setLineWidth(int lineWidth)
{
    if (lineWidth < 0)
        lineWidth = 0;

    if (m_data->lineWidth != lineWidth) {
        invalidateCache();

        m_data->lineWidth = lineWidth;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return line width of the frame
 * \sa setLineWidth(), frameShadow(), lineWidth()
 * \endif
 * \if CHINESE
 * @brief 返回框架的线宽
 * \sa setLineWidth(), frameShadow(), lineWidth()
 * \endif
 */
int QwtDial::lineWidth() const
{
    return m_data->lineWidth;
}

/**
 * \if ENGLISH
 * @brief Return bounding rectangle of the circle inside the frame
 * \sa setLineWidth(), scaleInnerRect(), boundingRect()
 * \endif
 * \if CHINESE
 * @brief 返回框架内圆的边界矩形
 * \sa setLineWidth(), scaleInnerRect(), boundingRect()
 * \endif
 */
QRect QwtDial::innerRect() const
{
    const int lw = lineWidth();
    return boundingRect().adjusted(lw, lw, -lw, -lw);
}

/**
 * \if ENGLISH
 * @brief Return bounding rectangle of the dial including the frame
 * \sa setLineWidth(), scaleInnerRect(), innerRect()
 * \endif
 * \if CHINESE
 * @brief 返回表盘的边界矩形（含框架）
 * \sa setLineWidth(), scaleInnerRect(), innerRect()
 * \endif
 */
QRect QwtDial::boundingRect() const
{
    const QRect cr = contentsRect();

    const int dim = qMin(cr.width(), cr.height());

    QRect inner(0, 0, dim, dim);
    inner.moveCenter(cr.center());

    return inner;
}

/**
 * \if ENGLISH
 * @brief Return rectangle inside the scale
 * \sa setLineWidth(), boundingRect(), innerRect()
 * \endif
 * \if CHINESE
 * @brief 返回刻度内的矩形
 * \sa setLineWidth(), boundingRect(), innerRect()
 * \endif
 */
QRect QwtDial::scaleInnerRect() const
{
    QRect rect = innerRect();

    const QwtAbstractScaleDraw* sd = scaleDraw();
    if (sd) {
        int scaleDist = qwtCeil(sd->extent(font()));
        scaleDist++;  // margin

        rect.adjust(scaleDist, scaleDist, -scaleDist, -scaleDist);
    }

    return rect;
}

/**
 * \if ENGLISH
 * @brief Change the mode of the dial
 * @details In case of QwtDial::RotateNeedle the needle is rotating, in case of
 *          QwtDial::RotateScale, the needle points to origin()
 *          and the scale is rotating.
 *          The default mode is QwtDial::RotateNeedle.
 * @param mode New mode
 * \sa mode(), setValue(), setOrigin()
 * \endif
 * \if CHINESE
 * @brief 更改表盘的模式
 * @details 如果是 QwtDial::RotateNeedle，指针是旋转的；如果是 QwtDial::RotateScale，
 *          指针指向 origin()，刻度是旋转的。
 *          默认模式是 QwtDial::RotateNeedle。
 * @param mode 新模式
 * \sa mode(), setValue(), setOrigin()
 * \endif
 */
void QwtDial::setMode(Mode mode)
{
    if (mode != m_data->mode) {
        invalidateCache();

        m_data->mode = mode;
        sliderChange();
    }
}

/**
 * \if ENGLISH
 * @brief Return mode of the dial
 * \sa setMode(), origin(), setScaleArc(), value()
 * \endif
 * \if CHINESE
 * @brief 返回表盘的模式
 * \sa setMode(), origin(), setScaleArc(), value()
 * \endif
 */
QwtDial::Mode QwtDial::mode() const
{
    return m_data->mode;
}

/**
 * \if ENGLISH
 * @brief Invalidate the internal caches used to speed up repainting
 * \endif
 * \if CHINESE
 * @brief 使用于加速重绘的内部缓存失效
 * \endif
 */
void QwtDial::invalidateCache()
{
    m_data->pixmapCache = QPixmap();
}

/*!
   Paint the dial
   \param event Paint event
 */
void QwtDial::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (m_data->mode == QwtDial::RotateScale) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);

        drawContents(&painter);

        painter.restore();
    }

    const QRect r = contentsRect();
    if (r.size() != m_data->pixmapCache.size()) {
        m_data->pixmapCache = QwtPainter::backingStore(this, r.size());
        m_data->pixmapCache.fill(Qt::transparent);

        QPainter p(&m_data->pixmapCache);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.translate(-r.topLeft());

        if (m_data->mode != QwtDial::RotateScale)
            drawContents(&p);

        if (lineWidth() > 0)
            drawFrame(&p);

        if (m_data->mode != QwtDial::RotateNeedle)
            drawNeedle(&p);
    }

    painter.drawPixmap(r.topLeft(), m_data->pixmapCache);

    if (m_data->mode == QwtDial::RotateNeedle)
        drawNeedle(&painter);

    if (hasFocus())
        drawFocusIndicator(&painter);
}

/*!
   Draw the focus indicator
   \param painter Painter
 */
void QwtDial::drawFocusIndicator(QPainter* painter) const
{
    QwtPainter::drawFocusRect(painter, this, boundingRect());
}

/*!
   Draw the frame around the dial

   \param painter Painter
   \sa lineWidth(), frameShadow()
 */
void QwtDial::drawFrame(QPainter* painter)
{
    QwtPainter::drawRoundFrame(painter, boundingRect(), palette(), lineWidth(), m_data->frameShadow);
}

/*!
   \brief Draw the contents inside the frame

   QPalette::Window is the background color outside of the frame.
   QPalette::Base is the background color inside the frame.
   QPalette::WindowText is the background color inside the scale.

   \param painter Painter

   \sa boundingRect(), innerRect(),
    scaleInnerRect(), QWidget::setPalette()
 */
void QwtDial::drawContents(QPainter* painter) const
{
    if (testAttribute(Qt::WA_NoSystemBackground) || palette().brush(QPalette::Base) != palette().brush(QPalette::Window)) {
        const QRectF br = boundingRect();

        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(palette().brush(QPalette::Base));
        painter->drawEllipse(br);
        painter->restore();
    }

    const QRectF insideScaleRect = scaleInnerRect();
    if (palette().brush(QPalette::WindowText) != palette().brush(QPalette::Base)) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(palette().brush(QPalette::WindowText));
        painter->drawEllipse(insideScaleRect);
        painter->restore();
    }

    const QPointF center = insideScaleRect.center();
    const double radius  = 0.5 * insideScaleRect.width();

    painter->save();
    drawScale(painter, center, radius);
    painter->restore();

    painter->save();
    drawScaleContents(painter, center, radius);
    painter->restore();
}

/*!
   Draw the needle

   \param painter Painter
   \param center Center of the dial
   \param radius Length for the needle
   \param direction Direction of the needle in degrees, counter clockwise
   \param colorGroup ColorGroup
 */
void QwtDial::drawNeedle(QPainter* painter, const QPointF& center, double radius, double direction, QPalette::ColorGroup colorGroup) const
{
    if (m_data->needle) {
        direction = 360.0 - direction;  // counter clockwise
        m_data->needle->draw(painter, center, radius, direction, colorGroup);
    }
}

void QwtDial::drawNeedle(QPainter* painter) const
{
    if (!isValid())
        return;

    QPalette::ColorGroup colorGroup;
    if (isEnabled())
        colorGroup = hasFocus() ? QPalette::Active : QPalette::Inactive;
    else
        colorGroup = QPalette::Disabled;

    const QRectF sr = scaleInnerRect();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    drawNeedle(painter, sr.center(), 0.5 * sr.width(), scaleMap().transform(value()) + 270.0, colorGroup);
    painter->restore();
}

/*!
   Draw the scale

   \param painter Painter
   \param center Center of the dial
   \param radius Radius of the scale
 */
void QwtDial::drawScale(QPainter* painter, const QPointF& center, double radius) const
{
    QwtRoundScaleDraw* sd = const_cast< QwtRoundScaleDraw* >(scaleDraw());
    if (sd == nullptr)
        return;

    sd->setRadius(radius);
    sd->moveCenter(center);

    QPalette pal = palette();

    const QColor textColor = pal.color(QPalette::Text);
    pal.setColor(QPalette::WindowText, textColor);  // ticks, backbone

    painter->setFont(font());
    painter->setPen(QPen(textColor, sd->penWidthF()));

    painter->setBrush(Qt::red);
    sd->draw(painter, pal);
}

/*!
   Draw the contents inside the scale

   Paints nothing.

   \param painter Painter
   \param center Center of the contents circle
   \param radius Radius of the contents circle
 */
void QwtDial::drawScaleContents(QPainter* painter, const QPointF& center, double radius) const
{
    Q_UNUSED(painter);
    Q_UNUSED(center);
    Q_UNUSED(radius);
}

/**
 * \if ENGLISH
 * @brief Set a needle for the dial
 * @param needle Needle
 * @warning The needle will be deleted, when a different needle is set or in ~QwtDial()
 * \endif
 * \if CHINESE
 * @brief 为表盘设置指针
 * @param needle 指针
 * @warning 当设置不同的指针或在 ~QwtDial() 时，指针将被删除
 * \endif
 */
void QwtDial::setNeedle(QwtDialNeedle* needle)
{
    if (needle != m_data->needle) {
        if (m_data->needle)
            delete m_data->needle;

        m_data->needle = needle;
        update();
    }
}

/**
 * \if ENGLISH
 * @brief Return needle
 * \sa setNeedle()
 * \endif
 * \if CHINESE
 * @brief 返回指针
 * \sa setNeedle()
 * \endif
 */
const QwtDialNeedle* QwtDial::needle() const
{
    return m_data->needle;
}

/**
 * \if ENGLISH
 * @brief Return needle
 * \sa setNeedle()
 * \endif
 * \if CHINESE
 * @brief 返回指针
 * \sa setNeedle()
 * \endif
 */
QwtDialNeedle* QwtDial::needle()
{
    return m_data->needle;
}

/**
 * \if ENGLISH
 * @brief Return the scale draw
 * \endif
 * \if CHINESE
 * @brief 返回刻度绘制器
 * \endif
 */
QwtRoundScaleDraw* QwtDial::scaleDraw()
{
    return static_cast< QwtRoundScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Return the scale draw
 * \endif
 * \if CHINESE
 * @brief 返回刻度绘制器
 * \endif
 */
const QwtRoundScaleDraw* QwtDial::scaleDraw() const
{
    return static_cast< const QwtRoundScaleDraw* >(abstractScaleDraw());
}

/**
 * \if ENGLISH
 * @brief Set an individual scale draw
 * @details The motivation for setting a scale draw is often to overload QwtRoundScaleDraw::label()
 *          to return individual tick labels.
 * @param scaleDraw Scale draw
 * @warning The previous scale draw is deleted
 * \endif
 * \if CHINESE
 * @brief 设置单独的刻度绘制器
 * @details 设置刻度绘制器的原因通常是为了重载 QwtRoundScaleDraw::label()
 *          以返回单独的刻度标签。
 * @param scaleDraw 刻度绘制器
 * @warning 之前的刻度绘制器将被删除
 * \endif
 */
void QwtDial::setScaleDraw(QwtRoundScaleDraw* scaleDraw)
{
    setAbstractScaleDraw(scaleDraw);
    sliderChange();
}

/**
 * \if ENGLISH
 * @brief Change the arc of the scale
 * @param[in] minArc Lower limit
 * @param[in] maxArc Upper limit
 * \sa minScaleArc(), maxScaleArc()
 * \endif
 * \if CHINESE
 * @brief 更改刻度的弧度范围
 * @param[in] minArc 下限
 * @param[in] maxArc 上限
 * \sa minScaleArc(), maxScaleArc()
 * \endif
 */
void QwtDial::setScaleArc(double minArc, double maxArc)
{
    if (minArc != 360.0 && minArc != -360.0)
        minArc = std::fmod(minArc, 360.0);
    if (maxArc != 360.0 && maxArc != -360.0)
        maxArc = std::fmod(maxArc, 360.0);

    double minScaleArc = qwtMinF(minArc, maxArc);
    double maxScaleArc = qwtMaxF(minArc, maxArc);

    if (maxScaleArc - minScaleArc > 360.0)
        maxScaleArc = minScaleArc + 360.0;

    if ((minScaleArc != m_data->minScaleArc) || (maxScaleArc != m_data->maxScaleArc)) {
        m_data->minScaleArc = minScaleArc;
        m_data->maxScaleArc = maxScaleArc;

        invalidateCache();
        sliderChange();
    }
}

/**
 * \if ENGLISH
 * @brief Set the lower limit for the scale arc
 * @param min Lower limit of the scale arc
 * \sa setScaleArc(), setMaxScaleArc()
 * \endif
 * \if CHINESE
 * @brief 设置刻度弧的下限
 * @param min 刻度弧下限
 * \sa setScaleArc(), setMaxScaleArc()
 * \endif
 */
void QwtDial::setMinScaleArc(double min)
{
    setScaleArc(min, m_data->maxScaleArc);
}

/**
 * \if ENGLISH
 * @brief Return lower limit of the scale arc
 * \sa setScaleArc()
 * \endif
 * \if CHINESE
 * @brief 返回刻度弧的下限
 * \sa setScaleArc()
 * \endif
 */
double QwtDial::minScaleArc() const
{
    return m_data->minScaleArc;
}

/**
 * \if ENGLISH
 * @brief Set the upper limit for the scale arc
 * @param max Upper limit of the scale arc
 * \sa setScaleArc(), setMinScaleArc()
 * \endif
 * \if CHINESE
 * @brief 设置刻度弧的上限
 * @param max 刻度弧上限
 * \sa setScaleArc(), setMinScaleArc()
 * \endif
 */
void QwtDial::setMaxScaleArc(double max)
{
    setScaleArc(m_data->minScaleArc, max);
}

/**
 * \if ENGLISH
 * @brief Return upper limit of the scale arc
 * \sa setScaleArc()
 * \endif
 * \if CHINESE
 * @brief 返回刻度弧的上限
 * \sa setScaleArc()
 * \endif
 */
double QwtDial::maxScaleArc() const
{
    return m_data->maxScaleArc;
}

/**
 * \if ENGLISH
 * @brief Change the origin
 * @details The origin is the angle where scale and needle is relative to.
 * @param origin New origin
 * \sa origin()
 * \endif
 * \if CHINESE
 * @brief 更改原点
 * @details 原点是刻度和指针相对的角度。
 * @param origin 新原点
 * \sa origin()
 * \endif
 */
void QwtDial::setOrigin(double origin)
{
    invalidateCache();

    m_data->origin = origin;
    sliderChange();
}

/**
 * \if ENGLISH
 * @brief The origin is the angle where scale and needle is relative to
 * @return Origin of the dial
 * \sa setOrigin()
 * \endif
 * \if CHINESE
 * @brief 原点是刻度和指针相对的角度
 * @return 表盘原点
 * \sa setOrigin()
 * \endif
 */
double QwtDial::origin() const
{
    return m_data->origin;
}

/**
 * \if ENGLISH
 * @brief Return size hint
 * \sa minimumSizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回尺寸提示
 * \sa minimumSizeHint()
 * \endif
 */
QSize QwtDial::sizeHint() const
{
    int sh = 0;
    if (scaleDraw())
        sh = qwtCeil(scaleDraw()->extent(font()));

    const int d = 6 * sh + 2 * lineWidth();

    QSize hint(d, d);
    if (!isReadOnly())
        hint = qwtExpandedToGlobalStrut(hint);

    return hint;
}

/**
 * \if ENGLISH
 * @brief Return minimum size hint
 * \sa sizeHint()
 * \endif
 * \if CHINESE
 * @brief 返回最小尺寸提示
 * \sa sizeHint()
 * \endif
 */
QSize QwtDial::minimumSizeHint() const
{
    int sh = 0;
    if (scaleDraw())
        sh = qwtCeil(scaleDraw()->extent(font()));

    const int d = 3 * sh + 2 * lineWidth();

    return QSize(d, d);
}

/*!
   \brief Determine what to do when the user presses a mouse button.

   \param pos Mouse position

   \retval True, when the inner circle contains pos
   \sa scrolledTo()
 */
bool QwtDial::isScrollPosition(const QPoint& pos) const
{
    const QRegion region(innerRect(), QRegion::Ellipse);
    if (region.contains(pos) && (pos != innerRect().center())) {
        double angle = QLineF(rect().center(), pos).angle();
        if (m_data->mode == QwtDial::RotateScale)
            angle = 360.0 - angle;

        double valueAngle = qwtNormalizeDegrees(90.0 - scaleMap().transform(value()));

        m_data->mouseOffset = qwtNormalizeDegrees(angle - valueAngle);
        m_data->arcOffset   = scaleMap().p1();

        return true;
    }

    return false;
}

/*!
   \brief Determine the value for a new position of the
         slider handle.

   \param pos Mouse position

   \return Value for the mouse position
   \sa isScrollPosition()
 */
double QwtDial::scrolledTo(const QPoint& pos) const
{
    double angle = QLineF(rect().center(), pos).angle();
    if (m_data->mode == QwtDial::RotateScale) {
        angle += scaleMap().p1() - m_data->arcOffset;
        angle = 360.0 - angle;
    }

    angle = qwtNormalizeDegrees(angle - m_data->mouseOffset);
    angle = qwtNormalizeDegrees(90.0 - angle);

    if (scaleMap().pDist() >= 360.0) {
        if (angle < scaleMap().p1())
            angle += 360.0;

        if (!wrapping()) {
            double boundedAngle = angle;

            const double arc = angle - scaleMap().transform(value());
            if (qAbs(arc) > 180.0) {
                boundedAngle = (arc > 0) ? scaleMap().p1() : scaleMap().p2();
            }

            m_data->mouseOffset += (boundedAngle - angle);

            angle = boundedAngle;
        }
    } else {
        const double boundedAngle = qwtBoundedAngle(scaleMap().p1(), angle, scaleMap().p2());

        if (!wrapping())
            m_data->mouseOffset += (boundedAngle - angle);

        angle = boundedAngle;
    }

    return scaleMap().invTransform(angle);
}

/*!
   Change Event handler
   \param event Change event

   Invalidates internal paint caches if necessary
 */
void QwtDial::changeEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::EnabledChange:
    case QEvent::FontChange:
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::LanguageChange:
    case QEvent::LocaleChange: {
        invalidateCache();
        break;
    }
    default:
        break;
    }

    QwtAbstractSlider::changeEvent(event);
}

/*!
   Wheel Event handler
   \param event Wheel event
 */
void QwtDial::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION < 0x050e00
    const QPoint wheelPos = event->pos();
#else
    const QPoint wheelPos = event->position().toPoint();
#endif

    const QRegion region(innerRect(), QRegion::Ellipse);
    if (region.contains(wheelPos))
        QwtAbstractSlider::wheelEvent(event);
}

void QwtDial::setAngleRange(double angle, double span)
{
    if (QwtRoundScaleDraw* sd = scaleDraw()) {
        angle = qwtNormalizeDegrees(angle - 270.0);
        sd->setAngleRange(angle, angle + span);
    }
}

/*!
   Invalidate the internal caches and call
   QwtAbstractSlider::scaleChange()
 */
void QwtDial::scaleChange()
{
    invalidateCache();
    QwtAbstractSlider::scaleChange();
}

void QwtDial::sliderChange()
{
    setAngleRange(m_data->origin + m_data->minScaleArc, m_data->maxScaleArc - m_data->minScaleArc);

    if (mode() == RotateScale) {
        const double arc = scaleMap().transform(value()) - scaleMap().p1();
        setAngleRange(m_data->origin - arc, m_data->maxScaleArc - m_data->minScaleArc);
    }

    QwtAbstractSlider::sliderChange();
}
