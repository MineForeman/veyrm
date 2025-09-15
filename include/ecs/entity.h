/**
 * @file entity.h
 * @brief Component-based entity for ECS architecture
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <typeindex>
#include <vector>
#include <string>

namespace ecs {

using EntityID = uint64_t;

/**
 * @class Entity
 * @brief Component container for game objects
 *
 * An entity is just an ID with a collection of components.
 * All behavior is implemented by systems that operate on
 * components, not in the entity itself.
 */
class Entity {
public:
    /**
     * @brief Construct entity with unique ID
     */
    Entity();

    /**
     * @brief Construct entity with specific ID
     * @param id Entity identifier
     */
    explicit Entity(EntityID id);

    /**
     * @brief Get entity's unique identifier
     * @return Entity ID
     */
    EntityID getID() const { return id; }

    /**
     * @brief Add a component to this entity
     * @tparam T Component type
     * @tparam Args Constructor arguments
     * @param args Arguments forwarded to component constructor
     * @return Reference to the added component
     */
    template<typename T, typename... Args>
    T& addComponent(Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *component;
        components[component->getType()] = std::move(component);
        return ref;
    }

    /**
     * @brief Add an existing component to this entity
     * @param component Component to add (ownership transferred)
     */
    void addComponent(std::unique_ptr<IComponent> component);

    /**
     * @brief Get a component by type
     * @tparam T Component type to retrieve
     * @return Pointer to component or nullptr if not found
     */
    template<typename T>
    T* getComponent() {
        // Find by type
        for (auto& [type, comp] : components) {
            if (auto* typed = dynamic_cast<T*>(comp.get())) {
                return typed;
            }
        }
        return nullptr;
    }

    /**
     * @brief Get a component by type (const version)
     * @tparam T Component type to retrieve
     * @return Const pointer to component or nullptr if not found
     */
    template<typename T>
    const T* getComponent() const {
        for (const auto& [type, comp] : components) {
            if (auto* typed = dynamic_cast<const T*>(comp.get())) {
                return typed;
            }
        }
        return nullptr;
    }

    /**
     * @brief Get component by ComponentType enum
     * @param type Type of component to retrieve
     * @return Pointer to component or nullptr if not found
     */
    IComponent* getComponent(ComponentType type);
    const IComponent* getComponent(ComponentType type) const;

    /**
     * @brief Check if entity has a component
     * @tparam T Component type to check
     * @return true if entity has the component
     */
    template<typename T>
    bool hasComponent() const {
        return getComponent<T>() != nullptr;
    }

    /**
     * @brief Check if entity has a component by type
     * @param type Type of component to check
     * @return true if entity has the component
     */
    bool hasComponent(ComponentType type) const;

    /**
     * @brief Remove a component by type
     * @tparam T Component type to remove
     * @return true if component was removed
     */
    template<typename T>
    bool removeComponent() {
        for (auto it = components.begin(); it != components.end(); ++it) {
            if (dynamic_cast<T*>(it->second.get())) {
                components.erase(it);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Remove a component by ComponentType
     * @param type Type of component to remove
     * @return true if component was removed
     */
    bool removeComponent(ComponentType type);

    /**
     * @brief Get all components
     * @return Map of all components
     */
    const std::unordered_map<ComponentType, std::unique_ptr<IComponent>>&
    getComponents() const { return components; }

    /**
     * @brief Clear all components from entity
     */
    void clearComponents() { components.clear(); }

    /**
     * @brief Clone this entity (deep copy of all components)
     * @return New entity with cloned components
     */
    std::unique_ptr<Entity> clone() const;

    /**
     * @brief Check if entity is valid (has any components)
     * @return true if entity has at least one component
     */
    bool isValid() const { return !components.empty(); }

    /**
     * @brief Add a tag to this entity
     * @param tag Tag string
     */
    void addTag(const std::string& tag) { tags.insert(tag); }

    /**
     * @brief Remove a tag from this entity
     * @param tag Tag string
     */
    void removeTag(const std::string& tag) { tags.erase(tag); }

    /**
     * @brief Check if entity has a tag
     * @param tag Tag to check
     * @return true if entity has the tag
     */
    bool hasTag(const std::string& tag) const {
        return tags.find(tag) != tags.end();
    }

    /**
     * @brief Get all tags
     * @return Set of all tags
     */
    const std::unordered_set<std::string>& getTags() const { return tags; }

private:
    static EntityID next_id;  ///< Next available entity ID
    EntityID id;              ///< This entity's unique ID

    std::unordered_map<ComponentType, std::unique_ptr<IComponent>> components;
    std::unordered_set<std::string> tags;  ///< Entity tags for categorization
};

} // namespace ecs