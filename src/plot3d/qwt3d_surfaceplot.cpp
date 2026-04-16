#include "qwt3d_surfaceplot.h"

using namespace std;
using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Constructs a SurfacePlot widget
 * @param[in] parent Parent widget
 * @details Initializes with dataNormals()==false, NOFLOOR, resolution() == 1
 * \endif
 *
 * \if CHINESE
 * @brief 构造 SurfacePlot 控件
 * @param[in] parent 父控件
 * @details 初始化时 dataNormals()==false, NOFLOOR, resolution() == 1
 * \endif
 */
SurfacePlot::SurfacePlot(QWidget* parent) : Plot3D(parent)
{
    datanormals_p   = false;
    normalLength_p  = 0.02;
    normalQuality_p = 3;

    resolution_p = 1;
    actualDataG_ = new GridData();
    actualDataC_ = new CellData();

    actualData_p = actualDataG_;

    floorstyle_ = NOFLOOR;
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
SurfacePlot::~SurfacePlot()
{
    delete actualDataG_;
    delete actualDataC_;
}

/**
 * \if ENGLISH
 * @brief Shows or hides data normals
 * @param[in] b True to show normals, false to hide
 * \endif
 *
 * \if CHINESE
 * @brief 显示或隐藏数据法线
 * @param[in] b true 显示法线，false 隐藏
 * \endif
 */
void SurfacePlot::showNormals(bool b)
{
    datanormals_p = b;
}

/**
 * \if ENGLISH
 * @brief Sets the normal vector length
 * @param[in] val Normal length value (0.0 to 1.0). Values < 0 or > 1 are ignored.
 * \endif
 *
 * \if CHINESE
 * @brief 设置法线向量长度
 * @param[in] val 法线长度值（0.0 到 1.0）。小于 0 或大于 1 的值将被忽略。
 * \endif
 */
void SurfacePlot::setNormalLength(double val)
{
    if (val < 0 || val > 1)
        return;
    normalLength_p = val;
}

/**
 * \if ENGLISH
 * @brief Sets the normal vector quality (number of arrow segments)
 * @param[in] val Quality value. Values < 3 are ignored.
 * \endif
 *
 * \if CHINESE
 * @brief 设置法线向量质量（箭头段数）
 * @param[in] val 质量值。小于 3 的值将被忽略。
 * \endif
 */
void SurfacePlot::setNormalQuality(int val)
{
    if (val < 3)
        return;
    normalQuality_p = val;
}

/**
 * \if ENGLISH
 * @brief Calculates the smallest x-y-z parallelepiped enclosing the data
 * @details It can be accessed by hull();
 * \endif
 *
 * \if CHINESE
 * @brief 计算包围数据的最小 x-y-z 平行六面体
 * @details 可以通过 hull() 访问。
 * \endif
 */
void SurfacePlot::calculateHull()
{
    if (actualData_p->empty())
        return;
    setHull(actualData_p->hull());
}

/**
 * \if ENGLISH
 * @brief Sets data resolution and updates widget
 * @param[in] res Resolution multiplier (res == 1 means original resolution). If res < 1, the function does nothing.
 * \endif
 *
 * \if CHINESE
 * @brief 设置数据分辨率并更新控件
 * @param[in] res 分辨率倍数（res == 1 表示原始分辨率）。如果 res < 1，函数不做任何操作。
 * \endif
 */
void SurfacePlot::setResolution(int res)
{
    if (!actualData_p || actualData_p->datatype == Qwt3D::POLYGON)
        return;

    if ((resolution_p == res) || res < 1)
        return;

    resolution_p = res;
    updateNormals();
    updateData();
    if (initializedGL())
        update();

    emit resolutionChanged(res);
}

void SurfacePlot::updateNormals()
{
    SaveGlDeleteLists(displaylists_p[ NormalObject ], 1);

    if ((plotStyle() == NOPLOT && !normals()) || !actualData_p)
        return;

    displaylists_p[ NormalObject ] = glGenLists(1);
    glNewList(displaylists_p[ NormalObject ], GL_COMPILE);

    if (actualData_p->datatype == Qwt3D::POLYGON)
        createNormalsC();
    else if (actualData_p->datatype == Qwt3D::GRID)
        createNormalsG();

    glEndList();
}

void SurfacePlot::createData()
{
    if (!actualData_p)
        return;
    if (actualData_p->datatype == Qwt3D::POLYGON)
        createDataC();
    else if (actualData_p->datatype == Qwt3D::GRID)
        createDataG();
}

void SurfacePlot::createFloorData()
{
    if (!actualData_p)
        return;
    if (actualData_p->datatype == Qwt3D::POLYGON)
        createFloorDataC();
    else if (actualData_p->datatype == Qwt3D::GRID)
        createFloorDataG();
}

/**
 * \if ENGLISH
 * @brief Returns the number of facets in the data
 * @return (columns,rows) for grid data, (number of cells,1) for polygon data, (0,0) otherwise
 * @details The returned value is not affected by resolution().
 * \endif
 *
 * \if CHINESE
 * @brief 返回数据中的面片数
 * @return 网格数据返回 (columns,rows)，多边形数据返回 (number of cells,1)，其他返回 (0,0)
 * @details 返回值不受 resolution() 影响。
 * \endif
 */
pair< int, int > SurfacePlot::facets() const
{
    if (!hasData())
        return pair< int, int >(0, 0);

    if (actualData_p->datatype == Qwt3D::POLYGON)
        return pair< int, int >(int(actualDataC_->cells.size()), 1);
    else if (actualData_p->datatype == Qwt3D::GRID)
        return pair< int, int >(actualDataG_->columns(), actualDataG_->rows());
    else
        return pair< int, int >(0, 0);
}

void SurfacePlot::createPoints()
{
    Dot pt;
    createEnrichment(pt);
}

void SurfacePlot::createEnrichment(Enrichment& p)
{
    if (!actualData_p)
        return;

    // todo future work
    if (p.type() != Enrichment::VERTEXENRICHMENT)
        return;

    p.assign(*this);
    p.drawBegin();

    VertexEnrichment* ve = static_cast< VertexEnrichment* >(&p);
    if (actualData_p->datatype == Qwt3D::POLYGON) {
        for (unsigned i = 0; i != actualDataC_->normals.size(); ++i)
            ve->draw(actualDataC_->nodes[ i ]);
    } else if (actualData_p->datatype == Qwt3D::GRID) {
        int step = resolution();
        for (int i = 0; i <= actualDataG_->columns() - step; i += step)
            for (int j = 0; j <= actualDataG_->rows() - step; j += step)
                ve->draw(Triple(actualDataG_->vertices[ i ][ j ][ 0 ],
                                actualDataG_->vertices[ i ][ j ][ 1 ],
                                actualDataG_->vertices[ i ][ j ][ 2 ]));
    }
    p.drawEnd();
}
