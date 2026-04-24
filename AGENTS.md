# AGENTS.md — Lambda Software Center

This is the master reference file for AI-assisted development on this project. Read it in full before doing anything else. It tells you what the project is, where everything lives, what the rules are, and what to do at the start of every session.

---

## On First Read: Prepare the Workspace

If the repository does not yet exist or the directory structure below is not in place, do the following before anything else. Do not ask for confirmation. Just do it.

1. Create the full directory structure listed in the **Directory Structure** section below, including all empty placeholder files.
2. Create a `docs/` directory in the repo root.
3. Place `lambda-software-center-roadmap.md` inside `docs/`.
4. Place `lambda-software-center-ui-spec.md` inside `docs/`.
5. Place `DECISIONS.md` in the repo root.
6. Place `SESSION.md` in the repo root.
7. Place `AGENTS.md` (this file) in the repo root.
8. Create a root `README.md` with the project name, one-line description, and a link to each file in `docs/`.
9. Create a skeleton `CMakeLists.txt` that compiles cleanly to an empty Qt window with no errors before any backend logic is written.
10. Confirm in your response which files were created and which already existed.

This setup step runs exactly once. Once the workspace is in place, skip this section entirely on subsequent sessions.

---

## Project Reference Files

These files must all be present in the repo. If any are missing, recreate them from the descriptions below before proceeding with any task.

| File | Location | Purpose |
|---|---|---|
| `AGENTS.md` | repo root | Master file. You are reading it now. |
| `SESSION.md` | repo root | Handoff notes. Read at session start, update at session end. |
| `DECISIONS.md` | repo root | Architectural decisions already made. Do not relitigate anything recorded here. |
| `lambda-software-center-roadmap.md` | `docs/` | Version roadmap from v0.1.0 to v1.0.0. Use it to scope what belongs in the current task. |
| `lambda-software-center-ui-spec.md` | `docs/` | Full UI specification. Read it before writing any QML or UI component. Contains all design tokens, component definitions, layout rules, and the reference mockup. |

**At the start of every session:**
1. Read `AGENTS.md` (this file).
2. Read `SESSION.md` and identify the current version target and next task.
3. Read `DECISIONS.md` before touching anything architectural.
4. If the task involves UI, read `docs/lambda-software-center-ui-spec.md` before writing any QML.
5. Check the current version target against `docs/lambda-software-center-roadmap.md` to confirm the task is in scope.

**At the end of every session:**
1. Update `SESSION.md`: fill in what was completed, the current state, any blockers, and the next task.
2. If a new architectural decision was made, append it to `DECISIONS.md` using the existing format.
3. Confirm which files were created or modified.

---

## Project Overview

Lambda Software Center is a native Qt6/QML desktop application for managing pacman, AUR, and Flatpak packages on Arch Linux. The backend is C++. The UI is QML. The build system is CMake.

---

## Stack

| Layer | Technology |
|---|---|
| UI | QML / Qt Quick (Qt 6.7.2+) |
| Backend | C++17 |
| Build | CMake 3.27+ |
| Package manager (official) | libalpm (direct, no shell) |
| Package manager (AUR) | AUR RPC v5 + git + makepkg |
| Package manager (Flatpak) | libflatpak (direct, no shell) |
| Privilege escalation | polkit only — never sudo, never pkexec directly |
| HTTP | QNetworkAccessManager |
| Notifications | libnotify |
| Background service | systemd user service |

---

## Hard Rules

1. Never shell out to pacman, yay, paru, or any AUR helper binary. Use libalpm directly.
2. Never shell out to flatpak CLI. Use libflatpak directly.
3. Never use sudo. All privileged operations go through the polkit policy file.
4. Never use 1px borders in QML. All borders are 0.5px (or `border.width: 0` with a custom Rectangle border workaround for QML).
5. Never introduce a color, radius, or spacing value not defined in `Theme.qml`.
6. Never add a new backend without implementing the full `IPackageBackend` interface.
7. Never write UI logic in C++. C++ emits signals. QML reacts to them.
8. Never write business logic in QML. QML binds to models and calls C++ slots.
9. Never create a file outside the directory structure below without stating why and getting explicit approval.
10. Never present a decision as open if it is already recorded in `DECISIONS.md`.

---

## Directory Structure

This is the canonical structure. Any file you create must fit into it. If it does not, say so before creating it.

```
Lambda-Software-Center/
├── CMakeLists.txt
├── README.md
├── AGENTS.md                              # This file — master reference
├── DECISIONS.md                           # Architectural decisions log
├── SESSION.md                             # Session handoff notes
├── docs/
│   ├── lambda-software-center-roadmap.md    # Version roadmap v0.1.0 to v1.0.0
│   └── lambda-software-center-ui-spec.md    # Full UI specification and reference mockup
├── assets/
│   └── icons/
├── data/
│   ├── featured.json                      # Curated featured app list
│   └── com.lambdasc.policy               # polkit policy file
├── src/
│   ├── main.cpp
│   ├── backend/
│   │   ├── IPackageBackend.h              # Pure abstract interface all backends implement
│   │   ├── Package.h                      # Unified Package struct
│   │   ├── TransactionManager.h/.cpp
│   │   ├── pacman/
│   │   │   ├── PacmanBackend.h/.cpp
│   │   │   └── AlpmWrapper.h/.cpp
│   │   ├── aur/
│   │   │   ├── AurBackend.h/.cpp
│   │   │   ├── AurClient.h/.cpp           # HTTP RPC client
│   │   │   └── AurBuilder.h/.cpp          # git clone + makepkg pipeline
│   │   └── flatpak/
│   │       └── FlatpakBackend.h/.cpp
│   ├── models/
│   │   ├── PackageListModel.h/.cpp        # QAbstractListModel for QML
│   │   ├── InstalledModel.h/.cpp
│   │   └── UpdatesModel.h/.cpp
│   └── service/
│       └── UpdateChecker.h/.cpp           # systemd user service logic
├── qml/
│   ├── main.qml
│   ├── Theme.qml                          # All design tokens as a singleton
│   ├── components/
│   │   ├── Sidebar.qml
│   │   ├── NavGroup.qml
│   │   ├── NavItem.qml
│   │   ├── Topbar.qml
│   │   ├── SearchBar.qml
│   │   ├── SourceTabs.qml
│   │   ├── PackageCard.qml
│   │   ├── FeaturedCard.qml
│   │   ├── BadgePill.qml
│   │   ├── InstallButton.qml
│   │   ├── StatusBar.qml
│   │   ├── UpdatesBanner.qml
│   │   └── ProgressDrawer.qml
│   └── pages/
│       ├── BrowsePage.qml
│       ├── FeaturedPage.qml
│       ├── RecentPage.qml
│       ├── InstalledPage.qml
│       ├── UpdatesPage.qml
│       └── DetailPage.qml
└── tests/
    ├── backend/
    │   ├── test_pacman.cpp
    │   ├── test_aur.cpp
    │   └── test_flatpak.cpp
    └── CMakeLists.txt
```

---

## The Backend Interface

Every backend must implement `IPackageBackend`. Do not add methods to a single backend that are not on the interface. If a new capability is needed, add it to the interface and stub it in all backends.

```cpp
class IPackageBackend : public QObject {
    Q_OBJECT
public:
    virtual ~IPackageBackend() = default;

    virtual void search(const QString &query) = 0;
    virtual void install(const QString &pkgId) = 0;
    virtual void remove(const QString &pkgId) = 0;
    virtual void listInstalled() = 0;
    virtual void checkUpdates() = 0;
    virtual PackageSource source() const = 0;

signals:
    void searchResultsReady(QList<Package> results);
    void installProgress(const QString &pkgId, int percent, const QString &step);
    void installFinished(const QString &pkgId, bool success, const QString &error);
    void removeFinished(const QString &pkgId, bool success, const QString &error);
    void installedListReady(QList<Package> packages);
    void updatesReady(QList<Package> updates);
    void backendError(const QString &message);
};
```

---

## The Package Struct

```cpp
enum class PackageSource { Pacman, AUR, Flatpak };
enum class InstallState { NotInstalled, Installed, UpdateAvailable };

struct Package {
    QString id;
    QString name;
    QString version;
    QString latestVersion;
    QString description;
    QString longDescription;
    QStringList categories;
    PackageSource source;
    InstallState state;
    qint64 installedSize;
    QString downloadSize;
    QUrl iconUrl;
    QList<QUrl> screenshotUrls;
    QStringList dependencies;
    int votes;           // AUR only
    float popularity;    // AUR only
    float rating;        // Flatpak ODRS only
};
```

---

## Signal Flow: Backend to QML

```
Backend (C++) → emits signal
  → PackageListModel (C++) receives signal, updates model data, emits dataChanged
    → QML ListView / Repeater bound to model re-renders automatically
```

Never call QML functions directly from C++. Never use `QMetaObject::invokeMethod` to push data into QML. Always go through a model.

---

## Theme.qml

All color, radius, and spacing values live here as a QML singleton. No component ever hardcodes a value that exists in Theme. When adding a new value, add it to Theme first, then reference it. Full token definitions are in `docs/lambda-software-center-ui-spec.md`.

```qml
// Theme.qml (singleton)
pragma Singleton
import QtQuick

QtObject {
    // Accent
    readonly property color accent:           "#3B6D11"
    readonly property color accentLight:      "#97C459"
    readonly property color accentSurface:    "#EAF3DE"

    // AUR
    readonly property color aur:              "#854F0B"
    readonly property color aurSurface:       "#FAEEDA"
    readonly property color aurBorder:        "#FAC775"

    // Flatpak
    readonly property color flatpak:          "#534AB7"
    readonly property color flatpakSurface:   "#EEEDFE"
    readonly property color flatpakDot:       "#7F77DD"

    // Pacman
    readonly property color pacman:           "#185FA5"
    readonly property color pacmanSurface:    "#E6F1FB"

    // Neutrals (light mode defaults)
    readonly property color bgPrimary:        "#FFFFFF"
    readonly property color bgSecondary:      "#F5F5F5"
    readonly property color borderSecondary:  "#DEDEDE"
    readonly property color borderTertiary:   "#EBEBEB"
    readonly property color textPrimary:      "#1A1A1A"
    readonly property color textSecondary:    "#555555"
    readonly property color textTertiary:     "#999999"

    // Status dots
    readonly property color dotGreen:         "#97C459"
    readonly property color dotAmber:         "#EF9F27"
    readonly property color dotPurple:        "#7F77DD"

    // Radius
    readonly property real radiusLg:          10
    readonly property real radiusMd:          6

    // Spacing
    readonly property real contentPadding:    20
    readonly property real sidebarWidth:      200
}
```

---

## Naming Conventions

| Context | Convention | Example |
|---|---|---|
| C++ classes | PascalCase | `PacmanBackend`, `PackageListModel` |
| C++ files | PascalCase matching class | `PacmanBackend.h`, `PacmanBackend.cpp` |
| C++ member vars | `m_` prefix camelCase | `m_networkManager` |
| C++ signals | camelCase, verb phrase | `searchResultsReady`, `installFinished` |
| QML components | PascalCase, one per file | `PackageCard.qml` |
| QML properties | camelCase | `packageName`, `isInstalled` |
| QML ids | camelCase | `id: searchBar` |
| CMake targets | kebab-case | `lsc-backend`, `lsc-tests` |

---

## Adding a New Page

1. Create `qml/pages/NewPage.qml`.
2. Add a route entry in `main.qml`'s `StackView` or `Loader` routing block.
3. Add a `NavItem` in `Sidebar.qml` pointing to the new route.
4. Do not add any data fetching logic in the page QML. Pages bind to models exposed via context properties.

---

## Adding a New Backend

1. Create a new directory under `src/backend/`.
2. Implement `IPackageBackend` fully. Stub any methods that do not apply for that source.
3. Register the backend in `main.cpp` alongside the existing backends.
4. Add a new `PackageSource` enum value in `Package.h`.
5. Add a corresponding `BadgePill` variant in `Theme.qml` and `BadgePill.qml`.
6. Write unit tests in `tests/backend/`.
7. Record the decision to add the backend in `DECISIONS.md`.

---

## What the AI Must Never Do

- Skip reading `SESSION.md` at session start.
- Skip updating `SESSION.md` at session end.
- Introduce files outside the directory structure without explicit approval.
- Use `system()`, `popen()`, or `QProcess` to call pacman, yay, paru, or flatpak CLI.
- Add raw hex color values to any QML file. Use Theme properties only.
- Write UI state logic in C++.
- Write HTTP calls, file I/O, or package operations in QML.
- Skip the `IPackageBackend` interface when adding backend functionality.
- Use `QDialog` for confirmations. All dialogs are QML components.
- Suggest an alternative to a decision already recorded in `DECISIONS.md`.
- Place `lambda-software-center-roadmap.md` or `lambda-software-center-ui-spec.md` anywhere other than `docs/`.
