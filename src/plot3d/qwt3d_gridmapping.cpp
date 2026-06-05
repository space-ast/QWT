#include "qwt3d_gridmapping.h"
#include "qwt3d_surfaceplot.h"

using namespace Qwt3D;

/**
 * @brief Default constructor
 * @details Initializes with no plot widget, zero mesh dimensions, zero domain,
 *          and unrestricted z range.
 */
GridMapping::GridMapping()
{
    plotwidget_p = 0;
    setMesh(0, 0);
    setDomain(0, 0, 0, 0);
    restrictRange(ParallelEpiped(Triple(-DBL_MAX, -DBL_MAX, -DBL_MAX),
                                 Triple(DBL_MAX, DBL_MAX, DBL_MAX)));
}

/**
 * @brief Sets the number of mesh columns and rows
 * @param columns Number of columns (u direction)
 * @param rows Number of rows (v direction)
 */
void GridMapping::setMesh(unsigned int columns, unsigned int rows)
{
    umesh_p = columns;
    vmesh_p = rows;
}

/**
 * @brief Sets the domain (parameter range) for the u and v directions
 * @param minu Minimum u value
 * @param maxu Maximum u value
 * @param minv Minimum v value
 * @param maxv Maximum v value
 */
void GridMapping::setDomain(double minu, double maxu, double minv, double maxv)
{
    minu_p = minu;
    maxu_p = maxu;
    minv_p = minv;
    maxv_p = maxv;
}

/**
 * @brief Restricts the data range to a parallelepiped
 * @param p The parallelepiped defining the restricted range
 */
void GridMapping::restrictRange(Qwt3D::ParallelEpiped const &p)
{
    range_p = p;
}