#ifndef qwt3d_graphplot_h
#define qwt3d_graphplot_h

#include "qwt3d_plot.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Base class for graph plotting widgets
 * @details GraphPlot is an intermediate class between Plot3D and concrete graph plot types.
 *          It provides a common base for different graph plotting implementations.
 * \endif
 *
 * \if CHINESE
 * @brief 图形绘图控件的基类
 * @details GraphPlot 是 Plot3D 和具体图形绘图类型之间的中间类，
 *          为不同的图形绘图实现提供公共基类。
 * \endif
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