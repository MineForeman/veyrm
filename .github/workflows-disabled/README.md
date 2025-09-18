# GitHub Actions Workflows

This repository uses several GitHub Actions workflows to automate code review, testing, and CI/CD processes.

## Workflows Overview

### 1. CI Pipeline (`ci.yml`)
**Trigger:** Push to `main` or `develop` branches, or PRs to `main`
**Purpose:** Build and test the C++ roguelike game
**Steps:**
- Install build dependencies (CMake, build-essential)
- Configure and build the project with CMake
- Run unit tests via CTest
- Verify the game executable runs without crashing

### 2. Auto Fix CI Failures (`auto-fix-ci.yml`)
**Trigger:** When the CI workflow fails on a PR
**Purpose:** Automatically fix CI failures using Claude AI
**Requirements:** `ANTHROPIC_API_KEY` secret must be configured
**Process:**
- Analyzes CI failure logs
- Creates a new branch for fixes
- Uses Claude to diagnose and fix the issues
- Can create PRs with the fixes

**Note:** Only runs on PRs from the same repository (not forks) and avoids infinite loops by checking branch names.

### 3. Claude Code Review (`claude-code-review.yml`)
**Trigger:** When a PR is opened or updated
**Purpose:** Automated code review by Claude AI
**Features:**
- Reviews code quality and best practices
- Identifies potential bugs and security issues
- Suggests performance improvements
- Checks test coverage
- Uses the `CLAUDE_CODE_OAUTH_TOKEN` secret

### 4. Claude Interactive (`claude.yml`)
**Trigger:** Comments containing `@claude` on issues or PRs
**Purpose:** Interactive assistance from Claude AI
**Use Cases:**
- Respond to questions in issue/PR comments
- Provide code suggestions
- Help with debugging
- Answer technical questions

## Required Secrets

1. **`CLAUDE_CODE_OAUTH_TOKEN`** ✅ (Already configured)
   - Used by claude.yml and claude-code-review.yml
   - Enables Claude interactions

2. **`ANTHROPIC_API_KEY`** ⚠️ (Needs to be added)
   - Required for auto-fix-ci.yml
   - Add it with: `gh secret set ANTHROPIC_API_KEY`

## Usage Examples

### Triggering CI
```bash
# Push to main/develop
git push origin main

# Create a PR
git checkout -b feature-branch
git push origin feature-branch
gh pr create
```

### Getting Claude's Help
- Comment `@claude help me fix this bug` on an issue or PR
- Claude will analyze and respond with suggestions

### Auto-fixing CI Failures
When CI fails on a PR:
1. The auto-fix workflow automatically triggers
2. Claude analyzes the failure logs
3. Creates a fix branch and attempts to resolve issues
4. May open a new PR with the fixes

## Workflow Permissions

All workflows have appropriate permissions:
- **Read:** contents, pull-requests, issues, actions
- **Write:** contents (for auto-fix), pull-requests (for auto-fix)
- **ID Token:** For potential OIDC authentication

## Best Practices

1. Always ensure tests pass locally before pushing
2. Review Claude's suggestions before merging
3. Monitor the auto-fix branches to ensure fixes are appropriate
4. Keep secrets secure and rotate them periodically