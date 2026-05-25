<p align="center">
  <img src="source/mero.ico" alt="MERO Logo" width="128" height="128">
</p>

<h1 align="center">MERO Compiler</h1>

<p align="center">
  <strong>Multi-Language Compiler for Roblox</strong><br>
  Python → Lua/Luau | Production-Grade | Zero Dependencies
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-1.0.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/platform-Windows-green.svg" alt="Platform">
  <img src="https://img.shields.io/badge/target-Roblox%20Studio-red.svg" alt="Target">
  <img src="https://img.shields.io/badge/license-Proprietary-orange.svg" alt="License">
</p>

---

## Overview

MERO is a **complete compiler system** that transforms Python source code into optimized Lua/Luau code ready to run inside **Roblox Studio**. Unlike simple transpilers, MERO implements a full compiler pipeline:

```
┌─────────┐    ┌────────┐    ┌──────────┐    ┌──────┐    ┌───────────┐    ┌─────────┐
│  Python │ →  │ Lexer  │ →  │  Parser  │ →  │  IR  │ →  │ Optimizer │ →  │ CodeGen │
│  Source │    │(tokens)│    │  (AST)   │    │      │    │           │    │(Lua/au) │
└─────────┘    └────────┘    └──────────┘    └──────┘    └───────────┘    └─────────┘
```

The compiler engine is built in **C/C++** for maximum performance, with a standalone Windows executable that requires **zero installation** — just run `MERO.exe`.

---

## Quick Start

### 1. Download

Download the latest release from the `release/` folder.

### 2. Run

Double-click **`MERO.exe`** or run from command line:

```
MERO.exe
```

### 3. Choose Target

```
 +======================================================+
 |          MERO Compiler v1.0.0                        |
 |          Python -> Lua/Luau for Roblox              |
 |          Developer: MERO:TG@QP4RM                   |
 +======================================================+

  Choose compilation target:

    [1] Python  ->  Lua   (.lua)
    [2] Python  ->  Luau  (.luau)

    [0] Exit

  >> Enter choice (0-2):
```

### 4. Enter File Path

```
  >> Enter Python file path: C:\Users\you\scripts\game.py

  Compiling to Luau...

  Compilation successful!
  Input:  C:\Users\you\scripts\game.py
  Output: C:\Users\you\scripts\game.luau
  Size:   1847 bytes
```

### 5. Use in Roblox

Copy the generated `.luau` file content into a **Script** or **LocalScript** in Roblox Studio. Copy the runtime modules from `release/runtime/` as **ModuleScripts**.

---

## Features

### Language Support

| Python Feature | Supported | Output |
|---|---|---|
| Functions & Closures | ✓ | `local function name()` |
| Classes & Inheritance | ✓ | Metatable-based OOP |
| Decorators | ✓ | Wrapper functions |
| Generators (yield) | ✓ | Coroutine-based |
| Async/Await | ✓ | Task scheduler |
| List Comprehensions | ✓ | Inline table construction |
| F-Strings | ✓ | String concatenation |
| Exception Handling | ✓ | pcall/xpcall wrappers |
| Type Annotations | ✓ | Luau type annotations |
| Multiple Inheritance | ✓ | Linearized MRO |
| Properties | ✓ | Getter/Setter metatables |
| Context Managers | ✓ | pcall with cleanup |
| Walrus Operator | ✓ | Local variable + assign |
| Unpacking (*, **) | ✓ | table.unpack / manual |

### Compiler Features

- **Optimization Level 0-3**: From no optimization to aggressive inlining
- **Dead Code Elimination**: Removes unreachable code paths
- **Constant Folding**: Evaluates compile-time expressions
- **Type Inference**: Basic type propagation for Luau annotations
- **0→1 Index Adjustment**: Automatic Python→Lua index conversion
- **Builtin Mapping**: 50+ Python builtins → Lua/Luau equivalents
- **Roblox API Integration**: Service imports, datatype constructors
- **Source Maps**: Track output lines back to source

### Optimization Pipeline

```
Level 0: No optimization (debug mode)
Level 1: Basic dead code removal
Level 2: Constant folding + dead code + simplification (default)
Level 3: Aggressive inlining + loop optimization + all of above
```

---

## Conversion Examples

### Functions

**Python:**
```python
def calculate_damage(base: int, multiplier: float = 1.5) -> float:
    return base * multiplier
```

**Luau Output:**
```lua
local function calculate_damage(base: number, multiplier: number?): number
    multiplier = multiplier or 1.5
    return base * multiplier
end
```

### Classes

**Python:**
```python
class Weapon:
    def __init__(self, name: str, damage: int):
        self.name = name
        self.damage = damage

    def attack(self, target):
        target.health -= self.damage
        print(f"{self.name} deals {self.damage} damage!")
```

**Luau Output:**
```lua
local Weapon = {}
Weapon.__index = Weapon

function Weapon.new(name: string, damage: number)
    local self = setmetatable({}, Weapon)
    self.name = name
    self.damage = damage
    return self
end

function Weapon:attack(target)
    target.health = target.health - self.damage
    _RT.print(self.name .. " deals " .. tostring(self.damage) .. " damage!")
end
```

### Comprehensions

**Python:**
```python
squares = [x**2 for x in range(10) if x % 2 == 0]
```

**Luau Output:**
```lua
local squares = {}
for _, x in _RT.range(10) do
    if x % 2 == 0 then
        table.insert(squares, x ^ 2)
    end
end
```

### Exception Handling

**Python:**
```python
try:
    result = risky_operation()
except ValueError as e:
    print(f"Error: {e}")
finally:
    cleanup()
```

**Luau Output:**
```lua
local __ok, __err = pcall(function()
    result = risky_operation()
end)
if not __ok then
    local e = __err
    _RT.print("Error: " .. tostring(e))
end
cleanup()
```

### Roblox Integration

**Python:**
```python
from roblox import Players, Workspace

def on_player_joined(player):
    character = player.Character
    character.Humanoid.WalkSpeed = 20

Players.PlayerAdded.Connect(on_player_joined)
```

**Luau Output:**
```lua
local Players = game:GetService("Players")
local Workspace = game:GetService("Workspace")

local function on_player_joined(player)
    local character = player.Character
    character.Humanoid.WalkSpeed = 20
end

Players.PlayerAdded:Connect(on_player_joined)
```

---

## Architecture

### Directory Structure

```
MERO/
├── release/                    # Ready-to-use binaries
│   ├── MERO.exe               # Main executable (no dependencies)
│   ├── mero.dll               # Compiler engine library
│   ├── mero.ico               # Application icon
│   └── runtime/               # Luau runtime modules for Roblox
│       ├── runtime.luau       # Core runtime (builtins)
│       ├── class_system.luau  # OOP support
│       ├── module_loader.luau # Import system
│       ├── exception.luau     # Try/except support
│       ├── async_runtime.luau # Async/await
│       ├── generator.luau     # Generators (yield)
│       ├── iterator.luau      # Iterator protocol
│       ├── decorator.luau     # Decorator support
│       ├── comprehension.luau # Comprehensions
│       ├── stdlib_math.luau   # Math library
│       ├── stdlib_string.luau # String library
│       ├── stdlib_table.luau  # List/Dict/Set operations
│       ├── stdlib_io.luau     # I/O operations
│       ├── stdlib_os.luau     # OS/Time functions
│       ├── sandbox.luau       # Sandboxed execution
│       ├── error_handler.luau # Error formatting
│       ├── event_system.luau  # Roblox event bindings
│       ├── type_system.luau   # Runtime type checking
│       └── memory_manager.luau# Memory optimization
│
├── source/                     # Full source code
│   ├── src/
│   │   ├── core_c/           # C core (memory management)
│   │   ├── engine_cpp/       # C++ compiler engine
│   │   ├── python_cli/       # Python CLI tools
│   │   ├── runtime_luau/     # Luau runtime source
│   │   └── win_app/          # Windows app source
│   ├── include/mero/         # C header files
│   ├── examples/             # Example Python files
│   ├── CMakeLists.txt        # CMake build system
│   └── .github/workflows/    # CI/CD pipeline
│
└── README.md                   # This file
```

### Compiler Layers

| Layer | Language | Files | Purpose |
|-------|----------|-------|---------|
| **Core** | C | 8 | Arena allocator, string interning, hash map, error system |
| **Engine** | C++ | 28+ | Lexer, parser, AST (63 node types), IR (25+ types), optimizer, codegen |
| **CLI** | Python | 25+ | Configuration, bindings, cache, REPL, bundler, minifier |
| **Runtime** | Luau | 20+ | Class system, iterators, async, exceptions, stdlib |

### Data Flow

```
                    ┌────────────────────────────────────────────────────┐
                    │              MERO.exe / mero.dll                    │
                    │                                                      │
  input.py ──────→ │  [Lexer]──→[Parser]──→[SemanticAnalyzer]──→[IR]     │
                    │                                              │       │
                    │                                              ▼       │
                    │  [TypeChecker]←──[Optimizer]←──[IRBuilder]          │
                    │       │                                              │
                    │       ▼                                              │
                    │  [LuauCodegen]──→ output.luau                       │
                    └────────────────────────────────────────────────────┘
```

---

## Runtime Setup in Roblox Studio

### Step 1: Create Module Structure

In Roblox Studio, create this hierarchy under `ServerScriptService` or `ReplicatedStorage`:

```
MeroRuntime (Folder)
├── runtime (ModuleScript)
├── class_system (ModuleScript)
├── module_loader (ModuleScript)
├── exception (ModuleScript)
├── stdlib_math (ModuleScript)
├── stdlib_string (ModuleScript)
├── stdlib_table (ModuleScript)
├── iterator (ModuleScript)
├── generator (ModuleScript)
├── decorator (ModuleScript)
├── comprehension (ModuleScript)
├── async_runtime (ModuleScript)
├── event_system (ModuleScript)
├── sandbox (ModuleScript)
├── error_handler (ModuleScript)
├── type_system (ModuleScript)
├── memory_manager (ModuleScript)
├── stdlib_io (ModuleScript)
└── stdlib_os (ModuleScript)
```

### Step 2: Copy Runtime Code

Copy the content of each `.luau` file from `release/runtime/` into the corresponding ModuleScript.

### Step 3: Place Compiled Scripts

Your compiled `.luau` files go into Script or LocalScript objects. Make sure they can reference the MeroRuntime folder via `script.Parent`.

---

## Building from Source

### Requirements

- **CMake 3.16+**
- **C++17 compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **Python 3.10+** (for CLI tools only)
- **MinGW-w64** (for Windows cross-compilation from Linux)

### Build Commands

**Windows (MSVC):**
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

**Windows (MinGW):**
```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**Linux:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Build Standalone Executable

```cmd
pip install pyinstaller
pyinstaller --onefile --console --icon=mero.ico --name=MERO mero_app.py
```

---

## Lua vs Luau: Which to Choose?

| Feature | Lua (.lua) | Luau (.luau) |
|---------|-----------|-------------|
| Type annotations | No | Yes |
| `--!strict` mode | No | Yes |
| Continue statement | No (goto workaround) | Yes |
| Compound operators (+=) | No (expanded) | Yes |
| Floor division (//) | math.floor wrapper | math.floor wrapper |
| String interpolation | Concatenation | Concatenation |
| **Best for** | Generic Lua 5.1 | Roblox Studio |

Choose **Luau** if targeting Roblox Studio (recommended). Choose **Lua** if targeting other Lua 5.1 environments.

---

## Supported Python Builtins

The runtime provides these Python builtins in Luau:

| Builtin | Status | Notes |
|---------|--------|-------|
| `print()` | ✓ | Multiple args, sep, end |
| `len()` | ✓ | Strings and tables |
| `range()` | ✓ | 1, 2, or 3 arguments |
| `type()` | ✓ | Python-style type names |
| `int()`, `float()`, `str()`, `bool()` | ✓ | Type conversion |
| `isinstance()` | ✓ | Class hierarchy check |
| `enumerate()` | ✓ | Index + value pairs |
| `zip()` | ✓ | Parallel iteration |
| `map()`, `filter()` | ✓ | Functional operations |
| `sorted()`, `reversed()` | ✓ | Sequence operations |
| `sum()`, `min()`, `max()` | ✓ | Aggregations |
| `any()`, `all()` | ✓ | Boolean reduction |
| `abs()`, `round()` | ✓ | Math operations |
| `hasattr()`, `getattr()`, `setattr()` | ✓ | Attribute access |
| `input()` | ✓ | Text input (where available) |
| `hex()`, `oct()`, `bin()` | ✓ | Number formatting |
| `id()`, `hash()` | ✓ | Object identity |

---

## Limitations & Workarounds

| Python Feature | Limitation | Workaround |
|---|---|---|
| `eval()` / `exec()` | Not supported (sandbox) | Use predefined functions |
| `open()` file I/O | No filesystem in Roblox | Use DataStore / HttpService |
| `threading` | No OS threads | Use coroutines/Task scheduler |
| `import` (external) | No pip packages | Use MERO runtime stdlib |
| Metaclasses | Partial support | Use decorators instead |
| Multiple `*args` unpacking | Single level only | Manual unpacking |

---

## Performance

MERO's C++ engine compiles Python files significantly faster than interpreted solutions:

| File Size | Tokens | Compile Time |
|-----------|--------|-------------|
| 100 lines | ~500 | <5ms |
| 1,000 lines | ~5,000 | <20ms |
| 10,000 lines | ~50,000 | <150ms |

Memory usage is minimized through arena allocation (bulk alloc, single free) and string interning (deduplicated identifiers).

---

## Developer

**MERO:TG@QP4RM**

---

## Version History

| Version | Changes |
|---------|---------|
| 1.0.0 | Initial release — full compiler pipeline, Windows standalone, Luau runtime |

---

<p align="center">
  <em>Built with precision. Engineered for Roblox.</em>
</p>
