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
 * @brief The QwtPlotLayoutEngine class
 * @details Originally a private class in QwtPlotLayout, previously written as class LayoutEngine in qwt_plot_layout.cpp.
 *          It was extracted as a public class because other layouts need to use it.
 * \endif
 * 
 * \if CHINESE
 * @brief QwtPlotLayoutEngine 类
 * @details 原来的 QwtPlotLayout 里的私有类，原来此类写在 qwt_plot_layout.cpp 中，class LayoutEngine，
 *          由于其它布局会用到，把它提取为公共类
 * \endif
 */
class QWT_EXPORT QwtPlotLayoutEngine
{
public:
    struct Dimensions
    {
        /**
         * \if ENGLISH
         * @brief Constructor
         * \endif
         */
        Dimensions();

        /**
         * \if ENGLISH
         * @brief Get axis dimension
         * \endif
         */
        int dimAxis(QwtAxisId axisId) const;

        /**
         * \if ENGLISH
         * @brief Set axis dimension
         * \endif
         */
        void setDimAxis(QwtAxisId axisId, int dim);

        /**
         * \if ENGLISH
         * @brief Get axes dimension
         * \endif
         */
        int dimAxes(int axisPos) const;

        /**
         * \if ENGLISH
         * @brief Get Y axes dimension
         * \endif
         */
        int dimYAxes() const;

        /**
         * \if ENGLISH
         * @brief Get X axes dimension
         * \endif
         */
        int dimXAxes() const;

        /**
         * \if ENGLISH
         * @brief Get centered rect
         * \endif
         */
        QRectF centered(const QRectF& rect, const QRectF& labelRect) const;

        /**
         * \if ENGLISH
         * @brief Get inner rect
         * \endif
         */
        QRectF innerRect(const QRectF& rect) const;

        int dimTitle;
        int dimFooter;

    private:
        int m_dimAxes[ QwtAxis::AxisPositions ];
    };

    class LayoutData
    {
    public:
        struct LegendData
        {
            /**
             * \if ENGLISH
             * @brief Initialize legend data
             * \endif
             */
            void init(const QwtAbstractLegend* legend);

            /**
             * \if ENGLISH
             * @brief Get legend hint
             * \endif
             */
            QSize legendHint(const QwtAbstractLegend* legend, const QRectF& rect) const;

            int frameWidth;
            int hScrollExtent;
            int vScrollExtent;
            QSize hint;
        };

        struct LabelData
        {
            /**
             * \if ENGLISH
             * @brief Initialize label data
             * \endif
             */
            void init(const QwtTextLabel* label);

            QwtText text;
            int frameWidth;
        };

        struct ScaleData
        {
            /**
             * \if ENGLISH
             * @brief Initialize scale data
             * \endif
             */
            void init(const QwtScaleWidget* axisWidget);

            /**
             * \if ENGLISH
             * @brief Reset scale data
             * \endif
             */
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

        struct CanvasData
        {
            /**
             * \if ENGLISH
             * @brief Initialize canvas data
             * \endif
             */
            void init(const QWidget* canvas);

            int contentsMargins[ QwtAxis::AxisPositions ];
        };

    public:
        enum Label
        {
            Title,
            Footer,

            NumLabels
        };

        /**
         * \if ENGLISH
         * @brief Constructor
         * \endif
         */
        LayoutData(const QwtPlot* plot);

        /**
         * \if ENGLISH
         * @brief Check if Y axes are symmetric
         * \endif
         */
        bool hasSymmetricYAxes() const;

        /**
         * \if ENGLISH
         * @brief Get axis data
         * \endif
         */
        ScaleData& axisData(QwtAxisId axisId);

        /**
         * \if ENGLISH
         * @brief Get axis data (const version)
         * \endif
         */
        const ScaleData& axisData(QwtAxisId axisId) const;

        /**
         * \if ENGLISH
         * @brief Get tick offset
         * \endif
         */
        double tickOffset(int axisPos) const;

        LegendData legendData;
        LabelData labelData[ NumLabels ];
        CanvasData canvasData;

    private:
        ScaleData m_scaleData[ QwtAxis::AxisPositions ];
    };

public:
    /**
     * \if ENGLISH
     * @brief Constructor
     * \endif
     */
    QwtPlotLayoutEngine();

    /**
     * \if ENGLISH
     * @brief Layout legend
     * \endif
     */
    QRectF layoutLegend(int plotLayoutOptions,
                        const LayoutData::LegendData& legendData,
                        const QRectF& rect,
                        const QSize& legendHint) const;

    /**
     * \if ENGLISH
     * @brief Align legend
     * \endif
     */
    QRectF alignLegend(const QSize& legendHint, const QRectF& canvasRect, const QRectF& legendRect) const;

    /**
     * \if ENGLISH
     * @brief Align scales
     * \endif
     */
    void alignScales(int plotLayoutOptions,
                     const LayoutData& layoutData,
                     QRectF& canvasRect,
                     QRectF scaleRect[ QwtAxis::AxisPositions ]) const;

    /**
     * \if ENGLISH
     * @brief Align scales to canvas
     * \endif
     */
    void alignScalesToCanvas(int plotLayoutOptions,
                             const LayoutData& layoutData,
                             const QRectF& canvasRect,
                             QRectF scaleRect[ QwtAxis::AxisPositions ]) const;

    /**
     * \if ENGLISH
     * @brief Layout dimensions
     * \endif
     */
    Dimensions layoutDimensions(int plotLayoutOptions, const LayoutData& layoutData, const QRectF& rect) const;

    /**
     * \if ENGLISH
     * @brief Set spacing
     * \endif
     */
    void setSpacing(unsigned int spacing);

    /**
     * \if ENGLISH
     * @brief Get spacing
     * \endif
     */
    unsigned int spacing() const;

    /**
     * \if ENGLISH
     * @brief Set align canvas
     * \endif
     */
    void setAlignCanvas(int axisPos, bool on);

    /**
     * \if ENGLISH
     * @brief Check if canvas is aligned
     * \endif
     */
    bool alignCanvas(int axisPos) const;

    /**
     * \if ENGLISH
     * @brief Set canvas margin
     * \endif
     */
    void setCanvasMargin(int axisPos, int margin);

    /**
     * \if ENGLISH
     * @brief Get canvas margin
     * \endif
     */
    int canvasMargin(int axisPos) const;

    /**
     * \if ENGLISH
     * @brief Set legend position
     * \endif
     */
    void setLegendPos(QwtPlot::LegendPosition pos);

    /**
     * \if ENGLISH
     * @brief Get legend position
     * \endif
     */
    QwtPlot::LegendPosition legendPos() const;

    /**
     * \if ENGLISH
     * @brief Set legend ratio
     * \endif
     */
    void setLegendRatio(double ratio);

    /**
     * \if ENGLISH
     * @brief Get legend ratio
     * \endif
     */
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
