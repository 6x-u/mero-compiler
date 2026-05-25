"""
MERO Compiler - Project Management
Handles project initialization and structure.
Developer: MERO:TG@QP4RM
"""
import json
from pathlib import Path
from .config import CompilerConfig, save_config
from .constants import VERSION


class ProjectManager:
    """Manages MERO project structure and configuration."""

    def init_project(self, directory: Path):
        """Initialize a new MERO project in the given directory."""
        config = CompilerConfig()
        config_path = directory / "mero.json"
        save_config(config, str(config_path))

        # Create standard directories
        (directory / "src").mkdir(exist_ok=True)
        (directory / "out").mkdir(exist_ok=True)
        (directory / "tests").mkdir(exist_ok=True)

        # Create example file
        example = directory / "src" / "main.py"
        if not example.exists():
            example.write_text(self._example_source(), encoding="utf-8")

        # Create .gitignore
        gitignore = directory / ".gitignore"
        if not gitignore.exists():
            gitignore.write_text(self._gitignore_content(), encoding="utf-8")

    def find_project_root(self, start: Path = None) -> Path:
        """Find the project root by looking for mero.json."""
        current = start or Path.cwd()
        for _ in range(20):
            if (current / "mero.json").exists():
                return current
            parent = current.parent
            if parent == current:
                break
            current = parent
        return Path.cwd()

    def _example_source(self) -> str:
        return '''"""
Example MERO Python source file.
This will be compiled to Luau for Roblox.
"""


def greet(name: str) -> str:
    """Generate a greeting message."""
    return f"Hello, {name}! Welcome to MERO."


class Player:
    """Represents a Roblox player."""

    def __init__(self, name: str, level: int = 1):
        self.name = name
        self.level = level
        self.health = 100
        self.inventory = []

    def take_damage(self, amount: int):
        self.health = max(0, self.health - amount)
        if self.health == 0:
            self.respawn()

    def respawn(self):
        self.health = 100
        print(f"{self.name} respawned!")

    def add_item(self, item: str):
        self.inventory.append(item)


def main():
    player = Player("MERO_User")
    print(greet(player.name))
    player.add_item("Sword")
    player.take_damage(50)
    print(f"Health: {player.health}")


if __name__ == "__main__":
    main()
'''

    def _gitignore_content(self) -> str:
        return """# MERO output
out/
*.luau
!src/**/*.luau

# Python
__pycache__/
*.pyc
.venv/
venv/

# IDE
.vscode/
.idea/

# Cache
.mero_cache/
"""
