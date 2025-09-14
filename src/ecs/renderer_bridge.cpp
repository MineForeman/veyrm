/**
 * @file renderer_bridge.cpp
 * @brief Implementation of Renderer ECS bridge
 */

#include "../../include/ecs/renderer_bridge.h"
#include <algorithm>

namespace ecs {

RendererBridge::RendererBridge(EntityManagerBridge* bridge)
    : entity_bridge(bridge) {
}

std::string RendererBridge::getEntityGlyph(std::shared_ptr<ecs::Entity> entity) const {
    if (!entity) return " ";

    auto* render = entity->getComponent<RenderableComponent>();
    if (render) {
        return render->glyph;
    }

    // Fall back to legacy entity if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy) {
            return legacy->glyph;
        }
    }

    return "?";
}

ftxui::Color RendererBridge::getEntityColor(std::shared_ptr<ecs::Entity> entity) const {
    if (!entity) return ftxui::Color::White;

    auto* render = entity->getComponent<RenderableComponent>();
    if (render) {
        return render->color;
    }

    // Fall back to legacy entity if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy) {
            return legacy->color;
        }
    }

    return ftxui::Color::White;
}

bool RendererBridge::isEntityVisible(std::shared_ptr<ecs::Entity> entity) const {
    if (!entity) return false;

    auto* render = entity->getComponent<RenderableComponent>();
    if (render) {
        return render->isVisible();
    }

    // Fall back to legacy entity if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy) {
            return legacy->isVisible();
        }
    }

    return false;
}

Point RendererBridge::getEntityPosition(std::shared_ptr<ecs::Entity> entity) const {
    if (!entity) return Point(-1, -1);

    auto* pos = entity->getComponent<PositionComponent>();
    if (pos) {
        return pos->position;
    }

    // Fall back to legacy entity if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy) {
            return legacy->getPosition();
        }
    }

    return Point(-1, -1);
}

ftxui::Element RendererBridge::renderEntitiesAt(int x, int y,
                                                const std::vector<std::vector<bool>>& fov) const {
    using namespace ftxui;

    if (!entity_bridge) {
        return text(" ");
    }

    // Get all entities at this position
    auto entities = entity_bridge->getEntitiesAtPosition(x, y);

    // Filter by visibility
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this, x, y, &fov](const std::shared_ptr<ecs::Entity>& entity) {
                auto* render = entity->getComponent<RenderableComponent>();
                if (!render || !render->isVisible()) return true;

                // Check FOV unless always visible
                if (!render->always_visible && !isInFOV(x, y, fov)) {
                    return true;
                }

                return false;
            }),
        entities.end()
    );

    if (entities.empty()) {
        return text(" ");
    }

    // Sort by render priority
    entities = sortByRenderPriority(entities);

    // Render the top entity
    return createEntityElement(entities.front());
}

std::shared_ptr<ecs::Entity> RendererBridge::getTopEntityAt(int x, int y,
                                                            const std::vector<std::vector<bool>>& fov) const {
    if (!entity_bridge) return nullptr;

    auto entities = entity_bridge->getEntitiesAtPosition(x, y);

    // Filter by visibility
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this, x, y, &fov](const std::shared_ptr<ecs::Entity>& entity) {
                auto* render = entity->getComponent<RenderableComponent>();
                if (!render || !render->isVisible()) return true;

                // Check FOV unless always visible
                if (!render->always_visible && !isInFOV(x, y, fov)) {
                    return true;
                }

                return false;
            }),
        entities.end()
    );

    if (entities.empty()) return nullptr;

    // Sort and return top
    entities = sortByRenderPriority(entities);
    return entities.front();
}

std::vector<std::shared_ptr<ecs::Entity>> RendererBridge::sortByRenderPriority(
    std::vector<std::shared_ptr<ecs::Entity>> entities) const {

    std::sort(entities.begin(), entities.end(),
        [](const std::shared_ptr<ecs::Entity>& a, const std::shared_ptr<ecs::Entity>& b) {
            auto* render_a = a->getComponent<RenderableComponent>();
            auto* render_b = b->getComponent<RenderableComponent>();

            if (!render_a) return false;
            if (!render_b) return true;

            return render_a->render_priority > render_b->render_priority;
        });

    return entities;
}

bool RendererBridge::doesEntityBlockSight(std::shared_ptr<ecs::Entity> entity) const {
    if (!entity) return false;

    auto* render = entity->getComponent<RenderableComponent>();
    if (render) {
        return render->blocks_sight;
    }

    // Fall back to legacy entity if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy) {
            return legacy->blocks_sight;
        }
    }

    return false;
}

void RendererBridge::setEntityVisibility(std::shared_ptr<ecs::Entity> entity, bool visible) {
    if (!entity) return;

    auto* render = entity->getComponent<RenderableComponent>();
    if (render) {
        render->setVisible(visible);
    }

    // Also update legacy entity if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy) {
            legacy->setVisible(visible);
        }
    }
}

void RendererBridge::updateEntitiesVisibility(const std::vector<std::vector<bool>>& fov) {
    if (!entity_bridge) return;

    auto entities = entity_bridge->getRenderableEntities();

    for (const auto& entity : entities) {
        auto* pos = entity->getComponent<PositionComponent>();
        auto* render = entity->getComponent<RenderableComponent>();

        if (!pos || !render) continue;

        // Always visible entities bypass FOV
        if (render->always_visible) {
            setEntityVisibility(entity, true);
        } else {
            bool visible = isInFOV(pos->position.x, pos->position.y, fov);
            setEntityVisibility(entity, visible);
        }
    }
}

ftxui::Element RendererBridge::createEntityElement(std::shared_ptr<ecs::Entity> entity) const {
    using namespace ftxui;

    if (!entity) return text(" ");

    std::string glyph = getEntityGlyph(entity);
    ftxui::Color color = getEntityColor(entity);

    return text(glyph) | ftxui::color(color);
}

std::vector<std::shared_ptr<ecs::Entity>> RendererBridge::getEntitiesInView(
    int view_x, int view_y, int view_width, int view_height,
    const std::vector<std::vector<bool>>& fov) const {

    std::vector<std::shared_ptr<ecs::Entity>> result;

    if (!entity_bridge) return result;

    auto entities = entity_bridge->getRenderableEntities();

    for (const auto& entity : entities) {
        auto* pos = entity->getComponent<PositionComponent>();
        auto* render = entity->getComponent<RenderableComponent>();

        if (!pos || !render || !render->isVisible()) continue;

        // Check if in view bounds
        int x = pos->position.x;
        int y = pos->position.y;

        if (x < view_x || x >= view_x + view_width ||
            y < view_y || y >= view_y + view_height) {
            continue;
        }

        // Check FOV unless always visible
        if (render->always_visible || isInFOV(x, y, fov)) {
            result.push_back(entity);
        }
    }

    return result;
}

bool RendererBridge::isInFOV(int x, int y, const std::vector<std::vector<bool>>& fov) const {
    if (fov.empty()) return true; // No FOV means everything visible

    if (y < 0 || y >= static_cast<int>(fov.size()) ||
        x < 0 || x >= static_cast<int>(fov[y].size())) {
        return false;
    }

    return fov[y][x];
}

} // namespace ecs