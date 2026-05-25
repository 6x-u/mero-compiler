"""
MERO Compiler - Module Resolver
Resolves Python import statements to file paths.
Developer: MERO:TG@QP4RM
"""
from pathlib import Path
from typing import Optional, List


class ModuleResolver:
    """Resolves Python module names to file system paths."""

    def __init__(self, search_paths: List[str] = None):
        self._search_paths = [Path(p) for p in (search_paths or ["."])]

    def add_search_path(self, path: str):
        """Add a directory to the module search path."""
        self._search_paths.append(Path(path))

    def resolve(self, module_name: str, from_file: str = None) -> Optional[Path]:
        """Resolve a module name to a file path."""
        parts = module_name.split(".")

        # Try relative import first
        if from_file:
            from_dir = Path(from_file).parent
            result = self._try_resolve(parts, from_dir)
            if result:
                return result

        # Try absolute paths
        for search_path in self._search_paths:
            result = self._try_resolve(parts, search_path)
            if result:
                return result

        return None

    def resolve_relative(self, module_name: str, level: int,
                         from_file: str) -> Optional[Path]:
        """Resolve a relative import (from . import X)."""
        from_dir = Path(from_file).parent
        for _ in range(level - 1):
            from_dir = from_dir.parent

        if not module_name:
            init = from_dir / "__init__.py"
            return init if init.exists() else None

        parts = module_name.split(".")
        return self._try_resolve(parts, from_dir)

    def _try_resolve(self, parts: List[str], base: Path) -> Optional[Path]:
        """Try to resolve module parts from a base directory."""
        # Try as package (directory with __init__.py)
        pkg_path = base
        for part in parts:
            pkg_path = pkg_path / part
        init_file = pkg_path / "__init__.py"
        if init_file.exists():
            return init_file

        # Try as module file
        if len(parts) > 0:
            module_path = base
            for part in parts[:-1]:
                module_path = module_path / part
            file_path = module_path / f"{parts[-1]}.py"
            if file_path.exists():
                return file_path

        return None

    def find_all_modules(self, directory: str) -> List[str]:
        """Find all importable modules in a directory."""
        modules = []
        base = Path(directory)

        for py_file in base.rglob("*.py"):
            if py_file.name.startswith("_") and py_file.name != "__init__.py":
                continue
            rel = py_file.relative_to(base)
            parts = list(rel.parts)
            if parts[-1] == "__init__.py":
                parts = parts[:-1]
            else:
                parts[-1] = parts[-1][:-3]  # Remove .py
            if parts:
                modules.append(".".join(parts))

        return sorted(modules)
