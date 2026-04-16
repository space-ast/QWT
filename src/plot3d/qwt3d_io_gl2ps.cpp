#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning(disable : 4786)
#endif

#include <time.h>
#include "qwt3d_openglhelper.h"
#include "gl2ps.h"
#include "qwt3d_io_gl2ps.h"
#include "qwt3d_plot.h"
#include "qwt_version_info.h"

using namespace Qwt3D;

/**
 * \if ENGLISH
 * @brief Provides a new VectorWriter object
 * @return A cloned copy of this VectorWriter as Functor pointer
 * \endif
 *
 * \if CHINESE
 * @brief 提供一个新的 VectorWriter 对象
 * @return 此 VectorWriter 的克隆副本作为 Functor 指针
 * \endif
 */
IO::Functor* VectorWriter::clone() const
{
    return new VectorWriter(*this);
}

VectorWriter::VectorWriter()
    : gl2ps_format_(GL2PS_EPS)
    , formaterror_(false)
    ,
#ifdef GL2PS_HAVE_ZLIB
    compressed_(true)
    ,
#else
    compressed_(false)
    ,
#endif
    sortmode_(SIMPLESORT)
    , landscape_(VectorWriter::AUTO)
    , textmode_(VectorWriter::PIXEL)
    , texfname_("")
{
}

/**
 * \if ENGLISH
 * @brief Sets the mode for text output
 * @param[in] val The underlying format for the generated output:
 *                PIXEL - poor quality but exact positioning;
 *                NATIVE - high quality but inexact positioning;
 *                TEX - high quality and exact positioning, arbitrary TeX strings
 *                as content for the saved labels are possible. The disadvantage is
 *                the need for an additionally TeX run to get the final output.
 * @param[in] fname Optional, used only in conjunction with TeX output; file name
 *                  for the generated TeX file. If not set, a file called
 *                  "OUTPUT.FOR.tex" will be generated, where "OUTPUT.FOR" describes
 *                  the file name argument for IO::save().
 * @note On Linux platforms, pdflatex seems a file named 'dump_0.pdf.tex' mistakenly
 *       to identify as PDF file.
 * \endif
 *
 * \if CHINESE
 * @brief 设置文本输出模式
 * @param[in] val 生成输出的底层格式：
 *                PIXEL - 质量较差但定位精确；
 *                NATIVE - 质量较高但定位不精确；
 *                TEX - 质量高且定位精确，可以使用任意 TeX 字串作为保存标签的内容。
 *                缺点是需要额外的 TeX 运行才能获得最终输出。
 * @param[in] fname 可选参数，仅在 TeX 输出模式下使用；生成的 TeX 文件的文件名。
 *                  如果未设置，将生成名为 "OUTPUT.FOR.tex" 的文件，
 *                  其中 "OUTPUT.FOR" 描述了 IO::save() 的文件名参数。
 * @note 在 Linux 平台上，pdflatex 可能会错误地将名为 'dump_0.pdf.tex' 的文件识别为 PDF 文件。
 * \endif
 */
void VectorWriter::setTextMode(TEXTMODE val, QString fname)
{
    textmode_ = val;
    texfname_ = (fname.isEmpty()) ? QString("") : fname;
}

#ifdef GL2PS_HAVE_ZLIB
/**
 * \if ENGLISH
 * @brief Turns compressed output on or off
 * @param[in] val True to enable compression, false to disable
 * @details No effect if zlib support has not been set.
 * \endif
 *
 * \if CHINESE
 * @brief 开启或关闭压缩输出
 * @param[in] val true 启用压缩，false 禁用
 * @details 如果未设置 zlib 支持则无效。
 * \endif
 */
void VectorWriter::setCompressed(bool val)
{
    compressed_ = val;
}
#else
/**
 * \if ENGLISH
 * @brief Turns compressed output on or off (no effect - zlib support not available)
 * \endif
 *
 * \if CHINESE
 * @brief 开启或关闭压缩输出（无效 - zlib 支持不可用）
 * \endif
 */
void VectorWriter::setCompressed(bool)
{
    compressed_ = false;
}
#endif

/**
 * \if ENGLISH
 * @brief Sets output format
 * @param[in] format Must be one of "EPS_GZ", "PS_GZ", "EPS", "PS", "PDF", "SVG" or "PGF" (case sensitive)
 * @return True on success, false for unknown format
 * \endif
 *
 * \if CHINESE
 * @brief 设置输出格式
 * @param[in] format 必须是 "EPS_GZ"、"PS_GZ"、"EPS"、"PS"、"PDF"、"SVG" 或 "PGF" 之一（区分大小写）
 * @return 成功返回 true，未知格式返回 false
 * \endif
 */
bool VectorWriter::setFormat(QString const& format)
{
    if (format == QString("EPS")) {
        gl2ps_format_ = GL2PS_EPS;
    } else if (format == QString("PS")) {
        gl2ps_format_ = GL2PS_PS;
    } else if (format == QString("PDF")) {
        gl2ps_format_ = GL2PS_PDF;
    } else if (format == QString("SVG")) {
        gl2ps_format_ = GL2PS_SVG;
    } else if (format == QString("PGF")) {
        gl2ps_format_ = GL2PS_PGF;
    }
#ifdef GL2PS_HAVE_ZLIB
    else if (format == QString("EPS_GZ")) {
        gl2ps_format_ = GL2PS_EPS;
    } else if (format == QString("PS_GZ")) {
        gl2ps_format_ = GL2PS_PS;
    }
#endif
    else {
        formaterror_ = true;
        return false;
    }
    formaterror_ = false;
    return true;
}

/**
 * \if ENGLISH
 * @brief Performs actual output
 * @param[in] plot Plot3D widget to export
 * @param[in] fname Output file name
 * @return True on success, false on format error or file open failure
 * \endif
 *
 * \if CHINESE
 * @brief 执行实际输出
 * @param[in] plot 要导出的 Plot3D 控件
 * @param[in] fname 输出文件名
 * @return 成功返回 true，格式错误或文件打开失败返回 false
 * \endif
 */
bool VectorWriter::operator()(Plot3D* plot, QString const& fname)
{
    if (formaterror_)
        return false;

    plot->makeCurrent();

    GLint bufsize = 0, state = GL2PS_OVERFLOW;
    GLint viewport[ 4 ];

    glGetIntegerv(GL_VIEWPORT, viewport);

    GLint options = GL2PS_SIMPLE_LINE_OFFSET | GL2PS_SILENT | GL2PS_DRAW_BACKGROUND | GL2PS_OCCLUSION_CULL | GL2PS_BEST_ROOT;

    if (compressed_)
        options |= GL2PS_COMPRESS;

    switch (landscape_) {
    case VectorWriter::AUTO:
        if (viewport[ 2 ] - viewport[ 0 ] > viewport[ 3 ] - viewport[ 0 ])
            options |= GL2PS_LANDSCAPE;
        break;
    case VectorWriter::ON:
        options |= GL2PS_LANDSCAPE;
        break;
    default:
        break;
    }

    int sortmode = GL2PS_SIMPLE_SORT;
    switch (sortmode_) {
    case VectorWriter::NOSORT:
        sortmode = GL2PS_NO_SORT;
        break;
    case VectorWriter::SIMPLESORT:
        sortmode = GL2PS_SIMPLE_SORT;
        break;
    case VectorWriter::BSPSORT:
        sortmode = GL2PS_BSP_SORT;
        break;
    default:
        break;
    }

    switch (textmode_) {
    case NATIVE:
        Label::useDeviceFonts(true);
        break;
    case PIXEL:
        Label::useDeviceFonts(false);
        break;
    case TEX:
        options |= GL2PS_NO_PIXMAP | GL2PS_NO_TEXT;
        break;
    default:
        break;
    }

    QString version = QString::number(QWT_VERSION_MAJ) + "." + QString::number(QWT_VERSION_MIN) + "."
                      + QString::number(QWT_VERSION_PAT);

    QString producer = QString("QwtPlot3D ") + version + " (beta)";

    FILE* fp = fopen(QWT3DLOCAL8BIT(fname), "wb");
    if (!fp) {
        Label::useDeviceFonts(false);
        return false;
    }
    while (state == GL2PS_OVERFLOW) {
        bufsize += 2 * 1024 * 1024;
        gl2psBeginPage("---",
                       QWT3DLOCAL8BIT(producer),
                       viewport,
                       gl2ps_format_,
                       sortmode,
                       options,
                       GL_RGBA,
                       0,
                       nullptr,
                       0,
                       0,
                       0,
                       bufsize,
                       fp,
                       QWT3DLOCAL8BIT(fname));

        plot->grabFramebuffer();
        state = gl2psEndPage();
    }
    fclose(fp);

    // extra TeX file
    if (textmode_ == TEX) {
        QString fn = (texfname_.isEmpty()) ? fname + ".tex" : texfname_;

        fp = fopen(QWT3DLOCAL8BIT(fn), "wb");
        if (!fp) {
            Label::useDeviceFonts(false);
            return false;
        }
        Label::useDeviceFonts(true);
        options &= ~GL2PS_NO_PIXMAP & ~GL2PS_NO_TEXT;
        state = GL2PS_OVERFLOW;
        while (state == GL2PS_OVERFLOW) {
            bufsize += 2 * 1024 * 1024;
            gl2psBeginPage("---",
                           QWT3DLOCAL8BIT(producer),
                           viewport,
                           GL2PS_TEX,
                           sortmode,
                           options,
                           GL_RGBA,
                           0,
                           nullptr,
                           0,
                           0,
                           0,
                           bufsize,
                           fp,
                           QWT3DLOCAL8BIT(fn));

            plot->updateData();
            plot->update();
            state = gl2psEndPage();
        }
        fclose(fp);
    }

    Label::useDeviceFonts(false);

    return true;
}

// moved

GLint Qwt3D::setDeviceLineWidth(GLfloat val)
{
    if (val < 0)
        val = 0;

    GLint ret = gl2psLineWidth(val);

    GLfloat lw[ 2 ];
    glGetFloatv(GL_LINE_WIDTH_RANGE, lw);

    if (val < lw[ 0 ])
        val = lw[ 0 ];
    else if (val > lw[ 1 ])
        val = lw[ 1 ];

    glLineWidth(val);
    return ret;
}

GLint Qwt3D::setDevicePointSize(GLfloat val)
{
    if (val < 0)
        val = 0;

    GLint ret = gl2psPointSize(val);

    GLfloat lw[ 2 ];
    glGetFloatv(GL_POINT_SIZE_RANGE, lw);

    if (val < lw[ 0 ])
        val = lw[ 0 ];
    else if (val > lw[ 1 ])
        val = lw[ 1 ];

    glPointSize(val);
    return ret;
}

GLint Qwt3D::drawDevicePixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    glDrawPixels(width, height, format, type, pixels);

    if (format != GL_RGBA || type != GL_UNSIGNED_BYTE)
        return GL2PS_ERROR;

    GLfloat* convertedpixel = static_cast< GLfloat* >(malloc(3 * width * height * sizeof(GLfloat)));
    if (!convertedpixel)
        return GL2PS_ERROR;

    const GLubyte* px = reinterpret_cast< const GLubyte* >(pixels);
    for (int i = 0; i != 3 * width * height; i += 3) {
        int pxi                 = (4 * i) / 3;
        convertedpixel[ i ]     = px[ pxi ] / float(255);
        convertedpixel[ i + 1 ] = px[ pxi + 1 ] / float(255);
        convertedpixel[ i + 2 ] = px[ pxi + 2 ] / float(255);
    }
    GLint ret = gl2psDrawPixels(width, height, 0, 0, GL_RGB, GL_FLOAT, convertedpixel);
    free(convertedpixel);
    return ret;
}

GLint Qwt3D::drawDeviceText(const char* str, const char* fontname, int fontsize, Triple pos, RGBA /*rgba*/, ANCHOR align, double gap)
{
    double vp[ 3 ];

    World2ViewPort(vp[ 0 ], vp[ 1 ], vp[ 2 ], pos.x, pos.y, pos.z);
    Triple start(vp[ 0 ], vp[ 1 ], vp[ 2 ]);

    GLdouble fcol[ 4 ];
    glGetDoublev(GL_CURRENT_COLOR, fcol);
    GLdouble bcol[ 4 ];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, bcol);

    //	glColor4d(color.r, color.g, color.b, color.a);
    //		glClearColor(color.r, color.g, color.b, color.a);

    GLint ret = GL2PS_SUCCESS;

    GLint a = GL2PS_TEXT_BL;
    switch (align) {
    case Center:
        a = GL2PS_TEXT_C;
        break;
    case CenterLeft:
        a = GL2PS_TEXT_CL;
        start += Triple(gap, 0, 0);
        break;
    case CenterRight:
        a = GL2PS_TEXT_CR;
        start += Triple(-gap, 0, 0);
        break;
    case BottomCenter:
        a = GL2PS_TEXT_B;
        start += Triple(0, gap, 0);
        break;
    case BottomLeft:
        a = GL2PS_TEXT_BL;
        start += Triple(gap, gap, 0);
        break;
    case BottomRight:
        a = GL2PS_TEXT_BR;
        start += Triple(-gap, gap, 0);
        break;
    case TopCenter:
        a = GL2PS_TEXT_T;
        start += Triple(0, -gap, 0);
        break;
    case TopLeft:
        a = GL2PS_TEXT_TL;
        start += Triple(gap, -gap, 0);
        break;
    case TopRight:
        a = GL2PS_TEXT_TR;
        start += Triple(-gap, -gap, 0);
        break;
    default:
        break;
    }

    ViewPort2World(vp[ 0 ], vp[ 1 ], vp[ 2 ], start.x, start.y, start.z);
    Triple adjpos(vp[ 0 ], vp[ 1 ], vp[ 2 ]);

    glRasterPos3d(adjpos.x, adjpos.y, adjpos.z);
    ret = gl2psTextOpt(str, fontname, static_cast< int >(fontsize), a, 0);
    glColor4dv(fcol);
    glClearColor(bcol[ 0 ], bcol[ 1 ], bcol[ 2 ], bcol[ 3 ]);
    return ret;
}

void Qwt3D::setDevicePolygonOffset(GLfloat factor, GLfloat units)
{
    glPolygonOffset(factor, units);
    gl2psEnable(GL2PS_POLYGON_OFFSET_FILL);
}
