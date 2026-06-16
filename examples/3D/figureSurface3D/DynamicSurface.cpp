#include "DynamicSurface.h"

#include <cmath>

#include "qwt3d_color.h"
#include "qwt3d_coordsys.h"
#include "qwt3d_types.h"

// ---- RippleFunction ----

RippleFunction::RippleFunction()
    : Qwt3D::Function()
{
    setMesh(DynamicSurfacePlot::GRID_SIZE, DynamicSurfacePlot::GRID_SIZE);
    setDomain(-DynamicSurfacePlot::DOMAIN_EXTENT, DynamicSurfacePlot::DOMAIN_EXTENT,
              -DynamicSurfacePlot::DOMAIN_EXTENT, DynamicSurfacePlot::DOMAIN_EXTENT);
}

double RippleFunction::operator()(double x, double y)
{
    return std::sin(2.0 * m_time + std::sqrt(x * x + y * y));
}

// ---- DynamicSurfacePlot ----

DynamicSurfacePlot::DynamicSurfacePlot(QWidget* parent)
    : Qwt3D::SurfacePlot(parent)
    , m_ripple()
{
    setTitle("Dynamic Ripple Surface: z = sin(2t + sqrt(x^2 + y^2))");
    setTitleFont("Arial", 12, QFont::Bold);
    setTitleColor(Qwt3D::RGBA(0.1, 0.1, 0.1, 1.0));

    // Configure coordinate system
    setupAxes();
    setCoordinateStyle(Qwt3D::BOX);

    // Configure data color (StandardColor for z-value-driven coloring)
    auto* stdColor = new Qwt3D::StandardColor(this);
    setDataColor(stdColor);

    // Configure lighting
    enableLighting(true);
    illuminate(0);
    setLightComponent(GL_DIFFUSE, 0.7, 0);
    setLightComponent(GL_SPECULAR, 0.3, 0);
    setShininess(10.0);
    setLightRotation(20.0, 30.0, 0.0, 0);

    // Configure plot style
    setPlotStyle(Qwt3D::FILLED);
    setShading(Qwt3D::GOURAUD);
    setFloorStyle(Qwt3D::FLOORISO);
    setPolygonOffset(0.8);
    setMeshColor(Qwt3D::RGBA(0.0, 0.0, 0.0, 0.15));
    setMeshLineWidth(0.5);
    setBackgroundColor(Qwt3D::RGBA(1.0, 1.0, 1.0, 1.0));

    // Enable mouse and keyboard interaction
    enableMouse(true);
    enableKeyboard(true);

    // Create the initial surface data
    m_ripple.create(*this);

    // Set initial view
    resetView();

    // Show color legend
    showColorLegend(true);
}

void DynamicSurfacePlot::setupAxes()
{
    for (unsigned i = 0; i < coordinates()->axes.size(); ++i) {
        coordinates()->axes[i].setMajors(5);
        coordinates()->axes[i].setMinors(4);
    }

    coordinates()->axes[Qwt3D::X1].setLabelString("X");
    coordinates()->axes[Qwt3D::Y1].setLabelString("Y");
    coordinates()->axes[Qwt3D::Z1].setLabelString("Z");
}

void DynamicSurfacePlot::advanceTime(double dt)
{
    m_ripple.setTime(m_ripple.time() + dt);
    m_ripple.create(*this);
    updateData();
    update();
    Q_EMIT timeChanged(m_ripple.time());
}

void DynamicSurfacePlot::resetView()
{
    setRotation(30, 0, 15);
    setViewportShift(0.05, 0);
    setScale(1, 1, 1);
    setZoom(0.9);
}

void DynamicSurfacePlot::crossSectionY0(QVector<double>& xData, QVector<double>& zData) const
{
    const int n = GRID_SIZE;
    const double ext = DOMAIN_EXTENT;
    const double step = 2.0 * ext / (n - 1);
    const double t = m_ripple.time();

    xData.resize(n);
    zData.resize(n);

    for (int i = 0; i < n; ++i) {
        double x = -ext + i * step;
        xData[i] = x;
        zData[i] = std::sin(2.0 * t + std::sqrt(x * x)); // y=0
    }
}

double DynamicSurfacePlot::zAtOrigin() const
{
    return std::sin(2.0 * m_ripple.time());
}

double DynamicSurfacePlot::time() const
{
    return m_ripple.time();
}
