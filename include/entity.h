/**
 * @file entity.h
 * @brief Base entity class for all game objects
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <memory>
#include <variant>
#include "point.h"
#include "color_scheme.h"

// Forward declarations
class Map;
struct MonsterAIData;

/**
 * @enum EntityType
 * @brief Types of entities that can be created in the game
 */
enum class EntityType {
    PLAYER,   ///< Player character entity
    MONSTER,  ///< Monster/enemy entity
    ITEM      ///< Item entity (potions, equipment, etc.)
};

/**
 * @class Entity
 * @brief Base class for all game entities (player, monsters, items)
 *
 * The Entity class provides core functionality for all game objects that
 * exist in the world. It handles position, rendering, movement, combat
 * interfaces, and basic properties. Derived classes (Player, Monster, Item)
 * extend this functionality for specific entity behaviors.
 *
 * @see Player
 * @see Monster
 * @see Item
 * @see EntityManager
 */
class Entity {
public:
    /**
     * @brief Construct a new Entity
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     * @param glyph Character representation for rendering
     * @param color Display color
     * @param name Entity name for messages and UI
     */
    Entity(int x, int y, const std::string& glyph, ftxui::Color color, const std::string& name);

    /// Virtual destructor for proper cleanup of derived classes
    virtual ~Entity() = default;

    // Position
    int x, y;                    ///< Current position in the world
    int prev_x, prev_y;          ///< Previous position (for animation/undo)

    // Rendering
    std::string glyph;           ///< Character(s) displayed for this entity
    ftxui::Color color;          ///< Color used when rendering

    // Properties
    std::string name;            ///< Display name of the entity
    bool blocks_movement = false;///< Whether this entity blocks movement
    bool blocks_sight = false;   ///< Whether this entity blocks line of sight

    // Component flags
    bool is_player = false;      ///< True if this is the player entity
    bool is_monster = false;     ///< True if this is a monster entity
    bool is_item = false;        ///< True if this is an item entity
    bool is_blocking = false;    ///< True if this blocks other entities

    // Stats (used by Player and Monster)
    int hp = 1;                  ///< Current hit points
    int max_hp = 1;              ///< Maximum hit points
    
    /**
     * @brief Set visibility state
     * @param visible Whether the entity should be visible
     */
    void setVisible(bool visible) { is_visible = visible; }

    /**
     * @brief Check if entity is visible
     * @return true if visible, false otherwise
     */
    bool isVisible() const { return is_visible; }

    /**
     * @brief Get AI data pointer if entity has AI
     * @return Pointer to AI data or nullptr if not a monster
     * @note Type-safe access to AI-specific data
     */
    MonsterAIData* getAIData();
    const MonsterAIData* getAIData() const;

    /**
     * @brief Set AI data for this entity
     * @param data Unique pointer to AI data (ownership transferred)
     * @note Only valid for monster entities
     */
    void setAIData(std::shared_ptr<MonsterAIData> data);

    /**
     * @brief Check if entity has AI data
     * @return true if entity has AI data attached
     */
    bool hasAIData() const;

    // Legacy compatibility - deprecated
    [[deprecated("Use getAIData() for type-safe access")]]
    void* getUserData() const;

    [[deprecated("Use setAIData() for type-safe access")]]
    void setUserData(void* data);

    /**
     * @brief Get attack bonus for combat calculations
     * @return Attack bonus value
     * @note Override in derived classes for custom bonuses
     */
    virtual int getAttackBonus() const;

    /**
     * @brief Get defense bonus for combat calculations
     * @return Defense bonus value
     * @note Override in derived classes for custom bonuses
     */
    virtual int getDefenseBonus() const;

    /**
     * @brief Get base damage dealt in combat
     * @return Base damage value
     * @note Override in derived classes for custom damage
     */
    virtual int getBaseDamage() const;

    /**
     * @brief Get name for combat messages
     * @return String to use in combat messages
     */
    virtual std::string getCombatName() const;

    /**
     * @brief Called when this entity attacks another
     * @param target The entity being attacked
     * @note Override for special attack effects
     */
    virtual void onAttack([[maybe_unused]] Entity& target) {}

    /**
     * @brief Called when this entity is hit in combat
     * @param attacker The attacking entity
     * @param damage Amount of damage received
     * @note Override for special defense effects
     */
    virtual void onHit([[maybe_unused]] Entity& attacker, [[maybe_unused]] int damage) {}

    /**
     * @brief Called when an attack against this entity misses
     * @param attacker The attacking entity
     * @note Override for dodge/miss effects
     */
    virtual void onMiss([[maybe_unused]] Entity& attacker) {}

private:
    bool is_visible = true;

    // Type-safe AI data storage using std::variant
    // std::monostate represents "no data" state
    // Using shared_ptr to avoid incomplete type issues
    using AIDataStorage = std::variant<std::monostate, std::shared_ptr<MonsterAIData>>;
    AIDataStorage ai_data_storage;
    
public:
    
    /**
     * @brief Move entity by relative offset
     * @param dx X-axis movement delta
     * @param dy Y-axis movement delta
     */
    virtual void move(int dx, int dy);

    /**
     * @brief Move entity to absolute position
     * @param new_x Target X coordinate
     * @param new_y Target Y coordinate
     */
    virtual void moveTo(int new_x, int new_y);

    /**
     * @brief Update entity state
     * @param delta_time Time elapsed since last update (seconds)
     * @note Override for animated or time-based behaviors
     */
    virtual void update(double delta_time);

    /**
     * @brief Get current position as Point
     * @return Current position
     */
    Point getPosition() const { return Point(x, y); }

    /**
     * @brief Check if entity is at specific coordinates
     * @param check_x X coordinate to check
     * @param check_y Y coordinate to check
     * @return true if at the specified position
     */
    bool isAt(int check_x, int check_y) const { return x == check_x && y == check_y; }

    /**
     * @brief Calculate distance to another entity
     * @param other Target entity
     * @return Euclidean distance
     */
    double distanceTo(const Entity& other) const;

    /**
     * @brief Calculate distance to coordinates
     * @param target_x Target X coordinate
     * @param target_y Target Y coordinate
     * @return Euclidean distance
     */
    double distanceTo(int target_x, int target_y) const;

    /**
     * @brief Called when another entity interacts with this one
     * @param other The interacting entity
     * @note Override for pickup, dialogue, etc.
     */
    virtual void onInteract([[maybe_unused]] Entity& other) {}

    /**
     * @brief Called when entity dies
     * @note Override for death effects, drops, etc.
     */
    virtual void onDeath() {}

    /**
     * @brief Check if entity can move to position
     * @param map Current map
     * @param new_x Target X coordinate
     * @param new_y Target Y coordinate
     * @return true if movement is valid
     */
    virtual bool canMoveTo(const Map& map, int new_x, int new_y) const;

    /**
     * @brief Get entity type
     * @return EntityType enum value
     */
    virtual EntityType getType() const { return EntityType::ITEM; }

    /**
     * @brief Check if entity blocks movement
     * @return true if blocks movement
     */
    virtual bool isBlocking() const { return blocks_movement; }

    /**
     * @brief Check if entity can take actions
     * @return true if entity can act (player/monster)
     */
    virtual bool canAct() const { return false; }
    
protected:
    /**
     * @brief Save current position as previous position
     * @note Called before movement for undo/animation support
     */
    void savePreviousPosition();
};