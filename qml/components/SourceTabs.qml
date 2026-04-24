import QtQuick
import LambdaSoftwareCenter

Row {
    id: sourceTabs
    spacing: 4

    signal indexClicked(int index)
    property int activeIndex: 0

    Repeater {
        model: [
            { label: "All",     filter: -1 },
            { label: "Pacman",  filter: 0 },
            { label: "AUR",     filter: 1 },
            { label: "Flatpak", filter: 2 }
        ]

        delegate: Rectangle {
            id: tabBg
            width: tabText.implicitWidth + 24
            height: 28
            radius: Theme.radiusMd
            color: sourceTabs.activeIndex === index ? Theme.accentSurface : "transparent"

            Text {
                id: tabText
                anchors.centerIn: parent
                text: modelData.label
                font.pixelSize: 12
                font.weight: sourceTabs.activeIndex === index ? Font.Medium : Font.Normal
                color: sourceTabs.activeIndex === index ? Theme.accent : Theme.textSecondary
            }

            MouseArea {
                anchors.fill: parent
                onClicked: sourceTabs.indexClicked(index)
            }

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: "transparent"
                border.color: sourceTabs.activeIndex === index ? Theme.accentLight : Theme.borderSecondary
                border.width: 1
            }
        }
    }
}
