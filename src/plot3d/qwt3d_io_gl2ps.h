#ifndef qwt3d_io_gl2ps_h
#define qwt3d_io_gl2ps_h

#include <time.h>

#include "qwt3d_types.h"
#include "qwt3d_io.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Provides EPS, PS, PDF, SVG, PGF and TeX output
 * @details VectorWriter provides vector graphics output through the gl2ps library,
 *          supporting EPS, PS, PDF, SVG, PGF and TeX formats.
 * \endif
 *
 * \if CHINESE
 * @brief 提供 EPS, PS, PDF, SVG, PGF 和 TeX 输出
 * @details VectorWriter 通过 gl2ps 库提供矢量图形输出，
 *          支持 EPS, PS, PDF, SVG, PGF 和 TeX 格式。
 * \endif
 */
class QWT3D_EXPORT VectorWriter : public IO::Functor
{
    friend class IO;

public:
    // The possible output formats for the text parts of the scene
    enum TEXTMODE {
        PIXEL,
        NATIVE,
        TEX
    };

    // The possible behaviour for landscape settings
    enum LANDSCAPEMODE {
        ON,
        OFF,
        AUTO
    };

    // The possible sorting types which are translated in gl2ps types
    enum SORTMODE {
        NOSORT,
        SIMPLESORT,
        BSPSORT
    };

    VectorWriter();

    // Sets landscape mode
    void setLandscape(LANDSCAPEMODE val) { landscape_ = val; }
    // Returns the current landscape mode
    LANDSCAPEMODE landscape() const { return landscape_; }

    void setTextMode(TEXTMODE val, QString fname = "");
    // Return current text output mode
    TEXTMODE textMode() const { return textmode_; }

    // Sets one of the SORTMODE sorting modes
    void setSortMode(SORTMODE val) { sortmode_ = val; }
    // Returns gl2ps sorting type
    SORTMODE sortMode() const { return sortmode_; }
    // Turns compressed output on or off (no effect if zlib support is not available)
    void setCompressed(bool val);
    // Returns compression mode (always false if zlib support has not been set)
    bool compressed() const { return compressed_; }

    bool setFormat(QString const &format);

private:
    IO::Functor *clone() const;
    bool operator()(Plot3D *plot, QString const &fname);

    GLint gl2ps_format_;
    bool formaterror_;
    bool compressed_;
    SORTMODE sortmode_;
    LANDSCAPEMODE landscape_;
    TEXTMODE textmode_;
    QString texfname_;
};

GLint setDeviceLineWidth(GLfloat val);
GLint setDevicePointSize(GLfloat val);
GLint drawDevicePixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
                       const void *pixels);
GLint drawDeviceText(const char *str, const char *fontname, int fontsize, Qwt3D::Triple pos,
                     Qwt3D::RGBA rgba, Qwt3D::ANCHOR align, double gap);
void setDevicePolygonOffset(GLfloat factor, GLfloat units);

} // ns

#endif /* include guarded */