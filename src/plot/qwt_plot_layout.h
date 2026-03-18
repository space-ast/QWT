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

#ifndef QWT_PLOT_LAYOUT_H
#define QWT_PLOT_LAYOUT_H

#include "qwt_global.h"
#include "qwt_plot.h"
#include "qwt_axis_id.h"
class QwtPlotLayoutEngine;
/**
 * \if ENGLISH
 * @brief Layout engine for QwtPlot
 * @details It is used by the QwtPlot widget to organize its internal widgets
 *          or by QwtPlot::print() to render its content to a QPaintDevice like
 *          a QPrinter, QPixmap/QImage or QSvgRenderer.
 * @sa QwtPlot::setPlotLayout()
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlot 的布局引擎
 * @details 它被 QwtPlot 部件用于组织其内部部件，
 *          或被 QwtPlot::print() 用于将其内容渲染到 QPaintDevice 上，
 *          如 QPrinter、QPixmap/QImage 或 QSvgRenderer。
 * @sa QwtPlot::setPlotLayout()
 * \endif
 */

class QWT_EXPORT QwtPlotLayout
{
public:
    /*!
       Options to configure the plot layout engine
       \sa activate(), QwtPlotRenderer
     */
    enum Option
    {
        //! Unused
        AlignScales = 0x01,

        /*!
           Ignore the dimension of the scrollbars. There are no
           scrollbars, when the plot is not rendered to widgets.
         */
        IgnoreScrollbars = 0x02,

        //! Ignore all frames.
        IgnoreFrames = 0x04,

        //! Ignore the legend.
        IgnoreLegend = 0x08,

        //! Ignore the title.
        IgnoreTitle = 0x10,

        //! Ignore the footer.
        IgnoreFooter = 0x20
    };

    Q_DECLARE_FLAGS(Options, Option)

    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
explicit QwtPlotLayout();

    /**
     * \if ENGLISH
     * @brief Destructor
     * \endif
     */
virtual ~QwtPlotLayout();

    /**
     * \if ENGLISH
     * @brief Set canvas margin
     * \endif
     */
void setCanvasMargin(int margin, int axis = -1);

    /**
     * \if ENGLISH
     * @brief Get canvas margin
     * \endif
     */
int canvasMargin(int axisId) const;

    /**
     * \if ENGLISH
     * @brief Set align canvas to scales
     * \endif
     */
void setAlignCanvasToScales(bool);

    /**
     * \if ENGLISH
     * @brief Set align canvas to scale
     * \endif
     */
void setAlignCanvasToScale(int axisId, bool);

    /**
     * \if ENGLISH
     * @brief Check if canvas is aligned to scale
     * \endif
     */
bool alignCanvasToScale(int axisId) const;

    /**
     * \if ENGLISH
     * @brief Set spacing
     * \endif
     */
void setSpacing(int);

    /**
     * \if ENGLISH
     * @brief Get spacing
     * \endif
     */
int spacing() const;

    /**
     * \if ENGLISH
     * @brief Set legend position
     * \endif
     */
void setLegendPosition(QwtPlot::LegendPosition pos, double ratio);

    /**
     * \if ENGLISH
     * @brief Set legend position
     * \endif
     */
void setLegendPosition(QwtPlot::LegendPosition pos);

    /**
     * \if ENGLISH
     * @brief Get legend position
     * \endif
     */
QwtPlot::LegendPosition legendPosition() const;

    /**
     * \if ENGLISH
     * @brief Set legend ratio
     * \endif
     */
void setLegendRatio(double ratio);

    /**
     * \if ENGLISH
     * @brief Get legend ratio
     * \endif
     */
double legendRatio() const;

    /**
     * \if ENGLISH
     * @brief Get minimum size hint
     * \endif
     */
virtual QSize minimumSizeHint(const QwtPlot*) const;

    /**
     * \if ENGLISH
     * @brief Activate the layout
     * \endif
     */
virtual void activate(const QwtPlot* plot, const QRectF& plotRect, Options options = Options());

    /**
     * \if ENGLISH
     * @brief Invalidate the layout
     * \endif
     */
virtual void invalidate();

    /**
     * \if ENGLISH
     * @brief Get title rect
     * \endif
     */
QRectF titleRect() const;

    /**
     * \if ENGLISH
     * @brief Get footer rect
     * \endif
     */
QRectF footerRect() const;

    /**
     * \if ENGLISH
     * @brief Get legend rect
     * \endif
     */
QRectF legendRect() const;

    /**
     * \if ENGLISH
     * @brief Get scale rect
     * \endif
     */
QRectF scaleRect(QwtAxisId) const;

    /**
     * \if ENGLISH
     * @brief Get canvas rect
     * \endif
     */
QRectF canvasRect() const;

protected:
    /**
     * \if ENGLISH
     * @brief Set title rect
     * \endif
     */
void setTitleRect(const QRectF&);

    /**
     * \if ENGLISH
     * @brief Set footer rect
     * \endif
     */
void setFooterRect(const QRectF&);

    /**
     * \if ENGLISH
     * @brief Set legend rect
     * \endif
     */
void setLegendRect(const QRectF&);

    /**
     * \if ENGLISH
     * @brief Set scale rect
     * \endif
     */
void setScaleRect(QwtAxisId, const QRectF&);

    /**
     * \if ENGLISH
     * @brief Set canvas rect
     * \endif
     */
void setCanvasRect(const QRectF&);

    /**
     * \if ENGLISH
     * @brief Get layout engine
     * \endif
     */
QwtPlotLayoutEngine* layoutEngine();

    /**
     * \if ENGLISH
     * @brief Do activate the layout
     * \endif
     */
void doActivate(const QwtPlot* plot, const QRectF& plotRect, Options options = Options());

private:
    Q_DISABLE_COPY(QwtPlotLayout)

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QwtPlotLayout::Options)

#endif
