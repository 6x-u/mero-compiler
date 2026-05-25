"""
MERO Compiler - Command Line Interface
Main entry point for the mero compiler tool.
Developer: MERO:TG@QP4RM
"""
import argparse
import sys
import os
import time
from pathlib import Path

from .compiler import MeroCompiler
from .config import CompilerConfig, load_config
from .logger import Logger, LogLevel
from .formatter import OutputFormatter
from .validator import InputValidator
from .constants import VERSION, BANNER


def create_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="mero",
        description="MERO - Python to Luau Compiler for Roblox",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=BANNER,
    )
    parser.add_argument("--version", action="version", version=f"mero {VERSION}")

    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # compile command
    compile_parser = subparsers.add_parser("compile", help="Compile Python to Luau")
    compile_parser.add_argument("input", help="Input Python file or directory")
    compile_parser.add_argument("-o", "--output", help="Output file/directory")
    compile_parser.add_argument("-O", "--opt-level", type=int, default=2,
                                choices=[0, 1, 2, 3], help="Optimization level")
    compile_parser.add_argument("--strict", action="store_true",
                                help="Enable strict type checking")
    compile_parser.add_argument("--minify", action="store_true",
                                help="Minify output code")
    compile_parser.add_argument("--no-types", action="store_true",
                                help="Disable type annotations in output")
    compile_parser.add_argument("--dump-ast", action="store_true",
                                help="Print AST for debugging")
    compile_parser.add_argument("--dump-ir", action="store_true",
                                help="Print IR for debugging")
    compile_parser.add_argument("--dump-tokens", action="store_true",
                                help="Print tokens for debugging")
    compile_parser.add_argument("--no-comments", action="store_true",
                                help="Strip comments from output")

    # watch command
    watch_parser = subparsers.add_parser("watch", help="Watch files and recompile on change")
    watch_parser.add_argument("input", help="Input directory to watch")
    watch_parser.add_argument("-o", "--output", help="Output directory")

    # init command
    subparsers.add_parser("init", help="Initialize a new MERO project")

    # check command
    check_parser = subparsers.add_parser("check", help="Type-check without compiling")
    check_parser.add_argument("input", help="Input file to check")

    # repl command
    subparsers.add_parser("repl", help="Start interactive REPL")

    # bundle command
    bundle_parser = subparsers.add_parser("bundle", help="Bundle multiple files into one")
    bundle_parser.add_argument("input", help="Entry point file")
    bundle_parser.add_argument("-o", "--output", help="Output file")

    # Common options
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    parser.add_argument("-q", "--quiet", action="store_true", help="Suppress non-error output")
    parser.add_argument("--config", help="Config file path")
    parser.add_argument("--color", choices=["auto", "always", "never"], default="auto")

    return parser


def cmd_compile(args, config: CompilerConfig, logger: Logger) -> int:
    input_path = Path(args.input)
    validator = InputValidator()

    if not validator.validate_input(input_path):
        logger.error(f"Invalid input: {validator.last_error}")
        return 1

    compiler = MeroCompiler(config)
    formatter = OutputFormatter(color=(args.color != "never"))

    if input_path.is_file():
        files = [input_path]
    else:
        files = list(input_path.rglob("*.py"))
        files = [f for f in files if not validator.should_skip(f)]

    if not files:
        logger.error("No Python files found")
        return 1

    logger.info(f"Compiling {len(files)} file(s)...")
    start_time = time.time()
    errors = 0

    for file in files:
        result = compiler.compile_file(str(file))

        if result.success:
            output_path = _resolve_output(file, input_path, args.output)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            output_path.write_text(result.output, encoding="utf-8")
            logger.info(f"  {file} -> {output_path}")
        else:
            errors += 1
            for err in result.errors:
                logger.error(f"  {err}")

    elapsed = time.time() - start_time
    logger.info(f"\nDone in {elapsed:.2f}s ({len(files) - errors}/{len(files)} succeeded)")

    return 1 if errors > 0 else 0


def cmd_watch(args, config: CompilerConfig, logger: Logger) -> int:
    from .watcher import FileWatcher

    input_path = Path(args.input)
    if not input_path.is_dir():
        logger.error("Watch input must be a directory")
        return 1

    watcher = FileWatcher(str(input_path), config, logger)
    logger.info(f"Watching {input_path} for changes (Ctrl+C to stop)...")

    try:
        watcher.start()
    except KeyboardInterrupt:
        watcher.stop()
        logger.info("\nStopped watching.")

    return 0


def cmd_init(logger: Logger) -> int:
    from .project import ProjectManager
    manager = ProjectManager()
    manager.init_project(Path.cwd())
    logger.info("Initialized MERO project in current directory")
    return 0


def cmd_check(args, config: CompilerConfig, logger: Logger) -> int:
    compiler = MeroCompiler(config)
    result = compiler.check_file(str(args.input))

    if result.success:
        logger.info(f"{args.input}: No errors found")
        if result.warnings:
            for w in result.warnings:
                logger.warn(f"  {w}")
        return 0
    else:
        for err in result.errors:
            logger.error(f"  {err}")
        return 1


def cmd_repl(config: CompilerConfig, logger: Logger) -> int:
    from .repl import MeroREPL
    repl = MeroREPL(config)
    repl.run()
    return 0


def _resolve_output(file: Path, input_base: Path, output_arg) -> Path:
    if output_arg:
        output_base = Path(output_arg)
        if input_base.is_file():
            return output_base
        rel = file.relative_to(input_base)
        return output_base / rel.with_suffix(".luau")
    return file.with_suffix(".luau")


def main() -> int:
    parser = create_parser()
    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 0

    log_level = LogLevel.ERROR if args.quiet else (LogLevel.DEBUG if args.verbose else LogLevel.INFO)
    logger = Logger(level=log_level, color=(args.color != "never"))
    config = load_config(args.config if hasattr(args, "config") and args.config else None)

    # Apply CLI overrides
    if args.command == "compile":
        config.opt_level = args.opt_level
        config.strict = args.strict
        config.minify = args.minify
        config.emit_types = not args.no_types
        config.dump_ast = args.dump_ast
        config.dump_ir = args.dump_ir
        config.dump_tokens = args.dump_tokens
        config.emit_comments = not args.no_comments

    commands = {
        "compile": lambda: cmd_compile(args, config, logger),
        "watch": lambda: cmd_watch(args, config, logger),
        "init": lambda: cmd_init(logger),
        "check": lambda: cmd_check(args, config, logger),
        "repl": lambda: cmd_repl(config, logger),
    }

    handler = commands.get(args.command)
    if handler:
        return handler()

    parser.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
