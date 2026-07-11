# FindPySide6Shiboken.cmake
#
# Locates the pip-installed shiboken6_generator (the shiboken6.exe binary, the
# shiboken headers, the PySide6 headers + typesystems + import libs) and exposes
# a vendored Shiboken6Tools CMake config so that a subsequent
#   find_package(Shiboken6Tools REQUIRED)
# succeeds and provides the `shiboken_generator_create_binding` macro.
#
# IMPORTANT / DEVIATION FROM ORIGINAL BRIEF:
#   The original brief assumed the pip wheel ships
#     <site-packages>/shiboken6_generator/lib/cmake/Shiboken6Tools/Shiboken6ToolsConfig.cmake
#   and the `shiboken_generator_create_binding` macro. That is NOT true for the
#   PySide6 / shiboken6-generator pip wheels (verified for 6.7.3 and 6.8.3):
#   the wheels ship the shiboken6.exe binary, libclang, the shiboken headers,
#   and PySide6's headers/typesystems, but NO CMake config and NO macro. Those
#   only exist in a source-built PySide6 (pyside-setup build tree).
#   To keep bindings/CMakeLists.txt verbatim (find_package(Shiboken6Tools) +
#   shiboken_generator_create_binding), this module points Shiboken6Tools_DIR at
#   a vendored shim config (bindings/cmake/Shiboken6Tools/Shiboken6ToolsConfig.cmake)
#   that defines the macro and the PySide6 / Shiboken6 imported targets.
#
# Inputs: Python_EXECUTABLE must already be set by the caller via
#   find_package(Python COMPONENTS Interpreter Development REQUIRED)
#
# Outputs (variables):
#   SHIBOKEN6_EXECUTABLE          - path to shiboken6.exe
#   SHIBOKEN6_INCLUDE_DIR         - shiboken headers (shiboken.h etc.)
#   PYSIDE6_INCLUDE_DIR           - PySide6 include root (has QtCore/, QtGui/, pyside*.h)
#   PYSIDE6_TYPESYSTEMS_DIR       - dir with typesystem_core.xml etc.
#   PYSIDE6_LIBRARIES             - PySide6 import lib (pyside6.abi3.lib)
#   SHIBOKEN6_LIBRARIES           - shiboken6 import lib (shiboken6.abi3.lib)
#   pyside_include_dir            - single-dir list consumed by bindings/CMakeLists.txt
#   Shiboken6Tools_DIR            - set so find_package(Shiboken6Tools) finds the shim
#   PySide6Shiboken_FOUND         - set to TRUE

# Require a Python interpreter to be found first by the caller.
if(NOT Python_EXECUTABLE)
    message(FATAL_ERROR "FindPySide6Shiboken requires Python_EXECUTABLE to be set first "
                        "(run find_package(Python COMPONENTS Interpreter Development REQUIRED)).")
endif()

# Resolve site-packages (handles lib vs lib64 on distros like RHEL).
execute_process(
    COMMAND ${Python_EXECUTABLE} -c
    "import site; print(next(p for p in site.getsitepackages() if 'site-packages' in p))"
    OUTPUT_VARIABLE _qwt_python_sitelib
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _qwt_sitelib_result
)
if(NOT _qwt_sitelib_result EQUAL 0)
    message(FATAL_ERROR "Could not determine Python site-packages directory.")
endif()
message(STATUS "Qwt bindings: Python site-packages = ${_qwt_python_sitelib}")

set(_qwt_sbg_dir "${_qwt_python_sitelib}/shiboken6_generator")
set(_qwt_pyside_dir "${_qwt_python_sitelib}/PySide6")
set(_qwt_shiboken_rt_dir "${_qwt_python_sitelib}/shiboken6")

# shiboken6.exe + shiboken headers live in the shiboken6_generator package.
find_program(SHIBOKEN6_EXECUTABLE NAMES shiboken6 shiboken6.exe
    HINTS "${_qwt_sbg_dir}" NO_DEFAULT_PATH)
if(NOT SHIBOKEN6_EXECUTABLE)
    message(FATAL_ERROR
        "shiboken6 executable not found in ${_qwt_sbg_dir}.\n"
        "Install it with: pip install shiboken6-generator")
endif()

find_path(SHIBOKEN6_INCLUDE_DIR NAMES shiboken.h HINTS "${_qwt_sbg_dir}/include" NO_DEFAULT_PATH)
find_file(SHIBOKEN6_LIBRARIES NAMES shiboken6.abi3.lib shiboken6.lib
    HINTS "${_qwt_shiboken_rt_dir}" NO_DEFAULT_PATH)

find_path(PYSIDE6_INCLUDE_DIR NAMES pyside6_global.h HINTS "${_qwt_pyside_dir}/include" NO_DEFAULT_PATH)
find_path(PYSIDE6_TYPESYSTEMS_DIR NAMES typesystem_core.xml
    HINTS "${_qwt_pyside_dir}/typesystems" NO_DEFAULT_PATH)
find_file(PYSIDE6_LIBRARIES NAMES pyside6.abi3.lib pyside6.lib
    HINTS "${_qwt_pyside_dir}" NO_DEFAULT_PATH)

foreach(_need SHIBOKEN6_INCLUDE_DIR SHIBOKEN6_LIBRARIES PYSIDE6_INCLUDE_DIR
                 PYSIDE6_TYPESYSTEMS_DIR PYSIDE6_LIBRARIES)
    if(NOT ${_need})
        message(FATAL_ERROR "FindPySide6Shiboken: required path ${_need} not found under ${_qwt_python_sitelib}. "
                            "Reinstall PySide6 / shiboken6-generator.")
    endif()
endforeach()

# Single-element list consumed verbatim by bindings/CMakeLists.txt
# (the foreach appends <dir>/QtCore and <dir>/QtGui).
set(pyside_include_dir "${PYSIDE6_INCLUDE_DIR}")

# Point Shiboken6Tools discovery at our vendored shim config directory.
# This makes the subsequent `find_package(Shiboken6Tools REQUIRED)` pick up
# bindings/cmake/Shiboken6Tools/Shiboken6ToolsConfig.cmake.
set(Shiboken6Tools_DIR "${CMAKE_CURRENT_LIST_DIR}/Shiboken6Tools" CACHE PATH
    "Path to the vendored Shiboken6Tools cmake shim config")

message(STATUS "Qwt bindings: shiboken6 exe   = ${SHIBOKEN6_EXECUTABLE}")
message(STATUS "Qwt bindings: shiboken include = ${SHIBOKEN6_INCLUDE_DIR}")
message(STATUS "Qwt bindings: PySide6 include = ${PYSIDE6_INCLUDE_DIR}")
message(STATUS "Qwt bindings: PySide6 typesys = ${PYSIDE6_TYPESYSTEMS_DIR}")
message(STATUS "Qwt bindings: Shiboken6Tools   = ${Shiboken6Tools_DIR} (vendored shim)")

set(PySide6Shiboken_FOUND TRUE)
