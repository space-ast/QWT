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

#include "qwt_compass.h"
#include "qwt_compass_rose.h"
#include "qwt_text.h"

#include <qevent.h>
#include <qmap.h>

class QwtCompassScaleDraw::PrivateData
{
public:
    QMap< double, QString > labelMap;
};

/**
 * \if ENGLISH
 *   \brief Constructor
 *   \details Initializes a label map for multiples of 45 degrees.
 *            The default map consists of the labels N, NE, E, SE, S, SW, W, NW.
 * \endif
 * \if CHINESE
 *   \brief 构造函数
 *   \details 初始化一个45度倍数的标签映射。
 *            默认映射包含标签N、NE、E、SE、S、SW、W、NW。
 * \endif
 */
QwtCompassScaleDraw::QwtCompassScaleDraw()
{
    m_data = new PrivateData;

    enableComponent(QwtAbstractScaleDraw::Backbone, false);
    enableComponent(QwtAbstractScaleDraw::Ticks, false);

    QMap< double, QString >& map = m_data->labelMap;

    map.insert(0.0, QString::fromLatin1("N"));
    map.insert(45.0, QString::fromLatin1("NE"));
    map.insert(90.0, QString::fromLatin1("E"));
    map.insert(135.0, QString::fromLatin1("SE"));
    map.insert(180.0, QString::fromLatin1("S"));
    map.insert(225.0, QString::fromLatin1("SW"));
    map.insert(270.0, QString::fromLatin1("W"));
    map.insert(315.0, QString::fromLatin1("NW"));

#if 0
    map.insert( 22.5, QString::fromLatin1( "NNE" ) );
    map.insert( 67.5, QString::fromLatin1( "NEE" ) );
    map.insert( 112.5, QString::fromLatin1( "SEE" ) );
    map.insert( 157.5, QString::fromLatin1( "SSE" ) );
    map.insert( 202.5, QString::fromLatin1( "SSW" ) );
    map.insert( 247.5, QString::fromLatin1( "SWW" ) );
    map.insert( 292.5, QString::fromLatin1( "NWW" ) );
    map.insert( 337.5, QString::fromLatin1( "NNW" ) );
#endif
}

/**
 * \if ENGLISH
 *   \brief Constructor
 *   \param[in] map Value to label map
 * \endif
 * \if CHINESE
 *   \brief 构造函数
 *   \param[in] map 数值到标签的映射
 * \endif
 */
QwtCompassScaleDraw::QwtCompassScaleDraw(const QMap< double, QString >& map)
{
    m_data           = new PrivateData;
    m_data->labelMap = map;

    enableComponent(QwtAbstractScaleDraw::Backbone, false);
    enableComponent(QwtAbstractScaleDraw::Ticks, false);
}

/**
 * \if ENGLISH
 *   \brief Destructor
 * \endif
 * \if CHINESE
 *   \brief 析构函数
 * \endif
 */
QwtCompassScaleDraw::~QwtCompassScaleDraw()
{
    delete m_data;
}

/**
 * \if ENGLISH
 *   \brief Set a map, mapping values to labels
 *   \param[in] map Value to label map
 *   \details The values of the major ticks are found by looking into this map.
 *            The default map consists of the labels N, NE, E, SE, S, SW, W, NW.
 *   \warning The map will have no effect for values that are no major tick values.
 *            Major ticks can be changed by QwtScaleDraw::setScale.
 *   \sa labelMap(), scaleDraw(), setScale()
 * \endif
 * \if CHINESE
 *   \brief 设置数值到标签的映射
 *   \param[in] map 数值到标签的映射
 *   \details 通过查找此映射来确定主要刻度的标签值。
 *            默认映射包含标签N、NE、E、SE、S、SW、W、NW。
 *   \warning 对于非主要刻度值，映射将不起作用。
 *            主要刻度可通过QwtScaleDraw::setScale进行更改。
 *   \sa labelMap(), scaleDraw(), setScale()
 * \endif
 */
void QwtCompassScaleDraw::setLabelMap(const QMap< double, QString >& map)
{
    m_data->labelMap = map;
}

/**
 * \if ENGLISH
 *   \brief Get the map, mapping values to labels
 *   \return Map, mapping values to labels
 *   \sa setLabelMap()
 * \endif
 * \if CHINESE
 *   \brief 获取数值到标签的映射
 *   \return 数值到标签的映射
 *   \sa setLabelMap()
 * \endif
 */
QMap< double, QString > QwtCompassScaleDraw::labelMap() const
{
    return m_data->labelMap;
}

/**
 * \if ENGLISH
 *   \brief Map a value to a corresponding label
 *   \param[in] value Value that will be mapped
 *   \details label() looks in the labelMap() for a corresponding label for value
 *            or returns a null text.
 *   \return Label corresponding to the value, or null text if not found
 *   \sa labelMap(), setLabelMap()
 * \endif
 * \if CHINESE
 *   \brief 将数值映射为对应的标签
 *   \param[in] value 要映射的数值
 *   \details label()在labelMap()中查找对应数值的标签，
 *            如果未找到则返回空文本。
 *   \return 对应数值的标签，若未找到则返回空文本
 *   \sa labelMap(), setLabelMap()
 * \endif
 */
QwtText QwtCompassScaleDraw::label(double value) const
{
    if (qFuzzyCompare(value + 1.0, 1.0))
        value = 0.0;

    if (value < 0.0)
        value += 360.0;

    QMap< double, QString >::const_iterator it = m_data->labelMap.constFind(value);

    if (it != m_data->labelMap.constEnd())
        return *it;

    return QwtText();
}

class QwtCompass::PrivateData
{
public:
    PrivateData() : rose(nullptr)
    {
    }

    ~PrivateData()
    {
        delete rose;
    }

    QwtCompassRose* rose;
};

/**
 * \if ENGLISH
 *   \brief Constructor
 *   \param[in] parent Parent widget
 *   \details Creates a compass widget with a scale, no needle and no rose.
 *            The default origin is 270.0 with no valid value. It accepts
 *            mouse and keyboard inputs and has no step size. The default mode
 *            is QwtDial::RotateNeedle.
 * \endif
 * \if CHINESE
 *   \brief 构造函数
 *   \param[in] parent 父控件
 *   \details 创建一个带有刻度盘、无指针、无罗盘花的指南针控件。
 *            默认原点为270.0，无有效值。接受鼠标和键盘输入，无步长。
 *            默认模式为QwtDial::RotateNeedle。
 * \endif
 */
QwtCompass::QwtCompass(QWidget* parent) : QwtDial(parent)
{
    m_data = new PrivateData;

    setScaleDraw(new QwtCompassScaleDraw());

    setOrigin(270.0);
    setWrapping(true);

    setScaleMaxMajor(36);
    setScaleMaxMinor(10);

    setScale(0.0, 360.0);  // degrees as default
    setTotalSteps(360);
}

/**
 * \if ENGLISH
 *   \brief Destructor
 * \endif
 * \if CHINESE
 *   \brief 析构函数
 * \endif
 */
QwtCompass::~QwtCompass()
{
    delete m_data;
}

/*!
   Draw the contents of the scale

   \param painter Painter
   \param center Center of the content circle
   \param radius Radius of the content circle
 */
void QwtCompass::drawScaleContents(QPainter* painter, const QPointF& center, double radius) const
{
    QPalette::ColorGroup cg;
    if (isEnabled())
        cg = hasFocus() ? QPalette::Active : QPalette::Inactive;
    else
        cg = QPalette::Disabled;

    double north = origin();
    if (isValid()) {
        if (mode() == RotateScale)
            north -= value();
    }

    const int margin = 4;
    drawRose(painter, center, radius - margin, 360.0 - north, cg);
}

/*!
   Draw the compass rose

   \param painter Painter
   \param center Center of the compass
   \param radius of the circle, where to paint the rose
   \param north Direction pointing north, in degrees counter clockwise
   \param cg Color group
 */
void QwtCompass::drawRose(QPainter* painter, const QPointF& center, double radius, double north, QPalette::ColorGroup cg) const
{
    if (m_data->rose)
        m_data->rose->draw(painter, center, radius, north, cg);
}

/**
 * \if ENGLISH
 *   \brief Set a rose for the compass
 *   \param[in] rose Compass rose
 *   \warning The rose will be deleted, when a different rose is set or in ~QwtCompass.
 *   \sa rose()
 * \endif
 * \if CHINESE
 *   \brief 为指南针设置罗盘花
 *   \param[in] rose 罗盘花对象
 *   \warning 当设置不同的罗盘花或在~QwtCompass析构时，原罗盘花将被删除。
 *   \sa rose()
 * \endif
 */
void QwtCompass::setRose(QwtCompassRose* rose)
{
    if (rose != m_data->rose) {
        if (m_data->rose)
            delete m_data->rose;

        m_data->rose = rose;
        update();
    }
}

/**
 * \if ENGLISH
 *   \brief Get the compass rose
 *   \return The compass rose (const version)
 *   \sa setRose()
 * \endif
 * \if CHINESE
 *   \brief 获取罗盘花
 *   \return 罗盘花对象（const版本）
 *   \sa setRose()
 * \endif
 */
const QwtCompassRose* QwtCompass::rose() const
{
    return m_data->rose;
}

/**
 * \if ENGLISH
 *   \brief Get the compass rose
 *   \return The compass rose
 *   \sa setRose()
 * \endif
 * \if CHINESE
 *   \brief 获取罗盘花
 *   \return 罗盘花对象
 *   \sa setRose()
 * \endif
 */
QwtCompassRose* QwtCompass::rose()
{
    return m_data->rose;
}

/*!
   Handles key events

   Beside the keys described in QwtDial::keyPressEvent numbers
   from 1-9 (without 5) set the direction according to their
   position on the num pad.

   \sa isReadOnly()
 */
void QwtCompass::keyPressEvent(QKeyEvent* kev)
{
    if (isReadOnly())
        return;

#if 0
    if ( kev->key() == Key_5 )
    {
        invalidate(); // signal ???
        return;
    }
#endif

    double newValue = value();

    if (kev->key() >= Qt::Key_1 && kev->key() <= Qt::Key_9) {
        if (mode() != RotateNeedle || kev->key() == Qt::Key_5)
            return;

        switch (kev->key()) {
        case Qt::Key_6:
            newValue = 180.0 * 0.0;
            break;
        case Qt::Key_3:
            newValue = 180.0 * 0.25;
            break;
        case Qt::Key_2:
            newValue = 180.0 * 0.5;
            break;
        case Qt::Key_1:
            newValue = 180.0 * 0.75;
            break;
        case Qt::Key_4:
            newValue = 180.0 * 1.0;
            break;
        case Qt::Key_7:
            newValue = 180.0 * 1.25;
            break;
        case Qt::Key_8:
            newValue = 180.0 * 1.5;
            break;
        case Qt::Key_9:
            newValue = 180.0 * 1.75;
            break;
        }
        newValue -= origin();
        setValue(newValue);
    } else {
        QwtDial::keyPressEvent(kev);
    }
}
