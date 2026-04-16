#ifndef __plot3d__
#define __plot3d__

#include <QOpenGLWidget>

#include "qwt3d_coordsys.h"
#include "qwt3d_enrichment_std.h"

namespace Qwt3D
{

/**
 * \if ENGLISH
 * @brief Base class for all plotting widgets
 * @details Plot3D handles all the common features for plotting widgets -
 *          coordinate system, transformations, mouse/keyboard handling,
 *          labeling etc. It contains some pure virtual functions and is,
 *          in so far, an abstract base class. The class provides interfaces
 *          for data handling and implements basic data controlled color allocation.
 * \endif
 *
 * \if CHINESE
 * @brief 所有绘图控件的基类
 * @details Plot3D 处理绘图控件的通用功能——坐标系、变换、
 *          鼠标/键盘处理、标注等。它包含一些纯虚函数，
 *          因此是一个抽象基类。该类提供数据处理的接口，
 *          并实现基本的数据控制颜色分配。
 * \endif
 */
class QWT3D_EXPORT Plot3D : public QOpenGLWidget
{
    Q_OBJECT

public:
    // Constructor
    Plot3D(QWidget* parent = 0);
    // Destructor
    virtual ~Plot3D();

    // Render to pixmap
    QPixmap renderPixmap(int w = 0, int h = 0, bool useContext = false);
    // Recalculate data
    void updateData();
    // Create coordinate system between two points
    void createCoordinateSystem(Qwt3D::Triple beg, Qwt3D::Triple end);
    // Returns pointer to CoordinateSystem object
    Qwt3D::CoordinateSystem* coordinates()
    {
        return &coordinates_p;
    }
    // Returns pointer to ColorLegend object
    Qwt3D::ColorLegend* legend()
    {
        return &legend_;
    }

    // Returns rotation around X axis [-360..360] (some angles are equivalent)
    double xRotation() const
    {
        return xRot_;
    }
    // Returns rotation around Y axis [-360..360] (some angles are equivalent)
    double yRotation() const
    {
        return yRot_;
    }
    // Returns rotation around Z axis [-360..360] (some angles are equivalent)
    double zRotation() const
    {
        return zRot_;
    }

    // Returns shift along X axis (object coordinates)
    double xShift() const
    {
        return xShift_;
    }
    // Returns shift along Y axis (object coordinates)
    double yShift() const
    {
        return yShift_;
    }
    // Returns shift along Z axis (object coordinates)
    double zShift() const
    {
        return zShift_;
    }

    // Returns relative shift [-1..1] along X axis (view coordinates)
    double xViewportShift() const
    {
        return xVPShift_;
    }
    // Returns relative shift [-1..1] along Y axis (view coordinates)
    double yViewportShift() const
    {
        return yVPShift_;
    }

    // Returns scaling for X values [0..inf]
    double xScale() const
    {
        return xScale_;
    }
    // Returns scaling for Y values [0..inf]
    double yScale() const
    {
        return yScale_;
    }
    // Returns scaling for Z values [0..inf]
    double zScale() const
    {
        return zScale_;
    }

    // Returns zoom (0..inf)
    double zoom() const
    {
        return zoom_;
    }

    // Returns orthogonal (true) or perspective (false) projection
    bool ortho() const
    {
        return ortho_;
    }
    // Set plot style
    void setPlotStyle(Qwt3D::PLOTSTYLE val);
    // Set plot style with Enrichment
    Qwt3D::Enrichment* setPlotStyle(Qwt3D::Enrichment const& val);
    // Returns plotting style
    Qwt3D::PLOTSTYLE plotStyle() const
    {
        return plotstyle_;
    }
    // Returns current Enrichment object used for plotting styles (if set, zero else)
    Qwt3D::Enrichment* userStyle() const
    {
        return userplotstyle_p;
    }
    // Set shading style
    void setShading(Qwt3D::SHADINGSTYLE val);
    // Returns shading style
    Qwt3D::SHADINGSTYLE shading() const
    {
        return shading_;
    }
    // Set number of isolines
    void setIsolines(int isolines);
    // Returns number of isolines
    int isolines() const
    {
        return isolines_;
    }

    // Enables/disables smooth data mesh lines. Default is false
    void setSmoothMesh(bool val)
    {
        smoothdatamesh_p = val;
    }
    // True if mesh antialiasing is on
    bool smoothDataMesh() const
    {
        return smoothdatamesh_p;
    }
    // Sets widgets background color
    void setBackgroundColor(Qwt3D::RGBA rgba);
    // Returns the widgets background color
    Qwt3D::RGBA backgroundRGBAColor() const
    {
        return bgcolor_;
    }
    // Sets color for data mesh
    void setMeshColor(Qwt3D::RGBA rgba);
    // Returns color for data mesh
    Qwt3D::RGBA meshColor() const
    {
        return meshcolor_;
    }
    // Sets line width for data mesh
    void setMeshLineWidth(double lw);
    // Returns line width for data mesh
    double meshLineWidth() const
    {
        return meshLineWidth_;
    }
    // Sets new data color object
    void setDataColor(Color* col);
    // Returns data color object
    const Color* dataColor() const
    {
        return datacolor_p;
    }

    // Add an Enrichment
    virtual Qwt3D::Enrichment* addEnrichment(Qwt3D::Enrichment const&);
    // Remove an Enrichment
    virtual bool degrade(Qwt3D::Enrichment*);

    // Returns rectangular hull
    Qwt3D::ParallelEpiped hull() const
    {
        return hull_;
    }

    // Show/hide color legend
    void showColorLegend(bool);

    // Sets style of coordinate system
    void setCoordinateStyle(Qwt3D::COORDSTYLE st);
    // Set polygon offset
    void setPolygonOffset(double d);
    // Returns relative value for polygon offset [0..1]
    double polygonOffset() const
    {
        return polygonOffset_;
    }

    // Set title position
    void setTitlePosition(double rely, double relx = 0.5, Qwt3D::ANCHOR = Qwt3D::TopCenter);
    // Set title font
    void setTitleFont(const QString& family, int pointSize, int weight = QFont::Normal, bool italic = false);
    // Set caption color
    void setTitleColor(Qwt3D::RGBA col)
    {
        title_.setColor(col);
    }
    // Set caption text (one row only)
    void setTitle(const QString& title)
    {
        title_.setString(title);
    }

    // Assign mouse states for rotations, scales, zoom and shifts
    void assignMouse(MouseState xrot,
                     MouseState yrot,
                     MouseState zrot,
                     MouseState xscale,
                     MouseState yscale,
                     MouseState zscale,
                     MouseState zoom,
                     MouseState xshift,
                     MouseState yshift);

    // Returns true, if the widget accept mouse input from the user
    bool mouseEnabled() const;
    // Assign keyboard states for rotations, scales, zoom and shifts
    void assignKeyboard(KeyboardState xrot_n,
                        KeyboardState xrot_p,
                        KeyboardState yrot_n,
                        KeyboardState yrot_p,
                        KeyboardState zrot_n,
                        KeyboardState zrot_p,
                        KeyboardState xscale_n,
                        KeyboardState xscale_p,
                        KeyboardState yscale_n,
                        KeyboardState yscale_p,
                        KeyboardState zscale_n,
                        KeyboardState zscale_p,
                        KeyboardState zoom_n,
                        KeyboardState zoom_p,
                        KeyboardState xshift_n,
                        KeyboardState xshift_p,
                        KeyboardState yshift_n,
                        KeyboardState yshift_p);

    // Returns true, if the widget accept keyboard input from the user
    bool keyboardEnabled() const;
    // Sets speed for keyboard driven transformations
    void setKeySpeed(double rot, double scale, double shift);
    // Gets speed for keyboard driven transformations
    void keySpeed(double& rot, double& scale, double& shift) const;

    // Returns true, if Lighting is enabled, false else
    bool lightingEnabled() const;
    // Turn light on
    void illuminate(unsigned light = 0);
    // Turn light off
    void blowout(unsigned light = 0);

    // Set material component (RGBA)
    void setMaterialComponent(GLenum property, double r, double g, double b, double a = 1.0);
    // Set material component (intensity)
    void setMaterialComponent(GLenum property, double intensity);
    // Set shininess exponent
    void setShininess(double exponent);
    // Set light component (RGBA)
    void setLightComponent(GLenum property, double r, double g, double b, double a = 1.0, unsigned light = 0);
    // Set light component (intensity)
    void setLightComponent(GLenum property, double intensity, unsigned light = 0);

    // Returns Light 'idx' rotation around X axis [-360..360] (some angles are equivalent)
    double xLightRotation(unsigned idx = 0) const
    {
        return (idx < 8) ? lights_[ idx ].rot.x : 0;
    }
    // Returns Light 'idx' rotation around Y axis [-360..360] (some angles are equivalent)
    double yLightRotation(unsigned idx = 0) const
    {
        return (idx < 8) ? lights_[ idx ].rot.y : 0;
    }
    // Returns Light 'idx' rotation around Z axis [-360..360] (some angles are equivalent)
    double zLightRotation(unsigned idx = 0) const
    {
        return (idx < 8) ? lights_[ idx ].rot.z : 0;
    }

    // Returns shift of Light 'idx' along X axis (object coordinates)
    double xLightShift(unsigned idx = 0) const
    {
        return (idx < 8) ? lights_[ idx ].shift.x : 0;
    }
    // Returns shift of Light 'idx' along Y axis (object coordinates)
    double yLightShift(unsigned idx = 0) const
    {
        return (idx < 8) ? lights_[ idx ].shift.y : 0;
    }
    // Returns shift of Light 'idx' along Z axis (object coordinates)
    double zLightShift(unsigned idx = 0) const
    {
        return (idx < 8) ? lights_[ idx ].shift.z : 0;
    }
    // Returns true if valid data available, false else
    bool hasData() const
    {
        return (actualData_p) ? !actualData_p->empty() : false;
    }

Q_SIGNALS:

    /**
     * \if ENGLISH
     * @brief Signal emitted when the rotation is changed
     * @param xAngle X axis rotation angle
     * @param yAngle Y axis rotation angle
     * @param zAngle Z axis rotation angle
     * \endif
     *
     * \if CHINESE
     * @brief 旋转角度变化时发出的信号
     * @param xAngle X轴旋转角度
     * @param yAngle Y轴旋转角度
     * @param zAngle Z轴旋转角度
     * \endif
     */
    void rotationChanged(double xAngle, double yAngle, double zAngle);

    /**
     * \if ENGLISH
     * @brief Signal emitted when the shift is changed
     * @param xShift X axis shift value
     * @param yShift Y axis shift value
     * @param zShift Z axis shift value
     * \endif
     *
     * \if CHINESE
     * @brief 位移变化时发出的信号
     * @param xShift X轴位移值
     * @param yShift Y轴位移值
     * @param zShift Z轴位移值
     * \endif
     */
    void shiftChanged(double xShift, double yShift, double zShift);

    /**
     * \if ENGLISH
     * @brief Signal emitted when the viewport shift is changed
     * @param xShift X viewport shift value
     * @param yShift Y viewport shift value
     * \endif
     *
     * \if CHINESE
     * @brief 视口位移变化时发出的信号
     * @param xShift X视口位移值
     * @param yShift Y视口位移值
     * \endif
     */
    void vieportShiftChanged(double xShift, double yShift);

    /**
     * \if ENGLISH
     * @brief Signal emitted when the scaling is changed
     * @param xScale X axis scale factor
     * @param yScale Y axis scale factor
     * @param zScale Z axis scale factor
     * \endif
     *
     * \if CHINESE
     * @brief 缩放比例变化时发出的信号
     * @param xScale X轴缩放因子
     * @param yScale Y轴缩放因子
     * @param zScale Z轴缩放因子
     * \endif
     */
    void scaleChanged(double xScale, double yScale, double zScale);

    /**
     * \if ENGLISH
     * @brief Signal emitted when the zoom is changed
     * @param zoom Zoom factor
     * \endif
     *
     * \if CHINESE
     * @brief 缩放因子变化时发出的信号
     * @param zoom 缩放因子
     * \endif
     */
    void zoomChanged(double);

    /**
     * \if ENGLISH
     * @brief Signal emitted when the projection mode is changed
     * @param ortho True for orthogonal, false for perspective
     * \endif
     *
     * \if CHINESE
     * @brief 投影模式变化时发出的信号
     * @param ortho 正交投影为true，透视投影为false
     * \endif
     */
    void projectionChanged(bool);

public Q_SLOTS:

    // Set rotation values
    void setRotation(double xVal, double yVal, double zVal);
    // Set shift values
    void setShift(double xVal, double yVal, double zVal);
    // Set viewport shift values
    void setViewportShift(double xVal, double yVal);
    // Set scale values
    void setScale(double xVal, double yVal, double zVal);
    // Set zoom value
    void setZoom(double);

    // Set orthogonal/perspective projection
    void setOrtho(bool);

    // Enable mouse input
    void enableMouse(bool val = true);
    // Disable mouse input
    void disableMouse(bool val = true);
    // Enable keyboard input
    void enableKeyboard(bool val = true);
    // Disable keyboard input
    void disableKeyboard(bool val = true);

    // Turn Lighting on or off
    void enableLighting(bool val = true);
    // Turn Lighting on or off
    void disableLighting(bool val = true);

    // Set light rotation
    void setLightRotation(double xVal, double yVal, double zVal, unsigned int idx = 0);
    // Set light shift
    void setLightShift(double xVal, double yVal, double zVal, unsigned int idx = 0);

    // Saves content to pixmap format
    virtual bool savePixmap(QString const& fileName,
                            QString const& format);
    // Saves content to vector format
    virtual bool saveVector(QString const& fileName,
                            QString const& format,
                            VectorWriter::TEXTMODE text,
                            VectorWriter::SORTMODE sortmode);
    // Saves content
    virtual bool save(QString const& fileName, QString const& format);

protected:
    using EnrichmentList = std::list< Qwt3D::Enrichment* >;
    using ELIT           = EnrichmentList::iterator;

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void keyPressEvent(QKeyEvent* e);

    Qwt3D::CoordinateSystem coordinates_p;
    Qwt3D::Color* datacolor_p;
    Qwt3D::Enrichment* userplotstyle_p;
    EnrichmentList elist_p;

    virtual void calculateHull() = 0;
    virtual void createData()    = 0;
    virtual void createEnrichment(Qwt3D::Enrichment&)
    {
    }
    virtual void createEnrichments();

    void createCoordinateSystem();
    void setHull(Qwt3D::ParallelEpiped p)
    {
        hull_ = p;
    }

    bool initializedGL() const
    {
        return initializedGL_;
    }

    enum OBJECTS
    {
        DataObject,
        LegendObject,
        NormalObject,
        DisplayListSize  // only to have a vector length ...
    };
    std::vector< GLuint > displaylists_p;
    Qwt3D::Data* actualData_p;

private:
    struct Light
    {
        Light() : unlit(true)
        {
        }
        bool unlit;
        Qwt3D::Triple rot;
        Qwt3D::Triple shift;
    };
    std::vector< Light > lights_;

    GLdouble xRot_, yRot_, zRot_, xShift_, yShift_, zShift_, zoom_, xScale_, yScale_, zScale_, xVPShift_, yVPShift_;

    Qwt3D::RGBA meshcolor_;
    double meshLineWidth_;
    Qwt3D::RGBA bgcolor_;
    Qwt3D::PLOTSTYLE plotstyle_;
    Qwt3D::SHADINGSTYLE shading_;
    Qwt3D::FLOORSTYLE floorstyle_;
    bool ortho_;
    double polygonOffset_;
    int isolines_;
    bool displaylegend_;
    bool smoothdatamesh_p;

    Qwt3D::ParallelEpiped hull_;

    Qwt3D::ColorLegend legend_;

    Label title_;
    Qwt3D::Tuple titlerel_;
    Qwt3D::ANCHOR titleanchor_;

    // mouse

    QPoint lastMouseMovePosition_;
    bool mpressed_;

    MouseState xrot_mstate_, yrot_mstate_, zrot_mstate_, xscale_mstate_, yscale_mstate_, zscale_mstate_, zoom_mstate_,
        xshift_mstate_, yshift_mstate_;

    bool mouse_input_enabled_;

    void setRotationMouse(MouseState bstate, double accel, QPoint diff);
    void setScaleMouse(MouseState bstate, double accel, QPoint diff);
    void setShiftMouse(MouseState bstate, double accel, QPoint diff);

    // keyboard

    bool kpressed_;

    KeyboardState xrot_kstate_[ 2 ], yrot_kstate_[ 2 ], zrot_kstate_[ 2 ], xscale_kstate_[ 2 ], yscale_kstate_[ 2 ],
        zscale_kstate_[ 2 ], zoom_kstate_[ 2 ], xshift_kstate_[ 2 ], yshift_kstate_[ 2 ];

    bool kbd_input_enabled_;
    double kbd_rot_speed_, kbd_scale_speed_, kbd_shift_speed_;

    void setRotationKeyboard(KeyboardState kseq, double speed);
    void setScaleKeyboard(KeyboardState kseq, double speed);
    void setShiftKeyboard(KeyboardState kseq, double speed);

    bool lighting_enabled_;
    void applyLight(unsigned idx);
    void applyLights();

    bool initializedGL_;
    bool renderpixmaprequest_;
};

}  // ns

#endif