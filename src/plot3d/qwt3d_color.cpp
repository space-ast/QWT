#include "qwt3d_color.h"
#include "qwt3d_plot.h"

using namespace Qwt3D;

/**
 * @brief Constructs a StandardColor object
 * @param data Plot3D data source for color mapping
 * @param size Number of color entries in the color vector
 * @details Creates a standard color mapping with the specified size and resets
 *          the color vector to default gradient values.
 */
StandardColor::StandardColor(Plot3D* data, unsigned size) : data_(data)
{
    Q_ASSERT(data_);

    reset(size);
}

/**
 * @brief Resets the color vector to a default gradient
 * @param size Number of color entries to generate
 * @details Creates a color vector of the given size with a blue-to-red gradient.
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
 * @brief Assigns a new ColorVector
 * @param cv The new color vector (also overwrites the constructor's size argument)
 */
void StandardColor::setColorVector(ColorVector const& cv)
{
    colors_ = cv;
}

/**
 * @brief Sets the alpha value for all colors
 * @param a Alpha value (0.0 to 1.0)
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
 * @brief Returns the color for a given z value
 * @param z The z coordinate value for color lookup
 * @return RGBA color corresponding to the z value
 * @details Maps the z value to a color index based on the data hull's z range.
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