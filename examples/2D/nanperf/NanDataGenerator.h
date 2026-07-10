#ifndef NAN_DATA_GENERATOR_H
#define NAN_DATA_GENERATOR_H

#include <QVector>
#include <QString>

/// Distribution of NaN values within a generated curve.
enum class NanCase
{
    Leading,          ///< NaN at the beginning, then signal
    LeadingTrailing,  ///< NaN at both ends, signal in the middle
    Middle,           ///< Signal at both ends, NaN in the middle
    Trailing,         ///< Signal first, NaN at the end
    Baseline          ///< No NaN (reference)
};

/// Generates X/Y sample arrays for the NaN rendering/performance example.
/// X is the monotonic finite index 0..N-1; NaN is placed only in Y.
class NanDataGenerator
{
public:
    /// Fill @a x and @a y for @a cs with @a numPoints points and @a nanFraction NaN.
    static void generate(NanCase cs, int numPoints, double nanFraction, QVector< double >& x, QVector< double >& y);

    /// The synthetic signal value at index @a i.
    static double signalValue(int i);

    /// Exact number of NaN points produced for @a cs.
    static int nanCount(NanCase cs, int numPoints, double nanFraction);

    /// Human-readable label (Chinese) for @a cs.
    static QString caseLabel(NanCase cs);
};

#endif
