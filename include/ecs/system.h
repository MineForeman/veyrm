/**
 * @file system.h
 * @brief Base system interface for Entity Component System
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "entity.h"
#include <vector>
#include <memory>
#include <functional>

namespace ecs {

// Forward declarations
class SystemManager;

/**
 * @class ISystem
 * @brief Base interface for all ECS systems
 *
 * Systems implement game logic by operating on entities
 * with specific component combinations. Each system is
 * responsible for one aspect of the game (movement, rendering, etc).
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /**
     * @brief Update system logic
     * @param entities Collection of all entities to process
     * @param delta_time Time elapsed since last update (seconds)
     */
    virtual void update(const std::vector<std::unique_ptr<Entity>>& entities,
                       double delta_time) = 0;

    /**
     * @brief Get system name for debugging
     * @return Human-readable system name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Check if system should process an entity
     * @param entity Entity to check
     * @return true if entity has required components
     */
    virtual bool shouldProcess(const Entity& entity) const = 0;

    /**
     * @brief Get system priority (lower = earlier execution)
     * @return Priority value for ordering
     */
    virtual int getPriority() const { return 100; }

    /**
     * @brief Check if system is enabled
     * @return true if system should run
     */
    bool isEnabled() const { return enabled; }

    /**
     * @brief Enable or disable system
     * @param enable New enabled state
     */
    void setEnabled(bool enable) { enabled = enable; }

protected:
    bool enabled = true;  ///< Whether system is active
};

/**
 * @class System
 * @brief CRTP base for concrete systems
 * @tparam Derived The derived system class
 *
 * Provides common functionality for systems using CRTP
 */
template<typename Derived>
class System : public ISystem {
public:
    std::string getName() const override {
        return typeid(Derived).name();
    }
};

/**
 * @class EntityView
 * @brief Helper for filtering entities by components
 *
 * Provides efficient iteration over entities that have
 * specific component combinations.
 */
class EntityView {
public:
    /**
     * @brief Filter entities by component types
     * @tparam Components Component types required
     * @param entities All entities to filter
     * @return Vector of entities with all required components
     */
    template<typename... Components>
    static std::vector<Entity*> filter(
        const std::vector<std::unique_ptr<Entity>>& entities) {

        std::vector<Entity*> filtered;
        for (const auto& entity : entities) {
            if (hasComponents<Components...>(*entity)) {
                filtered.push_back(entity.get());
            }
        }
        return filtered;
    }

    /**
     * @brief Process entities with specific components
     * @tparam Components Component types required
     * @param entities All entities to process
     * @param processor Function to call for each matching entity
     */
    template<typename... Components>
    static void forEach(
        const std::vector<std::unique_ptr<Entity>>& entities,
        std::function<void(Entity&)> processor) {

        for (const auto& entity : entities) {
            if (hasComponents<Components...>(*entity)) {
                processor(*entity);
            }
        }
    }

private:
    template<typename First>
    static bool hasComponents(const Entity& entity) {
        return entity.hasComponent<First>();
    }

    template<typename First, typename Second, typename... Rest>
    static bool hasComponents(const Entity& entity) {
        return entity.hasComponent<First>() &&
               hasComponents<Second, Rest...>(entity);
    }
};

} // namespace ecs