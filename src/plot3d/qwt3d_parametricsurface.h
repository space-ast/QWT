#ifndef qwt3d_parametricsurface_h
#define qwt3d_parametricsurface_h

#include "qwt3d_gridmapping.h"

namespace Qwt3D {

class SurfacePlot;

/**
 * \if ENGLISH
 * @brief Abstract base class for parametric surfaces
 * \endif
 *
 * \if CHINESE
 * @brief 参数化表面的抽象基类
 * \endif
 */
class QWT3D_EXPORT ParametricSurface : public GridMapping
{

public:
    // Constructs ParametricSurface object w/o assigned SurfacePlot
    ParametricSurface();
    // Constructs ParametricSurface object and assigns a SurfacePlot
    explicit ParametricSurface(Qwt3D::SurfacePlot &plotWidget);
    // Constructs ParametricSurface object and assigns a SurfacePlot
    explicit ParametricSurface(Qwt3D::SurfacePlot *plotWidget);
    // Overwrite this
    virtual Qwt3D::Triple operator()(double u, double v) = 0;
    // Assigns a new SurfacePlot and creates a data representation for it
    virtual bool create(Qwt3D::SurfacePlot &plotWidget);
    // Creates data representation for the actual assigned SurfacePlot
    virtual bool create();
    // Assigns the object to another widget - call before create()
    void assign(Qwt3D::SurfacePlot &plotWidget);
    // Assigns the object to another widget - call before create()
    void assign(Qwt3D::SurfacePlot *plotWidget);
    // Provide information about periodicity of the 'u' resp. 'v' domains
    void setPeriodic(bool u, bool v);

private:
    bool uperiodic_, vperiodic_;
};

} // ns

#endif /* include guarded */