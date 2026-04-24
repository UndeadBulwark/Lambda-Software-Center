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
    readonly property color pacmanSurface:  "#E6F1FB"

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
