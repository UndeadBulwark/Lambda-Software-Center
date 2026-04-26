import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LambdaSoftwareCenter 1.0

Item {
    id: detailPage

    property var packageData: null

    // Populated by the backend when it has a real version list.
    // Defaults to the single current version so the picker always has something.
    property var availableVersions: packageData ? [packageData.version] : []

    // Tracks which version the user has selected in the picker.
    property string selectedVersion: packageData ? packageData.version : ""

    // ─── Helpers ────────────────────────────────────────────────────────────

    function badgeForSource(src) {
        if (src === 0) return "pacman"
        if (src === 1) return "aur"
        if (src === 2) return "flatpak"
        return "pacman"
    }

    function sourceColor(src) {
        if (src === 0) return "#185FA5"
        if (src === 1) return "#854F0B"
        if (src === 2) return "#534AB7"
        return Theme.textTertiary
    }

    function sourceSurface(src) {
        if (src === 0) return "#E6F1FB"
        if (src === 1) return "#FAEEDA"
        if (src === 2) return "#EEEDFE"
        return Theme.bgSecondary
    }

    function formatBytes(bytes) {
        if (!bytes || bytes <= 0) return "–"
        if (bytes < 1024)        return bytes + " B"
        if (bytes < 1048576)     return (bytes / 1024).toFixed(1) + " KB"
        if (bytes < 1073741824)  return (bytes / 1048576).toFixed(1) + " MB"
        return (bytes / 1073741824).toFixed(2) + " GB"
    }

    function openInstallDialog() {
        var dlg = Window.window.confirmDialog
        dlg.packageName    = packageData.name
        dlg.packageVersion = selectedVersion
        dlg.sourceBadge    = badgeForSource(packageData.source)
        dlg.dependencyList = packageData.dependencies || []
        dlg.downloadSize   = formatBytes(packageData.downloadSize)
        dlg.mode           = "install"
        dlg.visible        = true
    }

    function openRemoveDialog() {
        var dlg = Window.window.confirmDialog
        dlg.packageName    = packageData.name
        dlg.packageVersion = packageData.version
        dlg.sourceBadge    = badgeForSource(packageData.source)
        dlg.dependencyList = []
        dlg.downloadSize   = ""
        dlg.mode           = "remove"
        dlg.visible        = true
    }

    // ─── Scrollable content ─────────────────────────────────────────────────

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
                margins: 20
            }
            spacing: 0

            // ── Back button ─────────────────────────────────────────────────

            Item {
                Layout.fillWidth: true
                implicitHeight: 36

                MouseArea {
                    id: backArea
                    width:  backRow.implicitWidth + 4
                    height: parent.height
                    hoverEnabled: true
                    cursorShape:  Qt.PointingHandCursor
                    onClicked: StackView.view.pop()

                    Row {
                        id: backRow
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        Text {
                            text:            "←"
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

            // ── App header row ───────────────────────────────────────────────

            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: 20
                spacing: 16

                // App icon
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

                // Name / version / badges
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

                        // Source badge pill
                        Rectangle {
                            implicitWidth:  sourcePillText.implicitWidth + 16
                            implicitHeight: 18
                            radius: 10
                            color: packageData ? sourceSurface(packageData.source) : "transparent"

                            Text {
                                id:             sourcePillText
                                anchors.centerIn: parent
                                text:           packageData ? badgeForSource(packageData.source) : ""
                                font.pixelSize: 10
                                font.weight:    Font.Medium
                                color:          packageData ? sourceColor(packageData.source) : Theme.textTertiary
                            }
                        }

                        // Installed badge pill (visible when installed)
                        Rectangle {
                            visible:        packageData && packageData.isInstalled
                            implicitWidth:  installedPillText.implicitWidth + 16
                            implicitHeight: 18
                            radius: 10
                            color:  "#EAF3DE"

                            Text {
                                id:             installedPillText
                                anchors.centerIn: parent
                                text:           "installed"
                                font.pixelSize: 10
                                font.weight:    Font.Medium
                                color:          "#3B6D11"
                            }
                        }
                    }
                }

                // Action controls (version picker + install/remove button)
                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignVCenter

                    // ── Version picker ───────────────────────────────────────
                    // Shown only when there is more than one available version,
                    // or when the package is not yet installed (so user can
                    // choose which version to install once the backend provides
                    // a real list).
                    ComboBox {
                        id: versionPicker
                        visible: (availableVersions.length > 1) ||
                                 (packageData && !packageData.isInstalled)
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
                            radius:       6
                            border.width: 0.5
                            border.color: versionPicker.pressed
                                          ? "#97C459"
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
                                radius:       6
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

                    // ── Install button (not installed) ───────────────────────

                    Rectangle {
                        id: installButton
                        visible:       packageData && !packageData.isInstalled
                        implicitWidth: installLabel.implicitWidth + 28
                        height:        30
                        radius:        6
                        color:         installArea.pressed
                                       ? "#2F560D"
                                       : "#3B6D11"

                        Behavior on color {
                            ColorAnimation { duration: 80 }
                        }

                        Text {
                            id:                installLabel
                            anchors.centerIn:  parent
                            text:              "Install"
                            font.pixelSize:    12
                            color:             "#EAF3DE"
                        }

                        MouseArea {
                            id:          installArea
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked:   detailPage.openInstallDialog()
                        }
                    }

                    // ── Remove button (installed) ────────────────────────────

                    Rectangle {
                        id: removeButton
                        visible:       packageData && packageData.isInstalled
                        implicitWidth: removeLabel.implicitWidth + 28
                        height:        30
                        radius:        6
                        color:         removeArea.containsMouse
                                       ? Theme.bgSecondary
                                       : "transparent"
                        border.width:  0.5
                        border.color:  removeArea.containsMouse
                                       ? Theme.borderSecondary
                                       : Theme.borderSecondary

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
                            onClicked:    detailPage.openRemoveDialog()
                        }
                    }
                }
            }

            // ── Divider ──────────────────────────────────────────────────────

            Rectangle {
                Layout.fillWidth:   true
                height:             0.5
                color:              Theme.borderTertiary
                Layout.bottomMargin: 20
            }

            // ── Description ──────────────────────────────────────────────────

            Text {
                Layout.fillWidth: true
                text: packageData
                      ? (packageData.longDescription !== ""
                         ? packageData.longDescription
                         : packageData.description)
                      : "No description available."
                font.pixelSize:   12
                color:            Theme.textSecondary
                wrapMode:         Text.WordWrap
                lineHeight:       1.45
                Layout.bottomMargin: 24
            }

            // ── Details section ───────────────────────────────────────────────

            Text {
                text:             "Details"
                font.pixelSize:   15
                font.weight:      Font.Medium
                color:            Theme.textPrimary
                Layout.bottomMargin: 14
            }

            // Key-value detail grid
            GridLayout {
                Layout.fillWidth:   true
                columns:            2
                columnSpacing:      24
                rowSpacing:         10
                Layout.bottomMargin: 8

                // Installed size
                Text {
                    text:           "Installed size"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? formatBytes(packageData.installedSize) : "–"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                // Download size
                Text {
                    text:           "Download size"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? formatBytes(packageData.downloadSize) : "–"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                // Source
                Text {
                    text:           "Source"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? badgeForSource(packageData.source) : "–"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                // Version
                Text {
                    text:           "Version"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    text:           packageData ? packageData.version : "–"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }

                // Dependencies
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

                // Flatpak rating (only shown when source === 2 and rating > 0)
                Text {
                    visible:        packageData && packageData.source === 2 && packageData.rating > 0
                    text:           "Rating"
                    font.pixelSize: 12
                    color:          Theme.textTertiary
                }
                Text {
                    visible:        packageData && packageData.source === 2 && packageData.rating > 0
                    text:           packageData ? packageData.rating.toFixed(1) + " / 5" : "–"
                    font.pixelSize: 12
                    color:          Theme.textSecondary
                }
            }

            // Bottom padding
            Item { implicitHeight: 20 }
        }
    }
}
