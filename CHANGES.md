## tag:v7.3.2 (2026-06-20)

### New Features

- **QwtColorMapPreset Applied to Spectrograms**
    - `QwtPlotSpectrogram` and `QwtPolarSpectrogram` now use Viridis as the default colormap, replacing the legacy blue-to-yellow linear map
    - Spectrogram and polar spectrogram examples updated with preset switching UI controls
    - Classic colormap types remain available for backward compatibility

- **QwtPolarPlot Background Unification**
    - Unified `QwtPolarPlot` background behavior with `QwtPlot`

### Changes

- **3D Default Colormap Migrated to Viridis**
    - `StandardColor::reset()` replaced blue-to-red gradient with viridis preset
    - `Qwt3DTheme` default preset changed from "jet" to "viridis" for consistency with modern visualization standards
    - Refreshed 3D screenshot assets to reflect the updated default colormap

### Documentation

- **Documentation Restructure** — flattened directory hierarchy, added 40+ new user guides
    - Covers curves, charts, 3D plots, interaction tools, and widgets
    - New developer guides: coding standards, comment conventions, PIMPL pattern usage
    - Updated mkdocs.yml navigation for the new flat structure with i18n folder organization
    - Unified screenshot and example path links

## tag:v7.3.1 (2026-06-17)

### New Features

- **qwt::core Shared Library**
    - Extracted `qwt::core` shared library (`qwtcore.dll`) containing color mapping and palette utilities
    - `QwtColorMap` refactored: removed `QwtInterval` dependency, new signature `rgb(vMin, vMax, value)`
    - `QwtColorCycle` migrated from plot module to core
    - New `QwtColorMapPreset` class with 22 scientific colormap presets (viridis, plasma, inferno, magma, cividis, jet, hot, cool, spring, summer, autumn, winter, gray, bone, copper, rainbow, hsv, turbo, coolwarm, rdylbu, rdylgn, spectral)
    - Backward-compatible `QwtColorMapCompat` namespace in `src/plot/qwt_colormap_compat.h` providing `QwtInterval`-based overloads

- **3D Theme System**
    - New `Qwt3DTheme` class with 10 preset themes: Default, Dark, Scientific, Warm, Cool, Matplotlib, EarthTones, Ocean, HighContrast, Presentation
    - New `ColorMapColor` adapter bridging 2D colormaps to 3D color functors
    - 5 lighting presets: NoLighting, FlatLight, Studio, Outdoor, Soft
    - `Plot3D::setTheme()`, `Plot3D::applyTheme()` for one-call theme application
    - `StandardColor::setPreset()` for quick colormap preset switching

### Architecture Changes

- Three-module architecture: `qwtcore.dll` → `qwtplot.dll` + `qwtplot3d.dll`
- `plot` and `plot3d` both link against `core` but remain independent of each other
- Runtime deployment requires `qwtcore.dll` + `qwtplot.dll` (+ `qwtplot3d.dll` if 3D enabled)
- Backward-compatible forwarding headers in `src/plot/` for existing `#include "qwt_color_map.h"` paths
- **Core Module Expansion**: Migrated 24 utility modules from `plot` to `core`, expanding core from 4 modules (color + global) to a complete foundational library of 28 modules:
    - Math utilities: `qwt_math.h/.cpp`, `qwt_simd_argminmax.h/.cpp`
    - Data types: `QwtInterval`, `QwtPoint3D`, `QwtPointPolar`, `QwtSamples`, `QwtBoxStatistics`
    - Geometry: `QwtBezier`, `QwtClipper`
    - Coordinate transforms: `QwtTransform`, `QwtScaleMap`, `QwtScaleDiv`, `QwtScaleEngine`
    - Date/Time: `QwtDate`, `QwtSystemClock`
    - Algorithms & compatibility: `qwt_algorithm.hpp`, `qwt_qt5qt6_compat.hpp`
    - Data containers: `QwtGridData`
    - Data series: `QwtSeriesData`, `QwtPointData`, `QwtSeriesStore`
    - Raster data: `QwtRasterData`, `QwtMatrixRasterData`, `QwtGridRasterData`

### Build System

- New `src/core/CMakeLists.txt` for the core library target
- Updated `src/plot/CMakeLists.txt` to link against `qwt::core`
- Updated `src/plot3d/CMakeLists.txt` to link against `qwt::core` and include theme sources
- Updated amalgamate tool templates to include core module files
- Updated `classincludes/` headers to point to core module paths
- Updated `tools/make-classinclude.py` for new module structure

### Examples

- **figureSurface3D** — added theme switching UI and fixed color legend refresh on theme change

### Bug Fixes

- Fixed 3D animation auto-start crash: deferred animation timer to `showEvent()` to prevent GL context access before widget is visible
- Fixed `figureSurface3D` color legend not refreshing when theme is switched

## tag:v7.3.0 (2026-06-12)

### New Features

- **QwtColorCycle — Color Cycle System**
    - Added `QwtColorCycle` class for managing color palettes with cyclic wrap-around, 7 built-in presets plus custom palettes
    - Integrated with `QwtPlot` — plot items automatically acquire colors from the color cycle on `attach()`
    - Supported items: `QwtPlotCurve`, `QwtPlotMultiBarChart`, `QwtPlotBoxChart`, `QwtPlotHistogram`, `QwtPlotIntervalCurve`, `QwtPlotBarChart`
    - Added color cycle unit tests

- **Curve Downsampling Filters**
    - Added `FilterPointsPixel` and `FilterPointsLTTB` paint attributes to `QwtPlotCurve`
        - `FilterPointsPixel`: pixel-column based downsampling for maximum rendering speed on very large datasets
        - `FilterPointsLTTB`: simplified LTTB (MinMax bucket) algorithm that preserves visual shape while downsampling
    - Changed default paint attribute from `FilterPoints` to `FilterPointsAggressive`
    - `qwtMapPointsQuad` improved NaN/Inf skipping logic and polygon memory pre-allocation

- **SIMD-Accelerated MinMax Bucket Downsampling**
    - Added `qwt_simd_argminmax` module providing SIMD-accelerated argmin/argmax computation
    - Supports SSE2/AVX2/NEON instruction sets with automatic runtime detection
    - Significantly boosts MinMax bucket downsampling performance

- **Picker Enhancements**
    - `QwtPlotSeriesDataPickerGroup` now provides click position (screen and data coordinates) on click events
    - `QwtPlotSeriesDataPicker` extended to handle all `QwtPlotSeriesItem` subtypes:
        - Trading curves (candlestick), interval curves, histograms, vector fields
        - Bar charts, box charts, spectro curves
    - `FeaturePoint` gained a `sampleData` field (QVariant) carrying full sample data (OHLC, interval, vector, etc.)
    - Sample types registered via `Q_DECLARE_METATYPE` for cross-module passing

- **Flat Style Controls**
    - `QwtSimpleCompassRose` — new `flatStyle` property replaces classic two-tone 3D effect with solid colors
    - `QwtWheel` — new `flatStyle` property replaces 3D gradients and borders with flat colors and simple lines
    - `QwtSlider` and `QwtThermo` — new `flatStyle` property for modern flat UI themes
    - `QwtDialNeedle` — multiple flat-style needle designs added
    - `QwtKnob` — defaults to `Flat` mode

### Performance

- `qwtProbeOrientation` and `qwtFindVisibleRange` optimized sampling strategy — probes up to 50 points with a minimum step of 1
- `qwtPixelColumnReduce` — pre-sized polyline vector, direct pointer assignment instead of dynamic appends
- Bins initialization simplified via `QVector` constructor

### Refactoring

- **PIMPL Macro Unification** (180+ files)
    - All classes migrated from manual PIMPL (raw `PrivateData* m_data`) to standard macro pattern
    - Headers use `QWT_DECLARE_PRIVATE(ClassName)`, implementations use `QWT_DECLARE_PUBLIC` + `QWT_PIMPL_CONSTRUCT` + `QWT_D/QWT_DC`
    - `unique_ptr` manages lifetime automatically; all `delete m_data` removed
    - Covers all modules: plot core, axes, text/legend, color maps, canvas, bar charts, interaction, data, splines, widgets, polar, etc.

- **3D Plot Module Overhaul** (53 files)
    - Added `override` specifiers to all virtual functions and destructors
    - `nullptr` replaces `0` for null pointer constants
    - `SurfacePlot` refactored to PIMPL pattern (`QWT_DECLARE_PRIVATE`)
    - `qwt3d_ptr` renamed to `ClonePtr`, unified include guards

- **Modern C++ Adoption**
    - `auto` keyword and range-based for loops throughout the codebase
    - Move semantics and C++11 default member initializers
    - `noexcept` added to trivial getters, setters, and constructors
    - `constexpr` expanded to intervals, axes, scale maps, and math utilities
    - C++ idiom modernization across `src/plot` and `src/plot3d`

### New Examples

- **renderbench** — curve rendering performance benchmark
    - `WindowedSeriesData` for real-time data stream simulation
    - Control panel for adjusting data size, downsampling mode, line width, etc.
    - Generates Markdown-formatted performance reports
    - Located at `examples/bench/renderbench/`
- **staticExample** — rewritten as comprehensive Qwt feature showcase
    - Demonstrates curves, spectrograms, bar charts, box charts, vector fields, and more
    - Modernized color palette and canvas backgrounds across all 2D examples

### Documentation

- Complete curve downsampling algorithm documentation
- Added `QWEN.md` AI Agent guidelines, updated build instructions
- Added English documentation and restructured bilingual documentation site
- Enforced English-only Doxygen comments across all source files
- Large-scale bilingual Doxygen comment completion (40+ batches) covering all core headers

### Bug Fixes

- Fixed Qt6 legend icon appearing blank due to `QwtNullPaintDevice::metric()` returning 0 for device pixel ratio
- Fixed `QwtPlotMarker` arrow endpoints — head and tail were swapped; improved legend icon rendering
- Fixed plot background rendering — now uses dynamic `backgroundRole` from `QPalette::Base` for correct theme-aware coloring

## tag:v7.2.1

- **Inside Tick Display**
    - Added `QwtPlot::TickDirection` enum: `TickOutside` (default) and `TickInside`
    - Added `QwtPlot::setAxisTickDirection()` and `QwtPlot::axisTickDirection()` to control tick direction per axis
    - Inside ticks extend from the canvas edge inward; the backbone and labels remain outside the canvas
    - Inside and outside ticks share the same style settings (length, color, pen width)
    - New example: `examples/2D/ticks_inside/`
    ![ticks_inside](./docs/assets/screenshots/ticks_inside.png)

## tag:v7.2.0

- **Box Chart (Box-and-Whisker Plot)**
    - Added `QwtPlotBoxChart` class for box-and-whisker plots
    - Added `QwtBoxStatisticsCalculator` for automatic statistics computation
    - New data structures: `QwtBoxSample`, `QwtBoxOutlierSample`, `QwtBoxChartData`, `QwtBoxOutlierChartData`
    - Supports both pre-computed and raw data input
    - Box styles: Rect, Diamond, Notch
    - Whisker methods: Tukey (1.5×IQR), percentile, min-max, standard deviation, standard error
    - Vertical and horizontal orientations
    - Automatic outlier detection with custom symbols and jitter display
    - New example: `examples/2D/boxchart/`
    ![Box Chart](./docs/assets/screenshots/BoxChart.png)

## tag:v7.0.8

- `QwtFigure` axis alignment — added `addAxisAlignment` to align sub-plot axes
    ![axis-alignment](./docs/assets/picture/figure-scale-aligment.png)
- `QwtPointMapper` NaN/Inf handling — prevents coordinate mapping corruption from abnormal data values
- `qwt_series_data.cpp` — NaN/Inf checks added to data range computation
- Fixed missing `QwtSlider` class in amalgamated file (Issue #4)

## tag:v7.0.7

- Moved `QwtScaleWidget::panScale` to `QwtPlot` as `panAxis`; fixed panning on logarithmic axes
- Renamed panner classes to reflect caching behavior:
    - `QwtPanner` → `QwtCachePanner`
    - `QwtPlotPanner` → `QwtPlotCachePanner`
    - `QwtPolarPanner` → `QwtPolarCachePanner`

    `QwtPlotPanner` (new, real-time):
    ![series-data-picker-yvalue](./docs/assets/screenshots/qwt-realtime-panner.gif)

- Rewrote `QwtPlotPanner` — now inherits `QwtPicker` for real-time dragging; original cached panner available as `QwtPlotCachePanner`
- Added `QwtCanvasPicker` base class for canvas-specific picker operations
- `QwtPlotPanner` supports linear, logarithmic, and multi-axis real-time panning
- Added `QwtPlotCanvasZoomer` — zooms the entire canvas without axis binding, supports multi-axis zoom
- Renamed `QwtPlotZoomer` → `QwtPlotAxisZoomer`
- `QwtPlotSeriesDataPicker` now inherits `QwtPicker` directly (no longer `QwtPlotPicker`)
- `QwtPlotMagnifier` supports multi-axis scaling
- `QwtScaleMap` — added move semantics
- Added `make-classinclude.py` for exporting the `classincludes` directory

v7.0.7 naming changes:

| Original Name | New Name | Note |
|:--|:--|:--|
| QwtPlotZoomer | QwtPlotAxisZoomer | Renamed — original could only bind 2 axes |
| (new) | QwtPlotCanvasZoomer | Whole-canvas zoomer |
| QwtPanner | QwtCachePanner | Original cannot drag in real-time, hence "Cache" |
| QwtPlotPanner | QwtPlotCachePanner | Same as above |
| QwtPolarPanner | QwtPolarCachePanner | Same as above |
| (new) | QwtPlotPanner | New real-time panner replacing the renamed one |

## tag:v7.0.6

- Added `QwtPlotSeriesDataPicker` for plot data picking
    ![series-data-picker-yvalue](./docs/assets/picture/series-data-picker-yvalue.gif)
    ![series-data-picker-nearest-value](./docs/assets/picture/series-data-picker-nearest-value.gif)
- Improved parasite plot refresh mechanism — full update on construction
- Changed some interface index types from `int` to `size_t`
- `QwtPlotSeriesDataPicker` supports date axis display
- Fixed `QwtFigureWidgetOverlay` event handling conflicting with axis actions

## tag:v7.0.5

- `QwtScaleWidget` built-in axis actions — pan and zoom
    - Added `scaleRect()` to get the tick region rectangle
    - Added `QwtScaleWidget::BuiltinActions` enum
    - Added `requestScaleRangeUpdate` signal for axis range refresh
    - Added `setSelected`/`isSelected` and `selectionChanged` signal
    - Added action properties: `zoomFactor`, `panScale`, etc.
- Added `QwtPlotScaleEventDispatcher` for axis event handling on plots
- Adjusted `QwtPlot` parasite plot interfaces
- Added `setEnableScaleBuildinActions`/`isEnableScaleBuildinActions`/`setupScaleEventDispatcher` to `QwtPlot`
- Added `QwtPlot::plotList()` — returns all plot objects including parasite and host plots

## tag:v7.0.4

- Implemented parasite plot functionality
- Added parasite plot example
- Added `QwtPlot::updateAxisEdgeMargin` for automatic axis positioning with parasite plots

## tag:v7.0.2

- Extracted `QwtPlotLayoutEngine` class
- Added `QwtPlotParasiteLayout` for parasite axis layout
- Added `QwtPlotTransparentCanvas` — fully transparent canvas
- `QwtFigure` — parasite axis support
- `QwtScaleWidget` — added `setEdgeMargin`/`edgeMargin` for tick-to-edge distance
- Adjusted `QwtScaleWidget` layout to support `edgeMargin`
- Added `parasitePlot` example
- `QwtPlot` — added `rescaleAxes`/`syncAxis` for quick axis adjustment
- NaN and Inf exception handling

## tag:v7.0.1

- Added `QwtFigure` — manages multiple `QwtPlot` instances in a grid layout (similar to Matplotlib Figure)

![figure](docs/assets/screenshots/qwt_figure.png)

- Added `figure` example demonstrating `QwtFigure` usage
- Added `QwtFigureWidgetOverlay` for interactive actions on `QwtFigure` — supports dragging and zooming sub-plots

## tag:v7.0.0

- Amalgamated entire project into single `QwtPlot.h` / `QwtPlot.cpp` files in `src-amalgamate/`
- Added `staticExample` example
- Enhanced `QwtPlotBarChart` interface to support pen and brush configuration
- Added `QwtGridRasterData` — supports 2D data tables with grid interpolation on X/Y axes (vs. `QwtMatrixRasterData`)
- `QwtLinearColorMap` — added `stopColors()`, renamed `colorStop()` → `stopPos()`
- Adjusted default initialization for a more modern look

`QwtPlotCanvas` initialization:
```cpp
QwtPlotCanvas::QwtPlotCanvas(QwtPlot* plot) : QFrame(plot), QwtPlotAbstractCanvas(this)
{
    ...

    setLineWidth(0);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::Box);
}
```

`QwtPlotLayout` initialization:
```cpp
QwtPlotLayout::QwtPlotLayout()
{
...
    setCanvasMargin(-1);
...
}
```

- Removed `QWT_MOC_INCLUDE`
- Adjusted class implementations for single-file amalgamation

------

**Qwt 7.0** is a modified version based on Qwt 6.2.0 source code. It complies with Qwt's open-source license, and the modifications are released as open-source.

Starting from this version, Qwt supports:
- The latest **C++11 standard**
- **CMake build system**

**Modifications in this version:**
1. Enhanced `QwtPlotBarChart` interface to support **pen and brush configuration**
2. Added `QwtGridRasterData` class — supports **2D data tables** with **grid interpolation** on X/Y axes (vs. `QwtMatrixRasterData`)
3. Added `stopColors()` function to `QwtLinearColorMap` and renamed `colorStop()` to `stopPos()`



Qwt 6.2.0

------

0) Requirement for Qt >= 4.8

1) Class Includes added

Include files, that match the class names are available now. So
it is possible to write "#include <QwtPlot>" now instead of "include qwt_plot.h"

2) BSD License for examples

Where possible the code of the examples is available under the 3-clause BSD License

3) MathML text renderer removed

The code can be found at https://github.com/uwerat/qwt-mml-dev now and is intended
to become a standalone lib. Anyone who is interested to work on it, please let me know.

4) Spline interpolation

The broken implementation of QwtSpline has been replaced by a bunch of classes
offering all sort of functionalities around splines.

The most popular spline approximation/interpolation algos have been implemented:

	- Basis
	- Cardinal
	- ParabolicBlending
	- Akima
	- The one used in MS Excel
	- Cubic

An implementation of the de Casteljau's algorithm has been added

	- QwtBezier

5) New plot items

	- QwtPlotVectorField
      A new type of plot item for vector fields

    - QwtPlotGraphicItem
      An item displaying a QwtGraphic image ( f.e used by QwtPlotSvgItem )

6) Plot Canvas

	- QwtAbstractPlotCanvas introduced
	- QwtPlotOpenGLCanvas added to support QOpenGLWidget

7) QwtPlotCurve

	- QwtPlotCurve::FilterPointsAggressive mode added - a fast weeding algo
      for huge datasets with increasing x or y values

	- QwtPlotCurve::closestPoint is virtual now
	- QwtPlotCurve: polygon clipping includes the painter clip
	- QwtPlotCurve::setLegendAttributes added
	- QwtValuePointData added for curves, where the x values are the index
	- a couple of new QwtPlotCurve::setSamples alternatives

8) QwtPlotSpectrogram

	- QwtPlotSpectrogram::setColorTableSize added
	- QwtRasterData::setInterval/interval changed into a pure virtual getter
	- QwtMatrixRasterData::BicubicInterpolation added
	- QwtMatrixRasterData::interval: API cleanup
	- QwtHueColorMap, QwtSaturationValueColorMap added

9) QwtPlotRenderer

	- using QPdfWriter where possible

10)
	- LOG_MIN/LOG_MAX removed, use QwtTransform::LogMin/LogMax instead ( values differ ! )
	- qwt_compat.h removed
	- qwtFuzzyGreaterOrEqual/qwtFuzzyLessOrEqual removed
	- qwtGetMin/qwtGetMax removed

11)
    - Not aligning unknown paint engines ( f.e EMF )
    - QwtNullPaintDevice is using a different type than QPaintEngine::User now

12) Many other changes ...
