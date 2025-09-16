# Veyrm Logging System

## Overview

Veyrm uses a comprehensive multi-level, category-based logging system for debugging and development. The system creates separate log files for different aspects of the game while maintaining a main debug log with all events chronologically.

## Log Files

All log files are stored in the `logs/` directory, which is automatically created on startup.

### Main Debug Log

- **File:** `logs/veyrm_debug.log`
- **Content:** Complete chronological record of all events
- **Purpose:** Full debugging context with complete timeline

### Category-Specific Logs

| Log File | Category | Content |
|----------|----------|---------|
| `veyrm_player.log` | PLAYER | Player actions, movements, interactions |
| `veyrm_ai.log` | AI, MOVE | Monster AI decisions and movements |
| `veyrm_combat.log` | COMBAT | Combat events, damage, critical hits |
| `veyrm_env.log` | ENV | Environment interactions (doors, terrain) |
| `veyrm_map.log` | MAP | Map generation events |
| `veyrm_system.log` | SYSTEM, ERROR, WARN, INFO, DEBUG, TRACE, UI, SAVE | System messages and errors |
| `veyrm_inventory.log` | INV | Item pickup, drop, use |
| `veyrm_turn.log` | TURN | Turn system events |
| `veyrm_fov.log` | FOV | Field of view updates |
| `veyrm_spawn.log` | SPAWN | Monster spawning events |
| `veyrm_input.log` | INPUT | Keystroke logging with action mapping |

## Log Levels

The logging system supports five severity levels:

1. **ERROR** (0) - Critical errors that may affect gameplay
2. **WARN** (1) - Warnings about potential issues
3. **INFO** (2) - Important informational messages
4. **DEBUG** (3) - Detailed debugging information
5. **TRACE** (4) - Very detailed trace information

Only messages at or below the configured level are logged. The default level is DEBUG.

## Usage in Code

### Using Convenience Macros

```cpp
// General logging by level
LOG_ERROR("Critical error occurred");
LOG_WARN("Warning message");
LOG_INFO("Information message");
LOG_DEBUG("Debug message");
LOG_TRACE("Trace message");

// Category-specific logging
LOG_PLAYER("Player moved north");
LOG_COMBAT("Player hit monster for 5 damage");
LOG_AI("Monster decided to flee");
LOG_ENV("Door opened at (10, 20)");
LOG_MAP("Generated room at (50, 50)");
LOG_INPUT("[INPUT] Key: 'ArrowDown' -> MOVE_DOWN");
```

### Direct API Usage

```cpp
// Initialize logging (called once at startup)
Log::init("logs/veyrm_debug.log", Log::DEBUG);

// Log messages
Log::player("Player action");
Log::combat("Combat event");
Log::ai("AI decision");
Log::environment("Environment interaction");

// Shutdown logging (called at exit)
Log::shutdown();
```

## Log Format

Each log entry follows this format:

```
HH:MM:SS.mmm [CATEGORY] Message
```

Example:

```
18:45:16.087 [PLAYER] Moving east (dx=1, dy=0)
18:45:16.087 [PLAYER] Target position: (166, 13)
18:45:16.088 [AI    ] Monster Gutter Rat moving to (136,49)
18:45:16.088 [COMBAT] Player hit Cave Spider for 8 damage (roll: 15 + 8 = 23 vs AC 12)
```

## Managing Logs

### Clearing Logs

Use the build script to clear all log files:

```bash
./build.sh clearlog
```

### Viewing Logs

```bash
# View main debug log
tail -f logs/veyrm_debug.log

# View specific category
tail -f logs/veyrm_player.log

# Search for specific events
grep "COMBAT" logs/veyrm_debug.log
grep "door" logs/veyrm_env.log
```

## Implementation Details

### File Structure

- **Header:** `include/log.h`
- **Implementation:** `src/log.cpp`

### Key Features

1. **Thread-Safe:** Uses file streams with immediate flush
2. **Automatic Directory Creation:** Creates `logs/` directory if it doesn't exist
3. **Timestamping:** Millisecond precision timestamps
4. **Category Routing:** Automatically routes messages to appropriate files
5. **Console Output:** Only ERROR level messages appear on console (to avoid interfering with game display)

### Adding New Categories

To add a new logging category:

1. Add the log file static member in `log.h`
2. Initialize it in `Log::init()`
3. Close it in `Log::shutdown()`
4. Add routing logic in `getCategoryLogFile()`
5. Create a convenience method and macro

Example:

```cpp
// In log.h
static std::ofstream questLogFile;
static void quest(const std::string& message);
#define LOG_QUEST(msg) Log::quest(msg)

// In log.cpp
std::ofstream Log::questLogFile;

void Log::quest(const std::string& message) {
    log(INFO, "QUEST", message);
}

// In getCategoryLogFile()
if (category == "QUEST") return questLogFile;
```

## Best Practices

1. **Use Category-Specific Macros:** Makes logs easier to filter and understand
2. **Include Context:** Log position coordinates, entity names, damage values
3. **Log State Changes:** Record when important state transitions occur
4. **Avoid Spam:** Use TRACE level for very frequent events (like FOV updates)
5. **Log Errors First:** Always log error conditions before handling them

## Performance Considerations

- Logging has minimal performance impact due to immediate file writes
- File streams are kept open during gameplay for efficiency
- Each category has its own file stream to reduce contention
- Console output is limited to ERROR level to avoid display issues

## Troubleshooting

### Logs Not Appearing

1. Check that `logs/` directory exists
2. Verify logging is initialized (`Log::init()` called)
3. Check log level configuration
4. Ensure proper category is being used

### Log Files Growing Too Large

1. Use `./build.sh clearlog` regularly during development
2. Consider implementing log rotation (not currently implemented)
3. Adjust log level to reduce verbosity

### Missing Events

1. Verify the event is being logged with appropriate macro
2. Check that the log level includes the event's level
3. Ensure the category routing is correct in `getCategoryLogFile()`
