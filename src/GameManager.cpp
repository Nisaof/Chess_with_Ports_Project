#include "GameManager.hpp"
#include "ChessBoard.hpp"
#include "MoveValidator.hpp"
#include "PortalSystem.hpp"
#include <stdexcept>
#include <iostream>

GameManager::GameManager(ChessBoard& board, MoveValidator& validator, PortalSystem& portal_system)
    : chess_board(board), validator(validator), portal_system(portal_system) {
}

bool GameManager::isInCheck(bool is_white_turn) const {
    
    Position king_position;
    bool king_found = false;

    // Find the king
    for (int y = 0; y < chess_board.getBoardSize() && !king_found; ++y) {
        for (int x = 0; x < chess_board.getBoardSize() && !king_found; ++x) {
            const auto& square = chess_board.getSquare({x, y});
            if (validator.toLowerCase(square.piece) == "king" && square.is_white == is_white_turn) {
                king_position = {x, y};
                king_found = true;
            }
        }
    }

    if (!king_found) {
        return false;
    }

    // Check for threats
    const std::vector<std::string> threatening_pieces = {"queen", "rook", "bishop", "knight", "pawn"};
    
    for (int y = 0; y < chess_board.getBoardSize(); ++y) {
        for (int x = 0; x < chess_board.getBoardSize(); ++x) {
            const auto& square = chess_board.getSquare({x, y});
            // Only check opponent pieces
            if (!square.is_empty() && square.is_white != is_white_turn) {
                std::string piece_lower = validator.toLowerCase(square.piece);
                // Check only threatening pieces
                if (std::find(threatening_pieces.begin(), threatening_pieces.end(), piece_lower) != threatening_pieces.end()) {
                    Position start = {x, y};
                    // Check if piece threatens the king
                    if (validator.isValidMove(square.piece, start, king_position, !is_white_turn, 
                                           chess_board, portal_system)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool GameManager::isCheckmate(bool is_white_turn) {
    // First check if in check
    if (!isInCheck(is_white_turn)) {
        return false;
    }

    // Check all pieces and possible moves
    for (int y = 0; y < chess_board.getBoardSize(); ++y) {
        for (int x = 0; x < chess_board.getBoardSize(); ++x) {
            const auto& square = chess_board.getSquare({x, y});
            if (!square.is_empty() && square.is_white == is_white_turn) {
                Position start = {x, y};
                
                // Check all possible destination squares
                for (int dy = 0; dy < chess_board.getBoardSize(); ++dy) {
                    for (int dx = 0; dx < chess_board.getBoardSize(); ++dx) {
                        Position end = {dx, dy};
                        if (start.x == end.x && start.y == end.y) continue;
                        
                        // Check if move is valid
                        if (validator.isValidMove(square.piece, start, end, is_white_turn,
                                               chess_board, portal_system)) {
                            // Create temporary board
                            ChessBoard temp_board = chess_board;
                            try {
                                // Make the move
                                auto captured_piece = temp_board.getSquare(end);
                                temp_board.placePiece(square.piece, square.is_white, end.x, end.y);
                                temp_board.placePiece("", false, start.x, start.y);
                                
                                // Check if still in check after move
                                GameManager temp_manager(temp_board, validator, portal_system);
                                if (!temp_manager.isInCheck(is_white_turn)) {
                                    return false; // Saving move exists
                                }
                            } catch (const std::exception& e) {
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }

    return true; // No saving move found
}

bool GameManager::isStalemate(bool is_white_turn) const {
    if (isInCheck(is_white_turn)) {
        return false;
    }

    for (int y = 0; y < chess_board.getBoardSize(); ++y) {
        for (int x = 0; x < chess_board.getBoardSize(); ++x) {
            const auto& square = chess_board.getSquare({x, y});
            if (!square.is_empty() && square.is_white == is_white_turn) {
                Position start = {x, y};
                for (int dy = -chess_board.getBoardSize(); dy <= chess_board.getBoardSize(); ++dy) {
                    for (int dx = -chess_board.getBoardSize(); dx <= chess_board.getBoardSize(); ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        Position end = {x + dx, y + dy};
                        if (chess_board.isInBounds(end) && 
                            validator.isValidMove(square.piece, start, end, is_white_turn, 
                                                 chess_board, portal_system)) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void GameManager::addToMoveHistory(const Move& move) {
    move_history.push(move);
}

void GameManager::undoMove() {
    if (move_history.empty()) {
        std::cout << "No moves to undo." << std::endl;
        return;
    }

    Move last_move = move_history.top();
    move_history.pop();

    try {
        // Restore the moved piece to its original position
        chess_board.placePiece(last_move.moved_piece, last_move.moved_piece_color, 
                             last_move.start.x, last_move.start.y);
        
        // If a piece was captured, restore it
        if (!last_move.captured_piece.empty()) {
            chess_board.placePiece(last_move.captured_piece, last_move.captured_piece_color, 
                                 last_move.end.x, last_move.end.y);
        } else {
            // If no piece was captured, clear the destination square
            chess_board.placePiece("", false, last_move.end.x, last_move.end.y);
        }

        std::cout << "Move undone: " << last_move.moved_piece << " from " 
                  << last_move.end.x << "," << last_move.end.y << " to "
                  << last_move.start.x << "," << last_move.start.y << std::endl;

        portal_system.updateCooldowns();
    } catch (const std::exception& e) {
        // On error, restore the move to history and report the error
        move_history.push(last_move);
        std::cerr << "Error undoing move: " << e.what() << std::endl;
    }
}