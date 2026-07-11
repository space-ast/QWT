######################################################################
# Qwt Examples - Copyright (C) 2002 Uwe Rathmann
# This file may be used under the terms of the 3-clause BSD License
######################################################################

# This example demonstrates how to integrate Qwt into a qmake project
# using the amalgamated single file (src-amalgamate/QwtPlot.h +
# QwtPlot.cpp). The merged source is compiled directly into the
# executable; it does NOT link against the built qwt libraries.
#
# See README.md for build steps and docs/build-guide/qmake-single-file.md
# for a detailed explanation of every dependency and macro below.

TEMPLATE = app
TARGET   = qmakeExample
CONFIG  += warn_on

# ---------------------------------------------------------------------------
# Amalgamated single file
#   From examples/qmakeExample two levels up reach the repository root,
#   then enter src-amalgamate. QwtPlot.h goes to HEADERS so that qmake's
#   automatic moc processes it (it contains every Q_OBJECT declaration).
#   QwtPlot.cpp has no Q_OBJECT of its own, so no moc rule is needed for it.
# ---------------------------------------------------------------------------
AMALGAMATE_DIR = $$PWD/../../src-amalgamate

INCLUDEPATH += $$AMALGAMATE_DIR
DEPENDPATH  += $$AMALGAMATE_DIR

HEADERS += $$AMALGAMATE_DIR/QwtPlot.h
SOURCES += $$AMALGAMATE_DIR/QwtPlot.cpp

SOURCES += main.cpp

# ---------------------------------------------------------------------------
# Qt module dependencies
#   The amalgamation inlines the complete Qwt project (core + plot +
#   plot3d, including SVG, OpenGL and 3D support), so every module below
#   is mandatory and cannot be disabled. On Qt 6 the OpenGLWidgets module
#   is also required (Qt 5 uses the deprecated QGLWidget path).
# ---------------------------------------------------------------------------
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core gui widgets svg concurrent opengl printsupport openglwidgets
    CONFIG += c++17
} else {
    QT += core gui widgets svg concurrent opengl printsupport
    CONFIG += c++11
}

# ---------------------------------------------------------------------------
# System OpenGL (GL) and GLU libraries
#   The inlined 3D module calls gl* and glu* functions directly. GLU lives in
#   its own system library, and on some Qt builds (e.g. the ANGLE/OpenGL ES
#   based MSVC packages) Qt5OpenGL.lib does not transitively pull in the
#   desktop OpenGL library either, so both must be linked explicitly.
#   On macOS the OpenGL framework already provides GL and GLU through the Qt
#   OpenGL module above, so nothing extra is needed there.
# ---------------------------------------------------------------------------
win32 {
    LIBS += -lopengl32 -lglu32
}
unix:!macx {
    LIBS += -lGL -lGLU
}

# ---------------------------------------------------------------------------
# Compile macros -- DO NOT define the QWT_*_DLL family
#   Qwt's *_global.h headers turn the *_EXPORT macros into
#   __declspec(dllexport)/dllimport only when the matching *_DLL macro is
#   defined. Since the amalgamated source is compiled straight into this
#   executable (not into a DLL, and not consuming a DLL), every *_EXPORT
#   macro must stay empty. Therefore leave these undefined:
#       QWT_DLL / QWT_MAKEDLL        (plot module)
#       QWTCORE_DLL / QWTCORE_MAKEDLL (core module)
#       QWT3D_DLL / QWT3D_MAKEDLL    (3D module)
#
#   QWT3D_NODLL is provided as a safety override: it forcibly undefines the
#   3D DLL macros. Uncomment it only if your build environment predefines
#   QWT3D_DLL and you need to cancel that.
# ---------------------------------------------------------------------------
# DEFINES += QWT3D_NODLL
