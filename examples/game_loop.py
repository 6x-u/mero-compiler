"""
MERO Example - Game Loop
Shows async/event patterns for Roblox game development.
"""


class GameState:
    def __init__(self):
        self.players = {}
        self.score = 0
        self.round_active = False
        self.round_time = 60

    def add_player(self, player_id: str, name: str):
        self.players[player_id] = {
            "name": name,
            "score": 0,
            "alive": True,
        }

    def remove_player(self, player_id: str):
        if player_id in self.players:
            del self.players[player_id]

    def start_round(self):
        self.round_active = True
        self.score = 0
        for pid in self.players:
            self.players[pid]["alive"] = True
            self.players[pid]["score"] = 0
        print("Round started!")

    def end_round(self):
        self.round_active = False
        winner = self.get_winner()
        if winner:
            print(f"Winner: {winner['name']} with {winner['score']} points!")
        else:
            print("Round ended - no winner")

    def get_winner(self):
        best = None
        for pid in self.players:
            p = self.players[pid]
            if best is None or p["score"] > best["score"]:
                best = p
        return best

    def player_scored(self, player_id: str, points: int):
        if player_id in self.players and self.round_active:
            self.players[player_id]["score"] += points
            print(f"{self.players[player_id]['name']} scored {points} points!")


def on_player_joined(game: GameState, player_id: str, name: str):
    game.add_player(player_id, name)
    print(f"{name} joined the game")
    if len(game.players) >= 2 and not game.round_active:
        game.start_round()


def on_player_left(game: GameState, player_id: str):
    if player_id in game.players:
        name = game.players[player_id]["name"]
        game.remove_player(player_id)
        print(f"{name} left the game")


def main():
    game = GameState()
    on_player_joined(game, "p1", "Alice")
    on_player_joined(game, "p2", "Bob")
    game.player_scored("p1", 10)
    game.player_scored("p2", 15)
    game.end_round()


if __name__ == "__main__":
    main()
