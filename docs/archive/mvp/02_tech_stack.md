# 2) Tech Choices

- **TUI:** [FTXUI] (clean layout, Unicode, color, reactive rendering).
- **Build:** CMake ≥ 3.25; C++23; dependency via FetchContent or vcpkg.
- **JSON:** nlohmann/json for save-games and content tables.
- **RNG:** std::mt19937_64 (swap to PCG later if needed).
- **Tests:** Catch2 for smoke tests (map connectivity, FOV symmetry, combat averages).
- **CI:** GitHub Actions building on macOS/Linux/Windows.

[FTXUI]: https://github.com/ArthurSonzogni/FTXUI
