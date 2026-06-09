#include "qwt_stylesheet_recorder.h"
#include "qwt_math.h"

/**
 * @brief Construct a stylesheet recorder with specified size
 *
 * @param[in] size Size of the recording area
 *
 */
QwtStyleSheetRecorder::QwtStyleSheetRecorder(const QSize& size) : QwtNullPaintDevice(), m_size(size)
{
}

/**
 * @brief Update the paint engine state
 *
 * Records changes to pen, brush, and brush origin from the paint engine state.
 *
 * @param[in] state Paint engine state containing changed attributes
 *
 */
void QwtStyleSheetRecorder::updateState(const QPaintEngineState& state)
{
    if (state.state() & QPaintEngine::DirtyPen) {
        m_pen = state.pen();
    }
    if (state.state() & QPaintEngine::DirtyBrush) {
        m_brush = state.brush();
    }
    if (state.state() & QPaintEngine::DirtyBrushOrigin) {
        m_origin = state.brushOrigin();
    }
}

/**
 * @brief Draw rectangles with floating-point coordinates
 *
 * Records rectangles to the border rect list for styled border rendering.
 *
 * @param[in] rects Array of rectangles to draw
 * @param[in] count Number of rectangles
 *
 */
void QwtStyleSheetRecorder::drawRects(const QRectF* rects, int count)
{
    for (int i = 0; i < count; i++)
border.rectList += rects[ i ];
}

/**
 * @brief Draw rectangles with integer coordinates
 *
 * Records rectangles to the border rect list for styled border rendering.
 *
 * @param[in] rects Array of rectangles to draw
 * @param[in] count Number of rectangles
 *
 */
void QwtStyleSheetRecorder::drawRects(const QRect* rects, int count)
{
    for (int i = 0; i < count; i++) {
        this->border.rectList += rects[ i ];
    }
}

/**
 * @brief Draw a painter path
 *
 * Analyzes the path to determine whether it represents a background fill
 * or border element. Paths centered in the rect are treated as backgrounds;
 * others are added to border path list.
 *
 * @param[in] path QPainterPath to analyze and record
 *
 */
void QwtStyleSheetRecorder::drawPath(const QPainterPath& path)
{
    const QRectF rect(QPointF(0.0, 0.0), m_size);
    if (path.controlPointRect().contains(rect.center())) {
        setCornerRects(path);
        alignCornerRects(rect);

        background.path   = path;
        background.brush  = m_brush;
        background.origin = m_origin;
    } else {
        border.pathList += path;
    }
}

/**
 * @brief Set corner rectangles from a path
 *
 * Extracts corner rectangle regions from a path's curve elements.
 * These rectangles represent areas where clipping may be needed
 * for rounded corners.
 *
 * @param[in] path QPainterPath containing curve elements
 *
 */
void QwtStyleSheetRecorder::setCornerRects(const QPainterPath& path)
{
    QPointF pos(0.0, 0.0);

    for (int i = 0; i < path.elementCount(); i++) {
        QPainterPath::Element el = path.elementAt(i);
        switch (el.type) {
        case QPainterPath::MoveToElement:
        case QPainterPath::LineToElement: {
            pos.setX(el.x);
            pos.setY(el.y);
            break;
        }
        case QPainterPath::CurveToElement: {
            QRectF r(pos, QPointF(el.x, el.y));
            clipRects += r.normalized();

            pos.setX(el.x);
            pos.setY(el.y);

            break;
        }
        case QPainterPath::CurveToDataElement: {
            if (clipRects.size() > 0) {
                QRectF r = clipRects.last();
                r.setCoords(qwtMinF(r.left(), el.x),
                            qwtMinF(r.top(), el.y),
                            qwtMaxF(r.right(), el.x),
                            qwtMaxF(r.bottom(), el.y));
                clipRects.last() = r.normalized();
            }
            break;
        }
        }
    }
}

/**
 * @brief Get the size metrics of the device
 *
 * @return The configured size of this recorder
 *
 */
QSize QwtStyleSheetRecorder::sizeMetrics() const
{
    return m_size;
}

void QwtStyleSheetRecorder::alignCornerRects(const QRectF& rect)
{
    for (int i = 0; i < clipRects.size(); i++) {
        QRectF& r = clipRects[ i ];
        if (r.center().x() < rect.center().x()) {
            r.setLeft(rect.left());
        } else {
            r.setRight(rect.right());
        }

        if (r.center().y() < rect.center().y()) {
            r.setTop(rect.top());
        } else {
            r.setBottom(rect.bottom());
        }
    }
}
