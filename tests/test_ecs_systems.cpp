#include <catch2/catch_test_macros.hpp>
#include "../include/ecs/system_manager.h"
#include "../include/ecs/movement_system.h"
#include "../include/ecs/render_system.h"
#include "../include/ecs/entity_factory.h"
#include "../include/map.h"

using namespace ecs;

TEST_CASE("SystemManager functionality", "[ecs][systems]") {
    SystemManager manager;

    SECTION("Register and retrieve systems") {
        Map test_map(20, 20);
        auto& movement = manager.registerSystem<MovementSystem>(&test_map);
        auto& render = manager.registerSystem<RenderSystem>(&test_map);

        REQUIRE(manager.getSystemCount() == 2);

        auto* retrieved_movement = manager.getSystem<MovementSystem>();
        REQUIRE(retrieved_movement != nullptr);
        REQUIRE(&movement == retrieved_movement);

        auto* retrieved_render = manager.getSystem<RenderSystem>();
        REQUIRE(retrieved_render != nullptr);
        REQUIRE(&render == retrieved_render);
    }

    SECTION("System execution order by priority") {
        Map test_map(20, 20);

        // MovementSystem has priority 10, RenderSystem has 90
        manager.registerSystem<RenderSystem>(&test_map);
        manager.registerSystem<MovementSystem>(&test_map);

        const auto& systems = manager.getSystems();
        REQUIRE(systems.size() == 2);

        // Movement should be first (lower priority value)
        REQUIRE(systems[0]->getName() == "MovementSystem");
        REQUIRE(systems[1]->getName() == "RenderSystem");
    }

    SECTION("Enable/disable systems") {
        Map test_map(20, 20);
        manager.registerSystem<MovementSystem>(&test_map);

        auto* movement = manager.getSystem<MovementSystem>();
        REQUIRE(movement->isEnabled() == true);

        manager.setSystemEnabled<MovementSystem>(false);
        REQUIRE(movement->isEnabled() == false);

        manager.setSystemEnabled<MovementSystem>(true);
        REQUIRE(movement->isEnabled() == true);
    }

    SECTION("Remove system") {
        Map test_map(20, 20);
        manager.registerSystem<MovementSystem>(&test_map);
        REQUIRE(manager.getSystemCount() == 1);

        bool removed = manager.removeSystem<MovementSystem>();
        REQUIRE(removed == true);
        REQUIRE(manager.getSystemCount() == 0);
        REQUIRE(manager.getSystem<MovementSystem>() == nullptr);
    }
}

TEST_CASE("MovementSystem functionality", "[ecs][systems][movement]") {
    Map test_map(20, 20);
    // Initialize map with all floor tiles
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 20; ++x) {
            test_map.setTile(x, y, TileType::FLOOR);
        }
    }

    SECTION("Basic movement") {
        World world;
        auto& movement = world.registerSystem<MovementSystem>(&test_map);

        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(5, 5);

        // Move entity
        bool result = movement.moveEntity(entity, 1, 0);
        REQUIRE(result == true);

        auto* pos = entity.getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 6);
        REQUIRE(pos->position.y == 5);
        REQUIRE(pos->previous_position.x == 5);
        REQUIRE(pos->previous_position.y == 5);
    }

    SECTION("Movement to absolute position") {
        World world;
        auto& movement = world.registerSystem<MovementSystem>(&test_map);

        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(5, 5);

        bool result = movement.moveEntityTo(entity, 10, 10);
        REQUIRE(result == true);

        auto* pos = entity.getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 10);
        REQUIRE(pos->position.y == 10);
    }

    SECTION("Blocked movement - map bounds") {
        World world;
        auto& movement = world.registerSystem<MovementSystem>(&test_map);

        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(0, 0);

        // Try to move out of bounds
        bool result = movement.moveEntity(entity, -1, 0);
        REQUIRE(result == false);

        auto* pos = entity.getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 0);  // Didn't move
        REQUIRE(pos->position.y == 0);
    }

    SECTION("Queued movement") {
        World world;
        auto& movement = world.registerSystem<MovementSystem>(&test_map);

        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(5, 5);

        // Queue movements
        movement.queueMove(entity.getID(), 1, 0);
        movement.queueMove(entity.getID(), 0, 1);

        // Process queue
        world.update(0.016);  // ~60 FPS

        auto* pos = entity.getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 6);
        REQUIRE(pos->position.y == 6);
    }

    SECTION("Entity collision detection") {
        World world;
        auto& movement = world.registerSystem<MovementSystem>(&test_map);

        auto& entity1 = world.createEntity();
        entity1.addComponent<PositionComponent>(5, 5);
        entity1.addComponent<CombatComponent>();  // Makes it blocking

        auto& entity2 = world.createEntity();
        entity2.addComponent<PositionComponent>(4, 5);

        // Check if position is blocked
        Entity* blocker = movement.getBlockingEntity(5, 5, world.getEntities());
        REQUIRE(blocker == &entity1);

        // Try to move entity2 into entity1's position
        bool valid = movement.isValidPosition(5, 5, world.getEntities(), &entity2);
        REQUIRE(valid == false);
    }
}

TEST_CASE("RenderSystem functionality", "[ecs][systems][render]") {
    Map test_map(20, 20);

    SECTION("Render data caching") {
        World world;
        auto& render = world.registerSystem<RenderSystem>(&test_map);

        auto& entity1 = world.createEntity();
        entity1.addComponent<PositionComponent>(5, 5);
        entity1.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);

        auto& entity2 = world.createEntity();
        entity2.addComponent<PositionComponent>(10, 10);
        entity2.addComponent<RenderableComponent>("g", ftxui::Color::Green);

        // Update render cache
        world.update(0.016);

        const auto& cache = render.getRenderData();
        REQUIRE(cache.size() == 2);
    }

    SECTION("Get entity at position") {
        World world;
        auto& render = world.registerSystem<RenderSystem>(&test_map);

        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(5, 5);
        entity.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);

        world.update(0.016);

        const RenderData* data = render.getEntityAt(5, 5);
        REQUIRE(data != nullptr);
        REQUIRE(data->glyph == "@");
        REQUIRE(data->position.x == 5);
        REQUIRE(data->position.y == 5);

        // No entity at different position
        data = render.getEntityAt(10, 10);
        REQUIRE(data == nullptr);
    }

    SECTION("Render priority") {
        World world;
        auto& render_sys = world.registerSystem<RenderSystem>(&test_map);

        auto& entity1 = world.createEntity();
        entity1.addComponent<PositionComponent>(5, 5);
        auto& render1 = entity1.addComponent<RenderableComponent>("1", ftxui::Color::White);
        render1.render_priority = 1;

        auto& entity2 = world.createEntity();
        entity2.addComponent<PositionComponent>(5, 5);  // Same position
        auto& render2 = entity2.addComponent<RenderableComponent>("2", ftxui::Color::White);
        render2.render_priority = 10;  // Higher priority

        world.update(0.016);

        // Higher priority entity should be on top
        const RenderData* data = render_sys.getEntityAt(5, 5);
        REQUIRE(data != nullptr);
        REQUIRE(data->glyph == "2");
    }

    SECTION("Visibility handling") {
        World world;
        auto& render_sys = world.registerSystem<RenderSystem>(&test_map);

        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(5, 5);
        auto& render = entity.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);

        // Set up FOV
        std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, false));
        fov[5][5] = true;  // Make position visible
        render_sys.setFOV(fov);

        REQUIRE(render_sys.isVisible(5, 5) == true);
        REQUIRE(render_sys.isVisible(10, 10) == false);

        // Test always_visible flag
        render.always_visible = true;
        world.update(0.016);

        auto grid = render_sys.renderToGrid(20, 20);
        REQUIRE(grid[5][5] == "@");  // Rendered despite FOV
    }

    SECTION("Render to grid") {
        World world;
        auto& render_sys = world.registerSystem<RenderSystem>(&test_map);

        auto& entity1 = world.createEntity();
        entity1.addComponent<PositionComponent>(2, 2);
        entity1.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);

        auto& entity2 = world.createEntity();
        entity2.addComponent<PositionComponent>(5, 5);
        entity2.addComponent<RenderableComponent>("g", ftxui::Color::Green);

        world.update(0.016);

        auto grid = render_sys.renderToGrid(10, 10);
        REQUIRE(grid[2][2] == "@");
        REQUIRE(grid[5][5] == "g");
        REQUIRE(grid[0][0] == " ");  // Empty space
    }
}

TEST_CASE("World entity and system management", "[ecs][world]") {
    SECTION("Create and manage entities") {
        World world;

        auto& entity1 = world.createEntity();
        auto& entity2 = world.createEntity();

        REQUIRE(world.getEntityCount() == 2);
        REQUIRE(entity1.getID() != entity2.getID());

        // Get entity by ID
        Entity* found = world.getEntity(entity1.getID());
        REQUIRE(found == &entity1);

        // Remove entity
        bool removed = world.removeEntity(entity1.getID());
        REQUIRE(removed == true);
        REQUIRE(world.getEntityCount() == 1);
    }

    SECTION("Add existing entities") {
        World world;

        auto player = PlayerFactory().create(10, 10);
        EntityID player_id = player->getID();

        world.addEntity(std::move(player));
        REQUIRE(world.getEntityCount() == 1);

        Entity* found = world.getEntity(player_id);
        REQUIRE(found != nullptr);
        REQUIRE(found->hasComponent<PositionComponent>());
    }

    SECTION("Integrated system update") {
        Map test_map(20, 20);
        // Initialize map with all floor tiles
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                test_map.setTile(x, y, TileType::FLOOR);
            }
        }

        World world;
        world.registerSystem<MovementSystem>(&test_map);
        world.registerSystem<RenderSystem>(&test_map);

        // Create entities with components
        auto& entity = world.createEntity();
        entity.addComponent<PositionComponent>(5, 5);
        entity.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);

        // Update all systems
        world.update(0.016);

        // Verify systems processed entities
        auto* render = world.getSystem<RenderSystem>();
        REQUIRE(render != nullptr);
        REQUIRE(render->getRenderData().size() == 1);
    }
}