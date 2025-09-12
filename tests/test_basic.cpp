#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

TEST_CASE("Basic C++ functionality", "[basic]") {
    SECTION("String operations") {
        std::string game = "Veyrm";
        REQUIRE(game.length() == 5);
        REQUIRE(game[0] == 'V');
    }
    
    SECTION("Vector operations") {
        std::vector<int> hp_values = {10, 20, 30};
        REQUIRE(hp_values.size() == 3);
        REQUIRE(hp_values[1] == 20);
    }
    
    SECTION("UTF-8 string handling") {
        std::string walls = "═║╔╗╚╝";
        REQUIRE(!walls.empty());
        // Note: UTF-8 chars are multi-byte
        REQUIRE(walls.length() > 6);
    }
}

TEST_CASE("Game constants", "[game]") {
    SECTION("Map dimensions") {
        constexpr int MAP_WIDTH = 80;
        constexpr int MAP_HEIGHT = 24;
        
        REQUIRE(MAP_WIDTH > 0);
        REQUIRE(MAP_HEIGHT > 0);
        REQUIRE(MAP_WIDTH * MAP_HEIGHT == 1920);
    }
    
    SECTION("Entity limits") {
        constexpr int MAX_INVENTORY = 10;
        constexpr int MAX_MONSTERS = 50;
        
        REQUIRE(MAX_INVENTORY > 0);
        REQUIRE(MAX_MONSTERS > MAX_INVENTORY);
    }
}