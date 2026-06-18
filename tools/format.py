#!/usr/bin/env python3
"""
Run clang-format on Qwt source files under src/.

The formatting rules are taken from the .clang-format file at the repository
root (WebKit-based, indent 4, column 120). clang-format is invoked with
``--style=file`` so it discovers that file automatically; include order is
preserved (``SortIncludes: false``), so the project's manually ordered
includes are left untouched.

The repository root is derived from this script's location (tools/format.py),
so the script works correctly regardless of the current working directory.

Third-party vendored code (src/plot3d/3rdparty/gl2ps/) and the generated
header src/qwt_version_info.h are excluded from formatting.

examples:
    python tools/format.py                 # format all src/ files in place
    python tools/format.py -m core         # only src/core
    python tools/format.py -m core,plot    # src/core and src/plot
    python tools/format.py -c              # only working-tree-changed files
    python tools/format.py -c --base master
    python tools/format.py -n -c           # list changed files, no changes
"""
import argparse
import os
import shutil
import subprocess
import sys
import textwrap
from pathlib import Path

# --- repository layout (derived from this script's location) ----------------
REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = REPO_ROOT / "src"

# The three build modules under src/. Used for --module filtering.
MODULES = ("core", "plot", "plot3d")

# Source extensions clang-format knows how to handle.
EXTENSIONS_LOWER = {".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hh", ".hxx"}

# Directory names (any depth under src/) that are skipped: vendored third-party
# code, build artifacts, and VCS metadata.
EXCLUDE_DIRS_LOWER = {"3rdparty", "third_party", "build", "cmakefiles", ".git"}

# Files (relative to src/) that are skipped because they are generated.
# src/qwt_version_info.h is produced from qwt_version_info.h.in at configure time.
EXCLUDE_FILES_REL = {"qwt_version_info.h"}

# Maximum number of files passed to a single clang-format invocation. Keeps
# command lines well below the Windows 8191-character limit for ~400 files.
CHUNK_SIZE = 50

# Toggled in main(); gates ANSI color output.
_USE_COLOR = False


# --- ANSI color support ------------------------------------------------------
class _C:
    """ANSI escape sequences for colored console output."""

    RESET = "\033[0m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    CYAN = "\033[36m"


def _enable_vt100():
    """Enable ANSI escape processing on Windows 10+ consoles (no-op elsewhere)."""
    if os.name != "nt":
        return
    try:
        import ctypes
        from ctypes import wintypes

        kernel32 = ctypes.windll.kernel32
        # GetStdHandle(STD_OUTPUT_HANDLE) -> -11
        handle = kernel32.GetStdHandle(-11)
        mode = wintypes.DWORD()
        if not kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
            return
        ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
        kernel32.SetConsoleMode(handle, mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
    except Exception:
        # Color is cosmetic; never let this fail the run.
        pass


def log(level, msg):
    """Print a bracket-prefixed message.

    level is one of 'OK', 'WARN', 'ERROR', 'INFO'. WARN/ERROR go to stderr,
    OK/INFO go to stdout. Coloring is applied only when enabled.
    """
    color = {
        "OK": _C.GREEN,
        "WARN": _C.YELLOW,
        "ERROR": _C.RED,
        "INFO": _C.CYAN,
    }.get(level, "")
    prefix = "[{}]".format(level)
    if _USE_COLOR and color:
        line = "{}{}{} {}".format(color, prefix, _C.RESET, msg)
    else:
        line = "{} {}".format(prefix, msg)
    stream = sys.stderr if level in ("ERROR", "WARN") else sys.stdout
    print(line, file=stream)


# --- argument parsing --------------------------------------------------------
def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        prog="format.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Run clang-format on Qwt source files under src/ using "
        "the .clang-format at the repository root.",
        epilog=textwrap.dedent(
            """\
            examples:
              python tools/format.py                 # format all src/ files in place
              python tools/format.py -m core         # only src/core
              python tools/format.py -m core,plot    # src/core and src/plot
              python tools/format.py -c              # only working-tree-changed files
              python tools/format.py -c --base master
              python tools/format.py -n -c           # list changed files, no changes
            """
        ),
    )
    parser.add_argument(
        "-m",
        "--module",
        action="append",
        metavar="MOD",
        help="restrict to module(s): core, plot, plot3d. repeatable or "
        "comma-separated. default: all three",
    )
    parser.add_argument(
        "-c",
        "--changed",
        action="store_true",
        help="format only files changed in the working tree (vs --base), "
        "including untracked new files",
    )
    parser.add_argument(
        "--base",
        default="HEAD",
        metavar="REF",
        help="base git ref for --changed (default: HEAD). e.g. --base master "
        "or --base origin/master",
    )
    parser.add_argument(
        "--clang-format",
        default=None,
        metavar="PATH",
        help="path to clang-format executable (default: auto-detect on PATH)",
    )
    parser.add_argument(
        "-n",
        "--dry-run",
        action="store_true",
        help="list files that would be formatted; do not modify anything",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="print each file as it is processed",
    )
    parser.add_argument(
        "--no-color",
        action="store_true",
        help="disable ANSI color output",
    )
    return parser.parse_args(argv)


# --- helpers -----------------------------------------------------------------
def resolve_modules(module_args):
    """Normalize -m inputs into a de-duplicated list of valid module names.

    Exits with status 2 on an unknown module name.
    """
    if not module_args:
        return list(MODULES)
    result = []
    for item in module_args:
        for part in item.split(","):
            part = part.strip().lower()
            if not part:
                continue
            if part not in MODULES:
                log("ERROR", "unknown module: {!r}. valid: {}".format(part, ", ".join(MODULES)))
                sys.exit(2)
            if part not in result:
                result.append(part)
    return result


def find_clang_format(user_path):
    """Return the clang-format executable path, or None if not found."""
    if user_path:
        p = Path(user_path)
        return str(p) if p.exists() else None
    return shutil.which("clang-format")


def clang_format_version(exe):
    """Return the output of `clang-format --version`, or '' on failure."""
    try:
        r = subprocess.run([exe, "--version"], capture_output=True, text=True, timeout=10)
        return (r.stdout or "").strip()
    except Exception:
        return ""


def is_excluded(path, src_dir):
    """True if path is under an excluded directory or is an excluded file."""
    for part in path.parts:
        if part.lower() in EXCLUDE_DIRS_LOWER:
            return True
    try:
        rel = path.relative_to(src_dir).as_posix()
    except ValueError:
        return False
    return rel in EXCLUDE_FILES_REL


def collect_all(src_dir, modules):
    """Walk src/<module> for each selected module and return source files."""
    files = []
    for mod in modules:
        mod_dir = src_dir / mod
        if not mod_dir.is_dir():
            log("WARN", "module directory missing, skipping: {}".format(mod_dir))
            continue
        for p in mod_dir.rglob("*"):
            if not p.is_file():
                continue
            if p.suffix.lower() not in EXTENSIONS_LOWER:
                continue
            if is_excluded(p, src_dir):
                continue
            files.append(p.resolve())
    return files


def _git(repo_root, args):
    """Run a git command in repo_root, returning (returncode, stdout, stderr)."""
    cmd = ["git", "-C", str(repo_root)] + args
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    except FileNotFoundError:
        log("ERROR", "git not found on PATH; --changed requires git.")
        sys.exit(2)
    return r.returncode, r.stdout, r.stderr


def collect_changed(repo_root, src_dir, base, modules):
    """Return working-tree-changed source files (tracked changes vs base
    plus untracked new files), restricted to selected modules."""
    # Tracked changes (staged + unstaged) relative to base.
    rc, out, err = _git(repo_root, ["diff", "--name-only", base, "--", "src/"])
    if rc != 0:
        log("ERROR", "git diff failed (base={!r}): {}".format(base, (err or out).strip()))
        sys.exit(2)
    paths = out.splitlines()

    # Untracked new files (respects .gitignore).
    rc2, out2, _ = _git(repo_root, ["ls-files", "--others", "--exclude-standard", "--", "src/"])
    if rc2 == 0:
        paths.extend(out2.splitlines())

    files = []
    seen = set()
    for line in paths:
        line = line.strip()
        if not line:
            continue
        p = (repo_root / line).resolve()
        if not p.is_file():
            continue  # deleted files appear here but no longer exist
        if p.suffix.lower() not in EXTENSIONS_LOWER:
            continue
        try:
            rel = p.relative_to(src_dir)
        except ValueError:
            continue  # not under src/
        top = rel.parts[0] if rel.parts else ""
        if top not in modules:
            continue
        if is_excluded(p, src_dir):
            continue
        key = str(p)
        if key in seen:
            continue
        seen.add(key)
        files.append(p)
    return files


def _format_one(exe, repo_root, path, verbose):
    """Format a single file in place. Return True on success."""
    rel = path.relative_to(repo_root).as_posix()
    r = subprocess.run(
        [exe, "-i", "--style=file", rel],
        cwd=str(repo_root),
        capture_output=True,
        text=True,
    )
    if r.returncode != 0:
        msg = (r.stderr or "").strip().replace("\n", " ")[:200]
        log("ERROR", "{}: {}".format(rel, msg or "exit {}".format(r.returncode)))
        return False
    if verbose:
        print("  formatted {}".format(rel))
    return True


def isolate_failures(exe, repo_root, chunk, verbose):
    """Re-run clang-format per file to identify individual failures.

    Returns the number of files that failed.
    """
    fails = 0
    for path in chunk:
        if not _format_one(exe, repo_root, path, verbose):
            fails += 1
    return fails


def run_clang_format(exe, repo_root, files, verbose):
    """Format all files in place. Returns the count of failures."""
    failures = 0
    total = len(files)
    for i in range(0, total, CHUNK_SIZE):
        chunk = files[i : i + CHUNK_SIZE]
        rels = [f.relative_to(repo_root).as_posix() for f in chunk]
        cmd = [exe, "-i", "--style=file"] + rels
        try:
            r = subprocess.run(
                cmd, cwd=str(repo_root), capture_output=True, text=True
            )
        except Exception as e:
            log("ERROR", "failed to invoke clang-format: {}".format(e))
            failures += len(chunk)
            continue
        if r.returncode != 0:
            # clang-format -i returns nonzero when at least one file in the
            # batch is unparseable; re-run per file to report exactly which.
            stderr = (r.stderr or "").strip()
            if stderr:
                for line in stderr.splitlines():
                    log("ERROR", line)
            failures += isolate_failures(exe, repo_root, chunk, verbose)
            continue
        if verbose:
            for rel in rels:
                print("  formatted {}".format(rel))
    return failures


# --- main --------------------------------------------------------------------
def main(argv=None):
    args = parse_args(argv)

    global _USE_COLOR
    _USE_COLOR = sys.stdout.isatty() and not args.no_color
    if _USE_COLOR:
        _enable_vt100()

    if not SRC_DIR.is_dir():
        log("ERROR", "src directory not found: {}".format(SRC_DIR))
        return 2

    exe = find_clang_format(args.clang_format)
    if exe is None:
        log("ERROR", "clang-format not found. Install it or pass --clang-format PATH.")
        return 2
    log("INFO", "clang-format: {}".format(exe))
    version = clang_format_version(exe)
    if version:
        log("INFO", "version: {}".format(version))

    modules = resolve_modules(args.module)

    if args.changed:
        log("INFO", "mode: changed files (base={}), modules={}".format(args.base, ", ".join(modules)))
        files = collect_changed(REPO_ROOT, SRC_DIR, args.base, modules)
    else:
        log("INFO", "mode: all source files, modules={}".format(", ".join(modules)))
        files = collect_all(SRC_DIR, modules)

    log("INFO", "found {} file(s) to format".format(len(files)))

    if not files:
        log("OK", "nothing to do")
        return 0

    if args.dry_run:
        for f in files:
            print("  {}".format(f.relative_to(REPO_ROOT).as_posix()))
        log("OK", "dry run: {} file(s) listed, nothing modified".format(len(files)))
        return 0

    failures = run_clang_format(exe, REPO_ROOT, files, args.verbose)
    if failures:
        log("ERROR", "{} file(s) failed to format".format(failures))
        return 1
    log("OK", "formatted {} file(s)".format(len(files)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
