#include "color_scheme.h"
#include <cstdlib>

// Static member definitions
TerminalTheme ColorScheme::current_theme = TerminalTheme::AUTO_DETECT;
ColorScheme::Colors ColorScheme::current_colors = ColorScheme::getDarkTheme();

ColorScheme::Colors ColorScheme::getDarkTheme() {
    return {
        // Terrain - bright colors on dark background
        .wall = Color::Cyan,
        .wall_memory = Color::Blue,
        .floor = Color::GrayLight,
        .floor_memory = Color::GrayDark,
        .void_tile = Color::Black,
        
        // Entities
        .player = Color::White,
        .monster = Color::Red,
        .item = Color::Yellow,
        
        // UI
        .ui_border = Color::Cyan,
        .ui_text = Color::White,
        .ui_highlight = Color::Yellow,
        
        // Status
        .health_high = Color::Green,
        .health_medium = Color::Yellow,
        .health_low = Color::Magenta,
        .health_critical = Color::Red
    };
}

ColorScheme::Colors ColorScheme::getLightTheme() {
    return {
        // Terrain - dark colors on light background
        .wall = Color::Blue,
        .wall_memory = Color::GrayDark,
        .floor = Color::Black,
        .floor_memory = Color::GrayDark,
        .void_tile = Color::GrayLight,
        
        // Entities
        .player = Color::Black,
        .monster = Color::Red,
        .item = Color::Blue,
        
        // UI
        .ui_border = Color::Blue,
        .ui_text = Color::Black,
        .ui_highlight = Color::Magenta,
        
        // Status
        .health_high = Color::Green,
        .health_medium = Color::Yellow,
        .health_low = Color::Magenta,
        .health_critical = Color::Red
    };
}

ColorScheme::Colors ColorScheme::getHighContrastTheme() {
    return {
        // Maximum contrast colors
        .wall = Color::Yellow,
        .wall_memory = Color::Blue,
        .floor = Color::White,
        .floor_memory = Color::GrayDark,
        .void_tile = Color::Black,
        
        // Entities
        .player = Color::Green,
        .monster = Color::Red,
        .item = Color::Cyan,
        
        // UI
        .ui_border = Color::Yellow,
        .ui_text = Color::White,
        .ui_highlight = Color::Magenta,
        
        // Status
        .health_high = Color::Green,
        .health_medium = Color::Yellow,
        .health_low = Color::Magenta,
        .health_critical = Color::Red
    };
}

TerminalTheme ColorScheme::detectTerminalTheme() {
    // Check common environment variables for theme hints
    const char* term_program = std::getenv("TERM_PROGRAM");
    [[maybe_unused]] const char* colorterm = std::getenv("COLORTERM");
    const char* term = std::getenv("TERM");
    
    // Check for light theme indicators
    if (term_program) {
        std::string tp(term_program);
        // Terminal.app on macOS often uses light themes
        if (tp == "Apple_Terminal") {
            // Could check further, but default to high contrast for safety
            return TerminalTheme::HIGH_CONTRAST;
        }
    }
    
    // Check for dark theme indicators (most terminals default to dark)
    if (term) {
        std::string t(term);
        if (t.find("256color") != std::string::npos || 
            t.find("xterm") != std::string::npos) {
            return TerminalTheme::DARK;
        }
    }
    
    // Default to dark theme (most common)
    return TerminalTheme::DARK;
}

ColorScheme::Colors ColorScheme::getTheme(TerminalTheme theme) {
    switch (theme) {
        case TerminalTheme::LIGHT:
            return getLightTheme();
        case TerminalTheme::HIGH_CONTRAST:
            return getHighContrastTheme();
        case TerminalTheme::AUTO_DETECT:
            return getTheme(detectTerminalTheme());
        case TerminalTheme::DARK:
        default:
            return getDarkTheme();
    }
}

void ColorScheme::setCurrentTheme(TerminalTheme theme) {
    current_theme = theme;
    current_colors = getTheme(theme);
}

TerminalTheme ColorScheme::getCurrentTheme() {
    if (current_theme == TerminalTheme::AUTO_DETECT) {
        current_theme = detectTerminalTheme();
        current_colors = getTheme(current_theme);
    }
    return current_theme;
}

const ColorScheme::Colors& ColorScheme::getCurrentColors() {
    if (current_theme == TerminalTheme::AUTO_DETECT) {
        current_theme = detectTerminalTheme();
        current_colors = getTheme(current_theme);
    }
    return current_colors;
}

std::string ColorScheme::getThemeName(TerminalTheme theme) {
    switch (theme) {
        case TerminalTheme::DARK:
            return "Dark";
        case TerminalTheme::LIGHT:
            return "Light";
        case TerminalTheme::HIGH_CONTRAST:
            return "High Contrast";
        case TerminalTheme::AUTO_DETECT:
            return "Auto-Detect";
        default:
            return "Unknown";
    }
}

void ColorScheme::initialize() {
    if (current_theme == TerminalTheme::AUTO_DETECT) {
        current_theme = detectTerminalTheme();
        current_colors = getTheme(current_theme);
    }
}