#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning(disable : 4305)
#endif

#include "qwt3d_colorlegend.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Constructs a legend object with an axis at the left side
 * @details The legend resides in the top-right area and has no caption.
 *          Scale numbering is shown.
 * \endif
 *
 * \if CHINESE
 * @brief 构造左侧带有坐标轴的图例对象
 * @details 图例位于右上区域，没有标题。显示刻度编号。
 * \endif
 */
ColorLegend::ColorLegend()
{
    axis_.setNumbers(true);
    axis_.setScaling(true);
    axis_.setNumberColor(RGBA(0, 0, 0, 1));
    axis_.setNumberAnchor(CenterRight);
    axis_.setNumberFont(QFont("Courier", 8));

    caption_.setFont("Courier", 10, QFont::Bold);
    caption_.setColor(RGBA(0, 0, 0, 1));
    axisposition_ = ColorLegend::Left;
    orientation_ = ColorLegend::BottomTop;
    showaxis_ = true;
    setRelPosition(Tuple(0.94, 1 - 0.36), Tuple(0.97, 1 - 0.04));
}

/**
 * \if ENGLISH
 * @brief Sets the legend title string
 * @param[in] s Title text string
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例标题字符串
 * @param[in] s 标题文本字符串
 * \endif
 */
void ColorLegend::setTitleString(QString const &s)
{
    caption_.setString(s);
}

/**
 * \if ENGLISH
 * @brief Sets the legend title font
 * @param[in] family Font family name
 * @param[in] pointSize Font point size
 * @param[in] weight Font weight
 * @param[in] italic Whether font is italic
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例标题字体
 * @param[in] family 字体族名
 * @param[in] pointSize 字体磅值大小
 * @param[in] weight 字体粗细
 * @param[in] italic 是否斜体
 * \endif
 */
void ColorLegend::setTitleFont(QString const &family, int pointSize, int weight, bool italic)
{
    caption_.setFont(family, pointSize, weight, italic);
}

/**
 * \if ENGLISH
 * @brief Sets axis scale limits
 * @param[in] start Start value
 * @param[in] stop Stop value
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴刻度范围
 * @param[in] start 起始值
 * @param[in] stop 结束值
 * \endif
 */
void ColorLegend::setLimits(double start, double stop)
{
    axis_.setLimits(start, stop);
}

/**
 * \if ENGLISH
 * @brief Sets number of major intervals
 * @param[in] majors Number of major intervals
 * \endif
 *
 * \if CHINESE
 * @brief 设置主刻度区间数
 * @param[in] majors 主刻度区间数
 * \endif
 */
void ColorLegend::setMajors(int majors)
{
    axis_.setMajors(majors);
}

/**
 * \if ENGLISH
 * @brief Sets number of minor intervals
 * @param[in] minors Number of minor intervals
 * \endif
 *
 * \if CHINESE
 * @brief 设置次刻度区间数
 * @param[in] minors 次刻度区间数
 * \endif
 */
void ColorLegend::setMinors(int minors)
{
    axis_.setMinors(minors);
}

/**
 * \if ENGLISH
 * @brief Enables or disables auto-scaling
 * @param[in] val True to enable auto-scaling, false to disable
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用自动缩放
 * @param[in] val true 启用自动缩放，false 禁用
 * \endif
 */
void ColorLegend::setAutoScale(bool val)
{
    axis_.setAutoScale(val);
}

/**
 * \if ENGLISH
 * @brief Sets predefined scale type
 * @param[in] val Scale type (LINEARSCALE or LOG10SCALE)
 * \endif
 *
 * \if CHINESE
 * @brief 设置预定义刻度类型
 * @param[in] val 刻度类型（LINEARSCALE 或 LOG10SCALE）
 * \endif
 */
void ColorLegend::setScale(SCALETYPE val)
{
    axis_.setScale(val);
}

/**
 * \if ENGLISH
 * @brief Sets a user-defined scale object
 * @param[in] val Pointer to a Scale object
 * \endif
 *
 * \if CHINESE
 * @brief 设置用户自定义刻度对象
 * @param[in] val Scale 对象的指针
 * \endif
 */
void ColorLegend::setScale(Scale *val)
{
    axis_.setScale(val);
}

/**
 * \if ENGLISH
 * @brief Sets the legend orientation and axis scale position
 * @param[in] orientation Legend orientation (BottomTop or TopBottom)
 * @param[in] pos Axis scale position (Left, Right, Top, or Bottom)
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例方向和坐标轴刻度位置
 * @param[in] orientation 图例方向（BottomTop 或 TopBottom）
 * @param[in] pos 坐标轴刻度位置（Left、Right、Top 或 Bottom）
 * \endif
 */
void ColorLegend::setOrientation(ORIENTATION orientation, SCALEPOSITION pos)
{
    orientation_ = orientation;
    axisposition_ = pos;

    if (orientation_ == BottomTop) {
        if (axisposition_ == Bottom || axisposition_ == Top)
            axisposition_ = Left;
    } else {
        if (axisposition_ == Left || axisposition_ == Right)
            axisposition_ = Bottom;
    }
}

/**
 * \if ENGLISH
 * @brief Sets relative position of the legend within the plot area
 * @param[in] relMin Minimum relative position (x,y)
 * @param[in] relMax Maximum relative position (x,y)
 * \endif
 *
 * \if CHINESE
 * @brief 设置图例在绘图区域内的相对位置
 * @param[in] relMin 最小相对位置（x,y）
 * @param[in] relMax 最大相对位置（x,y）
 * \endif
 */
void ColorLegend::setRelPosition(Tuple relMin, Tuple relMax)
{
    relMin_ = relMin;
    relMax_ = relMax;
}

void ColorLegend::setGeometryInternal()
{
    double ot = .99;

    getMatrices(modelMatrix, projMatrix, viewport);
    pe_.minVertex = relativePosition(Triple(relMin_.x, relMin_.y, ot));
    pe_.maxVertex = relativePosition(Triple(relMax_.x, relMax_.y, ot));

    double diff = 0;
    Triple b;
    Triple e;

    switch (axisposition_) {
    case ColorLegend::Left:
        b = pe_.minVertex;
        e = pe_.maxVertex;
        e.x = b.x;
        axis_.setTicOrientation(-1, 0, 0);
        axis_.setNumberAnchor(CenterRight);
        diff = pe_.maxVertex.x - pe_.minVertex.x;
        break;
    case ColorLegend::Right:
        e = pe_.maxVertex;
        b = pe_.minVertex;
        b.x = e.x;
        axis_.setTicOrientation(+1, 0, 0);
        axis_.setNumberAnchor(CenterLeft);
        diff = pe_.maxVertex.x - pe_.minVertex.x;
        break;
    case ColorLegend::Top:
        e = pe_.maxVertex;
        b = pe_.minVertex;
        b.z = e.z;
        axis_.setTicOrientation(0, 0, +1);
        axis_.setNumberAnchor(BottomCenter);
        diff = pe_.maxVertex.z - pe_.minVertex.z;
        break;
    case ColorLegend::Bottom:
        b = pe_.minVertex;
        e = pe_.maxVertex;
        e.z = b.z;
        axis_.setTicOrientation(0, 0, -1);
        axis_.setNumberAnchor(TopCenter);
        diff = pe_.maxVertex.z - pe_.minVertex.z;
        break;
    default:
        break;
    }

    axis_.setPosition(b, e);
    diff /= 10;

    axis_.setTicLength(diff, 0.6 * diff);

    Triple c;
    c.x = pe_.minVertex.x + ((pe_.maxVertex - pe_.minVertex) / 2).x;
    c.z = pe_.maxVertex.z;
    c.z += (pe_.maxVertex.z - pe_.minVertex.z) / 20;
    c.y = pe_.maxVertex.y;

    caption_.setPosition(c, BottomCenter);
}

/**
 * \if ENGLISH
 * @brief Draws the color legend
 * @details Renders the color legend including color bar, axis, and caption.
 * \endif
 *
 * \if CHINESE
 * @brief 绘制颜色图例
 * @details 渲染颜色图例，包括颜色条、坐标轴和标题。
 * \endif
 */
void ColorLegend::draw()
{
    if (colors.empty())
        return;

    setGeometryInternal();

    saveGLState();

    Triple one = pe_.minVertex;
    Triple two = pe_.maxVertex;

    double h = (orientation_ == ColorLegend::BottomTop) ? (two - one).z / colors.size()
                                                        : (two - one).x / colors.size();

    // glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLStateBewarer(GL_POLYGON_OFFSET_FILL, true);
    //	glPolygonOffset(0.0, 0.0);

    glColor4d(0, 0, 0, 1);
    glBegin(GL_LINE_LOOP);
    glVertex3d(one.x, one.y, one.z);
    glVertex3d(one.x, one.y, two.z);
    glVertex3d(two.x, one.y, two.z);
    glVertex3d(two.x, one.y, one.z);
    glEnd();

    size_t size = colors.size();
    RGBA rgb;

    if (orientation_ == ColorLegend::BottomTop) {
        for (unsigned i = 1; i <= size; ++i) {
            rgb = colors[i - 1];
            glColor4d(rgb.r, rgb.g, rgb.b, rgb.a);
            glBegin(GL_POLYGON);
            glVertex3d(one.x, one.y, one.z + (i - 1) * h);
            glVertex3d(one.x, one.y, one.z + i * h);
            glVertex3d(two.x, one.y, one.z + i * h);
            glVertex3d(two.x, one.y, one.z + (i - 1) * h);
            glEnd();
        }
    } else {
        for (unsigned i = 1; i <= size; ++i) {
            rgb = colors[i - 1];
            glColor4d(rgb.r, rgb.g, rgb.b, rgb.a);
            glBegin(GL_POLYGON);
            glVertex3d(one.x + (i - 1) * h, one.y, one.z);
            glVertex3d(one.x + i * h, one.y, one.z);
            glVertex3d(one.x + i * h, one.y, two.z);
            glVertex3d(one.x + (i - 1) * h, one.y, two.z);
            glEnd();
        }
    }

    restoreGLState();

    if (showaxis_)
        axis_.draw();

    caption_.draw();
}