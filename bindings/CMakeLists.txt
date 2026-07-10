# bindings/CMakeLists.txt
# PySide6 / Shiboken6 bindings for Qwt.
# This POC binds a single class (QwtInterval) to validate the toolchain.

cmake_minimum_required(VERSION 3.5)

# ---- Dependencies -----------------------------------------------------------
# Qt6 must already be found by the top-level CMakeLists.txt (find_package(QT NAMES Qt6 ...)).

find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Locate shiboken6_generator (pip-installed) and add its cmake dir to CMAKE_PREFIX_PATH.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(PySide6Shiboken REQUIRED)

# Now Shiboken6Tools is discoverable.
find_package(Shiboken6Tools REQUIRED)

# PySide6 include dirs (set by Shiboken6Tools as pyside_include_dir).
# For core we only need QtCore + QtGui headers on the parse side.
foreach(_inc_dir ${pyside_include_dir})
    list(APPEND _qwt_pyside_includes "${_inc_dir}/QtCore")
    list(APPEND _qwt_pyside_includes "${_inc_dir}/QtGui")
endforeach()

# ---- Binding module: qwtcore ------------------------------------------------
set(_qwt_bindings_lib "qwtcore")
set(_qwt_global_header ${CMAKE_CURRENT_SOURCE_DIR}/core/global.h)
set(_qwt_typesystem ${CMAKE_CURRENT_SOURCE_DIR}/core/typesystem_qwtcore.xml)

# Shiboken generates one wrapper .cpp per type + one module wrapper.
# For the POC there is only QwtInterval.
set(_qwt_generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/${_qwt_bindings_lib}/qwtcore_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/${_qwt_bindings_lib}/qwtinterval_wrapper.cpp
)

shiboken_generator_create_binding(
    EXTENSION_TARGET  ${_qwt_bindings_lib}
    GENERATED_SOURCES ${_qwt_generated_sources}
    HEADERS           ${_qwt_global_header}
    TYPESYSTEM_FILE   ${_qwt_typesystem}
    LIBRARY_TARGET    qwt::core
    QT_MODULES        Core Gui
)

# Generation-time defines: make shiboken see the python-wrapper hooks + import-side exports.
target_compile_definitions(${_qwt_bindings_lib} PRIVATE
    QWT_PYTHON_WRAPPER
    QWTCORE_DLL
    QT_NO_KEYWORDS
)

# Output next to the qwt DLLs so the test harness can find both.
set_target_properties(${_qwt_bindings_lib} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
