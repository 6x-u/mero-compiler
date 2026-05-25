"""
MERO Compiler - Configuration Management
Developer: MERO:TG@QP4RM
"""
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, Dict, Any
import json


@dataclass
class CompilerConfig:
    opt_level: int = 2
    strict: bool = False
    minify: bool = False
    emit_types: bool = True
    emit_comments: bool = True
    dump_ast: bool = False
    dump_ir: bool = False
    dump_tokens: bool = False
    use_cache: bool = True
    indent_size: int = 4
    module_prefix: str = "MERO"
    target_roblox: bool = True
    source_map: bool = False
    output_dir: Optional[str] = None
    exclude_patterns: list = field(default_factory=lambda: ["__pycache__", ".git", "venv"])
    plugins: list = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "opt_level": self.opt_level,
            "strict": self.strict,
            "minify": self.minify,
            "emit_types": self.emit_types,
            "emit_comments": self.emit_comments,
            "use_cache": self.use_cache,
            "indent_size": self.indent_size,
            "module_prefix": self.module_prefix,
            "target_roblox": self.target_roblox,
            "source_map": self.source_map,
            "exclude_patterns": self.exclude_patterns,
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "CompilerConfig":
        valid_fields = {f.name for f in cls.__dataclass_fields__.values()}
        filtered = {k: v for k, v in data.items() if k in valid_fields}
        return cls(**filtered)


def load_config(path: Optional[str] = None) -> CompilerConfig:
    """Load configuration from file or defaults."""
    if path:
        config_path = Path(path)
    else:
        config_path = _find_config()

    if config_path and config_path.exists():
        data = json.loads(config_path.read_text(encoding="utf-8"))
        return CompilerConfig.from_dict(data)

    return CompilerConfig()


def save_config(config: CompilerConfig, path: str):
    """Save configuration to file."""
    Path(path).write_text(
        json.dumps(config.to_dict(), indent=2),
        encoding="utf-8"
    )


def _find_config() -> Optional[Path]:
    """Search for config file in current and parent directories."""
    names = ["mero.json", ".merorc", "mero.config.json"]
    cwd = Path.cwd()

    for _ in range(10):
        for name in names:
            candidate = cwd / name
            if candidate.exists():
                return candidate
        parent = cwd.parent
        if parent == cwd:
            break
        cwd = parent

    return None
