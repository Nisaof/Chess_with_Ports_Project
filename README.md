# Chess with Portals

A modern C++ chess game implementation featuring a unique portal system that adds strategic depth to traditional chess gameplay.

## Features

- **Classic Chess Rules**: Full implementation of standard chess rules including castling, en passant, pawn promotion, check, checkmate, and stalemate detection
- **Portal System**: Strategic portals that allow pieces to teleport across the board with configurable properties:
  - Direction preservation options
  - Color restrictions
  - Cooldown mechanisms
- **Customizable Configuration**: JSON-based configuration system for:
  - Custom board sizes (up to 26x26)
  - Custom piece types and movements
  - Portal placement and properties
  - Game settings
- **Game Management**:
  - Move validation
  - Move history tracking
  - Undo functionality
  - Turn-based gameplay
- **Interactive CLI**: Command-line interface for playing the game

## Requirements

- C++20 compatible compiler (GCC, Clang, or MSVC)
- Make (for building)
- curl (for downloading dependencies)

## Building

The project uses a Makefile for building:

```bash
# Build the project (automatically downloads dependencies)
make

# Clean build artifacts
make clean

# Clean everything including dependencies
make distclean
```

The build process will automatically download the nlohmann/json library if it's not present.

## Running the Game

### Method 1: Using Make (Recommended)

```bash
# Run with default configuration
make run
```

### Method 2: Direct Execution

```bash
# Run with default configuration file
./bin/chess_game data/chess_pieces.json

# Run with simple display format
./bin/chess_game data/chess_pieces.json simple
```

## Gameplay

### Commands

- `move <start> <end> <piece>` - Move a piece (e.g., `move a1 b2 king`)
- `undo` - Undo the last move
- `quit` - Exit the game

### Example Game Session

```
Initial board:
[Board display]
Commands: move <start> <end> <piece> (e.g., move a1 b2 king), undo, quit

White player's turn > move e2 e4 pawn
Move successful: e2 -> e4
[Board display]

Black player's turn > move e7 e5 pawn
Move successful: e7 -> e5
[Board display]

White player's turn > quit
Game ended.
```

### Position Notation

Positions use standard chess notation:
- Columns: a, b, c, d, e, f, g, h (for 8x8 board)
- Rows: 1, 2, 3, 4, 5, 6, 7, 8
- Example: `a1` (bottom-left), `h8` (top-right)

### Piece Names

Use English piece names (case-insensitive):
- `king`, `queen`, `rook`, `bishop`, `knight`, `pawn`

## Configuration

The game is configured via JSON files. See `data/chess_pieces.json` for an example configuration.

### Configuration Structure

```json
{
  "game_settings": {
    "name": "Custom Chess",
    "board_size": 8,
    "turn_limit": 100
  },
  "pieces": [...],
  "custom_pieces": [...],
  "portals": [
    {
      "type": "Portal",
      "id": "portal1",
      "positions": {
        "entry": { "x": 2, "y": 3 },
        "exit": { "x": 5, "y": 4 }
      },
      "properties": {
        "preserve_direction": true,
        "allowed_colors": ["white", "black"],
        "cooldown": 1
      }
    }
  ]
}
```

### Portal Properties

- `preserve_direction`: If true, pieces maintain their movement direction when exiting the portal
- `allowed_colors`: Array of colors that can use the portal ("white", "black", or both)
- `cooldown`: Number of turns before the portal can be used again

## Project Structure

```
.
├── bin/              # Compiled executable
├── data/             # Configuration files
├── include/          # Header files
│   ├── ChessBoard.hpp
│   ├── ConfigReader.hpp
│   ├── GameManager.hpp
│   ├── MoveValidator.hpp
│   ├── Piece.hpp
│   └── PortalSystem.hpp
├── obj/              # Object files
├── src/              # Source files
│   ├── ChessBoard.cpp
│   ├── ConfigReader.cpp
│   ├── GameManager.cpp
│   ├── main.cpp
│   ├── MoveValidator.cpp
│   ├── Piece.cpp
│   └── PortalSystem.cpp
├── third_party/      # External dependencies
│   └── nlohmann/     # JSON library
├── Makefile
└── README.md
```

## Architecture

- **ChessBoard**: Manages the game board state and piece placement
- **ConfigReader**: Parses JSON configuration files
- **GameManager**: Handles game logic, check/checkmate detection, and move history
- **MoveValidator**: Validates piece movements according to chess rules
- **PortalSystem**: Manages portal mechanics and cooldowns

## Data Structures

The implementation uses the following C++ standard library data structures:

- **`std::unordered_map`**: 
  - Board state storage (`ChessBoard::board`) - maps position strings to squares
  - Portal cooldown tracking (`PortalSystem::cooldowns_`) - maps portal IDs to cooldown values
  - Piece positions (`PieceConfig::positions`) - maps color strings to position vectors
  - Custom abilities (`SpecialAbilities::custom_abilities`) - maps ability names to boolean values

- **`std::vector`**: 
  - Portal configurations (`PortalSystem::portals_`)
  - Piece configurations (`GameConfig::pieces`, `GameConfig::custom_pieces`)
  - Position lists for pieces
  - Move edges in pathfinding (`MoveValidator::getMoveEdges`)
  - Allowed colors for portals

- **`std::stack`**: 
  - Move history (`GameManager::move_history`) - enables undo functionality by storing moves in LIFO order

- **`std::queue`**: 
  - Portal cooldown queue (`PortalSystem::cooldown_queue_`) - manages cooldown decrements turn by turn

- **`std::unordered_set`**: 
  - Visited positions in BFS validation (`MoveValidator::bfsValidateMove`) - tracks explored squares during pathfinding

- **`std::string`**: 
  - Piece names, position notation, portal IDs, and configuration parsing

- **`nlohmann::json`**: 
  - JSON parsing for configuration files (`ConfigReader`)

## Algorithm Complexity

### Pathfinding with BFS

The `MoveValidator::bfsValidateMove` function uses **Breadth-First Search (BFS)** to validate piece movements, especially for complex paths that may involve portals. This algorithm ensures that all reachable positions are explored systematically.

**Time Complexity**: O(V + E)
- **V**: Number of vertices (board squares) = board_size²
- **E**: Number of edges (possible moves from each square)

**Space Complexity**: O(V)
- Stores visited positions in an `std::unordered_set`
- Queue can hold at most V positions

The BFS approach is particularly useful for:
- Validating multi-step piece movements
- Finding paths through portals
- Ensuring move legality across the entire board
- Handling custom piece movement patterns

For a standard 8×8 chess board, this results in O(64 + E) complexity, where E depends on the piece type and current board state.

## Game Rules

### Standard Chess Rules

The game implements all standard chess rules:

1. **Piece Movements**:
   - **King**: Moves one square in any direction
   - **Queen**: Moves any number of squares horizontally, vertically, or diagonally
   - **Rook**: Moves any number of squares horizontally or vertically
   - **Bishop**: Moves any number of squares diagonally
   - **Knight**: Moves in an L-shape (2 squares in one direction, then 1 square perpendicular)
   - **Pawn**: 
     - Moves forward one square (or two on first move)
     - Captures diagonally
     - Promotes to Queen, Rook, Bishop, or Knight when reaching the opposite end

2. **Special Moves**:
   - **Castling**: King moves two squares toward a rook, and the rook moves to the square the king crossed
   - **En Passant**: Pawn can capture an opponent's pawn that just moved two squares forward
   - **Pawn Promotion**: When a pawn reaches the opposite end, it must be promoted to another piece

3. **Game States**:
   - **Check**: King is under attack by an opponent's piece
   - **Checkmate**: King is in check and no legal move can remove the threat
   - **Stalemate**: Player has no legal moves but is not in check (game ends in draw)

4. **Turn-Based Play**: 
   - Players alternate turns (White moves first)
   - A player cannot move their opponent's pieces
   - A player cannot move into check (exposing their own king)

### Portal Rules

The portal system adds unique mechanics:

1. **Portal Usage**:
   - Pieces can teleport by moving to a portal entry point
   - The piece automatically exits at the portal's exit point
   - Portal usage is subject to cooldown and color restrictions

2. **Portal Properties**:
   - **Direction Preservation**: If enabled, pieces maintain their movement direction when exiting
   - **Color Restrictions**: Portals can be configured to allow only white pieces, only black pieces, or both
   - **Cooldown**: After use, a portal may be unavailable for a specified number of turns

3. **Portal Validation**:
   - Portal must not be on cooldown
   - Piece color must be allowed by the portal
   - Exit square must be valid (within bounds, not occupied by same color)

4. **Move History**:
   - Portal teleportation is recorded as a separate move in the history
   - Undo functionality restores both the initial move and portal teleportation

## Troubleshooting

### Executable Not Found

If you get "executable not found", rebuild the project:
```bash
make clean
make
```

### Configuration File Not Found

Make sure you're running from the project root directory, or provide the relative path to the configuration file:
```bash
./bin/chess_game data/chess_pieces.json
```

### Permission Denied

If you get a permission error, make the executable executable:
```bash
chmod +x bin/chess_game
```

## License

This project is provided as-is for educational and personal use.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.
