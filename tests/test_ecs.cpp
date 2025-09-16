#include <catch2/catch_test_macros.hpp>
#include "../include/ecs/entity.h"
#include "../include/ecs/position_component.h"
#include "../include/ecs/renderable_component.h"
#include "../include/ecs/health_component.h"
#include "../include/ecs/combat_component.h"

using namespace ecs;

TEST_CASE("ECS Entity basic operations", "[ecs][entity]") {
    SECTION("Entity has unique ID") {
        Entity e1;
        Entity e2;
        REQUIRE(e1.getID() != e2.getID());
        REQUIRE(e1.getID() < e2.getID());
    }

    SECTION("Entity starts with no components") {
        Entity entity;
        REQUIRE(entity.isValid() == false);
        REQUIRE(entity.getComponents().empty());
    }

    SECTION("Can add and retrieve components") {
        Entity entity;

        // Add position component
        auto& pos = entity.addComponent<PositionComponent>(10, 20);
        REQUIRE(entity.hasComponent<PositionComponent>());
        REQUIRE(entity.isValid());

        // Retrieve and verify
        auto* retrieved = entity.getComponent<PositionComponent>();
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->position.x == 10);
        REQUIRE(retrieved->position.y == 20);
        REQUIRE(&pos == retrieved);  // Same object
    }

    SECTION("Can have multiple components") {
        Entity entity;

        entity.addComponent<PositionComponent>(5, 5);
        entity.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);
        entity.addComponent<HealthComponent>(100);

        REQUIRE(entity.hasComponent<PositionComponent>());
        REQUIRE(entity.hasComponent<RenderableComponent>());
        REQUIRE(entity.hasComponent<HealthComponent>());
        REQUIRE(!entity.hasComponent<CombatComponent>());
    }

    SECTION("Can remove components") {
        Entity entity;
        entity.addComponent<PositionComponent>(0, 0);
        entity.addComponent<HealthComponent>(50);

        REQUIRE(entity.hasComponent<PositionComponent>());
        entity.removeComponent<PositionComponent>();
        REQUIRE(!entity.hasComponent<PositionComponent>());
        REQUIRE(entity.hasComponent<HealthComponent>());
    }

    SECTION("Can clone entity with all components") {
        Entity original;
        original.addComponent<PositionComponent>(10, 20);
        original.addComponent<HealthComponent>(100);

        auto cloned = original.clone();
        REQUIRE(cloned->getID() != original.getID());
        REQUIRE(cloned->hasComponent<PositionComponent>());
        REQUIRE(cloned->hasComponent<HealthComponent>());

        // Components are deep copied
        auto* orig_pos = original.getComponent<PositionComponent>();
        auto* clone_pos = cloned->getComponent<PositionComponent>();
        REQUIRE(orig_pos != clone_pos);  // Different objects
        REQUIRE(clone_pos->position.x == 10);
        REQUIRE(clone_pos->position.y == 20);
    }
}

TEST_CASE("PositionComponent functionality", "[ecs][position]") {
    SECTION("Basic position operations") {
        PositionComponent pos(10, 20);
        REQUIRE(pos.position.x == 10);
        REQUIRE(pos.position.y == 20);
        REQUIRE(pos.previous_position.x == 10);
        REQUIRE(pos.previous_position.y == 20);
    }

    SECTION("Movement tracking") {
        PositionComponent pos(0, 0);
        pos.moveTo(5, 5);

        REQUIRE(pos.position.x == 5);
        REQUIRE(pos.position.y == 5);
        REQUIRE(pos.previous_position.x == 0);
        REQUIRE(pos.previous_position.y == 0);
    }

    SECTION("Relative movement") {
        PositionComponent pos(10, 10);
        pos.moveBy(5, -3);

        REQUIRE(pos.position.x == 15);
        REQUIRE(pos.position.y == 7);
        REQUIRE(pos.previous_position.x == 10);
        REQUIRE(pos.previous_position.y == 10);
    }

    SECTION("Position queries") {
        PositionComponent pos(10, 20);
        REQUIRE(pos.isAt(10, 20));
        REQUIRE(!pos.isAt(5, 5));

        Point target(15, 20);
        REQUIRE(pos.distanceTo(target) == 5.0);
    }
}

TEST_CASE("RenderableComponent functionality", "[ecs][renderable]") {
    SECTION("Default construction") {
        RenderableComponent render;
        REQUIRE(render.glyph == "?");
        REQUIRE(render.color == ftxui::Color::White);
        REQUIRE(render.is_visible == true);
    }

    SECTION("Custom construction") {
        RenderableComponent render("@", ftxui::Color::Red, false);
        REQUIRE(render.glyph == "@");
        REQUIRE(render.color == ftxui::Color::Red);
        REQUIRE(render.is_visible == false);
    }

    SECTION("Visibility control") {
        RenderableComponent render;
        REQUIRE(render.isVisible());

        render.setVisible(false);
        REQUIRE(!render.isVisible());
    }

    SECTION("Glyph and color changes") {
        RenderableComponent render("a", ftxui::Color::Blue);

        render.setGlyph("A");
        REQUIRE(render.glyph == "A");

        render.setColor(ftxui::Color::Green);
        REQUIRE(render.color == ftxui::Color::Green);
    }
}

TEST_CASE("HealthComponent functionality", "[ecs][health]") {
    SECTION("Basic health operations") {
        HealthComponent health(100);
        REQUIRE(health.getHealth() == 100);
        REQUIRE(health.getMaxHealth() == 100);
        REQUIRE(health.isAlive());
        REQUIRE(!health.isDead());
        REQUIRE(health.isFullHealth());
    }

    SECTION("Taking damage") {
        HealthComponent health(100);

        int dealt = health.takeDamage(30);
        REQUIRE(dealt == 30);
        REQUIRE(health.getHealth() == 70);
        REQUIRE(health.isAlive());

        dealt = health.takeDamage(100);  // Overkill
        REQUIRE(dealt == 70);  // Only had 70 HP left
        REQUIRE(health.getHealth() == 0);
        REQUIRE(health.isDead());
    }

    SECTION("Healing") {
        HealthComponent health(100, 50);  // 50/100 HP

        int healed = health.heal(30);
        REQUIRE(healed == 30);
        REQUIRE(health.getHealth() == 80);

        healed = health.heal(50);  // Overheal
        REQUIRE(healed == 20);  // Only healed to max
        REQUIRE(health.getHealth() == 100);
        REQUIRE(health.isFullHealth());
    }

    SECTION("Health percentage") {
        HealthComponent health(100, 75);
        REQUIRE(health.getHealthPercent() == 75);

        health.takeDamage(25);
        REQUIRE(health.getHealthPercent() == 50);

        health.takeDamage(50);
        REQUIRE(health.getHealthPercent() == 0);
    }

    SECTION("Max health changes") {
        HealthComponent health(100, 80);

        health.setMaxHealth(150, false);  // Don't heal
        REQUIRE(health.getMaxHealth() == 150);
        REQUIRE(health.getHealth() == 80);

        health.setMaxHealth(200, true);  // Heal to max
        REQUIRE(health.getMaxHealth() == 200);
        REQUIRE(health.getHealth() == 200);

        health.setMaxHealth(50);  // Reduce max (clamps current)
        REQUIRE(health.getMaxHealth() == 50);
        REQUIRE(health.getHealth() == 50);
    }
}

TEST_CASE("CombatComponent functionality", "[ecs][combat]") {
    SECTION("Basic combat stats") {
        CombatComponent combat(5, 2, 3);
        REQUIRE(combat.base_damage == 5);
        REQUIRE(combat.attack_bonus == 2);
        REQUIRE(combat.defense_bonus == 3);
        REQUIRE(combat.min_damage == 5);
        REQUIRE(combat.max_damage == 5);
    }

    SECTION("Damage range") {
        CombatComponent combat;
        combat.setDamageRange(2, 8);

        REQUIRE(combat.min_damage == 2);
        REQUIRE(combat.max_damage == 8);
        REQUIRE(combat.base_damage == 5);  // Average
    }

    SECTION("Combat modifiers") {
        CombatComponent combat(5, 10, 10);
        combat.attack_modifier = 5;
        combat.defense_modifier = -2;

        REQUIRE(combat.getTotalAttackBonus() == 15);
        REQUIRE(combat.getTotalDefenseBonus() == 8);
    }

    SECTION("Combat status effects") {
        CombatComponent combat;
        REQUIRE(combat.canAttack());
        REQUIRE(combat.canDefend());

        combat.is_stunned = true;
        REQUIRE(!combat.canAttack());
        REQUIRE(!combat.canDefend());

        combat.is_stunned = false;
        combat.is_sleeping = true;
        REQUIRE(combat.canAttack());
        REQUIRE(!combat.canDefend());
    }
}

TEST_CASE("ECS Integration - Player entity", "[ecs][integration]") {
    SECTION("Create player with components") {
        Entity player;

        // Add all player components
        player.addComponent<PositionComponent>(10, 10);
        player.addComponent<RenderableComponent>("@", ftxui::Color::Yellow);
        player.addComponent<HealthComponent>(100);
        player.addComponent<CombatComponent>(6, 3, 2);

        // Set combat name
        if (auto* combat = player.getComponent<CombatComponent>()) {
            combat->combat_name = "Player";
            combat->attack_verb = "strikes";
        }

        // Verify all components present
        REQUIRE(player.hasComponent<PositionComponent>());
        REQUIRE(player.hasComponent<RenderableComponent>());
        REQUIRE(player.hasComponent<HealthComponent>());
        REQUIRE(player.hasComponent<CombatComponent>());

        // Simulate taking damage
        if (auto* health = player.getComponent<HealthComponent>()) {
            health->takeDamage(30);
            REQUIRE(health->getHealth() == 70);
        }

        // Simulate movement
        if (auto* pos = player.getComponent<PositionComponent>()) {
            pos->moveBy(1, 0);
            REQUIRE(pos->position.x == 11);
            REQUIRE(pos->position.y == 10);
        }
    }
}

TEST_CASE("ECS Integration - Monster entity", "[ecs][integration]") {
    SECTION("Create goblin with components") {
        Entity goblin;

        goblin.addComponent<PositionComponent>(5, 5);
        goblin.addComponent<RenderableComponent>("g", ftxui::Color::Green);
        goblin.addComponent<HealthComponent>(20);
        goblin.addComponent<CombatComponent>(3, 1, 0);

        auto* combat = goblin.getComponent<CombatComponent>();
        REQUIRE(combat != nullptr);
        combat->combat_name = "Goblin";
        combat->setDamageRange(1, 4);

        // Simulate combat
        auto* health = goblin.getComponent<HealthComponent>();
        REQUIRE(health != nullptr);

        // Player attacks goblin
        health->takeDamage(8);
        REQUIRE(health->getHealth() == 12);

        // Goblin is injured but alive
        REQUIRE(health->isAlive());
        REQUIRE(!health->isDead());
    }
}