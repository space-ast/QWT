#ifndef qwt3d_reader_h
#define qwt3d_reader_h

#include "qwt3d_io.h"

namespace Qwt3D {

/**
 * @brief Functor for reading of native files containing grid data
 * @details As a standard input functor associated with "mes" and "MES" file extensions.
 */
class QWT3D_EXPORT NativeReader : public IO::Functor
{
    friend class IO;

public:
    NativeReader();

private:
    IO::Functor *clone() const { return new NativeReader(*this); }
    bool operator()(Plot3D *plot, QString const &fname);
    static const char *magicstring;
    double minz_, maxz_;
    bool collectInfo(FILE *&file, QString const &fname, unsigned &xmesh, unsigned &ymesh,
                     double &minx, double &maxx, double &miny, double &maxy);
};

} // ns

#endif