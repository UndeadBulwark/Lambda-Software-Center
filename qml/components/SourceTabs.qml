import QtQuick

Row {
    id: sourceTabs
    spacing: 4

    signal filterChanged(int filter)
    property int activeFilter: -1

    Repeater {
        model: [
            { label: "All",     value: -1 },
            { label: "Pacman",  value: 0 },
            { label: "AUR",     value: 1 },
            { label: "Flatpak", value: 2 }
        ]

        delegate: Rectangle {
            id: tabBg
            width: tabText.implicitWidth + 24
            height: 28
            radius: Theme.radiusMd
            color: sourceTabs.activeFilter === modelData.value ? Theme.accentSurface : "transparent"

            Text {
                id: tabText
                anchors.centerIn: parent
                text: modelData.label
                font.pixelSize: 12
                font.weight: sourceTabs.activeFilter === modelData.value ? Font.Medium : Font.Normal
                color: sourceTabs.activeFilter === modelData.value ? Theme.accent : Theme.textSecondary
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    sourceTabs.activeFilter = modelData.value;
                    sourceTabs.filterChanged(modelData.value);
                }
            }

            // Border for all tabs; only visible on inactive (active tab has bg fill)
            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: "transparent"
                border.color: sourceTabs.activeFilter === modelData.value ? Theme.accentLight : Theme.borderSecondary
                border.width: 1
                visible: true
            }
        }
    }
}
