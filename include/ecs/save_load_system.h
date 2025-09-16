/**
 * @file save_load_system.h
 * @brief System for saving and loading game state
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include "logger_interface.h"
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace ecs {

/**
 * @class SaveLoadSystem
 * @brief Manages game state persistence
 */
class SaveLoadSystem : public System<SaveLoadSystem> {
public:
    static constexpr int SAVE_VERSION = 1;

    SaveLoadSystem(const std::string& save_directory = "saves",
                   ILogger* logger = nullptr);
    ~SaveLoadSystem() = default;

    void update(const std::vector<std::unique_ptr<Entity>>&, double) override {
        // SaveLoad system is event-driven
    }

    /**
     * @brief Save entity to JSON
     * @param entity Entity to save
     * @param json Output JSON object
     * @return true if successful
     */
    bool saveEntity(const Entity& entity, nlohmann::json& json) const;

    /**
     * @brief Load entity from JSON
     * @param json Input JSON object
     * @return Entity or nullptr on error
     */
    std::unique_ptr<Entity> loadEntity(const nlohmann::json& json) const;

    /**
     * @brief Save world state
     * @param entities All entities to save
     * @param filename Save file name
     * @return true if successful
     */
    bool saveWorld(const std::vector<std::unique_ptr<Entity>>& entities,
                   const std::string& filename) const;

    /**
     * @brief Load world state
     * @param filename Save file name
     * @return Loaded entities
     */
    std::vector<std::unique_ptr<Entity>> loadWorld(const std::string& filename) const;

    /**
     * @brief Quick save
     * @param entities Entities to save
     * @return true if successful
     */
    bool quickSave(const std::vector<std::unique_ptr<Entity>>& entities) const;

    /**
     * @brief Quick load
     * @return Loaded entities
     */
    std::vector<std::unique_ptr<Entity>> quickLoad() const;

    /**
     * @brief Get list of save files
     * @return Vector of save file names
     */
    std::vector<std::string> getSaveFiles() const;

    /**
     * @brief Delete save file
     * @param filename File to delete
     * @return true if successful
     */
    bool deleteSave(const std::string& filename) const;

    int getPriority() const override { return 100; }  // Low priority

    bool shouldProcess(const Entity&) const override { return false; }

private:
    std::string save_dir;
    ILogger* logger;
};

} // namespace ecs