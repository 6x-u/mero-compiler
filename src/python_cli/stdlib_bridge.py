"""
MERO Compiler - Standard Library Bridge
Maps Python stdlib modules to MERO runtime equivalents.
Developer: MERO:TG@QP4RM
"""
from typing import Dict, Optional, Set


STDLIB_MODULES: Dict[str, str] = {
    "math": "MeroRuntime.stdlib_math",
    "string": "MeroRuntime.stdlib_string",
    "os": "MeroRuntime.stdlib_os",
    "os.path": "MeroRuntime.stdlib_os",
    "sys": "MeroRuntime.stdlib_os",
    "time": "MeroRuntime.stdlib_os",
    "random": "MeroRuntime.stdlib_os",
    "collections": "MeroRuntime.stdlib_table",
    "itertools": "MeroRuntime.iterator",
    "functools": "MeroRuntime.decorator",
    "typing": None,  # Stripped at compile time
    "dataclasses": None,  # Handled specially by compiler
    "abc": None,  # Handled by class system
    "enum": None,  # Handled by class system
}

SUPPORTED_IMPORTS: Dict[str, Set[str]] = {
    "math": {
        "pi", "e", "inf", "nan", "tau",
        "floor", "ceil", "sqrt", "log", "log2", "log10",
        "exp", "pow", "sin", "cos", "tan",
        "asin", "acos", "atan", "atan2",
        "factorial", "gcd", "lcm",
        "isnan", "isinf", "isfinite",
        "radians", "degrees", "fabs", "fmod",
    },
    "random": {
        "random", "randint", "choice", "shuffle", "sample",
    },
    "time": {
        "time", "sleep", "perf_counter",
    },
    "os": {
        "getenv", "system",
    },
}


class StdlibBridge:
    """Maps Python standard library usage to MERO runtime calls."""

    def __init__(self):
        self._used_modules: Set[str] = set()

    def is_supported(self, module: str) -> bool:
        """Check if a stdlib module is supported."""
        return module in STDLIB_MODULES

    def get_runtime_module(self, module: str) -> Optional[str]:
        """Get the MERO runtime module name for a stdlib module."""
        return STDLIB_MODULES.get(module)

    def is_import_supported(self, module: str, name: str) -> bool:
        """Check if a specific import from a module is supported."""
        if module not in SUPPORTED_IMPORTS:
            return False
        return name in SUPPORTED_IMPORTS[module]

    def mark_used(self, module: str):
        """Mark a module as used (for dependency tracking)."""
        self._used_modules.add(module)

    def get_used_modules(self) -> Set[str]:
        """Get all used modules."""
        return self._used_modules.copy()

    def generate_requires(self) -> str:
        """Generate Luau require statements for used modules."""
        lines = []
        for mod in sorted(self._used_modules):
            runtime_mod = STDLIB_MODULES.get(mod)
            if runtime_mod:
                var_name = mod.replace(".", "_")
                lines.append(
                    f'local {var_name} = require(script.Parent:WaitForChild("{runtime_mod}"))'
                )
        return "\n".join(lines)
