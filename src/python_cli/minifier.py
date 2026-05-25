"""
MERO Compiler - Luau Code Minifier
Reduces output size for production deployment.
Developer: MERO:TG@QP4RM
"""
import re
from typing import Set


class LuauMinifier:
    """Minifies generated Luau code for minimal file size."""

    def __init__(self):
        self._var_counter = 0
        self._var_map: dict = {}
        self._preserved: Set[str] = {
            "self", "game", "workspace", "script",
            "Instance", "Enum", "Vector3", "CFrame",
            "Color3", "UDim2", "math", "string", "table",
            "coroutine", "task", "bit32", "os", "debug",
            "true", "false", "nil", "print", "error",
            "pcall", "xpcall", "typeof", "tostring", "tonumber",
            "require", "setmetatable", "getmetatable",
            "rawget", "rawset", "pairs", "ipairs", "next",
            "select", "unpack", "type", "assert", "warn",
            "_RT",
        }

    def minify(self, source: str) -> str:
        """Minify Luau source code."""
        lines = source.split("\n")
        result = []

        for line in lines:
            processed = self._process_line(line)
            if processed is not None:
                result.append(processed)

        output = "\n".join(result)
        output = self._collapse_whitespace(output)
        return output

    def _process_line(self, line: str) -> str:
        """Process a single line for minification."""
        stripped = line.strip()

        # Remove empty lines
        if not stripped:
            return None

        # Remove single-line comments (but preserve --!strict)
        if stripped.startswith("--") and not stripped.startswith("--!"):
            return None

        # Remove trailing comments
        code_part = self._remove_inline_comment(stripped)

        # Collapse spaces
        code_part = re.sub(r'\s+', ' ', code_part)

        return code_part

    def _remove_inline_comment(self, line: str) -> str:
        """Remove inline comments, preserving strings."""
        in_string = False
        string_char = None
        i = 0
        while i < len(line):
            c = line[i]
            if in_string:
                if c == '\\':
                    i += 2
                    continue
                if c == string_char:
                    in_string = False
            else:
                if c in ('"', "'"):
                    in_string = True
                    string_char = c
                elif c == '-' and i + 1 < len(line) and line[i + 1] == '-':
                    return line[:i].rstrip()
            i += 1
        return line

    def _collapse_whitespace(self, source: str) -> str:
        """Collapse unnecessary whitespace."""
        # Remove spaces around operators where safe
        source = re.sub(r'\s*([=+\-*/<>~])\s*', r'\1', source)
        # Restore necessary spaces
        source = source.replace("local", "local ")
        source = source.replace("function", "function ")
        source = source.replace("return", "return ")
        source = source.replace("if", "if ")
        source = source.replace("then", " then")
        source = source.replace("else", " else")
        source = source.replace("elseif", " elseif ")
        source = source.replace("for", "for ")
        source = source.replace("while", "while ")
        source = source.replace("do", " do")
        source = source.replace("end", "\nend")
        source = source.replace("not", "not ")
        source = source.replace("and", " and ")
        source = source.replace("or", " or ")
        source = source.replace("in", " in ")
        return source
