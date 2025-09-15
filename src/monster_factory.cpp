#include "monster_factory.h"
#include "monster.h"
#include "config.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace ftxui;

MonsterFactory& MonsterFactory::getInstance() {
    static MonsterFactory instance;
    return instance;
}

bool MonsterFactory::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open monster file: " << filename << std::endl;
            return false;
        }
        
        json data;
        file >> data;
        return loadFromJson(data);
    } catch (const std::exception& e) {
        std::cerr << "Error loading monster file: " << e.what() << std::endl;
        return false;
    }
}

bool MonsterFactory::loadDefaultMonsters() {
    std::string monster_file = Config::getInstance().getDataFilePath("monsters.json");
    return loadFromFile(monster_file);
}

bool MonsterFactory::loadFromJson(const json& data) {
    try {
        if (!data.contains("monsters")) {
            std::cerr << "JSON missing 'monsters' array" << std::endl;
            return false;
        }
        
        for (const auto& monster_data : data["monsters"]) {
            MonsterTemplate tmpl;

            // Required fields
            tmpl.id = monster_data["id"];
            tmpl.name = monster_data["name"];

            // Handle both old and new JSON formats
            if (monster_data.contains("components")) {
                // New component-based format
                const auto& components = monster_data["components"];

                // Get glyph
                std::string glyph_str = monster_data["glyph"];
                tmpl.glyph = glyph_str.empty() ? '?' : glyph_str[0];

                // Health component
                if (components.contains("health")) {
                    tmpl.hp = components["health"]["max_hp"];
                } else {
                    tmpl.hp = 10; // default
                }

                // Combat component
                if (components.contains("combat")) {
                    const auto& combat = components["combat"];
                    tmpl.attack = combat.value("attack_bonus", 0);
                    tmpl.defense = combat.value("defense_bonus", 0);
                } else {
                    tmpl.attack = 0;
                    tmpl.defense = 0;
                }

                // Stats component for speed
                if (components.contains("stats")) {
                    int dexterity = components["stats"].value("dexterity", 10);
                    tmpl.speed = 100 + (dexterity - 10) * 5;
                } else {
                    tmpl.speed = 100;
                }

                tmpl.xp_value = monster_data.value("xp_value", 10);
            } else {
                // Old direct format
                std::string glyph_str = monster_data["glyph"];
                tmpl.glyph = glyph_str.empty() ? '?' : glyph_str[0];
                tmpl.hp = monster_data["hp"];
                tmpl.attack = monster_data["attack"];
                tmpl.defense = monster_data["defense"];
                tmpl.speed = monster_data["speed"];
                tmpl.xp_value = monster_data["xp_value"];
            }
            
            // Optional fields
            if (monster_data.contains("description")) {
                tmpl.description = monster_data["description"];
            } else {
                tmpl.description = tmpl.name;
            }
            
            if (monster_data.contains("color")) {
                tmpl.color = parseColor(monster_data["color"]);
            } else {
                tmpl.color = Color::Red;
            }
            
            if (monster_data.contains("threat_level")) {
                std::string threat_str = monster_data["threat_level"];
                tmpl.threat_level = threat_str.empty() ? 'a' : threat_str[0];
            } else {
                tmpl.threat_level = 'a';
            }
            
            // Behavior flags
            if (monster_data.contains("flags")) {
                const auto& flags = monster_data["flags"];
                if (flags.contains("aggressive")) {
                    tmpl.aggressive = flags["aggressive"];
                }
                if (flags.contains("can_open_doors")) {
                    tmpl.can_open_doors = flags["can_open_doors"];
                }
                if (flags.contains("can_see_invisible")) {
                    tmpl.can_see_invisible = flags["can_see_invisible"];
                }
            }
            
            // Store template
            templates[tmpl.id] = tmpl;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing monster JSON: " << e.what() << std::endl;
        return false;
    }
}

std::unique_ptr<Monster> MonsterFactory::createMonster(
    const std::string& species, 
    int x, int y) {
    
    auto it = templates.find(species);
    if (it == templates.end()) {
        std::cerr << "Unknown monster species: " << species << std::endl;
        return nullptr;
    }
    
    const MonsterTemplate& tmpl = it->second;
    
    auto monster = std::make_unique<Monster>(x, y, species);
    
    // Set stats
    monster->setStats(tmpl.hp, tmpl.hp, tmpl.attack, tmpl.defense, 
                      tmpl.speed, tmpl.xp_value);
    
    // Set metadata
    monster->setMetadata(tmpl.name, tmpl.description, tmpl.glyph, 
                        tmpl.color, tmpl.threat_level);
    
    // Set behavior flags
    monster->setFlags(tmpl.aggressive, tmpl.can_open_doors, 
                      tmpl.can_see_invisible);
    
    return monster;
}

std::vector<std::string> MonsterFactory::getAvailableSpecies() const {
    std::vector<std::string> species;
    for (const auto& [id, tmpl] : templates) {
        species.push_back(id);
    }
    return species;
}

bool MonsterFactory::hasSpecies(const std::string& species) const {
    return templates.find(species) != templates.end();
}

std::string MonsterFactory::getMonsterName(const std::string& species) const {
    auto it = templates.find(species);
    if (it != templates.end()) {
        return it->second.name;
    }
    return "Unknown Monster";
}

char MonsterFactory::getThreatLevel(const std::string& species) const {
    auto it = templates.find(species);
    if (it != templates.end()) {
        return it->second.threat_level;
    }
    return '?';
}

Color MonsterFactory::parseColor(const std::string& color_str) const {
    // Convert color string to FTXUI Color
    if (color_str == "red") return Color::Red;
    if (color_str == "green") return Color::Green;
    if (color_str == "blue") return Color::Blue;
    if (color_str == "yellow") return Color::Yellow;
    if (color_str == "magenta") return Color::Magenta;
    if (color_str == "cyan") return Color::Cyan;
    if (color_str == "white") return Color::White;
    if (color_str == "black") return Color::Black;
    if (color_str == "brown") return Color::RGB(139, 69, 19);  // Brown RGB
    if (color_str == "gray" || color_str == "grey") return Color::GrayDark;
    if (color_str == "dark_gray" || color_str == "dark_grey") return Color::GrayDark;
    if (color_str == "light_gray" || color_str == "light_grey") return Color::GrayLight;
    
    // Default to white if unknown
    return Color::White;
}