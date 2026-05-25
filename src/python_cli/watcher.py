"""
MERO Compiler - File Watcher
Watches Python files for changes and triggers recompilation.
Developer: MERO:TG@QP4RM
"""
import os
import time
from pathlib import Path
from typing import Dict, Set
from .config import CompilerConfig
from .compiler import MeroCompiler
from .logger import Logger
from .validator import InputValidator


class FileWatcher:
    """Watches directories for file changes and recompiles."""

    def __init__(self, directory: str, config: CompilerConfig, logger: Logger):
        self.directory = Path(directory)
        self.config = config
        self.logger = logger
        self.compiler = MeroCompiler(config)
        self.validator = InputValidator()
        self._running = False
        self._mtimes: Dict[str, float] = {}
        self._poll_interval = 0.5

    def start(self):
        """Start watching for file changes."""
        self._running = True
        self._scan_initial()

        while self._running:
            changed = self._detect_changes()
            for path in changed:
                self._recompile(path)
            time.sleep(self._poll_interval)

    def stop(self):
        """Stop the file watcher."""
        self._running = False

    def _scan_initial(self):
        """Initial scan to record file modification times."""
        for py_file in self.directory.rglob("*.py"):
            if not self.validator.should_skip(py_file):
                self._mtimes[str(py_file)] = py_file.stat().st_mtime

    def _detect_changes(self) -> Set[Path]:
        """Detect files that have changed since last scan."""
        changed = set()

        for py_file in self.directory.rglob("*.py"):
            if self.validator.should_skip(py_file):
                continue
            path_str = str(py_file)
            mtime = py_file.stat().st_mtime

            if path_str not in self._mtimes:
                self._mtimes[path_str] = mtime
                changed.add(py_file)
            elif self._mtimes[path_str] < mtime:
                self._mtimes[path_str] = mtime
                changed.add(py_file)

        return changed

    def _recompile(self, path: Path):
        """Recompile a changed file."""
        self.logger.info(f"Change detected: {path}")
        result = self.compiler.compile_file(str(path))

        if result.success:
            output_path = path.with_suffix(".luau")
            output_path.write_text(result.output, encoding="utf-8")
            self.logger.info(f"  Compiled -> {output_path}")
        else:
            for err in result.errors:
                self.logger.error(f"  {err}")
