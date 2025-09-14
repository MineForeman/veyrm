/**
 * @file system_manager.h
 * @brief Manager for coordinating ECS systems
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <typeindex>
#include <unordered_map>

namespace ecs {

/**
 * @class SystemManager
 * @brief Manages and coordinates all ECS systems
 *
 * The SystemManager is responsible for:
 * - Registering and storing systems
 * - Managing system execution order
 * - Updating all systems each frame
 * - Providing access to specific systems
 */
class SystemManager {
public:
    SystemManager() = default;
    ~SystemManager() = default;

    // Disable copy
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    // Enable move
    SystemManager(SystemManager&&) = default;
    SystemManager& operator=(SystemManager&&) = default;

    /**
     * @brief Register a new system
     * @tparam T System type
     * @tparam Args Constructor argument types
     * @param args Arguments to forward to system constructor
     * @return Reference to the created system
     */
    template<typename T, typename... Args>
    T& registerSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *system;

        // Store by type for retrieval
        system_map[std::type_index(typeid(T))] = system.get();

        // Add to update list
        systems.push_back(std::move(system));

        // Re-sort by priority
        sortSystems();

        return ref;
    }

    /**
     * @brief Get a registered system by type
     * @tparam T System type to retrieve
     * @return Pointer to system or nullptr if not found
     */
    template<typename T>
    T* getSystem() {
        auto it = system_map.find(std::type_index(typeid(T)));
        if (it != system_map.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Get a registered system by type (const)
     * @tparam T System type to retrieve
     * @return Const pointer to system or nullptr if not found
     */
    template<typename T>
    const T* getSystem() const {
        auto it = system_map.find(std::type_index(typeid(T)));
        if (it != system_map.end()) {
            return dynamic_cast<const T*>(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Update all enabled systems
     * @param entities All entities to process
     * @param delta_time Time since last update (seconds)
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities,
                double delta_time) {
        for (auto& system : systems) {
            if (system->isEnabled()) {
                system->update(entities, delta_time);
            }
        }
    }

    /**
     * @brief Enable or disable a system
     * @tparam T System type
     * @param enabled New enabled state
     * @return true if system was found and updated
     */
    template<typename T>
    bool setSystemEnabled(bool enabled) {
        if (T* system = getSystem<T>()) {
            system->setEnabled(enabled);
            return true;
        }
        return false;
    }

    /**
     * @brief Remove a system
     * @tparam T System type to remove
     * @return true if system was removed
     */
    template<typename T>
    bool removeSystem() {
        auto type = std::type_index(typeid(T));

        // Remove from map
        auto map_it = system_map.find(type);
        if (map_it == system_map.end()) {
            return false;
        }

        ISystem* system_ptr = map_it->second;
        system_map.erase(map_it);

        // Remove from vector
        auto vec_it = std::find_if(systems.begin(), systems.end(),
            [system_ptr](const std::unique_ptr<ISystem>& sys) {
                return sys.get() == system_ptr;
            });

        if (vec_it != systems.end()) {
            systems.erase(vec_it);
            return true;
        }

        return false;
    }

    /**
     * @brief Clear all systems
     */
    void clear() {
        systems.clear();
        system_map.clear();
    }

    /**
     * @brief Get number of registered systems
     * @return System count
     */
    size_t getSystemCount() const {
        return systems.size();
    }

    /**
     * @brief Get all systems (for iteration)
     * @return Vector of all systems
     */
    const std::vector<std::unique_ptr<ISystem>>& getSystems() const {
        return systems;
    }

private:
    std::vector<std::unique_ptr<ISystem>> systems;  ///< All registered systems
    std::unordered_map<std::type_index, ISystem*> system_map; ///< Type lookup

    /**
     * @brief Sort systems by priority
     */
    void sortSystems() {
        std::stable_sort(systems.begin(), systems.end(),
            [](const std::unique_ptr<ISystem>& a,
               const std::unique_ptr<ISystem>& b) {
                return a->getPriority() < b->getPriority();
            });
    }
};

/**
 * @class World
 * @brief Container for entities and systems
 *
 * The World class combines entity storage with system management,
 * providing a complete ECS environment.
 */
class World {
public:
    World() = default;
    ~World() = default;

    // Entity management

    /**
     * @brief Create a new entity
     * @return Reference to created entity
     */
    Entity& createEntity() {
        entities.push_back(std::make_unique<Entity>());
        return *entities.back();
    }

    /**
     * @brief Add an existing entity to the world
     * @param entity Entity to add (ownership transferred)
     * @return Reference to added entity
     */
    Entity& addEntity(std::unique_ptr<Entity> entity) {
        entities.push_back(std::move(entity));
        return *entities.back();
    }

    /**
     * @brief Remove an entity by ID
     * @param id Entity ID to remove
     * @return true if entity was removed
     */
    bool removeEntity(EntityID id) {
        auto it = std::find_if(entities.begin(), entities.end(),
            [id](const std::unique_ptr<Entity>& e) {
                return e->getID() == id;
            });

        if (it != entities.end()) {
            entities.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Get entity by ID
     * @param id Entity ID to find
     * @return Pointer to entity or nullptr
     */
    Entity* getEntity(EntityID id) {
        for (auto& entity : entities) {
            if (entity->getID() == id) {
                return entity.get();
            }
        }
        return nullptr;
    }

    /**
     * @brief Clear all entities
     */
    void clearEntities() {
        entities.clear();
    }

    /**
     * @brief Get all entities
     * @return Vector of all entities
     */
    const std::vector<std::unique_ptr<Entity>>& getEntities() const {
        return entities;
    }

    /**
     * @brief Get number of entities
     * @return Entity count
     */
    size_t getEntityCount() const {
        return entities.size();
    }

    // System management (delegated)

    /**
     * @brief Register a system
     * @tparam T System type
     * @tparam Args Constructor arguments
     * @param args Arguments for system constructor
     * @return Reference to created system
     */
    template<typename T, typename... Args>
    T& registerSystem(Args&&... args) {
        return systems.registerSystem<T>(std::forward<Args>(args)...);
    }

    /**
     * @brief Get a system by type
     * @tparam T System type
     * @return Pointer to system or nullptr
     */
    template<typename T>
    T* getSystem() {
        return systems.getSystem<T>();
    }

    /**
     * @brief Update all systems
     * @param delta_time Time since last update
     */
    void update(double delta_time) {
        systems.update(entities, delta_time);
    }

    /**
     * @brief Get the system manager
     * @return Reference to system manager
     */
    SystemManager& getSystemManager() {
        return systems;
    }

private:
    std::vector<std::unique_ptr<Entity>> entities;  ///< All entities
    SystemManager systems;                          ///< System manager
};

} // namespace ecs