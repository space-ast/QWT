#include "qwt3d_color.h"
#include "qwt3d_plot.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Constructs a StandardColor object
 * @param[in] data Plot3D data source for color mapping
 * @param[in] size Number of color entries in the color vector
 * @details Creates a standard color mapping with the specified size and resets
 *          the color vector to default gradient values.
 * \endif
 *
 * \if CHINESE
 * @brief 构造 StandardColor 对象
 * @param[in] data 用于颜色映射的 Plot3D 数据源
 * @param[in] size 颜色向量中的颜色条目数
 * @details 创建指定大小的标准颜色映射，并将颜色向量重置为默认渐变值。
 * \endif
 */
StandardColor::StandardColor(Plot3D* data, unsigned size) : data_(data)
{
    Q_ASSERT(data_);

    reset(size);
}

/**
 * \if ENGLISH
 * @brief Resets the color vector to a default gradient
 * @param[in] size Number of color entries to generate
 * @details Creates a color vector of the given size with a blue-to-red gradient.
 * \endif
 *
 * \if CHINESE
 * @brief 将颜色向量重置为默认渐变
 * @param[in] size 要生成的颜色条目数
 * @details 创建给定大小的颜色向量，使用蓝到红的渐变。
 * \endif
 */
void StandardColor::reset(unsigned size)
{
    colors_ = ColorVector(size);
    RGBA elem;

    double dsize = size;

    for (unsigned int i = 0; i != size; ++i) {
        elem.r       = i / dsize;
        elem.g       = i / dsize / 4;
        elem.b       = 1 - i / dsize;
        elem.a       = 1.0;
        colors_[ i ] = elem;
    }
}

/**
 * \if ENGLISH
 * @brief Assigns a new ColorVector
 * @param[in] cv The new color vector (also overwrites the constructor's size argument)
 * \endif
 *
 * \if CHINESE
 * @brief 分配新的颜色向量
 * @param[in] cv 新的颜色向量（也会覆盖构造函数的大小参数）
 * \endif
 */
void StandardColor::setColorVector(ColorVector const& cv)
{
    colors_ = cv;
}

/**
 * \if ENGLISH
 * @brief Sets the alpha value for all colors
 * @param[in] a Alpha value (0.0 to 1.0)
 * \endif
 *
 * \if CHINESE
 * @brief 设置所有颜色的透明度值
 * @param[in] a 透明度值（0.0 到 1.0）
 * \endif
 */
void StandardColor::setAlpha(double a)
{
    if (a < 0 || a > 1)
        return;

    RGBA elem;

    for (unsigned int i = 0; i != colors_.size(); ++i) {
        elem         = colors_[ i ];
        elem.a       = a;
        colors_[ i ] = elem;
    }
}

/**
 * \if ENGLISH
 * @brief Returns the color for a given z value
 * @param[in] z The z coordinate value for color lookup
 * @return RGBA color corresponding to the z value
 * @details Maps the z value to a color index based on the data hull's z range.
 * \endif
 *
 * \if CHINESE
 * @brief 返回给定 z 值对应的颜色
 * @param[in] z 用于颜色查找的 z 坐标值
 * @return 与 z 值对应的 RGBA 颜色
 * @details 根据数据外壳的 z 范围将 z 值映射到颜色索引。
 * \endif
 */
RGBA StandardColor::operator()(double, double, double z) const
{
    Q_ASSERT(data_);
    int index = static_cast< int >((colors_.size() - 1) * (z - data_->hull().minVertex.z)
                                   / (data_->hull().maxVertex.z - data_->hull().minVertex.z));
    if (index < 0)
        index = 0;
    if (static_cast< unsigned int >(index) > colors_.size() - 1)
        index = static_cast< int >(colors_.size() - 1);
    return colors_[ index ];
}