#ifndef QWTPLOTLAYOUTENGINE_H
#define QWTPLOTLAYOUTENGINE_H
#include <QRectF>
#include <QFont>
class QWidget;
// qwt
#include "qwt_global.h"
#include "qwt_axis.h"
#include "qwt_axis_id.h"
#include "qwt_text.h"
#include "qwt_plot.h"
class QwtAbstractLegend;
class QwtTextLabel;
class QwtScaleWidget;
/**
 * \if ENGLISH
 * @brief Layout engine for QwtPlot components
 * @details Originally a private class in QwtPlotLayout, previously written as class LayoutEngine in qwt_plot_layout.cpp.
 *          It was extracted as a public class because other layouts need to use it.
 * \endif
 *
 * \if CHINESE
 * @brief QwtPlot组件的布局引擎
 * @details 原来的QwtPlotLayout里的私有类，原来此类写在qwt_plot_layout.cpp中，class LayoutEngine，
 *          由于其它布局会用到，把它提取为公共类。
 * \endif
 */
class QWT_EXPORT QwtPlotLayoutEngine
{
public:
    /**
     * \if ENGLISH
     * @brief Structure holding dimension values for layout calculation
     * @details Contains width/height values for title, footer, and all four axes.
     * \endif
     *
     * \if CHINESE
     * @brief 保存布局计算尺寸值的结构体
     * @details 包含标题、页脚以及四个坐标轴的宽度/高度值。
     * \endif
     */
    struct Dimensions
    {
        // Constructor
        Dimensions();

        // Get the dimension for a specific axis
        int dimAxis(QwtAxisId axisId) const;

        // Set the dimension for a specific axis
        void setDimAxis(QwtAxisId axisId, int dim);

        // Get the dimension for an axis position (YLeft, YRight, XTop, XBottom)
        int dimAxes(int axisPos) const;

        // Get the total width of left and right Y axes
        int dimYAxes() const;

        // Get the total height of top and bottom X axes
        int dimXAxes() const;

        // Center a label rectangle within available space after accounting for Y axes
        QRectF centered(const QRectF& rect, const QRectF& labelRect) const;

        // Calculate inner rectangle after reserving space for all axes
        QRectF innerRect(const QRectF& rect) const;

        int dimTitle;
        int dimFooter;

    private:
        int m_dimAxes[ QwtAxis::AxisPositions ];
    };

    /**
     * \if ENGLISH
     * @brief Data structure for layout calculation
     * @details Contains cached data extracted from plot components for efficient layout calculation.
     * \endif
     *
     * \if CHINESE
     * @brief 布局计算数据结构
     * @details 包含从绘图组件提取的缓存数据，用于高效的布局计算。
     * \endif
     */
    class LayoutData
    {
    public:
        /**
         * \if ENGLISH
         * @brief Data for legend layout calculation
         * @details Contains frame width, scroll extents, and size hint for the legend.
         * \endif
         *
         * \if CHINESE
         * @brief 图例布局计算数据
         * @details 包含图例的边框宽度、滚动范围和尺寸提示。
         * \endif
         */
        struct LegendData
        {
            // Initialize legend data from a QwtAbstractLegend
            void init(const QwtAbstractLegend* legend);

            // Calculate optimal legend size for the given rectangle
            QSize legendHint(const QwtAbstractLegend* legend, const QRectF& rect) const;

            int frameWidth;
            int hScrollExtent;
            int vScrollExtent;
            QSize hint;
        };

        /**
         * \if ENGLISH
         * @brief Data for title/footer label layout calculation
         * @details Contains text and frame width for a label widget.
         * \endif
         *
         * \if CHINESE
         * @brief 标题/页脚标签布局计算数据
         * @details 包含标签部件的文本和边框宽度。
         * \endif
         */
        struct LabelData
        {
            // Initialize label data from a QwtTextLabel
            void init(const QwtTextLabel* label);

            QwtText text;
            int frameWidth;
        };

        /**
         * \if ENGLISH
         * @brief Data for axis scale widget layout calculation
         * @details Contains visibility, font, border distances, and tick offset for an axis.
         * \endif
         *
         * \if CHINESE
         * @brief 坐标轴刻度部件布局计算数据
         * @details 包含坐标轴的可见性、字体、边框距离和刻度偏移量。
         * \endif
         */
        struct ScaleData
        {
            // Initialize scale data from a QwtScaleWidget
            void init(const QwtScaleWidget* axisWidget);

            // Reset scale data to default values
            void reset();

            bool isVisible;
            const QwtScaleWidget* scaleWidget;
            QFont scaleFont;
            int start;
            int end;
            int baseLineOffset;
            double tickOffset;
            int dimWithoutTitle;
            int edgeMargin;
        };

        /**
         * \if ENGLISH
         * @brief Data for canvas layout calculation
         * @details Contains content margins for the canvas widget.
         * \endif
         *
         * \if CHINESE
         * @brief 画布布局计算数据
         * @details 包含画布部件的内容边距。
         * \endif
         */
        struct CanvasData
        {
            // Initialize canvas data from a QWidget
            void init(const QWidget* canvas);

            int contentsMargins[ QwtAxis::AxisPositions ];
        };

        /**
         * \if ENGLISH
         * @brief Label type enumeration
         * @details Identifies title or footer labels in the layout.
         * \endif
         *
         * \if CHINESE
         * @brief 标签类型枚举
         * @details 标识布局中的标题或页脚标签。
         * \endif
         */
        enum Label
        {
            Title,   //!< \if ENGLISH Title label \endif \if CHINESE 标题标签 \endif
            Footer,  //!< \if ENGLISH Footer label \endif \if CHINESE 页脚标签 \endif

            NumLabels  //!< \if ENGLISH Number of label types \endif \if CHINESE 标签类型数量 \endif
        };

        // Construct LayoutData from a QwtPlot
        LayoutData(const QwtPlot* plot);

        // Check if left and right Y axes have the same visibility state
        bool hasSymmetricYAxes() const;

        // Get mutable scale data for a specific axis
        ScaleData& axisData(QwtAxisId axisId);

        // Get const scale data for a specific axis
        const ScaleData& axisData(QwtAxisId axisId) const;

        // Get tick offset for a specific axis position
        double tickOffset(int axisPos) const;

        LegendData legendData;
        LabelData labelData[ NumLabels ];
        CanvasData canvasData;

    private:
        ScaleData m_scaleData[ QwtAxis::AxisPositions ];
    };

public:
    // Constructor
    QwtPlotLayoutEngine();

    // Calculate and return the legend rectangle within the available space
    QRectF layoutLegend(int plotLayoutOptions,
                        const LayoutData::LegendData& legendData,
                        const QRectF& rect,
                        const QSize& legendHint) const;

    // Align the legend rectangle relative to the canvas
    QRectF alignLegend(const QSize& legendHint, const QRectF& canvasRect, const QRectF& legendRect) const;

    // Align scale rectangles with the canvas, adjusting positions for proper layout
    void alignScales(int plotLayoutOptions,
                     const LayoutData& layoutData,
                     QRectF& canvasRect,
                     QRectF scaleRect[ QwtAxis::AxisPositions ]) const;

    // Align scale rectangles to canvas boundaries
    void alignScalesToCanvas(int plotLayoutOptions,
                             const LayoutData& layoutData,
                             const QRectF& canvasRect,
                             QRectF scaleRect[ QwtAxis::AxisPositions ]) const;

    // Calculate layout dimensions for title, footer, and all axes
    Dimensions layoutDimensions(int plotLayoutOptions, const LayoutData& layoutData, const QRectF& rect) const;

    // Set the spacing between plot components
    void setSpacing(unsigned int spacing);

    // Get the spacing between plot components
    unsigned int spacing() const;

    // Set whether the canvas should align to the scale at a given axis position
    void setAlignCanvas(int axisPos, bool on);

    // Check if the canvas is aligned to the scale at a given axis position
    bool alignCanvas(int axisPos) const;

    // Set the margin between canvas and scale at a given axis position
    void setCanvasMargin(int axisPos, int margin);

    // Get the margin between canvas and scale at a given axis position
    int canvasMargin(int axisPos) const;

    // Set the legend position
    void setLegendPos(QwtPlot::LegendPosition pos);

    // Get the legend position
    QwtPlot::LegendPosition legendPos() const;

    // Set the legend ratio for size calculation
    void setLegendRatio(double ratio);

    // Get the legend ratio for size calculation
    double legendRatio() const;

private:
    /**
     * \if ENGLISH
     * @brief Get height for width
     * \endif
     */
    int heightForWidth(LayoutData::Label labelType,
                       const LayoutData& layoutData,
                       int plotLayoutOptions,
                       double width,
                       int axesWidth) const;

    QwtPlot::LegendPosition m_legendPos;
    double m_legendRatio;

    unsigned int m_canvasMargin[ QwtAxis::AxisPositions ] = { 0, 0, 0, 0 };
    bool m_alignCanvas[ QwtAxis::AxisPositions ];

    unsigned int m_spacing;
};

#endif  // QWTPLOTLAYOUTENGINE_H
