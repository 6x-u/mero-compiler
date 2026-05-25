"""
MERO Compiler - Constants
Developer: MERO:TG@QP4RM
"""

VERSION = "1.0.0"
PROJECT_NAME = "MERO"
DEVELOPER = "MERO:TG@QP4RM"

BANNER = """
╔══════════════════════════════════════╗
║  MERO Compiler v1.0.0               ║
║  Python → Luau for Roblox           ║
║  Developer: MERO:TG@QP4RM           ║
╚══════════════════════════════════════╝
"""

SUPPORTED_EXTENSIONS = [".py"]
OUTPUT_EXTENSION = ".luau"
CONFIG_FILES = ["mero.json", ".merorc", "mero.config.json"]
DEFAULT_EXCLUDE = ["__pycache__", ".git", "venv", "node_modules", ".env", "dist", "build"]
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10 MB
LUAU_RESERVED_WORDS = frozenset([
    "and", "break", "do", "else", "elseif", "end",
    "false", "for", "function", "if", "in", "local",
    "nil", "not", "or", "repeat", "return", "then",
    "true", "until", "while", "continue", "type", "export",
])
