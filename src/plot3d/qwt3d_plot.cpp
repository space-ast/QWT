#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning(disable : 4305)
#pragma warning(disable : 4786)
#endif

#include "qwt3d_plot.h"
#include "qwt3d_enrichment.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Constructs a Plot3D widget
 * @param[in] parent Parent widget
 * @details This should be the first call in your derived classes constructors.
 * \endif
 *
 * \if CHINESE
 * @brief 构造 Plot3D 控件
 * @param[in] parent 父控件
 * @details 这应该是派生类构造函数中的第一个调用。
 * \endif
 */
Plot3D::Plot3D(QWidget *parent) : QOpenGLWidget(parent)
{
    initializedGL_ = false;
    renderpixmaprequest_ = false;
    xRot_ = yRot_ = zRot_ = 0.0; // default object rotation

    xShift_ = yShift_ = zShift_ = xVPShift_ = yVPShift_ = 0.0;
    xScale_ = yScale_ = zScale_ = 1.0;
    zoom_ = 1;
    ortho_ = true;
    plotstyle_ = FILLEDMESH;
    userplotstyle_p = 0;
    shading_ = GOURAUD;
    floorstyle_ = NOFLOOR;
    isolines_ = 10;
    displaylegend_ = false;
    smoothdatamesh_p = false;
    actualData_p = 0;

    lastMouseMovePosition_ = QPoint(0, 0);
    mpressed_ = false;
    mouse_input_enabled_ = true;

    setPolygonOffset(0.5);
    setMeshColor(RGBA(0.0, 0.0, 0.0));
    setMeshLineWidth(1);
    setBackgroundColor(RGBA(1.0, 1.0, 1.0, 1.0));

    displaylists_p = std::vector<GLuint>(DisplayListSize);
    for (unsigned k = 0; k != displaylists_p.size(); ++k) {
        displaylists_p[k] = 0;
    }

    datacolor_p = new StandardColor(this, 100);
    title_.setFont("Courier", 16, QFont::Bold);
    title_.setString("");

    setTitlePosition(0.95);

    kbd_input_enabled_ = true;

    setFocusPolicy(Qt::StrongFocus);
    assignMouse(Qt::LeftButton, MouseState(Qt::LeftButton, Qt::ShiftModifier), Qt::LeftButton,
                MouseState(Qt::LeftButton, Qt::AltModifier),
                MouseState(Qt::LeftButton, Qt::AltModifier),
                MouseState(Qt::LeftButton, Qt::AltModifier | Qt::ShiftModifier),
                MouseState(Qt::LeftButton, Qt::AltModifier | Qt::ControlModifier),
                MouseState(Qt::LeftButton, Qt::ControlModifier),
                MouseState(Qt::LeftButton, Qt::ControlModifier));

    assignKeyboard(Qt::Key_Down, Qt::Key_Up, KeyboardState(Qt::Key_Right, Qt::ShiftModifier),
                   KeyboardState(Qt::Key_Left, Qt::ShiftModifier), Qt::Key_Right, Qt::Key_Left,
                   KeyboardState(Qt::Key_Right, Qt::AltModifier),
                   KeyboardState(Qt::Key_Left, Qt::AltModifier),
                   KeyboardState(Qt::Key_Down, Qt::AltModifier),
                   KeyboardState(Qt::Key_Up, Qt::AltModifier),
                   KeyboardState(Qt::Key_Down, Qt::AltModifier | Qt::ShiftModifier),
                   KeyboardState(Qt::Key_Up, Qt::AltModifier | Qt::ShiftModifier),
                   KeyboardState(Qt::Key_Down, Qt::AltModifier | Qt::ControlModifier),
                   KeyboardState(Qt::Key_Up, Qt::AltModifier | Qt::ControlModifier),
                   KeyboardState(Qt::Key_Right, Qt::ControlModifier),
                   KeyboardState(Qt::Key_Left, Qt::ControlModifier),
                   KeyboardState(Qt::Key_Down, Qt::ControlModifier),
                   KeyboardState(Qt::Key_Up, Qt::ControlModifier));
    setKeySpeed(3, 5, 5);

    legend_.setLimits(0, 100);
    legend_.setMajors(10);
    legend_.setMinors(2);
    legend_.setOrientation(ColorLegend::BottomTop, ColorLegend::Left);

    lighting_enabled_ = false;
    disableLighting();
    lights_ = std::vector<Light>(8);
}

/**
 * \if ENGLISH
 * @brief Destructor - releases allocated resources
 * \endif
 *
 * \if CHINESE
 * @brief 析构函数 - 释放已分配的资源
 * \endif
 */
Plot3D::~Plot3D()
{
    makeCurrent();
    SaveGlDeleteLists(displaylists_p[0], static_cast<GLsizei>(displaylists_p.size()));
    datacolor_p->destroy();
    delete userplotstyle_p;
    for (ELIT it = elist_p.begin(); it != elist_p.end(); ++it)
        delete (*it);

    elist_p.clear();
}

/**
 * \if ENGLISH
 * @brief Sets up the OpenGL rendering state
 * \endif
 *
 * \if CHINESE
 * @brief 设置 OpenGL 渲染状态
 * \endif
 */
void Plot3D::initializeGL()
{
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    // Set up the lights

    disableLighting();

    GLfloat whiteAmb[4] = { 1.0, 1.0, 1.0, 1.0 };

    setLightShift(0, 0, 3000);
    glEnable(GL_COLOR_MATERIAL);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, whiteAmb);

    setMaterialComponent(GL_DIFFUSE, 1.0);
    setMaterialComponent(GL_SPECULAR, 0.3);
    setMaterialComponent(GL_SHININESS, 5.0);
    setLightComponent(GL_DIFFUSE, 1.0);
    setLightComponent(GL_SPECULAR, 1.0);

    initializedGL_ = true;
    if (renderpixmaprequest_) {
        updateData();
        renderpixmaprequest_ = false;
    }
}

/**
 * \if ENGLISH
 * @brief Paints the widget's content
 * \endif
 *
 * \if CHINESE
 * @brief 绘制控件内容
 * \endif
 */
void Plot3D::paintGL()
{
    glClearColor(bgcolor_.r, bgcolor_.g, bgcolor_.b, bgcolor_.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    applyLights();

    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(0.0, 0.0, 1.0, 0.0);
    glRotatef(0.0, 0.0, 0.0, 1.0);

    if (displaylegend_) {
        legend_.draw();
    }
    title_.setRelPosition(titlerel_, titleanchor_);
    title_.draw();

    Triple beg = coordinates_p.first();
    Triple end = coordinates_p.second();

    Triple center = beg + (end - beg) / 2;
    double radius = (center - beg).length();

    glLoadIdentity();

    glRotatef(xRot_ - 90, 1.0, 0.0, 0.0);
    glRotatef(yRot_, 0.0, 1.0, 0.0);
    glRotatef(zRot_, 0.0, 0.0, 1.0);

    glScalef(zoom_ * xScale_, zoom_ * yScale_, zoom_ * zScale_);

    glTranslatef(xShift_ - center.x, yShift_ - center.y, zShift_ - center.z);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (beg != end) {
        if (ortho_) {
            glOrtho(-radius, +radius, -radius, +radius, 0, 40 * radius);
        } else {
            glFrustum(-radius, +radius, -radius, +radius, 5 * radius, 400 * radius);
        }
    } else {
        if (ortho_)
            glOrtho(-1.0, 1.0, -1.0, 1.0, 10.0, 100.0);
        else
            glFrustum(-1.0, 1.0, -1.0, 1.0, 10.0, 100.0);
    }

    glTranslatef(xVPShift_ * 2 * radius, yVPShift_ * 2 * radius, -7 * radius);

    if (lighting_enabled_)
        glEnable(GL_NORMALIZE);

    for (unsigned i = 0; i != displaylists_p.size(); ++i) {
        if (i != LegendObject)
            glCallList(displaylists_p[i]);
    }
    coordinates_p.draw();

    if (lighting_enabled_)
        glDisable(GL_NORMALIZE);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/**
 * \if ENGLISH
 * @brief Sets up the OpenGL view port
 * @param[in] w New width
 * @param[in] h New height
 * \endif
 *
 * \if CHINESE
 * @brief 设置 OpenGL 视口
 * @param[in] w 新宽度
 * @param[in] h 新高度
 * \endif
 */
void Plot3D::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    paintGL();
}

/**
 * \if ENGLISH
 * @brief Creates a coordinate system with generating corners beg and end
 * @param[in] beg Minimum vertex of the coordinate system
 * @param[in] end Maximum vertex of the coordinate system
 * \endif
 *
 * \if CHINESE
 * @brief 创建具有生成角点 beg 和 end 的坐标系
 * @param[in] beg 坐标系的最小顶点
 * @param[in] end 坐标系的最大顶点
 * \endif
 */
void Plot3D::createCoordinateSystem(Triple beg, Triple end)
{
    if (beg != coordinates_p.first() || end != coordinates_p.second())
        coordinates_p.init(beg, end);
}

/**
 * \if ENGLISH
 * @brief Creates a coordinate system from data
 * @details Calculates the hull first, then creates the coordinate system from hull boundaries.
 * \endif
 *
 * \if CHINESE
 * @brief 从数据创建坐标系
 * @details 先计算外壳，然后从外壳边界创建坐标系。
 * \endif
 */
void Plot3D::createCoordinateSystem()
{
    calculateHull();
    Triple beg = hull().minVertex; // Irix 6.5 compiler bug
    Triple end = hull().maxVertex;
    createCoordinateSystem(beg, end);
}

/**
 * \if ENGLISH
 * @brief Shows or hides the color legend
 * @param[in] show True to show, false to hide
 * \endif
 *
 * \if CHINESE
 * @brief 显示或隐藏颜色图例
 * @param[in] show true 显示，false 隐藏
 * \endif
 */
void Plot3D::showColorLegend(bool show)
{
    displaylegend_ = show;
    if (show)
        datacolor_p->createVector(legend_.colors);
    update();
}

/**
 * \if ENGLISH
 * @brief Sets the mesh color
 * @param[in] rgba Mesh color as RGBA value
 * \endif
 *
 * \if CHINESE
 * @brief 设置网格颜色
 * @param[in] rgba 网格颜色的 RGBA 值
 * \endif
 */
void Plot3D::setMeshColor(RGBA rgba)
{
    meshcolor_ = rgba;
}

/**
 * \if ENGLISH
 * @brief Sets the background color
 * @param[in] rgba Background color as RGBA value
 * \endif
 *
 * \if CHINESE
 * @brief 设置背景颜色
 * @param[in] rgba 背景颜色的 RGBA 值
 * \endif
 */
void Plot3D::setBackgroundColor(RGBA rgba)
{
    bgcolor_ = rgba;
}

/**
 * \if ENGLISH
 * @brief Assigns a new coloring object for the data
 * @param[in] col Pointer to a new Color object
 * \endif
 *
 * \if CHINESE
 * @brief 为数据分配新的颜色对象
 * @param[in] col 新的 Color 对象指针
 * \endif
 */
void Plot3D::setDataColor(Color *col)
{
    Q_ASSERT(datacolor_p);

    datacolor_p->destroy();
    datacolor_p = col;
}

/**
 * \if ENGLISH
 * @brief Sets up orthogonal or perspective mode and updates widget
 * @param[in] val True for orthogonal projection, false for perspective
 * \endif
 *
 * \if CHINESE
 * @brief 设置正交或透视模式并更新控件
 * @param[in] val true 为正交投影，false 为透视投影
 * \endif
 */
void Plot3D::setOrtho(bool val)
{
    if (val == ortho_)
        return;
    ortho_ = val;
    update();

    emit projectionChanged(val);
}

/**
 * \if ENGLISH
 * @brief Sets style of coordinate system
 * @param[in] st Coordinate system style (NOCOORD, BOX, or FRAME)
 * \endif
 *
 * \if CHINESE
 * @brief 设置坐标系样式
 * @param[in] st 坐标系样式（NOCOORD、BOX 或 FRAME）
 * \endif
 */
void Plot3D::setCoordinateStyle(COORDSTYLE st)
{
    coordinates_p.setStyle(st);
    update();
}

/**
 * \if ENGLISH
 * @brief Sets plot style for the standard plotting types
 * @param[in] val Plot style value. An argument of value Qwt3D::USER is ignored.
 * \endif
 *
 * \if CHINESE
 * @brief 设置标准绘图类型的绘图样式
 * @param[in] val 绘图样式值。值为 Qwt3D::USER 的参数将被忽略。
 * \endif
 */
void Plot3D::setPlotStyle(PLOTSTYLE val)
{
    if (val == Qwt3D::USER)
        return;
    delete userplotstyle_p;
    userplotstyle_p = 0;
    plotstyle_ = val;
}

/**
 * \if ENGLISH
 * @brief Sets plot style to Qwt3D::USER with an associated enrichment object
 * @param[in] obj Reference to an Enrichment object
 * @return Pointer to the cloned enrichment object
 * \endif
 *
 * \if CHINESE
 * @brief 将绘图样式设置为 Qwt3D::USER 并关联一个装饰对象
 * @param[in] obj Enrichment 对象的引用
 * @return 克隆的装饰对象指针
 * \endif
 */
Qwt3D::Enrichment *Plot3D::setPlotStyle(Qwt3D::Enrichment const &obj)
{
    if (&obj == userplotstyle_p)
        return userplotstyle_p;

    delete userplotstyle_p;
    userplotstyle_p = obj.clone();
    plotstyle_ = Qwt3D::USER;
    return userplotstyle_p;
}

/**
 * \if ENGLISH
 * @brief Sets shading style
 * @param[in] val Shading style (FLAT or GOURAUD)
 * \endif
 *
 * \if CHINESE
 * @brief 设置着色样式
 * @param[in] val 着色样式（FLAT 或 GOURAUD）
 * \endif
 */
void Plot3D::setShading(SHADINGSTYLE val)
{
    if (val == shading_)
        return;

    shading_ = val;

    switch (shading_) {
    case FLAT:
        glShadeModel(GL_FLAT);
        break;
    case GOURAUD:
        glShadeModel(GL_SMOOTH);
        break;
    default:
        break;
    }
    update();
}

/**
 * \if ENGLISH
 * @brief Sets number of isolines
 * @param[in] steps Number of isolines. The lines are equidistant between minimal and maximal Z value.
 * \endif
 *
 * \if CHINESE
 * @brief 设置等值线数量
 * @param[in] steps 等值线数量。等值线在最小和最大 Z 值之间等距分布。
 * \endif
 */
void Plot3D::setIsolines(int steps)
{
    if (steps < 0)
        return;

    isolines_ = steps;
}

/**
 * \if ENGLISH
 * @brief Sets polygon offset
 * @param[in] val Polygon offset value
 * @details The function affects the OpenGL rendering process. Try different values
 *          for surfaces with polygons only and with mesh and polygons.
 * \endif
 *
 * \if CHINESE
 * @brief 设置多边形偏移
 * @param[in] val 多边形偏移值
 * @details 此函数影响 OpenGL 渲染过程。对于仅含多边形和含网格与多边形的表面，
 *          请尝试不同的值。
 * \endif
 */
void Plot3D::setPolygonOffset(double val)
{
    polygonOffset_ = val;
}

/**
 * \if ENGLISH
 * @brief Sets the mesh line width
 * @param[in] val Line width value (must be >= 0)
 * \endif
 *
 * \if CHINESE
 * @brief 设置网格线宽
 * @param[in] val 线宽值（必须 >= 0）
 * \endif
 */
void Plot3D::setMeshLineWidth(double val)
{
    Q_ASSERT(val >= 0);

    if (val < 0)
        return;

    meshLineWidth_ = val;
}

/**
 * \if ENGLISH
 * @brief Sets relative caption position
 * @param[in] rely Relative Y position (0-1). (0.5,0.5) means the anchor point lies in the center of the screen.
 * @param[in] relx Relative X position (0-1)
 * @param[in] anchor Anchor type for title alignment
 * \endif
 *
 * \if CHINESE
 * @brief 设置标题的相对位置
 * @param[in] rely 相对 Y 位置（0-1）。(0.5,0.5) 表示锚点位于屏幕中心。
 * @param[in] relx 相对 X 位置（0-1）
 * @param[in] anchor 标题对齐的锚点类型
 * \endif
 */
void Plot3D::setTitlePosition(double rely, double relx, Qwt3D::ANCHOR anchor)
{
    titlerel_.y = (rely < 0 || rely > 1) ? 0.5 : rely;
    titlerel_.x = (relx < 0 || relx > 1) ? 0.5 : relx;

    titleanchor_ = anchor;
}

/**
 * \if ENGLISH
 * @brief Sets the caption font
 * @param[in] family Font family name
 * @param[in] pointSize Font point size
 * @param[in] weight Font weight
 * @param[in] italic Whether font is italic
 * \endif
 *
 * \if CHINESE
 * @brief 设置标题字体
 * @param[in] family 字体族名
 * @param[in] pointSize 字体磅值大小
 * @param[in] weight 字体粗细
 * @param[in] italic 是否斜体
 * \endif
 */
void Plot3D::setTitleFont(const QString &family, int pointSize, int weight, bool italic)
{
    title_.setFont(family, pointSize, weight, italic);
}

/**
 * \if ENGLISH
 * @brief Adds an enrichment object to the plot
 * @param[in] e Reference to an Enrichment object
 * @return Pointer to the cloned enrichment object added to the list
 * \endif
 *
 * \if CHINESE
 * @brief 向绘图添加装饰对象
 * @param[in] e Enrichment 对象的引用
 * @return 添加到列表中的克隆装饰对象指针
 * \endif
 */
Enrichment *Plot3D::addEnrichment(Enrichment const &e)
{
    if (elist_p.end() == std::find(elist_p.begin(), elist_p.end(), &e))
        elist_p.push_back(e.clone());
    return elist_p.back();
}

/**
 * \if ENGLISH
 * @brief Removes an enrichment object from the plot
 * @param[in] e Pointer to the Enrichment object to remove
 * @return True if the enrichment was found and removed, false otherwise
 * \endif
 *
 * \if CHINESE
 * @brief 从绘图中移除装饰对象
 * @param[in] e 要移除的 Enrichment 对象指针
 * @return 找到并移除装饰时返回 true，否则返回 false
 * \endif
 */
bool Plot3D::degrade(Enrichment *e)
{
    ELIT it = std::find(elist_p.begin(), elist_p.end(), e);

    if (it != elist_p.end()) {
        delete (*it);
        elist_p.erase(it);
        return true;
    }
    return false;
}

void Plot3D::createEnrichments()
{
    for (ELIT it = elist_p.begin(); it != elist_p.end(); ++it) {
        this->createEnrichment(**it);
    }
}

/**
 * \if ENGLISH
 * @brief Updates OpenGL data representation
 * \endif
 *
 * \if CHINESE
 * @brief 更新 OpenGL 数据表示
 * \endif
 */
void Plot3D::updateData()
{
    makeCurrent();
    GLStateBewarer dt(GL_DEPTH_TEST, true);
    GLStateBewarer ls(GL_LINE_SMOOTH, true);

    calculateHull();

    SaveGlDeleteLists(displaylists_p[DataObject], 1); // nur Daten

    displaylists_p[DataObject] = glGenLists(1);
    glNewList(displaylists_p[DataObject], GL_COMPILE);

    this->createEnrichments();
    this->createData();

    glEndList();
}
