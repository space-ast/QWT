# QWT 项目 Doxygen 双语注释重构进度追踪

## 说明
本文件用于追踪 QWT 项目注释重构任务的进度，确保每次执行都能知道哪些文件已经处理，哪些尚未处理。

## 处理规则
1. **源文件 (.cpp)**: 添加中英双语 Doxygen 注释（使用 `\if ENGLISH` 和 `\if CHINESE`）
2. **头文件 (.h)**: 
   - public 函数仅保留英文简要注释
   - 类的 Doxygen 注释保留双语
   - Q_SIGNALS 下的信号函数保留双语注释
3. **严禁修改代码逻辑**，仅修改注释

## 文件处理状态

### 已处理文件

#### 第一批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_bezier.h - 添加类的双语 Doxygen 注释
- src/plot/qwt_clipper.h - 添加命名空间和函数的双语简要注释
- src/plot/qwt_curve_fitter.h - 添加类和枚举的双语 Doxygen 注释

**源文件 (.cpp):**
- src/plot/qwt_bezier.cpp - 所有函数的双语 Doxygen 注释
- src/plot/qwt_clipper.cpp - 所有函数的双语 Doxygen 注释
- src/plot/qwt_curve_fitter.cpp - 所有函数的双语 Doxygen 注释

#### 第二批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_math.h - 数学函数的双语注释（qwtFuzzyCompare, qwtSign, qwtSqr, qwtFastAtan, qwtRadians, qwtDegrees, qwtCeil, qwtFloor, qwtVerifyRange, qwtDistance, qwt_is_nan_or_inf 等）

**源文件 (.cpp):**
- src/plot/qwt_math.cpp - qwtNormalizeRadians, qwtNormalizeDegrees, qwtRand 的双语注释

#### 第三批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_interval.h - QwtInterval 类及其所有内联函数的双语注释
- src/plot/qwt_samples.h - QwtIntervalSample 类的双语注释

**源文件 (.cpp):**
- src/plot/qwt_interval.cpp - QwtInterval 方法的详细双语注释 (normalize, normalized, inverted)

#### 第四批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_point_3d.h - QwtPoint3D 类及其所有内联函数的双语注释
- src/plot/qwt_point_polar.h - QwtPointPolar 类及其所有内联函数的双语注释

**源文件 (.cpp):**

#### 第五批 (2026-03-15)
**头文件 (.h):**
- src/qwt_global.h - 宏定义的双语 Doxygen 注释
- src/plot/qwt_axis.h - 坐标轴枚举和方法的双语注释
- src/plot/qwt_axis_id.h - 坐标轴ID的双语注释

**源文件 (.cpp):**

#### 第六批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_utils.h - 工具函数的双语注释
- src/plot/qwt_system_clock.h - QwtSystemClock 类的双语注释
- src/plot/qwt_transform.h - QwtTransform 类的双语注释

**源文件 (.cpp):**
- src/plot/qwt_utils.cpp - 工具函数的双语 Doxygen 注释
- src/plot/qwt_system_clock.cpp - QwtSystemClock 方法的详细双语注释
- src/plot/qwt_transform.cpp - QwtTransform 类的详细双语注释

#### 第七批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_widget_overlay.h - QwtWidgetOverlay 类的双语注释
- src/plot/qwt_wheel.h - QwtWheel 类的双语注释

**源文件 (.cpp):**
- src/plot/qwt_widget_overlay.cpp - 所有函数的双语 Doxygen 注释
- src/plot/qwt_wheel.cpp - 所有函数的双语 Doxygen 注释

#### 第八批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_weeding_curve_fitter.h - QwtWeedingCurveFitter 类的双语注释
- src/plot/qwt_vectorfield_symbol.h - QwtVectorFieldSymbol 接口及实现类的双语注释

**源文件 (.cpp):**
- src/plot/qwt_weeding_curve_fitter.cpp - 所有函数的双语 Doxygen 注释
- src/plot/qwt_vectorfield_symbol.cpp - 所有函数的双语 Doxygen 注释

#### 第九批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_abstract_legend.h - QwtAbstractLegend 类的双语注释，public 函数英文简要注释
- src/plot/qwt_abstract_scale.h - QwtAbstractScale 类的双语注释，public/protected 函数英文简要注释
- src/plot/qwt_abstract_scale_draw.h - QwtAbstractScaleDraw 类及枚举的双语 Doxygen 注释

**源文件 (.cpp):**
- src/plot/qwt_abstract_legend.cpp - 所有方法的双语 Doxygen 注释（构造函数、析构函数、scrollExtent）
- src/plot/qwt_abstract_scale.cpp - 所有方法的双语 Doxygen 注释（构造函数、析构函数、setScale 系列、transform 系列等）
- src/plot/qwt_abstract_scale_draw.cpp - 部分方法的双语 Doxygen 注释（构造函数、析构函数、enableComponent、setScaleDiv 等）

#### 第十批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_abstract_slider.h - QwtAbstractSlider 类的双语注释，public/protected 函数英文简要注释，signals 双语注释
- src/plot/qwt_magnifier.h - QwtMagnifier 类的双语注释，public/protected 函数英文简要注释
- src/plot/qwt_legend_label.h - QwtLegendLabel 类的双语注释，public 函数英文简要注释，signals 双语注释

**源文件 (.cpp):**
- src/plot/qwt_abstract_slider.cpp - 部分方法的双语 Doxygen 注释（构造函数、析构函数、setValid、isValid 等）

#### 第十一批 (2026-03-15)
**源文件 (.cpp):**
- src/plot/qwt_abstract_slider.cpp - 继续添加双语 Doxygen 注释（setReadOnly、isReadOnly、setTracking、isTracking、mousePressEvent、mouseMoveEvent、mouseReleaseEvent、wheelEvent、keyPressEvent 等）

#### 第十二批 (2026-03-15)
**源文件 (.cpp):**
- src/plot/qwt_abstract_slider.cpp - 完成剩余方法的双语 Doxygen 注释（setPageSteps、pageSteps、setStepAlignment、stepAlignment、setValue、value、setWrapping、wrapping、setInvertedControls、invertedControls、incrementValue、incrementedValue、boundedValue、alignedValue、scaleChange、sliderChange）

#### 第十三批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_legend.h - 为所有 public 函数添加英文简要注释
- src/plot/qwt_legend_data.h - 为枚举和所有 public 函数添加双语注释

**源文件 (.cpp):**
- src/plot/qwt_legend.cpp - 所有方法的双语 Doxygen 注释（构造函数、析构函数、setMaxColumns、maxColumns、setDefaultItemMode、defaultItemMode、contentsWidget、legendWidget、legendWidgets、itemInfo、eventFilter、sizeHint、heightForWidth、horizontalScrollBar、verticalScrollBar、updateLegend、createWidget、updateWidget、itemClicked、itemChecked、renderLegend、renderItem、isEmpty、scrollExtent）
- src/plot/qwt_legend_data.cpp - 所有方法的双语 Doxygen 注释（构造函数、析构函数、setValues、values、setValue、value、hasRole、isValid、icon、title、mode）

#### 第十五批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_text_label.h - 添加类的双语 Doxygen 注释和所有函数的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_text_label.cpp - 所有方法的双语 Doxygen 注释（构造函数 x2、析构函数、setPlainText、plainText、setText x2、clear、text、indent、setIndent、margin、setMargin、sizeHint、minimumSizeHint、heightForWidth、textRect、drawText、paintEvent、drawContents、init、defaultIndent）

#### 第十六批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_text_engine.h - 添加 3 个类（QwtTextEngine、QwtPlainTextEngine、QwtRichTextEngine）的双语 Doxygen 注释和所有函数的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_text_engine.cpp - 所有方法的双语 Doxygen 注释（QwtTextEngine 构造函数/析构函数、QwtPlainTextEngine 所有方法、QwtRichTextEngine 所有方法）

#### 第十七批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_thermo.h - 添加类的双语 Doxygen 注释、枚举的双语注释和所有函数的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_thermo.cpp - 所有方法的双语 Doxygen 注释（构造函数、析构函数、setRangeFlags、rangeFlags、setValue、value、setScaleDraw、scaleDraw x2、paintEvent、resizeEvent、changeEvent、layoutThermo、pipeRect、setOrientation、orientation、setOriginMode、originMode、setOrigin、origin、setScalePosition、scalePosition、scaleChange、drawLiquid、setSpacing、spacing、setBorderWidth、borderWidth、setColorMap、colorMap x2、setFillBrush、fillBrush、setAlarmBrush、alarmBrush、setAlarmLevel、alarmLevel、setPipeWidth、pipeWidth、setAlarmEnabled、alarmEnabled、sizeHint、minimumSizeHint、fillRect、alarmRect）

#### 第十八批 (2026-03-15)
**源文件 (.cpp):**

#### 第十九批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_slider.h - 添加类的双语 Doxygen 注释、ScalePosition 枚举的双语注释和所有函数的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_slider.cpp - 所有方法的双语 Doxygen 注释（构造函数 x2、析构函数、setOrientation、orientation、setScalePosition、scalePosition、setBorderWidth、borderWidth、setSpacing、spacing、setHandleSize、handleSize、setScaleDraw、scaleDraw x2、scaleChange、setUpdateInterval、updateInterval、drawSlider、drawHandle、isScrollPosition、scrolledTo、mousePressEvent、mouseReleaseEvent、timerEvent、paintEvent、resizeEvent、changeEvent、event、layoutSlider、setTrough、hasTrough、setGroove、hasGroove、sizeHint、minimumSizeHint、handleRect、sliderRect）

#### 第二十批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_series_store.h - 添加 QwtAbstractSeriesStore 和 QwtSeriesStore 类的双语 Doxygen 注释和所有函数的英文简要注释
- src/plot/qwt_series_data.h - 添加 QwtSeriesData 和 QwtArraySeriesData 类的双语 Doxygen 注释，所有数据接口类的英文简要注释，qwtUpperSampleIndex 模板函数的双语详细注释

**源文件 (.cpp):**
- src/plot/qwt_series_data.cpp - 所有 qwtBoundingRect 函数的双语 Doxygen 注释，所有数据类构造函数和 boundingRect 方法的双语注释

#### 第二十一批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_scale_widget.h - 添加类的双语 Doxygen 注释、BuiltinActions 枚举的双语注释和所有函数的英文简要注释
- src/plot/qwt_scale_engine.h - 添加 QwtScaleArithmetic、QwtScaleEngine、QwtLinearScaleEngine、QwtLogScaleEngine 类的双语 Doxygen 注释和所有方法的英文简要注释
- src/plot/qwt_scale_draw.h - 添加类的双语 Doxygen 注释、Alignment 枚举的双语注释和所有方法的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_scale_engine.cpp - 所有方法的双语 Doxygen 注释（QwtScaleArithmetic 的 4 个静态方法、QwtScaleEngine 的 16 个方法、QwtLinearScaleEngine 的 8 个方法、QwtLogScaleEngine 的 8 个方法）
- src/plot/qwt_scale_draw.cpp - 部分方法的双语注释（构造函数、析构函数、alignment、setAlignment、orientation，其余方法将在后续批次继续）

#### 第二十二批 (2026-03-15)
**头文件 (.h):**
- src/plot/qwt_round_scale_draw.h - 添加类的双语 Doxygen 注释和所有方法的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_round_scale_draw.cpp - 所有方法的双语 Doxygen 注释（构造函数、析构函数、setRadius、radius、moveCenter、center、setAngleRange、drawLabel、drawTick、drawBackbone、extent）

#### 第二十三批 (2026-03-16)
**头文件 (.h):**
- src/plot/qwt_scale_div.h - 为所有 public 函数添加英文简要注释
- src/plot/qwt_date.h - 添加类的双语 Doxygen 注释，Week0Type 和 IntervalType 枚举的双语注释，所有方法的英文简要注释
- src/plot/qwt_text.h - 添加类的双语 Doxygen 注释，TextFormat、PaintAttribute、LayoutAttribute 枚举的双语注释，所有方法的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_scale_draw.cpp - 完成剩余方法的双语 Doxygen 注释（构造函数、析构函数、alignment、setAlignment、orientation、getBorderDistHint、minLabelDist、extent、minLength、labelPosition、drawTick、drawBackbone、move、pos、setLength、length、drawLabel、boundingLabelRect、labelTransformation、labelRect、labelSize、setLabelRotation、labelRotation、setLabelAlignment、labelAlignment、maxLabelWidth、maxLabelHeight、updateMap）
- src/plot/qwt_scale_div.cpp - 所有方法的双语 Doxygen 注释（4个构造函数、setInterval x2、interval、setLowerBound、lowerBound、setUpperBound、upperBound、range、operator==、operator!=、fuzzyCompare、isEmpty、isIncreasing、contains、invert、inverted、bounded、setTicks、ticks）
- src/plot/qwt_date.cpp - 所有方法的双语 Doxygen 注释（toDateTime、toDouble、ceil、floor、minDate、maxDate、dateOfWeek0、weekNumber、utcOffset、toString）
- src/plot/qwt_text.cpp - 主要方法的双语 Doxygen 注释（构造函数 x3、析构函数、operator=、operator==、operator!=、setText、text、setRenderFlags、isNull、isEmpty）

#### 第二十四批 (2026-03-16)
**头文件 (.h):**
- src/plot/qwt_symbol.h - 添加类的双语 Doxygen 注释，Style 和 CachePolicy 枚举的双语注释，所有方法的英文简要注释
- src/plot/qwt_spline.h - 添加类的双语 Doxygen 注释，BoundaryType、BoundaryPosition、BoundaryCondition 枚举的双语注释，所有方法的英文简要注释
- src/plot/qwt_spline_basis.h - 添加类的双语 Doxygen 注释，所有方法的英文简要注释
- src/plot/qwt_spline_cubic.h - 添加类的双语 Doxygen 注释，所有方法的英文简要注释

**源文件 (.cpp):**
- src/plot/qwt_symbol.cpp - 主要方法的双语 Doxygen 注释（构造函数 x3、析构函数、setCachePolicy、cachePolicy、setPath、path、setGraphic、graphic、setSize、size、setBrush、brush、setPen、pen、setStyle、style、setColor、drawSymbol、drawSymbols、boundingRect）
- src/plot/qwt_spline.cpp - 主要方法的双语 Doxygen 注释（构造函数、析构函数、setParametrization、parametrization、setBoundaryType、boundaryType、setBoundaryCondition、boundaryCondition、setBoundaryValue、boundaryValue、painterPath、polygon、locality）
- src/plot/qwt_spline_basis.cpp - 主要方法的双语 Doxygen 注释（构造函数、析构函数、setDegree、degree、painterPath、polynomials、locality、isValid）
- src/plot/qwt_spline_cubic.cpp - 主要方法的双语 Doxygen 注释（构造函数、析构函数、painterPath、polynomials、locality、bezierControlLines）

#### 第二十五批 (2026-03-16)
**头文件 (.h):**
- src/plot/qwt_spline_pleasing.h - 添加类的双语 Doxygen 注释，所有方法的英文简要注释
- src/plot/qwt_spline_local.h - 添加类的双语 Doxygen 注释，Type 枚举的双语注释，所有方法的英文简要注释
- src/plot/qwt_spline_curve_fitter.h - 添加类的双语 Doxygen 注释，所有方法的英文简要注释
- src/plot/qwt_slider.h - 已有双语注释，无需修改

**源文件 (.cpp):**
- src/plot/qwt_spline_pleasing.cpp - 主要方法的双语 Doxygen 注释（构造函数、析构函数、locality、painterPath、bezierControlLines）
- src/plot/qwt_spline_local.cpp - 主要方法的双语 Doxygen 注释（构造函数、析构函数、type、painterPath、bezierControlLines、slopes、polynomials）
- src/plot/qwt_spline_curve_fitter.cpp - 已有注释，无需修改

### 未处理文件

#### 头文件 (.h) - src 目录
```
src/qwt_version_info.h
src/plot3d/qwt3d_types.h
src/plot3d/qwt3d_portability.h
src/plot3d/qwt3d_plot.h
src/plot3d/qwt3d_io.h
src/plot3d/qwt3d_global.h
src/plot/qwt_thermo.h
src/plot/qwt_text_label.h
src/plot/qwt_text_engine.h
src/plot/qwt_stylesheet_recorder.h
src/plot/qwt_spline_pleasing.h
src/plot/qwt_spline_local.h
src/plot/qwt_spline_curve_fitter.h
src/plot/qwt_spline_pleasing.h
src/plot/qwt_spline_local.h
src/plot/qwt_spline_curve_fitter.h
src/plot/qwt_slider.h
src/plot/qwt_series_store.h
src/plot/qwt_series_data.h
src/plot/qwt_scale_widget.h
src/plot/qwt_scale_engine.h
src/plot/qwt_scale_draw.h
src/plot/qwt_sampling_thread.h
src/plot/qwt_raster_data.h
src/plot/qwt_polar_spectrogram.h
src/plot/qwt_polar_renderer.h
src/plot/qwt_polar_plot.h
src/plot/qwt_polar_picker.h
src/plot/qwt_polar_marker.h
src/plot/qwt_polar_magnifier.h
src/plot/qwt_polar_itemdict.h
src/plot/qwt_polar_grid.h
src/plot/qwt_polar_curve.h
src/plot/qwt_polar_fitter.h
src/plot/qwt_polar_canvas.h
src/plot/qwt_polar_cache_panner.h
src/plot/qwt_plot_zoneitem.h
src/plot/qwt_plot_vectorfield.h
src/plot/qwt_point_data.h
src/plot/qwt_plot_tradingcurve.h
src/plot/qwt_plot_textlabel.h
src/plot/qwt_plot_spectrogram.h
src/plot/qwt_plot_spectrocurve.h
src/plot/qwt_plot_seriesitem.h
src/plot/qwt_plot_series_data_picker.h
src/plot/qwt_plot_shapeitem.h
src/plot/qwt_plot_rescaler.h
src/plot/qwt_plot_scaleitem.h
src/plot/qwt_plot_rasteritem.h
src/plot/qwt_plot_renderer.h
src/plot/qwt_plot_picker.h
src/plot/qwt_plot_panner.h
src/plot/qwt_plot_multi_barchart.h
src/plot/qwt_plot_opengl_canvas.h
src/plot/qwt_plot_marker.h
src/plot/qwt_plot_legenditem.h
src/plot/qwt_plot_magnifier.h
src/plot/qwt_plot_intervalcurve.h
src/plot/qwt_plot_histogram.h
src/plot/qwt_plot_grid.h
src/plot/qwt_plot_graphicitem.h
src/plot/qwt_plot_glcanvas.h
src/plot/qwt_plot_directpainter.h
src/plot/qwt_plot_dict.h
src/plot/qwt_plot_curve.h
src/plot/qwt_plot_canvas_zoomer.h
src/plot/qwt_plot_canvas.h
src/plot/qwt_plot_cache_panner.h
src/plot/qwt_plot_barchart.h
src/plot/qwt_plot_axis_zoomer.h
src/plot/qwt_plot.h
src/plot/qwt_plot_abstract_barchart.h
src/plot/qwt_picker_machine.h
src/plot/qwt_picker.h
src/plot/qwt_matrix_raster_data.h
src/plot/qwt_null_paintdevice.h
src/plot/qwt_math.h
src/plot/qwt_legend.h
src/plot/qwt_grid_raster_data.h
src/plot/qwt_knob.h
src/plot/qwt_graphic.h
src/plot/qwt_figure_widget_overlay.h
src/plot/qwt_dyngrid_layout.h
src/plot/qwt_dial_needle.h
src/plot/qwt_dial.h
src/plot/qwt_date_scale_engine.h
src/plot/qwt_date_scale_draw.h
src/plot/qwt_compass_rose.h
src/plot/qwt_counter.h
src/plot/qwt_compass.h
src/plot/qwt_color_map.h
src/plot/qwt_cache_panner.h
src/plot/qwt_arrow_button.h
src/plot/qwt_analog_clock.h
src/plot/qwt_plot_series_data_picker_group.h
src/plot3d/qwt3d_volumeplot.h
src/plot3d/qwt3d_surfaceplot.h
src/plot3d/qwt3d_scale.h
src/plot3d/qwt3d_parametricsurface.h
src/plot3d/qwt3d_openglhelper.h
src/plot3d/qwt3d_multiplot.h
src/plot3d/qwt3d_mapping.h
src/plot3d/qwt3d_label.h
src/plot3d/qwt3d_io_reader.h
src/plot3d/qwt3d_io_gl2ps.h
src/plot3d/qwt3d_helper.h
src/plot3d/qwt3d_gridmapping.h
src/plot3d/qwt3d_graphplot.h
src/plot3d/qwt3d_function.h
src/plot3d/qwt3d_enrichment_std.h
src/plot3d/qwt3d_enrichment.h
src/plot3d/qwt3d_drawable.h
src/plot3d/qwt3d_coordsys.h
src/plot3d/qwt3d_colorlegend.h
src/plot3d/qwt3d_color.h
src/plot3d/qwt3d_axis.h
src/plot3d/qwt3d_autoscaler.h
src/plot3d/qwt3d_autoptr.h
src/plot3d/3rdparty/gl2ps/gl2ps.h
src/plot/qwt_text.h
src/plot/qwt_symbol.h
src/plot/qwt_spline_polynomial.h
src/plot/qwt_spline_parametrization.h
src/plot/qwt_scale_map.h
src/plot/qwt_scale_div.h
src/plot/qwt_samples.h
src/plot/qwt_polar_layout.h
src/plot/qwt_polar_item.h
src/plot/qwt_point_polar.h
src/plot/qwt_polar.h
src/plot/qwt_point_mapper.h
src/plot/qwt_point_3d.h
src/plot/qwt_plot_transparent_canvas.h
src/plot/qwt_plot_svgitem.h
src/plot/qwt_plot_scale_event_dispatcher.h
src/plot/qwt_plot_layout_engine.h
src/plot/qwt_plot_layout.h
src/plot/qwt_plot_item.h
src/plot/qwt_plot_abstract_canvas.h
src/plot/qwt_pixel_matrix.h
src/plot/qwt_parasite_plot_layout.h
src/plot/qwt_painter_command.h
src/plot/qwt_painter.h
src/plot/qwt_legend_data.h
src/plot/qwt_interval_symbol.h
src/plot/qwt_interval.h
src/plot/qwt_figure_layout.h
src/plot/qwt_figure.h
src/plot/qwt_event_patterns.h
src/plot/qwt_curve_fitter.h
src/plot/qwt_column_symbol.h
src/plot/qwt_clipper.h
src/plot/qwt_canvas_picker.h
src/plot/qwt_bezier.h
```

#### 源文件 (.cpp) - src 目录
```
src/plot3d/qwt3d_types.cpp
src/plot3d/qwt3d_surfaceplot.cpp
src/plot3d/qwt3d_parametricsurface.cpp
src/plot3d/qwt3d_lighting.cpp
src/plot3d/qwt3d_io_reader.cpp
src/plot3d/qwt3d_io_gl2ps.cpp
src/plot3d/qwt3d_io.cpp
src/plot3d/qwt3d_function.cpp
src/plot3d/qwt3d_drawable.cpp
src/plot3d/qwt3d_coordsys.cpp
src/plot3d/qwt3d_color.cpp
src/plot3d/qwt3d_autoscaler.cpp
src/plot/qwt_spline_pleasing.cpp
src/plot/qwt_slider.cpp
src/plot/qwt_scale_widget.cpp
src/plot/qwt_scale_map.cpp
src/plot/qwt_scale_engine.cpp
src/plot/qwt_polar_spectrogram.cpp
src/plot/qwt_polar_renderer.cpp
src/plot/qwt_polar_plot.cpp
src/plot/qwt_polar_picker.cpp
src/plot/qwt_polar_magnifier.cpp
src/plot/qwt_polar_itemdict.cpp
src/plot/qwt_polar_item.cpp
src/plot/qwt_polar_grid.cpp
src/plot/qwt_polar_curve.cpp
src/plot/qwt_polar_canvas.cpp
src/plot/qwt_polar_cache_panner.cpp
src/plot/qwt_plot_vectorfield.cpp
src/plot/qwt_plot_spectrogram.cpp
src/plot/qwt_plot_scaleitem.cpp
src/plot/qwt_plot_rescaler.cpp
src/plot/qwt_plot_renderer.cpp
src/plot/qwt_plot_picker.cpp
src/plot/qwt_plot_panner.cpp
src/plot/qwt_plot_opengl_canvas.cpp
src/plot/qwt_plot_multi_barchart.cpp
src/plot/qwt_plot_marker.cpp
src/plot/qwt_plot_item.cpp
src/plot/qwt_plot_legenditem.cpp
src/plot/qwt_plot_histogram.cpp
src/plot/qwt_plot_intervalcurve.cpp
src/plot/qwt_plot_glcanvas.cpp
src/plot/qwt_plot_dict.cpp
src/plot/qwt_plot_directpainter.cpp
src/plot/qwt_plot_curve.cpp
src/plot/qwt_plot_canvas.cpp
src/plot/qwt_plot_barchart.cpp
src/plot/qwt_plot_cache_panner.cpp
src/plot/qwt_plot_axis_zoomer.cpp
src/plot/qwt_plot_axis.cpp
src/plot/qwt_plot.cpp
src/plot/qwt_picker.cpp
src/plot/qwt_painter.cpp
src/plot/qwt_null_paintdevice.cpp
src/plot/qwt_legend_label.cpp
src/plot/qwt_magnifier.cpp
src/plot/qwt_legend.cpp
src/plot/qwt_graphic.cpp
src/plot/qwt_figure_widget_overlay.cpp
src/plot/qwt_event_patterns.cpp
src/plot/qwt_dyngrid_layout.cpp
src/plot/qwt_dial.cpp
src/plot/qwt_compass.cpp
src/plot/qwt_clipper.cpp
src/plot/qwt_cache_panner.cpp
src/plot/qwt_arrow_button.cpp
src/plot/qwt_analog_clock.cpp
src/plot/qwt_plot_series_data_picker_group.cpp
src/plot3d/qwt3d_scale.cpp
src/plot3d/qwt3d_plot.cpp
src/plot3d/qwt3d_movements.cpp
src/plot3d/qwt3d_mousekeyboard.cpp
src/plot3d/qwt3d_meshplot.cpp
src/plot3d/qwt3d_label.cpp
src/plot3d/qwt3d_gridplot.cpp
src/plot3d/qwt3d_gridmapping.cpp
src/plot3d/qwt3d_enrichment_std.cpp
src/plot3d/qwt3d_dataviews.cpp
src/plot3d/qwt3d_colorlegend.cpp
src/plot3d/qwt3d_axis.cpp
src/plot/qwt_text_label.cpp
src/plot/qwt_spline_polynomial.cpp
src/plot/qwt_spline_parametrization.cpp
src/plot/qwt_spline_local.cpp
src/plot/qwt_spline_curve_fitter.cpp

src/plot/qwt_series_data.cpp
src/plot/qwt_sampling_thread.cpp
src/plot/qwt_round_scale_draw.cpp
src/plot/qwt_raster_data.cpp
src/plot/qwt_polar_marker.cpp
src/plot/qwt_polar_layout.cpp
src/plot/qwt_polar_fitter.cpp
src/plot/qwt_point_polar.cpp
src/plot/qwt_point_mapper.cpp
src/plot/qwt_point_data.cpp
src/plot/qwt_plot_zoneitem.cpp
src/plot/qwt_plot_transparent_canvas.cpp
src/plot/qwt_plot_tradingcurve.cpp
src/plot/qwt_plot_textlabel.cpp
src/plot/qwt_plot_svgitem.cpp
src/plot/qwt_plot_spectrocurve.cpp
src/plot/qwt_plot_seriesitem.cpp
src/plot/qwt_plot_shapeitem.cpp
src/plot/qwt_plot_series_data_picker.cpp
src/plot/qwt_plot_scale_event_dispatcher.cpp
src/plot/qwt_plot_rasteritem.cpp
src/plot/qwt_plot_magnifier.cpp
src/plot/qwt_plot_layout_engine.cpp
src/plot/qwt_plot_layout.cpp
src/plot/qwt_plot_grid.cpp
src/plot/qwt_plot_graphicitem.cpp
src/plot/qwt_plot_canvas_zoomer.cpp
src/plot/qwt_plot_abstract_canvas.cpp
src/plot/qwt_plot_abstract_barchart.cpp
src/plot/qwt_pixel_matrix.cpp
src/plot/qwt_picker_machine.cpp
src/plot/qwt_parasite_plot_layout.cpp
src/plot/qwt_painter_command.cpp
src/plot/qwt_matrix_raster_data.cpp
src/plot/qwt_math.cpp
src/plot/qwt_legend_data.cpp
src/plot/qwt_knob.cpp
src/plot/qwt_interval_symbol.cpp
src/plot/qwt_interval.cpp
src/plot/qwt_grid_raster_data.cpp
src/plot/qwt_figure_layout.cpp
src/plot/qwt_figure.cpp
src/plot/qwt_dial_needle.cpp
src/plot/qwt_date_scale_engine.cpp
src/plot/qwt_date_scale_draw.cpp
src/plot/qwt_curve_fitter.cpp
src/plot/qwt_counter.cpp
src/plot/qwt_compass_rose.cpp
src/plot/qwt_column_symbol.cpp
src/plot/qwt_color_map.cpp
src/plot/qwt_canvas_picker.cpp
src/plot/qwt_bezier.cpp
```

## 批次处理策略
由于项目庞大，将按以下批次处理：
1. 每批次处理 5-10 个文件（包括对应的.h 和.cpp）
2. 每次处理完成后更新此文件
3. 下次执行时从此文件读取已处理文件列表

## 注释格式示例

### .cpp 文件注释格式
```cpp
/**
 * \if ENGLISH
 * @brief [English Brief]
 * @param [param_name] [English Description]
 * @details [English Details]
 * \endif
 *
 * \if CHINESE
 * @brief [中文简要]
 * @param [param_name] [中文描述]
 * @details [中文详情]
 * \endif
 */
```

### .h 文件注释格式
```cpp
// Header file - English brief only for public methods
class MyClass {
public:
    /// Constructor for MyClass (English only)
    MyClass();
};

// For class documentation (bilingual)
/**
 * \if ENGLISH
 * @brief MyClass class description
 * \endif
 * \if CHINESE
 * @brief MyClass 类描述
 * \endif
 */

// For Qt signals (bilingual)
/**
 * \if ENGLISH
 * @brief Signal emitted when something happens
 * \endif
 * \if CHINESE
 * @brief 当某事发生时发出的信号
 * \endif
 */
Q_SIGNALS:
    void somethingHappened();
```

## 下次执行说明
1. 读取此文件查看已处理文件
2. 从未处理列表中选择下一批文件
3. 处理完成后更新此文件
4. 提交 Git 时明确列出改动的文件

## 特殊说明
- **忽略的文件**: `src-amalgamate/` 目录下的合并文件不处理
- **代码安全**: 严禁修改任何代码逻辑，仅修改注释
