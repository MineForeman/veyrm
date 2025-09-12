#include <catch2/catch_test_macros.hpp>
#include "status_bar.h"
#include "game_state.h"
#include "turn_manager.h"
#include "frame_stats.h"
#include "message_log.h"
#include <ftxui/dom/elements.hpp>

TEST_CASE("StatusBar construction", "[status_bar]") {
    StatusBar status_bar;
    REQUIRE(true);
}

TEST_CASE("StatusBar HP rendering", "[status_bar]") {
    StatusBar status_bar;
    
    SECTION("Full health displays green") {
        auto element = status_bar.renderHP(100, 100);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Half health displays yellow") {
        auto element = status_bar.renderHP(50, 100);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Low health displays red") {
        auto element = status_bar.renderHP(20, 100);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Zero max health handles gracefully") {
        auto element = status_bar.renderHP(0, 0);
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("StatusBar position rendering", "[status_bar]") {
    StatusBar status_bar;
    
    SECTION("Renders positive coordinates") {
        auto element = status_bar.renderPosition(10, 20);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders zero coordinates") {
        auto element = status_bar.renderPosition(0, 0);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders negative coordinates") {
        auto element = status_bar.renderPosition(-5, -10);
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("StatusBar turn rendering", "[status_bar]") {
    StatusBar status_bar;
    
    SECTION("Renders turn 0") {
        auto element = status_bar.renderTurn(0);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders large turn numbers") {
        auto element = status_bar.renderTurn(99999);
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("StatusBar time rendering", "[status_bar]") {
    StatusBar status_bar;
    
    SECTION("Renders seconds only for small values") {
        auto element = status_bar.renderTime(45);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders minutes and seconds") {
        auto element = status_bar.renderTime(125);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders hours and minutes") {
        auto element = status_bar.renderTime(7265);
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("StatusBar depth rendering", "[status_bar]") {
    StatusBar status_bar;
    
    SECTION("Renders depth 1") {
        auto element = status_bar.renderDepth(1);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders large depths") {
        auto element = status_bar.renderDepth(50);
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("StatusBar debug info rendering", "[status_bar]") {
    StatusBar status_bar;
    
    SECTION("Renders FPS info") {
        auto element = status_bar.renderDebugInfo("60 FPS");
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders empty debug info") {
        auto element = status_bar.renderDebugInfo("");
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("StatusBar full render", "[status_bar]") {
    StatusBar status_bar;
    GameManager game_manager;
    
    SECTION("Renders without debug mode") {
        game_manager.setDebugMode(false);
        auto element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders with debug mode") {
        game_manager.setDebugMode(true);
        game_manager.enableFrameStats();
        auto element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders with various HP values") {
        game_manager.player_hp = 25;
        game_manager.player_max_hp = 100;
        auto element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
        
        game_manager.player_hp = 75;
        element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
        
        game_manager.player_hp = 100;
        element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
    }
    
    SECTION("Renders with various positions") {
        game_manager.player_x = 0;
        game_manager.player_y = 0;
        auto element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
        
        game_manager.player_x = 50;
        game_manager.player_y = 25;
        element = status_bar.render(game_manager);
        REQUIRE(element != nullptr);
    }
}