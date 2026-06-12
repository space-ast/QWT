/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <qwt_series_data.h>
#include <qwt_math.h>
#include <QVector>
#include <cmath>

class WindowedSeriesData : public QwtSeriesData< QPointF >
{
public:
    WindowedSeriesData()
        : m_offset(0)
        , m_displaySize(0)
    {
    }

    void generate(int totalSize, bool useWave)
    {
        m_xData.resize(totalSize);
        m_yData.resize(totalSize);

        for (int i = 0; i < totalSize; i++) {
            const double x = static_cast< double >(i);
            m_xData[ i ] = x;

            if (useWave) {
                const double period = 1.0;
                const double c      = 5.0;
                double v            = std::fmod(x * 0.001, period);
                const double amplitude
                    = std::abs(x * 0.001 - std::round(x * 0.001 / c) * c) / (0.5 * c);
                v = amplitude * std::sin(v / period * 2.0 * M_PI);
                v += 0.3 * std::sin(x * 0.05);
                v += 0.1 * std::sin(x * 0.2);
                m_yData[ i ] = v;
            } else {
                m_yData[ i ]
                    = 2.0 * (qwtRand() / (static_cast< double >(RAND_MAX) + 1)) - 1.0;
            }
        }

        m_totalSize = totalSize;
    }

    void setWindow(int offset, int displaySize)
    {
        m_offset      = offset;
        m_displaySize = displaySize;
        cachedBoundingRect = QRectF(0, 0, -1, -1);
    }

    int maxOffset(int displaySize) const
    {
        return m_totalSize - displaySize;
    }

    int totalSize() const { return m_totalSize; }

    virtual size_t size() const override
    {
        return static_cast< size_t >(m_displaySize);
    }

    virtual QPointF sample(size_t i) const override
    {
        const int idx = m_offset + static_cast< int >(i);
        return QPointF(m_xData[ idx ], m_yData[ idx ]);
    }

    virtual QRectF boundingRect() const override
    {
        if (cachedBoundingRect.width() < 0) {
            if (m_displaySize == 0) {
                cachedBoundingRect = QRectF(0, 0, 0, 0);
            } else {
                double minY = m_yData[ m_offset ];
                double maxY = minY;
                for (int i = 1; i < m_displaySize; i++) {
                    const double y = m_yData[ m_offset + i ];
                    if (y < minY)
                        minY = y;
                    else if (y > maxY)
                        maxY = y;
                }
                const double x0 = m_xData[ m_offset ];
                const double x1 = m_xData[ m_offset + m_displaySize - 1 ];
                cachedBoundingRect = QRectF(x0, minY, x1 - x0, maxY - minY);
            }
        }
        return cachedBoundingRect;
    }

private:
    QVector< double > m_xData;
    QVector< double > m_yData;
    int m_totalSize   = 0;
    int m_offset      = 0;
    int m_displaySize = 0;
};
