#include <catch2/catch_test_macros.hpp>
#include "layout_system.h"

TEST_CASE("LayoutSystem: Default dimensions", "[layout]") {
    LayoutSystem layout;
    
    SECTION("Default terminal size is valid") {
        REQUIRE(layout.isTerminalSizeValid() == true);
    }
    
    SECTION("Default panel dimensions") {
        auto map_dims = layout.getMapDimensions();
        auto status_dims = layout.getStatusDimensions();
        auto log_dims = layout.getLogDimensions();
        
        // Map should take most of the space
        REQUIRE(map_dims.width >= LayoutSystem::LayoutConfig::MIN_MAP_WIDTH);
        REQUIRE(map_dims.height >= LayoutSystem::LayoutConfig::MIN_MAP_HEIGHT);
        
        // Status panel minimum dimensions  
        REQUIRE(status_dims.width >= LayoutSystem::LayoutConfig::MIN_STATUS_WIDTH);
        REQUIRE(status_dims.height >= LayoutSystem::LayoutConfig::MIN_STATUS_HEIGHT);
        
        // Log panel minimum dimensions (same width as status)
        REQUIRE(log_dims.width == status_dims.width);
        REQUIRE(log_dims.height >= LayoutSystem::LayoutConfig::MIN_LOG_HEIGHT);
    }
}

TEST_CASE("LayoutSystem: Minimum terminal size validation", "[layout]") {
    LayoutSystem layout;
    
    SECTION("Terminal too small - width") {
        layout.updateDimensions(79, 24);  // One less than minimum width
        REQUIRE(layout.isTerminalSizeValid() == false);
        REQUIRE(layout.getTerminalSizeError().find("Terminal too small") != std::string::npos);
    }
    
    SECTION("Terminal too small - height") {
        layout.updateDimensions(80, 23);  // One less than minimum height
        REQUIRE(layout.isTerminalSizeValid() == false);
        REQUIRE(layout.getTerminalSizeError().find("Terminal too small") != std::string::npos);
    }
    
    SECTION("Terminal exactly minimum size") {
        layout.updateDimensions(80, 24);
        REQUIRE(layout.isTerminalSizeValid() == true);
        REQUIRE(layout.getTerminalSizeError() == "");
    }
    
    SECTION("Terminal larger than minimum") {
        layout.updateDimensions(120, 40);
        REQUIRE(layout.isTerminalSizeValid() == true);
        REQUIRE(layout.getTerminalSizeError() == "");
    }
}

TEST_CASE("LayoutSystem: Responsive layout calculation", "[layout]") {
    LayoutSystem layout;
    
    SECTION("Small terminal (80x24)") {
        layout.updateDimensions(80, 24);
        
        auto map_dims = layout.getMapDimensions();
        auto status_dims = layout.getStatusDimensions();
        auto log_dims = layout.getLogDimensions();
        
        // Check panels fit within terminal
        // Layout: |map|status| with 3 chars for borders
        int total_width = map_dims.width + status_dims.width + 3; // +3 for borders
        REQUIRE(total_width <= 80);
        
        // Height should be used efficiently
        int right_column_height = status_dims.height + log_dims.height + 1; // +1 for separator
        REQUIRE(right_column_height <= 22); // 24 - 2 for top/bottom borders
    }
    
    SECTION("Medium terminal (100x30)") {
        layout.updateDimensions(100, 30);
        
        auto map_dims = layout.getMapDimensions();
        auto status_dims = layout.getStatusDimensions();
        auto log_dims = layout.getLogDimensions();
        
        // Map should take majority of width (but status has minimum width)
        float width_ratio = static_cast<float>(map_dims.width) / (map_dims.width + status_dims.width);
        REQUIRE(width_ratio >= 0.50f);  // At least half
        REQUIRE(width_ratio <= 0.80f);  // But not too much
        
        // All panels should meet or exceed minimums  
        REQUIRE(map_dims.width >= LayoutSystem::LayoutConfig::MIN_MAP_WIDTH);
        REQUIRE(status_dims.height >= LayoutSystem::LayoutConfig::MIN_STATUS_HEIGHT);
        REQUIRE(log_dims.height >= LayoutSystem::LayoutConfig::MIN_LOG_HEIGHT);
    }
    
    SECTION("Large terminal (160x50)") {
        layout.updateDimensions(160, 50);
        
        auto map_dims = layout.getMapDimensions();
        auto status_dims = layout.getStatusDimensions();
        auto log_dims = layout.getLogDimensions();
        
        // Map should be much larger
        REQUIRE(map_dims.width > 100);
        REQUIRE(map_dims.height > 40);
        
        // Right column should also be larger
        REQUIRE(status_dims.width > LayoutSystem::LayoutConfig::MIN_STATUS_WIDTH);
        REQUIRE(status_dims.height > LayoutSystem::LayoutConfig::MIN_STATUS_HEIGHT);
        REQUIRE(log_dims.height > LayoutSystem::LayoutConfig::MIN_LOG_HEIGHT);
    }
}

TEST_CASE("LayoutSystem: Layout proportions", "[layout]") {
    LayoutSystem layout;
    
    SECTION("Status vs Log height ratio") {
        layout.updateDimensions(100, 40);
        
        auto status_dims = layout.getStatusDimensions();
        auto log_dims = layout.getLogDimensions();
        
        // Status should take roughly 40% of right column height
        float total_right_height = static_cast<float>(status_dims.height + log_dims.height);
        float status_ratio = status_dims.height / total_right_height;
        
        // Allow some flexibility due to minimum size constraints
        REQUIRE(status_ratio >= 0.30f);
        REQUIRE(status_ratio <= 0.50f);
    }
    
    SECTION("Map width dominance") {
        layout.updateDimensions(120, 35);
        
        auto map_dims = layout.getMapDimensions();
        auto status_dims = layout.getStatusDimensions();
        
        // Map should always be wider than status panel
        REQUIRE(map_dims.width > status_dims.width);
        
        // Map should take majority of width
        float total_width = static_cast<float>(map_dims.width + status_dims.width);
        float map_ratio = map_dims.width / total_width;
        REQUIRE(map_ratio >= 0.65f);
    }
}

TEST_CASE("LayoutSystem: Edge cases", "[layout]") {
    LayoutSystem layout;
    
    SECTION("Very small terminal") {
        layout.updateDimensions(40, 15);
        
        REQUIRE(layout.isTerminalSizeValid() == false);
        
        // Even when invalid, dimensions should be set to minimums
        auto map_dims = layout.getMapDimensions();
        REQUIRE(map_dims.width == LayoutSystem::LayoutConfig::MIN_MAP_WIDTH);
        REQUIRE(map_dims.height == LayoutSystem::LayoutConfig::MIN_MAP_HEIGHT);
    }
    
    SECTION("Extremely wide terminal") {
        layout.updateDimensions(300, 24);
        
        REQUIRE(layout.isTerminalSizeValid() == true);
        
        auto map_dims = layout.getMapDimensions();
        auto status_dims = layout.getStatusDimensions();
        
        // Should still maintain reasonable proportions
        REQUIRE(map_dims.width < 250);  // Some reasonable maximum
        REQUIRE(status_dims.width >= LayoutSystem::LayoutConfig::MIN_STATUS_WIDTH);
    }
    
    SECTION("Extremely tall terminal") {
        layout.updateDimensions(80, 100);
        
        REQUIRE(layout.isTerminalSizeValid() == true);
        
        auto map_dims = layout.getMapDimensions();
        auto log_dims = layout.getLogDimensions();
        
        // Height should be used effectively
        REQUIRE(map_dims.height > 80);  // Most of the height
        REQUIRE(log_dims.height > LayoutSystem::LayoutConfig::MIN_LOG_HEIGHT);
    }
}

TEST_CASE("LayoutSystem: Terminal size error messages", "[layout]") {
    LayoutSystem layout;
    
    SECTION("Error message format") {
        layout.updateDimensions(60, 20);
        
        auto error = layout.getTerminalSizeError();
        REQUIRE(error.find("80x24") != std::string::npos);  // Shows minimum
        REQUIRE(error.find("60x20") != std::string::npos);  // Shows current
    }
    
    SECTION("No error when valid") {
        layout.updateDimensions(100, 30);
        REQUIRE(layout.getTerminalSizeError() == "");
    }
}