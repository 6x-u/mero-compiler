"""
MERO Compiler - Utilities
Common helper functions used across the compiler.
Developer: MERO:TG@QP4RM
"""
import hashlib
import os
import re
from pathlib import Path
from typing import List, Optional


def slugify(name: str) -> str:
    """Convert a name to a valid Luau identifier."""
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name)
    if name and name[0].isdigit():
        name = '_' + name
    return name


def compute_hash(content: str) -> str:
    """Compute SHA-256 hash of content."""
    return hashlib.sha256(content.encode("utf-8")).hexdigest()


def find_python_files(directory: str, exclude: List[str] = None) -> List[Path]:
    """Recursively find all Python files in a directory."""
    exclude = exclude or ["__pycache__", ".git", "venv", "node_modules"]
    results = []

    for root, dirs, files in os.walk(directory):
        dirs[:] = [d for d in dirs if d not in exclude]
        for f in files:
            if f.endswith(".py"):
                results.append(Path(root) / f)

    return sorted(results)


def relative_path(file: Path, base: Path) -> str:
    """Get relative path from base, or absolute if not relative."""
    try:
        return str(file.relative_to(base))
    except ValueError:
        return str(file)


def indent_code(code: str, spaces: int = 4) -> str:
    """Indent all lines of code."""
    prefix = " " * spaces
    return "\n".join(prefix + line if line.strip() else "" for line in code.split("\n"))


def strip_docstrings(source: str) -> str:
    """Remove docstrings from Python source."""
    pattern = r'"""[\s\S]*?"""|\'\'\'[\s\S]*?\'\'\''
    return re.sub(pattern, '', source)


def count_lines(text: str) -> int:
    """Count non-empty lines in text."""
    return sum(1 for line in text.split("\n") if line.strip())


def format_size(bytes_count: int) -> str:
    """Format byte count as human-readable string."""
    if bytes_count < 1024:
        return f"{bytes_count} B"
    elif bytes_count < 1024 * 1024:
        return f"{bytes_count / 1024:.1f} KB"
    return f"{bytes_count / (1024 * 1024):.1f} MB"


def is_dunder(name: str) -> bool:
    """Check if name is a dunder method."""
    return name.startswith("__") and name.endswith("__")


def sanitize_luau_name(name: str) -> str:
    """Ensure a name is valid in Luau."""
    from .constants import LUAU_RESERVED_WORDS
    if name in LUAU_RESERVED_WORDS:
        return f"_{name}"
    return slugify(name)
