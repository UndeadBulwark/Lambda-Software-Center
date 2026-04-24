# Lambda Software Center — Version Roadmap

---

## Languages

- **C++** — all backend logic, libalpm wrapper, libflatpak wrapper, AUR HTTP client, transaction manager
- **QML** — the entire UI layer, all components, routing, animations
- **CMake** — build system

No Python or scripting layer unless plugin support is added later (optional).

---

## Tools and Libraries

| Tool / Library | What it does |
|---|---|
| Qt 6.6+ | The entire framework: window, networking, threading, models |
| QML / Qt Quick | UI rendering and component system |
| libalpm | Arch's native pacman library, used directly instead of shelling out |
| libflatpak | Flatpak's C library for managing Flatpak packages |
| QNetworkAccessManager | HTTP client for AUR RPC v5 API calls |
| polkit | Privilege escalation for pacman transactions |
| AppStream / AppStreamQt | Package metadata, icons, screenshots, content ratings |
| git | Cloning PKGBUILDs from AUR |
| makepkg | Building AUR packages from PKGBUILDs |
| libnotify | Desktop notifications from the background service |
| systemd (user service) | Background update checker daemon |
| GitHub Actions | CI pipeline: build, test, QML lint |

The core architectural decision is that libalpm and libflatpak are linked C libraries called directly, not wrapped shell commands. That is what makes this a native app rather than a glorified terminal wrapper.



---

## v0.1.0 — Backend Foundation

The app does not exist yet. This version is a headless library and CLI harness that proves all three backends work.

- libalpm wrapper with search, install, remove, list installed, check updates
- AUR RPC v5 client over QNetworkAccessManager, PKGBUILD clone and makepkg pipeline
- libflatpak wrapper matching the same interface
- Unified `Package` struct shared across all three backends
- `TransactionManager` skeleton: queue, execute, emit progress signals
- polkit policy file for privileged pacman operations
- CLI test harness that exercises every backend function
- Unit tests with mocked HTTP responses and sandboxed pacman DB

**Exit criteria:** All three backends can search, install, and remove packages from the command line with no UI.

---

## v0.2.0 — Application Shell

The window exists. No install functionality yet, just navigation and live data rendering.

- `ApplicationWindow` with QML `StackView` routing
- Sidebar: Discover (Browse, Featured, Recent), Library (Installed, Updates), Sources (Pacman, AUR, Flatpak)
- `Theme.qml` singleton encoding all color tokens, radii, and spacing from the mockup
- Reusable QML components: `PackageCard`, `FeaturedCard`, `BadgePill`, `SourceTab`, `StatusBar`, `UpdatesBanner`
- `QAbstractListModel` subclasses wired to each backend, exposed to QML via context properties
- Debounced search bar (250ms) with source filter tabs
- Status bar showing backend readiness and total package count

**Exit criteria:** The app launches, searches all three sources, and renders cards with live data. Nothing installs yet.

---

## v0.3.0 — Package Detail View

Clicking a card opens a full detail panel before any action is taken.

- Slide-in detail sheet or routed detail page
- Full description, version, installed size, download size, source badge, dependencies list
- PKGBUILD viewer for AUR packages (plaintext, syntax highlighted)
- AppStream metadata integration for packages that have it: long descriptions, content ratings, categories
- Flatpak screenshot carousel where AppStream data is available
- AUR vote count and popularity score displayed
- Flatpak ODRS rating displayed where available
- Install and Remove buttons present but gated behind v0.4.0

**Exit criteria:** Every card has a working detail view with real metadata.

---

## v0.4.0 — Install and Remove

The app can make changes to the system.

- Install flow: confirmation dialog showing dependencies, size delta, and source
- AUR install flow adds a mandatory PKGBUILD review step before building
- Progress drawer at the bottom of the window with per-step status (download, verify, build, install)
- Remove flow with orphan detection and follow-up cleanup prompt
- polkit elevation integrated and tested for pacman transactions
- Transaction error handling with readable failure messages
- Install and Remove buttons live in the detail view

**Exit criteria:** A user can install and remove packages from all three sources through the UI with no terminal required.

---

## v0.5.0 — Update Manager

The updates banner in the mockup is functional.

- Full update scan on launch across pacman, AUR, and Flatpak
- Updates page listing each pending update with current version, new version, and changelog where available (Flatpak has this; AUR does not reliably)
- "Update all" runs a sequenced full upgrade across all three backends
- Per-package update from the updates list
- Update progress shown in the same progress drawer from v0.4.0
- AUR updates surface the PKGBUILD diff between installed and incoming versions

**Exit criteria:** A user can update their entire system from within the app.

---

## v0.6.0 — Discovery and Curation

The Browse and Featured views have real curated content.

- Curated featured list shipped as a versioned JSON feed, updatable independently of the app
- Category system mapped to AppStream categories
- Recent additions feed pulled from AUR's RPC `by=submittedafter` and Flatpak remote metadata
- Installed view shows all installed packages across all three sources in a unified list with source badges
- Icon pipeline: AppStream icons preferred, Flatpak icons from remote, AUR falls back to generated initials icon, all cached to disk

**Exit criteria:** The Discover section feels like a real storefront, not a raw package list.

---

## v0.7.0 — Background Service and Notifications

The app can alert the user to updates without being open.

- systemd user service that checks for updates on a configurable schedule
- Desktop notification via libnotify when updates are available, with a count
- Clicking the notification opens the Updates page directly
- Notification respects a quiet hours setting
- Service is opt-in and installed separately from the main app, enabled via the settings page

**Exit criteria:** A user who never opens the app still gets notified when updates are ready.

---

## v0.8.0 — Settings and Configuration

The app is configurable without editing files manually.

- Settings page accessible from the sidebar
- AUR toggle (enable or disable AUR backend entirely)
- PKGBUILD auto-review toggle (require manual review before every AUR build vs. trust repeat packages)
- Flatpak remote management: add, remove, and prioritize remotes beyond Flathub
- Update check frequency selector for the background service
- Theme selector: follow system, force light, force dark
- Cache management: clear icon cache, clear package metadata cache

**Exit criteria:** All meaningful user-facing behaviors are configurable through the UI.

---

## v0.9.0 — Polish, Accessibility, and Performance

Feature complete. This version is about quality.

- Full keyboard navigation through sidebar, card grids, and detail views
- Screen reader labels on all interactive elements
- Virtual list rendering for large result sets using QML ListView margin tuning
- Lazy icon loading with async fetch and local disk cache
- Pacman DB sync cached in memory, re-synced on explicit refresh or timed interval
- Smooth transitions between routes and for the detail sheet
- All known bugs from v0.4.0 through v0.8.0 resolved
- End-to-end integration test suite covering install, remove, and update flows against a sandboxed environment

**Exit criteria:** The app passes a full accessibility audit and handles 14,000+ packages without perceptible lag.

---

## v1.0.0 — Stable Release

Distribution-ready. No new features, only hardening and packaging.

- PKGBUILD published to AUR
- Flatpak manifest published to Flathub (Flatpak variant disables pacman and AUR backends, manages Flatpak only)
- GitHub Actions CI: build, unit tests, QML lint, integration tests on each push
- Man page and `--help` output for any CLI flags
- Full README, contributing guide, and architecture documentation
- Polkit policy reviewed and scoped to minimum required privileges
- No known crash bugs, no known data loss paths

**Exit criteria:** Ships to users. Arch package is installable from AUR with a single `yay -S lambda-software-center` command.

---

## Summary

| Version | Theme | Key Deliverable |
|---|---|---|
| 0.1.0 | Backend Foundation | All three backends work headlessly |
| 0.2.0 | Application Shell | Navigable UI with live search data |
| 0.3.0 | Package Detail View | Full metadata, AppStream, screenshots |
| 0.4.0 | Install and Remove | System changes work end to end |
| 0.5.0 | Update Manager | Full upgrade flow across all backends |
| 0.6.0 | Discovery and Curation | Featured, categories, curated content |
| 0.7.0 | Background Service | Update notifications without the app open |
| 0.8.0 | Settings | Configurable behavior through the UI |
| 0.9.0 | Polish and Performance | Accessibility, speed, stability |
| 1.0.0 | Stable Release | Packaged and published to AUR and Flathub |
