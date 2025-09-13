#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

/**
 * LayoutSystem manages the responsive layout of the game UI
 * 
 * The layout consists of three main panels:
 * - Map Panel: Main game view (left side)
 * - Status Panel: Player stats and info (right top)
 * - Log Panel: Message log (right bottom)
 * 
 * Layout adjusts to terminal size while maintaining minimum requirements:
 * - Minimum terminal size: 80x24
 * - Map panel: Takes most of the space
 * - Status panel: Fixed height for essential info
 * - Log panel: Variable height, shows recent messages
 */
class LayoutSystem {
public:
    struct Dimensions {
        int width;
        int height;
    };
    
    struct LayoutConfig {
        // Minimum terminal dimensions
        static constexpr int MIN_TERMINAL_WIDTH = 80;
        static constexpr int MIN_TERMINAL_HEIGHT = 24;
        
        // Panel minimum dimensions
        static constexpr int MIN_MAP_WIDTH = 50;
        static constexpr int MIN_MAP_HEIGHT = 20;
        static constexpr int MIN_STATUS_WIDTH = 27;  // Reduced to fit in 80 cols
        static constexpr int MIN_STATUS_HEIGHT = 10;
        static constexpr int MIN_LOG_HEIGHT = 5;
        
        // Preferred panel ratios (when space allows)
        static constexpr float MAP_WIDTH_RATIO = 0.75f;  // Map takes 75% of width
        static constexpr float STATUS_HEIGHT_RATIO = 0.4f;  // Status takes 40% of right column height
    };
    
    LayoutSystem();
    ~LayoutSystem() = default;
    
    // Update layout based on terminal dimensions
    void updateDimensions(int terminal_width, int terminal_height);
    
    // Get calculated dimensions for each panel
    Dimensions getMapDimensions() const { return map_dims; }
    Dimensions getStatusDimensions() const { return status_dims; }
    Dimensions getLogDimensions() const { return log_dims; }
    
    // Check if terminal meets minimum size requirements
    bool isTerminalSizeValid() const { return terminal_valid; }
    
    // Get terminal size error message
    std::string getTerminalSizeError() const;
    
    // Apply responsive sizing to components
    ftxui::Decorator applyMapLayout() const;
    ftxui::Decorator applyStatusLayout() const;
    ftxui::Decorator applyLogLayout() const;
    
    // Get the full layout container structure
    ftxui::Component createLayoutContainer(
        ftxui::Component map_panel,
        ftxui::Component status_panel,
        ftxui::Component log_panel
    ) const;
    
private:
    Dimensions terminal_dims;
    Dimensions map_dims;
    Dimensions status_dims;
    Dimensions log_dims;
    bool terminal_valid;
    
    void calculateLayout();
};