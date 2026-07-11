// global.h — aggregated header for qwtcore Shiboken bindings.
// Shiboken parses this single translation unit to discover all types to wrap.
//
// Include order follows src/core/CMakeLists.txt QWTCORE_HEADER_FILES.
// Excludes: qwtcore_global.h (no types, just macros), qwt_math.h (C helpers,
//   qwtMinF etc. — bind only if needed later), qwt_algorithm.hpp (internal
//   template helper), qwt_qt5qt6_compat.hpp (compat shims), qwt_simd_argminmax.h
//   (internal SIMD), qwt_grid_data.hpp (5-param template, deferred per spec),
//   qwt_series_store.h (template, internal storage).

#ifndef QWTCORE_BINDING_GLOBAL_H
#define QWTCORE_BINDING_GLOBAL_H

// Some qwt headers forward-declare Qt value types (QPolygonF, QPolygon, QVector)
// without including their definitions. Shiboken needs the full type visible to
// synthesize constructors for value-returning signatures (e.g. QwtBezier::toPolygon
// returns QPolygonF by value). Pull in the QtGui definitions before the qwt headers.
#include <QPolygon>
#include <QPolygonF>
#include <QVector>

#include "qwt_colormap.h"
#include "qwt_color_cycle.h"
#include "qwt_colormap_preset.h"
#include "qwt_interval.h"
#include "qwt_point_3d.h"
#include "qwt_point_polar.h"
#include "qwt_system_clock.h"
#include "qwt_samples.h"
#include "qwt_bezier.h"
#include "qwt_clipper.h"
#include "qwt_date.h"
#include "qwt_transform.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_box_statistics.h"
#include "qwt_scale_engine.h"
#include "qwt_series_data.h"
#include "qwt_point_data.h"
#include "qwt_raster_data.h"
#include "qwt_matrix_raster_data.h"
#include "qwt_grid_raster_data.h"

#endif // QWTCORE_BINDING_GLOBAL_H
