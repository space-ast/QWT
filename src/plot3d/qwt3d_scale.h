#ifndef qwt3d_scale_h
#define qwt3d_scale_h

#include <qstring.h>
#include "qwt3d_types.h"
#include "qwt3d_autoscaler.h"
#include "qwt3d_autoptr.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Non-visual scale class encapsulating tic generation
 * @details The class encapsulates non-visual scales. It is utilized by Axis and also
 *          collaborates closely with AutoScaler. A Scale allows control over all aspects
 *          of tic generation including arbitrary transformations of tic values into corresponding
 *          strings. The strings contain what eventually will be shown as tic labels.
 *          Standard linear and logarithmic scales have been integrated into the Axis interface.
 *          User-defined axes can be derived from Scale, LinearScale et al.
 * \endif
 *
 * \if CHINESE
 * @brief 非可视化刻度类，封装刻度线生成
 * @details 该类封装非可视化刻度。它由 Axis 使用，并与 AutoScaler 紧密协作。
 *          Scale 允许控制刻度线生成的所有方面，包括将刻度值任意转换为对应的字符串。
 *          字串最终将显示为刻度标签。标准线性和对数刻度已集成到 Axis 接口中。
 *          用户自定义坐标轴可以从 Scale、LinearScale 等派生。
 * \endif
 */
class QWT3D_EXPORT Scale
{
    friend class Axis;
    friend class qwt3d_ptr<Scale>;

protected:
    Scale();
    virtual ~Scale() { }
    virtual QString ticLabel(unsigned int idx) const;

    virtual void setLimits(double start, double stop);
    // Sets number of major intervals
    virtual void setMajors(int val) { majorintervals_p = val; }
    // Sets number of minor intervals per major interval
    virtual void setMinors(int val)
    {
        minorintervals_p = val;
    }
    virtual void setMajorLimits(double start, double stop);

    // Returns major intervals
    int majors() const { return majorintervals_p; }
    // Returns minor intervals
    int minors() const { return minorintervals_p; }

    // Derived classes should return a new heap based object here
    virtual Scale *clone() const = 0;
    // This function should setup the 2 vectors for major and minor positions
    virtual void calculate() = 0;
    virtual int autoscale(double &a, double &b, double start, double stop, int ivals);

    std::vector<double> majors_p, minors_p;
    double start_p, stop_p;
    int majorintervals_p, minorintervals_p;
    double mstart_p, mstop_p;

private:
    void destroy() const { delete this; }
};

/**
 * \if ENGLISH
 * @brief The standard (1:1) mapping class for axis numbering
 * \endif
 *
 * \if CHINESE
 * @brief 坐标轴编号的标准（1:1）映射类
 * \endif
 */
class QWT3D_EXPORT LinearScale : public Scale
{
    friend class Axis;
    friend class qwt3d_ptr<Scale>;

protected:
    int autoscale(double &a, double &b, double start, double stop, int ivals);
    // Returns a new heap based object utilized from qwt3d_ptr
    Scale *clone() const { return new LinearScale(*this); }
    void calculate();
    LinearAutoScaler autoscaler_p;
};

/**
 * \if ENGLISH
 * @brief Log10 scale
 * \endif
 *
 * \if CHINESE
 * @brief log10 对数刻度
 * \endif
 */
class QWT3D_EXPORT LogScale : public Scale
{
    friend class Axis;
    friend class qwt3d_ptr<Scale>;

protected:
    QString ticLabel(unsigned int idx) const;
    void setMinors(int val);
    // Standard ctor
    LogScale();
    // Returns a new heap based object utilized from qwt3d_ptr
    Scale *clone() const { return new LogScale; }
    void calculate();

private:
    void setupCounter(double &k, int &step);
};

} // namespace Qwt3D

#endif /* include guarded */