
# QWT CMake By Czyt1988

cmake_minimum_required(VERSION 3.5)

########################################################
# Version Definition
########################################################
set(QWT_VERSION_MAJOR 7)
set(QWT_VERSION_MINOR 1)
set(QWT_VERSION_PATCH 0)
set(QWT_VERSION "${QWT_VERSION_MAJOR}.${QWT_VERSION_MINOR}.${QWT_VERSION_PATCH}")
message(STATUS "QWT Version is ${QWT_VERSION}")


########################################################
# Project Definition
########################################################
set(QWT_PROJECT_NAME "qwt-project")

project(${QWT_PROJECT_NAME} 
        VERSION ${QWT_VERSION} 
        LANGUAGES CXX
        DESCRIPTION "Qwt - Qt Widgets for Technical Applications"
)

########################################################
# General Settings
########################################################
set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_RELWITHDEBINFO_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_MINSIZEREL_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")

# C++ standard, require C++17
set(CMAKE_CXX_STANDARD 17)
# Enforce the requirement
set(CMAKE_CXX_STANDARD_REQUIRED ON)

########################################################
# QWT Configuration
########################################################
option(QWT_CONFIG_QWTPLOT "Enable all classes needed to use QwtPlot" ON)
option(QWT_CONFIG_QWTPOLAR "Enable all classes needed for QwtPolar functionality. Requires QWT_CONFIG_QWTPLOT to be enabled" ON)
option(QWT_CONFIG_QWTWIDGETS "Enable all other widgets (sliders, dials, etc.) besides QwtPlot" ON)
option(QWT_CONFIG_QWTSVG "Enable SVG support for plot canvas and SVG export functionality" ON)
option(QWT_CONFIG_QWTOPENGL "Enable OpenGL plot canvas support" ON)
option(QWT_CONFIG_QWTPLOT_3D "Enable QwtPlot3D module" ON)

option(QWT_CONFIG_BUILD_EXAMPLE "Build example applications" ON)
option(QWT_CONFIG_BUILD_PLAYGROUND "Build playground applications" ON)
option(QWT_CONFIG_BUILD_STATIC_EXAMPLE "Build static examples (takes relatively long compilation time)" ON)

########################################################
# MSVC Settings
########################################################
if(MSVC)
    # Enable UTF-8 encoding for MSVC
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    # Ensure __cplusplus is defined when using MSVC and enable parallel build
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /MP")
endif()

########################################################
# Install Path Settings
########################################################
set(QWT_MIN_QT_VERSION 5.10)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)

# Determine platform architecture
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(QWT_PLATFORM_NAME "x86")
else()
    set(QWT_PLATFORM_NAME "x64")
endif()

set(QWT_BIN_DIR_NAME bin_qt${QT_VERSION}_${CMAKE_BUILD_TYPE}_${QWT_PLATFORM_NAME})


##################################
# Install
##################################
message(STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")

########################################################
# Source Directories
########################################################
add_subdirectory(src)

if(QWT_CONFIG_BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()
if(QWT_CONFIG_BUILD_PLAYGROUND)
    add_subdirectory(playground)
endif()

