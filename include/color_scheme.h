#pragma once

#include <ftxui/screen/color.hpp>
#include <string>

using Color = ftxui::Color;

enum class TerminalTheme {
    DARK,
    LIGHT,
    HIGH_CONTRAST,
    AUTO_DETECT
};

class ColorScheme {
public:
    struct Colors {
        // Terrain colors
        Color wall;
        Color wall_memory;
        Color floor;
        Color floor_memory;
        Color void_tile;
        
        // Entity colors
        Color player;
        Color monster;
        Color item;
        
        // UI colors
        Color ui_border;
        Color ui_text;
        Color ui_highlight;
        
        // Status colors
        Color health_high;
        Color health_medium;
        Color health_low;
        Color health_critical;
    };
    
    // Get predefined themes
    static Colors getDarkTheme();
    static Colors getLightTheme();
    static Colors getHighContrastTheme();
    
    // Auto-detect best theme based on terminal
    static TerminalTheme detectTerminalTheme();
    static Colors getTheme(TerminalTheme theme);
    
    // Get/Set current theme
    static void setCurrentTheme(TerminalTheme theme);
    static TerminalTheme getCurrentTheme();
    static const Colors& getCurrentColors();
    
    // Theme names for UI
    static std::string getThemeName(TerminalTheme theme);
    
private:
    static TerminalTheme current_theme;
    static Colors current_colors;
    
    // Initialize with auto-detected theme
    static void initialize();
};