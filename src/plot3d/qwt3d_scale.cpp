#include "qwt3d_scale.h"

using namespace Qwt3D;

Scale::Scale()
    : start_p(0.), stop_p(0.), majorintervals_p(0), minorintervals_p(0), mstart_p(0.), mstop_p(0.)
{
}

/**
 * @brief Maps the double value at tic-position idx to a final representation
 * @param idx The current major tic index
 * @return The QString representation for the value corresponding to a valid index, an empty QString else.
 * @details The default return value is simply the tic values QString representation.
 *          Overwrite this function, if you plan to transform the value in some way.
 *          See e.g. LogScale::ticLabel.
 */
QString Scale::ticLabel(unsigned int idx) const
{
    if (idx < majors_p.size()) {
        return QString::number(majors_p[idx]);
    }
    return QString("");
}

/**
 * @brief Sets start and stop value for the scale
 * @param start Scale start value
 * @param stop Scale stop value
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
 * @brief Sets value of first and last major tic
 * @param start First major tic value
 * @param stop Last major tic value
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
 * @brief Autoscales the axis
 * @param[out] a First major tic after applying autoscaling
 * @param[out] b Last major tic after applying autoscaling
 * @param start Scale begin
 * @param stop Scale end
 * @param ivals Requested number of major intervals
 * @return Number of major intervals after autoscaling
 * @details The default implementation sets a=start, b=stop and returns ivals.
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
 * @brief Applies LinearAutoScaler::execute() for autoscaling
 * @param[out] a First major tic after autoscaling
 * @param[out] b Last major tic after autoscaling
 * @param start Scale begin
 * @param stop Scale end
 * @param ivals Requested number of major intervals
 * @return Number of major intervals after autoscaling
 */
int LinearScale::autoscale(double &a, double &b, double start, double stop, int ivals)
{
    return autoscaler_p.execute(a, b, start, stop, ivals);
}

/**
 * @brief Creates the major and minor vector for the scale
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
 * @brief Creates major and minor vectors for the logarithmic scale
 * @warning If the interval is too small, the scale becomes empty or will contain
 *          only a single major tic. There is no automatism (also not planned for now)
 *          for an 'intelligent' guess, what to do. Better switch manually to linear
 *          scales in such cases.
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
 * @brief Sets the minor intervals for the logarithmic scale
 * @param val Number of minor intervals (only 9, 5, 3, or 2 are accepted)
 * @details They will produce mantissa sets of {2,3,4,5,6,7,8,9}, {2,4,6,8}, {2,5} or {5} respectively.
 */
void LogScale::setMinors(int val)
{
    if ((val == 2) || (val == 3) || (val == 5) || (val == 9))
        minorintervals_p = val;
}

/**
 * @brief Default constructor - sets 9 minor intervals
 */
LogScale::LogScale()
{
    minorintervals_p = 9;
}

/**
 * @brief Returns a power of 10 associated to the major value at index idx
 * @param idx The current major tic index
 * @return The QString representation of 10^value for valid index, empty QString else
 */
QString LogScale::ticLabel(unsigned int idx) const
{
    if (idx < majors_p.size()) {
        double val = majors_p[idx];
        return QString::number(pow(double(10), val));
    }
    return QString("");
}
