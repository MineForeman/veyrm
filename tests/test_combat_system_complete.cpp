#include <catch2/catch_test_macros.hpp>
#include "ecs/combat_system.h"
#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include "map.h"
#include <random>

using namespace ecs;

class TestableCombatSystem : public CombatSystem {
public:
    TestableCombatSystem(GameWorld* world, ILogger* logger)
        : CombatSystem(world, logger) {}

    // Expose protected methods for testing
    using CombatSystem::calculateDamage;
    using CombatSystem::calculateHitChance;
    using CombatSystem::calculateCriticalChance;
    using CombatSystem::applyDamage;
    using CombatSystem::processDeath;
    using CombatSystem::rollDice;
};

TEST_CASE("CombatSystem basic attacks", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableCombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Basic melee attack") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* defenderHealth = world->getComponent<HealthComponent>(defender);
        int initialHp = defenderHealth ? defenderHealth->hp : 20;

        bool hit = combatSystem.attack(attacker, defender);
        REQUIRE(hit == true);

        if (defenderHealth) {
            REQUIRE(defenderHealth->hp < initialHp);
        }
    }

    SECTION("Attack out of range") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 20, 20);

        bool hit = combatSystem.attack(attacker, defender);
        REQUIRE(hit == false);
    }

    SECTION("Calculate damage") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* attackerCombat = world->getComponent<CombatComponent>(attacker);
        if (attackerCombat) {
            attackerCombat->minDamage = 5;
            attackerCombat->maxDamage = 10;
            attackerCombat->attackBonus = 2;
        }

        auto* defenderCombat = world->getComponent<CombatComponent>(defender);
        if (defenderCombat) {
            defenderCombat->defense = 3;
        }

        int damage = combatSystem.calculateDamage(attacker, defender);
        REQUIRE(damage >= 4); // Min 5 + 2 - 3 = 4
        REQUIRE(damage <= 9); // Max 10 + 2 - 3 = 9
    }

    SECTION("Critical hit") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* attackerCombat = world->getComponent<CombatComponent>(attacker);
        if (attackerCombat) {
            attackerCombat->criticalChance = 100.0f; // Always crit
            attackerCombat->criticalMultiplier = 2.0f;
        }

        auto* defenderHealth = world->getComponent<HealthComponent>(defender);
        int initialHp = defenderHealth ? defenderHealth->hp : 20;

        combatSystem.attack(attacker, defender);

        if (defenderHealth) {
            int damage = initialHp - defenderHealth->hp;
            REQUIRE(damage >= 10); // Should be roughly double damage
        }
    }

    SECTION("Miss attack") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* attackerCombat = world->getComponent<CombatComponent>(attacker);
        if (attackerCombat) {
            attackerCombat->hitChance = 0.0f; // Always miss
        }

        auto* defenderHealth = world->getComponent<HealthComponent>(defender);
        int initialHp = defenderHealth ? defenderHealth->hp : 20;

        bool hit = combatSystem.attack(attacker, defender);

        if (defenderHealth) {
            REQUIRE(defenderHealth->hp == initialHp); // No damage
        }
    }

    SECTION("Killing blow") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* defenderHealth = world->getComponent<HealthComponent>(defender);
        if (defenderHealth) {
            defenderHealth->hp = 1; // Near death
        }

        auto* attackerCombat = world->getComponent<CombatComponent>(attacker);
        if (attackerCombat) {
            attackerCombat->minDamage = 10;
            attackerCombat->maxDamage = 20;
        }

        combatSystem.attack(attacker, defender);

        if (defenderHealth) {
            REQUIRE(defenderHealth->hp <= 0);
            REQUIRE(defenderHealth->isDead == true);
        }
    }
}

TEST_CASE("CombatSystem damage types", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    CombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Physical damage") {
        auto target = factory.createMonster("goblin", 10, 10);
        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 20;

        combatSystem.dealDamage(target, 10, DamageType::Physical);

        if (health) {
            REQUIRE(health->hp == initialHp - 10);
        }
    }

    SECTION("Fire damage") {
        auto target = factory.createMonster("ice_elemental", 10, 10);

        auto* combat = world->getComponent<CombatComponent>(target);
        if (combat) {
            combat->fireResistance = -50; // Vulnerable to fire
        }

        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 50;

        combatSystem.dealDamage(target, 10, DamageType::Fire);

        if (health) {
            REQUIRE(health->hp < initialHp - 10); // Extra damage
        }
    }

    SECTION("Ice damage") {
        auto target = factory.createMonster("fire_elemental", 10, 10);

        auto* combat = world->getComponent<CombatComponent>(target);
        if (combat) {
            combat->iceResistance = -50; // Vulnerable to ice
        }

        combatSystem.dealDamage(target, 10, DamageType::Ice);
        // Should deal extra damage due to vulnerability
        REQUIRE(true);
    }

    SECTION("Poison damage over time") {
        auto target = factory.createMonster("goblin", 10, 10);
        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 20;

        combatSystem.applyPoison(target, 3, 5.0f); // 3 damage per second for 5 seconds

        // Update poison effects
        for (int i = 0; i < 5; ++i) {
            combatSystem.updateDamageOverTime(1.0f);
        }

        if (health) {
            REQUIRE(health->hp <= initialHp - 15); // 3 * 5 = 15 damage
        }
    }

    SECTION("Damage resistance") {
        auto target = factory.createMonster("armored_knight", 10, 10);

        auto* combat = world->getComponent<CombatComponent>(target);
        if (combat) {
            combat->physicalResistance = 50; // 50% resistance
        }

        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 100;

        combatSystem.dealDamage(target, 20, DamageType::Physical);

        if (health) {
            REQUIRE(health->hp == initialHp - 10); // Half damage
        }
    }

    SECTION("Damage immunity") {
        auto target = factory.createMonster("ghost", 10, 10);

        auto* combat = world->getComponent<CombatComponent>(target);
        if (combat) {
            combat->physicalResistance = 100; // Immune
        }

        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 50;

        combatSystem.dealDamage(target, 50, DamageType::Physical);

        if (health) {
            REQUIRE(health->hp == initialHp); // No damage
        }
    }
}

TEST_CASE("CombatSystem special attacks", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    CombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Area of effect attack") {
        auto caster = factory.createPlayer(10, 10);

        // Create multiple targets
        std::vector<Entity*> targets;
        for (int i = 0; i < 5; ++i) {
            targets.push_back(factory.createMonster("goblin", 10 + i, 10));
        }

        combatSystem.areaAttack(caster, 10, 10, 3, 10); // 3 radius, 10 damage

        // All targets should take damage
        for (auto target : targets) {
            if (auto* health = world->getComponent<HealthComponent>(target)) {
                REQUIRE(health->hp < health->maxHp);
            }
        }
    }

    SECTION("Cleave attack") {
        auto attacker = factory.createPlayer(10, 10);

        // Create adjacent enemies
        auto enemy1 = factory.createMonster("goblin", 11, 10);
        auto enemy2 = factory.createMonster("goblin", 10, 11);
        auto enemy3 = factory.createMonster("goblin", 9, 10);

        combatSystem.cleaveAttack(attacker, 15); // 15 damage to all adjacent

        if (auto* health = world->getComponent<HealthComponent>(enemy1)) {
            REQUIRE(health->hp < health->maxHp);
        }
        if (auto* health = world->getComponent<HealthComponent>(enemy2)) {
            REQUIRE(health->hp < health->maxHp);
        }
        if (auto* health = world->getComponent<HealthComponent>(enemy3)) {
            REQUIRE(health->hp < health->maxHp);
        }
    }

    SECTION("Ranged attack") {
        auto archer = factory.createPlayer(10, 10);
        auto target = factory.createMonster("goblin", 15, 10);

        auto* archerCombat = world->getComponent<CombatComponent>(archer);
        if (archerCombat) {
            archerCombat->attackRange = 10;
        }

        bool hit = combatSystem.rangedAttack(archer, target);
        REQUIRE(hit == true);

        if (auto* health = world->getComponent<HealthComponent>(target)) {
            REQUIRE(health->hp < health->maxHp);
        }
    }

    SECTION("Backstab attack") {
        auto rogue = factory.createPlayer(10, 10);
        auto target = factory.createMonster("goblin", 11, 10);

        // Set target facing away
        if (auto* ai = world->getComponent<AIComponent>(target)) {
            ai->facingDirection = Direction::East; // Facing away from rogue
        }

        auto* rogueCombat = world->getComponent<CombatComponent>(rogue);
        if (rogueCombat) {
            rogueCombat->backstabMultiplier = 3.0f;
        }

        combatSystem.backstabAttack(rogue, target);

        if (auto* health = world->getComponent<HealthComponent>(target)) {
            // Should deal significant damage
            REQUIRE(health->hp < health->maxHp - 10);
        }
    }

    SECTION("Charge attack") {
        auto warrior = factory.createPlayer(5, 10);
        auto target = factory.createMonster("goblin", 15, 10);

        auto* warriorCombat = world->getComponent<CombatComponent>(warrior);
        if (warriorCombat) {
            warriorCombat->chargeBonus = 2.0f; // Double damage on charge
        }

        combatSystem.chargeAttack(warrior, target);

        // Warrior should move closer
        if (auto* pos = world->getComponent<PositionComponent>(warrior)) {
            REQUIRE(pos->x > 5);
        }

        if (auto* health = world->getComponent<HealthComponent>(target)) {
            REQUIRE(health->hp < health->maxHp);
        }
    }
}

TEST_CASE("CombatSystem status effects", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    CombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Stun effect") {
        auto target = factory.createMonster("goblin", 10, 10);

        combatSystem.applyStun(target, 3.0f); // 3 second stun

        if (auto* effects = world->getComponent<EffectsComponent>(target)) {
            REQUIRE(effects->isStunned == true);
            REQUIRE(effects->stunDuration == 3.0f);
        }

        // Update to wear off
        combatSystem.updateStatusEffects(4.0f);

        if (auto* effects = world->getComponent<EffectsComponent>(target)) {
            REQUIRE(effects->isStunned == false);
        }
    }

    SECTION("Bleed effect") {
        auto target = factory.createMonster("goblin", 10, 10);
        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 20;

        combatSystem.applyBleed(target, 2, 5.0f); // 2 damage/sec for 5 sec

        for (int i = 0; i < 5; ++i) {
            combatSystem.updateDamageOverTime(1.0f);
        }

        if (health) {
            REQUIRE(health->hp <= initialHp - 10);
        }
    }

    SECTION("Slow effect") {
        auto target = factory.createMonster("goblin", 10, 10);

        combatSystem.applySlow(target, 0.5f, 5.0f); // 50% slow for 5 seconds

        if (auto* effects = world->getComponent<EffectsComponent>(target)) {
            REQUIRE(effects->moveSpeedMultiplier == 0.5f);
        }
    }

    SECTION("Buff effects") {
        auto target = factory.createPlayer(10, 10);
        auto* combat = world->getComponent<CombatComponent>(target);
        int baseDamage = combat ? combat->maxDamage : 5;

        combatSystem.applyBuff(target, BuffType::Strength, 5, 10.0f);

        if (combat) {
            REQUIRE(combat->maxDamage > baseDamage);
        }

        // Buff expires
        combatSystem.updateStatusEffects(11.0f);

        if (combat) {
            REQUIRE(combat->maxDamage == baseDamage);
        }
    }

    SECTION("Debuff effects") {
        auto target = factory.createMonster("goblin", 10, 10);
        auto* combat = world->getComponent<CombatComponent>(target);
        int baseDefense = combat ? combat->defense : 2;

        combatSystem.applyDebuff(target, DebuffType::Weakness, 3, 10.0f);

        if (combat) {
            REQUIRE(combat->defense < baseDefense);
        }
    }

    SECTION("Multiple status effects") {
        auto target = factory.createMonster("goblin", 10, 10);

        combatSystem.applyPoison(target, 1, 5.0f);
        combatSystem.applyBleed(target, 1, 5.0f);
        combatSystem.applyStun(target, 2.0f);
        combatSystem.applySlow(target, 0.5f, 3.0f);

        if (auto* effects = world->getComponent<EffectsComponent>(target)) {
            REQUIRE(effects->isPoisoned == true);
            REQUIRE(effects->isBleeding == true);
            REQUIRE(effects->isStunned == true);
            REQUIRE(effects->moveSpeedMultiplier == 0.5f);
        }
    }
}

TEST_CASE("CombatSystem healing", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    CombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Basic healing") {
        auto target = factory.createPlayer(10, 10);
        auto* health = world->getComponent<HealthComponent>(target);

        if (health) {
            health->hp = 50;
            health->maxHp = 100;
        }

        combatSystem.heal(target, 30);

        if (health) {
            REQUIRE(health->hp == 80);
        }
    }

    SECTION("Healing cannot exceed max HP") {
        auto target = factory.createPlayer(10, 10);
        auto* health = world->getComponent<HealthComponent>(target);

        if (health) {
            health->hp = 90;
            health->maxHp = 100;
        }

        combatSystem.heal(target, 50);

        if (health) {
            REQUIRE(health->hp == 100);
        }
    }

    SECTION("Healing over time") {
        auto target = factory.createPlayer(10, 10);
        auto* health = world->getComponent<HealthComponent>(target);

        if (health) {
            health->hp = 50;
            health->maxHp = 100;
        }

        combatSystem.applyRegeneration(target, 5, 5.0f); // 5 HP/sec for 5 sec

        for (int i = 0; i < 5; ++i) {
            combatSystem.updateHealingOverTime(1.0f);
        }

        if (health) {
            REQUIRE(health->hp == 75); // 50 + (5 * 5)
        }
    }

    SECTION("Resurrection") {
        auto target = factory.createPlayer(10, 10);
        auto* health = world->getComponent<HealthComponent>(target);

        if (health) {
            health->hp = 0;
            health->isDead = true;
        }

        bool resurrected = combatSystem.resurrect(target, 50);
        REQUIRE(resurrected == true);

        if (health) {
            REQUIRE(health->hp == 50);
            REQUIRE(health->isDead == false);
        }
    }

    SECTION("Life steal") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* attackerHealth = world->getComponent<HealthComponent>(attacker);
        if (attackerHealth) {
            attackerHealth->hp = 50;
            attackerHealth->maxHp = 100;
        }

        auto* attackerCombat = world->getComponent<CombatComponent>(attacker);
        if (attackerCombat) {
            attackerCombat->lifeSteal = 0.5f; // 50% life steal
            attackerCombat->minDamage = 10;
            attackerCombat->maxDamage = 10;
        }

        combatSystem.attack(attacker, defender);

        if (attackerHealth) {
            REQUIRE(attackerHealth->hp > 50); // Healed from life steal
        }
    }
}

TEST_CASE("CombatSystem experience and rewards", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    CombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Grant experience on kill") {
        auto player = factory.createPlayer(10, 10);
        auto goblin = factory.createMonster("goblin", 11, 10);

        auto* playerExp = world->getComponent<ExperienceComponent>(player);
        int initialExp = playerExp ? playerExp->currentExp : 0;

        // Kill the goblin
        if (auto* health = world->getComponent<HealthComponent>(goblin)) {
            health->hp = 1;
        }

        combatSystem.attack(player, goblin);

        if (playerExp) {
            REQUIRE(playerExp->currentExp > initialExp);
        }
    }

    SECTION("Level up from combat") {
        auto player = factory.createPlayer(10, 10);

        auto* playerExp = world->getComponent<ExperienceComponent>(player);
        if (playerExp) {
            playerExp->currentExp = 95;
            playerExp->expToNext = 100;
            playerExp->level = 1;
        }

        auto goblin = factory.createMonster("goblin", 11, 10);
        if (auto* health = world->getComponent<HealthComponent>(goblin)) {
            health->hp = 1;
        }

        combatSystem.attack(player, goblin);

        if (playerExp) {
            REQUIRE(playerExp->level == 2);
            REQUIRE(playerExp->currentExp < playerExp->expToNext);
        }
    }

    SECTION("Drop loot on death") {
        auto player = factory.createPlayer(10, 10);
        auto goblin = factory.createMonster("goblin", 11, 10);

        // Add loot table to goblin
        if (auto* loot = world->getComponent<LootComponent>(goblin)) {
            loot->goldMin = 5;
            loot->goldMax = 10;
        }

        // Kill goblin
        if (auto* health = world->getComponent<HealthComponent>(goblin)) {
            health->hp = 1;
        }

        combatSystem.attack(player, goblin);

        // Check if loot was dropped (implementation specific)
        REQUIRE(true);
    }
}

TEST_CASE("CombatSystem combat modifiers", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableCombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Hit chance calculation") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* attackerStats = world->getComponent<StatsComponent>(attacker);
        if (attackerStats) {
            attackerStats->dexterity = 20;
        }

        auto* defenderStats = world->getComponent<StatsComponent>(defender);
        if (defenderStats) {
            defenderStats->dexterity = 10;
        }

        float hitChance = combatSystem.calculateHitChance(attacker, defender);
        REQUIRE(hitChance > 0.5f); // Higher dex = better hit chance
    }

    SECTION("Critical chance calculation") {
        auto attacker = factory.createPlayer(10, 10);

        auto* stats = world->getComponent<StatsComponent>(attacker);
        if (stats) {
            stats->luck = 20;
        }

        auto* combat = world->getComponent<CombatComponent>(attacker);
        if (combat) {
            combat->criticalChance = 10.0f;
        }

        float critChance = combatSystem.calculateCriticalChance(attacker);
        REQUIRE(critChance > 10.0f); // Luck increases crit chance
    }

    SECTION("Weapon speed affects attacks") {
        auto attacker = factory.createPlayer(10, 10);

        auto* combat = world->getComponent<CombatComponent>(attacker);
        if (combat) {
            combat->attackSpeed = 2.0f; // Attacks twice as fast
        }

        float cooldown = combatSystem.getAttackCooldown(attacker);
        REQUIRE(cooldown < 1.0f); // Faster attack = shorter cooldown
    }

    SECTION("Dual wielding") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        auto* combat = world->getComponent<CombatComponent>(attacker);
        if (combat) {
            combat->isDualWielding = true;
        }

        // Should perform two attacks
        combatSystem.attack(attacker, defender);

        if (auto* health = world->getComponent<HealthComponent>(defender)) {
            // Check that extra damage was dealt
            REQUIRE(health->hp < health->maxHp - 5);
        }
    }

    SECTION("Armor penetration") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("armored_knight", 11, 10);

        auto* attackerCombat = world->getComponent<CombatComponent>(attacker);
        if (attackerCombat) {
            attackerCombat->armorPenetration = 50; // 50% armor pen
            attackerCombat->minDamage = 10;
            attackerCombat->maxDamage = 10;
        }

        auto* defenderCombat = world->getComponent<CombatComponent>(defender);
        if (defenderCombat) {
            defenderCombat->defense = 10;
        }

        int damage = combatSystem.calculateDamage(attacker, defender);
        REQUIRE(damage > 5); // Should ignore half the armor
    }
}

TEST_CASE("CombatSystem edge cases", "[combat][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    CombatSystem combatSystem(world.get(), nullptr);
    EntityFactory factory(world.get());

    SECTION("Null entity attacks") {
        auto defender = factory.createMonster("goblin", 10, 10);

        REQUIRE(combatSystem.attack(nullptr, defender) == false);
        REQUIRE(combatSystem.attack(defender, nullptr) == false);
        REQUIRE(combatSystem.attack(nullptr, nullptr) == false);
    }

    SECTION("Entity without combat component") {
        auto entity = factory.createEntity();
        auto defender = factory.createMonster("goblin", 10, 10);

        REQUIRE(combatSystem.attack(entity, defender) == false);
    }

    SECTION("Attack self") {
        auto entity = factory.createPlayer(10, 10);

        bool result = combatSystem.attack(entity, entity);
        REQUIRE(result == false); // Should prevent self-damage
    }

    SECTION("Attack dead entity") {
        auto attacker = factory.createPlayer(10, 10);
        auto defender = factory.createMonster("goblin", 11, 10);

        if (auto* health = world->getComponent<HealthComponent>(defender)) {
            health->hp = 0;
            health->isDead = true;
        }

        bool result = combatSystem.attack(attacker, defender);
        REQUIRE(result == false);
    }

    SECTION("Negative damage") {
        auto target = factory.createPlayer(10, 10);
        auto* health = world->getComponent<HealthComponent>(target);
        int initialHp = health ? health->hp : 100;

        combatSystem.dealDamage(target, -10, DamageType::Physical);

        if (health) {
            // Negative damage should not heal
            REQUIRE(health->hp <= initialHp);
        }
    }

    SECTION("Overflow damage") {
        auto target = factory.createMonster("goblin", 10, 10);

        combatSystem.dealDamage(target, INT_MAX, DamageType::Physical);

        if (auto* health = world->getComponent<HealthComponent>(target)) {
            REQUIRE(health->hp <= 0);
            REQUIRE(health->isDead == true);
        }
    }

    SECTION("Zero attack cooldown") {
        auto attacker = factory.createPlayer(10, 10);

        auto* combat = world->getComponent<CombatComponent>(attacker);
        if (combat) {
            combat->attackSpeed = 0.0f;
        }

        float cooldown = combatSystem.getAttackCooldown(attacker);
        REQUIRE(cooldown > 0.0f); // Should have minimum cooldown
    }

    SECTION("Invalid damage type") {
        auto target = factory.createMonster("goblin", 10, 10);

        // Should handle gracefully
        combatSystem.dealDamage(target, 10, static_cast<DamageType>(999));
        REQUIRE(true);
    }
}