#include "qwt3d_scale.h"

using namespace Qwt3D;

Scale::Scale()
    : start_p(0.), stop_p(0.), majorintervals_p(0), minorintervals_p(0), mstart_p(0.), mstop_p(0.)
{
}

/**
 * \if ENGLISH
 * @brief Maps the double value at tic-position idx to a final representation
 * @param[in] idx The current major tic index
 * @return The QString representation for the value corresponding to a valid index, an empty QString else.
 * @details The default return value is simply the tic values QString representation.
 *          Overwrite this function, if you plan to transform the value in some way.
 *          See e.g. LogScale::ticLabel.
 * \endif
 *
 * \if CHINESE
 * @brief 将刻度位置 idx 处的双精度值映射为最终表示
 * @param[in] idx 当前主刻度索引
 * @return 对应有效索引值的 QString 表示，否则为空 QString。
 * @details 默认返回值是刻度值的 QString 表示。
 *          如果需要以某种方式转换值，请重写此函数。
 *          参见 LogScale::ticLabel。
 * \endif
 */
QString Scale::ticLabel(unsigned int idx) const
{
    if (idx < majors_p.size()) {
        return QString::number(majors_p[idx]);
    }
    return QString("");
}

/**
 * \if ENGLISH
 * @brief Sets start and stop value for the scale
 * @param[in] start Scale start value
 * @param[in] stop Scale stop value
 * \endif
 *
 * \if CHINESE
 * @brief 设置刻度的起始值和结束值
 * @param[in] start 刻度起始值
 * @param[in] stop 刻度结束值
 * \endif
 */
void Scale::setLimits(double start, double stop)
{
    if (start < stop) {
        start_p = start;
        stop_p = stop;
        return;
    }
    start_p = stop;
    stop_p = start;
}

/**
 * \if ENGLISH
 * @brief Sets value of first and last major tic
 * @param[in] start First major tic value
 * @param[in] stop Last major tic value
 * \endif
 *
 * \if CHINESE
 * @brief 设置第一个和最后一个主刻度的值
 * @param[in] start 第一个主刻度值
 * @param[in] stop 最后一个主刻度值
 * \endif
 */
void Scale::setMajorLimits(double start, double stop)
{
    if (start < stop) {
        mstart_p = start;
        mstop_p = stop;
        return;
    }
    mstart_p = stop;
    mstop_p = start;
}

/**
 * \if ENGLISH
 * @brief Autoscales the axis
 * @param[out] a First major tic after applying autoscaling
 * @param[out] b Last major tic after applying autoscaling
 * @param[in] start Scale begin
 * @param[in] stop Scale end
 * @param[in] ivals Requested number of major intervals
 * @return Number of major intervals after autoscaling
 * @details The default implementation sets a=start, b=stop and returns ivals.
 * \endif
 *
 * \if CHINESE
 * @brief 自动缩放坐标轴
 * @param[out] a 应用自动缩放后的第一个主刻度
 * @param[out] b 应用自动缩放后的最后一个主刻度
 * @param[in] start 刻度起始
 * @param[in] stop 刻度结束
 * @param[in] ivals 请求的主区间数
 * @return 自动缩放后的主区间数
 * @details 默认实现设置 a=start, b=stop 并返回 ivals。
 * \endif
 */
int Scale::autoscale(double &a, double &b, double start, double stop, int ivals)
{
    a = start;
    b = stop;
    return ivals;
}

/***************************
 *
 * linear scales
 *
 ***************************/

/**
 * \if ENGLISH
 * @brief Applies LinearAutoScaler::execute() for autoscaling
 * @param[out] a First major tic after autoscaling
 * @param[out] b Last major tic after autoscaling
 * @param[in] start Scale begin
 * @param[in] stop Scale end
 * @param[in] ivals Requested number of major intervals
 * @return Number of major intervals after autoscaling
 * \endif
 *
 * \if CHINESE
 * @brief 应用 LinearAutoScaler::execute() 进行自动缩放
 * @param[out] a 自动缩放后的第一个主刻度
 * @param[out] b 自动缩放后的最后一个主刻度
 * @param[in] start 刻度起始
 * @param[in] stop 刻度结束
 * @param[in] ivals 请求的主区间数
 * @return 自动缩放后的主区间数
 * \endif
 */
int LinearScale::autoscale(double &a, double &b, double start, double stop, int ivals)
{
    return autoscaler_p.execute(a, b, start, stop, ivals);
}

/**
 * \if ENGLISH
 * @brief Creates the major and minor vector for the scale
 * \endif
 *
 * \if CHINESE
 * @brief 创建刻度的主刻度和次刻度向量
 * \endif
 */
void LinearScale::calculate()
{
    majors_p.clear();
    minors_p.clear();

    double interval = mstop_p - mstart_p;

    double runningval;
    int i = 0;

    // majors

    // first tic
    //  if (mstart_p<start_p || mstop_p>stop_p)
    //    return;

    majors_p.push_back(mstart_p);

    // remaining tics
    for (i = 1; i <= majorintervals_p; ++i) {
        double t = double(i) / majorintervals_p;
        runningval = mstart_p + t * interval;
        if (runningval > stop_p)
            break;
        if (isPracticallyZero(mstart_p, -t * interval)) // prevent rounding errors near 0
            runningval = 0.0;
        majors_p.push_back(runningval);
    }
    majorintervals_p = static_cast<int>(majors_p.size());
    if (majorintervals_p)
        --majorintervals_p;

    // minors

    if (!majorintervals_p || !minorintervals_p) // no valid interval
    {
        minorintervals_p = 0;
        return;
    }

    // start_p      mstart_p
    //  |_____________|_____ _ _ _

    double step = (majors_p[1] - majors_p[0]) / minorintervals_p;
    if (isPracticallyZero(step))
        return;

    runningval = mstart_p - step;
    while (runningval > start_p) {
        minors_p.push_back(runningval);
        runningval -= step;
    }

    //       mstart_p            mstop_p
    //  ________|_____ _ _ _ _ _ ___|__________

    for (i = 0; i != majorintervals_p; ++i) {
        runningval = majors_p[i] + step;
        for (int j = 0; j != minorintervals_p; ++j) {
            minors_p.push_back(runningval);
            runningval += step;
        }
    }

    //    mstop_p       stop_p
    // _ _ _|_____________|

    runningval = mstop_p + step;
    while (runningval < stop_p) {
        minors_p.push_back(runningval);
        runningval += step;
    }
}

void LogScale::setupCounter(double &k, int &step)
{
    switch (minorintervals_p) {
    case 9:
        k = 9;
        step = 1;
        break;
    case 5:
        k = 8;
        step = 2;
        break;
    case 3:
        k = 5;
        step = 3;
        break;
    case 2:
        k = 5;
        step = 5;
        break;
    default:
        k = 9;
        step = 1;
    }
}

/**
 * \if ENGLISH
 * @brief Creates major and minor vectors for the logarithmic scale
 * @warning If the interval is too small, the scale becomes empty or will contain
 *          only a single major tic. There is no automatism (also not planned for now)
 *          for an 'intelligent' guess, what to do. Better switch manually to linear
 *          scales in such cases.
 * \endif
 *
 * \if CHINESE
 * @brief 创建对数刻度的主刻度和次刻度向量
 * @warning 如果区间太小，刻度将变为空或仅包含一个主刻度。
 *          目前没有（也未计划）自动"智能"判断机制。
 *          此类情况下建议手动切换到线性刻度。
 * \endif
 */
void LogScale::calculate()
{
    majors_p.clear();
    minors_p.clear();

    if (start_p < DBL_MIN_10_EXP)
        start_p = DBL_MIN_10_EXP;
    if (stop_p > DBL_MAX_10_EXP)
        stop_p = DBL_MAX_10_EXP;

    double interval = stop_p - start_p;
    if (interval <= 0)
        return;

    double runningval = floor(start_p);
    while (runningval <= stop_p) {
        if (runningval >= start_p)
            majors_p.push_back(runningval);
        ++runningval;
    }
    majorintervals_p = static_cast<int>(majors_p.size());
    if (majorintervals_p)
        --majorintervals_p;

    if (majors_p.size() < 1) // not even a single major tic
    {
        return;
    }

    // minors

    // start_p      mstart_p
    //  |_____________|_____ _ _ _

    double k;
    int step;
    setupCounter(k, step);
    runningval = log10(k) + (majors_p[0] - 1);
    while (runningval > start_p && k > 1) {
        minors_p.push_back(runningval);
        k -= step;
        runningval = log10(k) + (majors_p[0] - 1);
    }

    //       mstart_p            mstop_p
    //  ________|_____ _ _ _ _ _ ___|__________

    for (int i = 0; i != majorintervals_p; ++i) {
        setupCounter(k, step);
        runningval = log10(k) + (majors_p[i]);
        while (k > 1) {
            minors_p.push_back(runningval);
            k -= step;
            runningval = log10(k) + (majors_p[i]);
        }
    }

    //    mstop_p       stop_p
    // _ _ _|_____________|

    setupCounter(k, step);
    runningval = log10(k) + (majors_p.back());
    do {
        k -= step;
        runningval = log10(k) + (majors_p.back());
    } while (runningval >= stop_p);
    while (k > 1) {
        minors_p.push_back(runningval);
        k -= step;
        runningval = log10(k) + (majors_p.back());
    }
}

/**
 * \if ENGLISH
 * @brief Sets the minor intervals for the logarithmic scale
 * @param[in] val Number of minor intervals (only 9, 5, 3, or 2 are accepted)
 * @details They will produce mantissa sets of {2,3,4,5,6,7,8,9}, {2,4,6,8}, {2,5} or {5} respectively.
 * \endif
 *
 * \if CHINESE
 * @brief 设置对数刻度的次区间数
 * @param[in] val 次区间数（仅接受 9、5、3 或 2）
 * @details 它们将分别产生尾数集 {2,3,4,5,6,7,8,9}、{2,4,6,8}、{2,5} 或 {5}。
 * \endif
 */
void LogScale::setMinors(int val)
{
    if ((val == 2) || (val == 3) || (val == 5) || (val == 9))
        minorintervals_p = val;
}

/**
 * \if ENGLISH
 * @brief Default constructor - sets 9 minor intervals
 * \endif
 *
 * \if CHINESE
 * @brief 默认构造函数 - 设置 9 个次区间
 * \endif
 */
LogScale::LogScale()
{
    minorintervals_p = 9;
}

/**
 * \if ENGLISH
 * @brief Returns a power of 10 associated to the major value at index idx
 * @param[in] idx The current major tic index
 * @return The QString representation of 10^value for valid index, empty QString else
 * \endif
 *
 * \if CHINESE
 * @brief 返回与索引 idx 处主刻度值关联的 10 的幂
 * @param[in] idx 当前主刻度索引
 * @return 有效索引时返回 10^value 的 QString 表示，否则返回空 QString
 * \endif
 */
QString LogScale::ticLabel(unsigned int idx) const
{
    if (idx < majors_p.size()) {
        double val = majors_p[idx];
        return QString::number(pow(double(10), val));
    }
    return QString("");
}
