# 8) Build & Run

## CMake quickstart

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./ang
```

## CMakeLists.txt (excerpt)

```cmake
cmake_minimum_required(VERSION 3.25)
project(ang-modern LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_executable(ang src/main.cpp)
target_link_libraries(ang PRIVATE ftxui::component ftxui::dom ftxui::screen nlohmann_json::nlohmann_json)
target_compile_definitions(ang PRIVATE FTXUI_MICROSOFT_TERMINAL_FALLBACK_COLORS)
```

## Notes

- Use UTF-8 locale in your terminal.
- If resizing, keep at least 80×24 for the default viewport.

```
