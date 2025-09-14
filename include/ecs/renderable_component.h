/**
 * @file renderable_component.h
 * @brief Visual representation component for entities
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include "../color_scheme.h"
#include <string>

namespace ecs {

/**
 * @class RenderableComponent
 * @brief Manages visual representation of entities
 *
 * This component stores all data needed to render an entity
 * including its glyph (character), color, and visibility state.
 */
class RenderableComponent : public Component<RenderableComponent> {
public:
    /**
     * @brief Construct renderable component
     * @param glyph Character(s) to display
     * @param color Display color
     * @param visible Initial visibility state
     */
    RenderableComponent(const std::string& glyph = "?",
                        ftxui::Color color = ftxui::Color::White,
                        bool visible = true)
        : glyph(glyph), color(color), is_visible(visible) {}

    ComponentType getType() const override {
        return ComponentType::RENDERABLE;
    }

    std::string getTypeName() const override {
        return "RenderableComponent";
    }

    /**
     * @brief Set visibility state
     * @param visible Whether entity should be rendered
     */
    void setVisible(bool visible) { is_visible = visible; }

    /**
     * @brief Check if entity is visible
     * @return true if entity should be rendered
     */
    bool isVisible() const { return is_visible; }

    /**
     * @brief Change the display glyph
     * @param new_glyph New character(s) to display
     */
    void setGlyph(const std::string& new_glyph) { glyph = new_glyph; }

    /**
     * @brief Change the display color
     * @param new_color New color for rendering
     */
    void setColor(ftxui::Color new_color) { color = new_color; }

public:
    // Public data for easy access
    std::string glyph;      ///< Character(s) displayed for this entity
    ftxui::Color color;     ///< Color used when rendering
    bool is_visible;        ///< Whether entity should be rendered

    // Optional rendering hints
    bool always_visible = false;  ///< Ignore FOV (for important items)
    int render_priority = 0;       ///< Higher priority renders on top
    bool blocks_sight = false;     ///< Whether this blocks line of sight
};

} // namespace ecs