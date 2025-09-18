# GitHub Workflows - Disabled

## Status: DISABLED

GitHub CI/CD workflows have been temporarily disabled and moved to `../workflows-disabled/` directory.

## Available Workflows (Currently Disabled)

The following workflows are available but disabled:

- **`ci.yml`** - Main CI pipeline with build and test
- **`auto-fix-ci.yml`** - Automated fixes for CI issues
- **`claude-code-review.yml`** - Claude-powered code review
- **`claude.yml`** - Claude integration workflow
- **`docs.yml`** - Documentation generation
- **`release.yml`** - Release automation

## Re-enabling Workflows

To re-enable GitHub Actions CI/CD:

1. **Move workflows back:**
   ```bash
   mv .github/workflows-disabled/*.yml .github/workflows/
   mv .github/workflows-disabled/README.md .github/workflows/
   ```

2. **Remove this README:**
   ```bash
   rm .github/workflows/README.md
   ```

3. **Commit and push:**
   ```bash
   git add .github/
   git commit -m "Re-enable GitHub Actions CI/CD workflows"
   git push
   ```

## Why Disabled?

Workflows were disabled on 2025-01-18 to focus on GitLab-based development workflow while preserving the GitHub Actions configurations for future use.

## Current Development Workflow

The project currently uses:
- **GitLab**: Primary remote repository
- **Local Testing**: `./build.sh test` for comprehensive testing
- **Docker**: PostgreSQL development environment
- **Manual CI/CD**: Build and test locally before commits

## Future Considerations

When re-enabling GitHub Actions:
- Ensure PostgreSQL test database is available for CI
- Update Docker setup in workflows if needed
- Verify all environment variables are configured
- Test workflows on a feature branch first