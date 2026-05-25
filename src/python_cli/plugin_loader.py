"""
MERO Compiler - Plugin Loader
Extensible plugin system for custom transformations.
Developer: MERO:TG@QP4RM
"""
import importlib
import importlib.util
from pathlib import Path
from typing import List, Dict, Any, Optional
from .exceptions import PluginError


class MeroPlugin:
    """Base class for MERO compiler plugins."""

    name: str = "unnamed"
    version: str = "0.0.0"

    def on_source(self, source: str, filename: str) -> str:
        """Transform source before compilation."""
        return source

    def on_output(self, output: str, filename: str) -> str:
        """Transform output after compilation."""
        return output

    def on_error(self, errors: List[str], filename: str):
        """Handle compilation errors."""
        pass


class PluginLoader:
    """Loads and manages compiler plugins."""

    def __init__(self):
        self._plugins: List[MeroPlugin] = []
        self._plugin_dirs: List[Path] = []

    def add_search_path(self, path: str):
        """Add a plugin search directory."""
        self._plugin_dirs.append(Path(path))

    def load_plugin(self, name: str) -> MeroPlugin:
        """Load a plugin by name or path."""
        # Try as a Python module
        try:
            module = importlib.import_module(f"mero_plugins.{name}")
            plugin = self._find_plugin_class(module)
            if plugin:
                self._plugins.append(plugin)
                return plugin
        except ImportError:
            pass

        # Try as a file path
        for search_dir in self._plugin_dirs:
            plugin_file = search_dir / f"{name}.py"
            if plugin_file.exists():
                plugin = self._load_from_file(plugin_file)
                if plugin:
                    self._plugins.append(plugin)
                    return plugin

        raise PluginError(f"Plugin not found: {name}")

    def load_all(self, names: List[str]) -> List[MeroPlugin]:
        """Load multiple plugins."""
        loaded = []
        for name in names:
            try:
                plugin = self.load_plugin(name)
                loaded.append(plugin)
            except PluginError as e:
                pass  # Skip failed plugins
        return loaded

    def apply_source_transforms(self, source: str, filename: str) -> str:
        """Apply all plugin source transformations."""
        for plugin in self._plugins:
            source = plugin.on_source(source, filename)
        return source

    def apply_output_transforms(self, output: str, filename: str) -> str:
        """Apply all plugin output transformations."""
        for plugin in self._plugins:
            output = plugin.on_output(output, filename)
        return output

    @property
    def plugins(self) -> List[MeroPlugin]:
        return self._plugins

    def _load_from_file(self, path: Path) -> Optional[MeroPlugin]:
        """Load a plugin from a .py file."""
        spec = importlib.util.spec_from_file_location(path.stem, str(path))
        if not spec or not spec.loader:
            return None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return self._find_plugin_class(module)

    def _find_plugin_class(self, module) -> Optional[MeroPlugin]:
        """Find and instantiate the plugin class in a module."""
        for attr_name in dir(module):
            attr = getattr(module, attr_name)
            if (isinstance(attr, type) and issubclass(attr, MeroPlugin) and
                    attr is not MeroPlugin):
                return attr()
        return None
