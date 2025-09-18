# Documentation Archive

Historical documentation preserved for reference. This includes completed development phases, old documentation versions, and deprecated content.

## Contents

### [Development Phases](phases/)

All completed development phase documentation from the journey to MVP:

- Phase 0: Project Setup
- Phase 1-3: Foundation
- Phase 4-6: Core Systems
- Phase 7-9: Gameplay
- Phase 10-12: Items, Inventory, and Save/Load

### [Architecture Decision Records](decisions/)

Historical decisions and their rationale:

- Technology choices
- Design patterns
- Architecture changes

### [Legacy Documentation](legacy/)

Older documentation that has been superseded:

- Original specifications
- Outdated guides
- Previous organizational structures

## Why Archive?

We preserve old documentation to:

- **Maintain History** - Understand how the project evolved
- **Learn from Decisions** - See what worked and what didn't
- **Support Migration** - Help users transition from old versions
- **Enable Research** - Allow analysis of development patterns

## Using Archived Content

### ⚠️ Important Notes

- **Archived content may be outdated** - Always check current docs first
- **APIs may have changed** - Code examples might not work
- **Decisions may be reversed** - Check current architecture
- **Links may be broken** - Files may have moved

### Finding Information

1. Check current documentation first
2. Use archive for historical context
3. Verify information is still valid
4. Update if you find useful content

## Archive Organization

```
archive/
├── phases/              # Development phases
│   ├── 0.1_*.md       # Project setup
│   ├── 1.*_*.md       # Core loop
│   ├── ...            # All phases
│   └── 12.1_*.md      # Save/load
├── decisions/          # ADRs
│   ├── adr-001.md     # First decision
│   └── ...            # All decisions
└── legacy/            # Old docs
    ├── old-structure/ # Previous organization
    └── deprecated/    # Deprecated content
```

## Archived Milestones

### Phase Completions

- **Phase 0-3**: Foundation (✅ Complete)
- **Phase 4-6**: Core Systems (✅ Complete)
- **Phase 7-9**: Gameplay (✅ Complete)
- **Phase 10-12**: MVP Features (✅ Complete)

### Major Refactorings

- **2025-09-14**: Documentation restructure
- **2025-09-14**: MVP completion
- **2025-09-13**: Save system optimization

## Restoration

If you need to restore archived content:

1. Locate the file in archive
2. Copy to appropriate new location
3. Update for current standards
4. Fix broken references
5. Test any code examples

## Contributing to Archive

When archiving documentation:

1. Move to appropriate subdirectory
2. Add note about when/why archived
3. Update this README
4. Preserve file history in git

## Search Archive

To search archived content:

```bash
# Search all archived files
grep -r "search term" docs/archive/

# Find specific phase
ls docs/archive/phases/ | grep "phase_name"

# List all archived files
find docs/archive -name "*.md"
```

---

*Note: For current documentation, return to [Main Docs](../README.md)*
