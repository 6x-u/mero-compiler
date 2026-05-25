"""
MERO Example - Player Class
Shows OOP compilation to Luau class system.
"""


class Player:
    def __init__(self, name: str, level: int = 1):
        self.name = name
        self.level = level
        self.health = 100
        self.max_health = 100
        self.inventory = []
        self.position = [0, 0, 0]

    def take_damage(self, amount: int):
        self.health = max(0, self.health - amount)
        if self.health == 0:
            self.on_death()

    def heal(self, amount: int):
        self.health = min(self.max_health, self.health + amount)

    def on_death(self):
        print(f"{self.name} has been defeated!")
        self.respawn()

    def respawn(self):
        self.health = self.max_health
        self.position = [0, 0, 0]
        print(f"{self.name} respawned!")

    def add_item(self, item: str):
        self.inventory.append(item)
        print(f"{self.name} picked up {item}")

    def level_up(self):
        self.level += 1
        self.max_health += 10
        self.health = self.max_health
        print(f"{self.name} reached level {self.level}!")


class Warrior(Player):
    def __init__(self, name: str):
        super().__init__(name)
        self.strength = 10
        self.defense = 5

    def attack(self, target):
        damage = self.strength * 2
        target.take_damage(damage)
        print(f"{self.name} attacks for {damage} damage!")


def main():
    hero = Warrior("MERO_Hero")
    enemy = Player("Goblin", 3)

    hero.add_item("Sword")
    hero.attack(enemy)
    hero.level_up()

    print(f"Hero health: {hero.health}")
    print(f"Enemy health: {enemy.health}")


if __name__ == "__main__":
    main()
