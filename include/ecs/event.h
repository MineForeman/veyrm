/**
 * @file event.h
 * @brief Simplified ECS event system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include "entity.h"

namespace ecs {

/**
 * @enum EventType
 * @brief Types of events in the ECS system
 */
enum class EventType {
    DAMAGE,
    DEATH,
    PICKUP,
    DROP,
    USE_ITEM,
    MOVE,
    ATTACK,
    SPAWN,
    DESPAWN,
    INTERACTION,
    STATE_CHANGE,
    CUSTOM
};

/**
 * @struct BaseEvent
 * @brief Base event data
 */
struct BaseEvent {
    EventType type;
    EntityID source_id = 0;
    EntityID target_id = 0;
    int value1 = 0;
    int value2 = 0;
    std::string text;
    double timestamp = 0.0;

    BaseEvent(EventType t, EntityID src = 0, EntityID tgt = 0)
        : type(t), source_id(src), target_id(tgt) {}
};

// Event handler function type
using EventHandler = std::function<void(const BaseEvent&)>;

/**
 * @class EventSystem
 * @brief Simple event system for ECS
 */
class EventSystem {
public:
    static EventSystem& getInstance() {
        static EventSystem instance;
        return instance;
    }

    /**
     * @brief Subscribe to an event type
     * @param type Event type
     * @param handler Handler function
     */
    void subscribe(EventType type, EventHandler handler) {
        handlers[static_cast<int>(type)].push_back(handler);
    }

    /**
     * @brief Emit an event
     * @param event Event to emit
     */
    void emit(const BaseEvent& event) {
        event_queue.push_back(event);
    }

    /**
     * @brief Process all queued events
     */
    void update() {
        auto queue = event_queue;
        event_queue.clear();

        for (const auto& event : queue) {
            auto& type_handlers = handlers[static_cast<int>(event.type)];
            for (const auto& handler : type_handlers) {
                handler(event);
            }
        }
    }

private:
    EventSystem() = default;
    std::vector<std::vector<EventHandler>> handlers{static_cast<size_t>(EventType::CUSTOM) + 1};
    std::vector<BaseEvent> event_queue;
};

// Convenience functions for creating events
inline BaseEvent DamageEvent(EntityID source, EntityID target, int damage, const std::string& text = "") {
    BaseEvent e(EventType::DAMAGE, source, target);
    e.value1 = damage;
    e.text = text;
    return e;
}

inline BaseEvent DeathEvent(EntityID entity, EntityID killer = 0, const std::string& text = "") {
    BaseEvent e(EventType::DEATH, entity, killer);
    e.text = text;
    return e;
}

inline BaseEvent AttackEvent(EntityID attacker, EntityID target, const std::string& text = "") {
    BaseEvent e(EventType::ATTACK, attacker, target);
    e.text = text;
    return e;
}

inline BaseEvent PickupEvent(EntityID item, EntityID picker) {
    BaseEvent e(EventType::PICKUP, picker, item);
    return e;
}

inline BaseEvent DropEvent(EntityID dropper, EntityID item, int x, int y) {
    BaseEvent e(EventType::DROP, dropper, item);
    e.value1 = x;
    e.value2 = y;
    return e;
}

inline BaseEvent UseItemEvent(EntityID user, EntityID item, EntityID target = 0) {
    BaseEvent e(EventType::USE_ITEM, user, target ? target : user);
    e.value1 = item;
    return e;
}

} // namespace ecs