#include "qwt3d_surfaceplot.h"
#include "qwt3d_function.h"

using namespace Qwt3D;

/**
 * @brief Default constructor
 */
Function::Function() : GridMapping()
{
}

/**
 * @brief Constructs a Function object and assigns a SurfacePlot
 * @param pw Reference to a SurfacePlot widget
 */
Function::Function(SurfacePlot& pw) : GridMapping()
{
    plotwidget_p = &pw;
}

/**
 * @brief Constructs a Function object and assigns a SurfacePlot
 * @param pw Pointer to a SurfacePlot widget
 */
Function::Function(SurfacePlot* pw) : GridMapping()
{
    plotwidget_p = pw;
}

/**
 * @brief Assigns the object to another widget - call before create()
 * @param plotWidget Reference to a SurfacePlot widget
 */
void Function::assign(SurfacePlot& plotWidget)
{
    if (&plotWidget != plotwidget_p)
        plotwidget_p = &plotWidget;
}

/**
 * @brief Assigns the object to another widget - call before create()
 * @param plotWidget Pointer to a SurfacePlot widget
 */
void Function::assign(SurfacePlot* plotWidget)
{
    if (plotWidget != plotwidget_p)
        plotwidget_p = plotWidget;
}

/**
 * @brief Sets minimum z value for the function
 * @param val Minimum z value
 */
void Function::setMinZ(double val)
{
    range_p.minVertex.z = val;
}

/**
 * @brief Sets maximum z value for the function
 * @param val Maximum z value
 */
void Function::setMaxZ(double val)
{
    range_p.maxVertex.z = val;
}

/**
 * @brief Creates data representation for the actual assigned SurfacePlot
 * @return True on success, false if mesh is too small or no widget assigned
 * @details Allocates data arrays, evaluates the function operator() over the
 *          mesh grid, clips values to the min/max z range, and loads data
 *          into the assigned SurfacePlot.
 */
bool Function::create()
{
    if ((umesh_p <= 2) || (vmesh_p <= 2) || !plotwidget_p)
        return false;

    /* allocate some space for the mesh */
    double** data = new double*[ umesh_p ];

    unsigned i, j;
    for (i = 0; i < umesh_p; i++) {
        data[ i ] = new double[ vmesh_p ];
    }

    /* get the data */

    double dx = (maxu_p - minu_p) / (umesh_p - 1);
    double dy = (maxv_p - minv_p) / (vmesh_p - 1);

    for (i = 0; i < umesh_p; ++i) {
        for (j = 0; j < vmesh_p; ++j) {
            data[ i ][ j ] = operator()(minu_p + i * dx, minv_p + j * dy);

            if (data[ i ][ j ] > range_p.maxVertex.z)
                data[ i ][ j ] = range_p.maxVertex.z;
            else if (data[ i ][ j ] < range_p.minVertex.z)
                data[ i ][ j ] = range_p.minVertex.z;
        }
    }

    Q_ASSERT(plotwidget_p);
    if (!plotwidget_p) {
        fprintf(stderr, "Function: no valid Plot3D Widget assigned");
    } else {
        static_cast< SurfacePlot* >(plotwidget_p)->loadFromData(data, umesh_p, vmesh_p, minu_p, maxu_p, minv_p, maxv_p);
    }

    for (i = 0; i < umesh_p; i++) {
        delete[] data[ i ];
    }

    delete[] data;

    return true;
}

/**
 * @brief Assigns a new SurfacePlot and creates a data representation for it
 * @param pl Reference to a SurfacePlot widget
 * @return True on success
 */
bool Function::create(SurfacePlot& pl)
{
    assign(pl);
    return create();
}
