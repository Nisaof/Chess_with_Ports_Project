#include "ChessBoard.hpp"
#include "ConfigReader.hpp"
#include "MoveValidator.hpp"
#include "PortalSystem.hpp"
#include "GameManager.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

// Parse position string (e.g., "a1" -> Position{0, 0})
bool parsePosition(const std::string& pos_str, Position& pos, int board_size) {
  if (pos_str.length() < 2) {
    std::cerr << "Invalid position\n";
    return false;
  }
  char col = std::tolower(pos_str[0]);
  std::string row_str = pos_str.substr(1);
  if (col < 'a' || col >= 'a' + board_size) {
    std::cerr << "Invalid position\n";
    return false;
  }
  try {
    int row = std::stoi(row_str) - 1; // User enters 1-8, program uses 0-7
    if (row < 0 || row >= board_size) {
      std::cerr << "Invalid position\n";
      return false;
    }
    pos = {col - 'a', row};
    return true;
  } catch (...) {
    std::cerr << "Invalid position\n";
    return false;
  }
}

// Process command line input
bool processMoveCommand(const std::string& command, ChessBoard& board, 
                        MoveValidator& validator, PortalSystem& portal_system, 
                        GameManager& game_manager, bool is_white_turn) {
  std::istringstream iss(command);
  std::string cmd, start_str, end_str, piece;
  iss >> cmd >> start_str >> end_str >> piece;
  if (cmd != "move" || start_str.empty() || end_str.empty() || piece.empty()) {
    std::cout << "Invalid command. Example: move a1 b2 king\n";
    return false;
  }

  Position start, end;
  if (!parsePosition(start_str, start, board.getBoardSize()) || 
      !parsePosition(end_str, end, board.getBoardSize())) {
    std::cout << "Invalid position. Example: a1, b2 (within bounds)\n";
    return false;
  }

  // Check piece color on board
  const auto& start_square = board.getSquare(start);
  if (start_square.is_empty()) {
    std::cout << "No piece at starting position.\n";
    return false;
  }

  // Check if piece matches current turn
  if (start_square.is_white != is_white_turn) {
    std::cout << (is_white_turn ? "White" : "Black") << " player's turn. "
              << (start_square.is_white ? "White" : "Black") << " piece selected.\n";
    return false;
  }

  // Check if piece type matches input
  std::string piece_lower = validator.toLowerCase(piece);
  std::string square_piece_lower = validator.toLowerCase(start_square.piece);
  if (piece_lower != square_piece_lower) {
    std::cout << "Piece at starting position (" << start_square.piece
              << ") does not match specified piece (" << piece << ").\n";
    return false;
  }

  // Validate and apply move
  if (validator.isValidMove(piece, start, end, start_square.is_white, board, portal_system)) {
    board.movePiece(start, end, validator, portal_system, game_manager);
    std::cout << "Move successful: " << start_str << " -> " << end_str << "\n";
    board.printBoard();
    return true;
  } else {
    std::cout << "Invalid move: " << piece << " from " << start_str << " to " << end_str << "\n";
    return false;
  }
}

int main(int argc, char* argv[]) {
  if (!std::cin.good()) {
    std::cerr << "Input error\n";
    return 1;
  }

  std::string config_file = (argc > 1) ? argv[1] : "data/chess_pieces.json";
  ConfigReader config_reader;
  if (!config_reader.loadFromFile(config_file)) {
    std::cerr << "Failed to load configuration file\n";
    return 1;
  }

  std::string display_format = (argc > 2 && std::string(argv[2]) == "simple") ? "simple" : "detailed";
  int board_size = config_reader.getConfig().game_settings.board_size;
  
  if (board_size <= 0 || board_size > 26) {
    std::cerr << "Invalid board size\n";
    return 1;
  }

  ChessBoard board(board_size, display_format);
  board.initializeBoard(config_reader.getConfig().pieces);
  MoveValidator validator;
  PortalSystem portal_system(config_reader.getConfig().portals);
  GameManager game_manager(board, validator, portal_system);

  std::cout << "Initial board:\n";
  board.printBoard();
  std::cout << "Commands: move <start> <end> <piece> (e.g., move a1 b2 king), undo, quit\n";

  bool is_white_turn = true;
  std::string command;
  while (true) {
    std::cout << (is_white_turn ? "White" : "Black") << " player's turn > ";
    std::cout.flush();
    
    if (!std::getline(std::cin, command)) {
      break;
    }

    if (command == "quit") {
      std::cout << "Game ended.\n";
      break;
    }

    if (command == "undo") {
      game_manager.undoMove();
      board.printBoard();
      is_white_turn = !is_white_turn;
      continue;
    }

    if (!command.empty()) {
      if (processMoveCommand(command, board, validator, portal_system, game_manager, is_white_turn)) {
        if (game_manager.isCheckmate(!is_white_turn)) {
          std::cout << (is_white_turn ? "White" : "Black") << " checkmate! Game over.\n";
          break;
        }
        if (game_manager.isStalemate(!is_white_turn)) {
          std::cout << "Game ended in stalemate.\n";
          break;
        }
        is_white_turn = !is_white_turn;
      }
    } else {
      std::cout << "Empty command. Example: move a1 b2 king\n";
    }
  }

  return 0;
}