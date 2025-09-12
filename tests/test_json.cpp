#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

TEST_CASE("JSON library basic functionality", "[json]") {
    SECTION("Create and parse JSON") {
        json j = {
            {"name", "Veyrm"},
            {"version", "0.0.2"},
            {"features", {"roguelike", "terminal", "json"}}
        };
        
        REQUIRE(j["name"] == "Veyrm");
        REQUIRE(j["version"] == "0.0.2");
        REQUIRE(j["features"].size() == 3);
    }
    
    SECTION("Serialize and deserialize") {
        json original = {
            {"hp", 100},
            {"attack", 10},
            {"defense", 5}
        };
        
        std::string serialized = original.dump();
        json parsed = json::parse(serialized);
        
        REQUIRE(parsed["hp"] == 100);
        REQUIRE(parsed["attack"] == 10);
        REQUIRE(parsed["defense"] == 5);
    }
    
    SECTION("Handle game data structures") {
        json monster = {
            {"id", "gutter_rat"},
            {"glyph", "r"},
            {"color", "grey"},
            {"hp", 3},
            {"atk", {1, 3}},
            {"def", 0},
            {"speed", 100}
        };
        
        REQUIRE(monster["id"] == "gutter_rat");
        REQUIRE(monster["hp"] == 3);
        REQUIRE(monster["atk"][0] == 1);
        REQUIRE(monster["atk"][1] == 3);
    }
}

TEST_CASE("JSON error handling", "[json]") {
    SECTION("Invalid JSON throws exception") {
        std::string invalid = "{invalid json}";
        REQUIRE_THROWS_AS(json::parse(invalid), json::parse_error);
    }
    
    SECTION("Missing keys return null") {
        json j = {{"exists", true}};
        REQUIRE(j["missing"].is_null());
    }
}