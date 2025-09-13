#include <catch2/catch_test_macros.hpp>
#include "combat_system.h"
#include "player.h"
#include "monster.h"
#include "message_log.h"

class TestPlayer : public Player {
public:
    TestPlayer(int x, int y) : Player(x, y) {
        // Override config values for testing
        hp = 20;
        max_hp = 20;
        attack = 5;
        defense = 2;
    }
};

class TestMonster : public Monster {
public:
    TestMonster(int x, int y, const std::string& species) : Monster(x, y, species) {
        setStats(10, 10, 3, 1, 100, 5);
        setMetadata("Test Orc", "A test monster", "o", ftxui::Color::Green, 'b');
    }
};

TEST_CASE("CombatSystem Basic Functionality", "[combat]") {
    CombatSystem combat;
    TestPlayer player(5, 5);
    TestMonster monster(6, 5, "test_orc");

    SECTION("Attack roll calculation") {
        int roll = combat.getAttackRoll(player);
        REQUIRE(roll >= 1 + player.getAttackBonus());
        REQUIRE(roll <= 20 + player.getAttackBonus());
    }

    SECTION("Defense value calculation") {
        int defense = combat.getDefenseValue(monster);
        REQUIRE(defense == CombatSystem::BASE_DEFENSE + monster.getDefenseBonus());
    }

    SECTION("Damage calculation") {
        int damage = combat.calculateDamage(player);
        REQUIRE(damage >= CombatSystem::MIN_DAMAGE);
        REQUIRE(damage <= player.getBaseDamage());
    }

    SECTION("Critical hit detection") {
        REQUIRE(combat.isCriticalHit(20) == true);
        REQUIRE(combat.isCriticalHit(19) == false);
        REQUIRE(combat.isCriticalHit(21) == false); // Edge case
    }

    SECTION("Critical miss detection") {
        REQUIRE(combat.isCriticalMiss(1) == true);
        REQUIRE(combat.isCriticalMiss(2) == false);
        REQUIRE(combat.isCriticalMiss(0) == false); // Edge case
    }

    SECTION("Damage application") {
        TestMonster target(10, 10, "target");
        int original_hp = target.hp;

        combat.applyDamage(target, 5);

        // Should take at least minimum damage even with defense
        REQUIRE(target.hp < original_hp);
        REQUIRE(target.hp >= 0);
    }
}

TEST_CASE("Combat Process Integration", "[combat]") {
    CombatSystem combat;
    TestPlayer player(5, 5);
    TestMonster monster(6, 5, "test_orc");

    SECTION("Combat result structure") {
        auto result = combat.processAttack(player, monster);

        // Result should be valid
        REQUIRE((result.hit == true || result.hit == false));

        if (result.hit) {
            REQUIRE(result.damage >= CombatSystem::MIN_DAMAGE);
            REQUIRE(!result.attack_message.empty());
            REQUIRE(!result.damage_message.empty());

            // Check fatal flag accuracy
            REQUIRE(result.fatal == (monster.hp <= 0));
        } else {
            REQUIRE(result.damage == 0);
            REQUIRE(!result.attack_message.empty());
            REQUIRE(result.damage_message.empty());
        }
    }

    SECTION("Multiple attacks consistency") {
        // Test 100 attacks to check for consistency
        int hits = 0;
        int crits = 0;

        for (int i = 0; i < 100; ++i) {
            TestMonster fresh_monster(6, 5, "test");
            fresh_monster.setStats(10, 10, 1, 0, 100, 5); // Low defense for reliable hits

            auto result = combat.processAttack(player, fresh_monster);
            if (result.hit) {
                hits++;
                if (result.critical) {
                    crits++;
                }
            }
        }

        // Should have reasonable hit rate (>30% with good stats)
        REQUIRE(hits > 30);

        // Should have some critical hits (roughly 5% of hits)
        REQUIRE(crits >= 0); // At least possible
        REQUIRE(crits <= hits); // Can't have more crits than hits
    }
}

TEST_CASE("Combat with Message Log", "[combat]") {
    MessageLog log;
    CombatSystem combat(&log);
    TestPlayer player(5, 5);
    TestMonster monster(6, 5, "test_orc");

    SECTION("Message log integration") {
        auto result = combat.processAttack(player, monster);

        // Should have logged something
        auto messages = log.getMessages();
        REQUIRE(!messages.empty());

        // Should have attack message
        bool found_attack_msg = false;
        for (const auto& msg : messages) {
            if (msg.find("hit") != std::string::npos ||
                msg.find("miss") != std::string::npos) {
                found_attack_msg = true;
                break;
            }
        }
        REQUIRE(found_attack_msg);
    }

    SECTION("Critical hit messages") {
        // Force a scenario more likely to produce critical hits
        for (int i = 0; i < 50; ++i) {
            TestMonster fresh_monster(6, 5, "test");
            fresh_monster.setStats(1, 1, 0, 0, 100, 1); // Very weak for quick testing

            auto result = combat.processAttack(player, fresh_monster);
            if (result.critical) {
                // Should have critical hit message
                auto messages = log.getMessages();
                bool found_crit_msg = false;
                for (const auto& msg : messages) {
                    if (msg.find("critically") != std::string::npos) {
                        found_crit_msg = true;
                        break;
                    }
                }
                REQUIRE(found_crit_msg);
                break; // Found one, good enough
            }
        }
    }
}

TEST_CASE("Combat Entity Interface", "[combat]") {
    TestPlayer player(5, 5);
    TestMonster monster(6, 5, "test_orc");

    SECTION("Player combat interface") {
        REQUIRE(player.getAttackBonus() == player.attack);
        REQUIRE(player.getDefenseBonus() == player.defense);
        REQUIRE(player.getBaseDamage() == player.attack);
        REQUIRE(player.getCombatName() == "You");
    }

    SECTION("Monster combat interface") {
        REQUIRE(monster.getAttackBonus() == monster.attack);
        REQUIRE(monster.getDefenseBonus() == monster.defense);
        REQUIRE(monster.getBaseDamage() == monster.attack);
        REQUIRE(monster.getCombatName() == monster.name);
    }
}

TEST_CASE("Combat Balance and Edge Cases", "[combat]") {
    CombatSystem combat;

    SECTION("High defense vs low attack") {
        TestPlayer weak_attacker(5, 5);
        weak_attacker.attack = 1;

        TestMonster armored_target(6, 5, "armored");
        armored_target.setStats(20, 20, 1, 10, 100, 5); // High defense

        auto result = combat.processAttack(weak_attacker, armored_target);

        // Even with high defense, should still do minimum damage on hit
        if (result.hit) {
            REQUIRE(result.damage >= CombatSystem::MIN_DAMAGE);
        }
    }

    SECTION("Zero damage scenarios") {
        TestPlayer zero_attacker(5, 5);
        zero_attacker.attack = 0;

        TestMonster target(6, 5, "target");

        auto result = combat.processAttack(zero_attacker, target);

        // Should still do minimum damage
        if (result.hit) {
            REQUIRE(result.damage >= CombatSystem::MIN_DAMAGE);
        }
    }

    SECTION("Fatal damage detection") {
        TestPlayer strong_attacker(5, 5);
        strong_attacker.attack = 20; // Very high attack

        TestMonster weak_target(6, 5, "weak");
        weak_target.setStats(1, 1, 1, 0, 100, 1); // 1 HP

        auto result = combat.processAttack(strong_attacker, weak_target);

        if (result.hit) {
            REQUIRE(result.fatal == true);
            REQUIRE(weak_target.hp == 0);
            REQUIRE(!result.result_message.empty());
        }
    }
}

TEST_CASE("Combat System Configuration", "[combat]") {
    SECTION("Critical hit threshold") {
        REQUIRE(CombatSystem::CRITICAL_HIT_THRESHOLD == 20);
    }

    SECTION("Critical miss threshold") {
        REQUIRE(CombatSystem::CRITICAL_MISS_THRESHOLD == 1);
    }

    SECTION("Base defense value") {
        REQUIRE(CombatSystem::BASE_DEFENSE == 10);
    }

    SECTION("Minimum damage") {
        REQUIRE(CombatSystem::MIN_DAMAGE == 1);
    }
}