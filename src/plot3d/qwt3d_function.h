#ifndef qwt3d_function_h
#define qwt3d_function_h

#include "qwt3d_gridmapping.h"

namespace Qwt3D {

class SurfacePlot;

/**
 * \if ENGLISH
 * @brief Abstract base class for mathematical functions
 * @details A Function encapsulates a mathematical function with rectangular domain. The user has to
 *          adapt the pure virtual operator() to get a working object. Also, the client code should call
 *          setDomain, setMesh and create for reasonable operating conditions.
 * \endif
 *
 * \if CHINESE
 * @brief 数学函数的抽象基类
 * @details Function 封装了一个具有矩形域的数学函数。用户必须适配纯虚 operator()
 *          以获得可工作的对象。此外，客户端代码应调用 setDomain、setMesh 和 create
 *          以获得合理的操作条件。
 * \endif
 */
class QWT3D_EXPORT Function : public GridMapping
{

public:
    // Constructs Function object w/o assigned SurfacePlot
    Function();
    // Constructs Function object and assigns a SurfacePlot
    explicit Function(Qwt3D::SurfacePlot &plotWidget);
    // Constructs Function object and assigns a SurfacePlot
    explicit Function(Qwt3D::SurfacePlot *plotWidget);
    // Overwrite this
    virtual double operator()(double x, double y) = 0;

    // Sets minimal z value
    void setMinZ(double val);
    // Sets maximal z value
    void setMaxZ(double val);

    // Assigns a new SurfacePlot and creates a data representation for it
    virtual bool create(Qwt3D::SurfacePlot &plotWidget);
    // Creates data representation for the actual assigned SurfacePlot
    virtual bool create();
    // Assigns the object to another widget - call before create()
    void assign(Qwt3D::SurfacePlot &plotWidget);
    // Assigns the object to another widget - call before create()
    void assign(Qwt3D::SurfacePlot *plotWidget);
};

} // ns

#endif /* include guarded */