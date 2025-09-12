#pragma once

#include <string>
#include <memory>
#include "point.h"
#include "color_scheme.h"

// Forward declarations
class Map;

// Entity types for creation
enum class EntityType {
    PLAYER,
    MONSTER,
    ITEM
};

// Base class for all game entities
class Entity {
public:
    // Constructor
    Entity(int x, int y, const std::string& glyph, ftxui::Color color, const std::string& name);
    virtual ~Entity() = default;
    
    // Position
    int x, y;
    
    // Previous position (for animation/undo)
    int prev_x, prev_y;
    
    // Rendering
    std::string glyph;
    ftxui::Color color;
    
    // Properties
    std::string name;
    bool blocks_movement = false;
    bool blocks_sight = false;
    
    // Component flags
    bool is_player = false;
    bool is_monster = false;
    bool is_item = false;
    
    // Visibility
    void setVisible(bool visible) { is_visible = visible; }
    bool isVisible() const { return is_visible; }
    
private:
    bool is_visible = true;
    
public:
    
    // Movement
    virtual void move(int dx, int dy);
    virtual void moveTo(int new_x, int new_y);
    
    // Updates
    virtual void update(double delta_time);
    
    // Utilities
    Point getPosition() const { return Point(x, y); }
    bool isAt(int check_x, int check_y) const { return x == check_x && y == check_y; }
    double distanceTo(const Entity& other) const;
    double distanceTo(int target_x, int target_y) const;
    
    // Interaction
    virtual void onInteract([[maybe_unused]] Entity& other) {}
    virtual void onDeath() {}
    virtual bool canMoveTo(const Map& map, int new_x, int new_y) const;
    
protected:
    // Save previous position before moving
    void savePreviousPosition();
};