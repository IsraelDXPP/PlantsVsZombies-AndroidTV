```markdown
# Agent Context: Recent Code Changes

This file provides context on recent modifications to the PvZ codebase, focusing on changes in `SeedChooserScreen.cpp`. These updates enhance support for **extra seeds mode** (`gVSSetupAddonWidget->mExtraSeedsMode`) in zombie choosers, particularly for positioning (`GetSeedPositionInChooser`) and keyboard navigation (`GetNextSeedInDir`). This adapts the zombie seed chooser to handle expanded layouts (e.g., 5 rows instead of 3) while preserving legacy behavior.

## Affected Functions

### `GetSeedPositionInChooser(int theIndex, int &x, int &y)`
**Purpose**: Computes pixel position (x, y) for a seed packet at index `theIndex` in the chooser grid.

**Key Changes**:
- **X-position logic**:
  - Default: `x = 53 * aCol + 22`.
  - Override: If `mIsZombieChooser && aRow == 3 && !isExtraSeedsMode`, use `x = 53 * aCol + 48` (offsets row 3 for legacy 3-row zombie layout).
- **Y-position logic**:
  - Compact spacing (`y = 70 * aRow + 123`): If `Has7Rows() || (mIsZombieChooser && isExtraSeedsMode)`.
  - Standard spacing (`y = 73 * aRow + 128`): Otherwise.
- **New Helper**: `bool isExtraSeedsMode = gVSSetupAddonWidget && gVSSetupAddonWidget->mExtraSeedsMode;`.
- **Impact**: Ensures consistent positioning in expanded zombie choosers (treats as 5-row layout like plants). Removes redundant zombie-specific y-override.

**Grid Assumptions**:
| Mode                  | Columns | Rows (Default/Extra) | Notes |
|-----------------------|---------|----------------------|-------|
| Plant Chooser        | Dynamic | 4 or 5 (Has7Rows)   | VS mode affects rows. |
| Zombie (No Extra)    | Dynamic | 3                    | Row 3 x-offset adjusted. |
| Zombie (Extra)       | Dynamic | 5                    | Matches plant compact y-spacing. |

### `GetNextSeedInDir(int theNumSeed, SeedDir theMoveDirection)`
**Purpose**: Computes the next seed index for keyboard navigation (up/down/left/right).

**Key Changes**:
- **Special Edge Cases** (bottom-right corners):
  - Applied **only** if `mIsZombieChooser && !isExtraSeedsMode`.
  - Handles `theNumSeed == 14` (down) or `18` (right): Stays on current seed (no wrap/movement).
- **Row Calculation** (`aRow`, `aCol`): Unchanged, handles Imitater specially.
- **DOWN Direction**:
  - Dynamic `aMaxRow`:
    ```
    int aMaxRow = mIsZombieChooser ? (isExtraSeedsMode ? 5 : 3)
                                   : (Has7Rows() ? 5 : 4);
    ```
  - Increment `aRow` only if `aRow < aMaxRow`.
- **New Helper**: Same `isExtraSeedsMode` as above.
- **Impact**: Enables keyboard selection in expanded zombie choosers (5 rows). Legacy 3-row zombies retain edge clamping. Other directions (UP/LEFT/RIGHT) unchanged.

**Navigation Bounds**:
| Mode                  | Max Rows | Edge Handling |
|-----------------------|----------|---------------|
| Plant Chooser        | 4 or 5  | Standard bounds. |
| Zombie (No Extra)    | 3       | Clamps 14↓/18→. |
| Zombie (Extra)       | 5       | Full grid nav. |

## Behavioral Summary
- **Extra Seeds Mode (`mExtraSeedsMode = true`)**: Zombie chooser aligns with plant chooser layout (compact y-spacing, 5 rows, full nav).
- **Legacy Compatibility**: Without extra mode, zombie chooser keeps 3-row quirks (x-offset row 3, edge clamping).
- **Dependencies**: Relies on `gVSSetupAddonWidget`, `NumColumns()`, `Has7Rows()`, VS mode checks.
- **No Impact**: Mouse handling (`MouseUp`), column count.

## Rationale (Inferred from Code/Comments)
- Chinese comment: "拓展僵尸选卡适配键盘选取" → "Expand zombie seed chooser for keyboard selection compatibility."
- Unifies layout/nav across modes, fixes positioning for expanded zombie packets.

**Testing Recommendations**:
- Toggle `mExtraSeedsMode` in VS zombie setup.
- Verify positions/nav with keyboard in 3-row vs. 5-row zombie choosers.
- Check Imitater edge cases, Has7Rows plants.

Last Updated: Based on commit `43e53ad` (from `6b62417`).
```

## Future Changes
Consult this context before modifying `SeedChooserScreen`. Update this file for new diffs.
```