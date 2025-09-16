/**
 * @file entity.cpp
 * @brief Implementation of component-based entity
 */

#include "../../include/ecs/entity.h"

namespace ecs {

// Initialize static member
EntityID Entity::next_id = 1;

Entity::Entity() : id(next_id++) {}

Entity::Entity(EntityID id) : id(id) {
    // Update next_id if necessary to avoid conflicts
    if (id >= next_id) {
        next_id = id + 1;
    }
}

void Entity::addComponent(std::unique_ptr<IComponent> component) {
    if (component) {
        components[component->getType()] = std::move(component);
    }
}

IComponent* Entity::getComponent(ComponentType type) {
    auto it = components.find(type);
    return (it != components.end()) ? it->second.get() : nullptr;
}

const IComponent* Entity::getComponent(ComponentType type) const {
    auto it = components.find(type);
    return (it != components.end()) ? it->second.get() : nullptr;
}

bool Entity::hasComponent(ComponentType type) const {
    return components.find(type) != components.end();
}

bool Entity::removeComponent(ComponentType type) {
    return components.erase(type) > 0;
}

std::unique_ptr<Entity> Entity::clone() const {
    auto cloned = std::make_unique<Entity>();

    for (const auto& [type, component] : components) {
        if (component) {
            cloned->addComponent(component->clone());
        }
    }

    return cloned;
}

} // namespace ecs