# Windows Native GitLab Runner Setup (Shell Executor)

This guide explains how to set up a native Windows GitLab Runner that executes jobs directly on the Windows machine without using Docker containers.

## Prerequisites

- Windows 10/11 or Windows Server
- Administrator access
- Build tools installed (CMake, Visual Studio or MinGW)

## Installation Steps

### 1. Download GitLab Runner

Download the Windows binary (64-bit):

```powershell
# Create directory for runner
New-Item -ItemType Directory -Path 'C:\GitLab-Runner' -Force
cd C:\GitLab-Runner

# Download runner executable
Invoke-WebRequest -Uri "https://gitlab-runner-downloads.s3.amazonaws.com/latest/binaries/gitlab-runner-windows-amd64.exe" -OutFile "gitlab-runner.exe"
```

### 2. Register the Runner with Shell Executor

#### Interactive Registration

```powershell
cd C:\GitLab-Runner
.\gitlab-runner.exe register
```

When prompted, enter:

- **GitLab instance URL:** `http://horse.local:8929` (or your GitLab URL)
- **Registration token:** Get from GitLab project → Settings → CI/CD → Runners
- **Description:** `Windows Native Runner`
- **Tags:** `windows,shell,native`
- **Executor:** `shell` ⚠️ **IMPORTANT: Choose "shell", NOT "docker-windows"!**
- **Shell:** `powershell` (recommended) or `pwsh` or `cmd`

#### Non-Interactive Registration (One Command)

```powershell
cd C:\GitLab-Runner
.\gitlab-runner.exe register `
  --non-interactive `
  --url "http://horse.local:8929" `
  --registration-token "YOUR_REGISTRATION_TOKEN" `
  --executor "shell" `
  --shell "powershell" `
  --description "Windows Native Runner" `
  --tag-list "windows,shell,native"
```

### 3. Install as Windows Service

```powershell
cd C:\GitLab-Runner

# Install service
.\gitlab-runner.exe install

# Start service
.\gitlab-runner.exe start

# Verify service is running
.\gitlab-runner.exe status
```

### 4. Verify Registration

```powershell
# List registered runners
.\gitlab-runner.exe list

# Check runner service
Get-Service gitlab-runner
```

You should see:

- **Executor:** `shell` (NOT docker or docker-windows)
- **Status:** Running

## Installing Build Tools

For the runner to build C++ projects, install these tools:

### Option A: Visual Studio Build Tools (Recommended)

```powershell
# Download and install Visual Studio Build Tools
# Visit: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022

# Or use Chocolatey:
choco install visualstudio2022buildtools
choco install visualstudio2022-workload-vctools
```

### Option B: MinGW (Lighter Alternative)

```powershell
# Install via Chocolatey
choco install mingw
choco install cmake
```

### Option C: Full Development Environment

```powershell
# Install everything needed
choco install cmake
choco install git
choco install visualstudio2022buildtools
choco install visualstudio2022-workload-vctools
```

## Troubleshooting

### If Runner Shows as docker-windows Executor

If your runner is still using docker-windows executor:

1. **Stop the runner:**

   ```powershell
   .\gitlab-runner.exe stop
   ```

2. **Unregister the wrong runner:**

   ```powershell
   .\gitlab-runner.exe unregister --all-runners
   ```

3. **Re-register with shell executor** (see Step 2 above)

4. **Restart the service:**

   ```powershell
   .\gitlab-runner.exe start
   ```

### Verify Shell Executor is Active

Check the config file at `C:\GitLab-Runner\config.toml`:

```toml
[[runners]]
  name = "Windows Native Runner"
  url = "http://horse.local:8929"
  token = "..."
  executor = "shell"  # ← Should be "shell", not "docker-windows"
  shell = "powershell"
```

### PowerShell Execution Policy

If scripts fail to run, you may need to adjust the execution policy:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope LocalMachine
```

## GitLab CI Configuration

Your `.gitlab-ci.yml` should NOT specify Docker images for Windows jobs:

```yaml
# ✅ Correct - Native Windows runner
build:windows:
  stage: build
  tags:
    - windows  # or windows,shell
  script:
    - cmake --version
    - mkdir build
    - cd build
    - cmake ..
    - cmake --build . --config Release

# ❌ Wrong - This tries to use Docker
build:windows:
  stage: build
  image: some-image  # Don't specify image for native Windows!
  tags:
    - windows
```

## Key Differences

| Executor | Runs In | Needs Docker | Can Access Host Tools |
|----------|---------|--------------|----------------------|
| docker-windows | Windows containers | Yes | No |
| shell | Directly on Windows | No | Yes |

## Benefits of Shell Executor

- ✅ No Docker overhead
- ✅ Direct access to installed tools (Visual Studio, etc.)
- ✅ Faster job startup
- ✅ No container compatibility issues
- ✅ Can access Windows-specific features
- ✅ Simpler debugging

## Security Note

The shell executor runs CI scripts directly on the host machine. Only use it for trusted projects or in isolated environments.
