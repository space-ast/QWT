#include "qwt3d_drawable.h"

using namespace Qwt3D;

Drawable::~Drawable()
{
    detachAll();
}

void Drawable::saveGLState()
{
    glGetBooleanv(GL_LINE_SMOOTH, &ls);
    glGetBooleanv(GL_POLYGON_SMOOTH, &pols);
    glGetFloatv(GL_LINE_WIDTH, &lw);
    glGetIntegerv(GL_BLEND_SRC, &blsrc);
    glGetIntegerv(GL_BLEND_DST, &bldst);
    glGetDoublev(GL_CURRENT_COLOR, col);
    glGetIntegerv(GL_LINE_STIPPLE_PATTERN, &pattern);
    glGetIntegerv(GL_LINE_STIPPLE_REPEAT, &factor);
    glGetBooleanv(GL_LINE_STIPPLE, &sallowed);
    glGetBooleanv(GL_TEXTURE_2D, &tex2d);
    glGetIntegerv(GL_POLYGON_MODE, polmode);
    glGetIntegerv(GL_MATRIX_MODE, &matrixmode);
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &poloffs[ 0 ]);
    glGetFloatv(GL_POLYGON_OFFSET_UNITS, &poloffs[ 1 ]);
    glGetBooleanv(GL_POLYGON_OFFSET_FILL, &poloffsfill);
}

void Drawable::restoreGLState()
{
    Enable(GL_LINE_SMOOTH, ls);
    Enable(GL_POLYGON_SMOOTH, pols);

    setDeviceLineWidth(lw);
    glBlendFunc(blsrc, bldst);
    glColor4dv(col);

    glLineStipple(factor, pattern);
    Enable(GL_LINE_STIPPLE, sallowed);
    Enable(GL_TEXTURE_2D, tex2d);
    glPolygonMode(polmode[ 0 ], polmode[ 1 ]);
    glMatrixMode(matrixmode);
    glPolygonOffset(poloffs[ 0 ], poloffs[ 1 ]);
    setDevicePolygonOffset(poloffs[ 0 ], poloffs[ 1 ]);

    Enable(GL_POLYGON_OFFSET_FILL, poloffsfill);
}

void Drawable::Enable(GLenum what, GLboolean val)
{
    if (val)
        glEnable(what);
    else
        glDisable(what);
}

void Drawable::attach(Drawable* dr)
{
    if (dlist.end() == std::find(dlist.begin(), dlist.end(), dr))
        if (dr) {
            dlist.push_back(dr);
        }
}

void Drawable::detach(Drawable* dr)
{
    std::list< Drawable* >::iterator it = std::find(dlist.begin(), dlist.end(), dr);

    if (it != dlist.end()) {
        dlist.erase(it);
    }
}
void Drawable::detachAll()
{
    dlist.clear();
}

/**
 * \if ENGLISH
 * @brief Converts viewport coordinates to world coordinates (glUnProject)
 * @param[in] win Viewport (window) coordinates
 * @param[out] err Optional error flag (true on failure)
 * @return World (object) coordinates
 * @warning Don't rely on (use) this in display lists!
 * \endif
 *
 * \if CHINESE
 * @brief 将视口坐标转换为世界坐标（glUnProject）
 * @param[in] win 视口（窗口）坐标
 * @param[out] err 可选的错误标志（失败时为 true）
 * @return 世界（对象）坐标
 * @warning 不要在显示列表中使用此功能！
 * \endif
 */
Triple Drawable::ViewPort2World(Triple win, bool* err)
{
    Triple obj;

    getMatrices(modelMatrix, projMatrix, viewport);
    int res = gluUnProject(win.x, win.y, win.z, modelMatrix, projMatrix, viewport, &obj.x, &obj.y, &obj.z);

    if (err)
        *err = (res) ? false : true;
    return obj;
}

/**
 * \if ENGLISH
 * @brief Converts world coordinates to viewport coordinates (glProject)
 * @param[in] obj World (object) coordinates
 * @param[out] err Optional error flag (true on failure)
 * @return Viewport (window) coordinates
 * @warning Don't rely on (use) this in display lists!
 * \endif
 *
 * \if CHINESE
 * @brief 将世界坐标转换为视口坐标（glProject）
 * @param[in] obj 世界（对象）坐标
 * @param[out] err 可选的错误标志（失败时为 true）
 * @return 视口（窗口）坐标
 * @warning 不要在显示列表中使用此功能！
 * \endif
 */
Triple Drawable::World2ViewPort(Triple obj, bool* err)
{
    Triple win;

    getMatrices(modelMatrix, projMatrix, viewport);
    int res = gluProject(obj.x, obj.y, obj.z, modelMatrix, projMatrix, viewport, &win.x, &win.y, &win.z);

    if (err)
        *err = (res) ? false : true;
    return win;
}

/**
 * \if ENGLISH
 * @brief Calculates world coordinates from relative viewport position
 * @param[in] rel Relative position in viewport coordinates
 * @return Corresponding world coordinates
 * @warning Don't rely on (use) this in display lists!
 * \endif
 *
 * \if CHINESE
 * @brief 从相对视口位置计算世界坐标
 * @param[in] rel 视口坐标中的相对位置
 * @return 对应的世界坐标
 * @warning 不要在显示列表中使用此功能！
 * \endif
 */
Triple Drawable::relativePosition(Triple rel)
{
    return ViewPort2World(Triple((rel.x - viewport[ 0 ]) * viewport[ 2 ], (rel.y - viewport[ 1 ]) * viewport[ 3 ], rel.z));
}

void Drawable::draw()
{
    saveGLState();

    for (auto* drawable : dlist) {
        drawable->draw();
    }
    restoreGLState();
}

void Drawable::setColor(double r, double g, double b, double a)
{
    color = RGBA(r, g, b, a);
}

void Drawable::setColor(RGBA rgba)
{
    color = rgba;
}
