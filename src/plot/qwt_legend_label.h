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

#ifndef QWT_LEGEND_LABEL_H
#define QWT_LEGEND_LABEL_H

#include "qwt_global.h"
#include "qwt_text_label.h"
#include "qwt_legend_data.h"

class QwtText;

/**
 * \if ENGLISH
 * @brief A widget representing an item on a QwtLegend
 * @details QwtLegendLabel is used to display legend items in a QwtLegend.
 *          It can show an icon and a text label, and supports different
 *          interaction modes (ReadOnly, Clickable, Checkable).
 * \sa QwtLegend, QwtLegendData
 * \endif
 * \if CHINESE
 * @brief 表示 QwtLegend 上某个条目的控件
 * @details QwtLegendLabel 用于在 QwtLegend 中显示图例条目。
 *          它可以显示图标和文本标签，并支持不同的交互模式
 *          （只读、可点击、可选中）。
 * \sa QwtLegend, QwtLegendData
 * \endif
 */
class QWT_EXPORT QwtLegendLabel : public QwtTextLabel
{
    Q_OBJECT
  public:
    /// Constructor for QwtLegendLabel (English only)
    explicit QwtLegendLabel( QWidget* parent = 0 );
    
    /// Destructor for QwtLegendLabel (English only)
    virtual ~QwtLegendLabel();

    /// Set the legend data (English only)
    void setData( const QwtLegendData& );
    
    /// Return the legend data (English only)
    const QwtLegendData& data() const;

    /// Set the item mode (English only)
    void setItemMode( QwtLegendData::Mode );
    
    /// Return the item mode (English only)
    QwtLegendData::Mode itemMode() const;

    /// Set the spacing between icon and text (English only)
    void setSpacing( int spacing );
    
    /// Return the spacing (English only)
    int spacing() const;

    /// Set the text label (English only)
    virtual void setText( const QwtText& ) override;

    /// Set the icon (English only)
    void setIcon( const QPixmap& );
    
    /// Return the icon (English only)
    QPixmap icon() const;

    /// Return the size hint (English only)
    virtual QSize sizeHint() const override;

    /// Return whether the item is checked (English only)
    bool isChecked() const;

  public Q_SLOTS:
    /// Set whether the item is checked (English only)
    void setChecked( bool on );

  Q_SIGNALS:
    /**
     * \if ENGLISH
     * @brief Signal emitted when the legend item has been clicked
     * \endif
     * \if CHINESE
     * @brief 当图例项被点击时发出的信号
     * \endif
     */
    void clicked();

    /**
     * \if ENGLISH
     * @brief Signal emitted when the legend item has been pressed
     * \endif
     * \if CHINESE
     * @brief 当图例项被按下时发出的信号
     * \endif
     */
    void pressed();

    /**
     * \if ENGLISH
     * @brief Signal emitted when the legend item has been released
     * \endif
     * \if CHINESE
     * @brief 当图例项被释放时发出的信号
     * \endif
     */
    void released();

    /**
     * \if ENGLISH
     * @brief Signal emitted when the legend item has been toggled
     * @param on True if checked, false otherwise
     * \endif
     * \if CHINESE
     * @brief 当图例项状态切换时发出的信号
     * @param on 如果选中为 true，否则为 false
     * \endif
     */
    void checked( bool );

  protected:
    /// Set whether the button is down (English only)
    void setDown( bool );
    
    /// Return whether the button is down (English only)
    bool isDown() const;

    /// Handle paint events (English only)
    virtual void paintEvent( QPaintEvent* ) override;
    
    /// Handle mouse press events (English only)
    virtual void mousePressEvent( QMouseEvent* ) override;
    
    /// Handle mouse release events (English only)
    virtual void mouseReleaseEvent( QMouseEvent* ) override;
    
    /// Handle key press events (English only)
    virtual void keyPressEvent( QKeyEvent* ) override;
    
    /// Handle key release events (English only)
    virtual void keyReleaseEvent( QKeyEvent* ) override;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
