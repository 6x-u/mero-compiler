"""
MERO Compiler - Logging System
Developer: MERO:TG@QP4RM
"""
import sys
from enum import IntEnum
from typing import TextIO


class LogLevel(IntEnum):
    DEBUG = 0
    INFO = 1
    WARN = 2
    ERROR = 3
    FATAL = 4


class Logger:
    """Structured logger with color support."""

    COLORS = {
        LogLevel.DEBUG: "\033[90m",
        LogLevel.INFO: "\033[0m",
        LogLevel.WARN: "\033[33m",
        LogLevel.ERROR: "\033[31m",
        LogLevel.FATAL: "\033[1;31m",
    }
    RESET = "\033[0m"
    LABELS = {
        LogLevel.DEBUG: "debug",
        LogLevel.INFO: "info",
        LogLevel.WARN: "warn",
        LogLevel.ERROR: "error",
        LogLevel.FATAL: "fatal",
    }

    def __init__(self, level: LogLevel = LogLevel.INFO, color: bool = True,
                 output: TextIO = sys.stderr):
        self.level = level
        self.color = color
        self.output = output

    def _emit(self, level: LogLevel, msg: str):
        if level < self.level:
            return
        if self.color:
            prefix = f"{self.COLORS[level]}[{self.LABELS[level]}]{self.RESET}"
        else:
            prefix = f"[{self.LABELS[level]}]"
        self.output.write(f"{prefix} {msg}\n")
        self.output.flush()

    def debug(self, msg: str): self._emit(LogLevel.DEBUG, msg)
    def info(self, msg: str): self._emit(LogLevel.INFO, msg)
    def warn(self, msg: str): self._emit(LogLevel.WARN, msg)
    def error(self, msg: str): self._emit(LogLevel.ERROR, msg)
    def fatal(self, msg: str): self._emit(LogLevel.FATAL, msg)
