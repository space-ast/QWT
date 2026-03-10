/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QwtAbstractLegend>

class LegendTreeView;
class QStandardItem;
class QModelIndex;
class QwtPlotItem;

class Legend : public QwtAbstractLegend
{
    Q_OBJECT

public:
    explicit Legend(QWidget* parent = NULL);
    virtual ~Legend();

    virtual void renderLegend(QPainter*, const QRectF&, bool fillBackground) const override;

    virtual bool isEmpty() const override;

    virtual int scrollExtent(Qt::Orientation) const override;

Q_SIGNALS:
    void checked(QwtPlotItem* plotItem, bool on, int index);

public Q_SLOTS:
    virtual void updateLegend(const QVariant&, const QList< QwtLegendData >&) override;

private Q_SLOTS:
    void handleClick(const QModelIndex&);

private:
    void updateItem(QStandardItem*, const QwtLegendData&);

    LegendTreeView* m_treeView;
};
