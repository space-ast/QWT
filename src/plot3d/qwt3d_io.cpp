#include <time.h>

#include "qwt3d_plot.h"
#include "qwt3d_io_gl2ps.h"
#include "qwt3d_io_reader.h"
#include <QImageWriter>

using namespace Qwt3D;

IO::Entry::Entry() : iofunc(0)
{
}

IO::Entry::~Entry()
{
    delete iofunc;
}

IO::Entry::Entry(IO::Entry const& e)
{
    if (this == &e)
        return;

    fmt    = e.fmt;
    iofunc = e.iofunc->clone();
}

void IO::Entry::operator=(IO::Entry const& e)
{
    if (this == &e)
        return;

    delete iofunc;
    fmt    = e.fmt;
    iofunc = e.iofunc->clone();
}

IO::Entry::Entry(QString const& s, Functor const& f) : fmt(s)
{
    iofunc = f.clone();
}

IO::Entry::Entry(QString const& s, Function f) : fmt(s)
{
    Wrapper w(f);
    iofunc = w.clone();
}

IO::FormatCompare::FormatCompare(IO::Entry const& e)
{
    e_ = e;
}

bool IO::FormatCompare::operator()(IO::Entry const& e)
{
    return (e.fmt == e_.fmt);
}

IO::FormatCompare2::FormatCompare2(QString s)
{
    s_ = s;
}

bool IO::FormatCompare2::operator()(IO::Entry const& e)
{
    return (e.fmt == s_);
}

bool IO::add_unique(Container& l, Entry const& e)
{
    FormatCompare comp(e);
    l.erase(std::remove_if(l.begin(), l.end(), comp), l.end());
    l.push_back(e);

    return true;
}

IO::IT IO::find(Container& l, QString const& fmt)
{
    FormatCompare2 comp(fmt);
    return std::find_if(l.begin(), l.end(), comp);
}

IO::Container& IO::rlist()
{
    static Container rl = Container();
    static bool rfirst  = true;
    if (rfirst) {
        rfirst = false;
        setupHandler();
    }
    return rl;
}

IO::Container& IO::wlist()
{
    static Container wl = Container();
    static bool wfirst  = true;
    if (wfirst) {
        wfirst = false;
        setupHandler();
    }
    return wl;
}

/**
 * \if ENGLISH
 * @brief Registers a new IO::Function for data input
 * @param[in] format Format string identifier
 * @param[in] func Input handler function
 * @return True on successful registration
 * @details Every call overwrites a formerly registered handler for the same format string (case sensitive).
 * \endif
 *
 * \if CHINESE
 * @brief 注册新的数据输入 IO::Function
 * @param[in] format 格式字符串标识符
 * @param[in] func 输入处理函数
 * @return 注册成功返回 true
 * @details 每次调用会覆盖相同格式字符串（区分大小写）的先前注册处理器。
 * \endif
 */
bool IO::defineInputHandler(QString const& format, IO::Function func)
{
    return add_unique(rlist(), Entry(format, func));
}

/**
 * \if ENGLISH
 * @brief Registers a new Functor for data input
 * @param[in] format Format string identifier
 * @param[in] func Input handler functor
 * @return True on successful registration
 * @details Every call overwrites a formerly registered handler for the same format string (case sensitive).
 * \endif
 *
 * \if CHINESE
 * @brief 注册新的数据输入 Functor
 * @param[in] format 格式字符串标识符
 * @param[in] func 输入处理仿函数
 * @return 注册成功返回 true
 * @details 每次调用会覆盖相同格式字符串（区分大小写）的先前注册处理器。
 * \endif
 */
bool IO::defineInputHandler(QString const& format, IO::Functor const& func)
{
    return add_unique(rlist(), Entry(format, func));
}

/**
 * \if ENGLISH
 * @brief Registers a new IO::Function for data output
 * @param[in] format Format string identifier
 * @param[in] func Output handler function
 * @return True on successful registration
 * @details Every call overwrites a formerly registered handler for the same format string (case sensitive).
 * \endif
 *
 * \if CHINESE
 * @brief 注册新的数据输出 IO::Function
 * @param[in] format 格式字符串标识符
 * @param[in] func 输出处理函数
 * @return 注册成功返回 true
 * @details 每次调用会覆盖相同格式字符串（区分大小写）的先前注册处理器。
 * \endif
 */
bool IO::defineOutputHandler(QString const& format, IO::Function func)
{
    return add_unique(wlist(), Entry(format, func));
}

/**
 * \if ENGLISH
 * @brief Registers a new Functor for data output
 * @param[in] format Format string identifier
 * @param[in] func Output handler functor
 * @return True on successful registration
 * @details Every call overwrites a formerly registered handler for the same format string (case sensitive).
 * \endif
 *
 * \if CHINESE
 * @brief 注册新的数据输出 Functor
 * @param[in] format 格式字符串标识符
 * @param[in] func 输出处理仿函数
 * @return 注册成功返回 true
 * @details 每次调用会覆盖相同格式字符串（区分大小写）的先前注册处理器。
 * \endif
 */
bool IO::defineOutputHandler(QString const& format, IO::Functor const& func)
{
    return add_unique(wlist(), Entry(format, func));
}

/**
 * \if ENGLISH
 * @brief Applies a reading IO::Function or IO::Functor
 * @param[in] plot Plot with the content that should be loaded
 * @param[in] fname File name
 * @param[in] format Input format
 * @return The return value from the called Function/Functor.
 *         Returns false if no registered handler could be found.
 * \endif
 *
 * \if CHINESE
 * @brief 应用数据读取 IO::Function 或 IO::Functor
 * @param[in] plot 要加载内容的绘图控件
 * @param[in] fname 文件名
 * @param[in] format 输入格式
 * @return 被调用的 Function/Functor 的返回值。
 *         如果找不到已注册的处理器，则返回 false。
 * \endif
 */
bool IO::load(Plot3D* plot, QString const& fname, QString const& format)
{
    IT it = IO::find(rlist(), format);

    if (it == rlist().end())
        return false;

    return (*it->iofunc)(plot, fname);
}

/**
 * \if ENGLISH
 * @brief Applies a writing IO::Function or IO::Functor
 * @param[in] plot Plot with the content that should be saved
 * @param[in] fname File name
 * @param[in] format Output format
 * @return The return value from the called Function/Functor.
 *         Returns false if no registered handler could be found.
 * \endif
 *
 * \if CHINESE
 * @brief 应用数据写入 IO::Function 或 IO::Functor
 * @param[in] plot 要保存内容的绘图控件
 * @param[in] fname 文件名
 * @param[in] format 输出格式
 * @return 被调用的 Function/Functor 的返回值。
 *         如果找不到已注册的处理器，则返回 false。
 * \endif
 */
bool IO::save(Plot3D* plot, QString const& fname, QString const& format)
{
    IT it = IO::find(wlist(), format);

    if (it == wlist().end())
        return false;

    return (*it->iofunc)(plot, fname);
}

/**
 * \if ENGLISH
 * @brief Returns a list of currently registered input formats
 * @return List of input format strings
 * \endif
 *
 * \if CHINESE
 * @brief 返回当前已注册的输入格式列表
 * @return 输入格式字符串列表
 * \endif
 */
QStringList IO::inputFormatList()
{
    QStringList list;
    for (IT it = rlist().begin(); it != rlist().end(); ++it)
        list.append(it->fmt);

    return list;
}

/**
 * \if ENGLISH
 * @brief Returns a list of currently registered output formats
 * @return List of output format strings
 * \endif
 *
 * \if CHINESE
 * @brief 返回当前已注册的输出格式列表
 * @return 输出格式字符串列表
 * \endif
 */
QStringList IO::outputFormatList()
{
    QStringList list;
    for (IT it = wlist().begin(); it != wlist().end(); ++it)
        list.append(it->fmt);

    return list;
}

/**
 * \if ENGLISH
 * @brief Returns the input functor in charge for format
 * @param[in] format Format string identifier
 * @return Pointer to the input functor, or 0 if non-existent
 * \endif
 *
 * \if CHINESE
 * @brief 返回负责指定格式的输入仿函数
 * @param[in] format 格式字符串标识符
 * @return 输入仿函数指针，不存在时返回 0
 * \endif
 */
IO::Functor* IO::inputHandler(QString const& format)
{
    IO::IT it = IO::find(rlist(), format);

    if (it == rlist().end())
        return 0;

    return it->iofunc;
}

/**
 * \if ENGLISH
 * @brief Returns the output functor in charge for format
 * @param[in] format Format string identifier
 * @return Pointer to the output functor, or 0 if non-existent
 * \endif
 *
 * \if CHINESE
 * @brief 返回负责指定格式的输出仿函数
 * @param[in] format 格式字符串标识符
 * @return 输出仿函数指针，不存在时返回 0
 * \endif
 */
IO::Functor* IO::outputHandler(QString const& format)
{
    IO::IT it = IO::find(wlist(), format);

    if (it == wlist().end())
        return 0;

    return it->iofunc;
}

bool PixmapWriter::operator()(Plot3D* plot, QString const& fname)
{
    QImage im = plot->grabFramebuffer();

    QImageWriter iio;
    iio.setFormat(QWT3DLOCAL8BIT(fmt_));
    iio.setQuality(quality_);
    iio.setFileName(fname);
    return iio.write(im);
}

/**
 * \if ENGLISH
 * @brief Calls Qt's QImageIO::setQuality() function
 * @param[in] val Quality value
 * \endif
 *
 * \if CHINESE
 * @brief 调用 Qt 的 QImageIO::setQuality() 函数
 * @param[in] val 质量值
 * \endif
 */
void PixmapWriter::setQuality(int val)
{
    quality_ = val;
}

void IO::setupHandler()
{
    QList< QByteArray > list         = QImageWriter::supportedImageFormats();
    QList< QByteArray >::Iterator it = list.begin();
    PixmapWriter qtw;
    while (it != list.end()) {
        qtw.fmt_ = *it;
        defineOutputHandler(*it, qtw);
        ++it;
    }
    VectorWriter vecfunc;
    vecfunc.setCompressed(false);
    vecfunc.setFormat("EPS");
    defineOutputHandler("EPS", vecfunc);
    vecfunc.setFormat("PS");
    defineOutputHandler("PS", vecfunc);

#ifdef GL2PS_HAVE_ZLIB
    vecfunc.setCompressed(true);
    vecfunc.setFormat("EPS_GZ");
    defineOutputHandler("EPS_GZ", vecfunc);
    vecfunc.setFormat("PS_GZ");
    defineOutputHandler("PS_GZ", vecfunc);
#endif
    vecfunc.setFormat("PDF");
    defineOutputHandler("PDF", vecfunc);
    vecfunc.setFormat("SVG");
    defineOutputHandler("SVG", vecfunc);
    vecfunc.setFormat("PGF");
    defineOutputHandler("PGF", vecfunc);

    defineInputHandler("mes", NativeReader());
    defineInputHandler("MES", NativeReader());
}

/**
 * \if ENGLISH
 * @brief Writes vector data supported by gl2ps
 * @param[in] fileName Output file name
 * @param[in] format Output format ("EPS", "PS", "PDF", "SVG", or "PGF")
 * @param[in] text Text handling mode
 * @param[in] sortmode Sort mode for polygon ordering
 * @return True on success
 * @deprecated Use Plot3D::save or IO::save instead.
 * @details If zlib has been configured, format types will be extended by "EPS_GZ" and "PS_GZ".
 *          Beware: BSPSORT turns out to behave very slowly and memory consuming, especially in cases
 *          where many polygons appear. It is still more exact than SIMPLESORT.
 * \endif
 *
 * \if CHINESE
 * @brief 写入 gl2ps 支持的矢量数据
 * @param[in] fileName 输出文件名
 * @param[in] format 输出格式（"EPS"、"PS"、"PDF"、"SVG" 或 "PGF"）
 * @param[in] text 文本处理模式
 * @param[in] sortmode 多边形排序模式
 * @return 成功时返回 true
 * @deprecated 请使用 Plot3D::save 或 IO::save 替代。
 * @details 如果配置了 zlib，格式类型将扩展为 "EPS_GZ" 和 "PS_GZ"。
 *          注意：BSPSORT 在出现大量多边形时非常缓慢且消耗内存，
 *          但比 SIMPLESORT 更精确。
 * \endif
 */
bool Plot3D::saveVector(QString const& fileName, QString const& format, VectorWriter::TEXTMODE text, VectorWriter::SORTMODE sortmode)
{
    if (format == "EPS" || format == "EPS_GZ" || format == "PS" || format == "PS_GZ" || format == "PDF"
        || format == "SVG" || format == "PGF") {
        VectorWriter* gl2ps = static_cast< VectorWriter* >(IO::outputHandler(format));
        if (gl2ps) {
            gl2ps->setSortMode(sortmode);
            gl2ps->setTextMode(text);
        }
        return IO::save(this, fileName, format);
    }
    return false;
}
/**
 * \if ENGLISH
 * @brief Saves the framebuffer to an image file
 * @param[in] fileName Output file name
 * @param[in] format Image file format supported by Qt
 * @return True on success
 * @deprecated Use Plot3D::save or IO::save instead.
 * \endif
 *
 * \if CHINESE
 * @brief 将帧缓冲区保存为图像文件
 * @param[in] fileName 输出文件名
 * @param[in] format Qt 支持的图像文件格式
 * @return 成功时返回 true
 * @deprecated 请使用 Plot3D::save 或 IO::save 替代。
 * \endif
 */
bool Plot3D::savePixmap(QString const& fileName, QString const& format)
{
    if (format == "EPS" || format == "EPS_GZ" || format == "PS" || format == "PS_GZ" || format == "PDF"
        || format == "SVG" || format == "PGF")
        return false;

    return IO::save(this, fileName, format);
}

/**
 * \if ENGLISH
 * @brief Saves content in one of the registered output formats
 * @param[in] fileName Output file name
 * @param[in] format Output format string
 * @return True on success
 * @details To modify the behaviour for more complex output handling use IO::outputHandler.
 * \endif
 *
 * \if CHINESE
 * @brief 以已注册的输出格式之一保存内容
 * @param[in] fileName 输出文件名
 * @param[in] format 输出格式字符串
 * @return 成功时返回 true
 * @details 对于更复杂的输出处理，请使用 IO::outputHandler 来修改行为。
 * \endif
 */
bool Plot3D::save(QString const& fileName, QString const& format)
{
    return IO::save(this, fileName, format);
}
