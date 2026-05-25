"""
MERO Compiler - Source Mapping
Maps generated Luau code back to Python source for debugging.
Developer: MERO:TG@QP4RM
"""
import json
from dataclasses import dataclass, field
from typing import Dict, List, Optional
from pathlib import Path


@dataclass
class SourceMapping:
    output_line: int
    source_file: str
    source_line: int
    source_col: int = 0
    name: Optional[str] = None


class SourceMap:
    """Bidirectional mapping between Python source and Luau output."""

    def __init__(self, source_file: str = ""):
        self.source_file = source_file
        self.mappings: List[SourceMapping] = []
        self._output_to_source: Dict[int, SourceMapping] = {}

    def add(self, output_line: int, source_line: int, source_col: int = 0,
            name: Optional[str] = None):
        """Add a mapping from output line to source location."""
        mapping = SourceMapping(
            output_line=output_line,
            source_file=self.source_file,
            source_line=source_line,
            source_col=source_col,
            name=name,
        )
        self.mappings.append(mapping)
        self._output_to_source[output_line] = mapping

    def lookup(self, output_line: int) -> Optional[SourceMapping]:
        """Look up source location for an output line."""
        return self._output_to_source.get(output_line)

    def to_json(self) -> str:
        """Serialize source map to JSON."""
        data = {
            "version": 1,
            "source_file": self.source_file,
            "mappings": [
                {
                    "out": m.output_line,
                    "src": m.source_line,
                    "col": m.source_col,
                    "name": m.name,
                }
                for m in self.mappings
            ],
        }
        return json.dumps(data, indent=2)

    @classmethod
    def from_json(cls, data: str) -> "SourceMap":
        """Deserialize source map from JSON."""
        parsed = json.loads(data)
        sm = cls(parsed.get("source_file", ""))
        for m in parsed.get("mappings", []):
            sm.add(m["out"], m["src"], m.get("col", 0), m.get("name"))
        return sm

    def save(self, path: str):
        """Save source map to file."""
        Path(path).write_text(self.to_json(), encoding="utf-8")

    @classmethod
    def load(cls, path: str) -> "SourceMap":
        """Load source map from file."""
        data = Path(path).read_text(encoding="utf-8")
        return cls.from_json(data)
