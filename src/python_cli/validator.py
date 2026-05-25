"""
MERO Compiler - Input Validator
Validates Python source files before compilation.
Developer: MERO:TG@QP4RM
"""
from pathlib import Path
from typing import Optional
from .constants import SUPPORTED_EXTENSIONS, MAX_FILE_SIZE, DEFAULT_EXCLUDE


class InputValidator:
    """Validates input files and directories."""

    def __init__(self):
        self.last_error: Optional[str] = None

    def validate_input(self, path: Path) -> bool:
        """Validate input path exists and is readable."""
        self.last_error = None

        if not path.exists():
            self.last_error = f"Path does not exist: {path}"
            return False

        if path.is_file():
            return self._validate_file(path)

        if path.is_dir():
            return self._validate_directory(path)

        self.last_error = f"Invalid path type: {path}"
        return False

    def validate_source(self, source: str, filename: str = "<string>") -> bool:
        """Validate source code content."""
        self.last_error = None

        if not source.strip():
            self.last_error = "Empty source"
            return False

        if len(source) > MAX_FILE_SIZE:
            self.last_error = f"File too large ({len(source)} bytes, max {MAX_FILE_SIZE})"
            return False

        try:
            source.encode("utf-8")
        except UnicodeEncodeError:
            self.last_error = "Source contains invalid characters"
            return False

        return True

    def should_skip(self, path: Path) -> bool:
        """Check if a file should be skipped during directory compilation."""
        parts = path.parts
        for exclude in DEFAULT_EXCLUDE:
            if exclude in parts:
                return True

        if path.name.startswith("."):
            return True

        if path.name.startswith("_") and path.name != "__init__.py":
            return True

        return False

    def _validate_file(self, path: Path) -> bool:
        """Validate a single file."""
        if path.suffix not in SUPPORTED_EXTENSIONS:
            self.last_error = f"Unsupported file type: {path.suffix}"
            return False

        if path.stat().st_size > MAX_FILE_SIZE:
            self.last_error = f"File too large: {path}"
            return False

        if not os.access(str(path), os.R_OK):
            self.last_error = f"File not readable: {path}"
            return False

        return True

    def _validate_directory(self, path: Path) -> bool:
        """Validate a directory contains compilable files."""
        py_files = list(path.rglob("*.py"))
        if not py_files:
            self.last_error = f"No Python files found in: {path}"
            return False
        return True


import os
