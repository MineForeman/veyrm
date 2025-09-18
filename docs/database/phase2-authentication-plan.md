# Phase 2: Authentication Implementation Plan

## Overview

Phase 2 builds on the foundation established in Phase 1 to add user authentication, account management, and session handling to Veyrm. This will enable cloud saves, leaderboards, and social features in later phases.

## Goals

- **Secure User Authentication**: Modern password hashing and secure session management
- **Seamless User Experience**: Simple registration/login flow with "Remember Me" functionality
- **Foundation for Social Features**: Player accounts that support cloud saves and leaderboards
- **Privacy-First Design**: Optional account creation with clear benefits

## Requirements Overview

### Phase 2: Authentication (Week 3)

- [ ] **Database Schema**: Design user authentication tables
- [ ] **PlayerRepository**: Implement user account CRUD operations
- [ ] **AuthenticationService**: Create authentication service with password hashing
- [ ] **Session Management**: Implement secure session handling
- [ ] **UI Components**: Create login/registration screens
- [ ] **Remember Me**: Add persistent login functionality
- [ ] **Testing**: Comprehensive authentication test suite
- [ ] **Documentation**: Complete API and usage documentation

---

## Detailed Implementation Plan

### 1. Database Schema Design

#### New Tables Required

```sql
-- Users/Players table
CREATE TABLE IF NOT EXISTS players (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    salt VARCHAR(32) NOT NULL,
    display_name VARCHAR(100),
    avatar_url VARCHAR(500),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP,
    email_verified BOOLEAN DEFAULT FALSE,
    account_status VARCHAR(20) DEFAULT 'active', -- active, suspended, deleted
    preferences JSONB DEFAULT '{}',

    -- Constraints
    CONSTRAINT username_length CHECK (LENGTH(username) >= 3),
    CONSTRAINT email_format CHECK (email ~* '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$')
);

-- Sessions table for session management
CREATE TABLE IF NOT EXISTS player_sessions (
    id SERIAL PRIMARY KEY,
    player_id INTEGER REFERENCES players(id) ON DELETE CASCADE,
    session_token VARCHAR(128) UNIQUE NOT NULL,
    refresh_token VARCHAR(128) UNIQUE,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_accessed TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip_address INET,
    user_agent TEXT,
    remember_me BOOLEAN DEFAULT FALSE,
    is_active BOOLEAN DEFAULT TRUE
);

-- Password reset tokens
CREATE TABLE IF NOT EXISTS password_reset_tokens (
    id SERIAL PRIMARY KEY,
    player_id INTEGER REFERENCES players(id) ON DELETE CASCADE,
    token VARCHAR(128) UNIQUE NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    used_at TIMESTAMP,
    is_used BOOLEAN DEFAULT FALSE
);

-- Email verification tokens
CREATE TABLE IF NOT EXISTS email_verification_tokens (
    id SERIAL PRIMARY KEY,
    player_id INTEGER REFERENCES players(id) ON DELETE CASCADE,
    token VARCHAR(128) UNIQUE NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    used_at TIMESTAMP,
    is_used BOOLEAN DEFAULT FALSE
);

-- Update existing save_games table to link to players
ALTER TABLE save_games
ADD COLUMN IF NOT EXISTS player_id INTEGER REFERENCES players(id) ON DELETE SET NULL;

-- Add indexes for performance
CREATE INDEX IF NOT EXISTS idx_players_username ON players(username);
CREATE INDEX IF NOT EXISTS idx_players_email ON players(email);
CREATE INDEX IF NOT EXISTS idx_sessions_token ON player_sessions(session_token);
CREATE INDEX IF NOT EXISTS idx_sessions_player_id ON player_sessions(player_id);
CREATE INDEX IF NOT EXISTS idx_save_games_player_id ON save_games(player_id);
```

#### Schema Integration with Existing Tables

- **save_games**: Add `player_id` foreign key to link saves to user accounts
- **leaderboards**: Add `player_id` foreign key for authenticated leaderboard entries
- **telemetry**: Add optional `player_id` for user-specific analytics (with consent)

### 2. PlayerRepository Implementation

#### Interface Design

```cpp
class PlayerRepository : public RepositoryBase<Player> {
public:
    // Basic CRUD operations
    std::optional<Player> findById(const std::string& id) override;
    std::optional<Player> findByUsername(const std::string& username);
    std::optional<Player> findByEmail(const std::string& email);
    std::vector<Player> findAll() override;
    std::string create(const Player& player) override;
    bool update(const Player& player) override;
    bool remove(const std::string& id) override;

    // Authentication-specific operations
    bool usernameExists(const std::string& username);
    bool emailExists(const std::string& email);
    bool updatePassword(const std::string& playerId, const std::string& passwordHash, const std::string& salt);
    bool updateLastLogin(const std::string& playerId);
    bool verifyEmail(const std::string& playerId);

    // Account management
    bool updatePreferences(const std::string& playerId, const nlohmann::json& preferences);
    bool updateAccountStatus(const std::string& playerId, const std::string& status);
    std::vector<Player> findRecentlyActive(int days = 30);
};
```

#### Player Model

```cpp
struct Player {
    std::string id;
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string salt;
    std::string displayName;
    std::string avatarUrl;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;
    std::optional<std::chrono::system_clock::time_point> lastLogin;
    bool emailVerified;
    std::string accountStatus;
    nlohmann::json preferences;

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, id, username, email, displayName,
                                   emailVerified, accountStatus, preferences);
};
```

### 3. AuthenticationService Implementation

#### Core Features

- **Password Hashing**: Use Argon2id (recommended) or bcrypt
- **Session Management**: Secure session tokens with expiration
- **Rate Limiting**: Prevent brute force attacks
- **Input Validation**: Secure username/email/password validation

#### Interface Design

```cpp
class AuthenticationService {
private:
    PlayerRepository& playerRepo;
    SessionRepository& sessionRepo;

public:
    // Registration
    struct RegisterRequest {
        std::string username;
        std::string email;
        std::string password;
        std::string displayName;
    };

    struct RegisterResult {
        bool success;
        std::string playerId;
        std::string error;
        std::vector<std::string> validationErrors;
    };

    RegisterResult registerPlayer(const RegisterRequest& request);

    // Login
    struct LoginRequest {
        std::string usernameOrEmail;
        std::string password;
        bool rememberMe = false;
    };

    struct LoginResult {
        bool success;
        std::string sessionToken;
        std::string refreshToken;
        Player player;
        std::string error;
    };

    LoginResult loginPlayer(const LoginRequest& request);

    // Session management
    bool validateSession(const std::string& sessionToken);
    std::optional<Player> getPlayerFromSession(const std::string& sessionToken);
    bool refreshSession(const std::string& refreshToken);
    bool logoutPlayer(const std::string& sessionToken);
    bool logoutAllSessions(const std::string& playerId);

    // Password management
    bool changePassword(const std::string& playerId, const std::string& currentPassword, const std::string& newPassword);
    std::string generatePasswordResetToken(const std::string& email);
    bool resetPassword(const std::string& token, const std::string& newPassword);

    // Account verification
    std::string generateEmailVerificationToken(const std::string& playerId);
    bool verifyEmail(const std::string& token);

private:
    // Security helpers
    std::string hashPassword(const std::string& password, const std::string& salt);
    std::string generateSalt();
    bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    std::string generateSecureToken();
    bool validatePasswordStrength(const std::string& password);
    bool validateUsername(const std::string& username);
    bool validateEmail(const std::string& email);
};
```

### 4. Session Management System

#### Session Storage Strategy

- **In-Memory Cache**: Redis-like cache for active sessions (future)
- **Database Persistence**: PostgreSQL for session durability
- **Token Security**: Cryptographically secure random tokens
- **Expiration Handling**: Automatic cleanup of expired sessions

#### Session Model

```cpp
struct PlayerSession {
    std::string id;
    std::string playerId;
    std::string sessionToken;
    std::string refreshToken;
    std::chrono::system_clock::time_point expiresAt;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point lastAccessed;
    std::string ipAddress;
    std::string userAgent;
    bool rememberMe;
    bool isActive;
};
```

#### Session Repository

```cpp
class SessionRepository : public RepositoryBase<PlayerSession> {
public:
    std::optional<PlayerSession> findByToken(const std::string& token);
    std::optional<PlayerSession> findByRefreshToken(const std::string& refreshToken);
    std::vector<PlayerSession> findByPlayerId(const std::string& playerId);
    bool updateLastAccessed(const std::string& sessionId);
    bool deactivateSession(const std::string& sessionId);
    bool deactivateAllPlayerSessions(const std::string& playerId);
    int cleanupExpiredSessions();
};
```

### 5. UI Components Implementation

#### Main Menu Integration

```cpp
class AuthenticationScreen : public ftxui::ComponentBase {
public:
    enum class Mode { LOGIN, REGISTER, FORGOT_PASSWORD };

private:
    Mode currentMode = Mode::LOGIN;
    AuthenticationService& authService;

    // Form fields
    std::string username;
    std::string email;
    std::string password;
    std::string confirmPassword;
    bool rememberMe = false;

    // UI state
    std::string errorMessage;
    bool isLoading = false;

public:
    ftxui::Element Render() override;
    bool OnEvent(ftxui::Event event) override;

private:
    ftxui::Component CreateLoginForm();
    ftxui::Component CreateRegisterForm();
    ftxui::Component CreateForgotPasswordForm();
    void HandleLogin();
    void HandleRegister();
    void HandleForgotPassword();
};
```

#### Screen Flow

1. **Main Menu**: Add "Login" and "Create Account" options
2. **Login Screen**: Username/email + password + remember me
3. **Registration Screen**: Username + email + password + confirm password
4. **Profile Screen**: View/edit account details, logout option
5. **Forgot Password**: Email input for password reset

#### UI Design Patterns

- **Form Validation**: Real-time validation with clear error messages
- **Loading States**: Show loading spinners during authentication
- **Error Handling**: User-friendly error messages
- **Responsive Design**: Works well in terminal environment

### 6. Remember Me Functionality

#### Implementation Strategy

- **Extended Sessions**: Longer-lived refresh tokens for "remember me"
- **Secure Storage**: Store refresh tokens securely
- **Automatic Login**: Check for valid refresh token on startup
- **Security Limits**: Limit number of concurrent "remember me" sessions

#### Token Management

- **Session Tokens**: Short-lived (1-4 hours), for active sessions
- **Refresh Tokens**: Long-lived (30-90 days), for "remember me"
- **Token Rotation**: Refresh tokens are rotated on use
- **Revocation**: Users can revoke all "remember me" sessions

### 7. Security Considerations

#### Password Security

- **Hashing**: Argon2id with appropriate parameters
- **Salt**: Unique salt per password
- **Strength Requirements**: Minimum length, complexity rules
- **Breach Protection**: Secure storage, no plaintext passwords

#### Session Security

- **HTTPS Only**: All authentication over secure connections (future)
- **Secure Tokens**: Cryptographically random, sufficient entropy
- **IP Validation**: Optional IP address validation
- **Session Fixation**: New session ID on login
- **CSRF Protection**: Anti-CSRF tokens for state-changing operations

#### Rate Limiting

- **Login Attempts**: Limit failed login attempts per IP/username
- **Registration**: Limit account creation per IP
- **Password Reset**: Limit password reset requests

### 8. Configuration Management

#### Environment Variables

```bash
# Authentication settings
AUTH_JWT_SECRET=<secure-random-key>
AUTH_SESSION_LIFETIME_HOURS=4
AUTH_REFRESH_LIFETIME_DAYS=30
AUTH_MAX_LOGIN_ATTEMPTS=5
AUTH_LOCKOUT_DURATION_MINUTES=15

# Password requirements
AUTH_MIN_PASSWORD_LENGTH=8
AUTH_REQUIRE_UPPERCASE=true
AUTH_REQUIRE_LOWERCASE=true
AUTH_REQUIRE_NUMBERS=true
AUTH_REQUIRE_SYMBOLS=false

# Email settings (for future email verification)
SMTP_HOST=localhost
SMTP_PORT=587
SMTP_USERNAME=
SMTP_PASSWORD=
FROM_EMAIL=noreply@veyrm.game
```

#### Configuration Class

```cpp
struct AuthConfig {
    std::string jwtSecret;
    int sessionLifetimeHours = 4;
    int refreshLifetimeDays = 30;
    int maxLoginAttempts = 5;
    int lockoutDurationMinutes = 15;

    // Password requirements
    int minPasswordLength = 8;
    bool requireUppercase = true;
    bool requireLowercase = true;
    bool requireNumbers = true;
    bool requireSymbols = false;

    static AuthConfig fromEnvironment();
};
```

### 9. Testing Strategy

#### Unit Tests

- **PlayerRepository**: CRUD operations, validation
- **AuthenticationService**: Registration, login, password hashing
- **SessionRepository**: Session management, cleanup
- **Password Hashing**: Verify hash/verify functions
- **Input Validation**: Username, email, password validation

#### Integration Tests

- **End-to-End Auth Flow**: Registration → login → session validation
- **Password Reset Flow**: Request → token validation → password change
- **Session Management**: Login → refresh → logout
- **Security Tests**: Rate limiting, invalid inputs, SQL injection attempts

#### Test Data

- **Mock Users**: Test accounts with known credentials
- **Invalid Data**: Test with invalid emails, weak passwords, etc.
- **Edge Cases**: Empty fields, extremely long inputs, special characters

### 10. Build System Integration

#### Database Commands Enhancement

```bash
# Add to build.sh
./build.sh db migrate-auth    # Run authentication schema migration
./build.sh db create-admin    # Create admin user for testing
./build.sh db test-auth      # Run authentication-specific tests
./build.sh db reset-auth     # Reset authentication tables only
```

#### Conditional Compilation

```cpp
#ifdef ENABLE_AUTHENTICATION
    // Authentication code
#else
    // Stub implementations for offline mode
#endif
```

---

## Implementation Timeline

### Week 3: Phase 2 Implementation

- **Day 1-2**: Database schema design and migration
- **Day 3-4**: PlayerRepository and basic CRUD operations
- **Day 5-7**: AuthenticationService with password hashing
- **Day 8-10**: Session management system
- **Day 11-14**: UI components (login/registration screens)
- **Day 15-17**: Remember Me functionality
- **Day 18-19**: Comprehensive testing
- **Day 20-21**: Documentation and polishing

### Dependencies

- **Phase 1**: Must be complete (database foundation)
- **External Libraries**:
  - Argon2 or bcrypt for password hashing
  - Optional: JWT library for session tokens
- **Testing**: Extend existing test framework

### Success Criteria

- [ ] Users can create accounts with secure passwords
- [ ] Users can log in with username/email and password
- [ ] Sessions are managed securely with proper expiration
- [ ] "Remember Me" functionality works across game restarts
- [ ] All security best practices are implemented
- [ ] UI is intuitive and handles errors gracefully
- [ ] All authentication features are thoroughly tested
- [ ] Documentation is complete and accurate

---

## Future Enhancements (Phase 3+)

### Phase 3 Integration Points

- **Cloud Saves**: Link save games to authenticated users
- **Profile Management**: User preferences, avatar management
- **Social Features**: Friend systems, shared leaderboards

### Security Enhancements

- **Two-Factor Authentication**: TOTP support
- **OAuth Integration**: Google, GitHub, Discord login
- **Email Verification**: Complete email verification flow
- **Account Recovery**: Secure account recovery options

### Advanced Features

- **Admin Panel**: User management interface
- **Audit Logging**: Track authentication events
- **Advanced Session Management**: Device management, location tracking
- **Progressive Enhancement**: Works offline, syncs when online

---

## Risk Mitigation

### Security Risks

- **Data Breaches**: Proper password hashing, secure token storage
- **Session Hijacking**: Secure token generation, HTTPS enforcement
- **Brute Force Attacks**: Rate limiting, account lockout
- **SQL Injection**: Parameterized queries, input validation

### User Experience Risks

- **Complex Registration**: Keep registration simple and optional
- **Lost Passwords**: Clear password reset flow
- **Account Lockout**: Fair rate limiting with clear recovery
- **Offline Play**: Ensure game works without authentication

### Technical Risks

- **Database Performance**: Proper indexing, query optimization
- **Session Storage**: Efficient session cleanup, memory management
- **Password Migration**: Secure migration of existing save files
- **Backward Compatibility**: Graceful handling of legacy saves

---

This comprehensive plan provides the roadmap for implementing secure, user-friendly authentication in Veyrm while maintaining the game's accessibility and charm.
