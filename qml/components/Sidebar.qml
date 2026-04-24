import QtQuick
import LambdaSoftwareCenter

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
        Item {
            id: logoRow
            width: parent.width
            height: 58

            Rectangle {
                id: logoIcon
                x: 16
                y: 10
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
                x: logoIcon.x + logoIcon.width + 8
                anchors.verticalCenter: logoIcon.verticalCenter
                text: "Lambda"
                font.pixelSize: 14
                font.weight: Font.Medium
                color: Theme.textPrimary
            }
        }

        // Discover group
        NavGroup {
            width: parent.width
            label: "Discover"

            NavItem {
                width: parent.width
                text: "Browse"
                active: sidebar.currentPage === "browse"
                onClicked: sidebar.requestPage("browse")
            }

            NavItem {
                width: parent.width
                text: "Featured"
                active: sidebar.currentPage === "featured"
                onClicked: sidebar.requestPage("featured")
            }

            NavItem {
                width: parent.width
                text: "Recent"
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
                active: sidebar.currentPage === "installed"
                onClicked: sidebar.requestPage("installed")
            }

            NavItem {
                width: parent.width
                text: "Updates"
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
}
