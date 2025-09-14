/**
 * @file monster_factory.h
 * @brief Factory for creating monsters from JSON templates
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <ftxui/screen/color.hpp>

class Monster;

class MonsterFactory {
public:
    static MonsterFactory& getInstance();
    
    // Load monster definitions from JSON
    bool loadFromFile(const std::string& filename);
    bool loadFromJson(const nlohmann::json& data);
    bool loadDefaultMonsters();  // Load from config data directory
    
    // Create monster instances
    std::unique_ptr<Monster> createMonster(
        const std::string& species, 
        int x, int y
    );
    
    // Query available monsters
    std::vector<std::string> getAvailableSpecies() const;
    bool hasSpecies(const std::string& species) const;
    
    // Get monster info
    std::string getMonsterName(const std::string& species) const;
    char getThreatLevel(const std::string& species) const;
    
    // Clear all loaded templates (mainly for testing)
    void clearTemplates() { templates.clear(); }
    
private:
    struct MonsterTemplate {
        std::string id;
        std::string name;
        std::string description;
        std::string glyph;
        ftxui::Color color;
        int hp;
        int attack;
        int defense;
        int speed;
        int xp_value;
        char threat_level;
        
        // Behavior flags
        bool aggressive = true;
        bool can_open_doors = false;
        bool can_see_invisible = false;
    };
    
    std::map<std::string, MonsterTemplate> templates;
    
    // Singleton
    MonsterFactory() = default;
    MonsterFactory(const MonsterFactory&) = delete;
    MonsterFactory& operator=(const MonsterFactory&) = delete;
    
    // Helper to parse color from string
    ftxui::Color parseColor(const std::string& color_str) const;
};