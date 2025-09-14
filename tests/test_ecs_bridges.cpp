#include <catch2/catch_test_macros.hpp>
#include "../include/ecs/entity_manager_bridge.h"
#include "../include/ecs/combat_system_bridge.h"
#include "../include/ecs/renderer_bridge.h"
#include "../include/ecs/entity_factory.h"
#include "../include/entity_manager.h"
#include "../include/combat_system.h"
#include "../include/player.h"
#include "../include/monster.h"

using namespace ecs;

TEST_CASE("EntityManagerBridge functionality", "[ecs][bridges]") {
    EntityManager legacy_manager;
    EntityManagerBridge bridge(&legacy_manager);

    SECTION("Sync legacy and ECS entities") {
        // Create legacy player
        auto legacy_player = legacy_manager.createPlayer(10, 10);

        // Create ECS entity from player
        std::shared_ptr<ecs::Entity> ecs_player(EntityAdapter::fromPlayer(*legacy_player).release());

        // Sync them
        bridge.syncEntity(legacy_player, ecs_player);

        // Verify mapping
        REQUIRE(bridge.getECSEntity(legacy_player) == ecs_player);
        REQUIRE(bridge.getLegacyEntity(ecs_player) == legacy_player);
    }

    SECTION("Query entities by position") {
        auto legacy_entity = legacy_manager.createPlayer(5, 5);
        std::shared_ptr<ecs::Entity> ecs_entity(EntityAdapter::fromPlayer(*std::dynamic_pointer_cast<Player>(legacy_entity)).release());
        bridge.syncEntity(legacy_entity, ecs_entity);

        auto entities_at_pos = bridge.getEntitiesAtPosition(5, 5);
        REQUIRE(entities_at_pos.size() == 1);
        REQUIRE(entities_at_pos[0] == ecs_entity);

        auto entities_at_wrong_pos = bridge.getEntitiesAtPosition(10, 10);
        REQUIRE(entities_at_wrong_pos.empty());
    }

    SECTION("Get combat entities") {
        // Create player with combat component
        auto legacy_player = legacy_manager.createPlayer(0, 0);
        std::shared_ptr<ecs::Entity> ecs_player(PlayerFactory().create(0, 0).release());
        bridge.syncEntity(legacy_player, ecs_player);

        auto combat_entities = bridge.getCombatEntities();
        REQUIRE(combat_entities.size() == 1);
        REQUIRE(combat_entities[0]->hasComponent<CombatComponent>());
    }

    SECTION("Get renderable entities") {
        auto legacy_player = legacy_manager.createPlayer(0, 0);
        std::shared_ptr<ecs::Entity> ecs_player(PlayerFactory().create(0, 0).release());
        bridge.syncEntity(legacy_player, ecs_player);

        auto renderable_entities = bridge.getRenderableEntities();
        REQUIRE(renderable_entities.size() == 1);
        REQUIRE(renderable_entities[0]->hasComponent<RenderableComponent>());
    }

    SECTION("Check position blocking") {
        auto legacy_player = legacy_manager.createPlayer(3, 3);
        std::shared_ptr<ecs::Entity> ecs_player(PlayerFactory().create(3, 3).release());
        bridge.syncEntity(legacy_player, ecs_player);

        REQUIRE(bridge.isPositionBlockedByCombatEntity(3, 3) == true);
        REQUIRE(bridge.isPositionBlockedByCombatEntity(4, 4) == false);
    }

    SECTION("Update positions from components") {
        auto legacy_player = legacy_manager.createPlayer(0, 0);
        std::shared_ptr<ecs::Entity> ecs_player(PlayerFactory().create(0, 0).release());
        bridge.syncEntity(legacy_player, ecs_player);

        // Move ECS entity
        auto* pos = ecs_player->getComponent<PositionComponent>();
        pos->moveTo(5, 5);

        // Update legacy from component
        bridge.updatePositionsFromComponents();

        REQUIRE(legacy_player->getPosition().x == 5);
        REQUIRE(legacy_player->getPosition().y == 5);
    }

    SECTION("Update health from components") {
        auto legacy_player = legacy_manager.createPlayer(0, 0);
        std::shared_ptr<ecs::Entity> ecs_player(PlayerFactory().create(0, 0).release());
        bridge.syncEntity(legacy_player, ecs_player);

        // Damage ECS entity
        auto* health = ecs_player->getComponent<HealthComponent>();
        health->takeDamage(30);

        // Update legacy from component
        bridge.updateHealthFromComponents();

        REQUIRE(legacy_player->hp == 70);
    }

    SECTION("Remove dead entities") {
        // Create a simple entity for testing since legacy and ECS factories have different monsters
        // We'll use createEntity which properly adds to the manager
        auto legacy_entity = legacy_manager.createEntity(EntityType::MONSTER, 0, 0);
        legacy_entity->hp = 10;
        legacy_entity->max_hp = 10;

        std::shared_ptr<ecs::Entity> ecs_monster(MonsterFactoryECS().create("goblin", 0, 0).release());
        bridge.syncEntity(legacy_entity, ecs_monster);

        // Kill the monster
        auto* health = ecs_monster->getComponent<HealthComponent>();
        health->takeDamage(1000);

        size_t count_before = legacy_manager.getEntityCount();
        REQUIRE(count_before > 0); // At least the monster exists

        bridge.removeDeadEntitiesFromComponents();
        size_t count_after = legacy_manager.getEntityCount();

        REQUIRE(count_after < count_before);
        REQUIRE(bridge.getECSEntity(legacy_entity) == nullptr);
    }
}

TEST_CASE("CombatSystemBridge functionality", "[ecs][bridges][combat]") {
    EntityManager legacy_manager;
    EntityManagerBridge entity_bridge(&legacy_manager);
    CombatSystem legacy_combat;
    CombatSystemBridge combat_bridge(&legacy_combat, &entity_bridge);

    SECTION("Process component attack") {
        // Create attacker and defender
        std::shared_ptr<ecs::Entity> attacker(PlayerFactory().create(0, 0).release());
        attacker->getComponent<CombatComponent>()->combat_name = "Attacker";
        std::shared_ptr<ecs::Entity> defender(MonsterFactoryECS().create("goblin", 1, 0).release());

        auto result = combat_bridge.processComponentAttack(attacker, defender);

        // Attack should have been processed
        REQUIRE(!result.attack_message.empty());

        if (result.hit) {
            REQUIRE(result.damage > 0);

            // Defender should have taken damage
            auto* health = defender->getComponent<HealthComponent>();
            REQUIRE(health->getHealth() < health->getMaxHealth());
        }
    }

    SECTION("Calculate component damage") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(0, 0).release());
        auto* combat = entity->getComponent<CombatComponent>();

        int damage = combat_bridge.calculateComponentDamage(*combat);
        REQUIRE(damage >= combat->min_damage);
        REQUIRE(damage <= combat->max_damage);
    }

    SECTION("Apply component damage") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(0, 0).release());
        auto* health = entity->getComponent<HealthComponent>();
        int initial_hp = health->getHealth();

        int damage_dealt = combat_bridge.applyComponentDamage(entity, 25);
        REQUIRE(damage_dealt == 25);
        REQUIRE(health->getHealth() == initial_hp - 25);
    }

    SECTION("Check attack/defend capability") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(0, 0).release());

        REQUIRE(combat_bridge.canAttack(entity) == true);
        REQUIRE(combat_bridge.canDefend(entity) == true);

        // Stun the entity
        auto* combat = entity->getComponent<CombatComponent>();
        combat->is_stunned = true;

        REQUIRE(combat_bridge.canAttack(entity) == false);
        REQUIRE(combat_bridge.canDefend(entity) == false);
    }

    SECTION("Get defense and attack values") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(0, 0).release());
        auto* combat = entity->getComponent<CombatComponent>();

        int defense = combat_bridge.getComponentDefenseValue(*combat);
        REQUIRE(defense == 10 + combat->getTotalDefenseBonus());

        int attack = combat_bridge.getComponentAttackBonus(*combat);
        REQUIRE(attack == combat->getTotalAttackBonus());
    }
}

TEST_CASE("RendererBridge functionality", "[ecs][bridges][render]") {
    EntityManager legacy_manager;
    EntityManagerBridge entity_bridge(&legacy_manager);
    RendererBridge render_bridge(&entity_bridge);

    SECTION("Get entity glyph and color") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(0, 0).release());

        std::string glyph = render_bridge.getEntityGlyph(entity);
        REQUIRE(glyph == "@");

        ftxui::Color color = render_bridge.getEntityColor(entity);
        REQUIRE(color == ftxui::Color::Yellow);
    }

    SECTION("Check entity visibility") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(0, 0).release());

        REQUIRE(render_bridge.isEntityVisible(entity) == true);

        render_bridge.setEntityVisibility(entity, false);
        REQUIRE(render_bridge.isEntityVisible(entity) == false);
    }

    SECTION("Get entity position") {
        std::shared_ptr<ecs::Entity> entity(PlayerFactory().create(5, 10).release());

        Point pos = render_bridge.getEntityPosition(entity);
        REQUIRE(pos.x == 5);
        REQUIRE(pos.y == 10);
    }

    SECTION("Check sight blocking") {
        std::shared_ptr<ecs::Entity> wall(EntityBuilder()
            .withPosition(0, 0)
            .withRenderable("#", ftxui::Color::GrayDark)
            .build().release());

        auto* render = wall->getComponent<RenderableComponent>();
        render->blocks_sight = true;

        REQUIRE(render_bridge.doesEntityBlockSight(wall) == true);

        std::shared_ptr<ecs::Entity> item(EntityBuilder()
            .withPosition(1, 1)
            .withRenderable("!", ftxui::Color::Yellow)
            .build().release());

        REQUIRE(render_bridge.doesEntityBlockSight(item) == false);
    }

    SECTION("Sort by render priority") {
        std::shared_ptr<ecs::Entity> entity1(EntityBuilder()
            .withPosition(0, 0)
            .withRenderable("1", ftxui::Color::White)
            .build().release());
        entity1->getComponent<RenderableComponent>()->render_priority = 1;

        std::shared_ptr<ecs::Entity> entity2(EntityBuilder()
            .withPosition(0, 0)
            .withRenderable("2", ftxui::Color::White)
            .build().release());
        entity2->getComponent<RenderableComponent>()->render_priority = 10;

        std::shared_ptr<ecs::Entity> entity3(EntityBuilder()
            .withPosition(0, 0)
            .withRenderable("3", ftxui::Color::White)
            .build().release());
        entity3->getComponent<RenderableComponent>()->render_priority = 5;

        std::vector<std::shared_ptr<ecs::Entity>> entities = {entity1, entity2, entity3};
        auto sorted = render_bridge.sortByRenderPriority(entities);

        REQUIRE(sorted.size() == 3);
        REQUIRE(sorted[0] == entity2); // Highest priority
        REQUIRE(sorted[1] == entity3);
        REQUIRE(sorted[2] == entity1); // Lowest priority
    }

    SECTION("Update visibility based on FOV") {
        // Create entities
        std::shared_ptr<ecs::Entity> visible_entity(PlayerFactory().create(5, 5).release());
        std::shared_ptr<ecs::Entity> hidden_entity(MonsterFactoryECS().create("goblin", 10, 10).release());

        auto legacy_player = legacy_manager.createPlayer(5, 5);
        auto legacy_monster = std::make_shared<::Entity>(10, 10, "g", ftxui::Color::Green, "Goblin");
        legacy_monster->setVisible(false);

        // Set entities to be visible initially
        visible_entity->getComponent<RenderableComponent>()->setVisible(true);
        hidden_entity->getComponent<RenderableComponent>()->setVisible(true);

        entity_bridge.syncEntity(legacy_player, visible_entity);
        entity_bridge.syncEntity(legacy_monster, hidden_entity);

        // Create FOV (only 5,5 is visible)
        std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, false));
        fov[5][5] = true;

        render_bridge.updateEntitiesVisibility(fov);

        REQUIRE(render_bridge.isEntityVisible(visible_entity) == true);
        REQUIRE(render_bridge.isEntityVisible(hidden_entity) == false);
    }

    SECTION("Get entities in view") {
        // Create entities at different positions
        std::shared_ptr<ecs::Entity> entity1(PlayerFactory().create(5, 5).release());
        std::shared_ptr<ecs::Entity> entity2(MonsterFactoryECS().create("goblin", 6, 6).release());
        std::shared_ptr<ecs::Entity> entity3(MonsterFactoryECS().create("orc", 15, 15).release()); // Out of view

        auto legacy1 = legacy_manager.createPlayer(5, 5);
        auto legacy2 = std::make_shared<::Entity>(6, 6, "g", ftxui::Color::Green, "Goblin");
        auto legacy3 = std::make_shared<::Entity>(15, 15, "o", ftxui::Color::RGB(139, 69, 19), "Orc");

        // Make sure all entities are visible for this test
        entity1->getComponent<RenderableComponent>()->setVisible(true);
        entity2->getComponent<RenderableComponent>()->setVisible(true);
        entity3->getComponent<RenderableComponent>()->setVisible(true);

        entity_bridge.syncEntity(legacy1, entity1);
        entity_bridge.syncEntity(legacy2, entity2);
        entity_bridge.syncEntity(legacy3, entity3);

        // FOV where everything is visible
        std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, true));

        // Get entities in a 10x10 view starting at (0,0)
        auto in_view = render_bridge.getEntitiesInView(0, 0, 10, 10, fov);

        REQUIRE(in_view.size() == 2); // entity1 and entity2
        REQUIRE(std::find(in_view.begin(), in_view.end(), entity1) != in_view.end());
        REQUIRE(std::find(in_view.begin(), in_view.end(), entity2) != in_view.end());
        REQUIRE(std::find(in_view.begin(), in_view.end(), entity3) == in_view.end());
    }
}