#ifndef __qwt3d_autoscaler__
#define __qwt3d_autoscaler__

#include <vector>
#include "qwt3d_global.h"
#include "qwt3d_autoptr.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Abstract base class for autoscaler
 * \endif
 *
 * \if CHINESE
 * @brief 自动缩放器的抽象基类
 * \endif
 */
class QWT3D_EXPORT AutoScaler
{
    friend class qwt3d_ptr<AutoScaler>;

protected:
    virtual AutoScaler *clone() const = 0;
    virtual int execute(double &a, double &b, double start, double stop, int ivals) = 0;
    virtual ~AutoScaler() { }

private:
    void destroy() const { delete this; }
};

/**
 * \if ENGLISH
 * @brief Automatic beautifying of linear scales
 * \endif
 *
 * \if CHINESE
 * @brief 线性刻度的自动美化缩放器
 * \endif
 */
class QWT3D_EXPORT LinearAutoScaler : public AutoScaler
{
    friend class LinearScale;

protected:
    LinearAutoScaler();
    explicit LinearAutoScaler(std::vector<double> &mantisses);
    AutoScaler *clone() const { return new LinearAutoScaler(*this); }
    int execute(double &a, double &b, double start, double stop, int ivals);

private:
    double start_, stop_;
    int intervals_;

    void init(double start, double stop, int ivals);
    double anchorvalue(double start, double mantisse, int exponent);
    int segments(int &l_intervals, int &r_intervals, double start, double stop, double anchor,
                 double mantissa, int exponent);
    std::vector<double> mantissi_;
};

} // ns

#endif