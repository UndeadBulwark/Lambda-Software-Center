import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LambdaSoftwareCenter

Rectangle {
    id: detailPage
    color: Theme.bgPrimary

    property var packageData: null
    signal goBack()
    signal installRequested(var pkg)
    signal removeRequested(var pkg)

    property var availableVersions: packageData ? [packageData.version] : []
    property string selectedVersion: packageData ? packageData.version : ""

    function badgeForSource(src) {
        if (src === 0) return "pacman"
        if (src === 1) return "aur"
        if (src === 2) return "flatpak"
        return "pacman"
    }

    function sourceColor(src) {
        if (src === 0) return Theme.pacman
        if (src === 1) return Theme.aur
        if (src === 2) return Theme.flatpak
        return Theme.textTertiary
    }

    function sourceSurface(src) {
        if (src === 0) return Theme.pacmanSurface
        if (src === 1) return Theme.aurSurface
        if (src === 2) return Theme.flatpakSurface
        return Theme.bgSecondary
    }

    function formatBytes(bytes) {
        if (!bytes || bytes <= 0) return "\u2013"
        if (bytes < 1024)        return bytes + " B"
        if (bytes < 1048576)     return (bytes / 1024).toFixed(1) + " KB"
        if (bytes < 1073741824)  return (bytes / 1048576).toFixed(1) + " MB"
        return (bytes / 1073741824).toFixed(2) + " GB"
    }

    Flickable {
        anchors.fill: parent
        contentHeight: pageColumn.implicitHeight + 40
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ColumnLayout {
            id: pageColumn
            anchors {
                left:   parent.left
                right:  parent.right
                top:    parent.top
                margins: Theme.contentPadding
            }
            spacing: 0

            Item {
                Layout.fillWidth: true
                implicitHeight: 36

                MouseArea {
                    id: backArea
                    width:  backRow.implicitWidth + 4
                    height: parent.height
                    hoverEnabled: true
                    cursorShape:  Qt.PointingHandCursor
                    onClicked: detailPage.goBack()

                    Row {
                        id: backRow
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        Text {
                            text:            "\u2190"
                            font.pixelSize:  13
                            color:           backArea.containsMouse ? Theme.textPrimary : Theme.textSecondary
                        }
                        Text {
                            text:            "Back"
                            font.pixelSize:  13
                            color:           backArea.containsMouse ? Theme.textPrimary : Theme.textSecondary
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: 20
                spacing: 16

                Rectangle {
                    width:  52
                    height: 52
                    radius: 12
                    color:  packageData ? sourceSurface(packageData.source) : Theme.bgSecondary
                    Layout.alignment: Qt.AlignTop

                    Text {
                        anchors.centerIn: parent
                        text:             packageData ? packageData.name.charAt(0).toUpperCase() : ""
                        font.pixelSize:   22
                        font.weight:      Font.Medium
                        color:            packageData ? sourceColor(packageData.source) : Theme.textTertiary
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Text {
                        text:           packageData ? packageData.name : ""
                        font.pixelSize: 20
                        font.weight:    Font.Medium
                        color:          Theme.textPrimary
                    }

                    RowLayout {
                        spacing: 8

                        Text {
                            text:           packageData ? packageData.version : ""
                            font.pixelSize: 11
                            color:          Theme.textTertiary
                        }

                        BadgePill {
                            variant: packageData ? badgeForSource(packageData.source) : "pacman"
                            label:   packageData ? badgeForSource(packageData.source) : ""
                            visible: packageData !== null
                        }

                        BadgePill {
                            variant: "installed"
                            label:   "installed"
                            visible: packageData && packageData.state === 1
                        }
                    }
                }

                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignVCenter

                    ComboBox {
                        id: versionPicker
                        visible: (availableVersions.length > 1) ||
                                 (packageData && packageData.state !== 1)
                        model: availableVersions.length > 0
                               ? availableVersions
                               : (packageData ? [packageData.version] : [])

                        implicitWidth:  150
                        implicitHeight: 30

                        onCurrentTextChanged: selectedVersion = currentText

                        contentItem: Text {
                            leftPadding:        10
                            rightPadding:       24
                            text:               versionPicker.displayText
                            font.pixelSize:     12
                            color:              Theme.textSecondary
                            verticalAlignment:  Text.AlignVCenter
                            elide:              Text.ElideRight
                        }

                        background: Rectangle {
                            color:        Theme.bgPrimary
                            radius:       Theme.radiusMd
                            border.width: 0.5
                            border.color: versionPicker.pressed
                                          ? Theme.accentLight
                                          : Theme.borderSecondary
                        }

                        indicator: Item {
                            x:      versionPicker.width - width - 8
                            y:      (versionPicker.height - height) / 2
                            width:  12
                            height: 12

                            Canvas {
                                anchors.fill: parent
                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    ctx.strokeStyle = Theme.textTertiary
                                    ctx.lineWidth   = 1.5
                                    ctx.lineCap     = "round"
                                    ctx.lineJoin    = "round"
                                    ctx.beginPath()
                                    ctx.moveTo(2, 4)
                                    ctx.lineTo(6, 8)
                                    ctx.lineTo(10, 4)
                                    ctx.stroke()
                                }
                            }
                        }

                        popup: Popup {
                            y:              versionPicker.height + 2
                            width:          versionPicker.width
                            implicitHeight: listView.contentHeight
                            padding:        1

                            background: Rectangle {
                                color:        Theme.bgPrimary
                                radius:       Theme.radiusMd
                                border.width: 0.5
                                border.color: Theme.borderSecondary
                            }

                            contentItem: ListView {
                                id: listView
                                clip:            true
                                implicitHeight:  contentHeight
                                model:           versionPicker.delegateModel
                                boundsBehavior:  Flickable.StopAtBounds
                            }
                        }

                        delegate: ItemDelegate {
                            id: versionDelegate
                            width:  parent ? parent.width : 0
                            height: 30

                            contentItem: Text {
                                leftPadding:       10
                                text:              modelData
                                font.pixelSize:    12
                                color:             Theme.textPrimary
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                color: versionDelegate.hovered
                                       ? Theme.bgSecondary
                                       : "transparent"
                            }
                        }
                    }

                    Rectangle {
                        visible:       packageData && packageData.state !== 1
                        implicitWidth: installLabel.implicitWidth + 28
                        height:        30
                        radius:        Theme.radiusMd
                        color:         installArea.pressed
                                       ? Theme.accentDark
                                       : Theme.accent

                        Behavior on color {
                            ColorAnimation { duration: 80 }
                        }

                        Text {
                            id:                installLabel
                            anchors.centerIn:  parent
                            text:              "Install"
                            font.pixelSize:    12
                            color:             Theme.accentSurface
                        }

                        MouseArea {
                            id:          installArea
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked:   detailPage.installRequested(packageData)
                        }
                    }

                    Rectangle {
                        visible:       packageData && packageData.state === 1
                        implicitWidth: removeLabel.implicitWidth + 28
                        height:        30
                        radius:        Theme.radiusMd
                        color:         removeArea.containsMouse
                                       ? Theme.bgSecondary
                                       : "transparent"
                        border.width:  0.5
                        border.color:  Theme.borderSecondary

                        Behavior on color {
                            ColorAnimation { duration: 80 }
                        }

                        Text {
                            id:               removeLabel
                            anchors.centerIn: parent
                            text:             "Remove"
                            font.pixelSize:   12
                            color:            Theme.textSecondary
                        }

                        MouseArea {
                            id:           removeArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape:  Qt.PointingHandCursor
                            onClicked:    detailPage.removeRequested(packageData)
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth:   true
                height:             0.5
                color:              Theme.borderTertiary
                Layout.bottomMargin: 20
            }

            Text {
                Layout.fillWidth: true
                text: packageData
                      ? (packageData.longDescription !== ""
                         ? packageData.longDescription
                         : packageData.description)
                      : "No description available."
                font.pixelSize:     12
                color:              Theme.textSecondary
                wrapMode:           Text.WordWrap
                lineHeight:         1.45
                lineHeightMode:     Text.ProportionalHeight
                Layout.bottomMargin: 24
            }

            Text {
                text:             "Details"
                font.pixelSize:   15
                font.weight:      Font.Medium
                color:            Theme.textPrimary
                Layout.bottomMargin: 14
            }

            GridLayout {
                Layout.fillWidth:   true
                columns:            2
                columnSpacing:      24
                rowSpacing:         10
                Layout.bottomMargin: 8

                Text {
                    text:           "Installed size"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? formatBytes(packageData.installedSize) : "\u2013"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                Text {
                    text:           "Download size"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? formatBytes(packageData.downloadSize) : "\u2013"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                Text {
                    text:           "Source"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? badgeForSource(packageData.source) : "\u2013"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                Text {
                    text:           "Version"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? packageData.version : "\u2013"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                Text {
                    text:            "Dependencies"
                    font.pixelSize:  12
                    color:           Theme.textTertiary
                    Layout.alignment: Qt.AlignTop
                }
                Text {
                    Layout.fillWidth: true
                    text: packageData && packageData.dependencies && packageData.dependencies.length > 0
                          ? packageData.dependencies.join(", ")
                          : "None"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                    wrapMode:       Text.WordWrap
                }

                Text {
                    visible:        packageData && packageData.source === 2 && packageData.rating > 0
                    text:           "Rating"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    visible:        packageData && packageData.source === 2 && packageData.rating > 0
                    text:           packageData ? packageData.rating.toFixed(1) + " / 5" : "\u2013"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }
            }

            Item { implicitHeight: 20 }
        }
    }
}