import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: confirmDialog
    anchors.fill: parent
    visible: false
    opacity: visible ? 1 : 0
    color: "#AA000000"
    z: 100

    // Data properties
    property string pkgId: ""
    property string pkgName: ""
    property string pkgVersion: ""
    property string pkgSource: ""
    property var pkgDependencies: []
    property string pkgSize: ""

    signal installConfirmed()
    signal dialogCancelled()

    // Scrim background
    MouseArea {
        anchors.fill: parent
        onClicked: confirmDialog.dialogCancelled()
    }

    // Fade animation
    Behavior on opacity {
        NumberAnimation { duration: 120 }
    }

    // Dialog card
    Rectangle {
        width: 420
        height: dialogContent.height + 48
        radius: Theme.radiusLg
        color: Theme.bgPrimary
        anchors.centerIn: parent
        z: 101

        border.color: Theme.borderSecondary
        border.width: 1

        Column {
            id: dialogContent
            width: parent.width - 48
            anchors.centerIn: parent
            spacing: 20

            // Header
            Column {
                width: parent.width
                spacing: 6

                Text {
                    text: "Install " + confirmDialog.pkgName
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: Theme.textPrimary
                }

                Row {
                    spacing: 8
                    BadgePill {
                        variant: confirmDialog.pkgSource
                        label: confirmDialog.pkgSource
                        visible: confirmDialog.pkgSource !== ""
                    }
                    Text {
                        text: confirmDialog.pkgVersion
                        font.pixelSize: 13
                        color: Theme.textTertiary
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Divider
            Rectangle {
                width: parent.width
                height: 1
                color: Theme.borderTertiary
            }

            // Size delta
            Row {
                spacing: 16

                MetaLine { label: "Download"; value: confirmDialog.pkgSize }
                MetaLine { label: "Size on disk"; value: confirmDialog.pkgSize }
            }

            // Dependencies
            Column {
                width: parent.width
                spacing: 8
                visible: confirmDialog.pkgDependencies.length > 0

                Text {
                    text: "Dependencies (" + confirmDialog.pkgDependencies.length + ")"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: Theme.textSecondary
                }

                Rectangle {
                    width: parent.width
                    height: depCol.height + 16
                    radius: Theme.radiusMd
                    color: Theme.bgSecondary
                    border.color: Theme.borderTertiary
                    border.width: 1
                    clip: true

                    Column {
                        id: depCol
                        width: parent.width - 24
                        anchors.centerIn: parent
                        spacing: 4

                        Repeater {
                            model: confirmDialog.pkgDependencies
                            Text {
                                text: modelData
                                font.pixelSize: 12
                                color: Theme.textSecondary
                            }
                        }
                    }
                }
            }

            // Actions
            Row {
                width: parent.width
                spacing: 12
                layoutDirection: Qt.RightToLeft

                ActionButton {
                    label: "Install"
                    primary: true
                    onClicked: confirmDialog.installConfirmed()
                }

                ActionButton {
                    label: "Cancel"
                    onClicked: confirmDialog.dialogCancelled()
                }
            }
        }
    }

    component MetaLine: Column {
        spacing: 2
        property string label
        property string value

        Text {
            text: label
            font.pixelSize: 11
            color: Theme.textTertiary
        }
        Text {
            text: value
            font.pixelSize: 13
            font.weight: Font.Medium
            color: Theme.textPrimary
        }
    }

    component ActionButton: Rectangle {
        width: btnText.implicitWidth + 24
        height: 32
        radius: Theme.radiusMd
        color: primary ? Theme.accent : Theme.bgSecondary
        border.color: primary ? Theme.accent : Theme.borderSecondary
        border.width: 1

        property string label
        property bool primary: false
        signal clicked()

        Text {
            id: btnText
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 13
            font.weight: Font.Medium
            color: primary ? "#FFFFFF" : Theme.textSecondary
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.clicked()
        }
    }
}
