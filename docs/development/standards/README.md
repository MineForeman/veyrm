# Development Standards

Coding standards and best practices for Veyrm development.

## Quick Reference

1. [Code Style Guide](code-style.md) - C++ coding conventions
2. [Git Workflow](git-workflow.md) - Version control practices
3. [Testing Standards](testing.md) - Test requirements
4. [Documentation Standards](documentation.md) - Documentation guidelines

## Overview

These standards ensure consistency, quality, and maintainability across the Veyrm codebase.

### Core Principles
- **Consistency** - Follow established patterns
- **Clarity** - Write readable, self-documenting code
- **Quality** - Maintain high standards
- **Testing** - Test all new features
- **Documentation** - Document public APIs

## Code Standards

### Language Version
- C++23 with modern features
- Prefer standard library over custom implementations
- Use smart pointers for memory management

### Style Overview
```cpp
class ExampleClass {
public:
    void doSomething();

private:
    int member_variable_;
    static constexpr int CONSTANT = 42;
};
```

See [Code Style Guide](code-style.md) for complete guidelines.

## Git Standards

### Branch Naming
- `feature/description` - New features
- `bugfix/description` - Bug fixes
- `refactor/description` - Code refactoring
- `docs/description` - Documentation

### Commit Messages
```
type: Brief description

Detailed explanation if needed.

Fixes #123
```

See [Git Workflow](git-workflow.md) for details.

## Testing Standards

### Requirements
- All new features must have tests
- Maintain >90% code coverage
- Tests must pass before merge
- Use Catch2 framework

### Test Structure
```cpp
TEST_CASE("Component: What is tested", "[category]") {
    SECTION("Specific scenario") {
        // Arrange
        // Act
        // Assert
    }
}
```

See [Testing Standards](testing.md) for complete guide.

## Documentation Standards

### Requirements
- Document all public APIs
- Include usage examples
- Keep documentation current
- Use Markdown format

### Documentation Types
- **Code Comments** - Explain why, not what
- **API Documentation** - Doxygen format
- **User Documentation** - Markdown files
- **Developer Guides** - How-to guides

See [Documentation Standards](documentation.md) for guidelines.

## Review Checklist

Before submitting code:

### Code Quality
- [ ] Follows code style guide
- [ ] No compiler warnings
- [ ] Memory leaks checked
- [ ] Error cases handled

### Testing
- [ ] Tests written
- [ ] Tests passing
- [ ] Coverage maintained
- [ ] Edge cases tested

### Documentation
- [ ] API documented
- [ ] Examples provided
- [ ] README updated
- [ ] Changelog updated

### Git
- [ ] Clean commit history
- [ ] Descriptive messages
- [ ] Branch up to date
- [ ] No merge conflicts

## Tools

### Formatting
```bash
clang-format -i src/*.cpp
```

### Linting
```bash
clang-tidy src/*.cpp
cppcheck --enable=all src/
```

### Testing
```bash
./build.sh test
```

## Enforcement

Standards are enforced through:
1. Code review process
2. Automated CI checks
3. Pre-commit hooks
4. Team agreement

## Updates

Standards evolve with the project:
1. Propose changes via RFC
2. Discuss with team
3. Update documentation
4. Communicate changes

---

*Continue to: [Code Style Guide](code-style.md) →*