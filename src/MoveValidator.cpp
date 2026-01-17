// MoveValidator.cpp
#include "MoveValidator.hpp"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iostream>

std::string MoveValidator::toLowerCase(const std::string& str) const {
  std::string lower = str;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { 
    return std::tolower(c); 
  });
  return lower;
}

std::vector<Position> MoveValidator::getMoveEdges(const std::string& piece_lower, 
                                                  const Position& pos, bool is_white, 
                                                  const ChessBoard& board) const {
    std::vector<Position> edges;
    int forward = is_white ? 1 : -1;  // White moves up, black moves down

    std::string piece_type = toLowerCase(piece_lower);

    if (piece_type == "pawn") {
        // Normal forward movement
        Position forward_one = {pos.x, pos.y + forward};
        if (board.isInBounds(forward_one) && board.getSquare(forward_one).is_empty()) {
            edges.push_back(forward_one);
            
            // First move: 2 squares forward
            if ((is_white && pos.y == 1) || (!is_white && pos.y == 6)) {
                Position forward_two = {pos.x, pos.y + 2 * forward};
                if (board.isInBounds(forward_two) && board.getSquare(forward_two).is_empty() &&
                    board.getSquare(forward_one).is_empty()) {
                    edges.push_back(forward_two);
                }
            }
        }

        // Diagonal capture moves
        std::vector<Position> captures = {
            {pos.x - 1, pos.y + forward},
            {pos.x + 1, pos.y + forward}
        };

        for (const auto& capture : captures) {
            if (board.isInBounds(capture)) {
                const auto& target = board.getSquare(capture);
                if (!target.is_empty() && target.is_white != is_white) {
                    edges.push_back(capture);
                }
            }
        }
    } else if (piece_type == "knight") {
        std::vector<std::pair<int, int>> knight_moves = {
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
        };
        for (const auto& move : knight_moves) {
            Position p = {pos.x + move.first, pos.y + move.second};
            if (board.isInBounds(p)) {
                const auto& target_square = board.getSquare(p);
                if (target_square.is_empty() || target_square.is_white != is_white) {
                    edges.push_back(p);
                }
            }
        }
    } else if (piece_type == "bishop") {
        std::vector<std::pair<int, int>> directions = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
        for (const auto& dir : directions) {
            for (int i = 1; i < board.getBoardSize(); ++i) {
                Position p = {pos.x + i * dir.first, pos.y + i * dir.second};
                if (!board.isInBounds(p)) break;
                const auto& target = board.getSquare(p);
                edges.push_back(p);
                if (!target.is_empty()) break;
            }
        }
    } else if (piece_type == "rook") {
        std::vector<std::pair<int, int>> directions = {{0,1}, {0,-1}, {1,0}, {-1,0}};
        for (const auto& dir : directions) {
            for (int i = 1; i < board.getBoardSize(); ++i) {
                Position p = {pos.x + i * dir.first, pos.y + i * dir.second};
                if (!board.isInBounds(p)) break;
                const auto& target = board.getSquare(p);
                edges.push_back(p);
                if (!target.is_empty()) break;
            }
        }
    } else if (piece_type == "queen") {
        std::vector<std::pair<int, int>> directions = {
            {0,1}, {0,-1}, {1,0}, {-1,0},
            {1,1}, {1,-1}, {-1,1}, {-1,-1}
        };
        for (const auto& dir : directions) {
            for (int i = 1; i < board.getBoardSize(); ++i) {
                Position p = {pos.x + i * dir.first, pos.y + i * dir.second};
                if (!board.isInBounds(p)) break;
                const auto& target = board.getSquare(p);
                edges.push_back(p);
                if (!target.is_empty()) break;
            }
        }
    } else if (piece_type == "king") {
        std::vector<std::pair<int, int>> directions = {
            {0,1}, {0,-1}, {1,0}, {-1,0},
            {1,1}, {1,-1}, {-1,1}, {-1,-1}
        };
        for (const auto& dir : directions) {
            Position p = {pos.x + dir.first, pos.y + dir.second};
            if (board.isInBounds(p)) {
                const auto& target_square = board.getSquare(p);
                if (target_square.is_empty() || target_square.is_white != is_white) {
                    edges.push_back(p);
                }
            }
        }
    }

    return edges;
}

bool MoveValidator::bfsValidateMove(const std::string& piece_lower, const Position& start, 
                                    const Position& end, bool is_white, 
                                    const ChessBoard& board, 
                                    const PortalSystem& portal_system) const {
  if (!board.isInBounds(start) || !board.isInBounds(end)) {
    return false;
  }

  // Hedef karede aynı renk taş varsa hareket geçersiz
  const auto& end_square = board.getSquare(end);
  if (!end_square.is_empty() && end_square.is_white == is_white) {
    return false;
  }

  // BFS için kuyruk ve ziyaret edilen kareler
  std::queue<Position> queue;
  std::unordered_set<std::string> visited;
  auto posToString = [](const Position& p) { 
    return std::to_string(p.x) + "," + std::to_string(p.y); 
  };

  queue.push(start);
  visited.insert(posToString(start));

  while (!queue.empty()) {
    Position current = queue.front();
    queue.pop();

    // Hedefe ulaşıldıysa
    if (current.x == end.x && current.y == end.y) {
      return true;
    }

    // Taşın hareket kenarları
    auto edges = getMoveEdges(piece_lower, current, is_white, board);

    // Portal bağlantıları
    if (piece_lower == "teleporter" && portal_system.isPortalMove(current, end)) {
      Position portal_exit = end; // Portal çıkış pozisyonu
      if (board.isInBounds(portal_exit)) {
        std::string exit_str = posToString(portal_exit);
        if (visited.find(exit_str) == visited.end()) {
          queue.push(portal_exit);
          visited.insert(exit_str);
        }
      }
    }

    // Normal hareket kenarları
    for (const auto& next : edges) {
      if (!board.isInBounds(next)) continue;
      std::string next_str = posToString(next);
      if (visited.find(next_str) != visited.end()) continue;

      // Hedef kareye engel kontrolü
      if (!board.getSquare(next).is_empty() && 
          (next.x != end.x || next.y != end.y)) {
        continue; // Engelle karşılaşılırsa bu yoldan devam etme
      }

      queue.push(next);
      visited.insert(next_str);
    }
  }

  return false;
}

bool MoveValidator::isValidMove(const std::string& piece, const Position& start, 
                               const Position& end, bool is_white, 
                               const ChessBoard& board, 
                               const PortalSystem& portal_system) const {
    // Başlangıç pozisyonunu kontrol et
    if (!board.isInBounds(start)) {
        return false;
    }

    // Başlangıç karesindeki taşı kontrol et
    const auto& start_square = board.getSquare(start);
    if (start_square.is_empty() || 
        toLowerCase(start_square.piece) != toLowerCase(piece) || 
        start_square.is_white != is_white) {
        return false;
    }

    // Hedef pozisyonu kontrol et
    if (!board.isInBounds(end)) {
        return false;
    }

    // Hedef karede kendi taşımız varsa geçersiz
    const auto& end_square = board.getSquare(end);
    if (!end_square.is_empty() && end_square.is_white == is_white) {
        return false;
    }

    std::string piece_lower = toLowerCase(piece);

    // Rok kontrolü
    if (piece_lower == "king" && abs(end.x - start.x) == 2 && end.y == start.y) {
        return validateCastling(start, end, is_white, board);
    }

    // Piyon özel hareketleri
    if (piece_lower == "pawn") {
        // En passant kontrolü
        if (isEnPassantMove(start, end, is_white, board)) {
            return true;
        }
        
        // Terfi kontrolü - son sıraya ulaşma
        if ((is_white && end.y == 7) || (!is_white && end.y == 0)) {
            // Hareket geçerliyse terfi edilebilir
            auto valid_moves = getMoveEdges(piece_lower, start, is_white, board);
            for (const auto& move : valid_moves) {
                if (move.x == end.x && move.y == end.y) {
                    return true;
                }
            }
            return false;
        }
    }

    // Portal move check
    if (portal_system.isPortalMove(start, end)) {
        std::cout << "\nPortal move detected!" << std::endl;
        bool valid = portal_system.validatePortalMove(piece, start, end, is_white, board);
        if (!valid) {
            std::cout << "Portal cannot be used - Cooldown or color restriction may apply." << std::endl;
        }
        return valid;
    }

    // Normal hareket kontrolü
    auto valid_moves = getMoveEdges(piece_lower, start, is_white, board);
    for (const auto& move : valid_moves) {
        if (move.x == end.x && move.y == end.y) {
            return true;
        }
    }

    return false;
}

bool MoveValidator::validateCastling(const Position& start, const Position& end, 
                                   bool is_white, const ChessBoard& board) const {
    // Check if king has moved
    if (start.x != 4 || start.y != (is_white ? 0 : 7)) {
        return false;
    }

    // Determine if kingside or queenside castling
    bool is_kingside = end.x > start.x;
    int rook_x = is_kingside ? 7 : 0;
    
    // Check if rook is in place and hasn't moved
    Position rook_pos = {rook_x, start.y};
    const auto& rook_square = board.getSquare(rook_pos);
    if (rook_square.is_empty() || toLowerCase(rook_square.piece) != "rook" || 
        rook_square.is_white != is_white) {
        return false;
    }

    // Check if there are pieces between king and rook
    int step = is_kingside ? 1 : -1;
    for (int x = start.x + step; x != rook_x; x += step) {
        if (!board.getSquare({x, start.y}).is_empty()) {
            return false;
        }
    }

    return true;
}

bool MoveValidator::isEnPassantMove(const Position& start, const Position& end,
                                  bool is_white, const ChessBoard& board) const {
    // En passant can only occur on 5th rank (for white) or 4th rank (for black)
    if ((is_white && start.y != 4) || (!is_white && start.y != 3)) {
        return false;
    }

    // Must be diagonal movement
    if (abs(end.x - start.x) != 1 || end.y != (is_white ? 5 : 2)) {
        return false;
    }

    // Destination square must be empty
    if (!board.getSquare(end).is_empty()) {
        return false;
    }

    // Captured pawn must have moved 2 squares in the last move
    Position captured_pos = {end.x, start.y};
    const auto& captured_square = board.getSquare(captured_pos);
    if (captured_square.is_empty() || toLowerCase(captured_square.piece) != "pawn" ||
        captured_square.is_white == is_white) {
        return false;
    }

    return true;
}