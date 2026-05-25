"""
MERO Compiler - High-level Compiler API
Provides Python interface to the C++ compiler engine.
Developer: MERO:TG@QP4RM
"""
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional
import time

from .config import CompilerConfig
from .bindings import MeroNativeCompiler
from .cache import CompilationCache
from .exceptions import CompileError


@dataclass
class CompileResult:
    output: str = ""
    success: bool = False
    errors: list = field(default_factory=list)
    warnings: list = field(default_factory=list)
    stats: dict = field(default_factory=dict)


class MeroCompiler:
    """Main compiler interface for Python to Luau compilation."""

    def __init__(self, config: Optional[CompilerConfig] = None):
        self.config = config or CompilerConfig()
        self._cache = CompilationCache() if self.config.use_cache else None
        self._native: Optional[MeroNativeCompiler] = None
        self._init_native()

    def _init_native(self):
        """Initialize the native C++ compiler backend."""
        try:
            self._native = MeroNativeCompiler(self.config.opt_level)
        except (OSError, ImportError):
            self._native = None

    def compile(self, source: str, filename: str = "<string>") -> CompileResult:
        """Compile Python source code to Luau."""
        start_time = time.perf_counter()

        if self._cache:
            cached = self._cache.get(source, self.config)
            if cached:
                return CompileResult(output=cached, success=True,
                                     stats={"cached": True})

        if self._native:
            result = self._compile_native(source, filename)
        else:
            result = self._compile_fallback(source, filename)

        elapsed = time.perf_counter() - start_time
        result.stats["total_ms"] = elapsed * 1000

        if result.success and self._cache:
            self._cache.put(source, self.config, result.output)

        return result

    def compile_file(self, path: str) -> CompileResult:
        """Compile a Python file to Luau."""
        filepath = Path(path)
        if not filepath.exists():
            return CompileResult(errors=[f"File not found: {path}"])
        if not filepath.suffix == ".py":
            return CompileResult(errors=[f"Not a Python file: {path}"])

        source = filepath.read_text(encoding="utf-8")
        return self.compile(source, str(filepath))

    def check_file(self, path: str) -> CompileResult:
        """Type-check a file without generating output."""
        filepath = Path(path)
        if not filepath.exists():
            return CompileResult(errors=[f"File not found: {path}"])

        source = filepath.read_text(encoding="utf-8")
        return self._check_source(source, str(filepath))

    def _compile_native(self, source: str, filename: str) -> CompileResult:
        """Use the C++ backend for compilation."""
        try:
            native_result = self._native.compile(source)
            if native_result.success:
                return CompileResult(
                    output=native_result.output,
                    success=True,
                    stats={"backend": "native"}
                )
            return CompileResult(
                errors=[native_result.error_message],
                stats={"backend": "native"}
            )
        except Exception as e:
            return CompileResult(errors=[str(e)])

    def _compile_fallback(self, source: str, filename: str) -> CompileResult:
        """Pure Python fallback compiler (slower but always available)."""
        from .fallback_compiler import FallbackCompiler
        fb = FallbackCompiler(self.config)
        return fb.compile(source, filename)

    def _check_source(self, source: str, filename: str) -> CompileResult:
        """Type-check source without code generation."""
        if self._native:
            return self._compile_native(source, filename)
        return CompileResult(success=True, warnings=["Type checking requires native backend"])
