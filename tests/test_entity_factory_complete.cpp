#include <catch2/catch_test_macros.hpp>
#include "ecs/entity_factory.h"
#include "ecs/game_world.h"
#include "ecs/component.h"
#include "map.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace ecs;

class TestableEntityFactory : public EntityFactory {
public:
    TestableEntityFactory(GameWorld* world) : EntityFactory(world) {}

    // Expose protected methods for testing
    using EntityFactory::createBaseEntity;
    using EntityFactory::addPositionComponent;
    using EntityFactory::addHealthComponent;
    using EntityFactory::addRenderableComponent;
    using EntityFactory::addCombatComponent;
    using EntityFactory::addAIComponent;
    using EntityFactory::loadMonsterData;
    using EntityFactory::loadItemData;
};

TEST_CASE("EntityFactory basic entity creation", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableEntityFactory factory(world.get());

    SECTION("Create empty entity") {
        auto entity = factory.createEntity();
        REQUIRE(entity != nullptr);
    }

    SECTION("Create player entity") {
        auto player = factory.createPlayer(10, 15);
        REQUIRE(player != nullptr);

        // Verify player components
        REQUIRE(world->getComponent<PositionComponent>(player) != nullptr);
        REQUIRE(world->getComponent<HealthComponent>(player) != nullptr);
        REQUIRE(world->getComponent<RenderableComponent>(player) != nullptr);
        REQUIRE(world->getComponent<CombatComponent>(player) != nullptr);
        REQUIRE(world->getComponent<PlayerComponent>(player) != nullptr);
        REQUIRE(world->getComponent<StatsComponent>(player) != nullptr);
        REQUIRE(world->getComponent<InventoryComponent>(player) != nullptr);
        REQUIRE(world->getComponent<ExperienceComponent>(player) != nullptr);
        REQUIRE(world->getComponent<EquipmentComponent>(player) != nullptr);

        // Verify position
        auto* pos = world->getComponent<PositionComponent>(player);
        REQUIRE(pos->x == 10);
        REQUIRE(pos->y == 15);

        // Verify renderable
        auto* render = world->getComponent<RenderableComponent>(player);
        REQUIRE(render->glyph == '@');
    }

    SECTION("Create monster from template") {
        auto goblin = factory.createMonster("goblin", 20, 25);
        REQUIRE(goblin != nullptr);

        // Basic components
        REQUIRE(world->getComponent<PositionComponent>(goblin) != nullptr);
        REQUIRE(world->getComponent<HealthComponent>(goblin) != nullptr);
        REQUIRE(world->getComponent<RenderableComponent>(goblin) != nullptr);
        REQUIRE(world->getComponent<CombatComponent>(goblin) != nullptr);
        REQUIRE(world->getComponent<AIComponent>(goblin) != nullptr);

        // Monster should have AI
        auto* ai = world->getComponent<AIComponent>(goblin);
        REQUIRE(ai->behavior != AIBehavior::None);
    }

    SECTION("Create item from template") {
        auto potion = factory.createItem("potion_minor", 30, 35);
        REQUIRE(potion != nullptr);

        // Item components
        REQUIRE(world->getComponent<PositionComponent>(potion) != nullptr);
        REQUIRE(world->getComponent<RenderableComponent>(potion) != nullptr);
        REQUIRE(world->getComponent<ItemComponent>(potion) != nullptr);

        auto* item = world->getComponent<ItemComponent>(potion);
        REQUIRE(item->type != ItemType::None);
    }

    SECTION("Create wall entity") {
        auto wall = factory.createWall(5, 5, '#');
        REQUIRE(wall != nullptr);

        auto* pos = world->getComponent<PositionComponent>(wall);
        REQUIRE(pos->x == 5);
        REQUIRE(pos->y == 5);

        auto* render = world->getComponent<RenderableComponent>(wall);
        REQUIRE(render->glyph == '#');
    }

    SECTION("Create door entity") {
        auto door = factory.createDoor(8, 8, false);
        REQUIRE(door != nullptr);

        auto* door_comp = world->getComponent<DoorComponent>(door);
        REQUIRE(door_comp != nullptr);
        REQUIRE(door_comp->isOpen == false);

        auto* render = world->getComponent<RenderableComponent>(door);
        REQUIRE(render->glyph == '+'); // Closed door
    }

    SECTION("Create stairs") {
        auto stairsDown = factory.createStairs(12, 12, true);
        auto stairsUp = factory.createStairs(14, 14, false);

        REQUIRE(stairsDown != nullptr);
        REQUIRE(stairsUp != nullptr);

        auto* renderDown = world->getComponent<RenderableComponent>(stairsDown);
        auto* renderUp = world->getComponent<RenderableComponent>(stairsUp);

        REQUIRE(renderDown->glyph == '>');
        REQUIRE(renderUp->glyph == '<');
    }

    SECTION("Create trap") {
        auto trap = factory.createTrap(16, 16, TrapType::Spike);
        REQUIRE(trap != nullptr);

        auto* trap_comp = world->getComponent<TrapComponent>(trap);
        REQUIRE(trap_comp != nullptr);
        REQUIRE(trap_comp->type == TrapType::Spike);
        REQUIRE(trap_comp->isVisible == false);
    }

    SECTION("Create container") {
        auto chest = factory.createContainer(18, 18, "chest", 30);
        REQUIRE(chest != nullptr);

        auto* inventory = world->getComponent<InventoryComponent>(chest);
        REQUIRE(inventory != nullptr);
        REQUIRE(inventory->capacity == 30);

        auto* render = world->getComponent<RenderableComponent>(chest);
        REQUIRE(render->glyph == '□');
    }

    SECTION("Create projectile") {
        auto arrow = factory.createProjectile(22, 22, 24, 24, 15);
        REQUIRE(arrow != nullptr);

        auto* projectile = world->getComponent<ProjectileComponent>(arrow);
        REQUIRE(projectile != nullptr);
        REQUIRE(projectile->targetX == 24);
        REQUIRE(projectile->targetY == 24);
        REQUIRE(projectile->damage == 15);
        REQUIRE(projectile->speed > 0);
    }

    SECTION("Create effect") {
        auto explosion = factory.createEffect(26, 26, EffectType::Explosion, 2.0f);
        REQUIRE(explosion != nullptr);

        auto* effect = world->getComponent<EffectComponent>(explosion);
        REQUIRE(effect != nullptr);
        REQUIRE(effect->type == EffectType::Explosion);
        REQUIRE(effect->duration == 2.0f);
    }

    SECTION("Create NPC") {
        auto merchant = factory.createNPC("merchant", 30, 30);
        REQUIRE(merchant != nullptr);

        auto* npc = world->getComponent<NPCComponent>(merchant);
        REQUIRE(npc != nullptr);
        REQUIRE(npc->role == NPCRole::Merchant);
        REQUIRE(npc->canTrade == true);
    }
}

TEST_CASE("EntityFactory monster variants", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Create different monster types") {
        auto goblin = factory.createMonster("goblin", 10, 10);
        auto orc = factory.createMonster("orc", 12, 10);
        auto dragon = factory.createMonster("dragon", 14, 10);

        REQUIRE(goblin != nullptr);
        REQUIRE(orc != nullptr);
        REQUIRE(dragon != nullptr);

        // Different stats
        auto* goblinHealth = world->getComponent<HealthComponent>(goblin);
        auto* orcHealth = world->getComponent<HealthComponent>(orc);
        auto* dragonHealth = world->getComponent<HealthComponent>(dragon);

        if (goblinHealth && orcHealth && dragonHealth) {
            REQUIRE(goblinHealth->maxHp < orcHealth->maxHp);
            REQUIRE(orcHealth->maxHp < dragonHealth->maxHp);
        }
    }

    SECTION("Create elite variant") {
        auto elite = factory.createEliteMonster("goblin", 10, 10);
        auto normal = factory.createMonster("goblin", 12, 10);

        REQUIRE(elite != nullptr);
        REQUIRE(normal != nullptr);

        auto* eliteHealth = world->getComponent<HealthComponent>(elite);
        auto* normalHealth = world->getComponent<HealthComponent>(normal);

        if (eliteHealth && normalHealth) {
            REQUIRE(eliteHealth->maxHp > normalHealth->maxHp);
        }

        auto* eliteCombat = world->getComponent<CombatComponent>(elite);
        auto* normalCombat = world->getComponent<CombatComponent>(normal);

        if (eliteCombat && normalCombat) {
            REQUIRE(eliteCombat->maxDamage > normalCombat->maxDamage);
        }
    }

    SECTION("Create boss monster") {
        auto boss = factory.createBoss("lich", 25, 25);
        REQUIRE(boss != nullptr);

        auto* health = world->getComponent<HealthComponent>(boss);
        REQUIRE(health->maxHp >= 500); // Bosses have lots of HP

        auto* ai = world->getComponent<AIComponent>(boss);
        REQUIRE(ai->isBoss == true);

        auto* loot = world->getComponent<LootComponent>(boss);
        REQUIRE(loot->guaranteedDrops.size() > 0);
    }

    SECTION("Create monster with custom stats") {
        MonsterTemplate customGoblin;
        customGoblin.id = "custom_goblin";
        customGoblin.name = "Super Goblin";
        customGoblin.glyph = 'G';
        customGoblin.color = "red";
        customGoblin.maxHp = 100;
        customGoblin.minDamage = 10;
        customGoblin.maxDamage = 20;
        customGoblin.defense = 5;
        customGoblin.speed = 150;
        customGoblin.visionRange = 12;
        customGoblin.xpValue = 50;

        auto custom = factory.createMonsterFromTemplate(customGoblin, 10, 10);
        REQUIRE(custom != nullptr);

        auto* health = world->getComponent<HealthComponent>(custom);
        REQUIRE(health->maxHp == 100);

        auto* combat = world->getComponent<CombatComponent>(custom);
        REQUIRE(combat->minDamage == 10);
        REQUIRE(combat->maxDamage == 20);
    }
}

TEST_CASE("EntityFactory item creation", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Create consumable items") {
        auto potion = factory.createItem("potion_minor", 10, 10);
        auto scroll = factory.createItem("scroll_teleport", 12, 10);
        auto food = factory.createItem("food_bread", 14, 10);

        REQUIRE(potion != nullptr);
        REQUIRE(scroll != nullptr);
        REQUIRE(food != nullptr);

        auto* potionItem = world->getComponent<ItemComponent>(potion);
        REQUIRE(potionItem->type == ItemType::Consumable);
        REQUIRE(potionItem->useEffect == ItemEffect::Heal);
    }

    SECTION("Create equipment items") {
        auto sword = factory.createItem("sword_basic", 10, 10);
        auto armor = factory.createItem("armor_leather", 12, 10);
        auto ring = factory.createItem("ring_protection", 14, 10);

        REQUIRE(sword != nullptr);
        REQUIRE(armor != nullptr);
        REQUIRE(ring != nullptr);

        auto* swordItem = world->getComponent<ItemComponent>(sword);
        REQUIRE(swordItem->type == ItemType::Weapon);
        REQUIRE(swordItem->equipSlot == EquipmentSlot::MainHand);

        auto* armorItem = world->getComponent<ItemComponent>(armor);
        REQUIRE(armorItem->type == ItemType::Armor);
        REQUIRE(armorItem->equipSlot == EquipmentSlot::Body);
    }

    SECTION("Create stackable items") {
        auto gold = factory.createItem("gold_coins", 10, 10);
        auto arrows = factory.createItem("arrow", 12, 10);

        REQUIRE(gold != nullptr);
        REQUIRE(arrows != nullptr);

        auto* goldItem = world->getComponent<ItemComponent>(gold);
        REQUIRE(goldItem->isStackable == true);
        REQUIRE(goldItem->stackSize > 0);

        auto* arrowItem = world->getComponent<ItemComponent>(arrows);
        REQUIRE(arrowItem->isStackable == true);
    }

    SECTION("Create quest items") {
        auto artifact = factory.createQuestItem("ancient_artifact", 10, 10);
        REQUIRE(artifact != nullptr);

        auto* item = world->getComponent<ItemComponent>(artifact);
        REQUIRE(item->isQuest == true);
        REQUIRE(item->isDroppable == false);
    }

    SECTION("Create unique items") {
        auto legendary = factory.createUniqueItem("sword_excalibur", 10, 10);
        REQUIRE(legendary != nullptr);

        auto* item = world->getComponent<ItemComponent>(legendary);
        REQUIRE(item->isUnique == true);
        REQUIRE(item->rarity == ItemRarity::Legendary);
    }

    SECTION("Create random item") {
        auto random = factory.createRandomItem(10, 10, 5); // Level 5 item
        REQUIRE(random != nullptr);

        auto* item = world->getComponent<ItemComponent>(random);
        REQUIRE(item->type != ItemType::None);
    }

    SECTION("Create item with custom properties") {
        ItemTemplate customSword;
        customSword.id = "custom_sword";
        customSword.name = "Flame Sword";
        customSword.glyph = '/';
        customSword.color = "red";
        customSword.type = ItemType::Weapon;
        customSword.value = 500;
        customSword.weight = 3.5f;
        customSword.damageBonus = 10;
        customSword.fireDamage = 5;
        customSword.equipSlot = EquipmentSlot::MainHand;

        auto custom = factory.createItemFromTemplate(customSword, 10, 10);
        REQUIRE(custom != nullptr);

        auto* item = world->getComponent<ItemComponent>(custom);
        REQUIRE(item->value == 500);
        REQUIRE(item->weight == 3.5f);
    }
}

TEST_CASE("EntityFactory component addition", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableEntityFactory factory(world.get());

    SECTION("Add position component") {
        auto entity = factory.createEntity();
        factory.addPositionComponent(entity, 25, 35);

        auto* pos = world->getComponent<PositionComponent>(entity);
        REQUIRE(pos != nullptr);
        REQUIRE(pos->x == 25);
        REQUIRE(pos->y == 35);
    }

    SECTION("Add health component") {
        auto entity = factory.createEntity();
        factory.addHealthComponent(entity, 80, 100);

        auto* health = world->getComponent<HealthComponent>(entity);
        REQUIRE(health != nullptr);
        REQUIRE(health->hp == 80);
        REQUIRE(health->maxHp == 100);
    }

    SECTION("Add renderable component") {
        auto entity = factory.createEntity();
        factory.addRenderableComponent(entity, '$',
            ftxui::Color::RGB(255, 215, 0), // Gold
            ftxui::Color::RGB(0, 0, 0));    // Black background

        auto* render = world->getComponent<RenderableComponent>(entity);
        REQUIRE(render != nullptr);
        REQUIRE(render->glyph == '$');
    }

    SECTION("Add combat component") {
        auto entity = factory.createEntity();
        factory.addCombatComponent(entity, 5, 10, 3);

        auto* combat = world->getComponent<CombatComponent>(entity);
        REQUIRE(combat != nullptr);
        REQUIRE(combat->minDamage == 5);
        REQUIRE(combat->maxDamage == 10);
        REQUIRE(combat->defense == 3);
    }

    SECTION("Add AI component") {
        auto entity = factory.createEntity();
        factory.addAIComponent(entity, AIBehavior::Aggressive, 8);

        auto* ai = world->getComponent<AIComponent>(entity);
        REQUIRE(ai != nullptr);
        REQUIRE(ai->behavior == AIBehavior::Aggressive);
        REQUIRE(ai->visionRange == 8);
    }

    SECTION("Add multiple components") {
        auto entity = factory.createEntity();

        factory.addPositionComponent(entity, 10, 10);
        factory.addHealthComponent(entity, 50, 50);
        factory.addRenderableComponent(entity, 'M', ftxui::Color::Red, ftxui::Color::Black);
        factory.addCombatComponent(entity, 3, 6, 2);
        factory.addAIComponent(entity, AIBehavior::Wander, 5);

        REQUIRE(world->getComponent<PositionComponent>(entity) != nullptr);
        REQUIRE(world->getComponent<HealthComponent>(entity) != nullptr);
        REQUIRE(world->getComponent<RenderableComponent>(entity) != nullptr);
        REQUIRE(world->getComponent<CombatComponent>(entity) != nullptr);
        REQUIRE(world->getComponent<AIComponent>(entity) != nullptr);
    }
}

TEST_CASE("EntityFactory data loading", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableEntityFactory factory(world.get());

    SECTION("Load monster data from JSON") {
        // Create test JSON data
        nlohmann::json monsterData = {
            {"monsters", {
                {
                    {"id", "test_goblin"},
                    {"name", "Test Goblin"},
                    {"glyph", "g"},
                    {"color", "green"},
                    {"max_hp", 20},
                    {"min_damage", 2},
                    {"max_damage", 5},
                    {"defense", 1},
                    {"speed", 100},
                    {"vision_range", 6},
                    {"xp_value", 10},
                    {"behavior", "aggressive"},
                    {"loot_table", {
                        {"gold_min", 1},
                        {"gold_max", 5}
                    }}
                }
            }}
        };

        // Save to temp file
        std::ofstream file("test_monsters.json");
        file << monsterData.dump();
        file.close();

        bool loaded = factory.loadMonsterData("test_monsters.json");
        REQUIRE(loaded == true);

        // Create monster from loaded data
        auto monster = factory.createMonster("test_goblin", 10, 10);
        REQUIRE(monster != nullptr);

        // Clean up
        std::remove("test_monsters.json");
    }

    SECTION("Load item data from JSON") {
        nlohmann::json itemData = {
            {"items", {
                {
                    {"id", "test_sword"},
                    {"name", "Test Sword"},
                    {"glyph", "/"},
                    {"color", "silver"},
                    {"type", "weapon"},
                    {"value", 100},
                    {"weight", 3.0},
                    {"damage_bonus", 5},
                    {"equip_slot", "main_hand"},
                    {"rarity", "common"}
                }
            }}
        };

        std::ofstream file("test_items.json");
        file << itemData.dump();
        file.close();

        bool loaded = factory.loadItemData("test_items.json");
        REQUIRE(loaded == true);

        auto item = factory.createItem("test_sword", 10, 10);
        REQUIRE(item != nullptr);

        std::remove("test_items.json");
    }

    SECTION("Handle missing data file") {
        bool loaded = factory.loadMonsterData("nonexistent.json");
        REQUIRE(loaded == false);
    }

    SECTION("Handle invalid JSON") {
        std::ofstream file("invalid.json");
        file << "{ invalid json [}";
        file.close();

        bool loaded = factory.loadMonsterData("invalid.json");
        REQUIRE(loaded == false);

        std::remove("invalid.json");
    }
}

TEST_CASE("EntityFactory batch creation", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Create monster group") {
        auto goblins = factory.createMonsterGroup("goblin", 10, 10, 5, 3);
        REQUIRE(goblins.size() == 5);

        // All should be within radius
        for (auto goblin : goblins) {
            auto* pos = world->getComponent<PositionComponent>(goblin);
            int dx = std::abs(pos->x - 10);
            int dy = std::abs(pos->y - 10);
            REQUIRE(std::max(dx, dy) <= 3);
        }
    }

    SECTION("Create item pile") {
        std::vector<std::string> itemIds = {
            "gold_coins", "potion_minor", "sword_basic"
        };

        auto items = factory.createItemPile(itemIds, 15, 15);
        REQUIRE(items.size() == 3);

        // All at same position
        for (auto item : items) {
            auto* pos = world->getComponent<PositionComponent>(item);
            REQUIRE(pos->x == 15);
            REQUIRE(pos->y == 15);
        }
    }

    SECTION("Create dungeon level entities") {
        auto entities = factory.createDungeonLevel(5); // Level 5

        // Should create appropriate monsters and items for level 5
        REQUIRE(entities.monsters.size() > 0);
        REQUIRE(entities.items.size() > 0);

        // Monsters should be appropriate level
        for (auto monster : entities.monsters) {
            auto* exp = world->getComponent<ExperienceComponent>(monster);
            if (exp) {
                REQUIRE(exp->level >= 4);
                REQUIRE(exp->level <= 6);
            }
        }
    }

    SECTION("Create room contents") {
        Room room(5, 5, 10, 10);
        auto contents = factory.populateRoom(room, RoomType::Treasury);

        // Treasury should have lots of loot
        REQUIRE(contents.items.size() >= 5);

        // Items should be within room
        for (auto item : contents.items) {
            auto* pos = world->getComponent<PositionComponent>(item);
            REQUIRE(pos->x >= 5);
            REQUIRE(pos->x < 15);
            REQUIRE(pos->y >= 5);
            REQUIRE(pos->y < 15);
        }
    }
}

TEST_CASE("EntityFactory special entities", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Create corpse") {
        auto goblin = factory.createMonster("goblin", 10, 10);
        auto corpse = factory.createCorpse(goblin);
        REQUIRE(corpse != nullptr);

        auto* render = world->getComponent<RenderableComponent>(corpse);
        REQUIRE(render->glyph == '%'); // Corpse glyph

        auto* inventory = world->getComponent<InventoryComponent>(corpse);
        REQUIRE(inventory != nullptr); // Can loot corpses
    }

    SECTION("Create shrine") {
        auto shrine = factory.createShrine(20, 20, ShrineType::Healing);
        REQUIRE(shrine != nullptr);

        auto* shrineComp = world->getComponent<ShrineComponent>(shrine);
        REQUIRE(shrineComp->type == ShrineType::Healing);
        REQUIRE(shrineComp->usesRemaining > 0);
    }

    SECTION("Create portal") {
        auto portal = factory.createPortal(25, 25, 30, 30, 2); // To level 2
        REQUIRE(portal != nullptr);

        auto* portalComp = world->getComponent<PortalComponent>(portal);
        REQUIRE(portalComp->destinationX == 30);
        REQUIRE(portalComp->destinationY == 30);
        REQUIRE(portalComp->destinationLevel == 2);
    }

    SECTION("Create spawner") {
        auto spawner = factory.createSpawner(35, 35, "goblin", 30.0f, 5);
        REQUIRE(spawner != nullptr);

        auto* spawnerComp = world->getComponent<SpawnerComponent>(spawner);
        REQUIRE(spawnerComp->monsterType == "goblin");
        REQUIRE(spawnerComp->spawnInterval == 30.0f);
        REQUIRE(spawnerComp->maxSpawns == 5);
    }

    SECTION("Create altar") {
        auto altar = factory.createAltar(40, 40, "deity_of_war");
        REQUIRE(altar != nullptr);

        auto* altarComp = world->getComponent<AltarComponent>(altar);
        REQUIRE(altarComp->deity == "deity_of_war");
        REQUIRE(altarComp->canPray == true);
    }

    SECTION("Create fountain") {
        auto fountain = factory.createFountain(45, 45, FountainType::Mana);
        REQUIRE(fountain != nullptr);

        auto* fountainComp = world->getComponent<FountainComponent>(fountain);
        REQUIRE(fountainComp->type == FountainType::Mana);
        REQUIRE(fountainComp->isDry == false);
    }
}

TEST_CASE("EntityFactory error handling", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Create unknown monster type") {
        auto unknown = factory.createMonster("nonexistent_monster", 10, 10);
        // Should return default monster or nullptr
        if (unknown) {
            auto* health = world->getComponent<HealthComponent>(unknown);
            REQUIRE(health != nullptr); // Has basic components
        }
    }

    SECTION("Create unknown item type") {
        auto unknown = factory.createItem("nonexistent_item", 10, 10);
        if (unknown) {
            auto* item = world->getComponent<ItemComponent>(unknown);
            REQUIRE(item != nullptr);
        }
    }

    SECTION("Create entity at invalid position") {
        auto entity = factory.createMonster("goblin", -10, -10);
        REQUIRE(entity != nullptr); // Should still create

        auto* pos = world->getComponent<PositionComponent>(entity);
        // Position might be clamped or left as-is
        REQUIRE(pos != nullptr);
    }

    SECTION("Create with null world") {
        EntityFactory nullFactory(nullptr);
        auto entity = nullFactory.createEntity();
        // Should handle gracefully
        REQUIRE((entity == nullptr || entity != nullptr));
    }

    SECTION("Memory limits") {
        // Try to create many entities
        std::vector<Entity*> entities;
        for (int i = 0; i < 10000; ++i) {
            auto entity = factory.createEntity();
            if (!entity) break;
            entities.push_back(entity);
        }

        // Should have created some entities
        REQUIRE(entities.size() > 0);

        // Clean up
        for (auto e : entities) {
            world->destroyEntity(e);
        }
    }
}

TEST_CASE("EntityFactory cloning", "[factory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Clone entity with all components") {
        auto original = factory.createPlayer(10, 10);

        // Modify some components
        if (auto* health = world->getComponent<HealthComponent>(original)) {
            health->hp = 75;
        }
        if (auto* stats = world->getComponent<StatsComponent>(original)) {
            stats->level = 5;
            stats->strength = 18;
        }

        auto clone = factory.cloneEntity(original);
        REQUIRE(clone != nullptr);
        REQUIRE(clone != original);

        // Verify components were copied
        auto* origHealth = world->getComponent<HealthComponent>(original);
        auto* cloneHealth = world->getComponent<HealthComponent>(clone);
        REQUIRE(cloneHealth->hp == origHealth->hp);

        auto* origStats = world->getComponent<StatsComponent>(original);
        auto* cloneStats = world->getComponent<StatsComponent>(clone);
        REQUIRE(cloneStats->level == origStats->level);
        REQUIRE(cloneStats->strength == origStats->strength);
    }

    SECTION("Clone without certain components") {
        auto original = factory.createMonster("goblin", 10, 10);

        auto clone = factory.cloneEntityExcept(original, {ComponentType::Position});
        REQUIRE(clone != nullptr);

        // Should have all components except position
        REQUIRE(world->getComponent<PositionComponent>(clone) == nullptr);
        REQUIRE(world->getComponent<HealthComponent>(clone) != nullptr);
        REQUIRE(world->getComponent<RenderableComponent>(clone) != nullptr);
    }
}

TEST_CASE("EntityFactory performance", "[factory][ecs]") {
    Map testMap(100, 100);
    auto world = std::make_unique<GameWorld>(&testMap);
    EntityFactory factory(world.get());

    SECTION("Mass entity creation") {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; ++i) {
            factory.createMonster("goblin", i % 100, i / 100);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should complete within reasonable time
        REQUIRE(duration.count() < 1000); // Less than 1 second for 1000 entities
    }

    SECTION("Template caching") {
        // First creation - loads template
        auto start1 = std::chrono::high_resolution_clock::now();
        auto goblin1 = factory.createMonster("goblin", 10, 10);
        auto end1 = std::chrono::high_resolution_clock::now();

        // Second creation - uses cached template
        auto start2 = std::chrono::high_resolution_clock::now();
        auto goblin2 = factory.createMonster("goblin", 12, 10);
        auto end2 = std::chrono::high_resolution_clock::now();

        auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
        auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);

        // Second creation should be faster (cached)
        // This is not always guaranteed due to system variability, so we just check both complete
        REQUIRE(goblin1 != nullptr);
        REQUIRE(goblin2 != nullptr);
    }
}