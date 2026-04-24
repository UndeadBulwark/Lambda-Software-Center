# SESSION.md — Lambda Software Center

Paste the contents of this file at the start of every new AI session.
Update it at the end of every session before closing.

---

## Project

Lambda Software Center — Qt6/QML native package manager for Arch Linux.
Manages pacman (via libalpm), AUR (RPC v5 + git + makepkg), and Flatpak (via libflatpak).

**Read before doing anything:**
- `AGENTS.md` — code structure, naming conventions, hard rules
- `DECISIONS.md` — architectural decisions already made, do not relitigate
- `ui-spec.md` — full UI design tokens, component catalog, layout rules

---

## Current Version Target

`v0.3.1` — Search fixes and UI polish ✅ **Complete**

**Also completed:** v0.3.0 Package Detail View, v0.2.0 Application Shell, system theme detection + dark/light palette support, multi-source search accumulation fix + model unit tests, search relevance sorting

---

## Session Closeout — Fri Apr 24 (v0.3.1 tag)

### v0.3.1 — Bugfix Release

Tagged and released on GitHub.

**Fixes included**:
1. **Multi-source search accumulation** — `searchModel` now accumulates results from pacman + AUR + Flatpak via `appendPackages()` instead of being disconnected entirely.
2. **Search relevance sorting** — exact match (1000 pts) → prefix (900) → name contains (800) → description (100). Applied in all three backends before emission.
3. **PackageCard badge layout** — "Installed" pill no longer overflows card bounds; source + installed badges sit side-by-side in a `Row`.

**Test suite**: 47 tests across 4 targets (17 pacman + 10 aur + 12 flatpak + 8 models).

**Pushed to GitHub**: tag `v0.3.1`, release at `https://github.com/UndeadBulwark/Lambda-Software-Center/releases/tag/v0.3.1`

---

## Session Closeout — Fri Apr 24 (continued)

### Search Result Relevance Sorting

**Root cause**: `alpm_db_search()` and AUR RPC return results in undefined/database order. Searching "firefox" could show `firefox-i18n-something` before `firefox` because the description happened to match and it was registered in a later sync DB.

**Fix applied**:
1. **`src/backend/PackageSearchUtils.h`**: New header-only utility with inline functions:
   - `searchRelevanceScore(const Package&, const QString&)`: 1000 exact match, 900 prefix, 800 name-contains, 100 description-only, 0 otherwise. Case-insensitive.
   - `sortPackagesBySearchRelevance(QList<Package>&, const QString&)`: `std::sort` with score descending, alphabetical `localeAwareCompare` tie-break.
2. **`src/backend/pacman/AlpmWrapper.cpp`**: After collecting all DB results, calls `sortPackagesBySearchRelevance(results, query)` before return.
3. **`src/backend/aur/AurClient.cpp`**: After parsing JSON array in `onReplyFinished()` and `onMockSearch()`, calls `sortPackagesBySearchRelevance(results, m_pendingQuery)` before emit.
4. **`src/backend/flatpak/FlatpakBackend.cpp`**: After mock result construction, sorts before emit.
5. **`tests/test_pacman.cpp`**: Added 2 tests:
   - `test_search_sorts_exact_match_first` — verifies `linux` appears before `linux-firmware`
   - `test_search_sorts_prefix_before_contains` — verifies exact < prefix < contains ordering

**All 4 test suites pass** (17 + 10 + 12 + 8 = 47 test cases total).

### Multi-Source Search Fix + Model Tests

**Root cause**: `searchModel` in `main.cpp` was declared but **never connected** to any backend's `searchResultsReady`. Backends emitted results, signal went nowhere, grid stayed empty. `installedModel` and `updatesModel` were correctly wired via `setPackages`, but `searchModel` had no connections at all.

**Fix applied**:
1. **`src/models/PackageListModel.h/.cpp`**: Added `appendPackages(const QList<Package>&)` method. Uses `beginInsertRows`/`endInsertRows` for correct incremental model notification. Empty guard prevents invalid row range signals.
2. **`src/main.cpp`**: Wired `pacmanBackend`, `aurBackend`, `flatpakBackend` `searchResultsReady` → `searchModel::appendPackages`. Kept `installedListReady`/`updatesReady` → `setPackages` for replace semantics.
3. **`tests/models/test_packagelistmodel.cpp`**: New test suite with 6 test cases, 8 assertions total:
   - `testAppendPackagesAccumulates` — row count, data at new index
   - `testAppendPackagesEmptyIsNoOp` — `rowsInserted` signal spy count == 0
   - `testAppendMultipleAtOnce` — single signal emission for batch insert
   - `testSetPackagesReplaces` — `setPackages` still works (model reset)
   - `testClearEmptiesModel` — `clear()` resets to zero rows
   - `testPackagesReturnsInternalList` — accessor returns correct copy
4. **`tests/CMakeLists.txt`**: Added `test_models` target via `add_backend_test(test_models models/test_packagelistmodel.cpp)`.

**All 4 test suites pass** (15 + 10 + 12 + 6 = 43 test cases total).

**Builds clean**: `lambda-software-center` executable links without undefined references. Offscreen launch produces zero QML warnings.

### v0.3.0 Previously Completed

- **Package Detail View (DetailPage.qml)**:
  - `StackView.push` from any `PackageCard` delegate, passing `model` reference as `packageData` property
  - Header row: 52×52 source-colored icon with initial, package name (`20px` bold), version, source badge
  - Ghost Install/Remove buttons (display-only, styled with `Theme.borderSecondary` border)
  - Description section: `longDescription` falls back to `description`
  - Metadata row: installed size (human-formatted bytes → KB/MB/GB), download size, dependency count
  - Dependencies list rendered as comma-separated text
  - Flatpak rating row: visible only when `source === 2 && rating > 0`, displays score out of 5
  - Back button at top: `← Back` text with `MouseArea` calling `StackView.view.pop()`
  - `Flickable` wrapper for scrollable content
  - All styling exclusively from `Theme.qml` — no hardcoded colors or sizes

- **PackageCard.qml**: Added `onClicked` handler in existing `MouseArea`:
  - `StackView.view.push("qrc:/LambdaSoftwareCenter/qml/pages/DetailPage.qml", { packageData: model })`
  - `model` reference passed directly, all delegate roles accessible via `packageData.name`, etc.

- **Scope exclusions correctly applied**:
  - AUR: no PKGBUILD viewer, no vote count, no popularity score (roadmap v0.5.0+)
  - AppStream: no integration (roadmap v0.6.0)
  - Screenshots: not implemented (roadmap v0.6.0)
  - Install/Remove: ghost buttons only, no transaction wiring (gated to v0.4.0)
  - No `featured.json` or `FeaturedPage` changes

- **All 3 tests still pass** (test_pacman 14/14, test_aur 8/8, test_flatpak 12/12).
- **Builds clean** — no QML errors on launch.
- **Pushed to GitHub `main`** as `3051e25`.

### v0.2.0 Previously Completed

- Full QML Application Shell with sidebar, topbar, search, source tabs, status bar
- BrowsePage with 3-column card grid, 250ms debounced search
- System theme detection via `QStyleHints`, live `Binding` to `Theme.isDark`
- Dark/light conditional palette in Theme.qml
- QML warnings diagnostic hook in main.cpp

### v0.2.0 Completed

- **Implemented full QML Application Shell** matching the UI spec exactly:
  - `main.qml`: Root `ApplicationWindow` with sidebar + main area layout. `StackView` page routing wired to sidebar nav items (`browse`, `featured`, `recent`, `installed`, `updates`).
  - `Sidebar.qml`: Fixed 200px width, logo, three `NavGroup`s (Discover, Library, Sources), `NavItem`s with source dots, active border accent (2px right), hover states.
  - `Topbar.qml`: Search bar + source tabs row with `Layout` positioning.
  - `SearchBar.qml`: 250ms debounced `TextField` with Canvas-drawn search icon. Emits `onSearchTextChanged` after debounce.
  - `SourceTabs.qml`: `All` / `Pacman` / `AUR` / `Flatpak` filter tabs with toggle styling (accent bg/border when active).
  - `StatusBar.qml`: Three status items with colored dots + right-aligned status text.
  - `PackageCard.qml`: Full data-bound card with `AppIcon` (initials, color-coded by source), `PackageName`, `Version`, `Description`, `BadgePill` source badge, and `Installed` state pill. Hover border transition.
  - `BadgePill.qml`: Four variants (`pacman`, `aur`, `flatpak`, `installed`) with correct colors.
  - `InstallButton.qml`: Ghost/primary/installed states (currently display-only).
- **Implemented `BrowsePage.qml`**: 3-column `GridView` bound to `searchModel`, empty state message, scroll indicator. Triggers backend search when `searchQuery` changes.
- **Wired 250ms debounced search**: `SearchBar` → `Topbar` → `main.qml` property → `BrowsePage` → `pacmanBackend.search()` / `aurBackend.search()` / `flatpakBackend.search()` respecting `sourceFilter`.
- **Added `PackageListModel::clear()`** Q_INVOKABLE method so BrowsePage clears previous results before new search.
- **Stubbed remaining pages** (`FeaturedPage`, `RecentPage`, `InstalledPage`, `UpdatesPage`, `DetailPage`) with placeholder text.
- **All 3 tests still pass** (test_pacman 14/14, test_aur 8/8, test_flatpak 12/12).
- **Builds clean**: Production target compiles with QML single compilation, no hardcoded colors anywhere.
- **Pushed to GitHub `main`**.

### QOL Fixes (post v0.2.0)

1. **Tab highlight bug fix**: Moved `activeTab` to `Topbar` as single source of truth. `SourceTabs` is now pure — delegates derive highlight from `activeIndex` comparison, emit `indexClicked(index)` on click. No tab holds independent state.
2. **Removed "Browse packages" heading**: Redundant heading below toolbar removed from `BrowsePage.qml`. Sidebar already indicates active section.
3. **Larger sidebar labels**: `NavGroup` labels increased from 10px to 11px, added `font.letterSpacing: 1.2`, added `topPadding: 8` for visual separation between groups.

### Theme Detection + Dark/Light Mode (QOL addition)

- **`src/main.cpp`**: Added `QStyleHints` include, `systemDarkMode` detection via `QGuiApplication::styleHints()->colorScheme()`, `colorSchemeChanged` signal connected to update context property live.
- **`qml/Theme.qml`**: Added `isDark` property (defaults to light), all neutral tokens become conditional ternary expressions (bgPrimary, bgSecondary, textPrimary, textSecondary, textTertiary, borderSecondary, borderTertiary). Brand/accent colors stay identical in both modes; surface backgrounds darken in dark mode.
- **`qml/main.qml`**: `Binding` element connects `Theme.isDark` to `systemDarkMode` context property, so the app responds to live system theme changes without restart.
- **Pushed:** `c32c1f5` on `main`.

- **Package Detail View (DetailPage.qml)**:
  - `StackView.push` from any `PackageCard` delegate, passing `model` reference as `packageData` property
  - Header row: 52×52 source-colored icon with initial, package name (`20px` bold), version, source badge
  - Ghost Install/Remove buttons (display-only, styled with `Theme.borderSecondary` border)
  - Description section: `longDescription` falls back to `description`
  - Metadata row: installed size (human-formatted bytes → KB/MB/GB), download size, dependency count
  - Dependencies list rendered as comma-separated text
  - Flatpak rating row: visible only when `source === 2 && rating > 0`, displays score out of 5
  - Back button at top: `← Back` text with `MouseArea` calling `StackView.view.pop()`
  - `Flickable` wrapper for scrollable content
  - All styling exclusively from `Theme.qml` — no hardcoded colors or sizes

- **PackageCard.qml**: Added `onClicked` handler in existing `MouseArea`:
  - `StackView.view.push("qrc:/LambdaSoftwareCenter/qml/pages/DetailPage.qml", { packageData: model })`
  - `model` reference passed directly, all delegate roles accessible via `packageData.name`, etc.

- **Scope exclusions correctly applied**:
  - AUR: no PKGBUILD viewer, no vote count, no popularity score (roadmap v0.5.0+)
  - AppStream: no integration (roadmap v0.6.0)
  - Screenshots: not implemented (roadmap v0.6.0)
  - Install/Remove: ghost buttons only, no transaction wiring (gated to v0.4.0)
  - No `featured.json` or `FeaturedPage` changes

- **All 3 tests still pass** (test_pacman 14/14, test_aur 8/8, test_flatpak 12/12).
- **Builds clean** — no QML errors on launch.
- **Pushed to GitHub `main`** as `3051e25`.

### v0.2.0 Previously Completed

- Full QML Application Shell with sidebar, topbar, search, source tabs, status bar
- BrowsePage with 3-column card grid, 250ms debounced search
- System theme detection via `QStyleHints`, live `Binding` to `Theme.isDark`
- Dark/light conditional palette in Theme.qml
- QML warnings diagnostic hook in main.cpp

### v0.2.0 Completed

- **Implemented full QML Application Shell** matching the UI spec exactly:
  - `main.qml`: Root `ApplicationWindow` with sidebar + main area layout. `StackView` page routing wired to sidebar nav items (`browse`, `featured`, `recent`, `installed`, `updates`).
  - `Sidebar.qml`: Fixed 200px width, logo, three `NavGroup`s (Discover, Library, Sources), `NavItem`s with source dots, active border accent (2px right), hover states.
  - `Topbar.qml`: Search bar + source tabs row with `Layout` positioning.
  - `SearchBar.qml`: 250ms debounced `TextField` with Canvas-drawn search icon. Emits `onSearchTextChanged` after debounce.
  - `SourceTabs.qml`: `All` / `Pacman` / `AUR` / `Flatpak` filter tabs with toggle styling (accent bg/border when active).
  - `StatusBar.qml`: Three status items with colored dots + right-aligned status text.
  - `PackageCard.qml`: Full data-bound card with `AppIcon` (initials, color-coded by source), `PackageName`, `Version`, `Description`, `BadgePill` source badge, and `Installed` state pill. Hover border transition.
  - `BadgePill.qml`: Four variants (`pacman`, `aur`, `flatpak`, `installed`) with correct colors.
  - `InstallButton.qml`: Ghost/primary/installed states (currently display-only).
- **Implemented `BrowsePage.qml`**: 3-column `GridView` bound to `searchModel`, empty state message, scroll indicator. Triggers backend search when `searchQuery` changes.
- **Wired 250ms debounced search**: `SearchBar` → `Topbar` → `main.qml` property → `BrowsePage` → `pacmanBackend.search()` / `aurBackend.search()` / `flatpakBackend.search()` respecting `sourceFilter`.
- **Added `PackageListModel::clear()`** Q_INVOKABLE method so BrowsePage clears previous results before new search.
- **Stubbed remaining pages** (`FeaturedPage`, `RecentPage`, `InstalledPage`, `UpdatesPage`, `DetailPage`) with placeholder text.
- **All 3 tests still pass** (test_pacman 14/14, test_aur 8/8, test_flatpak 12/12).
- **Builds clean**: Production target compiles with QML single compilation, no hardcoded colors anywhere.
- **Pushed to GitHub `main`**.

### QOL Fixes (post v0.2.0)

1. **Tab highlight bug fix**: Moved `activeTab` to `Topbar` as single source of truth. `SourceTabs` is now pure — delegates derive highlight from `activeIndex` comparison, emit `indexClicked(index)` on click. No tab holds independent state.
2. **Removed "Browse packages" heading**: Redundant heading below toolbar removed from `BrowsePage.qml`. Sidebar already indicates active section.
3. **Larger sidebar labels**: `NavGroup` labels increased from 10px to 11px, added `font.letterSpacing: 1.2`, added `topPadding: 8` for visual separation between groups.

### Theme Detection + Dark/Light Mode (QOL addition)

- **`src/main.cpp`**: Added `QStyleHints` include, `systemDarkMode` detection via `QGuiApplication::styleHints()->colorScheme()`, `colorSchemeChanged` signal connected to update context property live.
- **`qml/Theme.qml`**: Added `isDark` property (defaults to light), all neutral tokens become conditional ternary expressions (bgPrimary, bgSecondary, textPrimary, textSecondary, textTertiary, borderSecondary, borderTertiary). Brand/accent colors stay identical in both modes; surface backgrounds darken in dark mode.
- **`qml/main.qml`**: `Binding` element connects `Theme.isDark` to `systemDarkMode` context property, so the app responds to live system theme changes without restart.
- **Pushed:** `c32c1f5` on `main`.

---

## Previous Session History

### v0.1.0 Completed

- Created GPL-3.0 LICENSE
- Rewrote README.md: badges, condensed roadmap, build instructions, CLI examples, AI assistance section
- Added mockup screenshot (`docs/screenshots/mockup.png`) and wired it into README
- Replaced existing test suite with 3 root-level tests enforcing stricter API contract
- Refactored `Package` struct: nested enums, `newVersion`, `flatpakRef`
- Refactored `AlpmWrapper`: sandboxed init with test DB copying
- Refactored `PacmanBackend`: async signal emission via `QTimer::singleShot(0)`
- Refactored `AurClient`: 250ms debounce, test seams under `#ifdef QT_TESTLIB_LIB`
- Refactored `AurBackend` and `FlatpakBackend` to match new interface
- CLI works headlessly: `--search`, `--list-installed`, `--check-updates`

---

## Current State

- Full QML Application Shell functional. Running `./lambda-software-center` shows:
  - Sidebar with navigation, active state, hover
  - Topbar with debounced search and source tabs
  - Browse page with 3-column card grid
  - Status bar with source indicators
  - **Theme auto-detects light/dark mode from system palette**
  - **Live theme switching works without restart**
- Search flow: type in search bar → 250ms debounce → `pacmanBackend.search()` + `aurBackend.search()` → results populate `searchModel` → GridView re-renders with `PackageCard` delegates.
- All data flows through `PackageListModel` (backend signal → model reset → QML delegate).
- No UI logic in C++; no business logic in QML.
- `libflatpak` remains stubbed (not installed).

---

## Blockers / Open Questions

- `checkUpdates()` slow (~20s on 1300+ packages). May need threading or caching later.
- Flatpak backend needs `libflatpak` installed to implement properly.
- App icons currently show source-colored circle with first initial (no AppStream integration yet).

---

## Next Task

`v0.4.0` — Install and Remove:
- Confirmation dialog with dependencies, size delta, source
- AUR install flow: mandatory PKGBUILD review step
- Progress drawer at window bottom with per-step status
- Remove flow with orphan detection
- polkit elevation for pacman transactions
- Transaction error handling with readable messages
- Install and Remove buttons wired to real transactions

---

## Files Touched Last Session

### Modified:
- `src/main.cpp` — added `qmlRegisterSingletonType` for Theme singleton, `QML warnings` diagnostic hook, fixed QRC load path, added `QStyleHints` include, `systemDarkMode` detection + `colorSchemeChanged` signal
- `qml/Theme.qml` — added `isDark` property, all neutral color tokens are conditional dark/light expressions
- `qml/main.qml` — full shell with StackView routing, `LambdaSoftwareCenter` import, live `Binding` for `Theme.isDark`
- `qml/components/Sidebar.qml` — logo, nav groups, sources, active/hover states
- `qml/components/NavGroup.qml` — labeled group container, no circular children binding
- `qml/components/NavItem.qml` — nav item with source dot, count pill, fixed `property var sourceDot: null`
- `qml/components/Topbar.qml` — fixed RowLayout without invalid Row padding properties
- `qml/components/SearchBar.qml` — debounced TextField, Canvas search icon
- `qml/components/SourceTabs.qml` — All/Pacman/AUR/Flatpak toggle tabs
- `qml/components/StatusBar.qml` — removed invalid `leftPadding`/`rightPadding`, used explicit `x`/`width`
- `qml/components/PackageCard.qml` — full data-bound package card
- `qml/components/BadgePill.qml` — four variant source/state pills
- `qml/components/InstallButton.qml` — ghost/primary/installed button states
- `qml/pages/BrowsePage.qml` — 3-column GridView bound to searchModel, empty state, search trigger
- `qml/pages/FeaturedPage.qml` — stub
- `qml/pages/RecentPage.qml` — stub
- `qml/pages/InstalledPage.qml` — stub
- `qml/pages/UpdatesPage.qml` — stub
- `qml/pages/DetailPage.qml` — stub
- `src/models/PackageListModel.h/.cpp` — added `clear()` Q_INVOKABLE method

---

## Versions

| Dependency | Version |
|---|---|
| Qt | 6.11.0 |
| libalpm | 16.0.1 |
| libflatpak | not installed |
| CMake | 4.3.2 |
| Compiler | GCC 15.2.1 |

---

## Notes

- AUR RPC base URL: `https://aur.archlinux.org/rpc/v5`
- polkit policy file lives at `data/com.lambdasc.policy`
- All UI tokens are in `qml/Theme.qml` — never hardcode colors or sizes
- Target name in CMake is `lsc_app`; output executable is `lambda-software-center`.
- Test seams are guarded by `#ifdef QT_TESTLIB_LIB` — never present in production builds.
- `letterSpacing` is not supported in `Text` in Qt Quick 1.0; workaround used in `NavGroup`
- **Singleton pattern**: Theme registered via `qmlRegisterSingletonType(QUrl("qrc:/LambdaSoftwareCenter/qml/Theme.qml"), "LambdaSoftwareCenter", 1, 0, "Theme")` in C++ root context. All QML files import `LambdaSoftwareCenter`.
- **Theme detection**: `QStyleHints::colorScheme()` → `systemDarkMode` context property → `Theme.isDark` → conditional palette swaps live across all components.
