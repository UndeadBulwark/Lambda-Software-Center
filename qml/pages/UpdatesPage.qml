import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LambdaSoftwareCenter

Rectangle {
    id: updatesPage
    color: Theme.bgPrimary

    signal updateAllRequested()
    signal updatePackageRequested(var pkg)

    Column {
        anchors.fill: parent
        anchors.margins: Theme.contentPadding
        spacing: 14

        Rectangle {
            id: updatesBanner
            width: parent.width
            height: bannerContent.height + 20
            radius: Theme.radiusMd
            color: Theme.aurSurface
            border.width: 0.5
            border.color: Theme.aurBorder
            visible: updatesModel.rowCount() > 0

            RowLayout {
                id: bannerContent
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 14
                spacing: 12

                Rectangle {
                    width: 14; height: 14; radius: 7
                    color: Theme.aur
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        anchors.centerIn: parent
                        text: "!"
                        font.pixelSize: 9
                        font.bold: true
                        color: Theme.aurSurface
                    }
                }

                Text {
                    text: updatesModel.rowCount() + " update" + (updatesModel.rowCount() === 1 ? "" : "s") + " available"
                    font.pixelSize: 12
                    color: Theme.aurText
                    Layout.alignment: Qt.AlignVCenter
                }

                Item { Layout.fillWidth: true; height: 1 }

                Rectangle {
                    width: updateAllText.width + 24
                    height: 26
                    radius: Theme.radiusMd
                    color: Theme.aur
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        id: updateAllText
                        text: "Update all"
                        font.pixelSize: 11
                        color: Theme.aurSurface
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: updatesPage.updateAllRequested()
                    }
                }
            }
        }

        Text {
            text: "Updates"
            font.pixelSize: 15
            font.weight: Font.Medium
            color: Theme.textPrimary
            visible: updatesModel.rowCount() > 0
        }

        ListView {
            id: updateList
            width: parent.width
            height: parent.height
                   - (updatesBanner.visible ? updatesBanner.height + 14 : 0)
                   - (updatesModel.rowCount() > 0 ? 34 : 0)
            clip: true
            spacing: 0
            model: updatesModel

            delegate: Rectangle {
                width: updateList.width
                height: 52
                color: index % 2 === 0 ? Theme.bgPrimary : Theme.bgSecondary

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 10

                    Rectangle {
                        id: srcIcon
                        width: 32; height: 32; radius: Theme.radiusMd
                        color: model.source === 1 ? Theme.aurSurface : Theme.pacmanSurface
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: model.source === 1 ? "A" : "P"
                            font.pixelSize: 13
                            font.bold: true
                            color: model.source === 1 ? Theme.aur : Theme.pacman
                        }
                    }

                    Column {
                        id: nameCol
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: model.name
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            color: Theme.textPrimary
                        }

                        Text {
                            text: model.version + "  \u2192  " + model.newVersion
                            font.pixelSize: 11
                            color: Theme.textTertiary
                        }
                    }

                    Item { width: parent.width - srcIcon.width - nameCol.width - updateBtn.width - 40; height: parent.height }

                    Rectangle {
                        id: updateBtn
                        width: 60; height: 26
                        radius: Theme.radiusMd
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: "Update"
                            font.pixelSize: 11
                            color: Theme.accentSurface
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                updatesPage.updatePackageRequested({
                                    "packageId": model.id,
                                    "name": model.name,
                                    "version": model.version,
                                    "source": model.source,
                                    "newVersion": model.newVersion,
                                    "dependencies": []
                                })
                            }
                        }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: "No updates available"
                font.pixelSize: 13
                color: Theme.textTertiary
                visible: updateList.count === 0
            }
        }
    }

    Component.onCompleted: {
        pacmanBackend.checkUpdates()
        aurBackend.checkUpdates()
    }
}