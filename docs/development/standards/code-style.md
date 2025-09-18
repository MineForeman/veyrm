# Veyrm Code Style Guide

## Overview

This guide defines the coding standards for the Veyrm project. Consistency improves readability, maintainability, and reduces cognitive load.

## C++ Version

We use **C++23** with modern features encouraged where they improve code clarity and safety.

## Naming Conventions

### Files

- **Source files**: `snake_case.cpp`
- **Header files**: `snake_case.h`
- **Test files**: `test_component_name.cpp`

### Classes and Structs

```cpp
// PascalCase for types
class GameManager { };
struct MonsterData { };

// Interfaces start with I
class IRenderable { };
```

### Functions and Methods

```cpp
// camelCase for functions
void updatePosition();
int calculateDamage();
bool isAlive() const;

// Getters don't use 'get' prefix
int health() const { return health_; }  // Not getHealth()

// Setters use 'set' prefix
void setHealth(int value) { health_ = value; }

// Boolean queries use 'is', 'has', 'can'
bool isEmpty() const;
bool hasItem() const;
bool canMove() const;
```

### Variables

```cpp
// Local variables: snake_case
int player_health = 100;
std::string item_name = "sword";

// Member variables: snake_case with trailing underscore
private:
    int health_;
    std::string name_;
    Point position_;

// Constants: UPPER_SNAKE_CASE
constexpr int MAX_HEALTH = 100;
const std::string DEFAULT_NAME = "Player";

// Static members: s_ prefix
static int s_instance_count;

// Global variables: g_ prefix (avoid when possible)
int g_debug_level = 0;
```

### Enums

```cpp
// Enum classes preferred, PascalCase
enum class GameState {
    MENU,      // Members in UPPER_SNAKE_CASE
    PLAYING,
    PAUSED,
    GAME_OVER
};

// Traditional enums (avoid)
enum Color {
    COLOR_RED,    // Prefix with enum name
    COLOR_GREEN,
    COLOR_BLUE
};
```

### Namespaces

```cpp
// snake_case for namespaces
namespace game_systems {
namespace combat {
    class CombatSystem { };
}  // namespace combat
}  // namespace game_systems
```

### Template Parameters

```cpp
// PascalCase for type parameters
template<typename TContainer, typename TValue>
class Cache { };

// snake_case for non-type parameters
template<int max_size>
class FixedArray { };
```

## Formatting

### Indentation

- Use **4 spaces** (no tabs)
- Indent access specifiers at class level
- Indent case labels in switch statements

### Braces

```cpp
// Allman style for classes and functions
class Entity
{
public:
    void update()
    {
        // implementation
    }
};

// K&R style acceptable for short blocks
if (condition) {
    doSomething();
} else {
    doSomethingElse();
}

// Single line allowed for trivial cases
if (x < 0) return false;

// Always use braces for clarity
if (condition) {
    action();  // Even single statements
}
```

### Line Length

- Maximum **100 characters** per line
- Break at logical points for readability

### Spacing

```cpp
// Space after keywords
if (condition)
while (running)
for (int i = 0; i < 10; ++i)

// Space around operators
int sum = a + b;
bool result = x > y && z < w;

// No space after function names
function();
object.method();

// Space after commas
function(a, b, c);
```

### Function Declarations

```cpp
// Short functions on one line
int getValue() const { return value_; }

// Longer functions split logically
void processCommand(const std::string& command,
                   const std::vector<std::string>& args,
                   bool verbose = false);

// Very long parameter lists
void complexFunction(
    const std::string& first_parameter,
    const std::string& second_parameter,
    const std::string& third_parameter,
    const std::string& fourth_parameter);
```

## Classes

### Class Layout

```cpp
class Example {
public:    // Public interface first
    // Constructors and destructor
    Example();
    explicit Example(int value);
    ~Example();

    // Public methods
    void publicMethod();

protected: // Protected members for inheritance
    void protectedMethod();

private:   // Private implementation
    void privateMethod();

    // Member variables last
    int value_;
    std::string name_;
};
```

### Constructors

```cpp
// Use member initializer lists
Player::Player(int x, int y, int health)
    : Entity(x, y)
    , health_(health)
    , max_health_(health)
{
    // Constructor body only for complex logic
}

// Prefer default member initialization
class Item {
private:
    int value_ = 0;
    bool stackable_ = false;
};
```

### Rule of Five

```cpp
// If you define any, define all or delete
class Resource {
public:
    Resource();                                  // Constructor
    ~Resource();                                 // Destructor
    Resource(const Resource&);                  // Copy constructor
    Resource& operator=(const Resource&);       // Copy assignment
    Resource(Resource&&) noexcept;             // Move constructor
    Resource& operator=(Resource&&) noexcept;  // Move assignment
};

// Or explicitly delete
class Singleton {
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};
```

## Functions

### Function Design

```cpp
// Prefer return values over out parameters
Point getPosition() const;  // Good
void getPosition(Point& out) const;  // Avoid

// Use const references for large objects
void processEntity(const Entity& entity);

// Use std::optional for nullable returns
std::optional<Item> findItem(int id) const;

// Mark const methods explicitly
int getHealth() const { return health_; }
```

### Parameter Passing

```cpp
// Pass primitives by value
void setHealth(int health);

// Pass objects by const reference
void addItem(const Item& item);

// Pass by value and move for sink parameters
void setName(std::string name) {
    name_ = std::move(name);
}

// Use forwarding references for perfect forwarding
template<typename T>
void forward(T&& value) {
    process(std::forward<T>(value));
}
```

## Memory Management

### Smart Pointers

```cpp
// Prefer unique_ptr for single ownership
std::unique_ptr<Entity> player;

// Use shared_ptr for shared ownership
std::shared_ptr<Texture> texture;

// Never use raw new/delete
Entity* entity = new Entity();  // BAD
auto entity = std::make_unique<Entity>();  // GOOD

// Use raw pointers for non-owning references
Entity* getPlayer() { return player_.get(); }
```

### RAII

```cpp
// Always use RAII for resources
class FileHandle {
    FILE* file_;
public:
    explicit FileHandle(const char* name)
        : file_(fopen(name, "r")) { }

    ~FileHandle() {
        if (file_) fclose(file_);
    }
};
```

## Error Handling

### Exceptions

```cpp
// Use exceptions for exceptional cases
if (!file.open()) {
    throw std::runtime_error("Failed to open file: " + filename);
}

// Catch by const reference
try {
    processFile();
} catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
}
```

### Error Codes

```cpp
// Use std::optional or expected for expected failures
std::optional<Config> loadConfig(const std::string& path);

// Use enum class for error codes
enum class ErrorCode {
    SUCCESS = 0,
    FILE_NOT_FOUND,
    INVALID_FORMAT,
    PERMISSION_DENIED
};
```

## Modern C++ Features

### Auto

```cpp
// Use auto for complex types
auto it = container.begin();
auto factory = std::make_unique<MonsterFactory>();

// Be explicit for clarity
int count = getCount();  // Not auto
bool valid = isValid();  // Not auto
```

### Range-based For

```cpp
// Prefer range-based for loops
for (const auto& entity : entities) {
    entity->update();
}

// Use structured bindings
for (const auto& [key, value] : map) {
    process(key, value);
}
```

### Constexpr

```cpp
// Use constexpr for compile-time constants
constexpr int TILE_SIZE = 16;
constexpr float PI = 3.14159f;

// Constexpr functions where possible
constexpr int square(int x) {
    return x * x;
}
```

### Lambda Expressions

```cpp
// Use lambdas for local functions
auto compare = [](const Entity& a, const Entity& b) {
    return a.health() < b.health();
};

// Capture explicitly
[this] { return health_; }       // Capture this
[&total] { total += value; }     // Capture by reference
[=] { return x + y; }            // Capture by value
```

## Comments

### Documentation Comments

```cpp
/**
 * @brief Calculate damage for an attack
 * @param attacker The attacking entity
 * @param defender The defending entity
 * @return Damage dealt (minimum 1)
 */
int calculateDamage(const Entity& attacker, const Entity& defender);
```

### Implementation Comments

```cpp
// Explain why, not what
health_ -= damage;  // Apply damage after armor reduction

// TODO: Implement critical hits
// FIXME: Pathfinding fails on diagonal walls
// NOTE: This uses O(n²) algorithm for simplicity
```

### Avoid Obvious Comments

```cpp
// BAD: Increment counter
counter++;

// GOOD: Reset counter after overflow
if (counter > MAX_COUNT) {
    counter = 0;  // Prevent integer overflow
}
```

## Testing

### Test Naming

```cpp
TEST_CASE("ComponentName: What is being tested", "[category]") {
    SECTION("Specific scenario") {
        // Test implementation
    }
}

TEST_CASE("Combat: Damage calculation with armor", "[combat]") {
    SECTION("Armor reduces damage") {
        REQUIRE(calculateDamage(10, 3) == 7);
    }
}
```

### Test Organization

```cpp
// Arrange
Entity attacker(10, 5);
Entity defender(5, 10);

// Act
int damage = calculateDamage(attacker, defender);

// Assert
REQUIRE(damage > 0);
REQUIRE(damage <= attacker.attack());
```

## Best Practices

### General

1. **DRY** - Don't Repeat Yourself
2. **KISS** - Keep It Simple, Stupid
3. **YAGNI** - You Aren't Gonna Need It
4. **Single Responsibility** - Each class/function does one thing
5. **Composition over Inheritance** - Prefer composition

### Specific to Veyrm

1. Use smart pointers for ownership
2. Pass by const reference for large objects
3. Mark methods const when they don't modify state
4. Use enum classes over plain enums
5. Prefer std::string over char*
6. Initialize all members
7. Avoid global state
8. Use namespace for grouping
9. Write tests for new features
10. Document public APIs

## Code Review Checklist

- [ ] Follows naming conventions
- [ ] Properly formatted (use clang-format)
- [ ] No compiler warnings
- [ ] Memory leaks checked
- [ ] Tests included and passing
- [ ] Documentation updated
- [ ] No code duplication
- [ ] Error cases handled
- [ ] Performance considered
- [ ] Thread safety noted

## Tools

### Formatting

```bash
# Format file
clang-format -i src/file.cpp

# Format all files
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Static Analysis

```bash
# Clang-tidy
clang-tidy src/*.cpp -- -std=c++23

# Cppcheck
cppcheck --enable=all src/
```

### .clang-format Configuration

```yaml
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
AllowShortFunctionsOnASingleLine: Inline
```

---

*Last Updated: 2025-09-14*
*Version: 1.0*
