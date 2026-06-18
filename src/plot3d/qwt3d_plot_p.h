#ifndef QWT3D_PLOT_P_H
#define QWT3D_PLOT_P_H

#include "qwt3d_plot.h"
#include "qwt3d_portability.h"

#include <QPoint>

namespace Qwt3D
{

class Plot3D::PrivateData
{
    QWT_DECLARE_PUBLIC(Plot3D)

public:
    PrivateData(Plot3D* q);

    struct Light
    {
        Light() : unlit(true)
        {
        }
        bool unlit;
        Triple rot;
        Triple shift;
    };

    CoordinateSystem m_coordinates;
    Color* m_dataColor;
    Enrichment* m_userPlotStyle;
    std::list< Enrichment* > m_enrichmentList;
    std::vector< GLuint > m_displayLists;
    Data* m_actualData;

    std::vector< Light > m_lights;

    GLdouble m_xRot, m_yRot, m_zRot;
    GLdouble m_xShift, m_yShift, m_zShift;
    GLdouble m_zoom;
    GLdouble m_xScale, m_yScale, m_zScale;
    GLdouble m_xVPShift, m_yVPShift;

    RGBA m_meshColor;
    double m_meshLineWidth;
    RGBA m_bgColor;
    PLOTSTYLE m_plotStyle;
    SHADINGSTYLE m_shading;
    FLOORSTYLE m_floorStyle;
    bool m_ortho;
    double m_polygonOffset;
    int m_isolines;
    bool m_displayLegend;
    bool m_smoothDataMesh;

    ParallelEpiped m_hull;

    ColorLegend m_legend;

    Label m_title;
    Tuple m_titleRel;
    ANCHOR m_titleAnchor;

    QPoint m_lastMouseMovePosition;
    bool m_pressed;

    MouseState m_xrotMState, m_yrotMState, m_zrotMState;
    MouseState m_xscaleMState, m_yscaleMState, m_zscaleMState;
    MouseState m_zoomMState;
    MouseState m_xshiftMState, m_yshiftMState;

    bool m_mouseInputEnabled;

    KeyboardState m_xrotKState[ 2 ], m_yrotKState[ 2 ], m_zrotKState[ 2 ];
    KeyboardState m_xscaleKState[ 2 ], m_yscaleKState[ 2 ], m_zscaleKState[ 2 ];
    KeyboardState m_zoomKState[ 2 ];
    KeyboardState m_xshiftKState[ 2 ], m_yshiftKState[ 2 ];

    bool m_kPressed;
    bool m_kbdInputEnabled;
    double m_kbdRotSpeed, m_kbdScaleSpeed, m_kbdShiftSpeed;

    bool m_lightingEnabled;
    bool m_initializedGL;
    bool m_renderPixmapRequest;

    Qwt3DTheme m_theme;
};

}  // ns

#endif  // QWT3D_PLOT_P_H
