#ifndef qwt3d_enrichment_h
#define qwt3d_enrichment_h

#include "qwt3d_global.h"
#include "qwt3d_types.h"

namespace Qwt3D {

class Plot3D;

/**
 * \if ENGLISH
 * @brief Abstract base class for data dependent visible user objects
 * @details Enrichments provide a framework for user defined OpenGL objects. The base class has a pure virtual
 *          function clone(). 2 additional functions are per default empty and could also get a new
 *          implementation in derived classes. They can be used for initialization issues or actions not
 *          depending on the related primitive.
 * \endif
 *
 * \if CHINESE
 * @brief 数据依赖可见用户对象的抽象基类
 * @details Enrichment 提供用户自定义 OpenGL 对象的框架。基类有一个纯虚函数 clone()。
 *          另外两个函数默认为空，可以在派生类中获得新的实现。
 *          它们可用于初始化或不依赖于相关图元的操作。
 * \endif
 */
class QWT3D_EXPORT Enrichment
{
public:
    // Type of the Enrichment - only VERTEXENRICHMENT's are defined at this moment
    enum TYPE {
        VERTEXENRICHMENT,
        EDGEENRICHMENT,
        FACEENRICHMENT,
        VOXELENRICHMENT
    };

    Enrichment() : plot(0) { }
    virtual ~Enrichment() { }
    // The derived class should give back a new Derived(something) here
    virtual Enrichment *clone() const = 0;
    // Empty per default. Can be overwritten
    virtual void drawBegin() {};
    // Empty per default. Can be overwritten
    virtual void drawEnd() {};
    // Assign to existent plot
    virtual void assign(Plot3D const &pl) { plot = &pl; }
    // Overwrite
    virtual TYPE type() const = 0;

protected:
    const Plot3D *plot;
};

/**
 * \if ENGLISH
 * @brief Abstract base class for vertex dependent visible user objects
 * @details VertexEnrichments introduce a specialized draw routine for vertex dependent data.
 *          draw() is called, when the Plot realizes its internal OpenGL data representation
 *          for every Vertex associated to his argument.
 * \endif
 *
 * \if CHINESE
 * @brief 顶点依赖可见用户对象的抽象基类
 * @details VertexEnrichment 为顶点依赖数据引入了专用的绘制流程。
 *          当 Plot 实现其内部 OpenGL 数据表示时，draw() 会为与其参数关联的
 *          每个顶点被调用。
 * \endif
 */
class QWT3D_EXPORT VertexEnrichment : public Enrichment
{
public:
    VertexEnrichment() : Qwt3D::Enrichment() { }
    // The derived class should give back a new Derived(something) here
    virtual Enrichment *clone() const = 0;
    // Overwrite this
    virtual void draw(Qwt3D::Triple const &) = 0;
    // This gives VERTEXENRICHMENT
    virtual TYPE type() const
    {
        return Qwt3D::Enrichment::VERTEXENRICHMENT;
    }
};

} // ns

#endif