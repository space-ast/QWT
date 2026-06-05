/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 2024   ChenZongYan <czy.t@163.com>
 *****************************************************************************/
#include "qwt_plot_series_data_picker.h"
// stl
#include <algorithm>
#include <limits>
// Qt
#include <QMouseEvent>
// qwt
#include "qwt_utils.h"
#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_item.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_scale_draw.h"
// qt
#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QHash>

// Whether to group by X values; grouping differentiates by the X axis that each item belongs to,
// primarily for multi-X-axis scenarios. Currently works correctly on Qt5 but is anomalous on Qt6
// for unknown reasons.
#ifndef QwtPlotSeriesDataPicker_XGroup
#define QwtPlotSeriesDataPicker_XGroup 0
#endif
/**
 * @brief Computes the search window range for finding the nearest point in curve data.
 *
 * This function uses binary search to quickly locate the approximate position of the target X
 * coordinate within the curve data, then sets the search start and end indices based on the
 * window size. For small datasets (fewer than 1000 points), the entire range is searched
 * directly; for larger datasets, window optimization is used to improve performance.
 *
 * @param[in] curveSize Total number of data points in the curve
 * @param[in] targetX Target X coordinate (in data coordinate space)
 * @param[in] data Curve data series, must be sorted in ascending X order
 * @param[in] windowSize Window size configuration
 *        - 0: No window, search the entire curve
 *        - Positive: Fixed window size (number of data points)
 *        - Negative: Adaptive window, using a percentage of total curve points (absolute value, e.g. -5 means 5%)
 * @return pair<startIndex,endIndex>, startIndex: computed search start index (inclusive); endIndex: computed search end index (inclusive)
 *
 * @note When the curve has fewer than 1000 points, window optimization is automatically disabled
 *       and the entire curve is searched for best accuracy.
 * @note Percentage calculation: windowSize = -5 means 5% of total curve points used as window size.
 * @note The function assumes curve data is sorted in ascending X order.
 * @note Adaptive window size is clamped between 50 (minimum) and 1000 (maximum) points.
 * @note If the computed window covers more than 80% of data points, it automatically degrades to searching the entire curve.
 *
 * @par Performance strategy:
 * - Data points < 1000: Search entire curve (linear search overhead acceptable)
 * - Data points >= 1000: Use window optimization (significantly reduces comparison count)
 *
 * @see qwtUpperSampleIndex
 * @see QwtSeriesData
 * @see QwtPlotSeriesDataPicker::pickNearestPoint
 */
QPair< size_t, size_t > calculateSearchWindow(
    size_t curveSize, double targetX, const QwtSeriesData< QPointF >& data, int windowSize = -5
)
{
    // Initialize default range: entire curve
    size_t startIndex;
    size_t endIndex;
    startIndex = 0;
    endIndex   = (curveSize > 0) ? curveSize - 1 : 0;

    // Define performance threshold: below this number of data points, window optimization is disabled
    const size_t WINDOW_OPTIMIZATION_THRESHOLD = 1000;

    // If the curve has small data or window usage is explicitly disabled, search entire range
    if (curveSize <= 1 || windowSize == 0 || curveSize < WINDOW_OPTIMIZATION_THRESHOLD) {
        return qMakePair(startIndex, endIndex);
    }

    // Compute actual window size
    size_t realWindowSize;
    if (windowSize < 0) {
        // Adaptive mode: use percentage of curve point count
        // windowSize = -5 means 5%, windowSize = -10 means 10%
        double percentage = std::abs(windowSize) / 100.0;
        realWindowSize    = static_cast< size_t >(curveSize * percentage);

        // Ensure adaptive window is within reasonable range
        const size_t MIN_ADAPTIVE_WINDOW = 50;
        const size_t MAX_ADAPTIVE_WINDOW = 1000;
        realWindowSize                   = std::max(realWindowSize, MIN_ADAPTIVE_WINDOW);
        realWindowSize                   = std::min(realWindowSize, MAX_ADAPTIVE_WINDOW);
    } else {
        // Fixed window size
        realWindowSize = static_cast< size_t >(windowSize);
    }

    // Ensure window size is within valid range
    realWindowSize = std::max< size_t >(1, realWindowSize);
    realWindowSize = std::min< size_t >(realWindowSize, curveSize);

    // Use binary search to locate approximate position of target X coordinate
    size_t centerIndex = qwtUpperSampleIndex< QPointF >(data, targetX, [](const double x, const QPointF& point) -> bool {
        return (x < point.x());
    });

    // Compute window boundaries based on center position
    if (centerIndex == curveSize) {
        // Case 1: Target X greater than all data points, on right side of curve
        // Window set at curve end
        if (realWindowSize < curveSize) {
            startIndex = curveSize - realWindowSize;
        }
        // endIndex already set to curveSize - 1
    } else if (centerIndex == 0) {
        // Case 2: Target X less than or equal to first data point, on left side of curve
        // Window set at curve start
        endIndex = std::min(realWindowSize - 1, curveSize - 1);
    } else {
        // Case 3: Target X within curve data range
        // Set window centered around centerIndex

        // Compute half-window width
        size_t halfWindow = realWindowSize / 2;

        // Compute start index, ensuring it does not go below 0
        if (centerIndex > halfWindow) {
            startIndex = centerIndex - halfWindow;
        } else {
            startIndex = 0;
        }

        // Compute end index, ensuring it does not exceed curve end
        endIndex = centerIndex + halfWindow;
        if (endIndex >= curveSize) {
            endIndex = curveSize - 1;
            // If end index was adjusted, adjust start index accordingly to maintain window size
            if (endIndex - startIndex + 1 > realWindowSize) {
                startIndex = endIndex - realWindowSize + 1;
            }
        } else {
            // If window size is odd, adjust end index to maintain exact window size
            if (realWindowSize % 2 == 1 && endIndex - startIndex + 1 < realWindowSize) {
                endIndex = startIndex + realWindowSize - 1;
                if (endIndex >= curveSize) {
                    endIndex = curveSize - 1;
                }
            }
        }
    }

    // Final boundary check, ensure indices are within valid range
    startIndex = std::min(startIndex, curveSize - 1);
    endIndex   = std::min(endIndex, curveSize - 1);

    // Ensure start index is not greater than end index
    if (startIndex > endIndex) {
        std::swap(startIndex, endIndex);
    }

    // Final window size validation
    // size_t actualWindowSize = endIndex - startIndex + 1;
    // If the window actually encompasses most of the data, search the whole curve instead
    // const double FULL_SEARCH_THRESHOLD = 0.8;  // 80%
    // if (actualWindowSize >= curveSize * FULL_SEARCH_THRESHOLD) {
    //     startIndex = 0;
    //     endIndex   = curveSize - 1;
    // }
    return qMakePair(startIndex, endIndex);
}

class QwtPlotSeriesDataPicker::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotSeriesDataPicker)
public:
    PrivateData(QwtPlotSeriesDataPicker* p);
#if QwtPlotSeriesDataPicker_XGroup
    struct GroupKey
    {
        QwtPlot* plot { nullptr };
        QwtAxisId axis { QwtAxis::XBottom };
        // Get effective plot (if parasite plot with shared axis, return host plot)
        QwtPlot* effectivePlot() const
        {
            if (plot && plot->isParasitePlot() && plot->isParasiteShareAxis(axis)) {
                QwtPlot* host = plot->hostPlot();
                if (host)
                    return host;
            }
            return plot;
        }

        bool operator==(const GroupKey& o) const
        {
            if (plot == o.plot && axis == o.axis) {
                return true;
            }

            // Handle parasite axis case
            QwtPlot* effPlot1 = effectivePlot();
            QwtPlot* effPlot2 = o.effectivePlot();

            return (effPlot1 == effPlot2) && (axis == o.axis);
        }

        // Provide hash function for QHash
        friend inline size_t qHash(const GroupKey& key, uint seed = 0)
        {
            // Use qHash to hash pointer and integer
            auto h1 = qHash(reinterpret_cast< quintptr >(key.effectivePlot()), seed);
            auto h2 = qHash(key.axis, seed);

            // Combine hash values
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    struct XGroup
    {
        GroupKey key;
        QString xValue;  // Common X value
        QList< const FeaturePoint* > fps;
    };
    QVector< XGroup > xGroups;
#endif
public:
    QwtPlotSeriesDataPicker::PickSeriesMode pickMode { QwtPlotSeriesDataPicker::PickYValue };
    QwtPlotSeriesDataPicker::TextPlacement textArea { QwtPlotSeriesDataPicker::TextPlaceAuto };
    QwtPlotSeriesDataPicker::InterpolationMode interpolationMode { QwtPlotSeriesDataPicker::LinearInterpolation };
    // Rendering related
    QBrush textBackgroundBrush { QColor(255, 255, 255, 180) };
    Qt::Alignment textAlignment { Qt::AlignLeft | Qt::AlignVCenter };
    // Recorded feature points
    int nearestSearchWindowSize { -5 };
    QList< FeaturePoint > featurePoints;
    int featurePointSize { 4 };      ///< Feature point size
    bool markFeaturePoint { true };  ///< Whether to mark captured feature points
    QPoint mousePos;
    bool enableShowXOnPicker { true };
    QPoint textTrackerOffset { 15, 0 };  ///< Tracker text offset; effective in TextFollowMouse mode (setting this prevents text from being too close to mouse cursor)
};

QwtPlotSeriesDataPicker::PrivateData::PrivateData(QwtPlotSeriesDataPicker* p) : q_ptr(p)
{
}

//===============================================================
// QwtPlotSeriesDataPicker
//===============================================================

QwtPlotSeriesDataPicker::QwtPlotSeriesDataPicker(QWidget* canvas) : QwtCanvasPicker(canvas), QWT_PIMPL_CONSTRUCT
{
    // Set tracking mode, always display tracking info
    setTrackerMode(QwtPicker::ActiveOnly);
    // Set rubber band to vertical line
    setRubberBand(QwtPicker::UserRubberBand);
    // Set state machine for point selection
    setStateMachine(new QwtPickerTrackerMachine);
    //
    QwtPlot* host = plot();
    if (host->isParasitePlot()) {
        host = host->hostPlot();
        if (!host) {
            host = plot();
        }
    }
    QList< QwtPlot* > allPlots = host->plotList();
    for (const QwtPlot* p : allPlots) {
        connect(p, &QwtPlot::itemAttached, this, &QwtPlotSeriesDataPicker::onPlotItemDetached);
    }
    connect(host, &QwtPlot::parasitePlotAttached, this, &QwtPlotSeriesDataPicker::onParasitePlotAttached);
}

QwtPlotSeriesDataPicker::~QwtPlotSeriesDataPicker()
{
}

/**
 * @brief Set pick mode
 * @param[in] mode Pick mode
 * @sa pickMode()
 */
void QwtPlotSeriesDataPicker::setPickMode(PickSeriesMode mode)

{
    QWT_D(d);
    if (mode == d->pickMode) {
        return;
    }
    d->pickMode = mode;
}

/**
 * @brief Get current pick mode
 * @return Current pick mode
 * @sa setPickMode()
 */
QwtPlotSeriesDataPicker::PickSeriesMode QwtPlotSeriesDataPicker::pickMode() const
{
    return m_data->pickMode;
}

/**
 * @brief Set text display area
 * @param[in] t Text placement option
 * @sa textArea()
 */
void QwtPlotSeriesDataPicker::setTextArea(QwtPlotSeriesDataPicker::TextPlacement t)
{
    m_data->textArea = t;
}

/**
 * @brief Get text display position
 * @return Text placement option
 * @sa setTextArea()
 */
QwtPlotSeriesDataPicker::TextPlacement QwtPlotSeriesDataPicker::textArea() const
{
    return m_data->textArea;
}

/**
 * @brief Set interpolation mode
 * @param[in] mode Interpolation mode
 * @sa interpolationMode()
 */
void QwtPlotSeriesDataPicker::setInterpolationMode(QwtPlotSeriesDataPicker::InterpolationMode mode)
{
    m_data->interpolationMode = mode;
}

/**
 * @brief Get interpolation mode
 * @return Current interpolation mode
 * @sa setInterpolationMode()
 */
QwtPlotSeriesDataPicker::InterpolationMode QwtPlotSeriesDataPicker::interpolationMode() const
{
    return m_data->interpolationMode;
}

/**
 * @brief Check if interpolation is enabled
 * @details If interpolation is enabled, when the mouse is not on a data point,
 *          the corresponding point on the connecting line will be interpolated.
 * @return True if interpolation is enabled
 * @sa interpolationMode()
 */
bool QwtPlotSeriesDataPicker::isInterpolation() const
{
    return m_data->interpolationMode != NoInterpolation;
}

/**
 * @brief Set nearest point search window size
 * @details Window size determines the search range for nearest points, avoiding full curve traversal.
 *          Window size can be set to negative values, which will use a percentage of curve point count:
 *          - 0: No window, search entire curve
 *          - Positive: Fixed window size (number of data points)
 *          - Negative: Adaptive window, uses percentage of total curve points (absolute value, e.g. -5 means 5%)
 * @param[in] windowSize Window size (default is -5)
 * @sa nearestSearchWindowSize()
 */
void QwtPlotSeriesDataPicker::setNearestSearchWindowSize(int windowSize)
{
    m_data->nearestSearchWindowSize = windowSize;
}

/**
 * @brief Get nearest point search window size
 * @return Window size (may be negative, see setNearestSearchWindowSize())
 * @sa setNearestSearchWindowSize()
 */
int QwtPlotSeriesDataPicker::nearestSearchWindowSize() const
{
    return m_data->nearestSearchWindowSize;
}

/**
 * @brief Enable/disable feature point drawing
 * @param[in] on Enable/disable
 * @sa isEnableDrawFeaturePoint()
 */
void QwtPlotSeriesDataPicker::setEnableDrawFeaturePoint(bool on)
{
    m_data->markFeaturePoint = on;
}

/**
 * @brief Check if feature point drawing is enabled
 * @return True if enabled
 * @sa setEnableDrawFeaturePoint()
 */
bool QwtPlotSeriesDataPicker::isEnableDrawFeaturePoint() const
{
    return m_data->markFeaturePoint;
}

/**
 * @brief Set drawn feature point size
 * @param[in] px Size in pixels
 * @sa drawFeaturePointSize()
 */
void QwtPlotSeriesDataPicker::setDrawFeaturePointSize(int px)
{
    m_data->featurePointSize = px;
}

/**
 * @brief Get drawn feature point size
 * @return Size in pixels
 * @sa setDrawFeaturePointSize()
 */
int QwtPlotSeriesDataPicker::drawFeaturePointSize() const
{
    return m_data->featurePointSize;
}

/**
 * @brief Set text area background brush
 * @param[in] br Background brush
 * @sa textBackgroundBrush()
 */
void QwtPlotSeriesDataPicker::setTextBackgroundBrush(const QBrush& br)
{
    m_data->textBackgroundBrush = br;
}

/**
 * @brief Get text area background brush
 * @return Background brush
 * @sa setTextBackgroundBrush()
 */
QBrush QwtPlotSeriesDataPicker::textBackgroundBrush() const
{
    return m_data->textBackgroundBrush;
}

/**
 * @brief Set text alignment
 * @param[in] al Alignment flags
 * @sa textAlignment()
 */
void QwtPlotSeriesDataPicker::setTextAlignment(Qt::Alignment al)
{
    m_data->textAlignment = al;
}

/**
 * @brief Get text alignment
 * @return Alignment flags
 * @sa setTextAlignment()
 */
Qt::Alignment QwtPlotSeriesDataPicker::textAlignment() const
{
    return m_data->textAlignment;
}

/**
 * @brief Enable/disable showing X value
 * @param[in] on Enable/disable
 * @sa isEnableShowXValue()
 */
void QwtPlotSeriesDataPicker::setEnableShowXValue(bool on)
{
    m_data->enableShowXOnPicker = on;
}

/**
 * @brief Check if showing X value is enabled
 * @return True if enabled
 * @sa setEnableShowXValue()
 */
bool QwtPlotSeriesDataPicker::isEnableShowXValue() const
{
    return m_data->enableShowXOnPicker;
}

/**
 * @brief Set the offset of tracker rectangle in TextFollowMouse mode
 *
 * This method configures the positional offset for the tracker rectangle when operating
 * in TextFollowMouse mode. The offset prevents the rectangle from being positioned
 * directly adjacent to the mouse cursor, which enhances visual clarity and prevents
 * the tracker from obscuring the text content beneath the cursor.
 *
 * @param offset The offset value in pixels. Positive values move the tracker away
 *               from the cursor position. Recommended values are typically between
 *               10-30 pixels for optimal user experience.
 * @note The offset is applied relative to the current mouse position.
 * @see textTrackerOffset()
 * @see TextPlacement
 */
void QwtPlotSeriesDataPicker::setTextTrackerOffset(const QPoint& offset)
{
    QWT_D(d);
    d->textTrackerOffset = offset;
}

/**
 * @brief Get the current tracker rectangle offset in TextFollowMouse mode
 *
 * This method returns the current offset value used to position the tracker rectangle
 * relative to the mouse cursor in TextFollowMouse mode. The offset ensures that the
 * tracker rectangle is not placed directly under the mouse, preventing visual
 * obstruction of the underlying content.
 *
 * @return The current offset as a QPoint, where x and y represent the horizontal
 *         and vertical offsets in pixels respectively.
 * @note A return value of QPoint(0, 0) indicates no offset is applied.
 * @see setTextTrackerOffset()
 * @see TextPlacement
 */
QPoint QwtPlotSeriesDataPicker::textTrackerOffset() const
{
    QWT_DC(d);
    return d->textTrackerOffset;
}

QwtText QwtPlotSeriesDataPicker::trackerText(const QPoint& pos) const
{
    if (!isEnabled()) {
        return QwtText();
    }
    const QwtPlot* currentPlot = plot();
    if (!currentPlot) {
        return QwtText();
    }
    // Also search host plot if any
    QString text;

    QWT_DC(d);
    if (d->featurePoints.isEmpty()) {
        return QwtText();
    }
    text = valueString(d->featurePoints);
    if (text.isEmpty()) {
        // Fall back to default tracker text
        return QwtPicker::trackerText(pos);
    }

    QwtText trackerText(text);
    trackerText.setRenderFlags(int(d->textAlignment) | Qt::TextWordWrap);
    trackerText.setBackgroundBrush(d->textBackgroundBrush);

    return trackerText;
}

QString QwtPlotSeriesDataPicker::valueString(const QList< FeaturePoint >& fps) const
{
    if (fps.isEmpty()) {
        return {};
    }

    auto fmtX = [ & ](const FeaturePoint& fp) -> QString {
        if (!fp.item) {
            return QString::number(fp.feature.x());
        }
        QwtPlot* p = fp.item->plot();
        if (!p) {
            return QString::number(fp.feature.x());
        }
        return formatAxisValue(fp.feature.x(), fp.item->xAxis(), p);
    };
    auto fmtY = [ & ](const FeaturePoint& fp) -> QString {
        if (!fp.item) {
            return QString::number(fp.feature.y());
        }
        QwtPlot* p = fp.item->plot();
        if (!p) {
            return QString::number(fp.feature.y());
        }
        return formatAxisValue(fp.feature.y(), fp.item->yAxis(), p);
    };

    QString out;

    if (pickMode() == PickYValue) {
#if QwtPlotSeriesDataPicker_XGroup
        QWT_DC(d);
        if (!isEnableShowXValue()) {
            // Do not show X value
            for (int i = 0; i < fps.size(); ++i) {
                if (i > 0)
                    out += "<br/>";
                const FeaturePoint& fp = fps[ i ];
                QString stry           = fmtY(fp);
                out += QString(R"(<font color="%1">■</font>%2:<b>%3</b>)")
                           .arg(Qwt::plotItemColor(fp.item).name(), fp.item->title().text(), stry);
            }
        } else {

            // Show X value, grouped by X axis
            for (int ig = 0; ig < d->xGroups.size(); ++ig) {
                const PrivateData::XGroup& g = d->xGroups[ ig ];
                QwtPlot* plot                = g.key.plot;
                if (!plot || g.fps.isEmpty())
                    continue;

                // Check whether X axis is visible
                bool isAxisVisible = false;

                if (g.key.axis == QwtAxis::XTop) {
                    isAxisVisible = plot->isAxisVisible(QwtAxis::XTop);
                } else if (g.key.axis == QwtAxis::XBottom) {
                    isAxisVisible = plot->isAxisVisible(QwtAxis::XBottom);
                } else {
                    // Axes at other positions, treat as visible by default
                    isAxisVisible = true;
                }

                // If X axis is not visible, do not show X value
                if (!isAxisVisible) {
                    // Directly show curve data without X axis title
                    for (int il = 0; il < g.fps.size(); ++il) {
                        const FeaturePoint* fp = g.fps[ il ];
                        if (!out.isEmpty())
                            out += "<br/>";
                        QString stry = fmtY(*fp);
                        out += QString(R"(<font color="%1">■</font>%2: <b>%3</b>)")
                                   .arg(Qwt::plotItemColor(fp->item).name(), fp->item->title().text(), stry);
                    }
                } else {
                    // X axis visible, show group header
                    if (!out.isEmpty())
                        out += "<br/>";

                    // Get X axis title
                    QString axisTitle;
                    if (g.key.axis == QwtAxis::XTop) {
                        axisTitle = plot->axisTitle(QwtAxis::XTop).text();
                    } else if (g.key.axis == QwtAxis::XBottom) {
                        axisTitle = plot->axisTitle(QwtAxis::XBottom).text();
                    }

                    if (axisTitle.isEmpty()) {
                        out += QString("<b>%1</b>").arg(g.xValue);
                    } else {
                        // Show X axis title and shared X value
                        out += QString("<b>%1: %2</b>").arg(axisTitle, g.xValue);
                    }

                    // Add all curves and their Y values within this group
                    for (int il = 0; il < g.fps.size(); ++il) {
                        const FeaturePoint* fp = g.fps[ il ];
                        out += "<br/>";
                        QString stry = fmtY(*fp);
                        out += QString(R"(<font color="%1">■</font>%2: <b>%3</b>)")
                                   .arg(Qwt::plotItemColor(fp->item).name(), fp->item->title().text(), stry);
                    }
                }
            }

            // If no grouping, fall back to non-grouped display (backup)
            if (out.isEmpty()) {
                for (int i = 0; i < fps.size(); ++i) {
                    if (i > 0)
                        out += "<br/>";
                    const FeaturePoint& fp = fps[ i ];
                    QString stry           = fmtY(fp);
                    out += QString(R"(<font color="%1">■</font>%2:<b>%3</b>)")
                               .arg(Qwt::plotItemColor(fp.item).name(), fp.item->title().text(), stry);
                }
            }
        }
#else
        if (isEnableShowXValue()) {
            // Show X value
            const FeaturePoint& fp = fps.first();
            out += fmtX(fp);
            out += "<br/>";
        }
        for (int i = 0; i < fps.size(); ++i) {
            if (i > 0) {
                out += "<br/>";
            }
            const FeaturePoint& fp = fps[ i ];
            QString stry           = fmtY(fp);
            out += QString(R"(<font color="%1">■</font>%2:<b>%3</b>)")
                       .arg(Qwt::plotItemColor(fp.item).name(), fp.item->title().text(), stry);
        }
#endif
    } else {
        // PickNearestPoint mode
        if (!isEnableShowXValue()) {
            // Show Y value only
            for (int i = 0; i < fps.size(); ++i) {
                if (i > 0)
                    out += "<br/>";
                out += fmtY(fps[ i ]);
            }
        } else {
            // Show full coordinates (X, Y)
            for (int i = 0; i < fps.size(); ++i) {
                if (i > 0)
                    out += "<br/>";
                const FeaturePoint& fp = fps[ i ];
                out += QString("(%1, %2)").arg(fmtX(fp), fmtY(fp));
            }
        }
    }
    return out;
}

/**
 * @brief Draw captured feature points
 *
 * Renders all captured feature points using the provided painter.
 *
 * @note Control visibility via setEnableDrawFeaturePoint()
 * @param painter Drawing context
 */
void QwtPlotSeriesDataPicker::drawAllFeaturePoints(QPainter* painter) const
{
    QWT_DC(d);
    const QList< QwtPlotSeriesDataPicker::FeaturePoint >& pickedFeatureDatas = d->featurePoints;
    for (int i = 0; i < pickedFeatureDatas.size(); ++i) {
        const QwtPlotSeriesDataPicker::FeaturePoint& fp = pickedFeatureDatas[ i ];
        QwtPlot* itemPlot                               = fp.item->plot();
        drawFeaturePoint(painter, itemPlot, fp.item, fp.feature);
    }
}

/**
 * @brief Draw a single feature point on the plot
 *
 * This virtual method is responsible for rendering individual feature points
 * on the QwtPlot canvas. Users can override this function to implement
 * custom drawing styles for feature points (e.g., different shapes, colors,
 * or visual effects).
 *
 * The default implementation draws a circular point at the specified position:
 * - Size: determined by drawFeaturePointSize()
 * - Border: 1-pixel outline using the curve color
 * - Fill: solid fill with the curve color darkened by 150%
 *
 * @param painter The QPainter object for drawing operations
 * @param plot Reference to the QwtPlot widget where drawing occurs
 * @param item The curve item associated with this feature point
 * @param itemPoint The coordinates of the feature point in plot coordinates
 *
 * @see drawFeaturePointSize()
 * @see setDrawFeaturePointSize()
 * @see drawAllFeaturePoints()
 */
void QwtPlotSeriesDataPicker::drawFeaturePoint(
    QPainter* painter, const QwtPlot* plot, const QwtPlotItem* item, const QPointF& itemPoint
) const
{
    if (!plot || !item) {
        return;
    }
    QWT_DC(d);
    const QwtScaleMap xMap = plot->canvasMap(item->xAxis());
    const QwtScaleMap yMap = plot->canvasMap(item->yAxis());
    // Transform point to screen coordinates
    QPointF screenPos = QwtScaleMap::transform(xMap, yMap, itemPoint);
    QColor itemColor  = Qwt::plotItemColor(item, Qt::black);
    // Draw the point
    painter->save();
    QColor fillColor = itemColor.darker(150);  // 150% darker, adjustable as needed
    // Set pen (border)
    painter->setPen(QPen(fillColor, 1));
    // Set brush (fill)
    painter->setBrush(QBrush(fillColor));
    painter->drawEllipse(screenPos.toPoint(), d->featurePointSize, d->featurePointSize);
    painter->restore();
}

void QwtPlotSeriesDataPicker::move(const QPoint& pos)
{
    updateFeaturePoint(pos);
    QwtPicker::move(pos);
}

/**
 * @brief Handle mouse press events on the widget
 * @details Emits clicked() for left-button presses, after updating feature points
 *          at the press position to ensure featurePoints() returns accurate data.
 *          Then delegates to base class for normal picker transition handling.
 * @param event The mouse press event
 */
void QwtPlotSeriesDataPicker::widgetMousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        updateFeaturePoint(pos);
        emit clicked(this, pos);
    }
    QwtPicker::widgetMousePressEvent(event);
}

/**
 * @brief Handle mouse double-click events on the widget
 * @details Emits doubleClicked() for left-button double-clicks.
 *          A double-click also triggers clicked() (via widgetMousePressEvent)
 *          before this signal. Then delegates to base class.
 * @param event The mouse double-click event
 */
void QwtPlotSeriesDataPicker::widgetMouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        emit doubleClicked(this, pos);
    }
    QwtPicker::widgetMouseDoubleClickEvent(event);
}

/**
 * @brief Returns the list of feature points currently picked by the tracker
 * @return Copy of the internal FeaturePoint list
 * @note Call from a clicked/doubleClicked signal handler to get data at the click position
 */
QList<QwtPlotSeriesDataPicker::FeaturePoint> QwtPlotSeriesDataPicker::featurePoints() const
{
    QWT_DC(d);
    return d->featurePoints;
}

QString QwtPlotSeriesDataPicker::formatAxisValue(double value, int axisId, QwtPlot* plot) const
{
    if (!plot) {
        return QString::number(value);
    }

    // Get the axis scale drawer
    const QwtScaleDraw* scaleDraw = plot->axisScaleDraw(axisId);
    if (scaleDraw) {
        // Use the axis formatter
        QwtText text = scaleDraw->label(value);
        return text.text();
    }

    // Fall back to default numeric display
    return QString::number(value);
}

void QwtPlotSeriesDataPicker::updateFeaturePoint(const QPoint& pos)
{
    const QwtPlot* currentPlot = plot();
    if (!currentPlot) {
        return;
    }
    m_data->mousePos = pos;
    switch (pickMode()) {
    case PickYValue:
        pickYValue(currentPlot, pos, isInterpolation());
        break;
    case PickNearestPoint:
        pickNearestPoint(currentPlot, pos, nearestSearchWindowSize());
        break;
    default:
        break;
    }
}

QRect QwtPlotSeriesDataPicker::trackerRect(const QFont& f) const
{
    QRect r = QwtPicker::trackerRect(f);
    // Early exit for cases that don't need rect position adjustment
    QwtPlotSeriesDataPicker::TextPlacement ta = textArea();
    if (QwtPlotSeriesDataPicker::TextPlaceAuto == ta && pickMode() == PickNearestPoint) {
        return r;
    }
    QWT_DC(d);
    const QRect plotRect = pickArea().boundingRect().toRect();
    // Adjust rect position based on textArea and pickMode
    if (QwtPlotSeriesDataPicker::TextPlaceAuto == ta) {
        // For TextPlaceAuto, only PickYValue mode needs special handling
        if (pickMode() == PickYValue) {
            // Predict offset position
            QPoint offset    = textTrackerOffset();
            QRect offsetRect = r.translated(offset);
            return ensureRectInBounds(offsetRect, plotRect);
        }
        // Other pickModes keep rect unchanged
        return r;
    }
    // Adjust based on specified textArea position
    switch (ta) {
    case TextFollowOnTop:
        r.moveTop(plotRect.top());
        break;
    case TextFollowOnBottom:
        r.moveBottom(plotRect.bottom());
        break;
    case TextOnCanvasTopRight:
        r.moveTopRight(plotRect.topRight());
        break;
    case TextOnCanvasTopLeft:
        r.moveTopLeft(plotRect.topLeft());
        break;
    case TextOnCanvasBottomRight:
        r.moveBottomRight(plotRect.bottomRight());
        break;
    case TextOnCanvasBottomLeft:
        r.moveBottomLeft(plotRect.bottomLeft());
        break;
    case TextOnCanvasTopAuto:
        // For auto mode, determine based on current mouse position
        if (d->mousePos.x() >= plotRect.width() - r.width()) {
            r.moveTopLeft(plotRect.topLeft());
        } else {
            r.moveTopRight(plotRect.topRight());
        }
        break;
    case TextOnCanvasBottomAuto:
        // For auto mode, determine based on current mouse position
        if (d->mousePos.x() >= plotRect.width() - r.width()) {
            r.moveBottomLeft(plotRect.bottomLeft());
        } else {
            r.moveBottomRight(plotRect.bottomRight());
        }
        break;
    default:
        // For unspecified textArea, keep rect unchanged
        break;
    }

    return r;
}


QRect QwtPlotSeriesDataPicker::ensureRectInBounds(const QRect& rect, const QRect& bounds) const
{
    QRect constrainedRect = rect;

    // Horizontal constraint
    if (constrainedRect.width() > bounds.width()) {
        // If the rectangle is wider than the plot area, align left
        constrainedRect.moveLeft(bounds.left());
    } else {
        if (constrainedRect.left() < bounds.left()) {
            constrainedRect.moveLeft(bounds.left());
        }
        if (constrainedRect.right() > bounds.right()) {
            constrainedRect.moveRight(bounds.right());
        }
    }

    // Vertical constraint
    if (constrainedRect.height() > bounds.height()) {
        // If the rectangle is taller than the plot area, align top
        constrainedRect.moveTop(bounds.top());
    } else {
        if (constrainedRect.top() < bounds.top()) {
            constrainedRect.moveTop(bounds.top());
        }
        if (constrainedRect.bottom() > bounds.bottom()) {
            constrainedRect.moveBottom(bounds.bottom());
        }
    }

    return constrainedRect;
}

void QwtPlotSeriesDataPicker::drawRubberBand(QPainter* painter) const
{
    // Mainly for PickNearestPoint
    if (!isActive()) {
        return;
    }
    if (!painter || !painter->isActive()) {
        return;
    }

    QPen rbPen              = rubberBandPen();
    const QPoint mousePoint = trackerPosition();
    if (mousePoint.isNull() || mousePoint.x() < 0 || mousePoint.y() < 0) {
        return;
    }
    switch (pickMode()) {
    case PickYValue: {
        painter->save();
        painter->setPen(rbPen);
        const QRect pRect = pickArea().boundingRect().toRect();
        QwtPainter::drawLine(painter, mousePoint.x(), pRect.top(), mousePoint.x(), pRect.bottom());
        painter->restore();
    } break;
    case PickNearestPoint: {
        if (m_data->featurePoints.isEmpty()) {
            return;
        }
        const QwtPlotSeriesDataPicker::FeaturePoint& fp = m_data->featurePoints.first();
        QwtPlot* itemPlot                               = fp.item->plot();
        if (!itemPlot) {
            return;
        }
        const QwtScaleMap xMap = itemPlot->canvasMap(fp.item->xAxis());
        const QwtScaleMap yMap = itemPlot->canvasMap(fp.item->yAxis());
        // Transform point to screen coordinates
        QPointF screenPos = QwtScaleMap::transform(xMap, yMap, fp.feature);
        QColor itemColor  = Qwt::plotItemColor(fp.item, Qt::black);
        rbPen.setColor(itemColor);
        painter->save();
        painter->setPen(rbPen);
        QwtPainter::drawLine(painter, mousePoint, screenPos);
        painter->restore();
    } break;
    default:
        break;
    }

    if (isEnableDrawFeaturePoint()) {
        drawAllFeaturePoints(painter);
    }
}

void QwtPlotSeriesDataPicker::setTrackerPosition(const QPoint& pos)
{
    updateFeaturePoint(pos);
    QwtPicker::setTrackerPosition(pos);
}

/**
 * @brief Get all pickable Y values at a specified screen position
 *
 * This method scans all pickable curves at a given screen coordinate position
 * and collects their corresponding data points. It supports both host and
 * parasitic plots, traversing through all related plot items regardless of
 * their hosting relationship.
 *
 * The function returns the total count of feature points successfully picked.
 * For each picked point, the internal data structures are updated with the
 * curve information and calculated Y values.
 *
 * @param plot The plot widget (can be either host or parasitic plot)
 * @param pos Screen position in widget coordinates
 * @param interpolate Whether to perform interpolation between data points.
 *                   When true, linear interpolation is applied if the position
 *                   falls between two data points; otherwise, the nearest
 *                   data point is selected.
 * @return The number of feature points successfully picked and stored.
 *         Returns 0 if no pickable items are found at the position.
 *
 * @note This function considers both host and parasitic plots. When called
 *       with a host plot, it includes all its parasitic plots; when called
 *       with a parasitic plot, it includes its host and all siblings.
 * @see featurePointList()
 * @see drawFeaturePoints()
 * @see clearFeaturePoints()
 */
int QwtPlotSeriesDataPicker::pickYValue(const QwtPlot* p, const QPoint& pos, bool interpolate)
{
    if (!p) {
        return 0;
    }
    QWT_D(d);
    QList< QwtPlotSeriesDataPicker::FeaturePoint >& featurePoints = d->featurePoints;
    featurePoints.clear();
#if QwtPlotSeriesDataPicker_XGroup
    d->xGroups.clear();
#endif
    const QList< QwtPlot* > plotList = p->plotList();

    // Collect all curves
    QList< QwtPlotCurve* > allCurves;
    for (QwtPlot* oneplot : plotList) {
        const QwtPlotItemList& items = oneplot->itemList();
        for (QwtPlotItem* item : items) {
            if (item->rtti() == QwtPlotItem::Rtti_PlotCurve) {
                QwtPlotCurve* curve = static_cast< QwtPlotCurve* >(item);
                if (curve->isVisible() && curve->dataSize() > 0) {
                    allCurves.append(curve);
                }
            }
        }
    }
#if QwtPlotSeriesDataPicker_XGroup
    if (pickMode() == PickYValue && isEnableShowXValue()) {
        // Group by X axis (curves in the same plot with the same X axis form a group)
        QHash< PrivateData::GroupKey, QList< QwtPlotCurve* > > curvesByAxis;

        for (QwtPlotCurve* curve : allCurves) {
            QwtPlot* p = curve->plot();
            PrivateData::GroupKey key { p, curve->xAxis() };
            curvesByAxis[ key ].append(curve);
        }

        // Process each X axis group
        for (auto it = curvesByAxis.begin(); it != curvesByAxis.end(); ++it) {
            const PrivateData::GroupKey& key     = it.key();
            const QList< QwtPlotCurve* >& curves = it.value();

            if (curves.isEmpty()) {
                continue;
            }

            // Create group
            PrivateData::XGroup group;
            group.key = key;

            // Get the plot for this X axis
            QwtPlot* plotForAxis = key.plot;
            if (!plotForAxis) {
                continue;
            }

            // Get the X axis map
            const QwtScaleMap xMap = plotForAxis->canvasMap(key.axis);

            // Compute the X value corresponding to the mouse position (shared X for all curves in this group)
            double mouseXValue = xMap.invTransform(pos.x());

            // Format X value as common X for this group
            group.xValue = formatAxisValue(mouseXValue, key.axis, plotForAxis);

            // Process all curves in this group
            for (QwtPlotCurve* curve : curves) {
                const size_t curveSize = curve->dataSize();
                if (curveSize == 0) {
                    continue;
                }

                // Get the curve bounding rectangle
                const QRectF br = curve->boundingRect();
                // Bounds check: is mouse X value within curve X range
                if (mouseXValue < br.left() || mouseXValue > br.right()) {
                    continue;
                }

                // Find the corresponding point in curve data
                size_t index =
                    qwtUpperSampleIndex< QPointF >(*curve->data(), mouseXValue, [](const double x, const QPointF& pos) -> bool {
                        return (x < pos.x());
                    });

                if (index == curveSize) {
                    continue;
                }

                // Create feature point
                FeaturePoint fp;
                fp.item  = curve;
                fp.index = index;

                if (interpolate && curveSize > 2 && index > 0) {
                    // Interpolation computation
                    const QPointF& p2 = curve->sample(index);
                    const QPointF& p1 = curve->sample(index - 1);
                    if (qFuzzyCompare(p1.x(), p2.x())) {
                        fp.feature = p2;
                    } else {
                        double t = (mouseXValue - p1.x()) / (p2.x() - p1.x());
                        QPointF interPoint;
                        interPoint.setX(mouseXValue);  // Use unified X value
                        interPoint.setY(p1.y() + t * (p2.y() - p1.y()));
                        fp.feature = interPoint;
                    }
                } else {
                    QPointF point = curve->sample(index);
                    // Use unified X value instead of the actual X from curve
                    fp.feature = QPointF(mouseXValue, point.y());
                }

                featurePoints.append(fp);
                group.fps.append(&featurePoints.last());
            }

            // If the group has feature points, add to group list
            if (!group.fps.isEmpty()) {
                d->xGroups.append(group);
            }
        }
    } else {
        // Not grouping X values, or not in PickYValue mode
        for (QwtPlotCurve* curve : allCurves) {
            const size_t curveSize = curve->dataSize();
            if (curveSize == 0)
                continue;

            QwtPlot* oneplot = curve->plot();

            // Get curve axis maps
            const QwtScaleMap xMap = oneplot->canvasMap(curve->xAxis());

            // Convert screen coordinates to curve coordinate space
            double x = xMap.invTransform(pos.x());

            // Pre-compute and cache bounding rectangle
            const QRectF br = curve->boundingRect();

            // Fast bounds check
            if (x < br.left() || x > br.right()) {
                continue;
            }

            size_t index = qwtUpperSampleIndex< QPointF >(*curve->data(), x, [](const double x, const QPointF& pos) -> bool {
                return (x < pos.x());
            });

            if (index == curveSize) {
                continue;
            }

            FeaturePoint fp;
            fp.item  = curve;
            fp.index = index;

            if (interpolate && curveSize > 2 && index > 0) {
                // Interpolation computation
                const QPointF& p2 = curve->sample(index);
                const QPointF& p1 = curve->sample(index - 1);
                if (qFuzzyCompare(p1.x(), p2.x())) {
                    fp.feature = p2;
                } else {
                    double t = (x - p1.x()) / (p2.x() - p1.x());
                    QPointF interPoint;
                    interPoint.setX(x);
                    interPoint.setY(p1.y() + t * (p2.y() - p1.y()));
                    fp.feature = interPoint;
                }
            } else {
                fp.feature = curve->sample(index);
            }

            featurePoints.append(fp);
        }
    }
#else
    for (QwtPlotCurve* curve : allCurves) {
        const size_t curveSize = curve->dataSize();
        if (curveSize == 0) {
            continue;
        }

        QwtPlot* oneplot = curve->plot();

        // Get curve axis maps
        const QwtScaleMap xMap = oneplot->canvasMap(curve->xAxis());

        // Convert screen coordinates to curve coordinate space
        double x = xMap.invTransform(pos.x());

        // Pre-compute and cache bounding rectangle
        const QRectF br = curve->boundingRect();
        if (x < br.left() || x > br.right()) {
            continue;
        }

        // Fast bounds check
        if (x < br.left() || x > br.right()) {
            continue;
        }

        size_t index = qwtUpperSampleIndex< QPointF >(*curve->data(), x, [](const double x, const QPointF& pos) -> bool {
            return (x < pos.x());
        });

        if (index == curveSize) {
            continue;
        }

        FeaturePoint fp;
        fp.item  = curve;
        fp.index = index;

        if (interpolate && curveSize > 2 && index > 0) {
            // Interpolation computation
            const QPointF& p2 = curve->sample(index);
            const QPointF& p1 = curve->sample(index - 1);
            if (qFuzzyCompare(p1.x(), p2.x())) {
                fp.feature = p2;
            } else {
                double t = (x - p1.x()) / (p2.x() - p1.x());
                QPointF interPoint;
                interPoint.setX(x);
                interPoint.setY(p1.y() + t * (p2.y() - p1.y()));
                fp.feature = interPoint;
            }
        } else {
            fp.feature = curve->sample(index);
        }

        featurePoints.append(fp);
    }
#endif
    return featurePoints.size();
}

/**
 * @brief Get the nearest pickable point at the specified screen position
 * @param plot Plot widget
 * @param pos Screen position
 * @param windowSize Window size
 *        - 0: No window, search entire curve
 *        - Positive: Fixed window size (number of data points)
 *        - Negative: Adaptive window, uses percentage of total curve points (absolute value, e.g. -5 means 5%, -10 means 10%)
 * @return Pair containing the nearest plot item and corresponding data point
 *
 * @note This function accounts for parasitic plots; whether passing a host or parasitic plot,
 *       it retrieves data from all related plots.
 */
int QwtPlotSeriesDataPicker::pickNearestPoint(const QwtPlot* plot, const QPoint& pos, int windowSize)
{
    if (!plot) {
        return 0;
    }
    QWT_D(d);
    QList< QwtPlotSeriesDataPicker::FeaturePoint >& featurePoints = d->featurePoints;
    featurePoints.clear();
#if QwtPlotSeriesDataPicker_XGroup
    d->xGroups.clear();
#endif
    QwtPlotSeriesDataPicker::FeaturePoint fp;

    double minScreenDistance         = std::numeric_limits< double >::max();
    const QList< QwtPlot* > plotList = plot->plotList();
    for (QwtPlot* oneplot : plotList) {
        const auto items = oneplot->itemList(QwtPlotItem::Rtti_PlotCurve);
        for (auto* item : items) {
            auto* curve            = static_cast< QwtPlotCurve* >(item);
            const size_t curveSize = static_cast< int >(curve->dataSize());
            if (!curve->isVisible() || curveSize == 0) {
                continue;
            }
            const auto* series = curve->data();

            /* Binary search for the segment closest to x, then compare distance squared */
            const QwtScaleMap xMap = oneplot->canvasMap(curve->xAxis());
            const QwtScaleMap yMap = oneplot->canvasMap(curve->yAxis());

            // Calculate search window
            double targetX      = xMap.invTransform(pos.x());
            auto searchWinIndex = calculateSearchWindow(curveSize, targetX, *series, windowSize);
            size_t startIndex   = searchWinIndex.first;
            size_t endIndex     = searchWinIndex.second;

            // Search for the nearest point within the calculated window
            double minDistance = std::numeric_limits< double >::max();
            QPointF candidateNearestPoint;
            size_t candidateIndex = startIndex;

            for (size_t i = startIndex; i <= endIndex; ++i) {
                QPointF point         = curve->sample(i);
                int screenX           = qRound(xMap.transform(point.x()));
                int screenY           = qRound(yMap.transform(point.y()));
                double dx             = double(screenX) - pos.x();
                double dy             = double(screenY) - pos.y();
                double screenDistance = dx * dx + dy * dy;

                if (screenDistance < minDistance) {
                    minDistance           = screenDistance;
                    candidateNearestPoint = point;
                    candidateIndex        = i;
                }
            }

            if (minDistance < minScreenDistance) {
                minScreenDistance = minDistance;
                fp.item           = item;
                fp.feature        = candidateNearestPoint;
                fp.index          = candidateIndex;
            }
        }
    }
    if (minScreenDistance < std::numeric_limits< double >::max()) {
        featurePoints.append(fp);
        return 1;
    }
    return featurePoints.size();
}

void QwtPlotSeriesDataPicker::onPlotItemDetached(QwtPlotItem* item, bool on)
{
    // Traverse to check if this item is present
    QWT_D(d);
    if (!on) {
        QList< QwtPlotSeriesDataPicker::FeaturePoint >& pickedFeatureDatas = d->featurePoints;
        for (int i = pickedFeatureDatas.size() - 1; i >= 0; --i) {
            const QwtPlotSeriesDataPicker::FeaturePoint& fp = pickedFeatureDatas[ i ];
            if (fp.item == item) {
#if QwtPlotSeriesDataPicker_XGroup
                // Synchronize removal from xGroups
                for (int j = d->xGroups.size() - 1; j >= 0; --j) {
                    PrivateData::XGroup& g = d->xGroups[ j ];
                    for (int k = g.fps.size() - 1; k >= 0; --k) {
                        if (g.fps[ k ] == &fp) {
                            g.fps.removeAt(k);
                        }
                    }
                }
#endif
                pickedFeatureDatas.removeAt(i);  // Reverse removal to avoid index confusion
            }
        }
    }
}

void QwtPlotSeriesDataPicker::onParasitePlotAttached(QwtPlot* parasiteplot, bool on)
{
    QWT_D(d);
    if (on) {
        // New parasite axis added, need to bind parasite axis onPlotItemDetached
        connect(parasiteplot, &QwtPlot::itemAttached, this, &QwtPlotSeriesDataPicker::onPlotItemDetached);
    } else {
        disconnect(parasiteplot, nullptr, this, nullptr);
        // clear featurePoints make it invalid
        d->featurePoints.clear();
#if QwtPlotSeriesDataPicker_XGroup
        // Synchronously remove existing info
        QWT_D(d);
        for (int i = d->xGroups.size() - 1; i >= 0; --i) {
            PrivateData::XGroup& g = d->xGroups[ i ];
            if (g.key.plot == parasiteplot) {
                d->xGroups.removeAt(i);
            }
        }
#endif
    }
}
