#include "qwt3d_surfaceplot.h"
#include "qwt3d_function.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Default constructor
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数
 * \endif
 */
Function::Function() : GridMapping()
{
}

/**
 * \if ENGLISH
 * @brief Constructs a Function object and assigns a SurfacePlot
 * @param[in] pw Reference to a SurfacePlot widget
 * \endif
 *
 * \if CHINESE
 * @brief 构造 Function 对象并分配 SurfacePlot
 * @param[in] pw SurfacePlot 控件的引用
 * \endif
 */
Function::Function(SurfacePlot& pw) : GridMapping()
{
    plotwidget_p = &pw;
}

/**
 * \if ENGLISH
 * @brief Constructs a Function object and assigns a SurfacePlot
 * @param[in] pw Pointer to a SurfacePlot widget
 * \endif
 *
 * \if CHINESE
 * @brief 构造 Function 对象并分配 SurfacePlot
 * @param[in] pw SurfacePlot 控件的指针
 * \endif
 */
Function::Function(SurfacePlot* pw) : GridMapping()
{
    plotwidget_p = pw;
}

/**
 * \if ENGLISH
 * @brief Assigns the object to another widget - call before create()
 * @param[in] plotWidget Reference to a SurfacePlot widget
 * \endif
 *
 * \if CHINESE
 * @brief 将对象分配到另一个控件 - 在 create() 之前调用
 * @param[in] plotWidget SurfacePlot 控件的引用
 * \endif
 */
void Function::assign(SurfacePlot& plotWidget)
{
    if (&plotWidget != plotwidget_p)
        plotwidget_p = &plotWidget;
}

/**
 * \if ENGLISH
 * @brief Assigns the object to another widget - call before create()
 * @param[in] plotWidget Pointer to a SurfacePlot widget
 * \endif
 *
 * \if CHINESE
 * @brief 将对象分配到另一个控件 - 在 create() 之前调用
 * @param[in] plotWidget SurfacePlot 控件的指针
 * \endif
 */
void Function::assign(SurfacePlot* plotWidget)
{
    if (plotWidget != plotwidget_p)
        plotwidget_p = plotWidget;
}

/**
 * \if ENGLISH
 * @brief Sets minimum z value for the function
 * @param[in] val Minimum z value
 * \endif
 *
 * \if CHINESE
 * @brief 设置函数的最小 z 值
 * @param[in] val 最小 z 值
 * \endif
 */
void Function::setMinZ(double val)
{
    range_p.minVertex.z = val;
}

/**
 * \if ENGLISH
 * @brief Sets maximum z value for the function
 * @param[in] val Maximum z value
 * \endif
 *
 * \if CHINESE
 * @brief 设置函数的最大 z 值
 * @param[in] val 最大 z 值
 * \endif
 */
void Function::setMaxZ(double val)
{
    range_p.maxVertex.z = val;
}

/**
 * \if ENGLISH
 * @brief Creates data representation for the actual assigned SurfacePlot
 * @return True on success, false if mesh is too small or no widget assigned
 * @details Allocates data arrays, evaluates the function operator() over the
 *          mesh grid, clips values to the min/max z range, and loads data
 *          into the assigned SurfacePlot.
 * \endif
 *
 * \if CHINESE
 * @brief 为实际分配的 SurfacePlot 创建数据表示
 * @return 成功时返回 true，网格太小或未分配控件时返回 false
 * @details 分配数据数组，在网格上求值函数 operator()，
 *          将值裁剪到 min/max z 范围，并将数据加载到分配的 SurfacePlot 中。
 * \endif
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
 * \if ENGLISH
 * @brief Assigns a new SurfacePlot and creates a data representation for it
 * @param[in] pl Reference to a SurfacePlot widget
 * @return True on success
 * \endif
 *
 * \if CHINESE
 * @brief 分配新的 SurfacePlot 并为其创建数据表示
 * @param[in] pl SurfacePlot 控件的引用
 * @return 成功时返回 true
 * \endif
 */
bool Function::create(SurfacePlot& pl)
{
    assign(pl);
    return create();
}
