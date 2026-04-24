import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: badgePill
    height: 18
    width: pillText.implicitWidth + 16
    radius: 10

    property string variant: "pacman"
    property string label: "pacman"

    color: {
        switch (variant) {
        case "pacman":   return Theme.pacmanSurface;
        case "aur":      return Theme.aurSurface;
        case "flatpak":  return Theme.flatpakSurface;
        case "installed": return Theme.accentSurface;
        default:         return Theme.bgSecondary;
        }
    }

    Text {
        id: pillText
        anchors.centerIn: parent
        text: badgePill.label
        font.pixelSize: 10
        font.weight: Font.Medium
        color: {
            switch (badgePill.variant) {
            case "pacman":   return Theme.pacman;
            case "aur":      return Theme.aur;
            case "flatpak":  return Theme.flatpak;
            case "installed": return Theme.accent;
            default:         return Theme.textSecondary;
            }
        }
    }
}
