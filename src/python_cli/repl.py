"""
MERO Compiler - Interactive REPL
Read-Eval-Print Loop for quick compilation testing.
Developer: MERO:TG@QP4RM
"""
import sys
from .compiler import MeroCompiler
from .config import CompilerConfig
from .constants import VERSION, BANNER


class MeroREPL:
    """Interactive REPL for MERO compiler."""

    def __init__(self, config: CompilerConfig = None):
        self.config = config or CompilerConfig()
        self.compiler = MeroCompiler(self.config)
        self._buffer = []
        self._history = []

    def run(self):
        """Start the REPL loop."""
        print(f"MERO REPL v{VERSION}")
        print("Type Python code to compile to Luau. Commands: :quit, :clear, :ast, :ir")
        print()

        while True:
            try:
                prompt = "... " if self._buffer else ">>> "
                line = input(prompt)
            except (EOFError, KeyboardInterrupt):
                print("\nGoodbye!")
                break

            if not self._buffer and line.startswith(":"):
                self._handle_command(line)
                continue

            self._buffer.append(line)

            if self._is_complete():
                source = "\n".join(self._buffer)
                self._execute(source)
                self._history.append(source)
                self._buffer = []

    def _handle_command(self, cmd: str):
        """Handle REPL commands."""
        cmd = cmd.strip().lower()

        if cmd in (":quit", ":q", ":exit"):
            print("Goodbye!")
            sys.exit(0)
        elif cmd == ":clear":
            self._buffer = []
            print("Buffer cleared.")
        elif cmd == ":ast":
            self.config.dump_ast = not self.config.dump_ast
            print(f"AST dump: {'on' if self.config.dump_ast else 'off'}")
        elif cmd == ":ir":
            self.config.dump_ir = not self.config.dump_ir
            print(f"IR dump: {'on' if self.config.dump_ir else 'off'}")
        elif cmd == ":history":
            for i, src in enumerate(self._history):
                print(f"[{i}] {src[:60]}...")
        elif cmd == ":help":
            print("Commands: :quit, :clear, :ast, :ir, :history, :help")
        else:
            print(f"Unknown command: {cmd}")

    def _execute(self, source: str):
        """Compile and display the result."""
        if not source.strip():
            return

        result = self.compiler.compile(source, "<repl>")

        if result.success:
            print("\033[36m--- Luau output ---\033[0m")
            # Strip header for REPL output
            lines = result.output.split("\n")
            output_lines = [l for l in lines if not l.startswith("--") and l.strip()]
            print("\n".join(output_lines))
            print()
        else:
            for err in result.errors:
                print(f"\033[31mError: {err}\033[0m")

    def _is_complete(self) -> bool:
        """Check if the current buffer forms a complete statement."""
        if not self._buffer:
            return False

        last_line = self._buffer[-1]

        # Empty line after indented block = complete
        if len(self._buffer) > 1 and last_line.strip() == "":
            return True

        # Single line without continuation
        if len(self._buffer) == 1:
            line = last_line.strip()
            if line.endswith(":"):
                return False
            if line.endswith("\\"):
                return False
            return True

        return False
