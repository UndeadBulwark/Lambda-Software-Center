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
