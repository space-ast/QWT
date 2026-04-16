#ifndef __LABELPIXMAP_H__
#define __LABELPIXMAP_H__

#include <qpixmap.h>
#include <qimage.h>
#include <qfont.h>
#include <qpainter.h>
#include <qfontmetrics.h>

#include "qwt3d_drawable.h"

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief A Qt string or an output device dependent string
 * @details Label provides text rendering on 3D plots, supporting both Qt string
 *          representation and device-dependent string output.
 * \endif
 *
 * \if CHINESE
 * @brief Qt 字符串或输出设备依赖的字符串
 * @details Label 提供三维绘图上的文本渲染，支持 Qt 字符串表示和设备依赖的字符串输出。
 * \endif
 */
class QWT3D_EXPORT Label : public Drawable
{

public:
    Label();
    // Construct label and initialize with font
    Label(const QString &family, int pointSize, int weight = QFont::Normal, bool italic = false);

    // Sets the labels font
    void setFont(QString const &family, int pointSize, int weight = QFont::Normal,
                 bool italic = false);

    // Fine tunes label
    void adjust(int gap);
    // Returns the gap caused by adjust()
    double gap() const { return gap_; }
    // Sets the labels position
    void setPosition(Qwt3D::Triple pos, ANCHOR a = BottomLeft);
    // Sets the labels position relative to screen
    void setRelPosition(Tuple rpos, ANCHOR a);
    // Receives bottom left label position
    Qwt3D::Triple first() const { return beg_; }
    // Receives top right label position
    Qwt3D::Triple second() const { return end_; }
    // Defines an anchor point for the labels surrounding rectangle
    ANCHOR anchor() const
    {
        return anchor_;
    }
    virtual void setColor(double r, double g, double b, double a = 1);
    virtual void setColor(Qwt3D::RGBA rgba);

    // Sets the labels string
    void setString(QString const &s);
    // Actual drawing
    void draw();

    // Decides about use of PDF standard fonts for PDF output
    static void useDeviceFonts(bool val);

private:
    Qwt3D::Triple beg_, end_, pos_;
    QPixmap pm_;
    QImage buf_, tex_;
    QFont font_;
    QString text_;

    ANCHOR anchor_;

    void init();
    void init(const QString &family, int pointSize, int weight = QFont::Normal,
              bool italic = false);
    void update();
    void convert2screen();
    double width() const;
    double height() const;

    int gap_;

    bool flagforupdate_;

    static bool devicefonts_;
};

} // ns

#endif