pragma Singleton
import QtQuick

QtObject {
    property bool isDark: false

    // Accent stays the same in light and dark — functional brand color
    readonly property color accent:           isDark ? "#3B6D11" : "#3B6D11"
    readonly property color accentLight:      isDark ? "#97C459" : "#97C459"
    readonly property color accentDark:       isDark ? "#2A4A0D" : "#2F560D"
    readonly property color accentSurface:    isDark ? "#2A3A1C" : "#EAF3DE"

    // AUR
    readonly property color aur:              isDark ? "#854F0B" : "#854F0B"
    readonly property color aurSurface:       isDark ? "#3A2A1A" : "#FAEEDA"
    readonly property color aurBorder:        isDark ? "#FAC775" : "#FAC775"
    readonly property color aurText:          isDark ? "#D4A050" : "#633806"

    // Flatpak
    readonly property color flatpak:          isDark ? "#534AB7" : "#534AB7"
    readonly property color flatpakSurface:     isDark ? "#2A1A3A" : "#EEEDFE"
    readonly property color flatpakDot:       isDark ? "#7F77DD" : "#7F77DD"

    // Pacman
    readonly property color pacman:           isDark ? "#185FA5" : "#185FA5"
    readonly property color pacmanSurface:     isDark ? "#1A2A3A" : "#E6F1FB"

    // Neutrals
    readonly property color bgPrimary:        isDark ? "#1C1C1E" : "#FFFFFF"
    readonly property color bgSecondary:      isDark ? "#242426" : "#F5F5F5"
    readonly property color borderSecondary:  isDark ? "#4A4A4C" : "#DEDEDE"
    readonly property color borderTertiary:   isDark ? "#3A3A3C" : "#EBEBEB"
    readonly property color textPrimary:      isDark ? "#F0F0F0" : "#1A1A1A"
    readonly property color textSecondary:    isDark ? "#B0B0B0" : "#555555"
    readonly property color textTertiary:     isDark ? "#888888" : "#999999"

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
