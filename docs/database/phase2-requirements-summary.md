# Phase 2: Authentication - Requirements Summary

## 🎯 **What Needs to Be Done for Phase 2: Authentication**

### Overview
Phase 2 builds secure user authentication on top of the PostgreSQL foundation from Phase 1. This enables cloud saves, leaderboards, and social features while maintaining game accessibility.

---

## ✅ **Requirements Checklist**

### 1. **Database Schema Design**
- [ ] Design `players` table with secure user data storage
- [ ] Create `player_sessions` table for session management
- [ ] Add `password_reset_tokens` and `email_verification_tokens` tables
- [ ] Update existing tables (`save_games`, `leaderboards`) to link to players
- [ ] Add proper indexes for performance
- [ ] Create database migration scripts

### 2. **PlayerRepository Implementation**
- [ ] Create `Player` model with JSON serialization
- [ ] Implement `PlayerRepository` class extending `RepositoryBase<Player>`
- [ ] Add user-specific methods: `findByUsername()`, `findByEmail()`
- [ ] Implement validation methods: `usernameExists()`, `emailExists()`
- [ ] Add account management methods: `updatePassword()`, `verifyEmail()`

### 3. **Authentication Service**
- [ ] Create `AuthenticationService` class with secure password hashing
- [ ] Implement user registration with input validation
- [ ] Add secure login functionality with rate limiting
- [ ] Create password change and reset functionality
- [ ] Add email verification system (tokens)
- [ ] Implement Argon2id password hashing (or bcrypt as fallback)

### 4. **Session Management System**
- [ ] Create `PlayerSession` model for session data
- [ ] Implement `SessionRepository` for session CRUD operations
- [ ] Design secure session token generation and validation
- [ ] Add refresh token system for "Remember Me" functionality
- [ ] Implement session cleanup for expired sessions
- [ ] Add session security features (IP validation, user agent tracking)

### 5. **UI Components**
- [ ] Create `AuthenticationScreen` component with FTXUI
- [ ] Design login form with username/email + password fields
- [ ] Build registration form with validation and confirmation
- [ ] Add forgot password screen for password reset
- [ ] Integrate authentication screens into main menu flow
- [ ] Implement real-time form validation and error display

### 6. **Remember Me Functionality**
- [ ] Implement long-lived refresh tokens (30-90 days)
- [ ] Add automatic login check on game startup
- [ ] Create token rotation system for security
- [ ] Add UI toggle for "Remember Me" option
- [ ] Implement "Log Out All Devices" functionality

### 7. **Security Implementation**
- [ ] Add rate limiting for login attempts and registration
- [ ] Implement input validation and sanitization
- [ ] Create secure random token generation
- [ ] Add password strength requirements and validation
- [ ] Implement protection against SQL injection and XSS
- [ ] Add session security measures (token expiration, rotation)

### 8. **Testing & Quality Assurance**
- [ ] Write unit tests for `PlayerRepository` CRUD operations
- [ ] Test `AuthenticationService` registration and login flows
- [ ] Create integration tests for end-to-end authentication
- [ ] Add security tests (rate limiting, invalid inputs, edge cases)
- [ ] Test session management and "Remember Me" functionality
- [ ] Verify UI components work correctly with various inputs

---

## 🛠️ **Technical Implementation Details**

### **Database Schema**
```sql
-- Key tables to implement:
players (id, username, email, password_hash, salt, ...)
player_sessions (id, player_id, session_token, expires_at, ...)
password_reset_tokens (id, player_id, token, expires_at, ...)
email_verification_tokens (id, player_id, token, expires_at, ...)
```

### **Core Classes**
```cpp
class Player { /* User account model */ };
class PlayerRepository : public RepositoryBase<Player> { /* User CRUD */ };
class AuthenticationService { /* Login/register logic */ };
class PlayerSession { /* Session model */ };
class SessionRepository : public RepositoryBase<PlayerSession> { /* Session CRUD */ };
class AuthenticationScreen : public ftxui::ComponentBase { /* UI component */ };
```

### **Security Requirements**
- **Password Hashing**: Argon2id with proper parameters
- **Session Tokens**: Cryptographically secure random tokens
- **Rate Limiting**: Max 5 login attempts per IP/username
- **Input Validation**: Username (3-50 chars), email format, password strength
- **Token Expiration**: Sessions 4 hours, refresh tokens 30 days

### **UI Flow**
1. **Main Menu** → Add "Login" and "Create Account" options
2. **Login Screen** → Username/email + password + "Remember Me"
3. **Registration Screen** → Username + email + password + confirm password
4. **Profile Screen** → Account details + logout option
5. **Error Handling** → Clear, user-friendly error messages

---

## 📋 **Build System Integration**

### **New Build Commands**
```bash
./build.sh db migrate-auth    # Run authentication schema migration
./build.sh db create-admin    # Create test admin user
./build.sh db test-auth      # Run authentication-specific tests
./build.sh db reset-auth     # Reset authentication tables only
```

### **Configuration**
- Add authentication settings to `config.yml`
- Environment variables for JWT secrets, session lifetimes
- Password requirements configuration
- Rate limiting settings

---

## 🔄 **Integration with Existing Systems**

### **ECS Integration**
- Link `save_games` table to authenticated users via `player_id`
- Update `PersistenceSystem` to handle user-specific saves
- Add player authentication checks in save/load operations

### **UI Integration**
- Extend main menu to include authentication options
- Add user profile display in game UI
- Show authentication status in status bar

### **Data Migration**
- Migrate existing anonymous save files to authenticated users
- Preserve local saves for offline play
- Handle conflicts between local and cloud saves

---

## ⚠️ **Important Considerations**

### **User Experience**
- **Optional Authentication**: Game remains fully playable without accounts
- **Offline Mode**: All features work offline, sync when available
- **Simple Registration**: Minimal required fields, clear benefits
- **Error Recovery**: Clear password reset and account recovery flows

### **Security**
- **No Plaintext Passwords**: All passwords properly hashed with salt
- **Secure Sessions**: Proper token generation and expiration
- **Rate Limiting**: Prevent brute force and abuse
- **Input Validation**: Protect against injection and XSS attacks

### **Performance**
- **Database Indexes**: Optimize queries with proper indexing
- **Session Cleanup**: Regular cleanup of expired sessions
- **Connection Pooling**: Reuse database connections efficiently
- **Caching**: Consider session caching for performance

---

## 🎯 **Success Criteria**

- [ ] **Secure Registration**: Users can create accounts with strong passwords
- [ ] **Reliable Login**: Users can log in with username/email and password
- [ ] **Session Management**: Secure sessions with proper expiration and refresh
- [ ] **Remember Me**: Persistent login across game restarts
- [ ] **Security**: All authentication follows security best practices
- [ ] **User Experience**: Intuitive UI with clear error messages
- [ ] **Testing**: Comprehensive test coverage for all authentication features
- [ ] **Documentation**: Complete API documentation and usage guides
- [ ] **Integration**: Seamless integration with existing game systems
- [ ] **Performance**: Fast authentication with minimal latency

---

## 📖 **Documentation References**

- **Detailed Implementation**: [Phase 2 Authentication Plan](./phase2-authentication-plan.md)
- **Database Schema**: See Phase 2 plan for complete SQL schema
- **Security Guidelines**: Industry best practices for authentication
- **UI Mockups**: Component designs and user flow diagrams
- **API Documentation**: Complete interface specifications

---

**🚀 Ready to implement secure, user-friendly authentication for Veyrm!**