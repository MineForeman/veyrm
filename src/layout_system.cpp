#include "layout_system.h"
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <sstream>

using namespace ftxui;

LayoutSystem::LayoutSystem() 
    : terminal_dims{80, 24}, 
      map_dims{60, 22},
      status_dims{19, 10},
      log_dims{19, 11},
      terminal_valid(true) {
    calculateLayout();
}

void LayoutSystem::updateDimensions(int terminal_width, int terminal_height) {
    terminal_dims.width = terminal_width;
    terminal_dims.height = terminal_height;
    calculateLayout();
}

void LayoutSystem::calculateLayout() {
    // Check minimum terminal size
    terminal_valid = (terminal_dims.width >= LayoutConfig::MIN_TERMINAL_WIDTH &&
                     terminal_dims.height >= LayoutConfig::MIN_TERMINAL_HEIGHT);
    
    if (!terminal_valid) {
        // Set minimal dimensions even if invalid
        map_dims = {LayoutConfig::MIN_MAP_WIDTH, LayoutConfig::MIN_MAP_HEIGHT};
        status_dims = {LayoutConfig::MIN_STATUS_WIDTH, LayoutConfig::MIN_STATUS_HEIGHT};
        log_dims = {LayoutConfig::MIN_STATUS_WIDTH, LayoutConfig::MIN_LOG_HEIGHT};
        return;
    }
    
    // The layout looks like:
    // ┌─map──┬─status─┐
    // │      │        │
    // │      ├────────┤
    // │      │  log   │
    // └──────┴────────┘
    
    // We need to fit: map_width + status_width + borders within terminal_width
    // Borders take up space: left(1) + middle(1) + right(1) = 3 chars
    int usable_width = terminal_dims.width - 3;
    int usable_height = terminal_dims.height - 2; // top and bottom borders
    
    // Start with ideal proportions
    int ideal_map_width = static_cast<int>(usable_width * LayoutConfig::MAP_WIDTH_RATIO);
    int ideal_status_width = usable_width - ideal_map_width;
    
    // Apply minimum constraints
    map_dims.width = std::max(ideal_map_width, LayoutConfig::MIN_MAP_WIDTH);
    status_dims.width = std::max(ideal_status_width, LayoutConfig::MIN_STATUS_WIDTH);
    
    // If minimums exceed available space, prioritize map
    if (map_dims.width + status_dims.width > usable_width) {
        // Try to fit both at minimum sizes
        if (LayoutConfig::MIN_MAP_WIDTH + LayoutConfig::MIN_STATUS_WIDTH <= usable_width) {
            map_dims.width = LayoutConfig::MIN_MAP_WIDTH;
            status_dims.width = usable_width - map_dims.width;
        } else {
            // Terminal is too small even for minimums
            map_dims.width = LayoutConfig::MIN_MAP_WIDTH;
            status_dims.width = LayoutConfig::MIN_STATUS_WIDTH;
        }
    }
    
    log_dims.width = status_dims.width;
    
    // Calculate heights
    map_dims.height = usable_height;
    
    // Split right column height between status and log
    int right_column_height = usable_height;
    int target_status_height = static_cast<int>(right_column_height * LayoutConfig::STATUS_HEIGHT_RATIO);
    
    // Ensure minimum heights
    status_dims.height = std::max(target_status_height, LayoutConfig::MIN_STATUS_HEIGHT);
    status_dims.height = std::min(status_dims.height, right_column_height - LayoutConfig::MIN_LOG_HEIGHT - 1);
    
    // Log gets remaining height
    log_dims.height = right_column_height - status_dims.height - 1; // -1 for separator
    log_dims.height = std::max(log_dims.height, LayoutConfig::MIN_LOG_HEIGHT);
}

std::string LayoutSystem::getTerminalSizeError() const {
    if (terminal_valid) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "Terminal too small! Minimum size: " 
        << LayoutConfig::MIN_TERMINAL_WIDTH << "x" 
        << LayoutConfig::MIN_TERMINAL_HEIGHT
        << " (Current: " << terminal_dims.width << "x" 
        << terminal_dims.height << ")";
    return oss.str();
}

Decorator LayoutSystem::applyMapLayout() const {
    return size(WIDTH, EQUAL, map_dims.width + 2) |  // +2 for borders
           size(HEIGHT, EQUAL, map_dims.height + 2);  // +2 for borders
}

Decorator LayoutSystem::applyStatusLayout() const {
    return size(WIDTH, EQUAL, status_dims.width + 2) |  // +2 for borders
           size(HEIGHT, EQUAL, status_dims.height + 2);  // +2 for borders
}

Decorator LayoutSystem::applyLogLayout() const {
    return size(WIDTH, EQUAL, log_dims.width + 2) |  // +2 for borders
           size(HEIGHT, EQUAL, log_dims.height + 2);  // +2 for borders
}

Component LayoutSystem::createLayoutContainer(
    Component map_panel,
    Component status_panel,
    Component log_panel) const {
    
    // If terminal is too small, show error message
    if (!terminal_valid) {
        return Renderer([this] {
            return vbox({
                text(getTerminalSizeError()) | color(Color::Red) | bold | center,
                separator(),
                text("Please resize your terminal") | center
            }) | border | center;
        });
    }
    
    // Create the three-panel layout
    // Left side: Map
    // Right side: Status (top) and Log (bottom)
    return Container::Horizontal({
        map_panel | applyMapLayout(),
        Container::Vertical({
            status_panel | applyStatusLayout(),
            log_panel | applyLogLayout()
        })
    });
}