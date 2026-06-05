#include "qwt_plot_series_data_picker_group.h"
#include "qwt_plot_series_data_picker.h"
#include <QWidget>
#include <QPointer>

class QwtPlotSeriesDataPickerGroup::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtPlotSeriesDataPickerGroup)
public:
    PrivateData(QwtPlotSeriesDataPickerGroup* p);
    QList< QwtPlotSeriesDataPicker* > pickers;
    bool enable { true };
};

QwtPlotSeriesDataPickerGroup::PrivateData::PrivateData(QwtPlotSeriesDataPickerGroup* p) : q_ptr(p)
{
}

//----------------------------------------------------
// QwtPlotSeriesDataPickerGroup
//----------------------------------------------------
/**
 * @brief Constructor
 * @param[in] par Parent object
 */
QwtPlotSeriesDataPickerGroup::QwtPlotSeriesDataPickerGroup(QObject* par) : QObject(par),QWT_PIMPL_CONSTRUCT
{
}

/**
 * @brief Destructor
 */
QwtPlotSeriesDataPickerGroup::~QwtPlotSeriesDataPickerGroup()
{
}

/**
 * @brief Adds a picker to the synchronization group
 * @param pick Pointer to QwtPlotSeriesDataPicker instance to be added to the group
 * @note The picker must belong to a QwtPlot with properly configured axes. Adding a null
 *       pointer or a picker already in the group has no effect. The group does not take
 *       ownership of the picker.
 * @warning Pickers from plots with significantly different X-axis scales may show positioning
 *          artifacts due to proportional mapping limitations.
 */
void QwtPlotSeriesDataPickerGroup::addPicker(QwtPlotSeriesDataPicker* pick)
{
    if (m_data->pickers.contains(pick)) {
        return;
    }
    m_data->pickers.append(pick);
    connect(pick, &QwtPlotSeriesDataPicker::moved, this, &QwtPlotSeriesDataPickerGroup::onPickerMove);
    connect(pick, &QwtPlotSeriesDataPicker::activated, this, &QwtPlotSeriesDataPickerGroup::onPickerActivated);
    connect(pick, &QwtPlotSeriesDataPicker::destroyed, this, &QwtPlotSeriesDataPickerGroup::onPickerDestroy);
    connect(pick, &QwtPlotSeriesDataPicker::clicked, this, &QwtPlotSeriesDataPickerGroup::onPickerClicked);
    connect(pick, &QwtPlotSeriesDataPicker::doubleClicked, this, &QwtPlotSeriesDataPickerGroup::onPickerDoubleClicked);
}

/**
 * @brief Remove a picker from the group
 * @param[in] pick Pointer to QwtPlotSeriesDataPicker instance to be removed from the group
 * @note If the picker is not in the group, this function has no effect.
 */
void QwtPlotSeriesDataPickerGroup::removePicker(QwtPlotSeriesDataPicker* pick)
{
    m_data->pickers.removeAll(pick);
}

/**
 * @brief Returns the list of all pickers currently in the group
 * @return QList containing pointers to all managed QwtPlotSeriesDataPicker instances
 * @note The returned list is a copy; modifications to it do not affect the internal group state.
 *       Use addPicker() to modify group membership.
 */
QList< QwtPlotSeriesDataPicker* > QwtPlotSeriesDataPickerGroup::pickers() const
{
    return m_data->pickers;
}

/**
 * @brief Set enabled state
 * @details When disabled, picker synchronization will not occur.
 * @param[in] on Enable/disable synchronization
 * @sa isEnabled()
 */
void QwtPlotSeriesDataPickerGroup::setEnabled(bool on)
{
    m_data->enable = on;
}

/**
 * @brief Check if synchronization is enabled
 * @return True if enabled
 * @sa setEnabled()
 */
bool QwtPlotSeriesDataPickerGroup::isEnabled() const
{
    return m_data->enable;
}

void QwtPlotSeriesDataPickerGroup::onPickerMove(const QPoint& pos)
{
    if (!isEnabled()) {
        return;
    }
    QwtPlotSeriesDataPicker* pick = qobject_cast< QwtPlotSeriesDataPicker* >(sender());
    if (!pick) {
        return;
    }
    // Convert to percentages relative to this picker's widget
    QWidget* canvas = pick->canvas();
    if (!canvas) {
        return;
    }
    double xPresent = qBound(0.0, (double)pos.x() / canvas->width(), 1.0);
    double yPresent = qBound(0.0, (double)pos.y() / canvas->height(), 1.0);
    // Have other pickers also set position at this location
    for (QwtPlotSeriesDataPicker* p : qwt_as_const(m_data->pickers)) {
        if (p == pick) {
            continue;
        }
        QWidget* c = p->canvas();
        if (!c) {
            continue;
        }
        double x = c->width() * xPresent;
        double y = c->height() * yPresent;
        p->setTrackerPosition(QPoint(x, y));
        p->update();
    }
}

void QwtPlotSeriesDataPickerGroup::onPickerActivated(bool on)
{
    if (!isEnabled()) {
        return;
    }
    QwtPlotSeriesDataPicker* pick = qobject_cast< QwtPlotSeriesDataPicker* >(sender());
    if (!pick) {
        return;
    }
    for (QwtPlotSeriesDataPicker* p : qwt_as_const(m_data->pickers)) {
        if (p == pick) {
            continue;
        }
        p->setActive(on);
    }
}

void QwtPlotSeriesDataPickerGroup::onPickerDestroy(QObject* obj)
{
    m_data->pickers.removeAll(static_cast< QwtPlotSeriesDataPicker* >(obj));
}

/**
 * @brief Handles click signal from a picker, syncs other pickers, then emits group's clicked signal
 * @param picker The picker that emitted the click signal
 * @param pos The click position
 */
void QwtPlotSeriesDataPickerGroup::onPickerClicked(QwtPlotSeriesDataPicker* picker, const QPoint& pos)
{
    if (!isEnabled()) {
        return;
    }

    QWidget* canvas = picker->canvas();
    if (!canvas) {
        return;
    }

    // Sync all other pickers to the proportional position
    double xPresent = qBound(0.0, (double)pos.x() / canvas->width(), 1.0);
    double yPresent = qBound(0.0, (double)pos.y() / canvas->height(), 1.0);

    for (QwtPlotSeriesDataPicker* p : qwt_as_const(m_data->pickers)) {
        if (p != picker) {
            QWidget* c = p->canvas();
            if (c) {
                double x = c->width() * xPresent;
                double y = c->height() * yPresent;
                p->setTrackerPosition(QPoint(x, y));
                p->update();  // Trigger featurePoint recalculation at synced position
            }
        }
    }

    emit clicked(picker, pos);
}

/**
 * @brief Handles double-click signal from a picker, syncs other pickers, then emits group's doubleClicked signal
 * @param picker The picker that emitted the double-click signal
 * @param pos The double-click position
 */
void QwtPlotSeriesDataPickerGroup::onPickerDoubleClicked(QwtPlotSeriesDataPicker* picker, const QPoint& pos)
{
    if (!isEnabled()) {
        return;
    }

    QWidget* canvas = picker->canvas();
    if (!canvas) {
        return;
    }

    // Sync all other pickers to the proportional position
    double xPresent = qBound(0.0, (double)pos.x() / canvas->width(), 1.0);
    double yPresent = qBound(0.0, (double)pos.y() / canvas->height(), 1.0);

    for (QwtPlotSeriesDataPicker* p : qwt_as_const(m_data->pickers)) {
        if (p != picker) {
            QWidget* c = p->canvas();
            if (c) {
                double x = c->width() * xPresent;
                double y = c->height() * yPresent;
                p->setTrackerPosition(QPoint(x, y));
                p->update();  // Trigger featurePoint recalculation at synced position
            }
        }
    }

    emit doubleClicked(picker, pos);
}
