/**
 * @file component.h
 * @brief Base component interface for Entity Component System
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <typeindex>
#include <string>

namespace ecs {

/**
 * @enum ComponentType
 * @brief Types of components that can be attached to entities
 */
enum class ComponentType {
    POSITION,     ///< Position and movement data
    RENDERABLE,   ///< Visual representation data
    HEALTH,       ///< Health and damage tracking
    COMBAT,       ///< Combat stats and abilities
    AI,           ///< AI behavior data
    INVENTORY,    ///< Item storage
    PHYSICS,      ///< Physical properties (blocking, etc)
    STATS,        ///< RPG statistics
    ITEM_DATA,    ///< Item-specific properties
    CUSTOM        ///< User-defined components
};

/**
 * @class IComponent
 * @brief Base interface for all entity components
 *
 * Components are pure data containers with minimal logic.
 * Systems operate on components to implement behavior.
 */
class IComponent {
public:
    virtual ~IComponent() = default;

    /**
     * @brief Get the type of this component
     * @return ComponentType enum value
     */
    virtual ComponentType getType() const = 0;

    /**
     * @brief Get human-readable name for debugging
     * @return Component type name
     */
    virtual std::string getTypeName() const = 0;

    /**
     * @brief Clone this component
     * @return Unique pointer to cloned component
     */
    virtual std::unique_ptr<IComponent> clone() const = 0;
};

/**
 * @class Component
 * @brief CRTP base for concrete components
 * @tparam Derived The derived component class
 *
 * Provides default implementations using CRTP pattern
 */
template<typename Derived>
class Component : public IComponent {
public:
    std::unique_ptr<IComponent> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

} // namespace ecs