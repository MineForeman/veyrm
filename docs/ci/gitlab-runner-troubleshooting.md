# Fixing Windows Docker Runner

## The Problem

The Windows Docker runner fails with:

```
ERROR: Failed to remove network for build
ERROR: Job failed: invalid volume specification: "\\.\\pipe\\docker_engine:\\.\\pipe\\docker_engine"
```

## Solution Options

### Option 1: Fix the config.toml (Recommended)

1. On the Windows machine, find the GitLab Runner config file:
   - Usually at `C:\GitLab-Runner\config.toml`

2. Look for the `[[runners]]` section and find the volumes configuration

3. Either remove the problematic volume:

   ```toml
   [[runners]]
     executor = "docker-windows"
     [runners.docker]
       # Remove or comment out this line:
       # volumes = ["\\\\.\\pipe\\docker_engine:\\\\.\\pipe\\docker_engine"]
   ```

4. Or fix it to use proper Windows format:

   ```toml
   [[runners]]
     executor = "docker-windows"
     [runners.docker]
       volumes = ["c:/gitlab-runner/cache:c:/cache"]
   ```

5. Restart the runner:

   ```powershell
   gitlab-runner restart
   ```

### Option 2: Re-register with Shell Executor (Simplest)

Instead of using docker-windows executor, use the shell executor which runs PowerShell directly:

```powershell
# Stop and unregister current runner
gitlab-runner stop
gitlab-runner unregister --all-runners

# Register with shell executor
gitlab-runner register `
  --non-interactive `
  --url "http://horse.local" `
  --registration-token "YOUR_TOKEN" `
  --executor "shell" `
  --description "Windows Shell Runner" `
  --tag-list "windows,shell"

# Start runner
gitlab-runner start
```

### Option 3: Use Linux Containers on Windows

Switch Docker Desktop to Linux containers mode and register as a regular docker executor:

```powershell
# In Docker Desktop, switch to Linux containers
# Then register runner
gitlab-runner register `
  --non-interactive `
  --url "http://horse.local" `
  --registration-token "YOUR_TOKEN" `
  --executor "docker" `
  --docker-image "alpine:latest" `
  --description "Windows Linux Docker Runner" `
  --tag-list "windows,docker,linux"
```

## For Native Windows Builds

If you want to build Windows executables, you need:

1. **Install Build Tools**:

   ```powershell
   # Install Chocolatey first
   Set-ExecutionPolicy Bypass -Scope Process -Force
   [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
   iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

   # Install build tools
   choco install -y cmake
   choco install -y visualstudio2022buildtools
   choco install -y visualstudio2022-workload-vctools
   ```

2. **Use Shell Executor** (Option 2 above)

3. **Update .gitlab-ci.yml**:

   ```yaml
   build:windows:
     stage: build
     tags:
       - windows
       - shell
     script:
       - mkdir build
       - cd build
       - cmake -G "Visual Studio 17 2022" -A x64 ..
       - cmake --build . --config Release
   ```

## Current Status

The Windows build is currently disabled in the CI pipeline. Once you fix the runner using one of the options above, update the `.gitlab-ci.yml` to use the correct tags.
