#ifndef qwt3d_multiplot_h
#define qwt3d_multiplot_h

#include "qwt3d_plot.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Multi-plot widget for combining multiple plot areas
 * @details MultiPlot is an intermediate class between Plot3D and concrete multi-plot
 *          implementations, allowing multiple plot areas in a single widget.
 * \endif
 *
 * \if CHINESE
 * @brief 组合多个绘图区域的多绘图控件
 * @details MultiPlot 是 Plot3D 和具体多绘图实现之间的中间类，
 *          允许在单个控件中包含多个绘图区域。
 * \endif
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