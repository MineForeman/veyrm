# Modern C++ Best Practices Guide

This document defines the coding standards and best practices for the Veyrm codebase. These guidelines prioritize safety, performance, and maintainability.

## Table of Contents

1. [Core Principles](#core-principles)
2. [Class Design](#class-design)
3. [Resource Management](#resource-management)
4. [Inheritance vs Composition](#inheritance-vs-composition)
5. [Type Design](#type-design)
6. [Error Handling](#error-handling)
7. [Performance Guidelines](#performance-guidelines)
8. [Code Examples](#code-examples)

## Core Principles

### 1. Prefer Composition Over Inheritance

- Use inheritance **only** for is-a polymorphic interfaces (behavior)
- Never use inheritance for code sharing or "kind-of" relationships
- Favor composition, free functions, or templates for code reuse

### 2. Rule of Zero

- Let the standard library manage resources (RAII)
- Write special members only when you directly own a resource
- Use smart pointers and standard containers

### 3. Small, Value-Like, Immutable Classes

- Keep classes small and focused on a single responsibility
- Make classes value-like (copyable, comparable) when appropriate
- Prefer immutability - establish invariants in constructors/factories
- Avoid setters; prefer constructing new instances

### 4. Interface Design

- Use pure abstract base classes for runtime polymorphism
- Always mark overrides with `override`
- Mark concrete leaf classes with `final`
- Virtual destructors for all polymorphic base classes

### 5. No Raw Owning Pointers

- **Owning**: `std::unique_ptr<T>` or `std::shared_ptr<T>`
- **Non-owning**: References, `std::span<T>`, `std::string_view`
- **Observer**: `T*` only when nullable observation is needed

### 6. Const-Correctness and Explicit

- Be const-correct everywhere
- Mark single-argument constructors `explicit`
- Use `noexcept` where appropriate (move constructors, swap)
- Use `[[nodiscard]]` for important return values

### 7. Minimize Class Surface Area

- Prefer non-member, non-friend functions when possible
- Keep classes minimal - expose behavior, not state
- Hide implementation details

## Class Design

### Class Design Checklist

```cpp
class MyClass final {  // Use final for leaf classes
public:
    // Constructors
    MyClass() = default;                           // Explicitly default if appropriate
    explicit MyClass(int value);                   // explicit for single-arg
    MyClass(const MyClass&) = default;             // Rule of Five/Zero
    MyClass(MyClass&&) noexcept = default;
    MyClass& operator=(const MyClass&) = default;
    MyClass& operator=(MyClass&&) noexcept = default;
    ~MyClass() = default;

    // Public interface - behavior, not getters/setters
    [[nodiscard]] bool isValid() const noexcept;
    void process();

private:
    // Establish invariants
    int value_{0};  // In-class member initialization
};
```

### Special Member Functions

Follow the **Rule of Five** or **Rule of Zero**:

```cpp
// Rule of Zero (preferred)
class Point {
    int x_{0};
    int y_{0};
    // Compiler generates all special members correctly
};

// Rule of Five (when managing resources)
class ResourceOwner {
public:
    ResourceOwner();
    ~ResourceOwner();
    ResourceOwner(const ResourceOwner&);
    ResourceOwner(ResourceOwner&&) noexcept;
    ResourceOwner& operator=(const ResourceOwner&);
    ResourceOwner& operator=(ResourceOwner&&) noexcept;
private:
    Resource* resource_;  // Only if you can't use smart pointers
};
```

## Resource Management

### RAII Pattern

```cpp
// Good: RAII with unique_ptr
class FileHandle {
public:
    explicit FileHandle(const std::string& path)
        : file_{std::fopen(path.c_str(), "r"), &std::fclose} {
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }

    // Non-copyable, movable
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&&) noexcept = default;
    FileHandle& operator=(FileHandle&&) noexcept = default;

private:
    std::unique_ptr<FILE, decltype(&std::fclose)> file_;
};
```

### Smart Pointer Usage

```cpp
// Ownership patterns
class Game {
    // Single owner
    std::unique_ptr<Player> player_;

    // Shared ownership (rare - avoid if possible)
    std::shared_ptr<TextureCache> texture_cache_;

    // Non-owning observation
    Observer* observer_{nullptr};  // Can be null

    // Non-owning reference (cannot be null)
    void processEntity(const Entity& entity);

    // View into contiguous data
    void processItems(std::span<const Item> items);
};
```

## Inheritance vs Composition

### When Inheritance is Appropriate

```cpp
// Good: Pure interface for runtime polymorphism
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void render(const Scene& scene) = 0;
};

class OpenGLRenderer final : public Renderer {
public:
    void render(const Scene& scene) override;
};

class VulkanRenderer final : public Renderer {
public:
    void render(const Scene& scene) override;
};
```

### When to Avoid Inheritance

```cpp
// Bad: Inheritance for code reuse
class Vehicle {
protected:
    int speed_;
    void move();
};

class Car : public Vehicle {  // Bad!
    // ...
};

// Good: Composition
class Car {
private:
    Engine engine_;      // HAS-A relationship
    Transmission trans_;
    void move();
};
```

### Alternatives to Inheritance

#### 1. Static Polymorphism (Templates + Concepts)

```cpp
template<typename T>
concept Drawable = requires(T t, Canvas& c) {
    { t.draw(c) } -> std::same_as<void>;
};

template<Drawable T>
void renderAll(std::span<T> objects, Canvas& canvas) {
    for (auto& obj : objects) {
        obj.draw(canvas);
    }
}
```

#### 2. Sum Types (std::variant)

```cpp
// Closed set of alternatives
using Shape = std::variant<Circle, Rectangle, Triangle>;

void draw(const Shape& shape, Canvas& canvas) {
    std::visit([&canvas](const auto& s) { s.draw(canvas); }, shape);
}
```

#### 3. Type Erasure

```cpp
class Function {
public:
    template<typename F>
    Function(F f) : impl_{std::make_unique<Model<F>>(std::move(f))} {}

    void operator()() const { impl_->call(); }

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual void call() const = 0;
    };

    template<typename F>
    struct Model final : Concept {
        F f_;
        explicit Model(F f) : f_{std::move(f)} {}
        void call() const override { f_(); }
    };

    std::unique_ptr<Concept> impl_;
};
```

## Type Design

### Value Types

```cpp
// Good: Simple value type with all comparisons
struct Point {
    int x{0};
    int y{0};

    // C++20 spaceship operator
    friend auto operator<=>(const Point&, const Point&) = default;

    // Arithmetic operations
    constexpr Point operator+(const Point& other) const noexcept {
        return {x + other.x, y + other.y};
    }

    constexpr Point operator-(const Point& other) const noexcept {
        return {x - other.x, y - other.y};
    }
};
```

### Strong Types

```cpp
// Avoid primitive obsession with strong types
template<typename T, typename Tag>
class StrongType {
public:
    explicit StrongType(T value) : value_{std::move(value)} {}

    [[nodiscard]] const T& get() const noexcept { return value_; }

    friend bool operator==(const StrongType& a, const StrongType& b) = default;
    friend auto operator<=>(const StrongType& a, const StrongType& b) = default;

private:
    T value_;
};

// Usage
using PlayerId = StrongType<int, struct PlayerIdTag>;
using MonsterId = StrongType<int, struct MonsterIdTag>;

void attackMonster(PlayerId attacker, MonsterId target);
// Compile error if arguments are swapped!
```

## Error Handling

### Exception Safety Guarantees

```cpp
class Container {
public:
    // Strong guarantee - state unchanged on exception
    void push_back(const T& value) {
        if (size_ == capacity_) {
            auto new_buffer = allocate(capacity_ * 2);
            // If this throws, nothing changes
            copy_all(buffer_, new_buffer);
            deallocate(buffer_);
            buffer_ = new_buffer;
            capacity_ *= 2;
        }
        new (buffer_ + size_) T{value};
        ++size_;
    }

    // Nothrow guarantee
    void clear() noexcept {
        while (size_ > 0) {
            buffer_[--size_].~T();
        }
    }
};
```

### Error Handling Strategies

```cpp
// 1. Exceptions for exceptional cases
class FileReader {
public:
    explicit FileReader(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("File not found: " + path);
        }
        // ...
    }
};

// 2. Optional for expected absence
[[nodiscard]] std::optional<Item> findItem(ItemId id) const noexcept;

// 3. Expected/Result for detailed errors (C++23 or external library)
template<typename T, typename E>
using Result = std::expected<T, E>;

[[nodiscard]] Result<Config, ParseError> parseConfig(std::string_view json);

// 4. Error codes for C interfaces
enum class ErrorCode {
    Success = 0,
    InvalidInput,
    ResourceExhausted,
};

[[nodiscard]] ErrorCode processData(const Data* data, size_t size) noexcept;
```

## Performance Guidelines

### Constexpr Everything Possible

```cpp
class Math {
public:
    // Compile-time computation when possible
    static constexpr double PI = 3.14159265358979323846;

    [[nodiscard]] static constexpr int factorial(int n) noexcept {
        return n <= 1 ? 1 : n * factorial(n - 1);
    }

    [[nodiscard]] static constexpr Point rotate90(Point p) noexcept {
        return {-p.y, p.x};
    }
};
```

### Move Semantics

```cpp
class Buffer {
public:
    // Move constructor should be noexcept
    Buffer(Buffer&& other) noexcept
        : data_{std::exchange(other.data_, nullptr)}
        , size_{std::exchange(other.size_, 0)} {}

    // Move assignment should be noexcept
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    // Enable move semantics for function parameters
    void setData(std::vector<uint8_t> data) {  // Pass by value
        data_ = std::move(data);  // Move into member
    }
};
```

### Small String Optimization (SSO) Awareness

```cpp
class Name {
    // Strings ≤ 15 chars (typically) don't allocate
    std::string name_;

public:
    // Pass small strings by value
    explicit Name(std::string name) : name_{std::move(name)} {}

    // Return by value for SSO
    [[nodiscard]] std::string toString() const { return name_; }
};
```

## Code Examples

### Complete Value Type

```cpp
// point.hpp
#pragma once
#include <compare>
#include <cmath>

class Point final {
public:
    constexpr Point() noexcept = default;
    constexpr Point(int x, int y) noexcept : x_{x}, y_{y} {}

    // Accessors
    [[nodiscard]] constexpr int x() const noexcept { return x_; }
    [[nodiscard]] constexpr int y() const noexcept { return y_; }

    // Comparisons (C++20)
    friend constexpr auto operator<=>(const Point&, const Point&) = default;

    // Arithmetic
    [[nodiscard]] constexpr Point operator+(const Point& other) const noexcept {
        return {x_ + other.x_, y_ + other.y_};
    }

    [[nodiscard]] constexpr Point operator-(const Point& other) const noexcept {
        return {x_ - other.x_, y_ - other.y_};
    }

    [[nodiscard]] constexpr Point operator*(int scalar) const noexcept {
        return {x_ * scalar, y_ * scalar};
    }

    // Distance calculation
    [[nodiscard]] double distanceTo(const Point& other) const noexcept {
        const auto dx = x_ - other.x_;
        const auto dy = y_ - other.y_;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Manhattan distance
    [[nodiscard]] constexpr int manhattanDistanceTo(const Point& other) const noexcept {
        return std::abs(x_ - other.x_) + std::abs(y_ - other.y_);
    }

private:
    int x_{0};
    int y_{0};
};

// Free function for scalar multiplication
[[nodiscard]] constexpr Point operator*(int scalar, const Point& p) noexcept {
    return p * scalar;
}
```

### Clean Interface with NVI Pattern

```cpp
// renderer.hpp
#pragma once
#include <memory>

class Scene;

// Non-Virtual Interface pattern
class Renderer {
public:
    virtual ~Renderer() = default;

    // Public non-virtual interface
    void render(const Scene& scene) {
        preRender();
        doRender(scene);
        postRender();
    }

    [[nodiscard]] bool isReady() const noexcept { return checkReady(); }

protected:
    // Protected virtual implementation
    virtual void preRender() {}
    virtual void doRender(const Scene& scene) = 0;
    virtual void postRender() {}
    virtual bool checkReady() const noexcept { return true; }
};

// Concrete implementation
class OpenGLRenderer final : public Renderer {
protected:
    void doRender(const Scene& scene) override;
    bool checkReady() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;  // Pimpl for ABI stability
};
```

### Strategy Pattern via Composition

```cpp
// movement_strategy.hpp
#pragma once
#include <memory>
#include <variant>

class Entity;
class GameState;

// Strategy interface
class MovementStrategy {
public:
    virtual ~MovementStrategy() = default;
    virtual Point getNextMove(const Entity& entity, const GameState& state) = 0;
};

// Concrete strategies
class RandomMovement final : public MovementStrategy {
public:
    Point getNextMove(const Entity& entity, const GameState& state) override;
};

class PathfindingMovement final : public MovementStrategy {
public:
    explicit PathfindingMovement(Point target);
    Point getNextMove(const Entity& entity, const GameState& state) override;

private:
    Point target_;
};

// Using composition
class Monster {
public:
    explicit Monster(std::unique_ptr<MovementStrategy> strategy)
        : movement_{std::move(strategy)} {}

    void update(const GameState& state) {
        if (movement_) {
            position_ = movement_->getNextMove(*this, state);
        }
    }

    // Change strategy at runtime
    void setMovementStrategy(std::unique_ptr<MovementStrategy> strategy) {
        movement_ = std::move(strategy);
    }

private:
    Point position_;
    std::unique_ptr<MovementStrategy> movement_;
};
```

### Modern Resource Management

```cpp
// texture.hpp
#pragma once
#include <memory>
#include <string>

class Texture final {
public:
    // Factory function instead of constructor
    [[nodiscard]] static std::unique_ptr<Texture> load(const std::string& path);

    // Delete copy operations
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Enable move operations
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    ~Texture();

    // Getters
    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] uint32_t id() const noexcept { return id_; }

private:
    // Private constructor - use factory
    Texture(uint32_t id, uint32_t width, uint32_t height);

    uint32_t id_{0};
    uint32_t width_{0};
    uint32_t height_{0};
};

// Usage
auto texture = Texture::load("player.png");
if (!texture) {
    // Handle error
}
```

## Testing Guidelines

### Testable Design

```cpp
// Dependency injection for testability
class GameEngine {
public:
    // Inject dependencies
    explicit GameEngine(
        std::unique_ptr<Renderer> renderer,
        std::unique_ptr<AudioSystem> audio,
        std::unique_ptr<InputHandler> input)
        : renderer_{std::move(renderer)}
        , audio_{std::move(audio)}
        , input_{std::move(input)} {}

    // Testable with mock implementations
    void update(double delta_time);

private:
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<AudioSystem> audio_;
    std::unique_ptr<InputHandler> input_;
};

// In tests
auto engine = GameEngine{
    std::make_unique<MockRenderer>(),
    std::make_unique<MockAudioSystem>(),
    std::make_unique<MockInputHandler>()
};
```

## Common Patterns

### Builder Pattern

```cpp
class ConfigBuilder {
public:
    ConfigBuilder& withResolution(int width, int height) & {
        width_ = width;
        height_ = height;
        return *this;
    }

    ConfigBuilder&& withResolution(int width, int height) && {
        width_ = width;
        height_ = height;
        return std::move(*this);
    }

    ConfigBuilder& withFullscreen(bool fullscreen) & {
        fullscreen_ = fullscreen;
        return *this;
    }

    [[nodiscard]] Config build() const {
        return Config{width_, height_, fullscreen_};
    }

private:
    int width_{1920};
    int height_{1080};
    bool fullscreen_{false};
};

// Usage
auto config = ConfigBuilder{}
    .withResolution(2560, 1440)
    .withFullscreen(true)
    .build();
```

### Observer Pattern (Modern)

```cpp
#include <functional>
#include <vector>

template<typename... Args>
class Signal {
public:
    using Handler = std::function<void(Args...)>;
    using HandlerId = size_t;

    [[nodiscard]] HandlerId connect(Handler handler) {
        handlers_.emplace_back(next_id_, std::move(handler));
        return next_id_++;
    }

    void disconnect(HandlerId id) {
        std::erase_if(handlers_, [id](const auto& h) { return h.first == id; });
    }

    void emit(Args... args) const {
        for (const auto& [id, handler] : handlers_) {
            handler(args...);
        }
    }

private:
    mutable std::vector<std::pair<HandlerId, Handler>> handlers_;
    HandlerId next_id_{0};
};

// Usage
class Player {
public:
    Signal<int> onHealthChanged;

    void takeDamage(int amount) {
        health_ -= amount;
        onHealthChanged.emit(health_);
    }

private:
    int health_{100};
};
```

## Anti-Patterns to Avoid

### 1. God Classes

```cpp
// Bad: Class doing too much
class Game {
    void handleInput();
    void updatePhysics();
    void renderGraphics();
    void playSound();
    void saveGame();
    void loadGame();
    // ... 500 more methods
};

// Good: Separate responsibilities
class Game {
    InputHandler input_;
    PhysicsEngine physics_;
    Renderer renderer_;
    AudioSystem audio_;
    SaveSystem saves_;
};
```

### 2. Primitive Obsession

```cpp
// Bad: Using primitives everywhere
void transfer(int from_account, int to_account, double amount);

// Good: Domain types
void transfer(AccountId from, AccountId to, Money amount);
```

### 3. Deep Inheritance Hierarchies

```cpp
// Bad: Deep hierarchy
class Entity { };
class LivingEntity : public Entity { };
class Creature : public LivingEntity { };
class Humanoid : public Creature { };
class Player : public Humanoid { };

// Good: Composition
class Player {
    Position position_;
    Health health_;
    Inventory inventory_;
    std::unique_ptr<MovementBehavior> movement_;
};
```

## Refactoring Checklist

When refactoring existing code:

- [ ] Add `explicit` to single-argument constructors
- [ ] Add `[[nodiscard]]` to getters and factory functions
- [ ] Add `noexcept` to move operations and non-throwing functions
- [ ] Add `const` wherever possible
- [ ] Add `final` to leaf classes
- [ ] Add `override` to virtual function overrides
- [ ] Replace raw pointers with smart pointers
- [ ] Replace `typedef` with `using`
- [ ] Replace macros with templates/constexpr
- [ ] Consider replacing inheritance with composition
- [ ] Consider replacing virtual functions with templates
- [ ] Ensure RAII for all resources
- [ ] Minimize header dependencies
- [ ] Use forward declarations where possible

## References

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Effective Modern C++ by Scott Meyers](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)
- [C++ Best Practices by Jason Turner](https://github.com/cpp-best-practices/cppbestpractices)

---

*This document is a living guide and should be updated as the codebase evolves and new best practices emerge.*
