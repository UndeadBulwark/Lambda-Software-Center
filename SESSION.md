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

`v0.4.0` — Install and Remove (Phase 1 UI complete) ✅

**Also completed:** v0.3.1 Search fixes and UI polish, v0.3.0 Package Detail View, v0.2.0 Application Shell, system theme detection + dark/light palette support, multi-source search accumulation fix + model unit tests, search relevance sorting

---

## Session Closeout — Fri Apr 24

### v0.4.0 Phase 1 — Install Dialog + Progress Drawer UI

**Completed before user left for work:**

1. **`qml/components/ConfirmDialog.qml`** — New modal overlay:
   - Package name header, version row with source badge pill
   - Download + disk size metadata lines
   - Scrollable dependency list in a rounded container with `Theme.bgSecondary`
   - Primary "Install" button (accent fill, white text) + ghost "Cancel" button
   - Dark scrim (`#AA000000`) with fade animation via `Behavior on opacity`
   - Clicking scrim or "Cancel" dismisses dialog; "Install" emits `installConfirmed()`

2. **`qml/components/ProgressDrawer.qml`** — Rewritten from empty stub:
   - Anchored to window bottom, slides up/down via `Behavior on height` with `NumberAnimation`
   - Package name, status text ("Downloading...", "Verifying...", "Installing...", "Complete"), percentage counter
   - Animated progress bar: `Rectangle` fill width bound to `percent / 100`, animated via `Behavior on width`
   - Error state support: red/orange bar and text when `isError` is true
   - `z: 50` so it sits above page content but below modal overlays

3. **`qml/pages/DetailPage.qml`** — Install button now functional:
   - Added `MouseArea` to ghost Install button
   - On click: populates `Window.window.confirmDialog` with `packageData.name`, `version`, `source` (via `badgeForSource()`), `dependencies`, `downloadSize`
   - Sets `confirmDialog.visible = true`

4. **`qml/main.qml`** — Global overlays + mock transaction flow:
   - `ConfirmDialog` and `ProgressDrawer` declared as global child items of `ApplicationWindow` (outside StackView)
   - `Timer { id: mockTransaction }` simulates realistic install flow: 500ms ticks, +20% per tick, status text transitions at 40% (Verify) and 60% (Install)
   - `Timer { id: hideTimer }` auto-hides drawer 2 seconds after "Complete"
   - `onInstallConfirmed` handler wires dialog → drawer → mock transaction → auto-hide

5. **`CMakeLists.txt`** — Added `qml/components/ConfirmDialog.qml` to `QML_FILES`

**Build/Test Result**:
- `make -j$(nproc)`: clean link, zero errors
- `ctest`: 4/4 suites pass (47 tests: 17 pacman + 10 aur + 12 flatpak + 8 models)
- Offscreen launch (`QT_QPA_PLATFORM=offscreen`): zero QML warnings
- Pushed to GitHub `main` as `5ed894a`

### v0.3.1 — Bugfix Release

Tagged and released on GitHub.

**Fixes included**:
1. **Multi-source search accumulation** — `searchModel` now accumulates results from pacman + AUR + Flatpak via `appendPackages()` instead of being disconnected entirely.
2. **Search relevance sorting** — exact match (1000 pts) → prefix (900) → name contains (800) → description (100). Applied in all three backends before emission.
3. **PackageCard badge layout** — "Installed" pill no longer overflows card bounds; source + installed badges sit side-by-side in a `Row`.

**Test suite**: 47 tests across 4 targets (17 pacman + 10 aur + 12 flatpak + 8 models).

**Pushed to GitHub**: tag `v0.3.1`, release at `https://github.com/UndeadBulwark/Lambda-Software-Center/releases/tag/v0.3.1`

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

- Full QML Application Shell functional with global overlay architecture:
  - Sidebar with navigation, active state, hover
  - Topbar with debounced search and source tabs
  - Browse page with 3-column card grid
  - Status bar with source indicators
  - **Theme auto-detects light/dark mode from system palette**
  - **Live theme switching works without restart**
  - **ConfirmDialog opens from DetailPage Install button with real package data**
  - **ProgressDrawer animates through mock transaction (Download → Verify → Install → Complete)**
- Search flow: type in search bar → 250ms debounce → `pacmanBackend.search()` + `aurBackend.search()` → results populate `searchModel` with relevance sorting → GridView re-renders with `PackageCard` delegates.
- All data flows through `PackageListModel` (backend signal → model reset/append → QML delegate).
- No UI logic in C++; no business logic in QML.
- `libflatpak` remains stubbed (not installed).

---

## Blockers / Open Questions

- `checkUpdates()` slow (~20s on 1300+ packages). May need threading or caching later.
- Flatpak backend needs `libflatpak` installed to implement properly.
- AUR install (PKGBUILD review → git clone → makepkg) gated to end of v0.4.x.

---

## Next Task

`v0.4.0` — Phase 2: Real TransactionManager + libalpm backend
- Implement `TransactionManager::install(const QString& pkgId)` method (currently signals only, no functionality)
- Wire `PacmanBackend::install()` to actual libalpm transaction APIs
- Replace mock `Timer` in `main.qml` with real `transactionStarted` / `transactionProgress` / `transactionFinished` signal connections
- Connect `ConfirmDialog.installConfirmed` to `transactionManager.install()` instead of mock timer
- Add polkit elevation for privileged operations

---

## Files Touched Last Session

### Modified:
- `qml/main.qml` — added ConfirmDialog + ProgressDrawer global overlays, mock transaction Timer
- `qml/pages/DetailPage.qml` — Install button MouseArea opens ConfirmDialog with packageData
- `qml/components/ProgressDrawer.qml` — full implementation: animated slide-up, progress bar, status text, error state
- `qml/components/ConfirmDialog.qml` — new modal overlay: package header, metadata, dependency list, Install/Cancel buttons
- `CMakeLists.txt` — added ConfirmDialog.qml to QML_FILES

---

## Previous Sessions' Files

### v0.3.1/Previous Modified:
- `SESSION.md` — session closeout notes
- `DECISIONS.md` — architectural decisions log
- `README.md` — project overview, badges, build instructions
- `src/backend/PackageSearchUtils.h` — new header-only search relevance utility
- `src/backend/pacman/AlpmWrapper.cpp` — relevance sort after DB search
- `src/backend/aur/AurClient.cpp` — relevance sort after JSON parse (both real + mock paths)
- `src/backend/flatpak/FlatpakBackend.cpp` — relevance sort after mock result construction
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
- **Overlay architecture**: ConfirmDialog and ProgressDrawer are global ApplicationWindow children, not StackView items. Child pages reference them via `Window.window.confirmDialog`. See DECISIONS.md D-011.
- **Mock transaction**: Timer-based simulation in `main.qml` validates UI wiring end-to-end before real backend integration. See DECISIONS.md D-012.
