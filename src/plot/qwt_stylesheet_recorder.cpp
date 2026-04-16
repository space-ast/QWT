#include "qwt_stylesheet_recorder.h"
#include "qwt_math.h"

/**
 * \if ENGLISH
 * @brief Construct a stylesheet recorder with specified size
 *
 * @param[in] size Size of the recording area
 * \endif
 *
 * \if CHINESE
 * @brief 构造指定大小的样式表记录器
 *
 * @param[in] size 记录区域的大小
 * \endif
 */
QwtStyleSheetRecorder::QwtStyleSheetRecorder(const QSize& size) : QwtNullPaintDevice(), m_size(size)
{
}

/**
 * \if ENGLISH
 * @brief Update the paint engine state
 *
 * Records changes to pen, brush, and brush origin from the paint engine state.
 *
 * @param[in] state Paint engine state containing changed attributes
 * \endif
 *
 * \if CHINESE
 * @brief 更新绘制引擎状态
 *
 * 从绘制引擎状态记录画笔、画刷和画刷原点的变化。
 *
 * @param[in] state 包含变化属性的绘制引擎状态
 * \endif
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
 * \if ENGLISH
 * @brief Draw rectangles with floating-point coordinates
 *
 * Records rectangles to the border rect list for styled border rendering.
 *
 * @param[in] rects Array of rectangles to draw
 * @param[in] count Number of rectangles
 * \endif
 *
 * \if CHINESE
 * @brief 绘制浮点坐标矩形
 *
 * 将矩形记录到边框矩形列表中，用于样式化边框渲染。
 *
 * @param[in] rects 矩形数组
 * @param[in] count 矩形数量
 * \endif
 */
void QwtStyleSheetRecorder::drawRects(const QRectF* rects, int count)
{
    for (int i = 0; i < count; i++)
border.rectList += rects[ i ];
}

/**
 * \if ENGLISH
 * @brief Draw rectangles with integer coordinates
 *
 * Records rectangles to the border rect list for styled border rendering.
 *
 * @param[in] rects Array of rectangles to draw
 * @param[in] count Number of rectangles
 * \endif
 *
 * \if CHINESE
 * @brief 绘制整数坐标矩形
 *
 * 将矩形记录到边框矩形列表中，用于样式化边框渲染。
 *
 * @param[in] rects 矩形数组
 * @param[in] count 矩形数量
 * \endif
 */
void QwtStyleSheetRecorder::drawRects(const QRect* rects, int count)
{
    for (int i = 0; i < count; i++) {
        this->border.rectList += rects[ i ];
    }
}

/**
 * \if ENGLISH
 * @brief Draw a painter path
 *
 * Analyzes the path to determine whether it represents a background fill
 * or border element. Paths centered in the rect are treated as backgrounds;
 * others are added to border path list.
 *
 * @param[in] path QPainterPath to analyze and record
 * \endif
 *
 * \if CHINESE
 * @brief 绘制画家路径
 *
 * 分析路径以确定其是背景填充还是边框元素。
 * 位于矩形中心的路径被视为背景；其他路径添加到边框路径列表。
 *
 * @param[in] path 要分析和记录的QPainterPath
 * \endif
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
 * \if ENGLISH
 * @brief Set corner rectangles from a path
 *
 * Extracts corner rectangle regions from a path's curve elements.
 * These rectangles represent areas where clipping may be needed
 * for rounded corners.
 *
 * @param[in] path QPainterPath containing curve elements
 * \endif
 *
 * \if CHINESE
 * @brief 从路径设置角落矩形
 *
 * 从路径的曲线元素中提取角落矩形区域。
 * 这些矩形表示可能需要裁剪的圆角区域。
 *
 * @param[in] path 包含曲线元素的QPainterPath
 * \endif
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
 * \if ENGLISH
 * @brief Get the size metrics of the device
 *
 * @return The configured size of this recorder
 * \endif
 *
 * \if CHINESE
 * @brief 获取设备的尺寸度量
 *
 * @return 此记录器的配置大小
 * \endif
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
