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

# Source .env file if it exists for database configuration
if [ -f "${PROJECT_ROOT}/.env" ]; then
    set -a  # Export all variables
    source "${PROJECT_ROOT}/.env"
    set +a  # Stop exporting
    echo -e "${GREEN}Loaded environment variables from .env${NC}"
fi

# Help CMake find PostgreSQL on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # Check common PostgreSQL installation paths
    if [ -d "/opt/homebrew/opt/postgresql@16" ]; then
        export PostgreSQL_ROOT="/opt/homebrew/opt/postgresql@16"
        echo -e "${GREEN}Found PostgreSQL at $PostgreSQL_ROOT${NC}"
    elif [ -d "/usr/local/opt/postgresql@16" ]; then
        export PostgreSQL_ROOT="/usr/local/opt/postgresql@16"
        echo -e "${GREEN}Found PostgreSQL at $PostgreSQL_ROOT${NC}"
    elif [ -d "/opt/homebrew/opt/postgresql" ]; then
        export PostgreSQL_ROOT="/opt/homebrew/opt/postgresql"
        echo -e "${GREEN}Found PostgreSQL at $PostgreSQL_ROOT${NC}"
    elif [ -d "/usr/local/opt/postgresql" ]; then
        export PostgreSQL_ROOT="/usr/local/opt/postgresql"
        echo -e "${GREEN}Found PostgreSQL at $PostgreSQL_ROOT${NC}"
    fi
fi

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

# Database management functions
create_db_program() {
    # Create a simple C++ program to manage database
    cat > "${BUILD_DIR}/db_manager.cpp" << 'EOF'
#include <iostream>
#include <string>

#ifdef ENABLE_DATABASE
#include "db/database_manager.h"
#include "log.h"

using namespace db;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <command>" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  create    - Create database tables" << std::endl;
        std::cout << "  clear     - Clear all data" << std::endl;
        std::cout << "  load      - Load initial data" << std::endl;
        std::cout << "  status    - Check database status" << std::endl;
        return 1;
    }

    // Initialize logging
    Log::init("db_manager");

    // Get database connection info from environment
    DatabaseConfig config;
    config.host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost";
    config.port = std::getenv("DB_PORT") ? std::stoi(std::getenv("DB_PORT")) : 5432;
    config.database = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "veyrm_game";
    config.username = std::getenv("DB_USER") ? std::getenv("DB_USER") : "veyrm_user";
    config.password = std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "secure_password";

    try {
        auto& db = DatabaseManager::getInstance();
        db.initialize(config);

        std::string command = argv[1];

        if (command == "create") {
            std::cout << "Creating database tables..." << std::endl;
            if (db.createTables()) {
                std::cout << "Tables created successfully!" << std::endl;
            } else {
                std::cout << "Failed to create tables." << std::endl;
                return 1;
            }
        } else if (command == "clear") {
            std::cout << "Clearing all database data..." << std::endl;
            if (db.clearAllData()) {
                std::cout << "Data cleared successfully!" << std::endl;
            } else {
                std::cout << "Failed to clear data." << std::endl;
                return 1;
            }
        } else if (command == "load") {
            std::cout << "Loading initial data..." << std::endl;
            if (db.loadInitialData()) {
                std::cout << "Initial data loaded successfully!" << std::endl;
            } else {
                std::cout << "Failed to load initial data." << std::endl;
                return 1;
            }
        } else if (command == "status") {
            std::cout << "Database Status:" << std::endl;
            std::cout << "  Connected: " << (db.testConnection() ? "Yes" : "No") << std::endl;
            std::cout << "  Version: " << db.getDatabaseVersion() << std::endl;
            std::cout << "  Schema Version: " << db.getCurrentSchemaVersion() << std::endl;
            std::cout << "  Data Loaded: " << (db.isDataLoaded() ? "Yes" : "No") << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            return 1;
        }

        db.shutdown();

    } catch (const std::exception& e) {
        std::cout << "Database error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#else
int main() {
    std::cout << "Database support not compiled. Rebuild with PostgreSQL support." << std::endl;
    return 1;
}
#endif
EOF
}

# Load environment variables from .env file
load_env() {
    if [ -f "${PROJECT_ROOT}/.env" ]; then
        echo -e "${BLUE}Loading environment variables from .env...${NC}"
        export $(grep -v '^#' "${PROJECT_ROOT}/.env" | xargs)
    else
        echo -e "${YELLOW}Warning: .env file not found. Using default values.${NC}"
        echo -e "${YELLOW}Copy .env.example to .env and customize for your setup.${NC}"
    fi
}

# Database command functions
db_create() {
    echo -e "${YELLOW}Creating database tables...${NC}"
    load_env
    create_db_program

    # Compile the database manager
    if command -v g++ >/dev/null 2>&1; then
        g++ -std=c++23 -I"${PROJECT_ROOT}/include" -I"${BUILD_DIR}/_deps/json-src/include" \
            -DENABLE_DATABASE "${BUILD_DIR}/db_manager.cpp" \
            "${PROJECT_ROOT}/src/db/database_manager.cpp" \
            "${PROJECT_ROOT}/src/log.cpp" \
            -lpq -o "${BUILD_DIR}/db_manager" 2>/dev/null || {
            echo -e "${RED}Failed to compile database manager. Using alternative method...${NC}"
            return 1
        }

        # Use environment variables from .env or defaults
        export DB_HOST=${DB_HOST:-localhost}
        export DB_PORT=${DB_PORT:-5432}
        export DB_NAME=${DB_NAME:-veyrm_db}
        export DB_USER=${DB_USER:-veyrm_admin}
        export DB_PASS=${DB_PASS:-changeme_to_secure_password}

        "${BUILD_DIR}/db_manager" create
    else
        echo -e "${RED}g++ not found. Cannot run database commands.${NC}"
        return 1
    fi
}

db_clear() {
    echo -e "${YELLOW}Clearing database data...${NC}"
    load_env
    create_db_program

    if command -v g++ >/dev/null 2>&1; then
        g++ -std=c++23 -I"${PROJECT_ROOT}/include" -I"${BUILD_DIR}/_deps/json-src/include" \
            -DENABLE_DATABASE "${BUILD_DIR}/db_manager.cpp" \
            "${PROJECT_ROOT}/src/db/database_manager.cpp" \
            "${PROJECT_ROOT}/src/log.cpp" \
            -lpq -o "${BUILD_DIR}/db_manager" 2>/dev/null || {
            echo -e "${RED}Failed to compile database manager.${NC}"
            return 1
        }

        export DB_HOST=${DB_HOST:-localhost}
        export DB_PORT=${DB_PORT:-5432}
        export DB_NAME=${DB_NAME:-veyrm_db}
        export DB_USER=${DB_USER:-veyrm_admin}
        export DB_PASS=${DB_PASS:-changeme_to_secure_password}

        "${BUILD_DIR}/db_manager" clear
    else
        echo -e "${RED}g++ not found. Cannot run database commands.${NC}"
        return 1
    fi
}

db_load() {
    echo -e "${YELLOW}Loading initial database data...${NC}"
    load_env
    create_db_program

    if command -v g++ >/dev/null 2>&1; then
        g++ -std=c++23 -I"${PROJECT_ROOT}/include" -I"${BUILD_DIR}/_deps/json-src/include" \
            -DENABLE_DATABASE "${BUILD_DIR}/db_manager.cpp" \
            "${PROJECT_ROOT}/src/db/database_manager.cpp" \
            "${PROJECT_ROOT}/src/log.cpp" \
            -lpq -o "${BUILD_DIR}/db_manager" 2>/dev/null || {
            echo -e "${RED}Failed to compile database manager.${NC}"
            return 1
        }

        export DB_HOST=${DB_HOST:-localhost}
        export DB_PORT=${DB_PORT:-5432}
        export DB_NAME=${DB_NAME:-veyrm_db}
        export DB_USER=${DB_USER:-veyrm_admin}
        export DB_PASS=${DB_PASS:-changeme_to_secure_password}

        "${BUILD_DIR}/db_manager" load
    else
        echo -e "${RED}g++ not found. Cannot run database commands.${NC}"
        return 1
    fi
}

db_status() {
    echo -e "${YELLOW}Checking database status...${NC}"
    load_env
    create_db_program

    if command -v g++ >/dev/null 2>&1; then
        g++ -std=c++23 -I"${PROJECT_ROOT}/include" -I"${BUILD_DIR}/_deps/json-src/include" \
            -DENABLE_DATABASE "${BUILD_DIR}/db_manager.cpp" \
            "${PROJECT_ROOT}/src/db/database_manager.cpp" \
            "${PROJECT_ROOT}/src/log.cpp" \
            -lpq -o "${BUILD_DIR}/db_manager" 2>/dev/null || {
            echo -e "${RED}Failed to compile database manager.${NC}"
            return 1
        }

        export DB_HOST=${DB_HOST:-localhost}
        export DB_PORT=${DB_PORT:-5432}
        export DB_NAME=${DB_NAME:-veyrm_db}
        export DB_USER=${DB_USER:-veyrm_admin}
        export DB_PASS=${DB_PASS:-changeme_to_secure_password}

        "${BUILD_DIR}/db_manager" status
    else
        echo -e "${RED}g++ not found. Cannot run database commands.${NC}"
        return 1
    fi
}

db_reset() {
    echo -e "${YELLOW}Resetting database (clear + load)...${NC}"
    db_clear && db_load
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

# Function to generate Doxygen documentation
generate_doxygen() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Doxygen Documentation Generator${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Check if doxygen is installed
    if ! command -v doxygen &> /dev/null; then
        echo -e "${RED}Error: doxygen is not installed${NC}"
        echo -e "${YELLOW}Install with: brew install doxygen (macOS)${NC}"
        echo -e "${YELLOW}         or: sudo apt-get install doxygen (Linux)${NC}"
        exit 1
    fi

    # Create docs directories if they don't exist
    mkdir -p "${PROJECT_ROOT}/docs/reference/api/generated"

    echo -e "${YELLOW}Creating Doxyfile configuration...${NC}"

    # Create Doxyfile
    cat > "${PROJECT_ROOT}/Doxyfile" << 'EOF'
# Doxygen configuration for Veyrm

# Project settings
PROJECT_NAME           = "Veyrm"
PROJECT_NUMBER         = "1.0.0"
PROJECT_BRIEF          = "Modern C++ Roguelike Game"
OUTPUT_DIRECTORY       = docs/reference/api/generated

# Input settings
INPUT                  = include src
FILE_PATTERNS          = *.h *.hpp *.cpp *.cc
RECURSIVE              = YES
EXCLUDE_PATTERNS       = */build/* */tests/*

# Build settings
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = YES
EXTRACT_STATIC         = YES
EXTRACT_LOCAL_CLASSES  = YES
HIDE_UNDOC_MEMBERS     = NO
HIDE_UNDOC_CLASSES     = NO

# Output settings
GENERATE_HTML          = YES
HTML_OUTPUT            = html
GENERATE_LATEX         = NO
GENERATE_MAN           = NO
GENERATE_XML           = NO

# HTML settings
HTML_COLORSTYLE_HUE    = 220
HTML_COLORSTYLE_SAT    = 100
HTML_COLORSTYLE_GAMMA  = 80
HTML_TIMESTAMP         = YES
HTML_DYNAMIC_SECTIONS  = YES

# Graph settings (if graphviz is available)
HAVE_DOT               = YES
CLASS_DIAGRAMS         = YES
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
GROUP_GRAPHS           = YES
UML_LOOK               = YES
TEMPLATE_RELATIONS     = YES
CALL_GRAPH             = NO
CALLER_GRAPH           = NO
GRAPHICAL_HIERARCHY    = YES
DIRECTORY_GRAPH        = YES
DOT_IMAGE_FORMAT       = svg
INTERACTIVE_SVG        = YES
DOT_TRANSPARENT        = YES

# Quiet output
QUIET                  = YES
WARNINGS               = YES
WARN_IF_UNDOCUMENTED   = NO
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = NO
EOF

    echo -e "${YELLOW}Generating documentation...${NC}"
    cd "${PROJECT_ROOT}"
    doxygen Doxyfile 2>&1 | grep -v "warning: .* is not documented" || true

    if [ $? -eq 0 ] || [ -f "${PROJECT_ROOT}/docs/reference/api/generated/html/index.html" ]; then
        echo
        echo -e "${GREEN}Documentation generated successfully!${NC}"
        echo -e "${CYAN}Output directory: ${PROJECT_ROOT}/docs/reference/api/generated/html${NC}"
        echo

        # Open in browser if on macOS
        if [[ "$OSTYPE" == "darwin"* ]]; then
            echo -e "${YELLOW}Opening documentation in browser...${NC}"
            open "${PROJECT_ROOT}/docs/reference/api/generated/html/index.html"
        else
            echo -e "${YELLOW}Open in browser:${NC}"
            echo "  file://${PROJECT_ROOT}/docs/reference/api/generated/html/index.html"
        fi
    else
        echo -e "${RED}Failed to generate documentation${NC}"
        exit 1
    fi
}

# Function to lint markdown files
lint_markdown() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Markdown Linter${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Check if markdownlint is installed
    if ! command -v markdownlint &> /dev/null; then
        echo -e "${RED}markdownlint not found. Installing...${NC}"
        npm install -g markdownlint-cli
    fi

    echo
    echo -e "${BOLD}Markdown Lint Options:${NC}"
    echo -e "${BLUE}1)${NC} Check all markdown files (no changes)"
    echo -e "${BLUE}2)${NC} Fix all markdown files automatically"
    echo -e "${BLUE}3)${NC} Check specific file"
    echo -e "${BLUE}4)${NC} Fix specific file"
    echo -e "${BLUE}5)${NC} Check CLAUDE.md"
    echo -e "${BLUE}6)${NC} Fix CLAUDE.md"
    echo -e "${BLUE}0)${NC} Back to main menu"
    echo
    read -p "Select option: " lint_option

    case $lint_option in
        1)
            echo -e "${YELLOW}Checking all markdown files...${NC}"
            markdownlint "**/*.md" --config .markdownlint.json --ignore-path .markdownlintignore || true
            echo -e "${GREEN}Check complete${NC}"
            ;;
        2)
            echo -e "${YELLOW}Fixing all markdown files...${NC}"
            markdownlint "**/*.md" --config .markdownlint.json --ignore-path .markdownlintignore --fix
            echo -e "${GREEN}Auto-fix complete${NC}"
            ;;
        3)
            read -p "Enter file path: " file_path
            echo -e "${YELLOW}Checking ${file_path}...${NC}"
            markdownlint "${file_path}" --config .markdownlint.json || true
            echo -e "${GREEN}Check complete${NC}"
            ;;
        4)
            read -p "Enter file path: " file_path
            echo -e "${YELLOW}Fixing ${file_path}...${NC}"
            markdownlint "${file_path}" --config .markdownlint.json --fix
            echo -e "${GREEN}Auto-fix complete${NC}"
            ;;
        5)
            echo -e "${YELLOW}Checking CLAUDE.md...${NC}"
            markdownlint "CLAUDE.md" --config .markdownlint.json || true
            echo -e "${GREEN}Check complete${NC}"
            ;;
        6)
            echo -e "${YELLOW}Fixing CLAUDE.md...${NC}"
            markdownlint "CLAUDE.md" --config .markdownlint.json --fix
            echo -e "${GREEN}Auto-fix complete${NC}"
            ;;
        0)
            return
            ;;
        *)
            echo -e "${RED}Invalid option${NC}"
            ;;
    esac

    echo
    read -p "Press Enter to continue..."
}

# Function to create a release
create_release() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Release Creator${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Get current version from config.yml
    CURRENT_VERSION=$(grep "^version:" "${PROJECT_ROOT}/config.yml" | sed 's/version: *"\(.*\)"/\1/')

    if [ -z "$CURRENT_VERSION" ]; then
        echo -e "${RED}Error: Could not determine current version from config.yml${NC}"
        exit 1
    fi

    echo -e "${GREEN}Current version: v$CURRENT_VERSION${NC}"

    # Parse version components
    IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT_VERSION"

    # Determine new version based on argument
    case "${1:-ask}" in
        major)
            NEW_MAJOR=$((MAJOR + 1))
            NEW_VERSION="$NEW_MAJOR.0.0"
            ;;
        minor)
            NEW_MINOR=$((MINOR + 1))
            NEW_VERSION="$MAJOR.$NEW_MINOR.0"
            ;;
        patch)
            NEW_PATCH=$((PATCH + 1))
            NEW_VERSION="$MAJOR.$MINOR.$NEW_PATCH"
            ;;
        custom)
            read -p "Enter new version (without 'v' prefix): " NEW_VERSION
            # Validate version format
            if ! [[ "$NEW_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9]+)?$ ]]; then
                echo -e "${RED}Error: Invalid version format. Use X.Y.Z or X.Y.Z-suffix${NC}"
                exit 1
            fi
            ;;
        ask|*)
            echo
            echo "Select version bump type:"
            echo "  1) Patch (${MAJOR}.${MINOR}.${PATCH} -> ${MAJOR}.${MINOR}.$((PATCH + 1)))"
            echo "  2) Minor (${MAJOR}.${MINOR}.${PATCH} -> ${MAJOR}.$((MINOR + 1)).0)"
            echo "  3) Major (${MAJOR}.${MINOR}.${PATCH} -> $((MAJOR + 1)).0.0)"
            echo "  4) Custom version"
            echo "  0) Cancel"
            echo
            read -p "Enter choice: " choice

            case "$choice" in
                1)
                    NEW_PATCH=$((PATCH + 1))
                    NEW_VERSION="$MAJOR.$MINOR.$NEW_PATCH"
                    ;;
                2)
                    NEW_MINOR=$((MINOR + 1))
                    NEW_VERSION="$MAJOR.$NEW_MINOR.0"
                    ;;
                3)
                    NEW_MAJOR=$((MAJOR + 1))
                    NEW_VERSION="$NEW_MAJOR.0.0"
                    ;;
                4)
                    read -p "Enter new version (without 'v' prefix): " NEW_VERSION
                    if ! [[ "$NEW_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9]+)?$ ]]; then
                        echo -e "${RED}Error: Invalid version format. Use X.Y.Z or X.Y.Z-suffix${NC}"
                        exit 1
                    fi
                    ;;
                0|*)
                    echo "Release cancelled."
                    exit 0
                    ;;
            esac
            ;;
    esac

    echo
    echo -e "${YELLOW}New version will be: v$NEW_VERSION${NC}"
    read -p "Continue? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi

    echo
    echo -e "${YELLOW}Updating version in all files...${NC}"

    # Update version in config.yml
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/version: \"$CURRENT_VERSION\"/version: \"$NEW_VERSION\"/" "${PROJECT_ROOT}/config.yml"
    else
        sed -i "s/version: \"$CURRENT_VERSION\"/version: \"$NEW_VERSION\"/" "${PROJECT_ROOT}/config.yml"
    fi

    # Update version in CMakeLists.txt
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/VERSION $CURRENT_VERSION/VERSION $NEW_VERSION/" "${PROJECT_ROOT}/CMakeLists.txt"
    else
        sed -i "s/VERSION $CURRENT_VERSION/VERSION $NEW_VERSION/" "${PROJECT_ROOT}/CMakeLists.txt"
    fi

    # Check if there are uncommitted changes
    if [ -n "$(git status --porcelain)" ]; then
        echo -e "${YELLOW}Committing version update...${NC}"
        git add config.yml CMakeLists.txt
        git commit -m "Release v$NEW_VERSION

- Updated version in config.yml
- Updated version in CMakeLists.txt"
    fi

    # Create and push tag
    echo -e "${YELLOW}Creating tag v$NEW_VERSION...${NC}"

    # Prompt for release notes
    echo
    echo "Enter release notes (press Ctrl+D when done):"
    echo "-------------------------------------------"
    RELEASE_NOTES=$(cat)

    if [ -z "$RELEASE_NOTES" ]; then
        RELEASE_NOTES="Release v$NEW_VERSION"
    fi

    git tag -a "v$NEW_VERSION" -m "$RELEASE_NOTES"

    echo
    echo -e "${GREEN}✓ Version updated to $NEW_VERSION${NC}"
    echo -e "${GREEN}✓ Tag v$NEW_VERSION created${NC}"
    echo
    echo "To trigger the GitHub release workflow, push with:"
    echo -e "${YELLOW}  git push && git push --tags${NC}"
    echo
    echo "Or to abort without releasing:"
    echo -e "${YELLOW}  git tag -d v$NEW_VERSION${NC}"
    echo -e "${YELLOW}  git reset --hard HEAD~1${NC}"
    echo
    echo -e "${CYAN}The GitHub Actions workflow will automatically:${NC}"
    echo "  • Build Linux and Windows binaries"
    echo "  • Create a GitHub release with the binaries"
    echo "  • Extract release notes from CHANGELOG.md"
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

# Function to build with coverage
build_coverage() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Building with Coverage${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Clean previous coverage data
    echo -e "${YELLOW}Cleaning previous coverage data...${NC}"
    find "${BUILD_DIR}" -name "*.gcda" -delete 2>/dev/null || true
    find "${BUILD_DIR}" -name "*.gcno" -delete 2>/dev/null || true
    rm -rf "${BUILD_DIR}/coverage" 2>/dev/null || true

    # Create build directory if it doesn't exist
    mkdir -p "${BUILD_DIR}"

    # Configure with coverage enabled using GCC for better coverage support
    echo -e "${YELLOW}Configuring with coverage enabled...${NC}"
    cd "${BUILD_DIR}" || exit 1

    # Use GCC for coverage instead of Apple Clang for better coverage support
    if command -v gcc-15 &> /dev/null; then
        echo -e "${GREEN}Using GCC-15 for better coverage support${NC}"
        CC=gcc-15 CXX=g++-15 cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DENABLE_AUTH=ON -DC4CORE_NO_DEBUG_BREAK=ON
    else
        echo -e "${YELLOW}Using default compiler (may have coverage issues on macOS)${NC}"
        cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DENABLE_AUTH=ON
    fi

    # Build the project using make for better coverage compatibility
    echo -e "${YELLOW}Building project...${NC}"
    JOBS=${JOBS:-$(nproc 2>/dev/null || echo 4)}
    make -j "${JOBS}"

    # Run tests to generate coverage data from project root (so data files are found)
    echo -e "${YELLOW}Running tests to generate coverage data...${NC}"
    cd "${PROJECT_ROOT}" || exit 1

    # Run tests from project root where data files are available
    if [ -f "${BUILD_DIR}/bin/veyrm_tests" ]; then
        "${BUILD_DIR}/bin/veyrm_tests"
        if [ $? -ne 0 ]; then
            echo -e "${RED}Warning: Some tests failed, but continuing with coverage generation${NC}"
        fi
    else
        echo -e "${RED}Error: Test executable not found at ${BUILD_DIR}/bin/veyrm_tests${NC}"
        exit 1
    fi

    echo -e "${GREEN}Coverage build complete!${NC}"
    echo -e "${YELLOW}Run './build.sh coverage-report' to generate HTML report${NC}"
}

# Function to generate coverage report
generate_coverage_report() {
    echo -e "${CYAN}=========================================${NC}"
    echo -e "${CYAN}       Coverage Report Generation${NC}"
    echo -e "${CYAN}=========================================${NC}"

    # Check for lcov
    if ! command -v lcov &> /dev/null; then
        echo -e "${RED}Error: lcov is not installed${NC}"
        echo -e "${YELLOW}Install with:${NC}"
        echo -e "${YELLOW}  macOS: brew install lcov${NC}"
        echo -e "${YELLOW}  Linux: sudo apt install lcov${NC}"
        exit 1
    fi

    # Check for genhtml
    if ! command -v genhtml &> /dev/null; then
        echo -e "${RED}Error: genhtml is not installed (part of lcov)${NC}"
        exit 1
    fi

    # Check if coverage data exists
    if ! find "${BUILD_DIR}" -name "*.gcda" 2>/dev/null | grep -q .; then
        echo -e "${RED}No coverage data found!${NC}"
        echo -e "${YELLOW}Run './build.sh coverage' first to build with coverage and run tests${NC}"
        exit 1
    fi

    local COVERAGE_DIR="${BUILD_DIR}/coverage"
    mkdir -p "${COVERAGE_DIR}"

    echo -e "${YELLOW}Capturing coverage data...${NC}"
    if ! lcov --capture \
         --directory "${BUILD_DIR}" \
         --output-file "${COVERAGE_DIR}/coverage.info" \
         --rc branch_coverage=1 \
         --ignore-errors deprecated,mismatch,inconsistent,gcov,source,unsupported,format,count,path,empty; then
        echo -e "${RED}Error: Failed to capture coverage data${NC}"
        exit 1
    fi

    # Check if coverage data was captured
    if [ ! -f "${COVERAGE_DIR}/coverage.info" ] || [ ! -s "${COVERAGE_DIR}/coverage.info" ]; then
        echo -e "${RED}Error: No coverage data captured${NC}"
        echo -e "${YELLOW}Make sure tests were run with coverage enabled${NC}"
        exit 1
    fi

    # Remove external libraries and test files from coverage
    echo -e "${YELLOW}Filtering coverage data...${NC}"
    if ! lcov --remove "${COVERAGE_DIR}/coverage.info" \
         '/usr/*' \
         '/Applications/*' \
         '*/build/_deps/*' \
         '*/tests/*' \
         '*/test_*.cpp' \
         '*/v1/*' \
         '*/_deps/*' \
         '*/catch2/*' \
         '*/ftxui/*' \
         '*/json/*' \
         '*/ryml/*' \
         --output-file "${COVERAGE_DIR}/coverage_filtered.info" \
         --rc branch_coverage=1 \
         --ignore-errors deprecated,mismatch,inconsistent,source,format,unused,path; then
        echo -e "${RED}Error: Failed to filter coverage data${NC}"
        exit 1
    fi

    # Generate HTML report
    echo -e "${YELLOW}Generating HTML report...${NC}"
    genhtml "${COVERAGE_DIR}/coverage_filtered.info" \
            --output-directory "${COVERAGE_DIR}/html" \
            --function-coverage \
            --title "Veyrm Code Coverage Report" \
            --legend \
            --ignore-errors category,mismatch,inconsistent,unsupported,source,deprecated \
            --show-details \
            --demangle-cpp

    # Check if HTML generation succeeded
    if [ ! -f "${COVERAGE_DIR}/html/index.html" ]; then
        echo -e "${RED}Error: HTML report generation failed${NC}"
        echo -e "${YELLOW}Raw coverage data is available at: ${COVERAGE_DIR}/coverage_filtered.info${NC}"
        exit 1
    fi

    # Print summary
    echo -e "${GREEN}=========================================${NC}"
    echo -e "${GREEN}Coverage report generated successfully!${NC}"
    echo -e "${GREEN}=========================================${NC}"
    lcov --summary "${COVERAGE_DIR}/coverage_filtered.info" --rc lcov_branch_coverage=1

    echo
    echo -e "${CYAN}HTML Report: ${COVERAGE_DIR}/html/index.html${NC}"

    # Open in browser on macOS
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo -e "${YELLOW}Opening report in browser...${NC}"
        open "${COVERAGE_DIR}/html/index.html"
    else
        echo -e "${YELLOW}Open ${COVERAGE_DIR}/html/index.html in your browser to view the report${NC}"
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

# Function to run with auto-login and keys
run_with_autologin() {
    local keystrokes="${1:-\\njjjq}"
    echo -e "${YELLOW}Running game with auto-login and automated keys...${NC}"
    echo -e "${CYAN}Auto-login: NRF${NC}"
    echo -e "${CYAN}Keystrokes: ${keystrokes}${NC}"
    if [ -f "${EXECUTABLE}" ]; then
        # Run from project root
        cd "${PROJECT_ROOT}"
        "${EXECUTABLE}" --username NRF --password '@Cwtt4eva' --keys "${keystrokes}"
        cd - > /dev/null
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to run with test credentials (manual play)
run_with_testcreds() {
    echo -e "${YELLOW}Running game with test credentials (manual play)...${NC}"
    echo -e "${CYAN}Test login: NRF${NC}"
    echo -e "${CYAN}Manual gameplay enabled${NC}"
    if [ -f "${EXECUTABLE}" ]; then
        # Run from project root
        cd "${PROJECT_ROOT}"
        "${EXECUTABLE}" --username NRF --password '@Cwtt4eva'
        cd - > /dev/null
    else
        echo -e "${RED}Executable not found. Build first.${NC}"
    fi
}

# Function to clear test data
clear_test_data() {
    echo -e "${YELLOW}Clearing test data for NRF user...${NC}"

    # Check if PostgreSQL is running
    if ! command -v docker-compose &> /dev/null; then
        echo -e "${RED}Error: docker-compose not found${NC}"
        return 1
    fi

    # Check if we're in the right directory or find PostgreSQL directory
    local postgres_dir="/Users/nrf/repos/PostgreSQL"
    if [ ! -f "${postgres_dir}/docker-compose.yml" ]; then
        echo -e "${RED}Error: PostgreSQL docker-compose.yml not found at ${postgres_dir}${NC}"
        return 1
    fi

    echo -e "${CYAN}Clearing game saves and entities for NRF (user_id=1)...${NC}"

    # Execute SQL to clear test data
    if docker-compose -f "${postgres_dir}/docker-compose.yml" exec postgres psql -U veyrm_admin -d veyrm_db -c "DELETE FROM game_entities WHERE user_id = 1; DELETE FROM game_saves WHERE user_id = 1;" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Test data cleared successfully${NC}"
        echo -e "${CYAN}  - Cleared all game saves for NRF${NC}"
        echo -e "${CYAN}  - Cleared all game entities for NRF${NC}"
    else
        echo -e "${RED}✗ Failed to clear test data${NC}"
        echo -e "${YELLOW}Make sure PostgreSQL is running: cd ${postgres_dir} && docker-compose up -d${NC}"
        return 1
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
    echo -e "${GREEN}Enter)${NC} Run Game ${GREEN}(default)${NC}"
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
    echo -e "${BLUE}14)${NC} Generate Doxygen Docs"
    echo -e "${BLUE}15)${NC} Create Release"
    echo -e "${BLUE}16)${NC} Lint Markdown Files"
    echo -e "${BLUE}17)${NC} Build with Coverage"
    echo -e "${BLUE}18)${NC} Generate Coverage Report"
    echo -e "${BLUE}19)${NC} Test with Credentials (Manual Play)"
    echo -e "${BLUE}20)${NC} Test with Auto-login + Keys"
    echo -e "${BLUE}21)${NC} Clear Test Data"
    echo -e "${BLUE}q)${NC} Quit"
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
    echo "  coverage               Build with coverage enabled and run tests"
    echo "  coverage-report        Generate HTML coverage report"
    echo "  dump [keystrokes]      Run dump mode test (frame-by-frame)"
    echo "  keys <keystrokes>      Run game with automated keys"
    echo "  autologin [keystrokes] Run game with auto-login and automated keys"
    echo "  testkeys               Run game with test credentials (manual play)"
    echo "  cleardata              Clear test data for NRF user"
    echo
    echo -e "${BOLD}Database Commands:${NC}"
    echo "  db create              Create database tables"
    echo "  db clear               Clear all database data"
    echo "  db load                Load initial data"
    echo "  db status              Check database status"
    echo "  db reset               Clear and reload data"
    echo "  check                  Run system checks"
    echo "  reset                  Reset terminal"
    echo "  clearlog               Clear all log files"
    echo "  gource [--clean]       Create Gource video (--clean deletes old videos)"
    echo "  diagram                Generate class diagrams with graphviz"
    echo "  docs|doxygen           Generate API documentation with Doxygen"
    echo "  release [type]         Create a release (patch|minor|major|custom)"
    echo "  lint|md [fix]          Check/fix markdown files (use 'fix' to auto-fix)"
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
    echo "  $0 keys '\\n\\d\\d\\dq'    # Run game with automated keys (Enter, Down 3x, quit)"
    echo "  $0 autologin          # Run game with auto-login and default keys"
    echo "  $0 autologin '\\n\\d\\d\\dq' # Run game with auto-login and custom keys"
    echo "  $0 testkeys           # Run game with test credentials (manual play)"
    echo "  $0 cleardata          # Clear test data for NRF user"
    echo "  $0 gource             # Create Gource video"
    echo "  $0 reset              # Reset terminal"
    echo "  $0 lint               # Check markdown files for issues"
    echo "  $0 lint fix           # Auto-fix markdown issues"
    echo "  $0 lint CLAUDE.md fix # Fix specific file"
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
        coverage)
            print_header
            build_coverage
            ;;
        coverage-report)
            print_header
            generate_coverage_report
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
                echo "Example: $0 keys '\\n\\d\\d\\dq'"
                exit 1
            fi
            print_header
            run_with_keys "$2"
            reset_terminal
            ;;
        autologin)
            print_header
            run_with_autologin "$2"
            reset_terminal
            ;;
        testkeys)
            print_header
            run_with_testcreds
            reset_terminal
            ;;
        cleardata)
            print_header
            clear_test_data
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
        docs|doxygen)
            print_header
            generate_doxygen
            ;;
        release)
            print_header
            create_release "${2}"
            ;;
        lint|markdown|md)
            # Handle markdown linting
            if [ "$2" = "fix" ] || [ "$2" = "--fix" ]; then
                echo -e "${YELLOW}Fixing all markdown files...${NC}"
                markdownlint "**/*.md" --config .markdownlint.json --ignore-path .markdownlintignore --fix
                echo -e "${GREEN}Auto-fix complete${NC}"
            elif [ "$2" = "check" ] || [ "$2" = "--check" ] || [ -z "$2" ]; then
                echo -e "${YELLOW}Checking all markdown files...${NC}"
                markdownlint "**/*.md" --config .markdownlint.json --ignore-path .markdownlintignore || true
                echo -e "${GREEN}Check complete${NC}"
            elif [ -f "$2" ]; then
                # If second argument is a file, lint that file
                if [ "$3" = "fix" ] || [ "$3" = "--fix" ]; then
                    echo -e "${YELLOW}Fixing ${2}...${NC}"
                    markdownlint "${2}" --config .markdownlint.json --fix
                    echo -e "${GREEN}Auto-fix complete${NC}"
                else
                    echo -e "${YELLOW}Checking ${2}...${NC}"
                    markdownlint "${2}" --config .markdownlint.json || true
                    echo -e "${GREEN}Check complete${NC}"
                fi
            else
                lint_markdown
            fi
            ;;
        db)
            print_header
            case "${2}" in
                create)
                    db_create
                    ;;
                clear)
                    db_clear
                    ;;
                load)
                    db_load
                    ;;
                status)
                    db_status
                    ;;
                reset)
                    db_reset
                    ;;
                *)
                    echo -e "${RED}Unknown database command: ${2}${NC}"
                    echo "Available commands: create, clear, load, status, reset"
                    exit 1
                    ;;
            esac
            ;;
        help|--help|-h)
            show_help
            ;;
        menu|"")
            # Interactive menu mode
            print_header
            while true; do
                show_menu
                read -p "Select option (Enter to run game, q to quit): " choice
                case $choice in
                    "")
                        # Default action: Run Game
                        run_game
                        ;;
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
                    14)
                        generate_doxygen
                        ;;
                    15)
                        create_release
                        ;;
                    16)
                        lint_markdown
                        ;;
                    17)
                        build_coverage
                        ;;
                    18)
                        generate_coverage_report
                        ;;
                    19)
                        run_with_testcreds
                        ;;
                    20)
                        echo -e "${CYAN}Enter keystrokes (or press Enter for default):${NC}"
                        echo -e "${CYAN}Example: \\n\\d\\d\\dq for Enter, Down 3x, Quit${NC}"
                        read -r custom_keys
                        run_with_autologin "${custom_keys:-\n\d\d\dq}"
                        ;;
                    21)
                        clear_test_data
                        ;;
                    q|Q)
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