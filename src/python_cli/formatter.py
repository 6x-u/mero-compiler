"""
MERO Compiler - Output Formatter
Formats compiler output and diagnostics for terminal display.
Developer: MERO:TG@QP4RM
"""
from typing import Optional


class OutputFormatter:
    """Formats compilation output for terminal display."""

    def __init__(self, color: bool = True):
        self.color = color

    def format_error(self, file: str, line: int, col: int, message: str,
                     source_line: Optional[str] = None) -> str:
        """Format an error message with source context."""
        parts = []

        location = f"{file}:{line}:{col}"
        if self.color:
            parts.append(f"\033[1m{location}: \033[31merror\033[0m\033[1m: {message}\033[0m")
        else:
            parts.append(f"{location}: error: {message}")

        if source_line:
            parts.append(f"  {line} | {source_line}")
            pointer = " " * (len(str(line)) + 3 + col - 1) + "^"
            if self.color:
                parts.append(f"\033[32m{pointer}\033[0m")
            else:
                parts.append(pointer)

        return "\n".join(parts)

    def format_warning(self, file: str, line: int, col: int, message: str) -> str:
        """Format a warning message."""
        location = f"{file}:{line}:{col}"
        if self.color:
            return f"\033[1m{location}: \033[33mwarning\033[0m\033[1m: {message}\033[0m"
        return f"{location}: warning: {message}"

    def format_stats(self, stats: dict) -> str:
        """Format compilation statistics."""
        lines = ["Compilation Statistics:"]
        if "total_ms" in stats:
            lines.append(f"  Total time: {stats['total_ms']:.2f}ms")
        if "lines_in" in stats:
            lines.append(f"  Input lines: {stats['lines_in']}")
        if "lines_out" in stats:
            lines.append(f"  Output lines: {stats['lines_out']}")
        if "cached" in stats:
            lines.append(f"  Cached: {'yes' if stats['cached'] else 'no'}")
        return "\n".join(lines)

    def format_success(self, input_file: str, output_file: str) -> str:
        """Format successful compilation message."""
        if self.color:
            return f"  \033[32m✓\033[0m {input_file} → {output_file}"
        return f"  OK {input_file} -> {output_file}"
