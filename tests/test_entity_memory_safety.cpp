#include <catch2/catch_test_macros.hpp>
#include "../include/entity.h"
#include "../include/monster.h"
#include "../include/monster_ai.h"
#include <memory>

TEST_CASE("Entity memory safety with AI data", "[entity][memory]") {
    SECTION("Type-safe AI data access") {
        Monster monster(10, 10, "goblin");

        // Initially no AI data
        REQUIRE(monster.hasAIData() == false);
        REQUIRE(monster.getAIData() == nullptr);

        // Add AI data using type-safe interface
        auto ai_data = std::make_shared<MonsterAIData>();
        ai_data->current_state = AIState::IDLE;
        ai_data->home_room_center = Point(5, 5);
        ai_data->turns_since_player_seen = 10;

        monster.setAIData(ai_data);

        // Verify AI data is accessible
        REQUIRE(monster.hasAIData() == true);
        REQUIRE(monster.getAIData() != nullptr);
        REQUIRE(monster.getAIData()->current_state == AIState::IDLE);
        REQUIRE(monster.getAIData()->home_room_center.x == 5);
        REQUIRE(monster.getAIData()->home_room_center.y == 5);
        REQUIRE(monster.getAIData()->turns_since_player_seen == 10);
    }

    SECTION("AI data ownership and lifetime") {
        auto monster = std::make_unique<Monster>(10, 10, "orc");
        auto ai_data = std::make_shared<MonsterAIData>();
        ai_data->current_state = AIState::HOSTILE;

        // Store weak reference to track lifetime
        std::weak_ptr<MonsterAIData> weak_ai = ai_data;

        monster->setAIData(ai_data);
        ai_data.reset(); // Release our reference

        // AI data should still exist (owned by entity)
        REQUIRE(!weak_ai.expired());
        REQUIRE(monster->getAIData() != nullptr);
        REQUIRE(monster->getAIData()->current_state == AIState::HOSTILE);

        // Destroy monster
        monster.reset();

        // AI data should now be destroyed
        REQUIRE(weak_ai.expired());
    }

    SECTION("Multiple entities can share AI data") {
        Monster monster1(10, 10, "goblin");
        Monster monster2(15, 15, "goblin");

        auto shared_ai = std::make_shared<MonsterAIData>();
        shared_ai->current_state = AIState::ALERT;

        monster1.setAIData(shared_ai);
        monster2.setAIData(shared_ai);

        // Both monsters share the same AI data
        REQUIRE(monster1.getAIData() == monster2.getAIData());

        // Modifying through one affects the other
        monster1.getAIData()->turns_since_player_seen = 5;
        REQUIRE(monster2.getAIData()->turns_since_player_seen == 5);
    }

    SECTION("Const correctness") {
        Monster monster(10, 10, "troll");
        auto ai_data = std::make_shared<MonsterAIData>();
        ai_data->current_state = AIState::FLEEING;
        monster.setAIData(ai_data);

        const Monster& const_monster = monster;
        const MonsterAIData* const_ai = const_monster.getAIData();

        REQUIRE(const_ai != nullptr);
        REQUIRE(const_ai->current_state == AIState::FLEEING);

        // Should not compile (const correctness):
        // const_ai->current_state = AIState::IDLE;
    }

    SECTION("Legacy getUserData compatibility") {
        Monster monster(10, 10, "skeleton");

        // Legacy interface should return nullptr initially
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        REQUIRE(monster.getUserData() == nullptr);
        #pragma GCC diagnostic pop

        // Add AI data through new interface
        auto ai_data = std::make_shared<MonsterAIData>();
        monster.setAIData(ai_data);

        // Legacy interface should return raw pointer
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        void* legacy_ptr = monster.getUserData();
        #pragma GCC diagnostic pop

        REQUIRE(legacy_ptr != nullptr);
        REQUIRE(legacy_ptr == ai_data.get());
    }

    SECTION("MonsterAI integration") {
        Monster monster(10, 10, "dragon");
        MonsterAI ai_system;

        // AI system should create AI data automatically
        REQUIRE(monster.hasAIData() == false);

        // Simulate AI system assigning data
        ai_system.ensureAIData(monster);

        REQUIRE(monster.hasAIData() == true);
        REQUIRE(monster.getAIData() != nullptr);
        REQUIRE(monster.getAIData()->current_state == AIState::IDLE);
    }
}

TEST_CASE("Entity without AI data", "[entity][memory]") {
    SECTION("Non-monster entities don't need AI data") {
        Entity base_entity(10, 10, "?", ftxui::Color::White, "unknown");

        REQUIRE(base_entity.hasAIData() == false);
        REQUIRE(base_entity.getAIData() == nullptr);

        // Entity destructor should handle no AI data gracefully
        // (tested implicitly when base_entity goes out of scope)
    }
}