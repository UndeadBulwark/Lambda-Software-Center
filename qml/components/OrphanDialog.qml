import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: orphanDialog
    anchors.fill: parent
    visible: false
    opacity: visible ? 1 : 0
    color: "#AA000000"
    z: 100

    property var orphanList: []
    property var selectedOrphans: []
    property var selectedOrphanList: {
        var result = [];
        for (var i = 0; i < orphanList.length; ++i) {
            if (selectedOrphans[i])
                result.push(orphanList[i]);
        }
        return result;
    }
    property int sourceInt: 0

    signal cleanupConfirmed()
    signal dialogSkipped()

    onOrphanListChanged: {
        var sel = [];
        for (var i = 0; i < orphanList.length; ++i)
            sel.push(true);
        selectedOrphans = sel;
    }

    function selectAll() {
        var sel = [];
        for (var i = 0; i < orphanList.length; ++i)
            sel.push(true);
        selectedOrphans = sel;
    }

    function deselectAll() {
        var sel = [];
        for (var i = 0; i < orphanList.length; ++i)
            sel.push(false);
        selectedOrphans = sel;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: orphanDialog.dialogSkipped()
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
                    text: "Clean up orphan packages?"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: Theme.textPrimary
                }

                Text {
                    text: orphanDialog.orphanList.length + " package" +
                          (orphanDialog.orphanList.length !== 1 ? "s" : "") +
                          " installed as dependencies are no longer required"
                    font.pixelSize: 13
                    color: Theme.textTertiary
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }

            Rectangle {
                width: parent.width
                height: 0.5
                color: Theme.borderTertiary
            }

            Column {
                width: parent.width
                spacing: 8

                Row {
                    width: parent.width
                    spacing: 12

                    Text {
                        id: orphansLabel
                        text: "Orphaned packages (" + orphanDialog.orphanList.length + ")"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: Theme.textSecondary
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Item {
                        width: parent.width - orphansLabel.implicitWidth
                              - selectAllText.implicitWidth - deselectAllText.implicitWidth - 36
                        height: 1
                    }

                    Text {
                        id: selectAllText
                        text: "Select all"
                        font.pixelSize: 11
                        color: selectAllArea.containsMouse ? Theme.textPrimary : Theme.accent
                        anchors.verticalCenter: parent.verticalCenter

                        MouseArea {
                            id: selectAllArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: orphanDialog.selectAll()
                        }
                    }

                    Text {
                        text: "\u00B7"
                        font.pixelSize: 11
                        color: Theme.textTertiary
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        id: deselectAllText
                        text: "Deselect all"
                        font.pixelSize: 11
                        color: deselectAllArea.containsMouse ? Theme.textPrimary : Theme.accent
                        anchors.verticalCenter: parent.verticalCenter

                        MouseArea {
                            id: deselectAllArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: orphanDialog.deselectAll()
                        }
                    }
                }

                // TODO: If the user unchecks packages and clicks Skip, those unchecked
                // selections are discarded. A future mechanism (e.g. persistent ignore list
                // or deferred cleanup queue in the Installed page) should preserve partial
                // selections so users can clean up some orphans now and return to others later.

                Rectangle {
                    width: parent.width
                    height: Math.min(orphanListView.contentHeight + 16, 200)
                    radius: Theme.radiusMd
                    color: Theme.bgSecondary
                    border.color: Theme.borderTertiary
                    border.width: 0.5
                    clip: true

                    ListView {
                        id: orphanListView
                        width: parent.width - 24
                        anchors.centerIn: parent
                        height: parent.height - 16
                        spacing: 4
                        model: orphanDialog.orphanList
                        clip: true

                        delegate: Row {
                            spacing: 8
                            width: orphanListView.width

                            Rectangle {
                                id: checkbox
                                width: 16
                                height: 16
                                radius: 3
                                y: 2
                                color: orphanDialog.selectedOrphans[index]
                                       ? Theme.accent : Theme.bgPrimary
                                border.color: orphanDialog.selectedOrphans[index]
                                              ? Theme.accent : Theme.borderSecondary
                                border.width: 0.5

                                Text {
                                    anchors.centerIn: parent
                                    text: orphanDialog.selectedOrphans[index] ? "\u2713" : ""
                                    font.pixelSize: 11
                                    color: Theme.accentSurface
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        var sel = orphanDialog.selectedOrphans.slice();
                                        sel[index] = !sel[index];
                                        orphanDialog.selectedOrphans = sel;
                                    }
                                }
                            }

                            Text {
                                text: modelData
                                font.pixelSize: 12
                                color: orphanDialog.selectedOrphans[index]
                                       ? Theme.textPrimary : Theme.textTertiary
                                anchors.verticalCenter: checkbox.verticalCenter
                            }
                        }
                    }
                }
            }

            Row {
                width: parent.width
                spacing: 12
                layoutDirection: Qt.RightToLeft

                ActionButton {
                    label: "Clean up (" + orphanDialog.selectedOrphanList.length + ")"
                    primary: true
                    enabled: orphanDialog.selectedOrphanList.length > 0
                    onClicked: {
                        orphanDialog.visible = false
                        orphanDialog.cleanupConfirmed()
                    }
                }

                ActionButton {
                    label: "Skip"
                    onClicked: {
                        orphanDialog.visible = false
                        orphanDialog.dialogSkipped()
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