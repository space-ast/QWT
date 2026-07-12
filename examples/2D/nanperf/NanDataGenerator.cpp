// MSVC does not expose M_PI from <cmath> unless _USE_MATH_DEFINES is defined
// before any header pulls in math.h.
#define _USE_MATH_DEFINES

#include "NanDataGenerator.h"

#include <QtGlobal>

#include <cmath>

double NanDataGenerator::signalValue(int i)
{
    const double t1 = 2.0 * M_PI * i / 2000.0;
    const double t2 = 2.0 * M_PI * i / 200.0;
    return std::sin(t1) + 0.3 * std::sin(t2);
}

int NanDataGenerator::nanCount(NanCase cs, int numPoints, double nanFraction)
{
    const int nanN = static_cast< int >(nanFraction * numPoints);
    switch (cs) {
    case NanCase::Leading:
    case NanCase::Middle:
    case NanCase::Trailing:
        return nanN;
    case NanCase::LeadingTrailing:
        return 2 * (nanN / 2);
    case NanCase::Baseline:
        return 0;
    }
    return 0;
}

void NanDataGenerator::generate(NanCase cs, int numPoints, double nanFraction, QVector< double >& x, QVector< double >& y)
{
    x.resize(numPoints);
    y.resize(numPoints);
    const int nanN     = static_cast< int >(nanFraction * numPoints);
    const int half     = nanN / 2;
    const int midStart = (numPoints - nanN) / 2;
    const double nan   = qQNaN();

    for (int i = 0; i < numPoints; ++i) {
        x[ i ] = static_cast< double >(i);

        bool isNan = false;
        switch (cs) {
        case NanCase::Leading:
            isNan = (i < nanN);
            break;
        case NanCase::LeadingTrailing:
            isNan = (i < half) || (i >= numPoints - half);
            break;
        case NanCase::Middle:
            isNan = (i >= midStart) && (i < midStart + nanN);
            break;
        case NanCase::Trailing:
            isNan = (i >= numPoints - nanN);
            break;
        case NanCase::Baseline:
            isNan = false;
            break;
        }
        y[ i ] = isNan ? nan : signalValue(i);
    }
}

QString NanDataGenerator::caseLabel(NanCase cs)
{
    switch (cs) {
    case NanCase::Leading:
        return QStringLiteral("Leading NaN");
    case NanCase::LeadingTrailing:
        return QStringLiteral("Leading & Trailing NaN");
    case NanCase::Middle:
        return QStringLiteral("Middle NaN");
    case NanCase::Trailing:
        return QStringLiteral("Trailing NaN");
    case NanCase::Baseline:
        return QStringLiteral("No NaN Baseline");
    }
    return QString();
}
