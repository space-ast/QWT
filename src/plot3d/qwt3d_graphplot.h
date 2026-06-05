#ifndef qwt3d_graphplot_h
#define qwt3d_graphplot_h

#include "qwt3d_plot.h"

namespace Qwt3D {

/**
 * @brief Base class for graph plotting widgets
 * @details GraphPlot is an intermediate class between Plot3D and concrete graph plot types.
 *          It provides a common base for different graph plotting implementations.
 */
class QWT3D_EXPORT GraphPlot : public Plot3D
{
    //    Q_OBJECT

public:
    // Constructor
    GraphPlot(QWidget *parent = 0, const char *name = 0);

protected:
    virtual void createData() = 0;
};

} // ns

#endif