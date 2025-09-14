# Developer Guides

Comprehensive guides for Veyrm developers, from setup to advanced topics.

## Getting Started

1. [Development Setup](setup.md) - Set up your development environment
2. [Architecture Overview](architecture.md) - Understand the system design
3. [Your First Contribution](contributing.md) - Make your first pull request

## Core Guides

### Essential
- [Development Setup](setup.md) - Environment and tools
- [Architecture](architecture.md) - System design and components
- [Build System](../../reference/commands/build-script.md) - Using build.sh
- [Testing Guide](testing.md) - Writing and running tests
- [Debugging](debugging.md) - Debugging techniques

### Development Workflow
- [Contributing](contributing.md) - Contribution guidelines
- [Git Workflow](../../development/standards/git-workflow.md) - Branching and commits
- [Code Review](code-review.md) - Review process
- [Release Process](release.md) - Creating releases

### System Guides
- [Entity System](systems/entities.md) - Working with entities
- [Map System](systems/maps.md) - Map generation and manipulation
- [Combat System](systems/combat.md) - Combat mechanics
- [AI System](systems/ai.md) - Monster AI
- [Save System](systems/save-load.md) - Persistence

## Advanced Topics

### Performance
- [Profiling](../../development/performance/profiling.md) - Performance analysis
- [Optimization](../../development/performance/optimization.md) - Optimization techniques
- [Benchmarks](../../development/performance/benchmarks.md) - Performance metrics

### Architecture
- [Design Patterns](patterns.md) - Patterns used in Veyrm
- [Memory Management](memory.md) - Smart pointers and RAII
- [Threading](threading.md) - Future: Multithreading

## Quick Reference

### Common Tasks
- [Adding a Monster](../content/monsters.md)
- [Adding an Item](../content/items.md)
- [Creating a Map](../content/maps.md)
- [Adding a Command](commands.md)
- [Implementing a System](systems.md)

### Debugging
- Enable debug mode: `--debug` flag
- Check logs: `logs/veyrm_debug.log`
- Run tests: `./build.sh test`
- Profile: See [Profiling Guide](../../development/performance/profiling.md)

## Best Practices

### Code Quality
- Follow [Code Style Guide](../../development/standards/code-style.md)
- Write tests for new features
- Document public APIs
- Handle errors gracefully

### Development Process
1. Create feature branch
2. Write tests first (TDD)
3. Implement feature
4. Update documentation
5. Submit pull request

## Resources

### Internal
- [API Reference](../../reference/api/README.md)
- [Configuration](../../reference/configuration/README.md)
- [Standards](../../development/standards/README.md)

### External
- [FTXUI Documentation](https://github.com/ArthurSonzogni/FTXUI)
- [Catch2 Documentation](https://github.com/catchorg/Catch2)
- [CMake Documentation](https://cmake.org/documentation/)

## Getting Help

- Check existing documentation
- Search the codebase
- Ask in GitHub Issues
- Review test cases

---

*Start with: [Development Setup](setup.md) →*