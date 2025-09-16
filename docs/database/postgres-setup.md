# PostgreSQL Database Setup

## Overview
Veyrm uses PostgreSQL for persistent game data storage including player profiles, save games, leaderboards, and telemetry data.

## Docker Development Environment

### Container Configuration

The PostgreSQL development environment runs in Docker with the following setup:

#### Environment Variables (.env)
```bash
# PostgreSQL Configuration
POSTGRES_DB=veyrm_db
POSTGRES_USER=veyrm_admin
POSTGRES_PASSWORD=changeme_to_secure_password
POSTGRES_PORT=5432

# PgAdmin Configuration (optional - used with --profile tools)
PGADMIN_EMAIL=admin@veyrm.local
PGADMIN_PASSWORD=changeme_to_secure_password
PGADMIN_PORT=5050
```

#### Dockerfile
- Base image: `postgres:16-alpine`
- Database name: `veyrm_db`
- Default user: `veyrm_admin`
- Initialization scripts: `/docker-entrypoint-initdb.d/`
- Health check: pg_isready every 10s
- Exposed port: 5432

#### Docker Compose Services

**PostgreSQL Service:**
- Container name: `veyrm-postgres`
- Port: 5432 (configurable via POSTGRES_PORT)
- Volumes:
  - `postgres_data`: Persistent data storage
  - `./init`: SQL initialization scripts (read-only)
  - `./backups`: Backup directory
- Network: `veyrm-network`
- Auto-restart: unless-stopped

**PgAdmin Service (optional):**
- Container name: `veyrm-pgadmin`
- Port: 5050 (configurable via PGADMIN_PORT)
- Profile: tools (run with `docker-compose --profile tools up`)
- Web interface for database management

### Starting the Database

```bash
# Start PostgreSQL only
docker-compose up -d postgres

# Start with PgAdmin
docker-compose --profile tools up -d

# View logs
docker-compose logs -f postgres

# Stop services
docker-compose down

# Stop and remove volumes (WARNING: deletes data)
docker-compose down -v
```

### Connection Details

From the host machine:
- Host: `localhost`
- Port: `5432`
- Database: `veyrm_db`
- Username: `veyrm_admin`
- Password: `changeme_to_secure_password`

From within Docker network:
- Host: `veyrm-postgres`
- Port: `5432`
- Database: `veyrm_db`
- Username: `veyrm_admin`
- Password: `changeme_to_secure_password`

### Database Schema

The database will be initialized with SQL scripts from the `./init` directory. These scripts should create:

1. **Player Profile Tables**
   - User accounts
   - Character profiles
   - Achievement tracking

2. **Save Game Tables**
   - Game state serialization
   - Map seeds and generation parameters
   - Entity positions and states

3. **Leaderboard Tables**
   - High scores
   - Speed run records
   - Daily/weekly/all-time rankings

4. **Telemetry Tables**
   - Gameplay metrics
   - Error logging
   - Performance data

### Backup and Restore

```bash
# Backup database
docker exec veyrm-postgres pg_dump -U veyrm_admin veyrm_db > ./backups/backup_$(date +%Y%m%d_%H%M%S).sql

# Restore database
docker exec -i veyrm-postgres psql -U veyrm_admin veyrm_db < ./backups/backup_file.sql
```

### Security Notes

⚠️ **Important**: The default passwords in the configuration are for development only. Before deploying:
1. Change all passwords to secure values
2. Use environment-specific .env files
3. Never commit .env files with production credentials
4. Consider using Docker secrets for production

## C++ Integration

### Required Libraries

To connect from C++, we'll use:
- **libpqxx**: Official C++ client library for PostgreSQL
- **Connection pooling**: For efficient connection management

### CMake Configuration

Add to CMakeLists.txt:
```cmake
find_package(PostgreSQL REQUIRED)
find_package(pqxx REQUIRED)

target_link_libraries(veyrm
    PostgreSQL::PostgreSQL
    pqxx
)
```

### Connection Example

```cpp
#include <pqxx/pqxx>

class DatabaseConnection {
private:
    std::unique_ptr<pqxx::connection> conn;

public:
    void connect() {
        std::string connection_string =
            "host=localhost "
            "port=5432 "
            "dbname=veyrm_db "
            "user=veyrm_admin "
            "password=changeme_to_secure_password";

        conn = std::make_unique<pqxx::connection>(connection_string);
    }
};
```

## Development Workflow

1. **Start Docker container**: `docker-compose up -d`
2. **Run migrations**: Apply any schema changes
3. **Develop locally**: Game connects to localhost:5432
4. **Test changes**: Use PgAdmin or psql to verify data
5. **Stop container**: `docker-compose down` when done

## Troubleshooting

### Container won't start
- Check if port 5432 is already in use
- Verify Docker daemon is running
- Check logs: `docker-compose logs postgres`

### Connection refused
- Ensure container is running: `docker ps`
- Check firewall settings
- Verify connection string parameters

### Permission denied
- Check file permissions on volumes
- Ensure postgres user has correct privileges
- Review Docker user mappings