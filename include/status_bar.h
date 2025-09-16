/**
 * @file status_bar.h
 * @brief Player status display UI component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

class GameManager;

class StatusBar {
public:
    StatusBar();
    ~StatusBar() = default;
    
    ftxui::Element render(const GameManager& game_manager) const;
    
    ftxui::Element renderHP(int current, int max) const;
    
    ftxui::Element renderPosition(int x, int y) const;
    
    ftxui::Element renderTurn(int turn) const;
    
    ftxui::Element renderTime(int time) const;
    
    ftxui::Element renderDepth(int depth) const;
    
    ftxui::Element renderDebugInfo(const std::string& fps_info) const;
    
private:
    ftxui::Color getHPColor(int current, int max) const;
    
    std::string formatTime(int time) const;
};