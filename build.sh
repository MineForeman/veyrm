#!/bin/bash
# build.sh - Build helper script for Veyrm

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Debug"
PROJECT_ROOT=$(dirname "$(realpath "$0")")
BUILD_DIR="${PROJECT_ROOT}/build"
EXECUTABLE="${BUILD_DIR}/bin/veyrm"
TEST_EXECUTABLE="${BUILD_DIR}/bin/veyrm_tests"

# Function to print header
print_header() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Veyrm Build System${NC}"
    echo -e "${CYAN}=========================================${NC}"
}

# Function to reset terminal
reset_terminal() {
    echo -e "${YELLOW}Resetting terminal...${NC}"
    printf '\033[?1003l\033[?1006l\033[?1015l\033[?1000l\033[?25h\033c'
    stty sane 2>/dev/null || true
    echo -e "${GREEN}Terminal reset complete${NC}"
}

# Function to clean build
clean_build() {
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "${BUILD_DIR}"
    echo -e "${GREEN}Clean complete${NC}"
}

# Function to clear logs
clear_logs() {
    echo -e "${YELLOW}Clearing log files...${NC}"
    rm -rf "${PROJECT_ROOT}/logs"
    rm -f "${PROJECT_ROOT}/"*.log
    echo -e "${GREEN}Logs cleared${NC}"
}

# Function to generate visual class diagram
class_diagram() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Visual Class Diagram Generator${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Check if graphviz is installed
    if ! command -v dot &> /dev/null; then
        echo -e "${RED}Error: graphviz is not installed${NC}"
        echo -e "${YELLOW}Install with: brew install graphviz${NC}"
        exit 1
    fi

    # Create tmp directory if it doesn't exist
    mkdir -p "${PROJECT_ROOT}/tmp"

    local dot_file="${PROJECT_ROOT}/tmp/veyrm_classes.dot"
    local output_svg="${PROJECT_ROOT}/tmp/veyrm_classes.svg"
    local output_png="${PROJECT_ROOT}/tmp/veyrm_classes.png"

    echo -e "${YELLOW}Analyzing C++ classes...${NC}"

    # Create DOT file with class relationships
    cat > "${dot_file}" << 'EOF'
digraph VeyrmClasses {
    // Graph settings
    rankdir=TB;
    node [shape=record, style=filled, fillcolor=lightyellow, fontname="Arial"];
    edge [fontname="Arial", fontsize=10];

    // Title
    labelloc="t";
    label="Veyrm Class Hierarchy";
    fontsize=20;
    fontname="Arial Bold";

    // Core Game Classes
    subgraph cluster_core {
        label="Core Systems";
        style=filled;
        color=lightgrey;

        Game [label="{Game|+run()\l+init()\l+shutdown()\l}", fillcolor=lightblue];
        GameScreen [label="{GameScreen|+render()\l+handleInput()\l+update()\l}"];
        MainMenuScreen [label="{MainMenuScreen|+render()\l+handleInput()\l}"];
        Screen [label="{«interface»\nScreen|+render()\l+handleInput()\l}", fillcolor=lightgreen];
    }

    // Entity System
    subgraph cluster_entities {
        label="Entity System";
        style=filled;
        color=lightgrey;

        Entity [label="{Entity|#x, y: int\l#hp, max_hp: int\l#symbol: char\l|+move()\l+takeDamage()\l}", fillcolor=lightcoral];
        Player [label="{Player|+attack: int\l+defense: int\l+exp: int\l|+gainExp()\l+levelUp()\l}"];
        Monster [label="{Monster|+threat_level: char\l+exp_reward: int\l+aggressive: bool\l|+dropLoot()\l}"];
        EntityManager [label="{EntityManager|+entities: vector\l|+spawn()\l+remove()\l+update()\l}"];
    }

    // Map System
    subgraph cluster_map {
        label="Map System";
        style=filled;
        color=lightgrey;

        Map [label="{Map|+tiles: vector\l+width, height: int\l|+getTile()\l+setTile()\l+isValid()\l}"];
        MapGenerator [label="{MapGenerator|+rooms: vector\l|+generate()\l+placeRooms()\l+connectRooms()\l}"];
        MapValidator [label="{MapValidator|+validate()\l+findPath()\l}"];
        Room [label="{Room|+x, y: int\l+width, height: int\l+lit: bool\l}"];
    }

    // Combat System
    subgraph cluster_combat {
        label="Combat System";
        style=filled;
        color=lightgrey;

        CombatSystem [label="{CombatSystem|+rollDice()\l+attack()\l+calculateDamage()\l+criticalHit()\l}", fillcolor=pink];
        CombatStats [label="{CombatStats|+hp: int\l+attack: int\l+defense: int\l+damage: string\l}"];
    }

    // AI System
    subgraph cluster_ai {
        label="AI System";
        style=filled;
        color=lightgrey;

        MonsterAI [label="{MonsterAI|+state: AIState\l|+update()\l+patrol()\l+chase()\l+flee()\l}", fillcolor=lightsteelblue];
        Pathfinder [label="{Pathfinder|+findPath()\l+aStar()\l}"];
        AIState [label="{«enum»\nAIState|IDLE\lALERT\lHOSTILE\lFLEEING\lRETURNING\l}", fillcolor=lightgreen];
    }

    // Utility Systems
    subgraph cluster_utils {
        label="Utilities";
        style=filled;
        color=lightgrey;

        FOV [label="{FOV|+calculate()\l+isVisible()\l+shadowcast()\l}"];
        Config [label="{Config|+instance: Config\l|+getInstance()\l+getValue()\l}", fillcolor=yellow];
        Log [label="{Log|+debug()\l+info()\l+error()\l+combat()\l}"];
        MessageLog [label="{MessageLog|+messages: deque\l|+addMessage()\l+render()\l}"];
        SpawnManager [label="{SpawnManager|+spawnMonster()\l+getSpawnLocation()\l}"];
        MonsterFactory [label="{MonsterFactory|+templates: map\l|+create()\l+loadTemplates()\l}"];
    }

    // Inheritance relationships
    Player -> Entity [label="inherits", style=solid, arrowhead=empty];
    Monster -> Entity [label="inherits", style=solid, arrowhead=empty];
    GameScreen -> Screen [label="implements", style=dashed, arrowhead=empty];
    MainMenuScreen -> Screen [label="implements", style=dashed, arrowhead=empty];

    // Composition relationships
    Game -> GameScreen [label="contains", style=solid, arrowhead=diamond];
    Game -> MainMenuScreen [label="contains", style=solid, arrowhead=diamond];
    GameScreen -> Map [label="uses", style=dashed];
    GameScreen -> Player [label="has", style=solid, arrowhead=diamond];
    GameScreen -> EntityManager [label="has", style=solid, arrowhead=diamond];
    GameScreen -> MessageLog [label="has", style=solid, arrowhead=diamond];
    GameScreen -> FOV [label="uses", style=dashed];

    EntityManager -> Monster [label="manages", style=solid, arrowhead=vee];
    EntityManager -> SpawnManager [label="uses", style=dashed, constraint=false];

    MapGenerator -> Room [label="creates", style=dashed, arrowhead=vee];
    MapGenerator -> Map [label="generates", style=dashed, arrowhead=vee];
    MapGenerator -> MapValidator [label="uses", style=dashed];

    Monster -> MonsterAI [label="has", style=solid, arrowhead=diamond];
    Monster -> CombatStats [label="has", style=solid, arrowhead=diamond];
    Player -> CombatStats [label="has", style=solid, arrowhead=diamond];

    MonsterAI -> Pathfinder [label="uses", style=dashed];
    MonsterAI -> AIState [label="has state", style=solid];

    GameScreen -> CombatSystem [label="uses", style=dashed];
    CombatSystem -> Entity [label="operates on", style=dashed];

    SpawnManager -> MonsterFactory [label="uses", style=dashed];
    MonsterFactory -> Monster [label="creates", style=dashed, arrowhead=vee];

    // Singleton pattern
    Config -> Config [label="singleton", style=dotted];
    Log -> Log [label="singleton", style=dotted];
}
EOF

    echo -e "${YELLOW}Generating visual diagram...${NC}"

    # Generate SVG
    dot -Tsvg "${dot_file}" -o "${output_svg}" 2>/dev/null

    # Generate PNG for better compatibility
    dot -Tpng "${dot_file}" -o "${output_png}" -Gdpi=150 2>/dev/null

    if [ $? -eq 0 ]; then
        echo
        echo -e "${GREEN}Class diagram generated successfully!${NC}"
        echo -e "${CYAN}Files created:${NC}"
        echo -e "  SVG: ${output_svg}"
        echo -e "  PNG: ${output_png}"
        echo

        # Open the diagram
        if [[ "$OSTYPE" == "darwin"* ]]; then
            echo -e "${YELLOW}Opening diagram...${NC}"
            open "${output_svg}"
        else
            echo -e "${YELLOW}Open in your image viewer:${NC}"
            echo "  ${output_svg}"
        fi

        echo
        echo -e "${YELLOW}Note: Diagrams are in tmp/ directory which is gitignored${NC}"
    else
        echo -e "${RED}Failed to generate class diagram${NC}"
        exit 1
    fi
}

# Function to generate class diagram with Doxygen+Graphviz (old version)
class_diagram_doxygen() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Class Diagram Generator${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Check if doxygen is installed
    if ! command -v doxygen &> /dev/null; then
        echo -e "${RED}Error: doxygen is not installed${NC}"
        echo -e "${YELLOW}Install with: brew install doxygen${NC}"
        exit 1
    fi

    # Check if graphviz is installed
    if ! command -v dot &> /dev/null; then
        echo -e "${RED}Error: graphviz is not installed${NC}"
        echo -e "${YELLOW}Install with: brew install graphviz${NC}"
        exit 1
    fi

    # Create tmp directory if it doesn't exist
    mkdir -p "${PROJECT_ROOT}/tmp"

    local output_dir="${PROJECT_ROOT}/tmp/doxygen"
    rm -rf "${output_dir}"
    mkdir -p "${output_dir}"

    echo -e "${YELLOW}Creating Doxyfile configuration...${NC}"

    # Create optimized Doxyfile for class diagrams
    cat > "${PROJECT_ROOT}/tmp/Doxyfile" << EOF
# Doxygen configuration for Veyrm class diagrams

# Project settings
PROJECT_NAME           = "Veyrm"
PROJECT_BRIEF          = "Modern Roguelike Game"
OUTPUT_DIRECTORY       = ${output_dir}

# Input settings
INPUT                  = ${PROJECT_ROOT}/include ${PROJECT_ROOT}/src
FILE_PATTERNS          = *.h *.cpp
RECURSIVE              = YES

# Extraction settings
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = YES
EXTRACT_STATIC         = YES
EXTRACT_LOCAL_CLASSES  = YES

# Output settings
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
GENERATE_XML           = NO

# Graph settings
HAVE_DOT               = YES
DOT_NUM_THREADS        = 0
CLASS_DIAGRAMS         = YES
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
UML_LOOK               = YES
UML_LIMIT_NUM_FIELDS   = 50
TEMPLATE_RELATIONS     = YES
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
CALL_GRAPH             = NO
CALLER_GRAPH           = NO
GRAPHICAL_HIERARCHY    = YES
DIRECTORY_GRAPH        = YES

# Graph appearance
DOT_IMAGE_FORMAT       = svg
INTERACTIVE_SVG        = YES
DOT_GRAPH_MAX_NODES    = 100
MAX_DOT_GRAPH_DEPTH    = 0
DOT_TRANSPARENT        = YES

# HTML settings
HTML_OUTPUT            = html
HTML_FILE_EXTENSION    = .html
HTML_COLORSTYLE_HUE    = 220
HTML_COLORSTYLE_SAT    = 100
HTML_COLORSTYLE_GAMMA  = 80
HTML_TIMESTAMP         = YES
HTML_DYNAMIC_SECTIONS  = YES
HTML_INDEX_NUM_ENTRIES = 100

# Quiet output
QUIET                  = YES
WARNINGS               = NO
WARN_IF_UNDOCUMENTED   = NO
WARN_IF_DOC_ERROR      = NO
EOF

    echo -e "${YELLOW}Generating documentation and class diagrams...${NC}"
    cd "${PROJECT_ROOT}"
    doxygen tmp/Doxyfile 2>&1 | grep -v "warning:" | grep -v "Notice:"

    if [ $? -eq 0 ]; then
        echo
        echo -e "${GREEN}Class diagrams generated successfully!${NC}"
        echo -e "${CYAN}Output directory: ${output_dir}/html${NC}"
        echo
        echo -e "${YELLOW}Main files of interest:${NC}"
        echo -e "  ${CYAN}Class hierarchy:${NC} ${output_dir}/html/inherits.html"
        echo -e "  ${CYAN}Class list:${NC} ${output_dir}/html/annotated.html"
        echo -e "  ${CYAN}File list:${NC} ${output_dir}/html/files.html"
        echo

        # Open in browser if on macOS
        if [[ "$OSTYPE" == "darwin"* ]]; then
            echo -e "${YELLOW}Opening in browser...${NC}"
            open "${output_dir}/html/index.html"
        else
            echo -e "${YELLOW}Open in browser:${NC}"
            echo "  file://${output_dir}/html/index.html"
        fi

        echo
        echo -e "${YELLOW}Note: Documentation is in tmp/ directory which is gitignored${NC}"
    else
        echo -e "${RED}Failed to generate class diagrams${NC}"
        exit 1
    fi
}

# Function to create Gource visualization video
gource_video() {
    # Parse arguments
    local delete_old=false
    if [ "$1" == "--clean" ] || [ "$1" == "-c" ]; then
        delete_old=true
    fi

    # Create tmp directory in repository if it doesn't exist
    mkdir -p "${PROJECT_ROOT}/tmp"

    # Delete old videos if requested
    if [ "$delete_old" = true ]; then
        echo -e "${YELLOW}Removing old Gource videos...${NC}"
        rm -f "${PROJECT_ROOT}/tmp/"*.mp4
    fi

    # Use fixed filename (will overwrite)
    local video_file="${PROJECT_ROOT}/tmp/veyrm-gource.mp4"

    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Gource Visualization${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Check if gource is installed
    if ! command -v gource &> /dev/null; then
        echo -e "${RED}Error: gource is not installed${NC}"
        echo -e "${YELLOW}Install with: brew install gource (macOS) or apt install gource (Linux)${NC}"
        exit 1
    fi

    # Check if ffmpeg is installed (needed for video output)
    if ! command -v ffmpeg &> /dev/null; then
        echo -e "${RED}Error: ffmpeg is not installed${NC}"
        echo -e "${YELLOW}Install with: brew install ffmpeg (macOS) or apt install ffmpeg (Linux)${NC}"
        exit 1
    fi

    echo -e "${YELLOW}Creating Gource visualization...${NC}"
    echo -e "${YELLOW}Output: ${video_file}${NC}"
    echo

    # Gource settings for a nice visualization
    gource \
        --title "Veyrm Development History" \
        --seconds-per-day 2.0 \
        --auto-skip-seconds 0.5 \
        --file-idle-time 60 \
        --max-file-lag 0.5 \
        --bloom-multiplier 0.5 \
        --bloom-intensity 0.5 \
        --highlight-users \
        --highlight-dirs \
        --multi-sampling \
        --stop-at-end \
        --hide mouse,progress \
        --font-size 18 \
        --date-format "%B %d, %Y" \
        --dir-name-depth 2 \
        --filename-time 3 \
        --start-date "2025-01-01" \
        --time-scale 1.5 \
        --max-files 0 \
        --background-colour 000000 \
        --font-colour FFFFFF \
        --dir-colour 888888 \
        --file-extensions \
        --output-framerate 30 \
        --output-ppm-stream - \
        "${PROJECT_ROOT}" | \
    ffmpeg -y -r 30 -f image2pipe -vcodec ppm -i - \
        -vcodec libx264 -preset medium -pix_fmt yuv420p -crf 23 \
        -threads 0 "${video_file}"

    if [ $? -eq 0 ]; then
        echo
        echo -e "${GREEN}Gource video created successfully!${NC}"
        echo -e "${CYAN}Video saved to: ${video_file}${NC}"
        echo -e "${CYAN}File size: $(du -h "${video_file}" | cut -f1)${NC}"
        echo
        echo -e "${YELLOW}Note: Video is in tmp/ directory which is gitignored${NC}"
        echo -e "${YELLOW}The file will be overwritten on next run${NC}"
        echo
        echo -e "${CYAN}Use --clean to delete old videos before creating new one${NC}"
    else
        echo -e "${RED}Failed to create Gource video${NC}"
        exit 1
    fi
}

# Function to configure project
configure_project() {
    local build_type=${1:-Debug}
    echo -e "${YELLOW}Configuring CMake (${build_type})...${NC}"
    mkdir -p "${BUILD_DIR}"
    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=${build_type}
}

# Function to build project
build_project() {
    echo -e "${YELLOW}Building project...${NC}"
    cmake --build "${BUILD_DIR}" --parallel $(nproc 2>/dev/null || echo 4)
    echo -e "${GREEN}Build complete!${NC}"
}

# Function to run tests
run_tests() {
    echo -e "${YELLOW}Running tests...${NC}"
    if [ -f "${TEST_EXECUTABLE}" ]; then
        # Run tests from project root
        cd "${PROJECT_ROOT}"
        "${TEST_EXECUTABLE}" || echo -e "${RED}Some tests failed${NC}"
        cd - > /dev/null
    else
        echo -e "${RED}Test executable not found. Build first.${NC}"
    fi
}

# Function to run dump mode test
run_dump_test() {
    # Don't interpret escape sequences - pass them as-is
    local keystrokes="${1:-\n\u\u\r\r\d\l}"
    echo -e "${YELLOW}Running dump mode test...${NC}"
    echo -e "${CYAN}Keystrokes: ${keystrokes}${NC}"
    if [ -f "${EXECUTABLE}" ]; then
        # Run from project root
        cd "${PROJECT_ROOT}"
        "${EXECUTABLE}" --dump "${keystrokes}"
        cd - > /dev/null
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to run with keys
run_with_keys() {
    local keystrokes="${1}"
    echo -e "${YELLOW}Running game with automated keys...${NC}"
    echo -e "${CYAN}Keystrokes: ${keystrokes}${NC}"
    if [ -f "${EXECUTABLE}" ]; then
        # Run from project root
        cd "${PROJECT_ROOT}"
        "${EXECUTABLE}" --keys "${keystrokes}"
        cd - > /dev/null
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to run the game
run_game() {
    if [ -f "${EXECUTABLE}" ]; then
        echo -e "${YELLOW}Starting Veyrm...${NC}"
        echo
        # Ask for map type
        echo -e "${CYAN}Select map type:${NC}"
        echo -e "${BLUE}1)${NC} Procedural Dungeon ${GREEN}(default)${NC} - Random room generation"
        echo -e "${BLUE}2)${NC} Test Dungeon - Fixed 5-room layout"
        echo -e "${BLUE}3)${NC} Test Room - Single 20x20 room"
        echo -e "${BLUE}4)${NC} Corridor Test - Long corridors"
        echo -e "${BLUE}5)${NC} Combat Arena - Open space"
        echo -e "${BLUE}6)${NC} Stress Test - Large map with many rooms"
        echo
        read -p "Press Enter for Procedural, or select (1-6): " map_choice
        
        # Run from project root
        cd "${PROJECT_ROOT}"
        case "${map_choice}" in
            1|"")
                echo -e "${GREEN}Loading Procedural Dungeon...${NC}"
                "${EXECUTABLE}" --map procedural
                ;;
            2)
                echo -e "${GREEN}Loading Test Dungeon...${NC}"
                "${EXECUTABLE}" --map dungeon
                ;;
            3)
                echo -e "${GREEN}Loading Test Room...${NC}"
                "${EXECUTABLE}" --map room
                ;;
            4)
                echo -e "${GREEN}Loading Corridor Test...${NC}"
                "${EXECUTABLE}" --map corridor
                ;;
            5)
                echo -e "${GREEN}Loading Combat Arena...${NC}"
                "${EXECUTABLE}" --map arena
                ;;
            6)
                echo -e "${GREEN}Loading Stress Test...${NC}"
                "${EXECUTABLE}" --map stress
                ;;
            *)
                echo -e "${RED}Invalid selection. Loading default...${NC}"
                "${EXECUTABLE}" --map procedural
                ;;
        esac
        cd - > /dev/null
        reset_terminal  # Auto-reset terminal after game exits
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to run with arguments
run_with_args() {
    if [ -f "${EXECUTABLE}" ]; then
        # Run from project root
        cd "${PROJECT_ROOT}"
        "${EXECUTABLE}" "$@"
        cd - > /dev/null
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to get binary stats
get_binary_stats() {
    local stats=""
    
    # Check if binary exists
    if [ -f "${EXECUTABLE}" ]; then
        # Get file size in human readable format
        local size=$(ls -lh "${EXECUTABLE}" | awk '{print $5}')
        # Get last modified time
        local modified=$(date -r "${EXECUTABLE}" "+%Y-%m-%d %H:%M:%S" 2>/dev/null || stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "${EXECUTABLE}" 2>/dev/null || echo "unknown")
        stats="${GREEN}✓${NC} Binary: ${size} | Built: ${modified}"
    else
        stats="${RED}✗${NC} Binary not built"
    fi
    
    # Check test status
    if [ -f "${TEST_EXECUTABLE}" ]; then
        local test_size=$(ls -lh "${TEST_EXECUTABLE}" | awk '{print $5}')
        stats="${stats}\n${GREEN}✓${NC} Tests: ${test_size}"
    else
        stats="${stats}\n${RED}✗${NC} Tests not built"
    fi
    
    # Check build type from CMakeCache if it exists
    if [ -f "${BUILD_DIR}/CMakeCache.txt" ]; then
        local build_type=$(grep CMAKE_BUILD_TYPE "${BUILD_DIR}/CMakeCache.txt" | cut -d= -f2)
        stats="${stats} | Mode: ${YELLOW}${build_type}${NC}"
    fi
    
    echo -e "${stats}"
}

# Function to show menu
show_menu() {
    echo
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    echo -e "${BOLD}                    Build Status${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    get_binary_stats
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    echo
    echo -e "${BOLD}Main Menu:${NC}"
    echo -e "${BLUE}1)${NC} Build (Debug)"
    echo -e "${BLUE}2)${NC} Build (Release)"
    echo -e "${BLUE}3)${NC} Clean Build (Debug)"
    echo -e "${BLUE}4)${NC} Clean Build (Release)"
    echo -e "${BLUE}5)${NC} Run Game"
    echo -e "${BLUE}6)${NC} Run Tests"
    echo -e "${BLUE}7)${NC} Run System Check"
    echo -e "${BLUE}8)${NC} Run Dump Mode Test"
    echo -e "${BLUE}9)${NC} Clean"
    echo -e "${BLUE}10)${NC} Reset Terminal"
    echo -e "${BLUE}11)${NC} Clear Logs"
    echo -e "${BLUE}12)${NC} Create Gource Video"
    echo -e "${BLUE}13)${NC} Generate Class Diagrams"
    echo -e "${BLUE}0)${NC} Exit"
    echo
}

# Function to show help
show_help() {
    print_header
    echo -e "${BOLD}Usage:${NC} $0 [COMMAND] [OPTIONS]"
    echo
    echo -e "${BOLD}Commands:${NC}"
    echo "  build [debug|release]  Build the project"
    echo "  clean                  Clean build directory"
    echo "  run [map_type]         Run the game (optionally specify map)"
    echo "  test                   Run tests"
    echo "  dump [keystrokes]      Run dump mode test (frame-by-frame)"
    echo "  keys <keystrokes>      Run game with automated keys"
    echo "  check                  Run system checks"
    echo "  reset                  Reset terminal"
    echo "  clearlog               Clear all log files"
    echo "  gource [--clean]       Create Gource video (--clean deletes old videos)"
    echo "  diagram                Generate class diagrams with Doxygen"
    echo "  docs                   Generate API documentation with Doxygen"
    echo "  menu                   Show interactive menu (default)"
    echo "  help                   Show this help"
    echo
    echo -e "${BOLD}Examples:${NC}"
    echo "  $0                     # Show interactive menu"
    echo "  $0 build              # Build in debug mode"
    echo "  $0 build release      # Build in release mode"
    echo "  $0 clean build        # Clean and build"
    echo "  $0 run                # Run the game (interactive map selection)"
    echo "  $0 run dungeon        # Run with dungeon map"
    echo "  $0 run arena          # Run with arena map"
    echo "  $0 test               # Run tests"
    echo "  $0 dump               # Run dump test with default keys"
    echo "  $0 dump '\\n\\u\\r'      # Run dump test with custom keys"
    echo "  $0 keys '\\njjjq'      # Run game with automated keys"
    echo "  $0 gource             # Create Gource video"
    echo "  $0 reset              # Reset terminal"
    echo
}

# Main script logic
main() {
    # Handle command-line arguments
    case "${1:-menu}" in
        build)
            print_header
            BUILD_TYPE="${2:-Debug}"
            [[ "${BUILD_TYPE}" == "release" ]] && BUILD_TYPE="Release"
            [[ "${BUILD_TYPE}" == "debug" ]] && BUILD_TYPE="Debug"
            configure_project "${BUILD_TYPE}"
            build_project
            ;;
        clean)
            print_header
            clean_build
            if [ "${2}" == "build" ]; then
                BUILD_TYPE="${3:-Debug}"
                configure_project "${BUILD_TYPE}"
                build_project
            fi
            ;;
        run)
            print_header
            # Check if a map type was specified
            if [ -n "${2}" ]; then
                case "${2}" in
                    procedural|room|dungeon|corridor|arena|stress)
                        run_with_args --map "${2}"
                        reset_terminal
                        ;;
                    *)
                        echo -e "${RED}Unknown map type: ${2}${NC}"
                        echo -e "Valid types: procedural, room, dungeon, corridor, arena, stress"
                        ;;
                esac
            else
                run_game
            fi
            ;;
        test)
            print_header
            run_tests
            ;;
        dump)
            print_header
            # Pass the keystrokes without interpreting escapes
            run_dump_test "$2"
            ;;
        keys)
            if [ -z "$2" ]; then
                echo -e "${RED}Error: keystrokes required${NC}"
                echo "Usage: $0 keys <keystrokes>"
                echo "Example: $0 keys '\\njjjq'"
                exit 1
            fi
            print_header
            run_with_keys "$2"
            reset_terminal
            ;;
        check)
            print_header
            run_with_args --test
            ;;
        reset)
            reset_terminal
            ;;
        clearlog)
            clear_logs
            ;;
        gource)
            gource_video "$2"
            ;;
        diagram)
            class_diagram
            ;;
        docs)
            print_header
            echo -e "${YELLOW}Generating Doxygen documentation...${NC}"
            if command -v doxygen &> /dev/null; then
                doxygen Doxyfile
                echo -e "${GREEN}Documentation generated in docs/reference/api/generated/${NC}"
                echo -e "${CYAN}Open docs/reference/api/generated/html/index.html to view${NC}"
            else
                echo -e "${RED}Error: Doxygen not installed${NC}"
                echo -e "Install with: brew install doxygen graphviz (macOS)"
                echo -e "         or: sudo apt-get install doxygen graphviz (Linux)"
                exit 1
            fi
            ;;
        help|--help|-h)
            show_help
            ;;
        menu|"")
            # Interactive menu mode
            print_header
            while true; do
                show_menu
                read -p "Select option: " choice
                case $choice in
                    1)
                        configure_project "Debug"
                        build_project
                        ;;
                    2)
                        configure_project "Release"
                        build_project
                        ;;
                    3)
                        clean_build
                        configure_project "Debug"
                        build_project
                        ;;
                    4)
                        clean_build
                        configure_project "Release"
                        build_project
                        ;;
                    5)
                        run_game
                        ;;
                    6)
                        run_tests
                        ;;
                    7)
                        run_with_args --test
                        ;;
                    8)
                        echo -e "${CYAN}Enter keystrokes (or press Enter for default):${NC}"
                        echo -e "${CYAN}Example: \\n\\u\\r\\d\\l for Enter, Up, Right, Down, Left${NC}"
                        read -r custom_keys
                        run_dump_test "${custom_keys:-\n\u\u\r\r\d\l}"
                        ;;
                    9)
                        clean_build
                        ;;
                    10)
                        reset_terminal
                        ;;
                    11)
                        clear_logs
                        ;;
                    12)
                        gource_video
                        ;;
                    13)
                        class_diagram
                        ;;
                    0)
                        echo -e "${GREEN}Goodbye!${NC}"
                        exit 0
                        ;;
                    *)
                        echo -e "${RED}Invalid option${NC}"
                        ;;
                esac
            done
            ;;
        *)
            echo -e "${RED}Unknown command: $1${NC}"
            echo "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"