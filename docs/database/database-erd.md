# Veyrm Database Schema (ERD)

## Overview

This document describes the complete PostgreSQL database schema for Veyrm, including all tables, relationships, indexes, and constraints. The database supports user authentication, cloud saves, game data, analytics, and future features like leaderboards and social functionality.

## Database Design Principles

- **Security First**: All user data properly protected with salted password hashes
- **Scalability**: Designed for multiple users with efficient indexing
- **Flexibility**: JSON storage for complex game state
- **Performance**: Connection pooling and optimized queries
- **Data Integrity**: Foreign key constraints and proper normalization

## Core Schema Diagram

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     USERS       │    │  USER_SESSIONS  │    │ USER_PROFILES   │
├─────────────────┤    ├─────────────────┤    ├─────────────────┤
│ id (PK)         │◄──┤ user_id (FK)    │    │ user_id (FK)    │──┐
│ username (UQ)   │    │ session_token   │    │ display_name    │  │
│ email (UQ)      │    │ refresh_token   │    │ avatar_url      │  │
│ password_hash   │    │ expires_at      │    │ preferences     │  │
│ salt            │    │ ip_address      │    │ created_at      │  │
│ email_verified  │    │ user_agent      │    │ updated_at      │  │
│ account_locked  │    │ created_at      │    └─────────────────┘  │
│ created_at      │    │ last_used       │                        │
│ updated_at      │    └─────────────────┘                        │
│ last_login      │                                               │
└─────────────────┘                                               │
        │                                                         │
        │                                                         │
        │                                                         │
        ▼                                                         │
┌─────────────────┐    ┌─────────────────┐                        │
│   SAVE_GAMES    │    │  SAVE_BACKUPS   │                        │
├─────────────────┤    ├─────────────────┤                        │
│ id (PK, UUID)   │◄──┤ save_id (FK)    │                        │
│ user_id (FK)    │    │ backup_data     │                        │
│ slot_number     │    │ backup_reason   │                        │
│ character_name  │    │ created_at      │                        │
│ character_level │    └─────────────────┘                        │
│ map_depth       │                                               │
│ play_time       │    ┌─────────────────┐                        │
│ save_data (JSON)│    │ SAVE_CONFLICTS │                        │
│ save_version    │    ├─────────────────┤                        │
│ created_at      │◄──┤ save_id (FK)    │                        │
│ updated_at      │    │ conflict_data   │                        │
│ sync_status     │    │ resolution      │                        │
└─────────────────┘    │ created_at      │                        │
                       └─────────────────┘                        │
                                                                  │
┌─────────────────┐    ┌─────────────────┐                        │
│  LEADERBOARDS   │    │   TELEMETRY     │                        │
├─────────────────┤    ├─────────────────┤                        │
│ id (PK)         │    │ id (PK)         │                        │
│ user_id (FK)    │────┤ user_id (FK)    │◄───────────────────────┘
│ category        │    │ event_type      │
│ score           │    │ event_data      │
│ character_name  │    │ session_id      │
│ achieved_at     │    │ created_at      │
│ game_version    │    └─────────────────┘
└─────────────────┘
```

## Security & Authentication Tables

### users
**Primary user accounts and authentication data**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | INTEGER | PRIMARY KEY, AUTO_INCREMENT | Unique user identifier |
| username | VARCHAR(50) | NOT NULL, UNIQUE | User's chosen username |
| email | VARCHAR(255) | NOT NULL, UNIQUE | User's email address |
| password_hash | VARCHAR(255) | NOT NULL | SHA256 hashed password |
| salt | VARCHAR(255) | NOT NULL | Password salt for security |
| email_verified | BOOLEAN | DEFAULT FALSE | Email verification status |
| account_locked | BOOLEAN | DEFAULT FALSE | Account lock status |
| failed_login_attempts | INTEGER | DEFAULT 0 | Failed login counter |
| last_failed_login | TIMESTAMP | NULL | Last failed login time |
| created_at | TIMESTAMP | DEFAULT NOW() | Account creation time |
| updated_at | TIMESTAMP | DEFAULT NOW() | Last account update |
| last_login | TIMESTAMP | NULL | Last successful login |

**Indexes:**
- Primary key on `id`
- Unique index on `username`
- Unique index on `email`

### user_sessions
**Session management for authentication**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | INTEGER | PRIMARY KEY, AUTO_INCREMENT | Session identifier |
| user_id | INTEGER | FOREIGN KEY → users(id) | Session owner |
| session_token | VARCHAR(255) | NOT NULL, UNIQUE | Session token |
| refresh_token | VARCHAR(255) | UNIQUE | Refresh token for renewal |
| expires_at | TIMESTAMP | NOT NULL | Session expiration time |
| refresh_expires_at | TIMESTAMP | NULL | Refresh token expiration |
| ip_address | INET | NULL | Client IP address |
| user_agent | TEXT | NULL | Client browser/app info |
| remember_me | BOOLEAN | DEFAULT FALSE | Long-lived session flag |
| created_at | TIMESTAMP | DEFAULT NOW() | Session creation time |
| last_used | TIMESTAMP | DEFAULT NOW() | Last session activity |
| revoked | BOOLEAN | DEFAULT FALSE | Session revocation status |
| revoked_at | TIMESTAMP | NULL | Session revocation time |

**Indexes:**
- Primary key on `id`
- Unique index on `session_token`
- Unique index on `refresh_token`

### user_profiles
**Extended user profile information**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| user_id | INTEGER | PRIMARY KEY, FOREIGN KEY → users(id) | Profile owner |
| display_name | VARCHAR(100) | NULL | User's display name |
| avatar_url | VARCHAR(500) | NULL | Profile picture URL |
| bio | TEXT | NULL | User biography |
| preferences | JSONB | DEFAULT '{}' | User preferences |
| stats | JSONB | DEFAULT '{}' | Player statistics |
| created_at | TIMESTAMP | DEFAULT NOW() | Profile creation time |
| updated_at | TIMESTAMP | DEFAULT NOW() | Last profile update |

## Game Data Tables

### save_games
**Cloud save storage with metadata**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | UUID | PRIMARY KEY, DEFAULT gen_random_uuid() | Save file identifier |
| user_id | INTEGER | FOREIGN KEY → users(id) | Save owner |
| slot_number | INTEGER | NOT NULL | Save slot (1-9, or negative for auto-saves) |
| character_name | VARCHAR(100) | NULL | Character name |
| character_level | INTEGER | NULL | Character level |
| map_depth | INTEGER | NULL | Current map depth |
| play_time | INTEGER | NULL | Total play time (seconds) |
| turn_count | INTEGER | NULL | Total game turns |
| save_data | JSONB | NOT NULL | Complete game state (JSON) |
| save_version | VARCHAR(20) | NOT NULL | Save format version |
| game_version | VARCHAR(20) | NOT NULL | Game version that created save |
| created_at | TIMESTAMP | DEFAULT NOW() | Save creation time |
| updated_at | TIMESTAMP | DEFAULT NOW() | Last save update |
| last_played_at | TIMESTAMP | DEFAULT NOW() | Last play session |
| device_id | VARCHAR(100) | NULL | Device identifier |
| device_name | VARCHAR(100) | NULL | Human-readable device name |
| sync_status | VARCHAR(20) | DEFAULT 'synced' | Synchronization status |

**Indexes:**
- Primary key on `id`
- Unique index on `(user_id, slot_number)`
- Index on `user_id`
- Index on `updated_at`

**Constraints:**
- Unique constraint on `(user_id, slot_number)`
- Check constraint: `slot_number BETWEEN -3 AND 9`

### save_backups
**Automatic backups of save games**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | UUID | PRIMARY KEY, DEFAULT gen_random_uuid() | Backup identifier |
| save_id | UUID | FOREIGN KEY → save_games(id) | Original save reference |
| backup_data | JSONB | NOT NULL | Backup save data |
| backup_reason | VARCHAR(50) | NOT NULL | Reason for backup |
| created_at | TIMESTAMP | DEFAULT NOW() | Backup creation time |

### save_conflicts
**Save conflict resolution tracking**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | UUID | PRIMARY KEY, DEFAULT gen_random_uuid() | Conflict identifier |
| save_id | UUID | FOREIGN KEY → save_games(id) | Conflicted save |
| conflict_data | JSONB | NOT NULL | Conflict information |
| resolution | VARCHAR(50) | NULL | How conflict was resolved |
| resolved_at | TIMESTAMP | NULL | Resolution timestamp |
| created_at | TIMESTAMP | DEFAULT NOW() | Conflict detection time |

## Game Content Tables

### monsters
**Monster definitions loaded from JSON**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | VARCHAR(50) | PRIMARY KEY | Monster type identifier |
| name | VARCHAR(100) | NOT NULL | Monster display name |
| glyph | CHAR(1) | NOT NULL | Display character |
| color_id | VARCHAR(20) | FOREIGN KEY → colors(id) | Display color |
| max_hp | INTEGER | NOT NULL | Maximum hit points |
| attack | INTEGER | NOT NULL | Attack value |
| defense | INTEGER | NOT NULL | Defense value |
| speed | INTEGER | DEFAULT 100 | Movement speed percentage |
| ai_type | VARCHAR(20) | DEFAULT 'basic' | AI behavior type |
| data | JSONB | DEFAULT '{}' | Additional monster data |

### items
**Item definitions loaded from JSON**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | VARCHAR(50) | PRIMARY KEY | Item type identifier |
| name | VARCHAR(100) | NOT NULL | Item display name |
| description | TEXT | NULL | Item description |
| glyph | CHAR(1) | NOT NULL | Display character |
| color_id | VARCHAR(20) | FOREIGN KEY → colors(id) | Display color |
| item_type | VARCHAR(20) | NOT NULL | Item category |
| value | INTEGER | DEFAULT 0 | Gold value |
| stackable | BOOLEAN | DEFAULT FALSE | Can stack in inventory |
| data | JSONB | DEFAULT '{}' | Additional item data |

### colors
**Color definitions for display**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | VARCHAR(20) | PRIMARY KEY | Color identifier |
| name | VARCHAR(50) | NOT NULL | Color name |
| hex_code | CHAR(7) | NULL | Hex color code |
| ansi_code | INTEGER | NULL | ANSI color code |

### abilities
**Ability/skill definitions**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | VARCHAR(50) | PRIMARY KEY | Ability identifier |
| name | VARCHAR(100) | NOT NULL | Ability display name |
| description | TEXT | NULL | Ability description |
| effect_type | VARCHAR(20) | NOT NULL | Type of effect |
| data | JSONB | DEFAULT '{}' | Ability parameters |

### tags
**Tagging system for content**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | VARCHAR(50) | PRIMARY KEY | Tag identifier |
| name | VARCHAR(100) | NOT NULL | Tag display name |
| description | TEXT | NULL | Tag description |
| category | VARCHAR(20) | NULL | Tag category |

## Many-to-Many Relationship Tables

### monster_abilities
**Abilities assigned to monsters**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| monster_id | VARCHAR(50) | FOREIGN KEY → monsters(id) | Monster reference |
| ability_id | VARCHAR(50) | FOREIGN KEY → abilities(id) | Ability reference |

**Primary Key:** `(monster_id, ability_id)`

### monster_tags
**Tags assigned to monsters**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| monster_id | VARCHAR(50) | FOREIGN KEY → monsters(id) | Monster reference |
| tag_id | VARCHAR(50) | FOREIGN KEY → tags(id) | Tag reference |

**Primary Key:** `(monster_id, tag_id)`

### item_abilities
**Abilities assigned to items**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| item_id | VARCHAR(50) | FOREIGN KEY → items(id) | Item reference |
| ability_id | VARCHAR(50) | FOREIGN KEY → abilities(id) | Ability reference |

**Primary Key:** `(item_id, ability_id)`

### item_tags
**Tags assigned to items**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| item_id | VARCHAR(50) | FOREIGN KEY → items(id) | Item reference |
| tag_id | VARCHAR(50) | FOREIGN KEY → tags(id) | Tag reference |

**Primary Key:** `(item_id, tag_id)`

## Analytics & Tracking Tables

### leaderboards
**High score tracking**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | INTEGER | PRIMARY KEY, AUTO_INCREMENT | Leaderboard entry ID |
| user_id | INTEGER | FOREIGN KEY → users(id) | Player reference |
| category | VARCHAR(50) | NOT NULL | Score category |
| score | BIGINT | NOT NULL | Score value |
| character_name | VARCHAR(100) | NULL | Character name |
| character_level | INTEGER | NULL | Character level |
| map_depth | INTEGER | NULL | Depth reached |
| play_time | INTEGER | NULL | Play time (seconds) |
| achieved_at | TIMESTAMP | DEFAULT NOW() | Achievement time |
| game_version | VARCHAR(20) | NOT NULL | Game version |
| save_data | JSONB | NULL | Optional save snapshot |

**Indexes:**
- Primary key on `id`
- Index on `(category, score DESC)`
- Index on `user_id`
- Index on `achieved_at`

### telemetry
**Gameplay analytics and metrics**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | UUID | PRIMARY KEY, DEFAULT gen_random_uuid() | Event identifier |
| user_id | INTEGER | FOREIGN KEY → users(id) NULL | User reference (optional) |
| session_id | VARCHAR(255) | NULL | Session identifier |
| event_type | VARCHAR(50) | NOT NULL | Type of event |
| event_data | JSONB | DEFAULT '{}' | Event details |
| device_info | JSONB | NULL | Device information |
| game_version | VARCHAR(20) | NOT NULL | Game version |
| created_at | TIMESTAMP | DEFAULT NOW() | Event timestamp |

**Indexes:**
- Primary key on `id`
- Index on `event_type`
- Index on `user_id`
- Index on `created_at`

## Authentication Support Tables

### password_reset_tokens
**Password reset token management**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | INTEGER | PRIMARY KEY, AUTO_INCREMENT | Token identifier |
| user_id | INTEGER | FOREIGN KEY → users(id) | User reference |
| token | VARCHAR(255) | NOT NULL, UNIQUE | Reset token |
| expires_at | TIMESTAMP | NOT NULL | Token expiration |
| used | BOOLEAN | DEFAULT FALSE | Token usage status |
| created_at | TIMESTAMP | DEFAULT NOW() | Token creation time |

### email_verification_tokens
**Email verification token management**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | INTEGER | PRIMARY KEY, AUTO_INCREMENT | Token identifier |
| user_id | INTEGER | FOREIGN KEY → users(id) | User reference |
| token | VARCHAR(255) | NOT NULL, UNIQUE | Verification token |
| expires_at | TIMESTAMP | NOT NULL | Token expiration |
| verified | BOOLEAN | DEFAULT FALSE | Verification status |
| created_at | TIMESTAMP | DEFAULT NOW() | Token creation time |

### user_login_history
**Login attempt tracking**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| id | INTEGER | PRIMARY KEY, AUTO_INCREMENT | History entry ID |
| user_id | INTEGER | FOREIGN KEY → users(id) | User reference |
| session_id | INTEGER | FOREIGN KEY → user_sessions(id) NULL | Session reference |
| ip_address | INET | NULL | Login IP address |
| user_agent | TEXT | NULL | Browser/client info |
| success | BOOLEAN | NOT NULL | Login success status |
| failure_reason | VARCHAR(100) | NULL | Failure reason if applicable |
| created_at | TIMESTAMP | DEFAULT NOW() | Login attempt time |

## Schema Migrations

### schema_migrations
**Database version tracking**

| Column | Type | Constraints | Description |
|--------|------|------------|-------------|
| version | VARCHAR(20) | PRIMARY KEY | Migration version |
| description | TEXT | NULL | Migration description |
| applied_at | TIMESTAMP | DEFAULT NOW() | Migration application time |

## JSON Schema Examples

### save_data (in save_games table)
```json
{
  "version": "1.0",
  "player": {
    "position": {"x": 10, "y": 5},
    "hp": {"current": 45, "max": 50},
    "level": 12,
    "experience": 2400,
    "gold": 150,
    "inventory": [
      {"id": "potion_healing", "quantity": 3},
      {"id": "sword_iron", "quantity": 1, "equipped": true}
    ]
  },
  "world": {
    "seed": 123456789,
    "depth": 3,
    "map_data": "...",
    "entities": [
      {
        "id": "monster_001",
        "type": "goblin",
        "position": {"x": 15, "y": 8},
        "hp": {"current": 8, "max": 12}
      }
    ]
  },
  "metadata": {
    "save_time": "2025-01-18T10:30:00Z",
    "turn_count": 1245,
    "play_duration": 3600
  }
}
```

### preferences (in user_profiles table)
```json
{
  "display": {
    "theme": "dark",
    "show_fps": false,
    "message_log": {
      "max_messages": 100,
      "visible_messages": 10
    }
  },
  "gameplay": {
    "auto_pickup": true,
    "confirm_dangerous_actions": true,
    "movement_delay": 50
  },
  "audio": {
    "master_volume": 0.8,
    "sfx_enabled": true,
    "music_enabled": true
  }
}
```

## Performance Optimizations

### Indexes
- **Primary Keys**: All tables have optimized primary keys
- **Foreign Keys**: All foreign key relationships are indexed
- **Query Optimization**: Indexes on frequently queried columns
- **Composite Indexes**: Multi-column indexes for complex queries

### Query Performance
- **Connection Pooling**: Efficient database connection management
- **Prepared Statements**: All queries use prepared statements
- **Batch Operations**: Bulk inserts and updates where applicable
- **JSON Indexing**: GIN indexes on frequently queried JSONB columns

### Storage Optimization
- **JSONB**: Binary JSON storage for better performance
- **UUID**: Distributed primary keys for save games
- **Compression**: Large JSON data automatically compressed
- **Partitioning Ready**: Schema designed for future table partitioning

## Security Features

### Data Protection
- **Password Hashing**: SHA256 with unique salts
- **Session Security**: Secure token generation and validation
- **SQL Injection Prevention**: All queries use prepared statements
- **Data Validation**: Application-level and database-level constraints

### Privacy
- **User Data Isolation**: Proper foreign key constraints
- **Audit Trail**: Login history and session tracking
- **Data Retention**: Configurable data cleanup policies
- **GDPR Ready**: Schema supports user data export/deletion

## Future Extensions

The schema is designed to support upcoming features:

### Social Features
- **Guilds**: Player organizations and group activities
- **Friends**: Friend lists and social interactions
- **Chat**: In-game messaging system
- **Achievements**: Player achievement tracking

### Advanced Gameplay
- **World State**: Persistent world changes
- **Game Entities**: Dynamic game world objects
- **Character System**: Multiple characters per user
- **Inventory System**: Detailed item management

### Analytics
- **Combat Logs**: Detailed battle tracking
- **Player Achievements**: Milestone tracking
- **Performance Metrics**: Game performance analytics
- **A/B Testing**: Feature testing support

## Maintenance

### Regular Tasks
- **Vacuum**: Regular VACUUM ANALYZE for performance
- **Index Monitoring**: Monitor index usage and effectiveness
- **Log Rotation**: Rotate and archive old telemetry data
- **Backup Strategy**: Regular automated backups

### Monitoring
- **Connection Monitoring**: Track connection pool usage
- **Query Performance**: Monitor slow queries
- **Storage Growth**: Track table and index sizes
- **Error Tracking**: Monitor database errors and warnings