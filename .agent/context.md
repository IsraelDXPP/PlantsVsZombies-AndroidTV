```markdown
# Agent Context: SeedChooserScreen.cpp Changes

## Overview
These changes update the `SeedChooserScreen` class in the Plants vs. Zombies (PvZ) UI to better support **Extra Seeds Mode** (`gVSSetupAddonWidget->mExtraSeedsMode`) in **Zombie Chooser** (`mIsZombieChooser == true`). Previously, Zombie Chooser was limited to 3 rows without full adaptation for keyboard navigation or positional consistency with Plant Chooser layouts.

Key improvements:
- **Dynamic layout positioning** (`GetSeedPositionInChooser`): Adjusts X/Y offsets for Zombie Chooser in Extra Seeds Mode to match extended grid (5 rows) spacing, aligning with `Has7Rows()` logic.
- **Keyboard navigation fixes** (`GetNextSeedInDir`): 
  - Handles edge cases (e.g., bottom-right boundaries) only in non-Extra Mode.
  - Computes max rows dynamically per direction to support 5 rows in Extra Mode.
- Removes redundant code paths, improving consistency between Zombie and Plant choosers.

These ensure **keyboard selection** ("拓展僵尸选卡适配键盘选取") works seamlessly in Versus (VS) mode with addons.

## Affected Functions

### `GetSeedPositionInChooser(int theIndex, int &x, int &y)`
**Purpose**: Computes pixel position (x, y) for a seed packet at grid index `theIndex`.

#### Before
- **X-position**:
  - Base: `x = 53 * aCol + 22`.
  - Override: If Zombie Chooser, `aRow == 3`, and **not** Extra Seeds Mode → `x = 53 * aCol + 48`.
- **Y-position**:
  - Base: `Has7Rows()` ? `70 * aRow + 123` : `73 * aRow + 128`.
  - Override: Always `70 * aRow + 123` for Zombie Chooser (ignores `Has7Rows()`).

#### After
- Caches `bool isExtraSeedsMode = gVSSetupAddonWidget && gVSSetupAddonWidget->mExtraSeedsMode`.
- **X-position**:
  ```cpp
  if (mIsZombieChooser && aRow == 3 && !isExtraSeedsMode) {
      x = 53 * aCol + 48;
  } else {
      x = 53 * aCol + 22;
  }
  ```
- **Y-position**:
  ```cpp
  if (Has7Rows() || (mIsZombieChooser && isExtraSeedsMode)) {
      y = 70 * aRow + 123;  // Tighter spacing for extended grids
  } else {
      y = 73 * aRow + 128;
  }
  ```
- **Impact**: Zombie Chooser in Extra Seeds Mode now uses 70px row spacing (like 7-row Plant grids), enabling 5 rows without clipping/misalignment. Row 3 X-offset preserved only for legacy 3-row mode.

#### Grid Assumptions
| Mode                  | Columns (`NumColumns()`) | Rows (computed) | X Base | Y Base (per row) |
|-----------------------|---------------------------|-----------------|--------|------------------|
| Plant (standard)      | ?                         | 4               | +22    | 73px + 128       |
| Plant (7 rows)        | ?                         | 5               | +22    | 70px + 123       |
| **Zombie (non-Extra)**| ?                         | **3**           | +48 (row 3) | 70px + 123   |
| **Zombie (Extra)**    | ?                         | **5**           | +22    | **70px + 123**   |

### `GetNextSeedInDir(int theNumSeed, SeedDir theMoveDirection)`
**Purpose**: Returns next seed index during keyboard navigation (up/down/left/right).

#### Before
- Special edges **only** if Zombie Chooser: Block `SEED_DIR_DOWN` from 14 or `SEED_DIR_RIGHT` from 18 (3-row grid bounds).
- Fixed `aNumRow` computed upfront:
  ```cpp
  mIsZombieChooser ? (ExtraSeedsMode ? 5 : 3) : (!mApp->IsVSMode() && Has7Rows() ? 5 : 4)
  ```
- `DOWN`: `if (aRow < aNumRow) ++aRow;`.

#### After
- Caches `isExtraSeedsMode` (reuse).
- Special edges **only** if `mIsZombieChooser && !isExtraSeedsMode` (skipped in 5-row Extra Mode).
- **Dynamic max rows per direction**:
  ```cpp
  case SeedDir::SEED_DIR_DOWN: {
      int aMaxRow = mIsZombieChooser ? (isExtraSeedsMode ? 5 : 3)
                                     : (Has7Rows() ? 5 : 4);
      if (aRow < aMaxRow) ++aRow;
  } break;
  ```
- Other directions (UP/LEFT/RIGHT) unchanged (column-based).

#### Impact
- Prevents invalid navigation in 3-row Zombie mode (e.g., no wrap from seed 14 down).
- Enables full 5-row traversal in Extra Mode without out-of-bounds.
- Imitater (`SEED_IMITATER`) handling unchanged (grid offset).

## Dependencies
- `gVSSetupAddonWidget`: Global VS addon widget (null-checked).
- `NumColumns()`: Unchanged, returns column count (likely 6-7 based on indices like 14/18).
- `Has7Rows()`: Plant-specific flag for extended packets.
- `SeedDir`: Enum `{SEED_DIR_UP, SEED_DIR_DOWN, SEED_DIR_LEFT, SEED_DIR_RIGHT}`.

## Testing Recommendations
- **Zombie Chooser, non-Extra**: Verify 3-row layout, positions (row 3 x+48), edges block (14↓, 18→).
- **Zombie Chooser, Extra**: 5 rows, standard x+22, y=70px spacing, full keyboard traversal.
- **Plant Chooser**: Unchanged behavior.
- Keyboard: Cycle all directions without skips/crashes.
- Visual: No overlaps/shifts in seed packet rendering.

## Related Files
- `VSSetupAddonWidget` (defines `mExtraSeedsMode`).
- UI rendering calls to `GetSeedPositionInChooser`.
- Input handling invoking `GetNextSeedInDir` (e.g., `MouseUp` nearby, unchanged).
```

## Migration Notes
No breaking changes; backward-compatible with prior modes. Refactor improves readability by centralizing `isExtraSeedsMode` and removing overrides.