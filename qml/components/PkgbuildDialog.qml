import QtQuick
import QtQuick.Controls
import LambdaSoftwareCenter

Rectangle {
    id: pkgbuildDialog
    anchors.fill: parent
    visible: false
    opacity: visible ? 1 : 0
    color: "#AA000000"
    z: 100

    property string pkgName: ""
    property string pkgVersion: ""
    property string pkgbuildContent: ""

    signal reviewConfirmed()
    signal reviewCancelled()

    MouseArea {
        anchors.fill: parent
        onClicked: pkgbuildDialog.reviewCancelled()
    }

    Behavior on opacity {
        NumberAnimation { duration: 120 }
    }

    Rectangle {
        width: 520
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
            spacing: 16

            Column {
                width: parent.width
                spacing: 6

                Text {
                    text: "Review PKGBUILD"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: Theme.textPrimary
                }

                Row {
                    spacing: 8
                    BadgePill {
                        variant: "aur"
                        label: "aur"
                    }
                    Text {
                        text: pkgbuildDialog.pkgName + " " + pkgbuildDialog.pkgVersion
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

            Text {
                text: "AUR packages are unreviewed by Arch. Review the PKGBUILD before proceeding."
                font.pixelSize: 12
                color: Theme.aur
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Rectangle {
                width: parent.width
                height: Math.min(pkgbuildFlickable.contentHeight + 16, 260)
                radius: Theme.radiusMd
                color: Theme.bgSecondary
                border.color: Theme.borderTertiary
                border.width: 0.5
                clip: true

                Flickable {
                    id: pkgbuildFlickable
                    width: parent.width - 24
                    anchors.centerIn: parent
                    height: parent.height - 16
                    contentWidth: width
                    contentHeight: pkgbuildText.implicitHeight
                    clip: true

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    TextEdit {
                        id: pkgbuildText
                        width: pkgbuildFlickable.width
                        text: pkgbuildDialog.pkgbuildContent
                        font.pixelSize: 12
                        font.family: "monospace"
                        color: Theme.textSecondary
                        wrapMode: TextEdit.NoWrap
                        readOnly: true
                        selectByMouse: true
                    }
                }
            }

            Row {
                width: parent.width
                spacing: 12
                layoutDirection: Qt.RightToLeft

                ActionButton {
                    label: "I've reviewed this \u2014 Continue"
                    primary: true
                    onClicked: {
                        pkgbuildDialog.visible = false
                        pkgbuildDialog.reviewConfirmed()
                    }
                }

                ActionButton {
                    label: "Cancel"
                    onClicked: {
                        pkgbuildDialog.visible = false
                        pkgbuildDialog.reviewCancelled()
                    }
                }
            }
        }
    }

    component ActionButton: Rectangle {
        width: btnText.implicitWidth + 24
        height: 32
        radius: Theme.radiusMd
        color: {
            if (!enabled) return Theme.bgSecondary
            return primary ? (area.pressed ? Theme.accentDark : Theme.accent) : Theme.bgSecondary
        }
        border.color: {
            if (!enabled) return Theme.borderTertiary
            return primary ? Theme.accent : Theme.borderSecondary
        }
        border.width: 0.5

        property string label
        property bool primary: false
        signal clicked()

        Text {
            id: btnText
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: {
                if (!parent.enabled) return Theme.textTertiary
                return parent.primary ? Theme.accentSurface : Theme.textSecondary
            }
        }

        MouseArea {
            id: area
            anchors.fill: parent
            enabled: parent.enabled
            onClicked: parent.clicked()
        }
    }
}