#ifndef __COLORGENERATOR_H__
#define __COLORGENERATOR_H__

#include <qstring.h>
#include "qwt3d_global.h"
#include "qwt3d_types.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Abstract base class for color functors
 * @details Use your own color model by providing an implementation of
 *          operator()(double x, double y, double z). Colors destructor has been
 *          declared protected, in order to use only heap based objects. Plot3D
 *          will handle the objects destruction. See StandardColor for an example.
 * \endif
 *
 * \if CHINESE
 * @brief 颜色函数的抽象基类
 * @details 通过提供 operator()(double x, double y, double z) 的实现来使用自定义颜色模型。
 *          Color 的析构函数声明为 protected，以便仅使用堆分配的对象。
 *          Plot3D 将负责对象的销毁。参见 StandardColor 作为示例。
 * \endif
 */
class QWT3D_EXPORT Color
{
public:
    // Implement your color model here
    virtual Qwt3D::RGBA operator()(double x, double y,
                                   double z) const = 0;
    virtual Qwt3D::RGBA operator()(Qwt3D::Triple const &t) const
    {
        return this->operator()(t.x, t.y, t.z);
    }
    // Should create a color vector usable by ColorLegend. The default implementation returns its argument
    virtual Qwt3D::ColorVector &createVector(Qwt3D::ColorVector &vec) { return vec; }

    void destroy() const { delete this; }

protected:
    virtual ~Color() { }
};

class Plot3D;

/**
 * \if ENGLISH
 * @brief Standard color model for Plot3D - implements the data driven operator()(double x, double y, double z)
 * @details The class has a ColorVector representing z values, which will be used by
 *          operator()(double x, double y, double z)
 * \endif
 *
 * \if CHINESE
 * @brief Plot3D 的标准颜色模型 - 实现数据驱动的 operator()(double x, double y, double z)
 * @details 该类有一个表示 z 值的 ColorVector，将由 operator()(double x, double y, double z) 使用。
 * \endif
 */
class QWT3D_EXPORT StandardColor : public Color
{
public:
    // Initializes with data and set up a ColorVector with a size of 100 z values (default)
    explicit StandardColor(Qwt3D::Plot3D *data, unsigned size = 100);
    // Receives z-dependent color from ColorVector
    Qwt3D::RGBA operator()(double x, double y,
                           double z) const;
    void setColorVector(Qwt3D::ColorVector const &cv);
    // Resets the standard colors
    void reset(unsigned size = 100);
    // Sets unitary alpha value for all colors
    void setAlpha(double a);
    // Creates color vector for ColorLegend - essentially a copy from the internal vector
    Qwt3D::ColorVector &createVector(Qwt3D::ColorVector &vec)
    {
        vec = colors_;
        return vec;
    }

protected:
    Qwt3D::ColorVector colors_;
    Qwt3D::Plot3D *data_;
};

} // ns

#endif