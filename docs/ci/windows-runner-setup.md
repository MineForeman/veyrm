# Fix Windows GitLab Runner - Step by Step

## The Problem

The volume mount `\\.\pipe\docker_engine:\\.\pipe\docker_engine` in docker-compose.yml is causing the error.

## Solution

### Step 1: Stop the current runner

```powershell
cd C:\Users\neil\repos\gitlab-runner
docker-compose down
```

### Step 2: Update docker-compose.yml

Remove the Docker pipe volume mount. Your docker-compose.yml should look like:

```yaml
services:
  gitlab-runner:
    image: gitlab-runner-windows:latest
    container_name: gitlab-runner
    restart: unless-stopped
    volumes:
      - C:\Users\neil\repos\gitlab-runner\config:C:\etc\gitlab-runner
      # REMOVE THIS LINE: - \\.\pipe\docker_engine:\\.\pipe\docker_engine
    environment:
      - RUNNER_EXECUTOR=docker-windows
      - CI_SERVER_URL=http://horse.local:8929
      - COMPOSE_CONVERT_WINDOWS_PATHS=1
    extra_hosts:
      - "horse.local:192.168.1.5"
    networks:
      - gitlab-runner-network

volumes:
  gitlab-runner-config:
    name: gitlab-runner-config
    external: true

networks:
  gitlab-runner-network:
    driver: nat
```

### Step 3: Check/Update config.toml

Edit `C:\Users\neil\repos\gitlab-runner\config\config.toml` and make sure the runner configuration doesn't have problematic volumes:

```toml
[[runners]]
  name = "Windows Docker Runner"
  url = "http://horse.local:8929"
  token = "YOUR_TOKEN"
  executor = "docker-windows"
  [runners.docker]
    tls_verify = false
    image = "mcr.microsoft.com/windows/servercore:ltsc2022"
    privileged = false
    disable_cache = false
    # Remove or comment out any volumes with pipe mounts
    # volumes = ["/cache"]  # This is OK
    # volumes = ["\\.\pipe\docker_engine:\\.\pipe\docker_engine"]  # REMOVE THIS
    shm_size = 0
```

### Step 4: Restart the runner

```powershell
docker-compose up -d
```

### Step 5: Verify it's working

```powershell
docker logs gitlab-runner
```

## Alternative: Use Shell Executor Instead

If the Docker executor continues to have issues, re-register the runner with shell executor:

### Option A: Register a new shell runner alongside

```powershell
docker exec -it gitlab-runner powershell
cd C:\gitlab-runner
.\gitlab-runner.exe register `
  --non-interactive `
  --url "http://horse.local:8929" `
  --registration-token "YOUR_TOKEN" `
  --executor "shell" `
  --description "Windows Shell Runner" `
  --tag-list "windows,shell"
```

### Option B: Completely switch to shell executor

1. Unregister the docker-windows runner
2. Register with shell executor
3. Update the GitLab CI to not use Docker images for Windows

## For GitLab CI

Once fixed, the Windows build in `.gitlab-ci.yml` can be updated to actually build:

```yaml
build:windows:
  stage: build
  tags:
    - windows
  script:
    - echo "Windows build starting"
    - mkdir build -ErrorAction SilentlyContinue
    - cd build
    - cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    - cmake --build . -j
  allow_failure: true
```

## Notes

- The docker-windows executor runs containers on the Windows host
- It doesn't need access to the Docker socket/pipe like Linux Docker-in-Docker setups
- The runner communicates with Docker internally using Windows APIs
