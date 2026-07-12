"""pytest config for qwtcore binding tests.

Ensures the built qwtcore extension module is importable by adding the
build output directory to sys.path, and forces offscreen Qt platform.
"""
import os
import sys

# Force offscreen so tests run in headless CI.
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

# The CMake build outputs qwtcore.pyd/.so to <build>/bin.
# Allow override via QWT_BINDINGS_BIN env var; fall back to a sibling of source tree.
_BINDINGS_BIN = os.environ.get("QWT_BINDINGS_BIN")
if not _BINDINGS_BIN:
    # Default: build/bin relative to repo root (two levels up from this file).
    _REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
    _BINDINGS_BIN = os.path.join(_REPO_ROOT, "build", "bin")

if os.path.isdir(_BINDINGS_BIN) and _BINDINGS_BIN not in sys.path:
    sys.path.insert(0, _BINDINGS_BIN)

# On Windows the qwtcore.pyd depends on qwtcore.dll, the Qt6 DLLs, and the
# PySide6 / shiboken6 abi3 DLLs. The VS multi-config generator nests the .pyd
# under <build>/bin/<Config>/; register the relevant DLL directories so the
# import succeeds without requiring the caller to set PATH first.
if sys.platform == "win32" and os.path.isdir(_BINDINGS_BIN):
    for _dll_dir in (_BINDINGS_BIN,):
        try:
            os.add_dll_directory(_dll_dir)
        except OSError:
            pass
    # The qwtcore.pyd depends on qwtcore.dll (in _BINDINGS_BIN), the Qt6 DLLs,
    # and the PySide6/shiboken6 abi3 DLLs (in their site-packages dirs).
    # Register the Qt bin dir (override via QT_BIN_DIR) and the PySide6 /
    # shiboken6 package dirs so Windows can locate all the .pyd's dependencies.
    _extra_dll_dirs = []
    _qt_bin = os.environ.get("QT_BIN_DIR")
    if _qt_bin and os.path.isdir(_qt_bin):
        _extra_dll_dirs.append(_qt_bin)
    # Derive the PySide6 / shiboken6 package dirs from their importable paths.
    try:
        import PySide6 as _ps
        _extra_dll_dirs.append(os.path.dirname(_ps.__file__))
    except ImportError:
        pass
    try:
        import shiboken6 as _sb
        _extra_dll_dirs.append(os.path.dirname(_sb.__file__))
    except ImportError:
        pass
    for _dll_dir in _extra_dll_dirs:
        try:
            os.add_dll_directory(_dll_dir)
        except OSError:
            pass
    # Pre-load PySide6 / shiboken6 so their abi3 runtime DLLs are resident
    # before the qwtcore extension is imported (mirrors how a normal PySide6
    # application starts up).
    try:
        import PySide6  # noqa: F401
        import shiboken6  # noqa: F401
    except ImportError:
        pass
