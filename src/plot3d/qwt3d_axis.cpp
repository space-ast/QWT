#include "qwt3d_axis.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Default constructor
 * @details Constructs an uninitialized axis with default parameters.
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数
 * @details 使用默认参数构造未初始化的坐标轴。
 * \endif
 */
Axis::Axis()
{
    init();
}

/**
 * \if ENGLISH
 * @brief Destructor
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数
 * \endif
 */
Axis::~Axis()
{
}

/**
 * \if ENGLISH
 * @brief Constructs an axis with specified start and end positions
 * @param[in] beg Start position of the axis
 * @param[in] end End position of the axis
 * \endif
 *
 * \if CHINESE
 * @brief 构造具有指定起点和终点位置的坐标轴
 * @param[in] beg 坐标轴的起点位置
 * @param[in] end 坐标轴的终点位置
 * \endif
 */
Axis::Axis(Triple beg, Triple end)
{
    init();
    setPosition(beg, end);
}

void Axis::init()
{
    detachAll();

    scale_ = qwt3d_ptr< Scale >(new LinearScale);

    beg_ = Triple(0.0, 0.0, 0.0);
    end_ = beg_;

    majorintervals_ = 0;
    minorintervals_ = 0;
    setMajors(1);
    setMinors(1);
    setLimits(0, 0);

    setTicOrientation(0.0, 0.0, 0.0);
    setTicLength(0.0, 0.0);
    setColor(0.0, 0.0, 0.0);
    setLineWidth(1.0);
    symtics_     = false;
    drawNumbers_ = false;
    drawLabel_   = false;

    drawTics_  = false;
    autoscale_ = true;
    markerLabel_.clear();
    numberfont_ = QFont("Courier", 12);
    setLabelFont(QFont("Courier", 14));

    numbercolor_ = RGBA(0, 0, 0, 0);

    setNumberAnchor(Center);

    numbergap_ = 0;
    labelgap_  = 0;
}

/**
 * \if ENGLISH
 * @brief Sets the axis position
 * @param[in] beg Start position of the axis
 * @param[in] end End position of the axis
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴位置
 * @param[in] beg 坐标轴的起点位置
 * @param[in] end 坐标轴的终点位置
 * \endif
 */
void Axis::setPosition(const Triple& beg, const Triple& end)
{
    beg_ = beg;
    end_ = end;
}

/**
 * \if ENGLISH
 * @brief Sets number of major intervals
 * @param[in] val Number of major intervals (always >= 1)
 * \endif
 *
 * \if CHINESE
 * @brief 设置主刻度区间数
 * @param[in] val 主刻度区间数（始终 >= 1）
 * \endif
 */
void Axis::setMajors(int val)
{
    if (val == majorintervals_)
        return;

    majorintervals_ = (val <= 0) ? 1 : val;  // always >= 1
}

/**
 * \if ENGLISH
 * @brief Sets number of minor intervals
 * @param[in] val Number of minor intervals (always >= 1)
 * @see LogScale::setMinors()
 * \endif
 *
 * \if CHINESE
 * @brief 设置次刻度区间数
 * @param[in] val 次刻度区间数（始终 >= 1）
 * @see LogScale::setMinors()
 * \endif
 */
void Axis::setMinors(int val)
{
    if (val == minorintervals_)
        return;

    minorintervals_ = (val <= 0) ? 1 : val;  // always >= 1
}

/**
 * \if ENGLISH
 * @brief Sets tic length
 * @param[in] majorl Length of major tics
 * @param[in] minorl Length of minor tics
 * \endif
 *
 * \if CHINESE
 * @brief 设置刻度线长度
 * @param[in] majorl 主刻度线长度
 * @param[in] minorl 次刻度线长度
 * \endif
 */
void Axis::setTicLength(double majorl, double minorl)
{
    lmaj_ = majorl;
    lmin_ = minorl;
}

/**
 * \if ENGLISH
 * @brief Sets tic orientation from individual components
 * @param[in] tx X component of tic orientation
 * @param[in] ty Y component of tic orientation
 * @param[in] tz Z component of tic orientation
 * \endif
 *
 * \if CHINESE
 * @brief 从各分量设置刻度线方向
 * @param[in] tx 刻度线方向的 X 分量
 * @param[in] ty 刻度线方向的 Y 分量
 * @param[in] tz 刻度线方向的 Z 分量
 * \endif
 */
void Axis::setTicOrientation(double tx, double ty, double tz)
{
    setTicOrientation(Triple(tx, ty, tz));
}

/**
 * \if ENGLISH
 * @brief Sets tic orientation from a Triple vector
 * @param[in] val Orientation vector for tics (will be normalized)
 * \endif
 *
 * \if CHINESE
 * @brief 从三元组向量设置刻度线方向
 * @param[in] val 刻度线的方向向量（将被归一化）
 * \endif
 */
void Axis::setTicOrientation(const Triple& val)
{
    orientation_ = val;
    orientation_.normalize();
}

/**
 * \if ENGLISH
 * @brief Sets line width for axis and tics
 * @param[in] val Thickness for axis base line
 * @param[in] majfac Relative thickness for axis major tics (majfac*val)
 * @param[in] minfac Relative thickness for axis minor tics (minfac*val)
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴和刻度线的线宽
 * @param[in] val 坐标轴基线的厚度
 * @param[in] majfac 主刻度线的相对厚度（majfac*val）
 * @param[in] minfac 次刻度线的相对厚度（minfac*val）
 * \endif
 */
void Axis::setLineWidth(double val, double majfac, double minfac)
{
    lineWidth_    = val;
    majLineWidth_ = majfac * lineWidth_;
    minLineWidth_ = minfac * lineWidth_;
}

/**
 * \if ENGLISH
 * @brief Draws the axis including base line, tics, and label
 * \endif
 *
 * \if CHINESE
 * @brief 绘制坐标轴，包括基线、刻度线和标签
 * \endif
 */
void Axis::draw()
{
    Drawable::draw();

    saveGLState();

    //	GLStateBewarer sb(GL_LINE_SMOOTH, true);
    //	glBlendFunc(GL_ONE, GL_ZERO);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(color.r, color.g, color.b, color.a);

    drawBase();
    drawTics();
    drawLabel();

    restoreGLState();
}

void Axis::drawLabel()
{
    if (!drawLabel_)
        return;

    Triple diff   = end() - begin();
    Triple center = begin() + diff / 2;

    Triple bnumber = biggestNumberString();
    //	double fac = 6*(second()-first()).length() / 100;

    switch (scaleNumberAnchor_) {
    case BottomLeft:
    case TopLeft:
    case CenterLeft:
        bnumber.y = 0;
        break;
    case BottomRight:
    case TopRight:
    case CenterRight:
        bnumber.x = -bnumber.x;
        bnumber.y = 0;
        break;
    case TopCenter:
        bnumber.x = 0;
        bnumber.y = -bnumber.y;
        break;
    case BottomCenter:
        bnumber.x = 0;
        break;
    default:
        break;
    }

    Triple pos = ViewPort2World(World2ViewPort(center + ticOrientation() * lmaj_) + bnumber);
    setLabelPosition(pos, scaleNumberAnchor_);

    label_.adjust(labelgap_);
    label_.draw();
}

void Axis::drawBase()
{
    setDeviceLineWidth(lineWidth_);
    glBegin(GL_LINES);
    glVertex3d(beg_.x, beg_.y, beg_.z);
    glVertex3d(end_.x, end_.y, end_.z);
    glEnd();
}

bool Axis::prepTicCalculation(Triple& startpoint)
{
    if (isPracticallyZero(start_, stop_))
        return false;

    autostart_ = start_;
    autostop_  = stop_;

    if (autoScale()) {
        setMajors(scale_->autoscale(autostart_, autostop_, start_, stop_, majors()));
        if (isPracticallyZero(autostart_, autostop_))
            return false;
    }

    scale_->setLimits(start_, stop_);
    scale_->setMajors(majors());
    scale_->setMinors(minors());
    scale_->setMajorLimits(autostart_, autostop_);
    scale_->calculate();

    startpoint = end_ - beg_;

    majorpos_.clear();
    minorpos_.clear();

    return true;
}

void Axis::recalculateTics()
{
    Triple runningpoint;
    if (false == prepTicCalculation(runningpoint))
        return;

    unsigned int i;

    for (i = 0; i != scale_->majors_p.size(); ++i) {
        double t = (scale_->majors_p[ i ] - start_) / (stop_ - start_);
        majorpos_.push_back(beg_ + t * runningpoint);
    }
    for (i = 0; i != scale_->minors_p.size(); ++i) {
        double t = (scale_->minors_p[ i ] - start_) / (stop_ - start_);
        minorpos_.push_back(beg_ + t * runningpoint);
    }
}

void Axis::drawTics()
{
    Triple runningpoint;
    if (!drawTics_ || false == prepTicCalculation(runningpoint))
        return;

    unsigned int i;
    Triple nadir;

    markerLabel_.resize(scale_->majors_p.size());
    setDeviceLineWidth(majLineWidth_);
    for (i = 0; i != scale_->majors_p.size(); ++i) {
        double t = (scale_->majors_p[ i ] - start_) / (stop_ - start_);
        nadir    = beg_ + t * runningpoint;
        majorpos_.push_back(drawTic(nadir, lmaj_));
        drawTicLabel(nadir + 1.2 * lmaj_ * orientation_, i);
    }
    setDeviceLineWidth(minLineWidth_);
    for (i = 0; i != scale_->minors_p.size(); ++i) {
        double t = (scale_->minors_p[ i ] - start_) / (stop_ - start_);
        nadir    = beg_ + t * runningpoint;
        minorpos_.push_back(drawTic(nadir, lmin_));
    }
}

void Axis::drawTicLabel(Triple pos, int mtic)
{
    if (!drawNumbers_ || (mtic < 0))
        return;

    markerLabel_[ mtic ].setFont(numberfont_.family(), numberfont_.pointSize(), numberfont_.weight(), numberfont_.italic());
    markerLabel_[ mtic ].setColor(numbercolor_);
    markerLabel_[ mtic ].setString(scale_->ticLabel(mtic));
    markerLabel_[ mtic ].setPosition(pos, scaleNumberAnchor_);
    markerLabel_[ mtic ].adjust(numbergap_);
    markerLabel_[ mtic ].draw();
}

Triple Axis::drawTic(Triple nadir, double length)
{
    double ilength = (symtics_) ? -length : 0.0;

    glBegin(GL_LINES);
    glVertex3d(nadir.x + ilength * orientation_.x, nadir.y + ilength * orientation_.y, nadir.z + ilength * orientation_.z);
    glVertex3d(nadir.x + length * orientation_.x, nadir.y + length * orientation_.y, nadir.z + length * orientation_.z);
    glEnd();
    return nadir;
}

/**
 * \if ENGLISH
 * @brief Sets the font for axis numbers
 * @param[in] family Font family name
 * @param[in] pointSize Font point size
 * @param[in] weight Font weight
 * @param[in] italic Whether font is italic
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴数字的字体
 * @param[in] family 字体族名
 * @param[in] pointSize 字体磅值大小
 * @param[in] weight 字体粗细
 * @param[in] italic 是否斜体
 * \endif
 */
void Axis::setNumberFont(QString const& family, int pointSize, int weight, bool italic)
{
    numberfont_ = QFont(family, pointSize, weight, italic);
}

/**
 * \if ENGLISH
 * @brief Sets the font for axis numbers
 * @param[in] font QFont object to use for axis numbers
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴数字的字体
 * @param[in] font 用于坐标轴数字的 QFont 对象
 * \endif
 */
void Axis::setNumberFont(QFont const& font)
{
    numberfont_ = font;
}

/**
 * \if ENGLISH
 * @brief Sets the color for axis numbers
 * @param[in] col RGBA color value for axis numbers
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴数字的颜色
 * @param[in] col 坐标轴数字的 RGBA 颜色值
 * \endif
 */
void Axis::setNumberColor(RGBA col)
{
    numbercolor_ = col;
}

/**
 * \if ENGLISH
 * @brief Sets the font for the axis label
 * @param[in] family Font family name
 * @param[in] pointSize Font point size
 * @param[in] weight Font weight
 * @param[in] italic Whether font is italic
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴标签的字体
 * @param[in] family 字体族名
 * @param[in] pointSize 字体磅值大小
 * @param[in] weight 字体粗细
 * @param[in] italic 是否斜体
 * \endif
 */
void Axis::setLabelFont(QString const& family, int pointSize, int weight, bool italic)
{
    labelfont_ = QFont(family, pointSize, weight, italic);
    label_.setFont(family, pointSize, weight, italic);
}

/**
 * \if ENGLISH
 * @brief Sets the font for the axis label
 * @param[in] font QFont object to use for the axis label
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴标签的字体
 * @param[in] font 用于坐标轴标签的 QFont 对象
 * \endif
 */
void Axis::setLabelFont(QFont const& font)
{
    setLabelFont(font.family(), font.pointSize(), font.weight(), font.italic());
}

/**
 * \if ENGLISH
 * @brief Sets the axis label string
 * @param[in] name The label text string
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴标签字符串
 * @param[in] name 标签文本字符串
 * \endif
 */
void Axis::setLabelString(QString const& name)
{
    label_.setString(name);
}

/**
 * \if ENGLISH
 * @brief Sets label position in conjunction with an anchoring strategy
 * @param[in] pos Position for the label
 * @param[in] an Anchor strategy for the label
 * \endif
 *
 * \if CHINESE
 * @brief 结合锚定策略设置标签位置
 * @param[in] pos 标签的位置
 * @param[in] an 标签的锚定策略
 * \endif
 */
void Axis::setLabelPosition(const Triple& pos, Qwt3D::ANCHOR an)
{
    label_.setPosition(pos, an);
}

/**
 * \if ENGLISH
 * @brief Sets color for the axis label
 * @param[in] col RGBA color value for the label
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标轴标签的颜色
 * @param[in] col 标签的 RGBA 颜色值
 * \endif
 */
void Axis::setLabelColor(RGBA col)
{
    label_.setColor(col);
}

Triple Axis::biggestNumberString()
{
    Triple ret;
    size_t size = markerLabel_.size();

    double width, height;

    for (unsigned i = 0; i != size; ++i) {
        width  = fabs((World2ViewPort(markerLabel_[ i ].second()) - World2ViewPort(markerLabel_[ i ].first())).x);
        height = fabs((World2ViewPort(markerLabel_[ i ].second()) - World2ViewPort(markerLabel_[ i ].first())).y);

        if (width > ret.x)
            ret.x = width + markerLabel_[ i ].gap();
        if (height > ret.y)
            ret.y = height + markerLabel_[ i ].gap();
        ;
    }
    return ret;
}

/**
 * \if ENGLISH
 * @brief Sets a user-defined scale object
 * @param[in] val Pointer to a Scale object. Use with a heap based initialized pointer only.
 *                The axis adopts ownership.
 * \endif
 *
 * \if CHINESE
 * @brief 设置用户自定义的刻度对象
 * @param[in] val Scale 对象的指针。仅使用基于堆的初始化指针。
 *                坐标轴将获得所有权。
 * \endif
 */
void Axis::setScale(Scale* val)
{
    scale_ = qwt3d_ptr< Scale >(val);
}

/**
 * \if ENGLISH
 * @brief Sets one of the predefined scaling types
 * @param[in] val Predefined scale type (LINEARSCALE or LOG10SCALE)
 * @warning Too small intervals in logarithmic scales lead to empty scales
 *          (or perhaps a scale only containing an isolated major tic).
 *          Better switch to linear scales in such cases.
 * \endif
 *
 * \if CHINESE
 * @brief 设置预定义的刻度类型之一
 * @param[in] val 预定义刻度类型（LINEARSCALE 或 LOG10SCALE）
 * @warning 对数刻度中过小的区间会导致空刻度（或仅包含孤立主刻度的刻度）。
 *          在这种情况下最好切换到线性刻度。
 * \endif
 */
void Axis::setScale(Qwt3D::SCALETYPE val)
{
    switch (val) {
    case Qwt3D::LINEARSCALE:
        setScale(new LinearScale);
        break;
    case Qwt3D::LOG10SCALE:
        setScale(new LogScale);
        setMinors(9);
        break;
    default:
        break;
    }
}