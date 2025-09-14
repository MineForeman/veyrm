/**
 * @file render_system.cpp
 * @brief Implementation of render system
 */

#include "../../include/ecs/render_system.h"
#include <algorithm>

namespace ecs {

void RenderSystem::update(const std::vector<std::unique_ptr<Entity>>& entities,
                          [[maybe_unused]] double delta_time) {
    // Clear and rebuild render cache
    render_cache.clear();

    for (const auto& entity : entities) {
        if (!shouldProcess(*entity)) continue;

        auto* pos = entity->getComponent<PositionComponent>();
        auto* render = entity->getComponent<RenderableComponent>();

        if (!render->isVisible()) continue;

        RenderData data;
        data.position = pos->getPosition();
        data.glyph = render->glyph;
        data.color = render->color;
        data.priority = render->render_priority;
        data.always_visible = render->always_visible;

        render_cache.push_back(data);
    }

    // Sort by position for efficient lookup
    sortRenderCache();
}

void RenderSystem::sortRenderCache() {
    std::sort(render_cache.begin(), render_cache.end(),
        [](const RenderData& a, const RenderData& b) {
            // Sort by position first, then priority
            if (a.position.y != b.position.y) {
                return a.position.y < b.position.y;
            }
            if (a.position.x != b.position.x) {
                return a.position.x < b.position.x;
            }
            return a.priority > b.priority;  // Higher priority on top
        });
}

const RenderData* RenderSystem::getEntityAt(int x, int y) const {
    const RenderData* result = nullptr;
    int highest_priority = -1;

    for (const auto& data : render_cache) {
        if (data.position.x == x && data.position.y == y) {
            if (data.priority > highest_priority) {
                highest_priority = data.priority;
                result = &data;
            }
        }
    }

    return result;
}

std::vector<const RenderData*> RenderSystem::getEntitiesAt(int x, int y) const {
    std::vector<const RenderData*> result;

    for (const auto& data : render_cache) {
        if (data.position.x == x && data.position.y == y) {
            result.push_back(&data);
        }
    }

    // Sort by priority (highest first)
    std::sort(result.begin(), result.end(),
        [](const RenderData* a, const RenderData* b) {
            return a->priority > b->priority;
        });

    return result;
}

std::vector<std::vector<std::string>> RenderSystem::renderToGrid(
    int width, int height, int view_x, int view_y) const {

    // Initialize grid with empty spaces
    std::vector<std::vector<std::string>> grid(height,
        std::vector<std::string>(width, " "));

    // Render entities
    for (const auto& data : render_cache) {
        int screen_x = data.position.x - view_x;
        int screen_y = data.position.y - view_y;

        // Check if in view bounds
        if (screen_x < 0 || screen_x >= width ||
            screen_y < 0 || screen_y >= height) {
            continue;
        }

        // Check visibility
        if (!data.always_visible && !isVisible(data.position.x, data.position.y)) {
            continue;
        }

        // Render the entity
        grid[screen_y][screen_x] = data.glyph;
    }

    return grid;
}

ftxui::Element RenderSystem::renderEntityElement(int x, int y) const {
    using namespace ftxui;

    const RenderData* data = getEntityAt(x, y);
    if (!data) {
        return text(" ");
    }

    // Check visibility
    if (!data->always_visible && !isVisible(x, y)) {
        return text(" ");
    }

    // Create colored text element
    return text(data->glyph) | color(data->color);
}

bool RenderSystem::isVisible(int x, int y) const {
    // If no FOV is set, everything is visible
    if (field_of_view.empty()) {
        return true;
    }

    // Check bounds
    if (y < 0 || y >= static_cast<int>(field_of_view.size()) ||
        x < 0 || x >= static_cast<int>(field_of_view[y].size())) {
        return false;
    }

    return field_of_view[y][x];
}

} // namespace ecs