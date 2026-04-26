# DECISIONS.md — Lambda Software Center

A log of non-obvious architectural and design decisions. Before suggesting an alternative approach, check here first. If a decision is recorded here it has already been made and should not be relitigated without a concrete new reason.

---

## D-001 — libalpm directly, not a helper binary

**Decision:** Talk to libalpm via its C API directly, not via yay, paru, or any other AUR helper.

**Why:** Shelling out to a helper binary means the app depends on a third-party tool being installed, makes error handling brittle (parsing terminal output), and prevents proper progress reporting. libalpm is the same library pacman itself uses. It gives us transaction locking, conflict callbacks, and progress signals at the C level.

**Trade-off:** More implementation work upfront. libalpm's API is low-level and requires careful transaction lifecycle management.

---

## D-002 — libflatpak directly, not flatpak CLI

**Decision:** Use libflatpak C library directly, not the flatpak command-line tool.

**Why:** Same reasoning as D-001. The CLI has no stable machine-readable output format, error handling is string matching, and progress reporting is lossy. libflatpak is the correct programmatic interface.

**Trade-off:** libflatpak documentation is sparse. Reference the GNOME Software source as a secondary guide.

---

## D-003 — polkit for privilege escalation, not sudo

**Decision:** All privileged pacman operations are authorized through a polkit policy file. We do not call sudo, pkexec directly, or ask the user for a password in our own UI.

**Why:** polkit is the correct desktop-grade privilege escalation mechanism. It integrates with the session agent (KDE, GNOME, etc.), respects the user's authentication timeout, and does not require storing or handling passwords in our process. sudo is a terminal tool, not an API.

**Trade-off:** Requires a polkit policy file to be installed to `/usr/share/polkit-1/actions/`. This must be part of the package installation.

---

## D-004 — QML for UI, C++ for everything else

**Decision:** The UI layer is entirely QML. No business logic, no data fetching, no package operations in QML. C++ handles all backend work and exposes data via models.

**Why:** Keeps a clean separation between UI and logic. QML is declarative and well-suited for reactive UI. C++ is where libalpm and libflatpak live. Mixing them produces untestable spaghetti.

**Trade-off:** Requires discipline. The boundary must be enforced: QML binds to models and calls C++ slots, C++ emits signals and updates models.

---

## D-005 — AUR RPC v5 for search, git + makepkg for builds

**Decision:** AUR package metadata and search comes from the AUR RPC API (`aur.archlinux.org/rpc/v5`). Building AUR packages uses git clone of the package's AUR repository followed by makepkg.

**Why:** The RPC API is the official, stable, documented interface for AUR metadata. There is no alternative. For building, makepkg is the only correct tool — it handles the PKGBUILD format, checksums, and produces a proper `.pkg.tar.zst` that libalpm can install.

**Trade-off:** AUR builds require git and makepkg to be present on the user's system. These are standard Arch tools and can be listed as hard dependencies.

---

## D-006 — No vendored AUR helper

**Decision:** We do not vendor or depend on yay, paru, or any other AUR helper.

**Why:** AUR helpers are user-space tools with their own opinions and configurations. Depending on one means inheriting their bugs, their config format, and their update cadence. We implement only what we need: RPC search and makepkg build. That is sufficient.

**Trade-off:** We reimplement a subset of what helpers do. Acceptable given the scope is a GUI app, not a general-purpose helper.

---

## D-007 — systemd user service for background update checks

**Decision:** The update notification daemon runs as a systemd user service, not a tray icon process or login item.

**Why:** Systemd user services are the correct mechanism on modern Arch systems. They start on login, respect system resources, can be enabled and disabled by the user, and integrate with journald for logging. A persistent tray process is wasteful for something that only needs to run periodically.

**Trade-off:** The service is opt-in and must be enabled explicitly. This is by design, not a limitation.

---

## D-008 — AppStream for metadata, not scraping

**Decision:** Package descriptions, icons, screenshots, and categories are sourced from AppStream XML (`/usr/share/metainfo`) and Flatpak's AppStream data. We do not scrape the web or the AUR web interface.

**Why:** AppStream is a standardized, machine-readable metadata format. It is already present on the user's system for packages that support it. Flatpak's remote AppStream data is fetched as part of the Flatpak refresh cycle. Scraping is fragile and potentially against terms of service.

**Trade-off:** Many pacman packages and most AUR packages do not have AppStream metadata. For these we fall back to the package description and a generated icon. No screenshots for non-AppStream packages.

---

## D-009 — PKGBUILD review is mandatory for AUR installs, not optional

**Decision:** The install flow for AUR packages always shows the PKGBUILD and requires the user to proceed past it. This step cannot be skipped on first install of a package.

**Why:** AUR packages are community-submitted and unreviewed by Arch. Running a PKGBUILD without reading it is a known security risk. The app makes an explicit choice to surface this rather than hide it for convenience.

**Trade-off:** Adds friction to AUR installs. This is intentional.

---

## D-010 — Flatpak variant disables pacman and AUR backends

**Decision:** If the app is distributed as a Flatpak (for users who want a sandboxed install), the pacman and AUR backends are disabled at compile time for that build target. The Flatpak build manages only Flatpak packages.

**Why:** A Flatpak sandbox cannot access libalpm or the host package database. Attempting to do so would either fail silently or require broad filesystem exceptions that defeat the purpose of sandboxing.

**Trade-off:** The Flatpak version is a reduced-capability build. Users who want full functionality install from AUR.

---

## D-011 — Modal overlays are global ApplicationWindow children, not StackView items

**Decision:** `ConfirmDialog` and `ProgressDrawer` are declared as direct children of `ApplicationWindow` in `main.qml`, outside the `StackView`. Child pages reference them via `Window.window.confirmDialog`. They are not pushed onto the `StackView`.

**Why:** Modal overlays must render above all routed page content regardless of StackView depth. If they were StackView items, they would be clipped by the StackView bounds and could be obscured by sibling StackView pages. Global z-ordering via `z` property is simpler and more reliable when the overlay sits above the StackView in the visual parent hierarchy.

**Trade-off:** Pages must know the global IDs or use `Window.window` lookup. This couples the page to the parent window structure. We mitigate by using consistent naming conventions (`confirmDialog`, `progressDrawer`) and ensuring all pages are children of the same window.

---

## D-012 — Mock transaction flow validates UI before backend integration

**Decision:** Before wiring `ConfirmDialog` → `TransactionManager` → `libalpm`, we implemented a mock transaction using a QML `Timer` inside `main.qml`. The Timer simulates download → verify → install → complete over 3 seconds, driving the `ProgressDrawer` visuals.

**Why:** Backend transaction logic for libalpm is complex (privilege escalation via polkit, transaction lifecycle, error callbacks). Building the UI with mocked signals lets us validate the entire visual chain—dialog → progress → completion → auto-hide—without the risk of system package changes during development. When the real backend is ready, we replace the Timer's `onTriggered` with `transactionManager.install(pkgId)`; the `ProgressDrawer` bindings remain unchanged.

**Trade-off:** The mock is dead code once backend integration is complete. It lives in `main.qml` as a temporary `Timer`. We will remove it in the commit that wires real transactions.

---

## D-013 — Sidebar source items removed; SourceTabs handles filtering

**Decision:** The "Sources" NavGroup (Pacman / AUR / Flatpak) was removed from `Sidebar.qml`. Source filtering is handled exclusively by `SourceTabs.qml` in the content area.

**Why:** The sidebar source items were cosmetic (no click handler, no `requestPage` signal, no active state). `SourceTabs` already provides All/Pacman/AUR/Flatpak filtering in the topbar area with full interactivity. Having the same concept in two places was redundant and the non-functional sidebar items were misleading to users.

**Trade-off:** Users cannot navigate to a dedicated source page from the sidebar. SourceTabs already provides this functionality inline with search results, which is the more natural UX — you filter when you're looking at packages, not before navigating.

---

## D-014 — Orphan detection runs automatically after pacman remove; cleanup is a separate polkit transaction

**Decision:** After every successful pacman remove, `AlpmWrapper::findOrphans()` scans the local DB for packages with `ALPM_PKG_REASON_DEPEND` and an empty `requiredby` list. If orphans are found, an `OrphanDialog` prompts the user with per-package checkboxes (all checked by default, select/deselect all). Cleanup runs as a separate `pkexec lsc-helper remove-orphans` invocation with `ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE`.

**Why:** The roadmap specifies "orphan detection and follow-up cleanup prompt." Auto-detecting after remove is the most natural UX — the user just removed a package, so they're in a cleanup mindset. Separate polkit auth means the user can review orphans before committing to removal, and the original remove is already committed even if they skip cleanup. Per-package checkboxes prevent accidental removal of packages with dirty DEPEND flags (CachyOS/CachyOS meta-package issue).

**Trade-offs:**
- Two polkit prompts for a "remove + cleanup" flow. Acceptable because the user explicitly opts into cleanup.
- `alpm_pkg_compute_requiredby()` misses dependency cycles (two DEPEND-packages that only require each other). Noted with a comment in code — same limitation as `pacman -Qdt`. Extremely rare in practice on Arch.
- AUR and Flatpak removes don't trigger orphan detection (only pacman). AUR doesn't have a local dep tree, and Flatpak has its own runtime dependency model not managed by alpm.
- Unchecked orphan selections are discarded on Skip — no persistent ignore list yet (noted as TODO in OrphanDialog.qml).

---

## D-015 — Install reason EXPLICIT flag set on every LSC install; one-time repair on first launch

**Decision:** Two-part fix for the dirty install reason flag problem:

1. **Post-install flag:** After every successful `lsc-helper install`, `alpm_pkg_set_reason(pkg, ALPM_PKG_REASON_EXPLICIT)` is called on the just-installed package. This prevents LSC from ever contributing dirty DEPEND flags.

2. **One-time repair:** On first launch, if sentinel file `~/.cache/lambda-software-center/reason-repair-done` doesn't exist, `AlpmWrapper::findDirtyReasons()` scans the local DB for `ALPM_PKG_REASON_DEPEND` packages that have a `.desktop` file in `/usr/share/applications/`. These are promoted to EXPLICIT via `pkexec lsc-helper set-reason`. Sentinal is created regardless of whether packages were found (prevents re-scan on clean systems).

**Why:** CachyOS and other Arch derivatives with meta-packages sometimes mark explicitly-desired packages as DEPEND instead of EXPLICIT. This causes `findOrphans()` to flag user-facing apps like firefox and corectrl for removal. Heuristic: `.desktop` file = user-facing application = should be EXPLICIT. This is the freedesktop.org standard — every GUI app installs one, libs/build tools don't.

**Trade-offs:**
- `alpm_pkg_set_reason()` requires root (writes to `/var/lib/pacman/local/`). Post-install flag is embedded in `do_install()` (already running as root via pkexec). One-time repair uses new `set-reason` helper action (separate pkexec prompt).
- False positives possible: a DEPEND package with a `.desktop` file that was legitimately installed as a dep. Extremely rare — if it has a `.desktop` file, it's a user-facing app the user likely wants to keep.
- One-time repair is silent (no dialog). The fix is harmless metadata correction, not worth interrupting the user.
- Repair is one-shot. The sentinel prevents re-running. LSC's own installs are always EXPLICIT post-fix, and OrphanDialog checkboxes protect against remaining dirty flags from other tools.

---

## D-017 — System upgrade uses alpm_sync_sysupgrade; selective upgrade uses alpm_add_pkg per-package

**Decision:** The `upgrade` helper action supports two modes: (1) full system upgrade with `alpm_sync_sysupgrade()` (marks all upgradable packages), (2) selective upgrade with explicit `alpm_add_pkg()` for N named packages. Both use `alpm_trans_init(ALLDEPS)`, `alpm_trans_prepare`, and `alpm_trans_commit` with the same progress callbacks as install.

**Why:** `alpm_sync_sysupgrade` is the standard pacman `-Syu` path — it handles dependency resolution, replaces, and conflicts for the full upgrade set. Selective upgrade is needed for per-package "Update" buttons in the Updates page. Post-commit `alpm_pkg_set_reason(EXPLICIT)` is applied in selective mode only (full upgrade doesn't change install reasons).

**Trade-offs:**
- Full system upgrade is binary — no "exclude" support. Per-package exclusion is not yet supported (deferred to later if needed).
- Selective upgrade skips packages not explicitly named. If a user clicks "Update" on one package, its dependencies may also need upgrading. `ALLDEPS` ensures deps are pulled in, but only if they're available in the sync DBs.
- TransactionManager routes upgrade through distinct `upgradeStarted/Progress/Finished` signals to avoid collision with install/remove transaction signals.

---

## D-018 — AUR update detection via AlpmWrapper::listForeignPackages() + AurClient::info() RPC comparison

**Decision:** AUR updates are detected by listing "foreign" packages (installed locally but not in any sync DB) via `AlpmWrapper::listForeignPackages()`, then querying AUR RPC `/info` for those packages, and comparing versions using heuristic string splitting.

**Why:** There is no AUR-specific local database. AUR packages are installed via `makepkg` and end up in the pacman local DB, but they have no corresponding sync DB entry. The only way to detect AUR packages is by their absence from sync DBs. AUR RPC `/info` provides the latest version for comparison.

**Trade-offs:**
- `listForeignPackages()` scans the entire local DB and all sync DBs. O(n²) in the worst case (local packages × sync packages). On a typical Arch system with ~1300 local and ~25000 sync packages, this takes ~1-2 seconds. Acceptable for a periodic check.
- The version comparison is a heuristic (splits on `.` and `-`, compares numeric segments when both are numbers, otherwise string). It does not use `alpm_pkg_vercmp()` because we're comparing AUR version strings outside an alpm handle. May produce false positives/negatives for unusual version schemes. `alpm_pkg_vercmp` could be used if we parse the versions through a temporary alpm handle, but that's over-engineering for now.
- The AUR RPC `/info` endpoint supports up to ~200 packages per request. Users with more than 200 AUR packages (extremely rare) would need batching. Not implemented yet.
- Flatpak updates are not checked (no libflatpak). Blocked until libflatpak is installed.

---

## D-016 — AUR install flow: git clone → PKGBUILD review → makepkg → install-local

**Decision:** AUR packages are installed via a multi-step pipeline: `AurBuilder.gitClone()` clones the AUR repo into `~/.cache/lambda-software-center/aur/<name>/`, the PKGBUILD is read from disk and shown in a `PkgbuildDialog` (mandatory review per D-009), `AurBuilder.makepkg()` runs `makepkg --syncdeps --noconfirm`, and the resulting `.pkg.tar.zst` is installed via `pkexec lsc-helper install-local <filepath>` (same libalpm transaction path as regular installs).

**Why:** This matches the standard Arch AUR workflow. `makepkg --syncdeps` handles build dependency resolution internally (calling sudo pacman as needed). The PKGBUILD review step is mandatory per D-009. Using `install-local` with `alpm_pkg_load` + `alpm_trans_commit` keeps the install path in libalpm rather than shelling out to `pacman -U`.

**Trade-offs:**
- `makepkg --syncdeps` will trigger its own polkit prompt (via sudo) for build deps. Two auth prompts per AUR install is unavoidable without a custom makepkg wrapper.
- Build dir is deleted-and-recloned on every install (no incremental pull). Simpler, avoids merge conflicts, negligible cost for small AUR repos.
- Search result cache (`QHash<QString, Package>`, lifetime of object, no expiry) is used to resolve `gitUrl` at install time. User must search before installing; error emitted if package not in cache.
- `alpm_pkg_set_reason(EXPLICIT)` is called post-install inside the helper (same as D-015), so AUR packages are always correctly marked.
- AUR update from UpdatesPage: routes through `aurBackend.install()` which triggers the full PKGBUILD review flow. ConfirmDialog (showing dependencies/size) is skipped for updates from the Updates page — PKGBUILD review is the mandatory gate per D-009. This is acceptable because: (1) the user already knows what they're updating, (2) PKGBUILD review provides the security gate, (3) dependency info is less relevant for updates.

---

## D-019 — removeFinished signal now properly emitted for remove operations

**Decision:** Both `PacmanBackend` and `AurBackend` now track whether the current transaction is an install or remove via `m_isRemove` flag, and emit `removeFinished`/`removeProgress` instead of `installFinished`/`installProgress` for remove operations. Previously, both backends unconditionally emitted `installFinished` on transaction completion, which meant the UI ProgressDrawer would never close after a successful remove.

**Why:** The `IPackageBackend` interface defines both `installFinished` and `removeFinished` signals, but the `TransactionManager` only emits `transactionFinished` (a single signal for both operations). The backends need to disambiguate which signal to re-emit based on whether they initiated an install or remove.

**Trade-offs:**
- The `m_isRemove` flag is reset after each transaction. If two rapid remove operations are queued, only the last one's signal routing is correct. Since `TransactionManager::busy()` prevents concurrent operations, this is safe.
- `PacmanBackend` now also emits `removeProgress` during remove operations (previously silent). This means the ProgressDrawer will show step labels like "Removing" during pacakge removal, which is useful UX.
