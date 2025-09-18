#include <catch2/catch_test_macros.hpp>
#include <boost/json.hpp>
#include <string>

using json = boost::json::value;

TEST_CASE("JSON library basic functionality", "[json]") {
    SECTION("Create and parse JSON") {
        boost::json::object j;
        j["name"] = "Veyrm";
        j["version"] = "0.0.2";
        boost::json::array features;
        features.push_back("roguelike");
        features.push_back("terminal");
        features.push_back("json");
        j["features"] = features;

        REQUIRE(boost::json::value_to<std::string>(j["name"]) == "Veyrm");
        REQUIRE(boost::json::value_to<std::string>(j["version"]) == "0.0.2");
        REQUIRE(j["features"].as_array().size() == 3);
    }
    
    SECTION("Serialize and deserialize") {
        boost::json::object original;
        original["hp"] = 100;
        original["attack"] = 10;
        original["defense"] = 5;

        std::string serialized = boost::json::serialize(original);
        boost::json::value parsed = boost::json::parse(serialized);

        REQUIRE(boost::json::value_to<int>(parsed.as_object().at("hp")) == 100);
        REQUIRE(boost::json::value_to<int>(parsed.as_object().at("attack")) == 10);
        REQUIRE(boost::json::value_to<int>(parsed.as_object().at("defense")) == 5);
    }
    
    SECTION("Handle game data structures") {
        boost::json::object monster;
        monster["id"] = "gutter_rat";
        monster["glyph"] = "r";
        monster["color"] = "grey";
        monster["hp"] = 3;
        boost::json::array atk;
        atk.push_back(1);
        atk.push_back(3);
        monster["atk"] = atk;
        monster["def"] = 0;
        monster["speed"] = 100;

        REQUIRE(boost::json::value_to<std::string>(monster["id"]) == "gutter_rat");
        REQUIRE(boost::json::value_to<int>(monster["hp"]) == 3);
        REQUIRE(boost::json::value_to<int>(monster["atk"].as_array()[0]) == 1);
        REQUIRE(boost::json::value_to<int>(monster["atk"].as_array()[1]) == 3);
    }
}

TEST_CASE("JSON error handling", "[json]") {
    SECTION("Invalid JSON throws exception") {
        std::string invalid = "{invalid json}";
        REQUIRE_THROWS(boost::json::parse(invalid));
    }

    SECTION("Missing keys handled safely") {
        boost::json::object j;
        j["exists"] = true;
        REQUIRE(!j.contains("missing"));
    }
}