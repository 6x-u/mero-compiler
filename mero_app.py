"""
MERO Compiler - Desktop Application
Python → Lua/Luau Compiler for Roblox
Developer: MERO:TG@QP4RM
"""
import os
import sys
from pathlib import Path

from src.python_cli.fallback_compiler import FallbackCompiler, CompileResult
from src.python_cli.config import CompilerConfig


BANNER = r"""
 ╔══════════════════════════════════════════════════════╗
 ║          MERO Compiler v1.0.0                       ║
 ║          Python → Lua/Luau for Roblox              ║
 ║          Developer: MERO:TG@QP4RM                   ║
 ╚══════════════════════════════════════════════════════╝
"""

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')


def show_menu():
    clear_screen()
    print(BANNER)
    print("  Choose compilation target:")
    print()
    print("    [1] Python  →  Lua   (.lua)")
    print("    [2] Python  →  Luau  (.luau)")
    print()
    print("    [0] Exit")
    print()


def get_choice() -> int:
    while True:
        try:
            choice = input("  >> Enter choice (0-2): ").strip()
            if choice in ("0", "1", "2"):
                return int(choice)
            print("  Invalid choice. Please enter 0, 1, or 2.")
        except (EOFError, KeyboardInterrupt):
            return 0


def get_file_path() -> str:
    while True:
        print()
        path = input("  >> Enter Python file path: ").strip()
        path = path.strip('"').strip("'")

        if not path:
            print("  No path entered. Try again.")
            continue

        if not os.path.exists(path):
            print(f"  File not found: {path}")
            continue

        if not path.endswith(".py"):
            print("  File must be a .py file.")
            continue

        return path


def compile_file(source_path: str, target: str) -> bool:
    """Compile a Python file to Lua or Luau."""
    config = CompilerConfig()
    config.opt_level = 2

    compiler = FallbackCompiler(config)

    try:
        source = Path(source_path).read_text(encoding="utf-8")
    except Exception as e:
        print(f"\n  Error reading file: {e}")
        return False

    result = compiler.compile(source, source_path)

    if not result.success:
        print("\n  Compilation failed:")
        for err in result.errors:
            print(f"    - {err}")
        return False

    output = result.output

    # For .lua target, remove --!strict and adjust syntax
    if target == "lua":
        output = convert_luau_to_lua(output)
        ext = ".lua"
    else:
        ext = ".luau"

    output_path = Path(source_path).with_suffix(ext)
    try:
        output_path.write_text(output, encoding="utf-8")
    except Exception as e:
        print(f"\n  Error writing output: {e}")
        return False

    print(f"\n  Compilation successful!")
    print(f"  Input:  {source_path}")
    print(f"  Output: {output_path}")
    print(f"  Size:   {len(output)} bytes")
    return True


def convert_luau_to_lua(luau_code: str) -> str:
    """Convert Luau-specific syntax to standard Lua 5.1 compatible code."""
    lines = luau_code.split("\n")
    result = []

    for line in lines:
        # Remove --!strict
        if line.strip() == "--!strict":
            result.append("-- MERO Generated Lua")
            continue

        # Remove type annotations from function params: (x: number) -> (x)
        import re
        # Remove : type from parameters
        line = re.sub(r':\s*\w+\??', '', line)
        # Remove -> return_type
        line = re.sub(r'\)\s*:\s*\w+', ')', line)
        # Replace += with = x +
        line = re.sub(r'(\w+)\s*\+=\s*(.+)', r'\1 = \1 + \2', line)
        # Replace -= with = x -
        line = re.sub(r'(\w+)\s*-=\s*(.+)', r'\1 = \1 - \2', line)
        # Replace *= with = x *
        line = re.sub(r'(\w+)\s*\*=\s*(.+)', r'\1 = \1 * \2', line)
        # Replace //= with = math.floor(x / y)
        line = re.sub(r'(\w+)\s*//=\s*(.+)', r'\1 = math.floor(\1 / \2)', line)
        # Replace // with math.floor division
        line = re.sub(r'(\S+)\s*//\s*(\S+)', r'math.floor(\1 / \2)', line)
        # Replace 'continue' with goto (Lua 5.2+) or comment
        if line.strip() == "continue":
            line = line.replace("continue", "-- continue (use goto in Lua 5.2+)")

        result.append(line)

    return "\n".join(result)


def main():
    while True:
        show_menu()
        choice = get_choice()

        if choice == 0:
            print("\n  Goodbye!")
            sys.exit(0)

        target = "lua" if choice == 1 else "luau"
        file_path = get_file_path()

        print(f"\n  Compiling to {'Lua' if target == 'lua' else 'Luau'}...")
        compile_file(file_path, target)

        print()
        input("  Press Enter to continue...")


if __name__ == "__main__":
    main()
