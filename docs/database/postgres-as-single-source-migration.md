# PostgreSQL as Single Source of Truth - Migration Plan

## Overview
This document outlines the migration from a hybrid local/database save system to PostgreSQL as the single source of truth for all game data.

## Current State Analysis

### Existing Systems
1. **Local Save System** (`GameSerializer`)
   - Saves to `~/.veyrm/saves/` as JSON files
   - Slot-based saves (1-9) and auto-saves (-1, -2, -3)
   - No user association

2. **Database System** (Partially Implemented)
   - Authentication via `users` table
   - Save games via `save_games` table
   - Cloud sync via `CloudSaveService`
   - Conditional compilation with `ENABLE_DATABASE`

3. **Hybrid Menu System**
   - Guest mode (local saves)
   - Authenticated mode (database saves)
   - Different menu options based on login state

## Migration Steps

### Phase 1: Remove Conditional Compilation
1. **CMakeLists.txt Changes**
   ```cmake
   # Make PostgreSQL mandatory
   find_package(PostgreSQL REQUIRED)
   # Remove ENABLE_DATABASE option
   # Remove all conditional compilation
   ```

2. **Remove `#ifdef ENABLE_DATABASE` blocks**
   - `src/main.cpp` - ~50 occurrences
   - `include/game_serializer.h` - Cloud service integration
   - `src/save_load_screen.cpp` - Database save checks
   - All other files with conditional database code

### Phase 2: Redirect Save System
1. **Update `GameSerializer` class**
   ```cpp
   class GameSerializer {
       // Remove all file-based operations
       // Delegate to SaveGameRepository
       bool saveGame(int slot) {
           return save_repo->saveGame(user_id, slot, serializeGameState());
       }
       bool loadGame(int slot) {
           auto data = save_repo->loadGame(user_id, slot);
           return deserializeGameState(data);
       }
   };
   ```

2. **Update `SaveLoadScreen`**
   - Remove local file checks
   - Always query database for saves
   - Show only saves for logged-in user

### Phase 3: Make Authentication Mandatory
1. **Change Initial Game State**
   ```cpp
   // In GameManager constructor
   GameManager::GameManager(MapType initial_map)
       : current_state(GameState::LOGIN),  // Changed from MENU
         previous_state(GameState::LOGIN),
   ```

2. **Update Main Flow**
   ```cpp
   void runInterface() {
       // Check if user is authenticated
       if (!isAuthenticated()) {
           // Launch login screen immediately
           auto result = login_screen->run();
           if (result != LoginScreen::Result::SUCCESS) {
               // Exit if login fails
               return;
           }
       }
       // Continue to main menu only after authentication
   }
   ```

3. **Remove Guest Options**
   - Remove "New Game (Guest)" menu option
   - Remove "Continue (Local)" menu option
   - Remove all guest-mode code paths

### Phase 4: Update Menu System
1. **Simplify Menu Structure**
   ```cpp
   // Only authenticated menu, no guest options
   menu_entries = {
       "New Game",
       "Continue",
       "Cloud Saves",
       "Leaderboards",
       "Settings",
       "Profile",
       "Logout",
       "About",
       "Quit"
   };
   ```

2. **Add Account Management**
   - Profile screen for changing password
   - Account deletion option
   - Session management

### Phase 5: Database Schema Finalization
1. **Ensure Complete Save Data**
   ```sql
   -- save_games table already has:
   -- user_id, slot, save_data (JSONB), timestamps
   -- Add if needed:
   ALTER TABLE save_games ADD COLUMN IF NOT EXISTS
       game_version VARCHAR(20),
       character_name VARCHAR(100),
       character_level INTEGER;
   ```

2. **Add Migration for Existing Saves**
   - One-time migration script to import local saves
   - Associate with user accounts where possible

## Implementation Order

1. **Day 1: Foundation**
   - Create backup of current code
   - Make PostgreSQL required in CMakeLists.txt
   - Start removing ENABLE_DATABASE conditionals

2. **Day 2: Authentication Flow**
   - Change initial state to LOGIN
   - Implement login-first flow
   - Remove guest menu options

3. **Day 3: Save System**
   - Redirect GameSerializer to use database
   - Update SaveLoadScreen for database-only
   - Remove file-based save code

4. **Day 4: Testing & Polish**
   - Test full flow: login → menu → game → save → load
   - Handle edge cases (connection loss, etc.)
   - Add proper error messages

## Benefits

1. **Single Source of Truth**
   - No sync conflicts between local and cloud
   - Consistent data across devices
   - Simplified backup strategy

2. **Better User Experience**
   - Automatic cloud saves
   - Access saves from any device
   - No manual save file management

3. **Enhanced Features**
   - Leaderboards with real user data
   - User profiles and statistics
   - Social features potential

4. **Simplified Codebase**
   - Remove conditional compilation complexity
   - Single save/load path
   - Easier to maintain and debug

## Risks & Mitigations

1. **Internet Requirement**
   - Risk: Game unplayable without internet
   - Mitigation: Local cache for offline play (sync when online)

2. **Database Downtime**
   - Risk: Server issues prevent playing
   - Mitigation: Graceful degradation, clear error messages

3. **Data Migration**
   - Risk: Loss of existing local saves
   - Mitigation: Migration tool, backup before conversion

4. **Performance**
   - Risk: Network latency on save/load
   - Mitigation: Async operations, progress indicators

## Testing Plan

1. **Unit Tests**
   - Authentication flow
   - Save/load operations
   - Error handling

2. **Integration Tests**
   - Full login → play → save → logout → login → load cycle
   - Network failure scenarios
   - Concurrent user sessions

3. **User Acceptance Testing**
   - Migration of existing saves
   - Performance under load
   - Error recovery

## Rollback Plan

If issues arise:
1. Git revert to previous version
2. Re-enable local saves temporarily
3. Keep ENABLE_DATABASE flag for gradual rollout
4. Database remains backward compatible

## Success Metrics

- Zero data loss during migration
- Login → game start < 3 seconds
- Save/load operations < 1 second
- 99.9% uptime for auth service
- User satisfaction maintained or improved