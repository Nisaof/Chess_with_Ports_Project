#include "PortalSystem.hpp"
#include <algorithm>
#include <iostream>

PortalSystem::PortalSystem(const std::vector<PortalConfig>& portals) : portals_(portals) {
    for (const auto& portal : portals_) {
        cooldowns_[portal.id] = 0;
    }
}

bool PortalSystem::isPortalMove(const Position& start, const Position& end) const {
    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            return true;
        }
    }
    return false;
}

bool PortalSystem::validatePortalMove(const std::string& piece, const Position& start, 
                                     const Position& end, bool is_white_turn, 
                                     const ChessBoard& board) const {
    const auto& square = board.getSquare(start);
    if (square.is_empty() || square.piece != piece || square.is_white != is_white_turn) {
        return false;
    }

    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            
            // First check cooldown
            if (isPortalInCooldown(start, end)) {
                return false;
            }
            
            // Check color restrictions
            std::string color = is_white_turn ? "white" : "black";
            auto it = std::find(portal.properties.allowed_colors.begin(), 
                               portal.properties.allowed_colors.end(), color);
            if (it == portal.properties.allowed_colors.end()) {
                std::cout << "\nPortal Error: This portal cannot be used by " << color << " pieces!" << std::endl;
                return false;
            }
            
            return true;
        }
    }
    return false;
}

void PortalSystem::handlePortalMove(const Position& start, const Position& end, ChessBoard& board) {
    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            auto square = board.getSquare(start);
            if (!square.is_empty()) {
                board.placePiece(square.piece, square.is_white, end.x, end.y);
                board.placePiece("", false, start.x, start.y);
                
                // Set cooldown count
                cooldowns_[portal.id] = portal.properties.cooldown;
                
                // Add to queue
                for (int i = 0; i < portal.properties.cooldown; i++) {
                    cooldown_queue_.push(portal.id);
                }
            }
            break;
        }
    }
}

bool PortalSystem::isPortalInCooldown(const Position& start, const Position& end) const {
    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            auto cooldown_it = cooldowns_.find(portal.id);
            if (cooldown_it != cooldowns_.end() && cooldown_it->second > 0) {
                std::cout << "\nPortal " << portal.id << " is on cooldown! "
                          << "Remaining turns: " << cooldown_it->second << std::endl;
                std::cout << "This portal cannot be used by any piece right now." << std::endl;
                return true;
            }
            return false;
        }
    }
    return false;
}

void PortalSystem::updateCooldowns() {
    if (cooldown_queue_.empty()) {
        return;
    }

    // Decrease cooldown for one portal each turn
    std::string portal_id = cooldown_queue_.front();
    cooldown_queue_.pop();
    
    auto it = cooldowns_.find(portal_id);
    if (it != cooldowns_.end() && it->second > 0) {
        it->second--;
        if (it->second == 0) {
            std::cout << "\nPortal " << portal_id << " is now ready for use!" << std::endl;
        }
    }
    
    // Show cooldown statuses
    bool has_cooldowns = false;
    for (const auto& pair : cooldowns_) {
        if (pair.second > 0) {
            if (!has_cooldowns) {
                std::cout << "\n--- PORTAL COOLDOWN STATUS ---";
                has_cooldowns = true;
            }
            std::cout << "\n" << pair.first << " -> Remaining cooldown: " << pair.second << " turns";
        }
    }
    if (has_cooldowns) {
        std::cout << "\n!!" << std::endl;
    }
}