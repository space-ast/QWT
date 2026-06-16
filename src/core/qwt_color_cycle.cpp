/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_color_cycle.h"

#include <qvector.h>

/**
 * @brief Construct with the Default10 palette (matplotlib classic)
 */
QwtColorCycle::QwtColorCycle()
    : m_colors(paletteColors(Default10))
{
}

/**
 * @brief Construct with a built-in palette
 * @param palette Built-in palette selector
 */
QwtColorCycle::QwtColorCycle(Palette palette)
    : m_colors(paletteColors(palette))
{
}

/**
 * @brief Construct with custom colors
 * @param colors Custom color list
 */
QwtColorCycle::QwtColorCycle(const QVector< QColor >& colors)
    : m_colors(colors)
{
}

/**
 * @brief Get a color by index with cyclic wrap-around
 * @param index Zero-based index (negative values are handled)
 * @return Color at position (index % count)
 */
QColor QwtColorCycle::color(int index) const
{
    if (m_colors.isEmpty())
        return Qt::black;

    if (index < 0)
        index = -index;

    return m_colors.at(index % m_colors.size());
}

/**
 * @brief Get the number of colors in the cycle
 * @return Number of colors
 */
int QwtColorCycle::count() const
{
    return m_colors.size();
}

/**
 * @brief Replace current colors with a built-in palette
 * @param palette Built-in palette selector
 */
void QwtColorCycle::setPalette(Palette palette)
{
    m_colors = paletteColors(palette);
}

/**
 * @brief Replace current colors with a custom list
 * @param colors Custom color list
 */
void QwtColorCycle::setColors(const QVector< QColor >& colors)
{
    m_colors = colors;
}

/**
 * @brief Get the current color list
 * @return Copy of the internal color list
 */
QVector< QColor > QwtColorCycle::colors() const
{
    return m_colors;
}

/**
 * @brief Get the color list for a built-in palette
 * @param palette Built-in palette selector
 * @return Color list for the specified palette
 */
QVector< QColor > QwtColorCycle::paletteColors(Palette palette)
{
    switch (palette) {
        case Default10:
            return {
                QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"),
                QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"),
                QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"),
                QColor("#17becf")
            };

        case Tab10:
            return {
                QColor("#4e79a7"), QColor("#f28e2b"), QColor("#e15759"),
                QColor("#76b7b2"), QColor("#59a14f"), QColor("#edc948"),
                QColor("#b07aa1"), QColor("#ff9da7"), QColor("#9c755f"),
                QColor("#bab0ac")
            };

        case Tab20:
            return {
                QColor("#4e79a7"), QColor("#a0cbe8"), QColor("#f28e2b"),
                QColor("#ffbe7d"), QColor("#e15759"), QColor("#ff9d9a"),
                QColor("#76b7b2"), QColor("#59a14f"), QColor("#8cd17d"),
                QColor("#b6992d"), QColor("#edc948"), QColor("#b07aa1"),
                QColor("#d37295"), QColor("#ff9da7"), QColor("#9c755f"),
                QColor("#bab0ac"), QColor("#86bcb6"), QColor("#499894"),
                QColor("#f1ce63"), QColor("#d4a6c8")
            };

        case Set1:
            return {
                QColor("#e41a1c"), QColor("#377eb8"), QColor("#4daf4a"),
                QColor("#984ea3"), QColor("#ff7f00"), QColor("#ffff33"),
                QColor("#a65628"), QColor("#f781bf"), QColor("#999999")
            };

        case Set2:
            return {
                QColor("#66c2a5"), QColor("#fc8d62"), QColor("#8da0cb"),
                QColor("#e78ac3"), QColor("#a6d854"), QColor("#ffd92f"),
                QColor("#e5c494"), QColor("#b3b3b3")
            };

        case Set3:
            return {
                QColor("#8dd3c7"), QColor("#ffffb3"), QColor("#bebada"),
                QColor("#fb8072"), QColor("#80b1d3"), QColor("#fdb462"),
                QColor("#b3de69"), QColor("#fccde5"), QColor("#d9d9d9"),
                QColor("#bc80bd"), QColor("#ccebc5"), QColor("#ffed6f")
            };

        case OkabeIto:
            return {
                QColor("#E69F00"), QColor("#56B4E9"), QColor("#009E73"),
                QColor("#F0E442"), QColor("#0072B2"), QColor("#D55E00"),
                QColor("#CC79A7"), QColor("#000000")
            };
    }

    return paletteColors(Default10);
}
