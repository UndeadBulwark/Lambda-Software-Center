import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: confirmDialog
    anchors.fill: parent
    visible: false
    opacity: visible ? 1 : 0
    color: "#AA000000"
    z: 100

    property string pkgId: ""
    property string pkgName: ""
    property string pkgVersion: ""
    property string pkgSource: ""
    property int pkgSourceInt: 0
    property var pkgDependencies: []
    property string pkgSize: ""
    property string pkgInstalledSize: ""
    property string mode: "install"

    signal installConfirmed()
    signal removeConfirmed()
    signal dialogCancelled()

    MouseArea {
        anchors.fill: parent
        onClicked: confirmDialog.dialogCancelled()
    }

    Behavior on opacity {
        NumberAnimation { duration: 120 }
    }

    Rectangle {
        width: 420
        height: Math.min(dialogContent.height + 48, parent.height - 40)
        radius: Theme.radiusLg
        color: Theme.bgPrimary
        anchors.centerIn: parent
        z: 101

        border.color: Theme.borderSecondary
        border.width: 0.5

        Column {
            id: dialogContent
            width: parent.width - 48
            anchors.centerIn: parent
            spacing: 20

            Column {
                width: parent.width
                spacing: 6

                Text {
                    text: (confirmDialog.mode === "remove" ? "Remove " : "Install ") + confirmDialog.pkgName
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

            Rectangle {
                width: parent.width
                height: 0.5
                color: Theme.borderTertiary
            }

            Row {
                spacing: 16
                visible: confirmDialog.mode === "install"

                MetaLine { label: "Download"; value: confirmDialog.pkgSize || "\u2013" }
                MetaLine { label: "Size on disk"; value: confirmDialog.pkgInstalledSize || "\u2013" }
            }

            Column {
                width: parent.width
                spacing: 8
                visible: confirmDialog.mode === "install" && confirmDialog.pkgDependencies.length > 0

                Text {
                    text: "Dependencies (" + confirmDialog.pkgDependencies.length + ")"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: Theme.textSecondary
                }

                Rectangle {
                    width: parent.width
                    height: Math.min(depListView.contentHeight + 16, 200)
                    radius: Theme.radiusMd
                    color: Theme.bgSecondary
                    border.color: Theme.borderTertiary
                    border.width: 0.5
                    clip: true

                    ListView {
                        id: depListView
                        width: parent.width - 24
                        anchors.centerIn: parent
                        height: parent.height - 16
                        spacing: 4
                        model: confirmDialog.pkgDependencies
                        clip: true

                        delegate: Text {
                            text: modelData
                            font.pixelSize: 12
                            color: Theme.textSecondary
                            width: depListView.width
                        }
                    }
                }
            }

            Row {
                width: parent.width
                spacing: 12
                layoutDirection: Qt.RightToLeft

                ActionButton {
                    label: confirmDialog.mode === "remove" ? "Remove" : "Install"
                    primary: true
                    onClicked: {
                        confirmDialog.visible = false
                        if (confirmDialog.mode === "remove")
                            confirmDialog.removeConfirmed()
                        else
                            confirmDialog.installConfirmed()
                    }
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
        color: primary ? (area.pressed ? Theme.accentDark : Theme.accent) : Theme.bgSecondary
        border.color: primary ? Theme.accent : Theme.borderSecondary
        border.width: 0.5

        property string label
        property bool primary: false
        signal clicked()

        Text {
            id: btnText
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: primary ? Theme.accentSurface : Theme.textSecondary
        }

        MouseArea {
            id: area
            anchors.fill: parent
            onClicked: parent.clicked()
        }
    }
}