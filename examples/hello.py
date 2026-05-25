"""
MERO Example - Hello World
Basic example showing Python to Luau compilation.
"""


def greet(name: str) -> str:
    return f"Hello, {name}!"


def main():
    message = greet("World")
    print(message)

    numbers = [1, 2, 3, 4, 5]
    total = sum(numbers)
    print(f"Sum: {total}")


if __name__ == "__main__":
    main()
