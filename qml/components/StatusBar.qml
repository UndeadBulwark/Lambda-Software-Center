import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: statusBar
    height: 28
    color: Theme.bgSecondary

    // Top border
    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: Theme.borderTertiary
    }

    Row {
        x: 20
        width: parent.width - 40
        height: parent.height
        spacing: 16

        Row {
            height: parent.height
            spacing: 5

            Rectangle {
                width: 6
                height: 6
                anchors.verticalCenter: parent.verticalCenter
                radius: 3
                color: Theme.dotGreen
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Pacman ready"
                font.pixelSize: 11
                color: Theme.textTertiary
            }
        }

        Row {
            height: parent.height
            spacing: 5

            Rectangle {
                width: 6
                height: 6
                anchors.verticalCenter: parent.verticalCenter
                radius: 3
                color: Theme.dotAmber
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "AUR helper"
                font.pixelSize: 11
                color: Theme.textTertiary
            }
        }

        Row {
            height: parent.height
            spacing: 5

            Rectangle {
                width: 6
                height: 6
                anchors.verticalCenter: parent.verticalCenter
                radius: 3
                color: Theme.dotPurple
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "Flatpak ready"
                font.pixelSize: 11
                color: Theme.textTertiary
            }
        }

        Item { width: parent.width * 0.5; height: 1 }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "Ready"
            font.pixelSize: 11
            color: Theme.textTertiary
        }
    }
}
