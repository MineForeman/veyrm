# Documentation Refactoring Phase 2: Structure & Naming

## Executive Summary

This document outlines a second-pass refactoring to establish a logical file/folder structure and consistent naming conventions for all documentation.

## Current Issues

### Structural Problems

1. **Mixed organizational schemes** - Some by topic, some by phase, some by type
2. **Inconsistent depth** - Some topics deeply nested, others at root
3. **Unclear hierarchy** - Not obvious what belongs where
4. **Scattered related content** - Similar topics in different locations

### Naming Inconsistencies

1. **Mixed case styles**:
   - UPPER_CASE.md (CURRENT_STATE.md)
   - PascalCase.md (DOC/VISUALIZATION_OPTIONS.md)
   - lowercase.md (some archived files)
   - Mixed separators (underscores vs hyphens)

2. **Inconsistent prefixes**:
   - Phase files: `0.1_`, `10.1_`, `12.1_`
   - No clear pattern for non-phase files

3. **Redundant naming**:
   - Files repeat directory names
   - Long, verbose filenames

## Proposed Structure

### New Directory Organization

```
docs/                           # Lowercase root (standard)
├── README.md                   # Documentation index
├── getting-started/            # Onboarding
│   ├── README.md              # Quick start guide
│   ├── installation.md        # Setup instructions
│   ├── first-game.md          # Playing tutorial
│   └── controls.md            # Control reference
│
├── guides/                     # How-to guides
│   ├── developer/             # Developer guides
│   │   ├── README.md          # Developer overview
│   │   ├── setup.md           # Dev environment setup
│   │   ├── architecture.md    # System architecture
│   │   ├── contributing.md    # Contribution guide
│   │   ├── testing.md         # Testing guide
│   │   └── debugging.md       # Debugging guide
│   │
│   ├── player/                # Player guides
│   │   ├── README.md          # Player guide index
│   │   ├── gameplay.md        # Gameplay mechanics
│   │   ├── combat.md          # Combat system
│   │   ├── items.md           # Items and inventory
│   │   └── strategies.md      # Tips and strategies
│   │
│   └── content/               # Content creation
│       ├── README.md          # Content guide index
│       ├── monsters.md        # Adding monsters
│       ├── items.md           # Adding items
│       ├── maps.md            # Map creation
│       └── modding.md         # Modding guide
│
├── reference/                  # Reference documentation
│   ├── api/                   # API reference
│   │   ├── README.md          # API overview
│   │   ├── classes/           # Class documentation
│   │   ├── systems/           # System documentation
│   │   └── generated/         # Doxygen output
│   │
│   ├── configuration/         # Configuration reference
│   │   ├── README.md          # Config overview
│   │   ├── game-config.md     # Game settings
│   │   ├── build-config.md    # Build settings
│   │   └── data-formats.md    # JSON/YAML formats
│   │
│   └── commands/              # Command reference
│       ├── README.md          # Commands overview
│       ├── build-script.md    # Build.sh commands
│       ├── cli-options.md     # CLI arguments
│       └── debug-commands.md  # Debug commands
│
├── design/                     # Design documentation
│   ├── architecture/          # Architecture docs
│   │   ├── README.md          # Architecture overview
│   │   ├── systems/           # System designs
│   │   ├── components/        # Component designs
│   │   └── diagrams/          # Architecture diagrams
│   │
│   ├── specifications/        # Technical specs
│   │   ├── README.md          # Specs overview
│   │   ├── mvp.md            # MVP specification
│   │   ├── combat.md         # Combat spec
│   │   ├── ai.md             # AI specification
│   │   └── networking.md     # Future: networking
│   │
│   └── world/                 # World design
│       ├── README.md          # Lore overview
│       ├── setting.md         # World setting
│       ├── characters.md      # NPCs and characters
│       ├── bestiary.md        # Monster lore
│       └── locations.md       # Places and dungeons
│
├── development/                # Development docs
│   ├── roadmap/               # Project roadmap
│   │   ├── README.md          # Roadmap overview
│   │   ├── completed/         # Completed milestones
│   │   ├── current.md         # Current sprint
│   │   └── future.md          # Future plans
│   │
│   ├── standards/             # Coding standards
│   │   ├── README.md          # Standards overview
│   │   ├── code-style.md      # Code style guide
│   │   ├── git-workflow.md    # Git conventions
│   │   ├── testing.md         # Testing standards
│   │   └── documentation.md   # Doc standards
│   │
│   └── performance/           # Performance docs
│       ├── README.md          # Performance overview
│       ├── benchmarks.md      # Current benchmarks
│       ├── profiling.md       # Profiling guide
│       └── optimization.md    # Optimization guide
│
├── project/                    # Project meta
│   ├── README.md              # Project overview
│   ├── changelog.md           # Version history
│   ├── license.md             # License information
│   ├── credits.md             # Contributors
│   └── status.md              # Current project status
│
└── archive/                    # Historical docs
    ├── README.md              # Archive overview
    ├── phases/                # Development phases
    ├── decisions/             # ADRs
    └── legacy/                # Old documentation
```

## Naming Conventions

### File Naming Rules

1. **Use lowercase with hyphens**: `file-name.md`
   - ✅ `getting-started.md`
   - ❌ `GETTING_STARTED.md`
   - ❌ `GettingStarted.md`

2. **README.md in every directory**: Provides overview and navigation

3. **Descriptive but concise**:
   - ✅ `combat-system.md`
   - ❌ `complete-documentation-of-the-combat-system-mechanics.md`

4. **No redundant prefixes**:
   - ✅ `guides/developer/testing.md`
   - ❌ `guides/developer/developer-testing-guide.md`

5. **Singular for single topics, plural for collections**:
   - ✅ `monster.md` (how to add one)
   - ✅ `monsters/` (directory of many)

### Directory Naming Rules

1. **Always lowercase**: `directory-name/`
2. **Use hyphens for multi-word**: `getting-started/`
3. **Plural for collections**: `guides/`, `standards/`
4. **Singular for single topics**: `performance/`, `architecture/`
5. **Action-based for processes**: `getting-started/`, `debugging/`

## Migration Plan

### Phase 1: Create New Structure ✅ COMPLETED

```bash
# Create new directory structure
mkdir -p docs/{getting-started,guides/{developer,player,content}}
mkdir -p docs/{reference/{api,configuration,commands}}
mkdir -p docs/{design/{architecture,specifications,world}}
mkdir -p docs/{development/{roadmap,standards,performance}}
mkdir -p docs/{project,archive}
```

**Status**: ✅ Complete - All directories created with proper structure and navigation READMEs

### Phase 2: Map Existing Files

| Current Location | New Location | New Name |
|-----------------|--------------|----------|
| `DOC/ARCHITECTURE.md` | `docs/guides/developer/` | `architecture.md` |
| `DOC/CONTROLS.md` | `docs/getting-started/` | `controls.md` |
| `DOC/DEVELOPER_GUIDE.md` | `docs/guides/developer/` | `setup.md` |
| `DOC/CODE_STYLE_GUIDE.md` | `docs/development/standards/` | `code-style.md` |
| `DOC/CURRENT_STATE.md` | `docs/project/` | `status.md` |
| `DOC/PERFORMANCE_BENCHMARKS.md` | `docs/development/performance/` | `benchmarks.md` |
| `DOC/API_DOCUMENTATION.md` | `docs/reference/api/` | `README.md` |
| `DOC/MVP_COMPLETE.md` | `docs/archive/` | `mvp-complete.md` |
| `DOC/WORLD/` | `docs/design/world/` | (various) |
| `DOC/PHASES/` | `docs/archive/phases/` | (keep) |
| `README.md` | `README.md` | (keep at root) |
| `CHANGELOG.md` | `docs/project/` | `changelog.md` |

### Phase 3: Update Cross-References

1. **Create reference map** of all internal links
2. **Update all links** to new paths
3. **Verify no broken links** with tooling
4. **Update code references** to documentation

### Phase 4: Add Navigation

Create README.md in each directory with:

- Purpose of the directory
- List of contents with descriptions
- Links to related sections
- Quick navigation to common tasks

### Phase 5: Cleanup

1. **Remove empty directories**
2. **Archive old structure**
3. **Update build scripts**
4. **Update CI/CD references**

## Benefits

### Improved Organization

- **Logical grouping** by purpose and audience
- **Clear hierarchy** from general to specific
- **Consistent depth** across topics
- **Related content together**

### Better Discovery

- **Predictable locations** for content
- **README navigation** in every directory
- **Purpose-driven structure**
- **Clear audience separation** (player vs developer)

### Easier Maintenance

- **Consistent naming** reduces confusion
- **Clear ownership** of sections
- **Simpler updates** with logical structure
- **Better version control** with smaller files

### Professional Appearance

- **Industry-standard structure**
- **Familiar to new developers**
- **Ready for documentation generators**
- **Scalable for growth**

## Implementation Checklist

### Pre-Migration

- [ ] Backup current documentation
- [ ] Create migration script
- [ ] Map all existing files
- [ ] Identify deprecated content

### Migration

- [ ] Create new directory structure
- [ ] Copy files to new locations
- [ ] Rename files per conventions
- [ ] Update internal links
- [ ] Create navigation READMEs

### Post-Migration

- [ ] Verify all links work
- [ ] Update build scripts
- [ ] Update code references
- [ ] Remove old structure
- [ ] Update contributing guide

### Validation

- [ ] All files accessible
- [ ] No broken links
- [ ] Search still works
- [ ] Build system updated
- [ ] Team informed of changes

## Rollback Plan

If migration causes issues:

1. Keep old `DOC/` structure for 30 days
2. Maintain redirect map
3. Provide migration guide
4. Support parallel structures temporarily

## Timeline

- **Day 1**: Create structure, migrate core docs
- **Day 2**: Migrate remaining docs, update links
- **Day 3**: Testing and validation
- **Day 4**: Team review and feedback
- **Day 5**: Final cleanup and archive

## Success Criteria

1. **All documentation accessible** in new structure
2. **No broken links** internally or from code
3. **Improved navigation** verified by team
4. **Consistent naming** throughout
5. **Positive feedback** from developers

## Notes

### Considerations

- Some tools expect `docs/` (lowercase)
- GitHub Pages prefers `docs/` directory
- Many doc generators assume this structure
- IDEs have better support for standard layouts

### Future Enhancements

- Add search functionality
- Generate site from markdown
- Add versioned documentation
- Implement doc linting
- Add automated link checking

---

*Created: 2025-09-14*
*Status: Proposed*
*Author: Documentation Team*
