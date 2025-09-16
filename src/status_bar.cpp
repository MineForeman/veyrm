#include "status_bar.h"
#include "game_state.h"
#include "turn_manager.h"
#include "frame_stats.h"
#include "ecs/game_world.h"
#include "ecs/position_component.h"
#include "ecs/health_component.h"
#include "ecs/player_component.h"
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <iomanip>

using namespace ftxui;

StatusBar::StatusBar() {}

Element StatusBar::render(const GameManager& game_manager) const {
    auto tm = game_manager.getTurnManager();

    // Get player data from ECS if available
    int hp = game_manager.player_hp;
    int max_hp = game_manager.player_max_hp;
    int x = game_manager.player_x;
    int y = game_manager.player_y;
    int depth = 1;

    // Try to get data directly from ECS
    auto* ecs_world = const_cast<GameManager&>(game_manager).getECSWorld();
    if (ecs_world) {
        auto* player_entity = ecs_world->getPlayerEntity();
        if (player_entity) {
            // Get position from ECS
            if (auto* pos = player_entity->getComponent<ecs::PositionComponent>()) {
                x = pos->position.x;
                y = pos->position.y;
            }
            // Get health from ECS
            if (auto* health = player_entity->getComponent<ecs::HealthComponent>()) {
                hp = health->hp;
                max_hp = health->max_hp;
            }
            // Get player-specific data from ECS
            if ([[maybe_unused]] auto* player_comp = player_entity->getComponent<ecs::PlayerComponent>()) {
                // Could display level, experience, gold here if needed
                // For now just use depth 1
            }
        }
    }

    // First line: HP and Position
    std::vector<Element> line1_elements;
    line1_elements.push_back(renderHP(hp, max_hp) | size(WIDTH, EQUAL, 15));
    line1_elements.push_back(separator());
    line1_elements.push_back(text("Position: ") | size(WIDTH, EQUAL, 10));
    line1_elements.push_back(renderPosition(x, y));

    // Second line: Turn, Time, and Depth
    std::vector<Element> line2_elements;
    line2_elements.push_back(renderTurn(tm->getCurrentTurn()) | size(WIDTH, EQUAL, 10));
    line2_elements.push_back(separator());
    line2_elements.push_back(renderTime(tm->getWorldTime()) | size(WIDTH, EQUAL, 10));
    line2_elements.push_back(separator());
    line2_elements.push_back(renderDepth(depth) | size(WIDTH, EQUAL, 10));

    if (game_manager.isDebugMode() && game_manager.getFrameStats()) {
        line2_elements.push_back(separator());
        line2_elements.push_back(renderDebugInfo(game_manager.getFrameStats()->format()));
    }

    // Stack the two lines vertically with separator
    return vbox({
        hbox(line1_elements),
        separatorHeavy(),  // Horizontal line between the two lines
        hbox(line2_elements)
    }) | border | size(HEIGHT, EQUAL, 5);  // Increased height for 2 lines + separator
}

Element StatusBar::renderHP(int current, int max) const {
    Color hp_color = getHPColor(current, max);
    return text("HP: " + std::to_string(current) + "/" + std::to_string(max)) 
           | bold 
           | color(hp_color);
}

Element StatusBar::renderPosition(int x, int y) const {
    return text(std::to_string(x) + ", " + std::to_string(y));
}

Element StatusBar::renderTurn(int turn) const {
    return text("Turn: " + std::to_string(turn));
}

Element StatusBar::renderTime(int time) const {
    return text("Time: " + formatTime(time));
}

Element StatusBar::renderDepth(int depth) const {
    return text("Depth: " + std::to_string(depth));
}

Element StatusBar::renderDebugInfo(const std::string& fps_info) const {
    return text(fps_info) | color(Color::Cyan);
}

Color StatusBar::getHPColor(int current, int max) const {
    if (max == 0) return Color::Red;
    
    float ratio = static_cast<float>(current) / static_cast<float>(max);
    
    if (ratio > 0.75f) {
        return Color::Green;
    } else if (ratio > 0.25f) {
        return Color::Yellow;
    } else {
        return Color::Red;
    }
}

std::string StatusBar::formatTime(int time) const {
    if (time < 100) {
        return std::to_string(time);
    }
    
    int hours = time / 3600;
    int minutes = (time % 3600) / 60;
    int seconds = time % 60;
    
    std::ostringstream oss;
    if (hours > 0) {
        oss << hours << "h" << std::setfill('0') << std::setw(2) << minutes << "m";
    } else if (minutes > 0) {
        oss << minutes << "m" << std::setfill('0') << std::setw(2) << seconds << "s";
    } else {
        oss << seconds << "s";
    }
    
    return oss.str();
}