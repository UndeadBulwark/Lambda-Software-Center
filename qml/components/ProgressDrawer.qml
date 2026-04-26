import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: progressDrawer
    anchors.bottom: parent.bottom
    width: parent.width
    height: visible ? drawerContent.height + 24 : 0
    color: Theme.bgPrimary
    border.color: Theme.borderSecondary
    border.width: 1
    visible: false
    clip: true
    z: 50

    property string pkgName: ""
    property string statusText: ""
    property int percent: 0
    property bool isError: false

    Behavior on height {
        NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
    }

    Column {
        id: drawerContent
        width: parent.width - 40
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 12
        spacing: 8

        // ── Top row: name + status ──
        Row {
            width: parent.width
            spacing: 12

            Text {
                text: progressDrawer.pkgName
                font.pixelSize: 14
                font.weight: Font.Medium
                color: Theme.textPrimary
                visible: progressDrawer.pkgName !== ""
            }

            Text {
                text: "\u2014"
                font.pixelSize: 14
                color: Theme.textTertiary
                visible: progressDrawer.pkgName !== ""
            }

            Text {
                text: progressDrawer.statusText
                font.pixelSize: 14
                color: progressDrawer.isError ? Theme.aur : Theme.textSecondary
            }
        }

        // ── Progress bar track ──
        Rectangle {
            id: progressTrack
            width: parent.width
            height: 6
            radius: 3
            color: Theme.bgSecondary
            border.color: Theme.borderTertiary
            border.width: 1

            // Filled portion
            Rectangle {
                height: parent.height
                width: Math.max(0, Math.min(parent.width,
                    parent.width * (progressDrawer.percent / 100)))
                radius: parent.radius
                color: progressDrawer.isError ? Theme.aur : Theme.accent

                Behavior on width {
                    NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
                }
            }
        }

        // ── Percentage ──
        Text {
            text: progressDrawer.percent + "%"
            font.pixelSize: 11
            font.weight: Font.Medium
            color: Theme.textTertiary
        }
    }
}
