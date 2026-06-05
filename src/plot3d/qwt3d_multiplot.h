#ifndef qwt3d_multiplot_h
#define qwt3d_multiplot_h

#include "qwt3d_plot.h"

namespace Qwt3D {

/**
 * @brief Multi-plot widget for combining multiple plot areas
 * @details MultiPlot is an intermediate class between Plot3D and concrete multi-plot
 *          implementations, allowing multiple plot areas in a single widget.
 */
class QWT3D_EXPORT MultiPlot : public Plot3D
{
    //    Q_OBJECT

public:
    // Constructor
    MultiPlot(QWidget *parent = 0, const char *name = 0) { }

protected:
    virtual void createData() = 0;
};

} // ns

#endif