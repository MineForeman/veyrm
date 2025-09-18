# Documentation Refactoring Plan

## Executive Summary

This document outlines needed improvements to the Veyrm documentation following the MVP milestone completion.

## Critical Issues to Address

### 1. Outdated Information

- **README.md**: Shows v0.10.1, should be v1.0.0-MVP
- **README.md**: Lists "Inventory System" as planned (it's complete)
- **CURRENT_STATE.md**: Last updated 2025-01-14, missing Phase 12
- **Version mismatches** across multiple files

### 2. Duplicate Files

- `CHANGELOG.md` exists in both root and DOC/ (identical)
- Multiple "changes summary" files for same phases
- Overlapping content between SPEC.md, README.md, and MVP/00_README.md

### 3. Inconsistent Formatting

- Mixed checkbox styles (✅, ✓, [x], - [x])
- Different heading levels for similar content
- Mixed date formats (YYYY-MM-DD vs other formats)
- Inconsistent bullet styles (-, *, •)
- Mixed file naming (underscores vs hyphens)

### 4. Missing Documentation

- No consolidated architecture diagram
- Missing API documentation
- No developer onboarding guide
- Incomplete troubleshooting section
- No code style guide
- Save/Load system user documentation
- Monster AI behavior details

### 5. Poor Organization

- Controls documentation scattered across 4+ files
- Build instructions repeated in multiple places
- Architecture descriptions duplicated
- No clear "start here" flow
- Missing cross-references between documents

## Recommended Actions

### Immediate Priority ✅ COMPLETED

1. **Update README.md** ✅
   - ✅ Changed version to v1.0.0-MVP
   - ✅ Marked Inventory System as complete
   - ✅ Updated feature list to reflect MVP status
   - ✅ Added missing keybindings (g, u, D, E, S, L, o)

2. **Consolidate Changelogs** ✅
   - ✅ Deleted DOC/CHANGELOG.md
   - ✅ Kept only root CHANGELOG.md
   - ✅ Added MVP milestone entry

3. **Update CURRENT_STATE.md** ✅
   - ✅ Added Phase 11-12 completion
   - ✅ Updated statistics (135 tests)
   - ✅ Reflected MVP status

### High Priority ✅ COMPLETED

1. **Create Architecture Document** ✅
   - ✅ Created `DOC/ARCHITECTURE.md`
   - ✅ Included system diagrams (ASCII art)
   - ✅ Showed component relationships
   - ✅ Documented data flow

2. **Consolidate Controls** ✅
   - ✅ Created single `DOC/CONTROLS.md`
   - ✅ Consolidated all keybindings
   - ✅ Resolved vi-keys inconsistency
   - ✅ Added testing key codes

3. **Standardize Formatting** ⏳ IN PROGRESS
   - Need to update all docs for consistency
   - Use ✅ for all checkmarks
   - ISO date format (YYYY-MM-DD)
   - Standardize bullet points to `-`

### Medium Priority ✅ COMPLETED

1. **Archive Completed Phases** ✅
   - ✅ Moved all phase files to `DOC/PHASES/ARCHIVE/`
   - ✅ Created README.md in PHASES directory
   - ✅ Preserved all historical documentation

2. **Create Developer Guide** ✅
   - ✅ Created `DOC/DEVELOPER_GUIDE.md`
   - ✅ Included setup instructions
   - ✅ Added architecture overview
   - ✅ Documented testing guidelines
   - ✅ Added contribution guidelines

3. **Merge Redundant Files** ✅
   - ✅ Moved phase summaries to DOC/ARCHIVE
   - ✅ Cleaned up redundant documentation
   - ✅ Organized file structure

### Low Priority ✅ COMPLETED

1. **Add Missing Sections** ✅
   - ✅ API documentation generation guide created
   - ✅ Performance benchmarks documentation
   - ✅ Code style guide established
   - User manual (deferred - not needed for MVP)

2. **Improve Navigation** ✅
   - ✅ Created DOC/README.md as navigation hub
   - ✅ Added cross-references between documents
   - ✅ Organized documentation by purpose
   - ✅ Created clear documentation structure

## File-Specific Changes

### Files to Update

- `README.md` - Version, features, quick start
- `CURRENT_STATE.md` - Phase 12, MVP status
- `DOC/IMPLEMENTATION_PLAN.md` - Mark phases complete
- `DOC/SPEC.md` - Add implementation links

### Files to Delete

- `DOC/CHANGELOG.md` (duplicate)
- Old session summaries in PHASES/
- Redundant change summary files

### Files to Create

- `DOC/ARCHITECTURE.md`
- `DOC/CONTROLS.md` (consolidated)
- `DOC/DEVELOPER_GUIDE.md`
- `DOC/REFACTORING/` (for this plan)

## Success Metrics

- [ ] No version mismatches
- [ ] No duplicate content
- [ ] Consistent formatting throughout
- [ ] Clear documentation flow
- [ ] All systems documented
- [ ] Easy navigation between docs

## Timeline

1. **Immediate**: Version updates, changelog consolidation (1 hour)
2. **Today**: High priority items (2-3 hours)
3. **This Week**: Medium priority items
4. **Future**: Low priority enhancements

## Notes

- Preserve all historical information in archives
- Maintain backwards compatibility references
- Keep user-facing docs separate from developer docs
- Ensure all changes are tracked in git
