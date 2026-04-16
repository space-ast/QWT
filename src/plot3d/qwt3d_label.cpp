#include <qbitmap.h>
#include "qwt3d_label.h"

using namespace Qwt3D;

bool Label::devicefonts_ = false;

/**
 * \if ENGLISH
 * @brief Default constructor
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数
 * \endif
 */
Label::Label()
{
    init();
}

/**
 * \if ENGLISH
 * @brief Constructs a Label with specified font parameters
 * @param[in] family Font family name
 * @param[in] pointSize Font point size
 * @param[in] weight Font weight
 * @param[in] italic Whether font is italic
 * \endif
 *
 * \if CHINESE
 * @brief 构造具有指定字体参数的标签
 * @param[in] family 字体族名
 * @param[in] pointSize 字体磅值大小
 * @param[in] weight 字体粗细
 * @param[in] italic 是否斜体
 * \endif
 */
Label::Label(const QString &family, int pointSize, int weight, bool italic)
{
    init(family, pointSize, weight, italic);
}

void Label::init(const QString &family, int pointSize, int weight, bool italic)
{
    init();
    font_ = QFont(family, pointSize, weight, italic);
}

void Label::init()
{
    beg_ = Triple(0.0, 0.0, 0.0);
    end_ = beg_;
    pos_ = beg_;
    setColor(0, 0, 0);
    pm_ = QPixmap(0, 0);
    font_ = QFont();
    anchor_ = BottomLeft;
    gap_ = 0;
    flagforupdate_ = true;
}

/**
 * \if ENGLISH
 * @brief Enables or disables device font rendering for all labels
 * @param[in] val True to use device fonts, false to use Qt-based rendering
 * \endif
 *
 * \if CHINESE
 * @brief 启用或禁用所有标签的设备字体渲染
 * @param[in] val true 使用设备字体，false 使用 Qt 渲染
 * \endif
 */
void Label::useDeviceFonts(bool val)
{
    devicefonts_ = val;
}

/**
 * \if ENGLISH
 * @brief Sets the label font
 * @param[in] family Font family name
 * @param[in] pointSize Font point size
 * @param[in] weight Font weight
 * @param[in] italic Whether font is italic
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签字体
 * @param[in] family 字体族名
 * @param[in] pointSize 字体磅值大小
 * @param[in] weight 字体粗细
 * @param[in] italic 是否斜体
 * \endif
 */
void Label::setFont(const QString &family, int pointSize, int weight, bool italic)
{
    font_ = QFont(family, pointSize, weight, italic);
    flagforupdate_ = true;
}

/**
 * \if ENGLISH
 * @brief Sets the label text string
 * @param[in] s Text string to display
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签文本字符串
 * @param[in] s 要显示的文本字符串
 * \endif
 */
void Label::setString(QString const &s)
{
    text_ = s;
    flagforupdate_ = true;
}

/**
 * \if ENGLISH
 * @brief Sets the label color from RGBA components
 * @param[in] r Red component
 * @param[in] g Green component
 * @param[in] b Blue component
 * @param[in] a Alpha component
 * \endif
 *
 * \if CHINESE
 * @brief 从 RGBA 分量设置标签颜色
 * @param[in] r 红色分量
 * @param[in] g 绿色分量
 * @param[in] b 蓝色分量
 * @param[in] a 透明度分量
 * \endif
 */
void Label::setColor(double r, double g, double b, double a)
{
    Drawable::setColor(r, g, b, a);
    flagforupdate_ = true;
}

/**
 * \if ENGLISH
 * @brief Sets the label color from an RGBA object
 * @param[in] rgba RGBA color value
 * \endif
 *
 * \if CHINESE
 * @brief 从 RGBA 对象设置标签颜色
 * @param[in] rgba RGBA 颜色值
 * \endif
 */
void Label::setColor(Qwt3D::RGBA rgba)
{
    Drawable::setColor(rgba);
    flagforupdate_ = true;
}

/**
 * \if ENGLISH
 * @brief Sets the label position and anchor point
 * @param[in] pos Position triple in world coordinates
 * @param[in] a Anchor type defining how the label aligns relative to pos
 * @details Anchor example:
 *          TopCenter (*) resp. BottomRight (X):
 *          +----*----+
 *          |  Pixmap |
 *          +---------X
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签位置和锚点
 * @param[in] pos 世界坐标中的位置三元组
 * @param[in] a 锚点类型，定义标签相对于 pos 的对齐方式
 * @details 锚点示例：
 *          TopCenter (*) 相应地 BottomRight (X)：
 *          +----*----+
 *          |  Pixmap |
 *          +---------X
 * \endif
 */
void Label::setPosition(Triple pos, ANCHOR a)
{
    anchor_ = a;
    pos_ = pos;
}

/**
 * \if ENGLISH
 * @brief Sets the label position relative to the viewport
 * @param[in] rpos Relative position tuple (x,y)
 * @param[in] a Anchor type defining how the label aligns
 * \endif
 *
 * \if CHINESE
 * @brief 设置标签相对于视口的位置
 * @param[in] rpos 相对位置元组（x,y）
 * @param[in] a 锚点类型，定义标签的对齐方式
 * \endif
 */
void Label::setRelPosition(Tuple rpos, ANCHOR a)
{
    double ot = 0.99;

    getMatrices(modelMatrix, projMatrix, viewport);
    beg_ = relativePosition(Triple(rpos.x, rpos.y, ot));
    setPosition(beg_, a);
}

void Label::update()
{
    QPainter p;
    QFontMetrics fm(font_);

    QFontInfo info(font_);

    QRect r = QRect(
            QPoint(0, 0),
            fm.size(Qwt3D::SingleLine, text_)); // fm.boundingRect(text_)  misbehaviour under linux;

    r.translate(0, -r.top());

    pm_ = QPixmap(r.width(), r.bottom());

    if (pm_.isNull()) // else crash under linux
    {
        r = QRect(QPoint(0, 0),
                  fm.size(Qwt3D::SingleLine, QString(" "))); // draw empty space else //todo
        r.translate(0, -r.top());
        pm_ = QPixmap(r.width(), r.bottom());
    }

    QBitmap bm(pm_.width(), pm_.height());
    bm.fill(Qt::color0);
    p.begin(&bm);
    p.setPen(Qt::color1);
    p.setFont(font_);
    p.drawText(0, r.height() - fm.descent() - 1, text_);
    p.end();

    pm_.setMask(bm);

    p.begin(&pm_);
    p.setFont(font_);
    p.setPen(Qt::SolidLine);
    p.setPen(GL2Qt(color.r, color.g, color.b));

    p.drawText(0, r.height() - fm.descent() - 1, text_);
    p.end();
    buf_ = pm_.toImage();
    tex_ = buf_.mirrored();
}

/**
 * \if ENGLISH
 * @brief Adds an additional shift to the anchor point
 * @param[in] gap Gap value in pixels
 * @details The shift direction depends on the anchor type:
 *          left aligned -->, right aligned <--, top aligned top-down,
 *          bottom aligned bottom-up. The unit is user space dependent
 *          (one pixel on screen - play around to get satisfying results).
 * \endif
 *
 * \if CHINESE
 * @brief 向锚点添加额外偏移
 * @param[in] gap 以像素为单位的间距值
 * @details 偏移方向取决于锚点类型：
 *          左对齐 -->，右对齐 <--，顶部对齐向下，底部对齐向上。
 *          单位依赖于用户空间（屏幕上一个像素 - 调整以获得满意结果）。
 * \endif
 */
void Label::adjust(int gap)
{
    gap_ = gap;
}

void Label::convert2screen()
{
    Triple start = World2ViewPort(pos_);

    switch (anchor_) {
    case BottomLeft:
        beg_ = pos_;
        break;
    case BottomRight:
        beg_ = ViewPort2World(start - Triple(width() + gap_, 0, 0));
        break;
    case BottomCenter:
        beg_ = ViewPort2World(start - Triple(width() / 2, -gap_, 0));
        break;
    case TopRight:
        beg_ = ViewPort2World(start - Triple(width() + gap_, height(), 0));
        break;
    case TopLeft:
        beg_ = ViewPort2World(start - Triple(-gap_, height(), 0));
        break;
    case TopCenter:
        beg_ = ViewPort2World(start - Triple(width() / 2, height() + gap_, 0));
        break;
    case CenterLeft:
        beg_ = ViewPort2World(start - Triple(-gap_, height() / 2, 0));
        break;
    case CenterRight:
        beg_ = ViewPort2World(start - Triple(width() + gap_, height() / 2, 0));
        break;
    case Center:
        beg_ = ViewPort2World(start - Triple(width() / 2, height() / 2, 0));
        break;
    default:
        break;
    }
    start = World2ViewPort(beg_);
    end_ = ViewPort2World(start + Triple(width(), height(), 0));
}

/**
 * \if ENGLISH
 * @brief Draws the label
 * \endif
 *
 * \if CHINESE
 * @brief 绘制标签
 * \endif
 */
void Label::draw()
{
    if (flagforupdate_) {
        update();
        flagforupdate_ = false;
    }

    if (buf_.isNull())
        return;

    GLboolean b;
    GLint func;
    GLdouble v;
    glGetBooleanv(GL_ALPHA_TEST, &b);
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &func);
    glGetDoublev(GL_ALPHA_TEST_REF, &v);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0.0);

    convert2screen();
    glRasterPos3d(beg_.x, beg_.y, beg_.z);

    int w = tex_.width();
    int h = tex_.height();

    if (devicefonts_) {
        drawDeviceText(QWT3DLOCAL8BIT(text_), "Courier", font_.pointSize(), pos_, color, anchor_,
                       gap_);
    } else {
        drawDevicePixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, tex_.bits());
    }

    glAlphaFunc(func, v);
    Enable(GL_ALPHA_TEST, b);
}

/**
 * \if ENGLISH
 * @brief Returns the label width in pixels
 * @return Label pixmap width
 * \endif
 *
 * \if CHINESE
 * @brief 返回标签宽度（像素）
 * @return 标签贴图宽度
 * \endif
 */
double Label::width() const
{
    return pm_.width();
}

/**
 * \if ENGLISH
 * @brief Returns the label height in pixels
 * @return Label pixmap height
 * \endif
 *
 * \if CHINESE
 * @brief 返回标签高度（像素）
 * @return 标签贴图高度
 * \endif
 */
double Label::height() const
{
    return pm_.height();
}