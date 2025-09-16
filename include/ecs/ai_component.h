/**
 * @file ai_component.h
 * @brief AI component type aliases for compatibility
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

// Include the actual AI component from ai_system.h
#include "ai_system.h"

namespace ecs {

// Type aliases for compatibility
using AIType = AIBehavior;

// Map old enum values to new ones
constexpr AIBehavior AIType_NONE = AIBehavior::PASSIVE;
constexpr AIBehavior AIType_PASSIVE = AIBehavior::PASSIVE;
constexpr AIBehavior AIType_AGGRESSIVE = AIBehavior::AGGRESSIVE;
constexpr AIBehavior AIType_DEFENSIVE = AIBehavior::DEFENSIVE;

// Additional AI state not in original
enum class AIState {
    IDLE,           ///< No current activity
    WANDERING,      ///< Random movement
    PURSUING,       ///< Chasing target
    ATTACKING,      ///< In combat
    FLEEING,        ///< Running away
    PATROLLING,     ///< Following patrol
    SEARCHING,      ///< Looking for target
    STUNNED,        ///< Temporarily disabled
    DEAD            ///< No longer active
};

} // namespace ecs