"""
MERO Compiler - Roblox API Mappings
Maps Python patterns to Roblox-specific Luau APIs.
Developer: MERO:TG@QP4RM
"""
from typing import Dict, List, Optional


ROBLOX_SERVICES = {
    "Players": "game:GetService('Players')",
    "Workspace": "workspace",
    "ReplicatedStorage": "game:GetService('ReplicatedStorage')",
    "ServerStorage": "game:GetService('ServerStorage')",
    "ServerScriptService": "game:GetService('ServerScriptService')",
    "StarterGui": "game:GetService('StarterGui')",
    "StarterPack": "game:GetService('StarterPack')",
    "StarterPlayer": "game:GetService('StarterPlayer')",
    "Lighting": "game:GetService('Lighting')",
    "SoundService": "game:GetService('SoundService')",
    "TweenService": "game:GetService('TweenService')",
    "RunService": "game:GetService('RunService')",
    "UserInputService": "game:GetService('UserInputService')",
    "HttpService": "game:GetService('HttpService')",
    "DataStoreService": "game:GetService('DataStoreService')",
    "MessagingService": "game:GetService('MessagingService')",
    "MarketplaceService": "game:GetService('MarketplaceService')",
    "TeleportService": "game:GetService('TeleportService')",
    "PathfindingService": "game:GetService('PathfindingService')",
    "PhysicsService": "game:GetService('PhysicsService')",
    "CollectionService": "game:GetService('CollectionService')",
    "Chat": "game:GetService('Chat')",
    "Teams": "game:GetService('Teams')",
    "BadgeService": "game:GetService('BadgeService')",
}

ROBLOX_DATATYPES = {
    "Vector3": {"new": "Vector3.new", "zero": "Vector3.zero", "one": "Vector3.one"},
    "Vector2": {"new": "Vector2.new", "zero": "Vector2.zero"},
    "CFrame": {"new": "CFrame.new", "identity": "CFrame.identity"},
    "Color3": {"new": "Color3.new", "fromRGB": "Color3.fromRGB", "fromHSV": "Color3.fromHSV"},
    "UDim2": {"new": "UDim2.new", "fromScale": "UDim2.fromScale", "fromOffset": "UDim2.fromOffset"},
    "UDim": {"new": "UDim.new"},
    "Rect": {"new": "Rect.new"},
    "Ray": {"new": "Ray.new"},
    "Region3": {"new": "Region3.new"},
    "NumberRange": {"new": "NumberRange.new"},
    "NumberSequence": {"new": "NumberSequence.new"},
    "ColorSequence": {"new": "ColorSequence.new"},
    "TweenInfo": {"new": "TweenInfo.new"},
    "BrickColor": {"new": "BrickColor.new", "random": "BrickColor.random"},
}

ROBLOX_ENUMS = [
    "Enum.EasingStyle", "Enum.EasingDirection", "Enum.Material",
    "Enum.PartType", "Enum.Font", "Enum.KeyCode",
    "Enum.UserInputType", "Enum.HumanoidStateType",
    "Enum.RenderPriority", "Enum.SortOrder", "Enum.ScaleType",
]


class RobloxAPIMapper:
    """Maps Python patterns to Roblox Luau APIs."""

    def __init__(self):
        self.services = ROBLOX_SERVICES
        self.datatypes = ROBLOX_DATATYPES

    def is_roblox_service(self, name: str) -> bool:
        return name in self.services

    def get_service_code(self, name: str) -> Optional[str]:
        return self.services.get(name)

    def is_roblox_datatype(self, name: str) -> bool:
        return name in self.datatypes

    def map_datatype_call(self, typename: str, method: str, args: List[str]) -> Optional[str]:
        dt = self.datatypes.get(typename)
        if not dt:
            return None
        luau_func = dt.get(method)
        if not luau_func:
            return None
        return f"{luau_func}({', '.join(args)})"

    def generate_service_imports(self, used_services: List[str]) -> str:
        """Generate Luau service variable declarations."""
        lines = []
        for service in used_services:
            if service in self.services:
                lines.append(f"local {service} = {self.services[service]}")
        return "\n".join(lines)

    def map_event_connection(self, obj: str, event: str, handler: str) -> str:
        """Map Python event handler to Roblox connection."""
        return f"{obj}.{event}:Connect({handler})"

    def map_instance_creation(self, class_name: str) -> str:
        """Map Instance creation."""
        return f'Instance.new("{class_name}")'
