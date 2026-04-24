import QtQuick

Rectangle {
    id: installButton
    height: 28
    width: btnText.implicitWidth + 20
    radius: Theme.radiusMd
    color: "transparent"

    property bool installed: false
    property bool primary: false
    signal clicked()

    Text {
        id: btnText
        anchors.centerIn: parent
        text: installed ? "Installed" : "Install"
        font.pixelSize: 12
        font.weight: installed ? Font.Medium : Font.Normal
        color: installed ? Theme.accent : Theme.textSecondary
    }

    // Border for ghost/installed
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: installed ? Theme.accentLight : Theme.borderSecondary
        border.width: 1
        visible: !primary
    }

    // Filled background for primary
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: Theme.accent
        visible: primary && !installed
    }

    MouseArea {
        anchors.fill: parent
        onClicked: installButton.clicked()
    }
}
