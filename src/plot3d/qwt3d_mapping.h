#ifndef qwt3d_mapping_h
#define qwt3d_mapping_h

#include <qstring.h>
#include "qwt3d_global.h"
#include "qwt3d_types.h"

namespace Qwt3D {

/**
 * @brief Abstract base class for general mappings
 */
class QWT3D_EXPORT Mapping
{

public:
    // Destructor
    virtual ~Mapping() { }
    // Descriptive String
    virtual QString name() const { return QString(""); }
};

} // ns

#endif /* include guarded */