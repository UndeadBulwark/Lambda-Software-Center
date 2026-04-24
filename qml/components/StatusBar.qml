import QtQuick

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
        anchors.fill: parent
        leftPadding: 20
        rightPadding: 20
        spacing: 16

        StatusItem { label: "Pacman ready";     dotColor: Theme.dotGreen }
        StatusItem { label: "AUR helper";       dotColor: Theme.dotAmber }
        StatusItem { label: "Flatpak ready";    dotColor: Theme.dotPurple }

        Item { width: 1; height: 1 }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "Ready"
            font.pixelSize: 11
            color: Theme.textTertiary
        }
    }

    component StatusItem: Row {
        spacing: 5
        anchors.verticalCenter: parent.verticalCenter
        property string label
        property color dotColor

        Rectangle {
            width: 6
            height: 6
            radius: 3
            color: dotColor
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: label
            font.pixelSize: 11
            color: Theme.textTertiary
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
