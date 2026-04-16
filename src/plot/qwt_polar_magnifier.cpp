/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_magnifier.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_point_polar.h"

#include <qevent.h>

class QwtPolarMagnifier::PrivateData
{
public:
    PrivateData() : unzoomKey(Qt::Key_Home), unzoomKeyModifiers(Qt::NoModifier)
    {
    }

    int unzoomKey;
    int unzoomKeyModifiers;
};

/**
 * \if ENGLISH
 * @brief Constructor
 * @param[in] canvas Plot canvas to be magnified
 * \endif
 *
 * \if CHINESE
 * @brief 构造函数
 * @param[in] canvas 要放大的绘图画布
 * \endif
 */
QwtPolarMagnifier::QwtPolarMagnifier(QwtPolarCanvas* canvas) : QwtMagnifier(canvas)
{
    m_data = new PrivateData();
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
QwtPolarMagnifier::~QwtPolarMagnifier()
{
    delete m_data;
}

/**
 * \if ENGLISH
 * @brief Assign key and modifiers that are used for unzooming
 * @details The default combination is Qt::Key_Home + Qt::NoModifier.
 * @param[in] key Key code
 * @param[in] modifiers Modifiers
 * @sa getUnzoomKey(), QwtPolarPlot::unzoom()
 * \endif
 *
 * \if CHINESE
 * @brief 设置用于取消缩放的按键和修饰键
 * @details 默认组合是 Qt::Key_Home + Qt::NoModifier。
 * @param[in] key 按键代码
 * @param[in] modifiers 修饰键
 * @sa getUnzoomKey(), QwtPolarPlot::unzoom()
 * \endif
 */
void QwtPolarMagnifier::setUnzoomKey(int key, int modifiers)
{
    m_data->unzoomKey          = key;
    m_data->unzoomKeyModifiers = modifiers;
}

/**
 * \if ENGLISH
 * @brief Get the key and modifiers that are used for unzooming
 * @param[out] key Key code
 * @param[out] modifiers Modifiers
 * @sa setUnzoomKey(), QwtPolarPlot::unzoom()
 * \endif
 *
 * \if CHINESE
 * @brief 获取用于取消缩放的按键和修饰键
 * @param[out] key 按键代码
 * @param[out] modifiers 修饰键
 * @sa setUnzoomKey(), QwtPolarPlot::unzoom()
 * \endif
 */
void QwtPolarMagnifier::getUnzoomKey(int& key, int& modifiers) const
{
    key       = m_data->unzoomKey;
    modifiers = m_data->unzoomKeyModifiers;
}

/**
 * \if ENGLISH
 * @brief Get the observed plot canvas
 * @return Observed plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图画布
 * @return 观察的绘图画布
 * \endif
 */
QwtPolarCanvas* QwtPolarMagnifier::canvas()
{
    return qobject_cast< QwtPolarCanvas* >(parent());
}

/**
 * \if ENGLISH
 * @brief Get the observed plot canvas (const version)
 * @return Observed plot canvas
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图画布（常量版本）
 * @return 观察的绘图画布
 * \endif
 */
const QwtPolarCanvas* QwtPolarMagnifier::canvas() const
{
    return qobject_cast< const QwtPolarCanvas* >(parent());
}

/**
 * \if ENGLISH
 * @brief Get the observed plot
 * @return Observed plot
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图
 * @return 观察的绘图
 * \endif
 */
QwtPolarPlot* QwtPolarMagnifier::plot()
{
    QwtPolarCanvas* c = canvas();
    if (c)
        return c->plot();

    return nullptr;
}

/**
 * \if ENGLISH
 * @brief Get the observed plot (const version)
 * @return Observed plot
 * \endif
 *
 * \if CHINESE
 * @brief 获取观察的绘图（常量版本）
 * @return 观察的绘图
 * \endif
 */
const QwtPolarPlot* QwtPolarMagnifier::plot() const
{
    const QwtPolarCanvas* c = canvas();
    if (c)
        return c->plot();

    return nullptr;
}

/*!
   Handle a key press event for the observed widget.

   \param event Key event
 */
void QwtPolarMagnifier::widgetKeyPressEvent(QKeyEvent* event)
{
    const int key   = event->key();
    const int state = event->modifiers();

    if (key == m_data->unzoomKey && state == m_data->unzoomKeyModifiers) {
        unzoom();
        return;
    }

    QwtMagnifier::widgetKeyPressEvent(event);
}

/**
 * \if ENGLISH
 * @brief Zoom in/out the zoomed area
 * @param[in] factor A value < 1.0 zooms in, a value > 1.0 zooms out
 * \endif
 *
 * \if CHINESE
 * @brief 放大/缩小缩放区域
 * @param[in] factor 值 < 1.0 时放大，值 > 1.0 时缩小
 * \endif
 */
void QwtPolarMagnifier::rescale(double factor)
{
    factor = qAbs(factor);
    if (factor == 1.0 || factor == 0.0)
        return;

    QwtPolarPlot* plt = plot();
    if (plt == nullptr)
        return;

    QwtPointPolar zoomPos;
    double newZoomFactor = plt->zoomFactor() * factor;

    if (newZoomFactor >= 1.0)
        newZoomFactor = 1.0;
    else
        zoomPos = plt->zoomPos();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->zoom(zoomPos, newZoomFactor);

    plt->setAutoReplot(autoReplot);
    plt->replot();
}

/**
 * \if ENGLISH
 * @brief Unzoom the plot widget
 * \endif
 *
 * \if CHINESE
 * @brief 取消绘图控件的缩放
 * \endif
 */
void QwtPolarMagnifier::unzoom()
{
    QwtPolarPlot* plt = plot();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->unzoom();

    plt->setAutoReplot(autoReplot);
    plt->replot();
}
