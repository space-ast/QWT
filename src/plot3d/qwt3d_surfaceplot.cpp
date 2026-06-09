#include "qwt3d_surfaceplot.h"

using namespace std;
using namespace Qwt3D;

/**
 * @brief Constructs a SurfacePlot widget
 * @param[in] parent Parent widget
 * @details Initializes with dataNormals()==false, NOFLOOR, resolution() == 1
 *
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
 * @brief Destructor
 *
 */
SurfacePlot::~SurfacePlot()
{
    delete actualDataG_;
    delete actualDataC_;
}

/**
 * @brief Shows or hides data normals
 * @param[in] b True to show normals, false to hide
 *
 */
void SurfacePlot::showNormals(bool b)
{
    datanormals_p = b;
}

/**
 * @brief Sets the normal vector length
 * @param[in] val Normal length value (0.0 to 1.0). Values < 0 or > 1 are ignored.
 *
 */
void SurfacePlot::setNormalLength(double val)
{
    if (val < 0 || val > 1)
        return;
    normalLength_p = val;
}

/**
 * @brief Sets the normal vector quality (number of arrow segments)
 * @param[in] val Quality value. Values < 3 are ignored.
 *
 */
void SurfacePlot::setNormalQuality(int val)
{
    if (val < 3)
        return;
    normalQuality_p = val;
}

/**
 * @brief Calculates the smallest x-y-z parallelepiped enclosing the data
 * @details It can be accessed by hull();
 *
 */
void SurfacePlot::calculateHull()
{
    if (actualData_p->empty())
        return;
    setHull(actualData_p->hull());
}

/**
 * @brief Sets data resolution and updates widget
 * @param[in] res Resolution multiplier (res == 1 means original resolution). If res < 1, the function does nothing.
 *
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
 * @brief Returns the number of facets in the data
 * @return (columns,rows) for grid data, (number of cells,1) for polygon data, (0,0) otherwise
 * @details The returned value is not affected by resolution().
 *
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
