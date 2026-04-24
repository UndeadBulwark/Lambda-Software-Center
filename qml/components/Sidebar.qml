import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: Theme.sidebarWidth
    height: parent.height
    color: Theme.bgSecondary

    signal requestPage(string page)
    property string currentPage: "browse"

    Column {
        id: sidebarCol
        anchors.fill: parent
        spacing: 0

        // Logo
        Row {
            id: logoRow
            width: parent.width
            height: 58
            padding: 0
            leftPadding: 16
            topPadding: 16
            rightPadding: 16
            bottomPadding: 12
            spacing: 8

            Rectangle {
                id: logoIcon
                width: 28
                height: 28
                radius: 7
                color: Theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "Δ"
                    color: "#C0DD97"
                    font.pixelSize: 16
                    font.weight: Font.Bold
                }
            }

            Text {
                id: logoText
                text: "Lambda"
                font.pixelSize: 14
                font.weight: Font.Medium
                color: Theme.textPrimary
                anchors.verticalCenter: logoIcon.verticalCenter
            }
        }

        // Discover group
        NavGroup {
            width: parent.width
            label: "Discover"

            NavItem {
                width: parent.width
                text: "Browse"
                icon: navIconList
                active: sidebar.currentPage === "browse"
                onClicked: sidebar.requestPage("browse")
            }

            NavItem {
                width: parent.width
                text: "Featured"
                icon: navIconStar
                active: sidebar.currentPage === "featured"
                onClicked: sidebar.requestPage("featured")
            }

            NavItem {
                width: parent.width
                text: "Recent"
                icon: navIconClock
                active: sidebar.currentPage === "recent"
                onClicked: sidebar.requestPage("recent")
            }
        }

        // Library group
        NavGroup {
            width: parent.width
            label: "Library"

            NavItem {
                width: parent.width
                text: "Installed"
                icon: navIconCheck
                active: sidebar.currentPage === "installed"
                onClicked: sidebar.requestPage("installed")
            }

            NavItem {
                width: parent.width
                text: "Updates"
                icon: navIconDownload
                active: sidebar.currentPage === "updates"
                count: 0
                onClicked: sidebar.requestPage("updates")
            }
        }

        // Sources group
        NavGroup {
            width: parent.width
            label: "Sources"

            NavItem {
                width: parent.width
                text: "Pacman"
                sourceDot: Theme.pacman
            }

            NavItem {
                width: parent.width
                text: "AUR"
                sourceDot: Theme.aur
            }

            NavItem {
                width: parent.width
                text: "Flatpak"
                sourceDot: Theme.flatpakDot
            }
        }
    }

    // Inline icon components (used via id reference)
    Component { id: navIconList;    Rectangle { width: 16; height: 16; color: "transparent" } }
    Component { id: navIconStar;    Rectangle { width: 16; height: 16; color: "transparent" } }
    Component { id: navIconClock;   Rectangle { width: 16; height: 16; color: "transparent" } }
    Component { id: navIconCheck;   Rectangle { width: 16; height: 16; color: "transparent" } }
    Component { id: navIconDownload; Rectangle { width: 16; height: 16; color: "transparent" } }
}
