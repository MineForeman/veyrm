/**
 * @file color_scheme.h
 * @brief Color scheme and theming system for Veyrm
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/screen/color.hpp>
#include <string>

using Color = ftxui::Color;

/**
 * @enum TerminalTheme
 * @brief Available color themes for the game
 */
enum class TerminalTheme {
    DARK,           ///< Dark theme (default)
    LIGHT,          ///< Light theme for bright terminals
    HIGH_CONTRAST,  ///< High contrast theme for accessibility
    AUTO_DETECT     ///< Automatically detect best theme
};

/**
 * @class ColorScheme
 * @brief Manages color themes and provides color constants for UI elements
 *
 * The ColorScheme class handles all color management for the game, providing
 * different themes optimized for various terminal environments. It supports
 * dark, light, and high-contrast themes, with automatic detection of the
 * best theme for the current terminal.
 *
 * @see TerminalTheme
 * @see RendererTUI
 */
class ColorScheme {
public:
    /**
     * @struct Colors
     * @brief Collection of colors for all game elements
     */
    struct Colors {
        // Terrain colors
        Color wall;         ///< Color for wall tiles
        Color wall_memory;  ///< Color for remembered walls (not currently visible)
        Color floor;        ///< Color for floor tiles
        Color floor_memory; ///< Color for remembered floors (not currently visible)
        Color void_tile;    ///< Color for unexplored/void areas

        // Entity colors
        Color player;       ///< Color for the player character
        Color monster;      ///< Color for monsters
        Color item;         ///< Color for items

        // UI colors
        Color ui_border;    ///< Color for UI borders and frames
        Color ui_text;      ///< Color for regular UI text
        Color ui_highlight; ///< Color for highlighted/selected UI elements

        // Status colors
        Color health_high;     ///< Color for high health (75-100%)
        Color health_medium;   ///< Color for medium health (50-74%)
        Color health_low;      ///< Color for low health (25-49%)
        Color health_critical; ///< Color for critical health (0-24%)
    };

    /**
     * @brief Get dark theme color palette
     * @return Colors configured for dark terminals
     */
    static Colors getDarkTheme();

    /**
     * @brief Get light theme color palette
     * @return Colors configured for light terminals
     */
    static Colors getLightTheme();

    /**
     * @brief Get high contrast theme color palette
     * @return Colors configured for accessibility
     */
    static Colors getHighContrastTheme();

    /**
     * @brief Auto-detect best theme based on terminal capabilities
     * @return Detected terminal theme
     */
    static TerminalTheme detectTerminalTheme();

    /**
     * @brief Get color palette for specified theme
     * @param theme The theme to get colors for
     * @return Colors for the specified theme
     */
    static Colors getTheme(TerminalTheme theme);

    /**
     * @brief Set the active color theme
     * @param theme Theme to activate
     */
    static void setCurrentTheme(TerminalTheme theme);

    /**
     * @brief Get the currently active theme
     * @return Current terminal theme
     */
    static TerminalTheme getCurrentTheme();

    /**
     * @brief Get the current color palette
     * @return Reference to current color configuration
     */
    static const Colors& getCurrentColors();

    /**
     * @brief Get human-readable name for a theme
     * @param theme Theme to get name for
     * @return String name of the theme
     */
    static std::string getThemeName(TerminalTheme theme);
    
private:
    static TerminalTheme current_theme; ///< Currently active theme
    static Colors current_colors;       ///< Current color configuration

    /**
     * @brief Initialize color scheme with auto-detected theme
     * @note Called automatically on first use
     */
    static void initialize();
};