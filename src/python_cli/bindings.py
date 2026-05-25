"""
MERO Compiler - C++ Native Bindings
ctypes interface to the MERO C++ compiler library.
Developer: MERO:TG@QP4RM
"""
import ctypes
import os
import sys
from pathlib import Path
from dataclasses import dataclass
from typing import Optional


class _MeroCompileResult(ctypes.Structure):
    _fields_ = [
        ("output", ctypes.c_char_p),
        ("success", ctypes.c_int),
        ("error_message", ctypes.c_char_p),
        ("output_length", ctypes.c_int),
    ]


@dataclass
class NativeCompileResult:
    output: str = ""
    success: bool = False
    error_message: str = ""


class MeroNativeCompiler:
    """Interface to the MERO C++ shared library."""

    def __init__(self, opt_level: int = 2):
        self._lib = self._load_library()
        self._setup_functions()
        self._handle = self._lib.mero_compiler_create(opt_level)

    def __del__(self):
        if hasattr(self, "_handle") and self._handle:
            self._lib.mero_compiler_destroy(self._handle)

    def compile(self, source: str) -> NativeCompileResult:
        """Compile Python source to Luau using the native backend."""
        source_bytes = source.encode("utf-8")
        raw_result = self._lib.mero_compiler_compile(
            self._handle, source_bytes, len(source_bytes)
        )

        result = NativeCompileResult()
        result.success = bool(raw_result.success)

        if result.success:
            result.output = raw_result.output.decode("utf-8") if raw_result.output else ""
        else:
            result.error_message = (
                raw_result.error_message.decode("utf-8") if raw_result.error_message else "Unknown error"
            )

        self._lib.mero_compile_result_free(ctypes.byref(raw_result))
        return result

    @staticmethod
    def version() -> str:
        lib = MeroNativeCompiler._load_library()
        lib.mero_compiler_version.restype = ctypes.c_char_p
        return lib.mero_compiler_version().decode("utf-8")

    @staticmethod
    def _load_library():
        """Find and load the MERO shared library."""
        lib_names = {
            "linux": "libmero.so",
            "darwin": "libmero.dylib",
            "win32": "mero.dll",
        }
        lib_name = lib_names.get(sys.platform, "libmero.so")

        search_paths = [
            Path(__file__).parent.parent.parent / "build",
            Path(__file__).parent.parent.parent / "build" / "lib",
            Path(__file__).parent / "lib",
            Path("/usr/local/lib"),
            Path("/usr/lib"),
        ]

        for path in search_paths:
            lib_path = path / lib_name
            if lib_path.exists():
                return ctypes.CDLL(str(lib_path))

        # Try system load path
        try:
            return ctypes.CDLL(lib_name)
        except OSError:
            raise OSError(
                f"Cannot find MERO native library ({lib_name}). "
                f"Build the project with CMake first, or use the fallback compiler."
            )

    def _setup_functions(self):
        """Set up ctypes function signatures."""
        self._lib.mero_compiler_create.restype = ctypes.c_void_p
        self._lib.mero_compiler_create.argtypes = [ctypes.c_int]

        self._lib.mero_compiler_destroy.restype = None
        self._lib.mero_compiler_destroy.argtypes = [ctypes.c_void_p]

        self._lib.mero_compiler_compile.restype = _MeroCompileResult
        self._lib.mero_compiler_compile.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int
        ]

        self._lib.mero_compile_result_free.restype = None
        self._lib.mero_compile_result_free.argtypes = [ctypes.POINTER(_MeroCompileResult)]

        self._lib.mero_compiler_version.restype = ctypes.c_char_p
        self._lib.mero_compiler_version.argtypes = []
