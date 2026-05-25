"""
MERO Compiler - Exception Types
Developer: MERO:TG@QP4RM
"""


class MeroError(Exception):
    """Base exception for all MERO errors."""
    pass


class CompileError(MeroError):
    """Raised when compilation fails."""
    def __init__(self, message: str, file: str = "", line: int = 0, column: int = 0):
        self.file = file
        self.line = line
        self.column = column
        super().__init__(f"{file}:{line}:{column}: {message}" if file else message)


class ParseError(CompileError):
    """Raised on syntax errors during parsing."""
    pass


class TypeCheckError(CompileError):
    """Raised on type checking failures."""
    pass


class SemanticError(CompileError):
    """Raised on semantic analysis failures."""
    pass


class ConfigError(MeroError):
    """Raised on configuration errors."""
    pass


class PluginError(MeroError):
    """Raised on plugin loading/execution errors."""
    pass


class CacheError(MeroError):
    """Raised on compilation cache errors."""
    pass


class BindingError(MeroError):
    """Raised when native bindings fail."""
    pass
