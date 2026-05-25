"""
MERO Compiler - Compilation Cache
Caches compilation results for faster recompilation.
Developer: MERO:TG@QP4RM
"""
import hashlib
import json
import os
from pathlib import Path
from typing import Optional
from .config import CompilerConfig


class CompilationCache:
    """File-based compilation cache using content hashing."""

    def __init__(self, cache_dir: Optional[str] = None):
        if cache_dir:
            self._dir = Path(cache_dir)
        else:
            self._dir = Path.home() / ".cache" / "mero"
        self._dir.mkdir(parents=True, exist_ok=True)
        self._max_entries = 1000
        self._max_size_mb = 100

    def get(self, source: str, config: CompilerConfig) -> Optional[str]:
        """Retrieve cached compilation result."""
        key = self._compute_key(source, config)
        cache_file = self._dir / f"{key}.luau"

        if cache_file.exists():
            try:
                return cache_file.read_text(encoding="utf-8")
            except (IOError, OSError):
                return None
        return None

    def put(self, source: str, config: CompilerConfig, output: str):
        """Store compilation result in cache."""
        key = self._compute_key(source, config)
        cache_file = self._dir / f"{key}.luau"

        try:
            cache_file.write_text(output, encoding="utf-8")
        except (IOError, OSError):
            pass

        self._evict_if_needed()

    def invalidate(self, source: str, config: CompilerConfig):
        """Remove a specific cache entry."""
        key = self._compute_key(source, config)
        cache_file = self._dir / f"{key}.luau"
        if cache_file.exists():
            cache_file.unlink()

    def clear(self):
        """Clear all cached entries."""
        for f in self._dir.glob("*.luau"):
            f.unlink()

    def stats(self) -> dict:
        """Return cache statistics."""
        files = list(self._dir.glob("*.luau"))
        total_size = sum(f.stat().st_size for f in files)
        return {
            "entries": len(files),
            "size_mb": total_size / (1024 * 1024),
            "cache_dir": str(self._dir),
        }

    def _compute_key(self, source: str, config: CompilerConfig) -> str:
        """Generate cache key from source and config."""
        hasher = hashlib.sha256()
        hasher.update(source.encode("utf-8"))
        config_str = json.dumps(config.to_dict(), sort_keys=True)
        hasher.update(config_str.encode("utf-8"))
        return hasher.hexdigest()[:32]

    def _evict_if_needed(self):
        """Remove old entries if cache exceeds limits."""
        files = list(self._dir.glob("*.luau"))
        if len(files) <= self._max_entries:
            return

        files.sort(key=lambda f: f.stat().st_mtime)
        to_remove = len(files) - self._max_entries
        for f in files[:to_remove]:
            f.unlink()
