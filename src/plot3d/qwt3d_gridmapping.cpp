#include "qwt3d_gridmapping.h"
#include "qwt3d_surfaceplot.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Default constructor
 * @details Initializes with no plot widget, zero mesh dimensions, zero domain,
 *          and unrestricted z range.
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数
 * @details 初始化时无绘图控件、零网格尺寸、零域范围，
 *          以及不受限制的 z 范围。
 * \endif
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
 * \if ENGLISH
 * @brief Sets the number of mesh columns and rows
 * @param[in] columns Number of columns (u direction)
 * @param[in] rows Number of rows (v direction)
 * \endif
 *
 * \if CHINESE
 * @brief 设置网格的列数和行数
 * @param[in] columns 列数（u 方向）
 * @param[in] rows 行数（v 方向）
 * \endif
 */
void GridMapping::setMesh(unsigned int columns, unsigned int rows)
{
    umesh_p = columns;
    vmesh_p = rows;
}

/**
 * \if ENGLISH
 * @brief Sets the domain (parameter range) for the u and v directions
 * @param[in] minu Minimum u value
 * @param[in] maxu Maximum u value
 * @param[in] minv Minimum v value
 * @param[in] maxv Maximum v value
 * \endif
 *
 * \if CHINESE
 * @brief 设置 u 和 v 方向的域（参数范围）
 * @param[in] minu u 方向最小值
 * @param[in] maxu u 方向最大值
 * @param[in] minv v 方向最小值
 * @param[in] maxv v 方向最大值
 * \endif
 */
void GridMapping::setDomain(double minu, double maxu, double minv, double maxv)
{
    minu_p = minu;
    maxu_p = maxu;
    minv_p = minv;
    maxv_p = maxv;
}

/**
 * \if ENGLISH
 * @brief Restricts the data range to a parallelepiped
 * @param[in] p The parallelepiped defining the restricted range
 * \endif
 *
 * \if CHINESE
 * @brief 将数据范围限制到平行六面体内
 * @param[in] p 定义限制范围的平行六面体
 * \endif
 */
void GridMapping::restrictRange(Qwt3D::ParallelEpiped const &p)
{
    range_p = p;
}