# Lambda Software Center — UI Specification

---

## Instruction Block

You are building UI for **Lambda Software Center**, a native Qt/QML desktop application for managing pacman, AUR, and Flatpak packages on Arch Linux. Before writing any component, read this entire document. Match design tokens exactly. Do not introduce colors, radii, spacing values, font sizes, or layout dimensions not defined here. Do not invent new component patterns when an existing one covers the use case. When in doubt, refer to the reference mockup at the bottom of this document.

The aesthetic is **refined utilitarian**: clean, dense but not cluttered, information-forward. It is not a web app. It is not material design. It does not use shadows, gradients, or decorative flourishes. Border lines and background tints do all the visual work.

---

## Design Tokens

### Colors

#### Accent — Arch Green
| Token | Value | Usage |
|---|---|---|
| `accent` | `#3B6D11` | Primary buttons, active nav item text, active nav border, installed badge text |
| `accent-dark` | `#2F560D` | Primary button pressed state |
| `accent-light` | `#97C459` | Active tab border, installed badge border |
| `accent-surface` | `#EAF3DE` | Active tab background, installed badge background, primary button text |

#### Source — AUR Amber
| Token | Value | Usage |
|---|---|---|
| `aur` | `#854F0B` | AUR badge text, updates button background, updates count pill |
| `aur-surface` | `#FAEEDA` | AUR badge background, updates banner background, updates button text |
| `aur-border` | `#FAC775` | Updates banner border |

#### Source — Flatpak Purple
| Token | Value | Usage |
|---|---|---|
| `flatpak` | `#534AB7` | Flatpak badge text |
| `flatpak-surface` | `#EEEDFE` | Flatpak badge background |
| `flatpak-dot` | `#7F77DD` | Flatpak status dot in statusbar |

#### Source — Pacman Blue
| Token | Value | Usage |
|---|---|---|
| `pacman` | `#185FA5` | Pacman badge text |
| `pacman-surface` | `#E6F1FB` | Pacman badge background |

#### Neutrals
These map to Qt theme variables in QML. In HTML prototyping use the values below for light mode.

| Token | Light mode value | Usage |
|---|---|---|
| `--color-background-primary` | `#FFFFFF` | Main content area, cards, topbar |
| `--color-background-secondary` | `#F5F5F5` | Sidebar, statusbar |
| `--color-border-primary` | `#C8C8C8` | — (reserved) |
| `--color-border-secondary` | `#DEDEDE` | Input borders, install button (ghost), card hover |
| `--color-border-tertiary` | `#EBEBEB` | Card borders (default), section dividers, topbar border |
| `--color-text-primary` | `#1A1A1A` | App names, section titles, nav item active |
| `--color-text-secondary` | `#555555` | Descriptions, nav items inactive, ghost button text |
| `--color-text-tertiary` | `#999999` | Version numbers, ratings, nav labels, status bar items, meta text |

#### Status Dots
| Color | Value | Meaning |
|---|---|---|
| Green | `#97C459` | Pacman ready |
| Amber | `#EF9F27` | AUR helper active |
| Purple | `#7F77DD` | Flatpak ready |

---

### Border Radius

| Token | Value | Usage |
|---|---|---|
| `--border-radius-lg` | `10px` | App root, cards, featured cards |
| `--border-radius-md` | `6px` | Buttons, search input, source tabs, updates button, badge pills (use `10px` for pill shape specifically) |

Badge pills use `border-radius: 10px` explicitly, not `--border-radius-md`.

---

### Typography

All text uses the system sans-serif. No decorative fonts. No icon fonts — use inline SVG only.

| Role | Size | Weight | Color token |
|---|---|---|---|
| Section title | `15px` | `500` | `text-primary` |
| Card / app name | `14px` | `500` | `text-primary` |
| Nav label (uppercase) | `10px` | `500` | `text-tertiary` |
| Logo text | `14px` | `500` | `text-primary` |
| Body / description | `12px` | `400` | `text-secondary` |
| Global body default | `13px` | `400` | `text-primary` |
| Version / meta / rating | `11px` | `400` | `text-tertiary` |
| Badge text | `10px` | `500` | (source-specific) |
| Button text (primary) | `12px` | `400` | `accent-surface` |
| Button text (ghost) | `12px` | `400` | `text-secondary` |
| Status bar text | `11px` | `400` | `text-tertiary` |
| Updates count pill | `10px` | `400` | `aur-surface` |
| Tab text | `12px` | `400` / `500` active | (see SourceTab) |
| Detail page title | `20px` | `500` | `text-primary` |
| Confirm dialog title | `18px` | `500` | `text-primary` |
| Version picker text | `12px` | `400` | `text-secondary` |

Letter spacing on nav labels: `0.08em`.
Line height on descriptions: `1.5`. On package descriptions: `1.45`.

---

### Spacing and Layout

| Element | Value |
|---|---|
| App root height | `620px` |
| Sidebar width | `200px` (fixed, no flex shrink) |
| Content padding | `20px` |
| Topbar padding | `12px 20px` |
| Sidebar padding | `12px 0` |
| Statusbar padding | `6px 20px` |
| Nav item padding | `7px 16px` |
| Nav label padding | `0 16px 6px` |
| Nav group margin-bottom | `18px` |
| Logo padding | `10px 16px 18px` |
| Card padding (featured, pkg) | `16px` / `14px` |
| Card gap (featured grid) | `12px` |
| Card gap (pkg grid) | `10px` |
| Badge gap | `6px` |
| Section title margin-bottom | `14px` |
| Section margin-bottom | `28px` |
| Status bar item gap | `16px` |
| Status dot size | `6px × 6px` |
| Search input height | `32px` |
| Search icon left offset | `10px` |
| Search input left padding | `32px` (to clear icon) |
| Detail page padding | `20px` |
| Detail header margin-bottom | `20px` |
| Detail grid column spacing | `24px` |
| Detail grid row spacing | `10px` |
| Confirm dialog width | `420px` |
| Confirm dialog padding | `24px` |
| Confirm dialog button height | `32px` |
| Progress drawer height (collapsed) | `0` |
| Progress drawer bar height | `6px` |
| Version picker width | `150px` |
| Version picker height | `30px` |

---

## Component Catalog

---

### AppRoot
The outermost container.

```
display: flex
height: 620px
border: 0.5px solid --color-border-secondary
border-radius: --border-radius-lg
overflow: hidden
font-size: 13px
```

Children: `Sidebar` (left), `MainArea` (right, flex: 1).

---

### Sidebar
```
width: 200px
min-width: 200px
background: --color-background-secondary
border-right: 0.5px solid --color-border-tertiary
display: flex
flex-direction: column
padding: 12px 0
```

Children: `Logo`, then one or more `NavGroup`.

---

### Logo
```
padding: 10px 16px 18px
display: flex
align-items: center
gap: 8px
```

Logo icon: `28×28px`, `background: #3B6D11`, `border-radius: 7px`, contains a white/light SVG (Arch triangle). Logo text: `14px`, weight `500`, `text-primary`.

---

### NavGroup
```
margin-bottom: 18px
```

Children: `NavLabel`, then one or more `NavItem`.

**NavLabel:**
```
padding: 0 16px 6px
font-size: 10px
font-weight: 500
letter-spacing: 0.08em
color: --color-text-tertiary
text-transform: uppercase
```

---

### NavItem

**Default state:**
```
display: flex
align-items: center
gap: 10px
padding: 7px 16px
color: --color-text-secondary
cursor: pointer
transition: background 0.1s
```

**Hover state:**
```
background: --color-background-primary
color: --color-text-primary
```

**Active state:**
```
background: --color-background-primary
color: #3B6D11
font-weight: 500
border-right: 2px solid #3B6D11
```

Nav icons: `16×16px`, `opacity: 0.7` default, `opacity: 1` active. Use inline SVG, `fill: currentColor`.

Nav items can have a trailing element (e.g. updates count pill). The pill sits `margin-left: auto` inside the flex row.

**Updates count pill:**
```
background: #854F0B
color: #FAEEDA
border-radius: 10px
padding: 1px 7px
font-size: 10px
```

**Source dot** (used in Sources nav group instead of an icon):
```
width: 8px
height: 8px
border-radius: 50%
flex-shrink: 0
```
Colors: Pacman `#185FA5`, AUR `#EF9F27`, Flatpak `#7F77DD`.

---

### MainArea
```
flex: 1
display: flex
flex-direction: column
overflow: hidden
```

Children: `Topbar`, `ContentArea`, `StatusBar`.

---

### Topbar
```
padding: 12px 20px
border-bottom: 0.5px solid --color-border-tertiary
display: flex
align-items: center
gap: 12px
background: --color-background-primary
```

Children: `SearchBar` (flex: 1), `SourceTabs`.

---

### SearchBar
Wrapper is `position: relative`. Input:
```
width: 100%
height: 32px
border: 0.5px solid --color-border-secondary
border-radius: --border-radius-md
background: --color-background-secondary
padding: 0 12px 0 32px
font-size: 13px
color: --color-text-primary
outline: none
```

Search icon: `14×14px`, `position: absolute`, `left: 10px`, vertically centered, `color: --color-text-tertiary`, inline SVG (circle + line, no fill, stroke-width 1.5).

---

### SourceTabs
```
display: flex
gap: 4px
```

**Tab — default:**
```
padding: 5px 12px
border-radius: --border-radius-md
font-size: 12px
border: 0.5px solid --color-border-secondary
background: transparent
color: --color-text-secondary
cursor: pointer
```

**Tab — active:**
```
background: #EAF3DE
color: #3B6D11
border-color: #97C459
font-weight: 500
```

Tabs: All, Pacman, AUR, Flatpak.

---

### ContentArea
```
flex: 1
overflow-y: auto
padding: 20px
```

Children stacked vertically: `UpdatesBanner` (if updates exist), then named sections each starting with a `SectionTitle`.

---

### SectionTitle
```
font-size: 15px
font-weight: 500
color: --color-text-primary
margin-bottom: 14px
```

---

### UpdatesBanner
```
background: #FAEEDA
border: 0.5px solid #FAC775
border-radius: --border-radius-md
padding: 10px 14px
display: flex
align-items: center
gap: 12px
margin-bottom: 20px
```

Contains: a `14×14` amber SVG icon, a text span (`font-size: 12px`, `color: #633806`, `flex: 1`), and an "Update all" button.

**Update all button:**
```
padding: 5px 12px
background: #854F0B
color: #FAEEDA
border: none
border-radius: --border-radius-md
font-size: 11px
cursor: pointer
```

---

### FeaturedGrid
```
display: grid
grid-template-columns: repeat(2, 1fr)
gap: 12px
margin-bottom: 28px
```

Contains `FeaturedCard` components. A hero card spans both columns.

---

### FeaturedCard

**Base:**
```
border: 0.5px solid --color-border-tertiary
border-radius: --border-radius-lg
padding: 16px
background: --color-background-primary
cursor: pointer
transition: border-color 0.15s
```

**Hover:**
```
border-color: --color-border-secondary
```

**Hero variant** (spans 2 columns):
```
grid-column: span 2
display: flex
gap: 16px
align-items: flex-start
```
Contains: `AppIcon` (large, 52×52px), a meta block (`flex: 1`), and an `InstallButton` pushed to the right.

Meta block children: `AppName`, `AppDescription`, `BadgeRow`.

**Standard variant** (1 column):
```
display: flex
gap: 12px
align-items: flex-start
```
Contains: `AppIcon` (small, 40×40px), a meta block. No trailing button in the standard featured card.

---

### AppIcon

**Large (hero):**
```
width: 52px
height: 52px
border-radius: 12px
flex-shrink: 0
display: flex
align-items: center
justify-content: center
font-size: 22px
```

**Small:**
```
width: 40px
height: 40px
border-radius: 10px
font-size: 18px
```

Background color is source/app specific. Examples from mockup: Firefox `#E6F1FB`, Heroic `#EEEDFE`, Spotify `#E1F5EE`. Use the lightest surface color of the most relevant source, or a custom tint. Content is an emoji or SVG icon.

---

### AppName
```
font-size: 14px
font-weight: 500
color: --color-text-primary
margin-bottom: 3px
```

---

### AppDescription
```
font-size: 12px
color: --color-text-secondary
line-height: 1.5
margin-bottom: 10px
```

In package grid cards, `line-height: 1.45` and `margin-bottom: 10px`.

---

### BadgeRow
```
display: flex
gap: 6px
align-items: center
flex-wrap: wrap
```

Contains `BadgePill` components and optional meta text.

---

### BadgePill

All badges: `padding: 2px 8px`, `border-radius: 10px`, `font-size: 10px`, `font-weight: 500`.

| Variant | Background | Text color |
|---|---|---|
| `pacman` | `#E6F1FB` | `#185FA5` |
| `aur` | `#FAEEDA` | `#854F0B` |
| `flatpak` | `#EEEDFE` | `#534AB7` |
| `installed` | `#EAF3DE` | `#3B6D11` |

Never mix badge variants. Each badge reflects exactly one source or state.

---

### InstallButton

**Primary (Install):**
```
background: #3B6D11
border: none (or border: 0.5px solid #3B6D11)
color: #EAF3DE
border-radius: --border-radius-md
cursor: pointer
```

In hero card: `padding: 6px 14px`, `font-size: 12px`.
In pkg grid card: `padding: 4px 10px`, `font-size: 11px`.

**Ghost (Install, uninstalled):**
```
background: transparent
border: 0.5px solid --color-border-secondary
color: --color-text-secondary
border-radius: --border-radius-md
cursor: pointer
```
Same padding as primary at each context.

**Installed state:**
```
background: #EAF3DE
border: 0.5px solid #97C459
color: #3B6D11
font-weight: 500
border-radius: --border-radius-md
```
Label: "Installed". Not clickable (or opens remove flow).

---

### PackageGrid (pkg grid)
```
display: grid
grid-template-columns: repeat(3, 1fr)
gap: 10px
margin-bottom: 28px
```

---

### PackageCard
```
border: 0.5px solid --color-border-tertiary
border-radius: --border-radius-lg
padding: 14px
padding-left: 16px
cursor: pointer
transition: border-color 0.15s
overflow: hidden
```

`padding-left` is `16px` (not `14px`) to clear the source accent bar.
`overflow: hidden` is required so the accent bar's corners are clipped by the card's border-radius.

**Hover:**
```
border-color: --color-border-secondary
```

**Source accent bar:**

A `2px` wide vertical bar pinned to the left edge of the card, full card height. Color is determined by the package source:

| Source | Color |
|---|---|
| pacman | `#185FA5` |
| aur | `#854F0B` |
| flatpak | `#534AB7` |

Each card has exactly one source, so the bar is always a single color. The bar sits inside the card's `overflow: hidden` container — no extra border or wrapper needed.

Internal structure:

**Header row:**
```
display: flex
align-items: center
gap: 10px
margin-bottom: 8px
```
Contains: `AppIcon` (small), a column with `PackageName` and `PackageVersion`.

**PackageName:** `font-size: 13px`, `font-weight: 500`, `color: text-primary`.
**PackageVersion:** `font-size: 11px`, `color: text-tertiary`.

**Description:** see `AppDescription` above.

**Footer row:**
```
display: flex
align-items: center
justify-content: space-between
```
Contains: badge(s) on the left, `InstallButton` on the right.

---

### StatusBar
```
padding: 6px 20px
border-top: 0.5px solid --color-border-tertiary
display: flex
align-items: center
gap: 16px
background: --color-background-secondary
```

**StatusItem:**
```
font-size: 11px
color: --color-text-tertiary
display: flex
align-items: center
gap: 5px
```

Contains a `StatusDot` and a label string.

**StatusDot:**
```
width: 6px
height: 6px
border-radius: 50%
```
Colors: green `#97C459`, amber `#EF9F27`, purple `#7F77DD`.

Package count sits `margin-left: auto`, `font-size: 11px`, `color: text-tertiary`.

---

---

### DetailPage

A routed page that replaces the content area when a `PackageCard` is clicked. Displays full package metadata, action controls, and a detail grid.

```
background: --color-background-primary
padding: 20px
overflow-y: auto
```

Children stacked vertically:

**Back button row:**
```
height: 36px
display: flex
align-items: center
cursor: pointer
```
Contains: `←` arrow text (`13px`, `text-secondary`, hover → `text-primary`), "Back" label (`13px`, `text-secondary`, hover → `text-primary`).
Clicking pops the `StackView`.

**Header row:**
```
display: flex
align-items: flex-start
gap: 16px
margin-bottom: 20px
```
Contains: `AppIcon` (large, 52×52px, source-colored background), name/version/badges column, action controls column (`VersionPicker` + Install/Remove button, `align: vertical-center`).

Name column children:
- Package name: `20px`, weight `500`, `text-primary`
- Version + badges `RowLayout`: version text (`11px`, `text-tertiary`), source `BadgePill`, installed `BadgePill` (visible when installed)

**Action controls row** (`align: vertical-center`, `spacing: 8px`):
- `VersionPicker` (see below)
- Install button: visible when not installed. Primary style (`background: #3B6D11`, `color: #EAF3DE`, `border-radius: 6px`, `padding: 6px 14px`, `font-size: 12px`). Pressed state: `background: #2F560D`.
- Remove button: visible when installed. Ghost style (`background: transparent`, `border: 0.5px solid --color-border-secondary`, `color: text-secondary`, `border-radius: 6px`, `padding: 6px 14px`, `font-size: 12px`). Hover: background fills to `bgSecondary`.

**Divider:**
```
height: 0.5px
background: --color-border-tertiary
margin-bottom: 20px
```

**Description:**
```
font-size: 12px
color: text-secondary
line-height: 1.45
margin-bottom: 24px
```
Falls back from `longDescription` to `description`.

**"Details" heading:**
```
font-size: 15px
font-weight: 500
color: text-primary
margin-bottom: 14px
```

**Detail grid** (`GridLayout`, 2 columns, `columnSpacing: 24px`, `rowSpacing: 10px`):

| Key label | Value |
|---|---|
| "Installed size" | `formatBytes(installedSize)` |
| "Download size" | `formatBytes(downloadSize)` |
| "Source" | `badgeForSource(source)` |
| "Version" | `package.version` |
| "Dependencies" | Comma-separated list (wrappable) |
| "Rating" | `X.X / 5` (only visible when `source === Flatpak && rating > 0`) |

Key labels: `12px`, `text-tertiary`. Values: `12px`, `text-secondary`.

---

### VersionPicker

A styled `ComboBox` for selecting which version of a package to install. Shown only when multiple versions are available or when the package is not yet installed.

```
implicit-width: 150px
implicit-height: 30px
border: 0.5px solid --color-border-secondary
border-radius: 6px
background: --color-background-primary
```

**Content text:** `12px`, `text-secondary`, left padding `10px`.
**Indicator:** Canvas-drawn chevron-down, `12×12px`, `text-tertiary`, `stroke-width: 1.5`.
**Popup:** Positioned `2px` below the picker. Same width as picker. Background: `bgPrimary`, `border-radius: 6px`, `border: 0.5px solid borderSecondary`.
**Delegate:** `ItemDelegate`, height `30px`. Text `12px`, `text-primary`, left padding `10px`. Hover: `bgSecondary`.

---

### ConfirmDialog

A modal overlay centered on the window. Used for both install and remove confirmations.

```
anchors: fill parent
background: #AA000000 (dark scrim)
z: 100
```

**Properties:**
- `pkgName: string`
- `pkgVersion: string`
- `pkgSource: string` (badge variant name: "pacman", "aur", "flatpak")
- `pkgDependencies: list<string>`
- `pkgSize: string`
- `mode: string` — `"install"` or `"remove"`

**Signals:**
- `installConfirmed()`
- `removeConfirmed()`
- `dialogCancelled()`

**Dialog card:**
```
width: 420px
border-radius: --border-radius-lg
background: --color-background-primary
border: 0.5px solid --color-border-secondary
z: 101
```

Card content (padding `24px`, spacing `20px`):

**Header:**
- Title: `"Install " + pkgName` (install mode) or `"Remove " + pkgName` (remove mode). `18px`, weight `500`, `text-primary`.
- Subtitle row: source `BadgePill` + version text (`13px`, `text-tertiary`)

**Divider:** `height: 0.5px`, `border-tertiary`

**Metadata rows** (visible only in install mode):
- Row with `MetaLine` items: "Download" → `pkgSize`, "Size on disk" → `pkgSize`

**Dependencies section** (visible only in install mode and `pkgDependencies.length > 0`):
- Header: `"Dependencies (N)"`, `13px`, weight `500`, `text-secondary`
- Container: `bgSecondary`, `border-radius: 6px`, `border: 0.5px solid borderTertiary`, padded `8px`
- Items: `12px`, `text-secondary`, vertical spacing `4px`

**Actions row** (right-aligned):
- Primary button: label is `"Install"` or `"Remove"` depending on `mode`. Style: `background: #3B6D11`, `color: #EAF3DE`, `border-radius: 6px`, `height: 32px`, `padding: 0 24px`. Pressed: `background: #2F560D`. On click: hides dialog, emits `installConfirmed()` or `removeConfirmed()`.
- Cancel button: `background: bgSecondary`, `border: 0.5px solid borderSecondary`, `color: text-secondary`, `border-radius: 6px`, `height: 32px`. On click: hides dialog, emits `dialogCancelled()`.

**Scrim click:** Clicking the dark scrim background emits `dialogCancelled()`.

**Fade animation:** `Behavior on opacity: NumberAnimation { duration: 120 }`

---

### ProgressDrawer

A panel anchored to the bottom of the main area (above the statusbar). Shows transaction progress during install, remove, or update operations.

```
anchors: bottom of main area, full width
background: --color-background-primary
border: 0.5px solid --color-border-secondary
border-radius: --border-radius-lg (top corners)
z: 50
```

**Properties:**
- `pkgName: string`
- `statusText: string` — step label ("Downloading...", "Verifying...", "Installing...", "Removing...", "Complete")
- `percent: int` — 0 to 100
- `isError: bool`

**Slide animation:**
```
Behavior on height: NumberAnimation { duration: 200; easing: InOutQuad }
height: visible ? contentHeight + 24 : 0
```

**Content** (padded `12px` top, `20px` horizontal, spacing `8px`):

**Top row:** Package name (`14px`, weight `500`, `text-primary`), dash separator (`14px`, `text-tertiary`), status text (`14px`, `text-secondary` or `aur` when `isError`).

**Progress bar track:** Full width, `6px` height, `border-radius: 3px`, `background: bgSecondary`, `border: 0.5px solid borderTertiary`.
**Fill:** Width bound to `percent / 100 * track.width`, animated (`Behavior on width: NumberAnimation { duration: 300; easing: InOutQuad }`). Color: `accent` normally, `aur` when `isError`.

**Percentage text:** `11px`, weight `500`, `text-tertiary`.

---

### RemoveButton

A ghost-style button displayed in the DetailPage header row when a package is installed.

```
background: transparent
border: 0.5px solid --color-border-secondary
color: --color-text-secondary
border-radius: 6px
padding: 6px 14px
font-size: 12px
height: 30px
cursor: pointer
```

**Hover:** background fills to `bgSecondary`, border stays `borderSecondary`.
**Pressed:** no additional visual change (hover state persists).
Label: "Remove".

---

## Layout Rules

1. The sidebar is always 200px wide and never resizes.
2. The content area scrolls vertically. The topbar, sidebar, and statusbar do not scroll.
3. All borders are `0.5px solid`. Never use `1px` borders.
4. Cards never use box-shadow. Borders and background color differences create depth.
5. The active nav item gets a `2px` right border in accent green, not a left border.
6. Section titles always have `28px` of space below the section they head (via `margin-bottom` on the grid or featured block beneath them).
7. The hero featured card always spans both columns. Standard featured cards are one column each.
8. The package grid is always 3 columns. It does not collapse.
9. Emoji or SVG are used for app icons. No raster images in prototyping.
10. Do not add new layout containers not present in this spec without flagging it explicitly.

---

## Interaction Patterns

**Card hover:** border color transitions from `--color-border-tertiary` to `--color-border-secondary` in `0.15s`. No scale, no shadow, no lift.

**Nav item hover:** background fills to `--color-background-primary` in `0.1s`. No underline, no border change.

**Search:** debounced at 250ms. Results replace the current card grid content in place. Source filter tabs narrow the active backends.

**Install button:** clicking a ghost Install button should open a detail view (slide-in panel or routed page), not immediately install. Confirmation always precedes any system action.

**Detail sheet:** slides in from the right over the content area, or routes to a full detail page replacing the content area. Not a modal overlay.

**Progress drawer:** appears at the bottom of the main area (above the statusbar) during active transactions. Shows step label and a progress indicator. Collapses when the transaction completes.

**Updates banner:** only visible when updates exist. "Update all" triggers the full update flow with a confirmation step before execution.

---

## Reference Mockup

The full HTML source of the original mockup follows. Treat it as ground truth for anything not explicitly covered above.

```html
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  .sc-root { display: flex; height: 620px; background: var(--color-background-primary); border: 0.5px solid var(--color-border-secondary); border-radius: var(--border-radius-lg); overflow: hidden; font-size: 13px; }
  .sc-sidebar { width: 200px; min-width: 200px; background: var(--color-background-secondary); border-right: 0.5px solid var(--color-border-tertiary); display: flex; flex-direction: column; padding: 12px 0; }
  .sc-logo { padding: 10px 16px 18px; display: flex; align-items: center; gap: 8px; }
  .sc-logo-icon { width: 28px; height: 28px; background: #3B6D11; border-radius: 7px; display: flex; align-items: center; justify-content: center; }
  .sc-logo-icon svg { width: 16px; height: 16px; fill: #C0DD97; }
  .sc-logo-text { font-size: 14px; font-weight: 500; color: var(--color-text-primary); }
  .sc-nav-group { margin-bottom: 18px; }
  .sc-nav-label { padding: 0 16px 6px; font-size: 10px; font-weight: 500; letter-spacing: 0.08em; color: var(--color-text-tertiary); text-transform: uppercase; }
  .sc-nav-item { display: flex; align-items: center; gap: 10px; padding: 7px 16px; cursor: pointer; color: var(--color-text-secondary); border-radius: 0; transition: background 0.1s; }
  .sc-nav-item:hover { background: var(--color-background-primary); color: var(--color-text-primary); }
  .sc-nav-item.active { background: var(--color-background-primary); color: #3B6D11; font-weight: 500; border-right: 2px solid #3B6D11; }
  .sc-nav-icon { width: 16px; height: 16px; opacity: 0.7; flex-shrink: 0; }
  .sc-nav-item.active .sc-nav-icon { opacity: 1; }
  .sc-main { flex: 1; display: flex; flex-direction: column; overflow: hidden; }
  .sc-topbar { padding: 12px 20px; border-bottom: 0.5px solid var(--color-border-tertiary); display: flex; align-items: center; gap: 12px; background: var(--color-background-primary); }
  .sc-search { flex: 1; position: relative; }
  .sc-search input { width: 100%; height: 32px; border: 0.5px solid var(--color-border-secondary); border-radius: var(--border-radius-md); background: var(--color-background-secondary); padding: 0 12px 0 32px; font-size: 13px; color: var(--color-text-primary); outline: none; }
  .sc-search-icon { position: absolute; left: 10px; top: 50%; transform: translateY(-50%); width: 14px; height: 14px; color: var(--color-text-tertiary); }
  .sc-source-tabs { display: flex; gap: 4px; }
  .sc-tab { padding: 5px 12px; border-radius: var(--border-radius-md); font-size: 12px; border: 0.5px solid var(--color-border-secondary); background: transparent; cursor: pointer; color: var(--color-text-secondary); }
  .sc-tab.active { background: #EAF3DE; color: #3B6D11; border-color: #97C459; font-weight: 500; }
  .sc-content { flex: 1; overflow-y: auto; padding: 20px; }
  .sc-section-title { font-size: 15px; font-weight: 500; color: var(--color-text-primary); margin-bottom: 14px; }
  .sc-featured { display: grid; grid-template-columns: repeat(2, 1fr); gap: 12px; margin-bottom: 28px; }
  .sc-featured-card { border: 0.5px solid var(--color-border-tertiary); border-radius: var(--border-radius-lg); padding: 16px; background: var(--color-background-primary); cursor: pointer; transition: border-color 0.15s; }
  .sc-featured-card:hover { border-color: var(--color-border-secondary); }
  .sc-featured-card.hero { grid-column: span 2; display: flex; gap: 16px; align-items: flex-start; }
  .sc-app-icon { width: 52px; height: 52px; border-radius: 12px; flex-shrink: 0; display: flex; align-items: center; justify-content: center; font-size: 22px; }
  .sc-app-icon.sm { width: 40px; height: 40px; border-radius: 10px; font-size: 18px; }
  .sc-hero-meta { flex: 1; }
  .sc-app-name { font-size: 14px; font-weight: 500; color: var(--color-text-primary); margin-bottom: 3px; }
  .sc-app-desc { font-size: 12px; color: var(--color-text-secondary); line-height: 1.5; margin-bottom: 10px; }
  .sc-badges { display: flex; gap: 6px; align-items: center; flex-wrap: wrap; }
  .sc-badge { padding: 2px 8px; border-radius: 10px; font-size: 10px; font-weight: 500; }
  .badge-pacman { background: #E6F1FB; color: #185FA5; }
  .badge-aur { background: #FAEEDA; color: #854F0B; }
  .badge-flatpak { background: #EEEDFE; color: #534AB7; }
  .badge-installed { background: #EAF3DE; color: #3B6D11; }
  .sc-install-btn { margin-left: auto; padding: 6px 14px; border: 0.5px solid var(--color-border-secondary); border-radius: var(--border-radius-md); font-size: 12px; background: transparent; color: var(--color-text-secondary); cursor: pointer; }
  .sc-install-btn.primary { background: #3B6D11; border-color: #3B6D11; color: #EAF3DE; }
  .sc-pkg-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin-bottom: 28px; }
  .sc-pkg-card { border: 0.5px solid var(--color-border-tertiary); border-radius: var(--border-radius-lg); padding: 14px; cursor: pointer; transition: border-color 0.15s; }
  .sc-pkg-card:hover { border-color: var(--color-border-secondary); }
  .sc-pkg-header { display: flex; align-items: center; gap: 10px; margin-bottom: 8px; }
  .sc-pkg-name { font-size: 13px; font-weight: 500; color: var(--color-text-primary); }
  .sc-pkg-version { font-size: 11px; color: var(--color-text-tertiary); }
  .sc-pkg-desc { font-size: 12px; color: var(--color-text-secondary); line-height: 1.45; margin-bottom: 10px; }
  .sc-pkg-footer { display: flex; align-items: center; justify-content: space-between; }
  .sc-rating { font-size: 11px; color: var(--color-text-tertiary); }
  .sc-updates-bar { background: #FAEEDA; border: 0.5px solid #FAC775; border-radius: var(--border-radius-md); padding: 10px 14px; display: flex; align-items: center; gap: 12px; margin-bottom: 20px; }
  .sc-updates-text { flex: 1; font-size: 12px; color: #633806; }
  .sc-updates-btn { padding: 5px 12px; background: #854F0B; color: #FAEEDA; border: none; border-radius: var(--border-radius-md); font-size: 11px; cursor: pointer; }
  .sc-statusbar { padding: 6px 20px; border-top: 0.5px solid var(--color-border-tertiary); display: flex; align-items: center; gap: 16px; background: var(--color-background-secondary); }
  .sc-status-item { font-size: 11px; color: var(--color-text-tertiary); display: flex; align-items: center; gap: 5px; }
  .sc-dot { width: 6px; height: 6px; border-radius: 50%; background: #97C459; }
  .sc-dot.amber { background: #EF9F27; }
  .sc-dot.purple { background: #7F77DD; }
</style>

<div class="sc-root">
  <div class="sc-sidebar">
    <div class="sc-logo">
      <div class="sc-logo-icon">
        <svg viewBox="0 0 16 16"><path d="M8 1L1 14h14L8 1zm0 3l4.5 8h-9L8 4z"/></svg>
      </div>
      <span class="sc-logo-text">Lambda</span>
    </div>
    <div class="sc-nav-group">
      <div class="sc-nav-label">Discover</div>
      <div class="sc-nav-item active">
        <svg class="sc-nav-icon" viewBox="0 0 16 16" fill="currentColor"><path d="M2 3h12v2H2zm0 4h12v2H2zm0 4h8v2H2z"/></svg>
        Browse
      </div>
      <div class="sc-nav-item">
        <svg class="sc-nav-icon" viewBox="0 0 16 16" fill="currentColor"><path d="M11 1a4 4 0 010 8 4 4 0 010-8zm0 1.5a2.5 2.5 0 100 5 2.5 2.5 0 000-5zM1 13.5C1 11 4 9.5 8 9.5s7 1.5 7 4v1H1v-1z"/></svg>
        Featured
      </div>
      <div class="sc-nav-item">
        <svg class="sc-nav-icon" viewBox="0 0 16 16" fill="currentColor"><circle cx="8" cy="8" r="6" fill="none" stroke="currentColor" stroke-width="1.5"/><path d="M8 5v4l2.5 1.5"/></svg>
        Recent
      </div>
    </div>
    <div class="sc-nav-group">
      <div class="sc-nav-label">Library</div>
      <div class="sc-nav-item">
        <svg class="sc-nav-icon" viewBox="0 0 16 16" fill="currentColor"><path d="M3 2h10a1 1 0 011 1v10a1 1 0 01-1 1H3a1 1 0 01-1-1V3a1 1 0 011-1zm2 4l-1 1 3 3 5-5-1-1-4 4-2-2z"/></svg>
        Installed
      </div>
      <div class="sc-nav-item" style="position:relative;">
        <svg class="sc-nav-icon" viewBox="0 0 16 16" fill="currentColor"><path d="M8 1v9m-4-4l4 4 4-4M3 13h10"/></svg>
        Updates
        <span style="margin-left:auto;background:#854F0B;color:#FAEEDA;border-radius:10px;padding:1px 7px;font-size:10px;">3</span>
      </div>
    </div>
    <div class="sc-nav-group">
      <div class="sc-nav-label">Sources</div>
      <div class="sc-nav-item">
        <span style="width:8px;height:8px;border-radius:50%;background:#185FA5;flex-shrink:0;"></span>
        Pacman
      </div>
      <div class="sc-nav-item">
        <span style="width:8px;height:8px;border-radius:50%;background:#EF9F27;flex-shrink:0;"></span>
        AUR
      </div>
      <div class="sc-nav-item">
        <span style="width:8px;height:8px;border-radius:50%;background:#7F77DD;flex-shrink:0;"></span>
        Flatpak
      </div>
    </div>
  </div>

  <div class="sc-main">
    <div class="sc-topbar">
      <div class="sc-search">
        <svg class="sc-search-icon" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="6.5" cy="6.5" r="4"/><path d="M11 11l3 3"/></svg>
        <input type="text" placeholder="Search packages..." />
      </div>
      <div class="sc-source-tabs">
        <div class="sc-tab active">All</div>
        <div class="sc-tab">Pacman</div>
        <div class="sc-tab">AUR</div>
        <div class="sc-tab">Flatpak</div>
      </div>
    </div>

    <div class="sc-content">
      <div class="sc-updates-bar">
        <svg width="14" height="14" viewBox="0 0 16 16" fill="#854F0B"><path d="M8 1v9m-4-4l4 4 4-4M3 13h10"/></svg>
        <span class="sc-updates-text">3 updates available — Firefox 126, mesa 24.1, yay 12.3</span>
        <button class="sc-updates-btn">Update all</button>
      </div>

      <div class="sc-section-title">Featured apps</div>
      <div class="sc-featured">
        <div class="sc-featured-card hero">
          <div class="sc-app-icon" style="background:#E6F1FB;">🦊</div>
          <div class="sc-hero-meta">
            <div class="sc-app-name">Firefox</div>
            <div class="sc-app-desc">Fast, private and secure browser from Mozilla. Sync bookmarks, history and open tabs across all your devices.</div>
            <div class="sc-badges">
              <span class="sc-badge badge-pacman">pacman</span>
              <span class="sc-badge badge-flatpak">flatpak</span>
              <span style="font-size:11px;color:var(--color-text-tertiary);">126.0.1 · 82 MB</span>
            </div>
          </div>
          <button class="sc-install-btn badge-installed" style="border-radius:var(--border-radius-md);padding:6px 14px;font-size:12px;font-weight:500;border:0.5px solid #97C459;background:#EAF3DE;color:#3B6D11;cursor:pointer;">Installed</button>
        </div>
        <div class="sc-featured-card">
          <div style="display:flex;gap:12px;align-items:flex-start;">
            <div class="sc-app-icon sm" style="background:#EEEDFE;">🎮</div>
            <div style="flex:1;">
              <div class="sc-app-name">Heroic Launcher</div>
              <div class="sc-app-desc" style="margin-bottom:8px;">Epic & GOG games on Linux</div>
              <div class="sc-badges">
                <span class="sc-badge badge-aur">AUR</span>
                <span class="sc-badge badge-flatpak">flatpak</span>
              </div>
            </div>
          </div>
        </div>
        <div class="sc-featured-card">
          <div style="display:flex;gap:12px;align-items:flex-start;">
            <div class="sc-app-icon sm" style="background:#E1F5EE;">🎵</div>
            <div style="flex:1;">
              <div class="sc-app-name">Spotify</div>
              <div class="sc-app-desc" style="margin-bottom:8px;">Music streaming client</div>
              <div class="sc-badges">
                <span class="sc-badge badge-aur">AUR</span>
                <span class="sc-badge badge-flatpak">flatpak</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="sc-section-title">Development tools</div>
      <div class="sc-pkg-grid">
        <div class="sc-pkg-card">
          <div class="sc-pkg-header">
            <div class="sc-app-icon sm" style="background:#E6F1FB;font-size:16px;">💻</div>
            <div>
              <div class="sc-pkg-name">VS Code</div>
              <div class="sc-pkg-version">1.89.0</div>
            </div>
          </div>
          <div class="sc-pkg-desc">Code editor by Microsoft. Extensions, debugger, Git integration.</div>
          <div class="sc-pkg-footer">
            <div style="display:flex;gap:5px;">
              <span class="sc-badge badge-aur">AUR</span>
              <span class="sc-badge badge-flatpak">flatpak</span>
            </div>
            <button class="sc-install-btn primary" style="padding:4px 10px;font-size:11px;border:none;border-radius:var(--border-radius-md);cursor:pointer;background:#3B6D11;color:#EAF3DE;">Install</button>
          </div>
        </div>
        <div class="sc-pkg-card">
          <div class="sc-pkg-header">
            <div class="sc-app-icon sm" style="background:#E1F5EE;font-size:16px;">🐍</div>
            <div>
              <div class="sc-pkg-name">Python</div>
              <div class="sc-pkg-version">3.12.3</div>
            </div>
          </div>
          <div class="sc-pkg-desc">High-level interpreted language. Standard library included.</div>
          <div class="sc-pkg-footer">
            <span class="sc-badge badge-pacman">pacman</span>
            <button class="sc-install-btn" style="padding:4px 10px;font-size:11px;border:0.5px solid var(--color-border-secondary);border-radius:var(--border-radius-md);cursor:pointer;">Install</button>
          </div>
        </div>
        <div class="sc-pkg-card">
          <div class="sc-pkg-header">
            <div class="sc-app-icon sm" style="background:#FAEEDA;font-size:16px;">🦀</div>
            <div>
              <div class="sc-pkg-name">Rust</div>
              <div class="sc-pkg-version">1.78.0</div>
            </div>
          </div>
          <div class="sc-pkg-desc">Systems language focused on safety, speed and concurrency.</div>
          <div class="sc-pkg-footer">
            <span class="sc-badge badge-pacman">pacman</span>
            <button class="sc-install-btn" style="padding:4px 10px;font-size:11px;border:0.5px solid var(--color-border-secondary);border-radius:var(--border-radius-md);cursor:pointer;">Install</button>
          </div>
        </div>
      </div>
    </div>

    <div class="sc-statusbar">
      <div class="sc-status-item"><div class="sc-dot"></div>Pacman ready</div>
      <div class="sc-status-item"><div class="sc-dot amber"></div>AUR helper: yay</div>
      <div class="sc-status-item"><div class="sc-dot purple"></div>Flatpak ready</div>
      <div style="margin-left:auto;font-size:11px;color:var(--color-text-tertiary);">14,382 packages available</div>
    </div>
  </div>
</div>
```
