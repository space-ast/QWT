#ifndef qwt3d_volumeplot_h__2004_03_06_01_52_begin_guarded_code
#define qwt3d_volumeplot_h__2004_03_06_01_52_begin_guarded_code

#include "qwt3d_plot.h"

namespace Qwt3D
{

/**
 * @brief Volume plot widget (TODO: not yet fully implemented)
 * @details VolumePlot is intended to provide 3D volume visualization capabilities.
 *          This class is a placeholder for future implementation.
 */
class QWT3D_EXPORT VolumePlot : public Plot3D
{
    //    Q_OBJECT

public:
    // Constructor
    VolumePlot(QWidget* parent = 0, const char* name = 0)
    {
    }

protected:
    virtual void createData() = 0;
};

}  // ns

#endif