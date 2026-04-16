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

#ifndef QWT_SAMPLING_THREAD_H
#define QWT_SAMPLING_THREAD_H

#include "qwt_global.h"
#include <qthread.h>

/**
 * \if ENGLISH
 * @brief A thread collecting samples at regular intervals.
 *
 * Continuous signals are converted into a discrete signal by
 * collecting samples at regular intervals. A discrete signal
 * can be displayed by a QwtPlotSeriesItem on a QwtPlot widget.
 *
 * QwtSamplingThread starts a thread calling periodically sample(),
 * to collect and store ( or emit ) a single sample.
 *
 * @sa QwtPlotCurve, QwtPlotSeriesItem
 * \endif
 *
 * \if CHINESE
 * @brief 定期收集样本的线程
 *
 * 通过定期收集样本，将连续信号转换为离散信号。离散信号可以通过 QwtPlotSeriesItem 在 QwtPlot 部件上显示。
 *
 * QwtSamplingThread 启动一个线程，定期调用 sample() 方法来收集和存储（或发出）单个样本。
 *
 * @sa QwtPlotCurve, QwtPlotSeriesItem
 * \endif
 */
class QWT_EXPORT QwtSamplingThread : public QThread
{
    Q_OBJECT

public:
    // Destructor
    virtual ~QwtSamplingThread();

    // Get the interval in seconds
    double interval() const;
    // Get the elapsed time since the thread was started in seconds
    double elapsed() const;

public Q_SLOTS:
    // Set the interval in seconds
    void setInterval(double interval);
    // Stop the thread
    void stop();

protected:
    /// Constructor
    explicit QwtSamplingThread(QObject* parent = nullptr);

    /// Run the thread
    virtual void run() override;

    /**
     * \if ENGLISH
     * @brief Collect a sample
     *
     * @param elapsed Time since the thread was started in seconds
     * @note Due to a bug in previous version elapsed was passed as
     *       seconds instead of miliseconds. To avoid breaking existing
     *       code we stay with seconds for now.
     * \endif
     *
     * \if CHINESE
     * @brief 收集样本
     *
     * @param elapsed 线程启动以来的时间（秒）
     * @note 由于之前版本的 bug，elapsed 被作为秒而不是毫秒传递。为了避免破坏现有代码，我们现在仍然使用秒。
     * \endif
     */
    virtual void sample(double elapsed) = 0;

private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
