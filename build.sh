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
        cd "${BUILD_DIR}"
        ctest --output-on-failure || echo -e "${RED}Some tests failed${NC}"
        cd - > /dev/null
    else
        echo -e "${RED}Test executable not found. Build first.${NC}"
    fi
}

# Function to run dump mode test
run_dump_test() {
    local keystrokes="${1:-\\n\\u\\u\\r\\r\\d\\l}"
    echo -e "${YELLOW}Running dump mode test...${NC}"
    echo -e "${CYAN}Keystrokes: ${keystrokes}${NC}"
    if [ -f "${EXECUTABLE}" ]; then
        "${EXECUTABLE}" --dump "${keystrokes}"
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to run the game
run_game() {
    if [ -f "${EXECUTABLE}" ]; then
        echo -e "${YELLOW}Starting Veyrm...${NC}"
        # Ask for map type
        echo -e "${CYAN}Select map type:${NC}"
        echo -e "${BLUE}1)${NC} Test Dungeon (default, 5 rooms)"
        echo -e "${BLUE}2)${NC} Test Room (single 20x20 room)"
        echo -e "${BLUE}3)${NC} Corridor Test (long corridors)"
        echo -e "${BLUE}4)${NC} Combat Arena (open space)"
        echo -e "${BLUE}5)${NC} Stress Test (large map)"
        echo
        read -p "Select map (1-5, or Enter for default): " map_choice
        
        case "${map_choice}" in
            2)
                "${EXECUTABLE}" --map room
                ;;
            3)
                "${EXECUTABLE}" --map corridor
                ;;
            4)
                "${EXECUTABLE}" --map arena
                ;;
            5)
                "${EXECUTABLE}" --map stress
                ;;
            *)
                "${EXECUTABLE}" --map dungeon
                ;;
        esac
        reset_terminal  # Auto-reset terminal after game exits
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to run with arguments
run_with_args() {
    if [ -f "${EXECUTABLE}" ]; then
        "${EXECUTABLE}" "$@"
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to show menu
show_menu() {
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
    echo "  dump [keystrokes]      Run dump mode test"
    echo "  check                  Run system checks"
    echo "  reset                  Reset terminal"
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
                    room|dungeon|corridor|arena|stress)
                        run_with_args --map "${2}"
                        reset_terminal
                        ;;
                    *)
                        echo -e "${RED}Unknown map type: ${2}${NC}"
                        echo -e "Valid types: room, dungeon, corridor, arena, stress"
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
            run_dump_test "${2}"
            ;;
        check)
            print_header
            run_with_args --test
            ;;
        reset)
            reset_terminal
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
                        read -p "Keystrokes: " custom_keys
                        run_dump_test "${custom_keys}"
                        ;;
                    9)
                        clean_build
                        ;;
                    10)
                        reset_terminal
                        ;;
                    0)
                        echo -e "${GREEN}Goodbye!${NC}"
                        exit 0
                        ;;
                    *)
                        echo -e "${RED}Invalid option${NC}"
                        ;;
                esac
                echo
                read -p "Press Enter to continue..."
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