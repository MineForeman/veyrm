# Phase 3: Save System Integration Plan

## Overview

Phase 3 integrates the existing local save system with PostgreSQL database to enable cloud saves, synchronization across devices, and save backup. The system will maintain backward compatibility with local saves while adding cloud functionality for authenticated users.

---

## 🎯 **Goals**

1. **Cloud Save Storage**: Store save games in PostgreSQL for authenticated users
2. **Synchronization**: Sync saves across multiple devices
3. **Conflict Resolution**: Handle save conflicts intelligently
4. **Auto-Save to Cloud**: Automatic cloud backup of progress
5. **Offline Fallback**: Continue using local saves when offline
6. **Save Migration**: Import existing local saves to cloud

---

## 📊 **Database Schema**

### Save Games Table

```sql
CREATE TABLE IF NOT EXISTS save_games (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    slot_number INTEGER NOT NULL,  -- 1-9 for manual, -1 to -3 for auto
    character_name VARCHAR(100),
    character_level INTEGER,
    map_depth INTEGER,
    play_time INTEGER,  -- seconds
    turn_count INTEGER,

    -- Save data
    save_data JSONB NOT NULL,  -- Complete game state
    save_version VARCHAR(20) NOT NULL,
    game_version VARCHAR(20) NOT NULL,

    -- Metadata
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    -- Sync info
    device_id VARCHAR(100),
    device_name VARCHAR(100),
    sync_status VARCHAR(20) DEFAULT 'synced',  -- synced, pending, conflict

    -- Unique constraint per user/slot
    UNIQUE(user_id, slot_number)
);

CREATE INDEX idx_saves_user ON save_games(user_id);
CREATE INDEX idx_saves_slot ON save_games(user_id, slot_number);
CREATE INDEX idx_saves_updated ON save_games(updated_at);
```

### Save Conflicts Table

```sql
CREATE TABLE IF NOT EXISTS save_conflicts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    save_id UUID REFERENCES save_games(id) ON DELETE CASCADE,
    conflicting_data JSONB NOT NULL,
    device_id VARCHAR(100),
    device_name VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved BOOLEAN DEFAULT FALSE,
    resolution_type VARCHAR(50)  -- local_wins, cloud_wins, merge
);
```

### Save Backups Table

```sql
CREATE TABLE IF NOT EXISTS save_backups (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    save_id UUID REFERENCES save_games(id) ON DELETE CASCADE,
    backup_data JSONB NOT NULL,
    backup_reason VARCHAR(100),  -- auto, manual, pre_sync, pre_delete
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Keep only last 5 backups per save
CREATE INDEX idx_backup_save ON save_backups(save_id, created_at DESC);
```

---

## 🏗️ **Implementation Components**

### 1. SaveGameRepository (`include/db/save_game_repository.h`)

```cpp
class SaveGameRepository : public RepositoryBase<SaveGame> {
public:
    // CRUD operations
    std::optional<SaveGame> create(const SaveGame& save);
    std::optional<SaveGame> findById(const std::string& id);
    std::vector<SaveGame> findByUserId(int user_id);
    std::optional<SaveGame> findByUserAndSlot(int user_id, int slot);
    bool update(const SaveGame& save);
    bool deleteById(const std::string& id);

    // Sync operations
    std::vector<SaveGame> getUnsyncedSaves(int user_id);
    bool markAsSynced(const std::string& save_id);

    // Conflict resolution
    bool createConflict(const SaveConflict& conflict);
    std::vector<SaveConflict> getUnresolvedConflicts(int user_id);
    bool resolveConflict(const std::string& conflict_id, ResolutionType type);

    // Backup operations
    bool createBackup(const std::string& save_id, const std::string& reason);
    std::vector<SaveBackup> getBackups(const std::string& save_id, int limit = 5);
    bool pruneOldBackups(const std::string& save_id, int keep_count = 5);
};
```

### 2. CloudSaveService (`include/services/cloud_save_service.h`)

```cpp
class CloudSaveService {
public:
    CloudSaveService(SaveGameRepository& repo, GameSerializer& serializer);

    // Save operations
    bool saveToCloud(int slot, const json& game_data);
    bool loadFromCloud(int slot, json& game_data);

    // Sync operations
    SyncResult syncSaves(int user_id);
    bool uploadLocalSave(int slot);
    bool downloadCloudSave(int slot);

    // Conflict resolution
    ConflictResolution detectConflict(const SaveGame& local, const SaveGame& cloud);
    bool resolveConflict(const std::string& conflict_id, ResolutionStrategy strategy);

    // Auto-save
    bool enableAutoSync(int interval_seconds = 300);
    bool disableAutoSync();
    bool performAutoSave();

    // Status
    SyncStatus getSyncStatus(int slot);
    std::vector<SaveInfo> getAllSaveInfo(bool include_local = true);

private:
    bool isOnline() const;
    std::string getDeviceId() const;
    std::string getDeviceName() const;
};
```

### 3. Save System Integration Updates

#### GameSerializer Enhancement

```cpp
class GameSerializer {
public:
    // Add cloud save support
    void setCloudSaveService(CloudSaveService* service);
    void setUserId(int user_id);

    // Enhanced save/load
    bool saveGame(int slot, bool upload_to_cloud = true);
    bool loadGame(int slot, bool prefer_cloud = true);

    // Sync operations
    bool syncWithCloud();
    SyncStatus getCloudSyncStatus(int slot);

private:
    CloudSaveService* cloud_service = nullptr;
    int current_user_id = 0;
};
```

#### SaveLoadScreen Enhancement

```cpp
class SaveLoadScreen {
public:
    // Add cloud save UI elements
    void showCloudStatus(int slot);
    void showSyncProgress();
    void showConflictResolution(const SaveConflict& conflict);

    // Add sync commands
    bool handleSyncCommand();
    bool handleUploadCommand(int slot);
    bool handleDownloadCommand(int slot);

private:
    CloudIndicator getCloudIndicator(int slot);
    std::string formatSyncStatus(SyncStatus status);
};
```

---

## 🔄 **Sync Logic**

### Conflict Detection

```cpp
enum class ConflictType {
    NONE,           // No conflict
    NEWER_LOCAL,    // Local save is newer
    NEWER_CLOUD,    // Cloud save is newer
    DIVERGED        // Both modified since last sync
};

ConflictType detectConflict(const SaveGame& local, const SaveGame& cloud) {
    if (local.updated_at == cloud.updated_at) return ConflictType::NONE;
    if (local.updated_at > cloud.updated_at) return ConflictType::NEWER_LOCAL;
    if (cloud.updated_at > local.updated_at) return ConflictType::NEWER_CLOUD;
    return ConflictType::DIVERGED;
}
```

### Resolution Strategies

1. **Last Write Wins**: Most recent save overwrites
2. **Local Priority**: Always keep local changes
3. **Cloud Priority**: Always use cloud version
4. **Manual Resolution**: Let user choose
5. **Smart Merge**: Merge non-conflicting changes (future)

### Auto-Sync Algorithm

```
Every N seconds (configurable):
1. Check if online and authenticated
2. For each save slot:
   a. Compare local vs cloud timestamps
   b. If local newer: upload
   c. If cloud newer: download
   d. If conflict: mark for resolution
3. Update sync status indicators
```

---

## 🎨 **UI Changes**

### Save/Load Screen

```
╔═══════════════════════════════════════════════════════╗
║                    SAVE GAME                          ║
╠════════════════════════════════════════════════════════╣
║ Slot │ Character │ Level │ Location │ Playtime │ Cloud ║
╟──────┼───────────┼───────┼──────────┼──────────┼───────╢
║  1   │ Thorin    │  12   │ Depth 5  │ 02:34:15 │  ☁✓   ║
║  2   │ Gandalf   │  8    │ Depth 3  │ 01:15:42 │  ☁↑   ║
║  3   │ [Empty]   │  -    │    -     │    -     │  -    ║
╟──────┴───────────┴───────┴──────────┴──────────┴───────╢
║ Auto │ Thorin    │  12   │ Depth 5  │ 02:30:00 │  ☁✓   ║
╚════════════════════════════════════════════════════════╝

Cloud Status:
  ☁✓ = Synced
  ☁↑ = Upload pending
  ☁↓ = Download available
  ☁! = Conflict
  ⊗  = Offline/Local only

[S] Save  [L] Load  [D] Delete  [↑↓] Select  [C] Cloud Sync
```

### Conflict Resolution Dialog

```
╔═══════════════════════════════════════════════════════╗
║              SAVE CONFLICT DETECTED                   ║
╠════════════════════════════════════════════════════════╣
║                                                        ║
║  Local Save:                Cloud Save:               ║
║  ────────────              ─────────────              ║
║  Character: Thorin          Character: Thorin         ║
║  Level: 12                  Level: 11                 ║
║  Location: Depth 5          Location: Depth 4         ║
║  Updated: 2 hours ago       Updated: 1 hour ago       ║
║  Device: Desktop            Device: Laptop            ║
║                                                        ║
║  Choose Resolution:                                   ║
║  [1] Keep Local (newer)                              ║
║  [2] Use Cloud                                       ║
║  [3] Create backup and keep both                     ║
║                                                        ║
╚════════════════════════════════════════════════════════╝
```

---

## 🧪 **Testing Strategy**

### Unit Tests

- SaveGameRepository CRUD operations
- CloudSaveService sync logic
- Conflict detection algorithms
- Backup management

### Integration Tests

- Full save/load cycle with cloud
- Sync with conflicts
- Offline mode fallback
- Auto-save functionality

### Test Scenarios

1. Save locally, sync to cloud
2. Save on device A, load on device B
3. Concurrent saves from multiple devices
4. Network interruption during sync
5. Conflict resolution workflows

---

## 📈 **Metrics to Track**

- Save operation latency
- Sync success rate
- Conflict frequency
- Average save size
- Backup storage usage
- User engagement with cloud saves

---

## 🔒 **Security Considerations**

1. **Data Encryption**: Encrypt save data at rest
2. **Access Control**: Users can only access their own saves
3. **Rate Limiting**: Prevent save spam
4. **Size Limits**: Cap save file size (e.g., 10MB)
5. **Validation**: Validate save data structure
6. **Audit Trail**: Log all save operations

---

## 📝 **Implementation Steps**

1. ✅ Create database schema
2. ⬜ Implement SaveGameRepository
3. ⬜ Create CloudSaveService
4. ⬜ Integrate with GameSerializer
5. ⬜ Update SaveLoadScreen UI
6. ⬜ Add sync indicators
7. ⬜ Implement conflict resolution
8. ⬜ Add auto-sync timer
9. ⬜ Write tests
10. ⬜ Document API

---

## 🚀 **Success Criteria**

- [ ] Local saves work without authentication
- [ ] Authenticated users get automatic cloud backup
- [ ] Saves sync across devices within 5 minutes
- [ ] Conflicts are detected and resolved
- [ ] UI clearly shows sync status
- [ ] Offline mode works seamlessly
- [ ] No data loss during sync
- [ ] Performance impact < 100ms per save

---

*Phase 3 ready for implementation*
