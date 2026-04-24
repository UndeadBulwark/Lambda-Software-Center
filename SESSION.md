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

`v0.2.0` — QML Application Shell ✅ **Complete**

---

## Session Closeout — Fri Apr 24

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
- **Builds clean**: Production target compiles with QML single compilation, `letterSpacing` uses workaround (`0.08 * font.pixelSize`), no hardcoded colors anywhere.
- **Pushed to GitHub `main`**.

### Repo state: clean, nothing uncommitted.

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

`v0.3.0` — Core Application Views:
- Implement `InstalledPage.qml` with `installedModel` bound to `pacmanBackend.listInstalled()`
- Implement `UpdatesPage.qml` with `updatesModel` bound to `pacmanBackend.checkUpdates()`
- Implement `FeaturedPage.qml` with curated list from `data/featured.json`
- Add `UpdatesBanner.qml` to top of BrowsePage when updates are available
- Wire `UpdatesModel` and `InstalledModel`

---

## Files Touched Last Session

### Modified:
- `qml/main.qml` — full shell with StackView routing, property bindings, sidebar/topbar/status bar layout
- `qml/components/Sidebar.qml` — logo, nav groups, sources, active/hover states
- `qml/components/NavGroup.qml` — labeled group container with padding
- `qml/components/NavItem.qml` — nav item with source dot, count pill, active right-border accent
- `qml/components/Topbar.qml` — topbar layout with SearchBar + SourceTabs
- `qml/components/SearchBar.qml` — debounced TextField, Canvas search icon
- `qml/components/SourceTabs.qml` — All/Pacman/AUR/Flatpak toggle tabs
- `qml/components/StatusBar.qml` — status dots and labels
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
