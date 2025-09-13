#pragma once

#include "point.h"
#include <string>
#include <vector>
#include <memory>

class Entity;
class Player;
class Monster;
class MessageLog;

class CombatSystem {
public:
    struct CombatResult {
        bool hit = false;
        int damage = 0;
        bool critical = false;
        bool fatal = false;
        std::string attack_message;
        std::string damage_message;
        std::string result_message;
    };

    CombatSystem();
    explicit CombatSystem(MessageLog* message_log);
    ~CombatSystem() = default;

    // Core combat methods
    CombatResult processAttack(Entity& attacker, Entity& defender);
    bool calculateHit(const Entity& attacker, const Entity& defender);
    int calculateDamage(const Entity& attacker);
    void applyDamage(Entity& target, int amount);

    // Combat utilities
    int getAttackRoll(const Entity& attacker);
    int getDefenseValue(const Entity& defender);
    bool isCriticalHit(int roll);
    bool isCriticalMiss(int roll);

    // Message integration
    void setMessageLog(MessageLog* log) { message_log = log; }
    void logCombatResult(const CombatResult& result);

    // Configuration
    static const int CRITICAL_HIT_THRESHOLD = 20;
    static const int CRITICAL_MISS_THRESHOLD = 1;
    static const int BASE_DEFENSE = 10;
    static const int MIN_DAMAGE = 1;

private:
    MessageLog* message_log = nullptr;

    // Combat message generation
    std::string generateAttackMessage(const Entity& attacker, const Entity& defender, bool hit, bool critical);
    std::string generateDamageMessage(const Entity& defender, int damage, bool fatal);

    // Internal helpers
    int rollD20();
    bool isPlayer(const Entity& entity);
    bool isMonster(const Entity& entity);
    std::string getEntityName(const Entity& entity);
};