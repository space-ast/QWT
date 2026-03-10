#ifndef QWTSTYLESHEETRECORDER_H
#define QWTSTYLESHEETRECORDER_H
#include "qwt_null_paintdevice.h"

#include <QList>
#include <QVector>
#include <QRectF>
#include <QRectF>
#include <QPainterPath>
#include <QBrush>
#include <QPointF>

class QWT_EXPORT QwtStyleSheetRecorder final : public QwtNullPaintDevice
{
public:
    explicit QwtStyleSheetRecorder(const QSize& size);
    virtual void updateState(const QPaintEngineState& state) override;
    virtual void drawRects(const QRectF* rects, int count) override;
    virtual void drawRects(const QRect* rects, int count) override;
    virtual void drawPath(const QPainterPath& path) override;
    void setCornerRects(const QPainterPath& path);

protected:
    virtual QSize sizeMetrics() const override;

private:
    void alignCornerRects(const QRectF& rect);

public:
    QVector< QRectF > clipRects;

    struct Border
    {
        QList< QPainterPath > pathList;
        QList< QRectF > rectList;
        QRegion clipRegion;
    } border;

    struct Background
    {
        QPainterPath path;
        QBrush brush;
        QPointF origin;
    } background;

private:
    const QSize m_size;

    QPen m_pen;
    QBrush m_brush;
    QPointF m_origin;
};

#endif  // QWTSTYLESHEETRECORDER_H
