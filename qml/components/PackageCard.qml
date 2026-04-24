import QtQuick

Rectangle {
    id: packageCard
    radius: Theme.radiusLg
    color: Theme.bgPrimary

    // Border workaround for 0.5px border visual
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: cardMouse.containsMouse ? Theme.borderSecondary : Theme.borderTertiary
        border.width: 1
    }

    MouseArea {
        id: cardMouse
        anchors.fill: parent
        hoverEnabled: true
    }

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        // Header row
        Row {
            width: parent.width
            spacing: 10

            Rectangle {
                id: appIcon
                width: 40
                height: 40
                radius: 10
                color: iconColorForSource(model.source)

                Text {
                    anchors.centerIn: parent
                    text: initialForName(model.name)
                    font.pixelSize: 18
                    color: Theme.textSecondary
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                Text {
                    text: model.name || ""
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: Theme.textPrimary
                }
                Text {
                    text: model.version || ""
                    font.pixelSize: 11
                    color: Theme.textTertiary
                }
            }
        }

        // Description
        Text {
            width: parent.width
            text: model.description || ""
            font.pixelSize: 12
            color: Theme.textSecondary
            lineHeightMode: Text.FixedHeight
            lineHeight: 12 * 1.45
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        // Footer row
        Row {
            width: parent.width
            spacing: 6

            BadgePill {
                variant: badgeForSource(model.source)
                label: badgeForSource(model.source)
                visible: model.source !== undefined
            }

            Item { width: parent.width - 60; height: 1 }

            BadgePill {
                variant: "installed"
                label: "Installed"
                visible: model.state === 1
            }
        }
    }

    function initialForName(name) {
        if (!name) return "?";
        return name.substring(0, 1).toUpperCase();
    }

    function iconColorForSource(source) {
        switch (source) {
        case 0: return Theme.pacmanSurface;
        case 1: return Theme.aurSurface;
        case 2: return Theme.flatpakSurface;
        default: return Theme.bgSecondary;
        }
    }

    function badgeForSource(source) {
        switch (source) {
        case 0: return "pacman";
        case 1: return "aur";
        case 2: return "flatpak";
        default: return "";
        }
    }
}
