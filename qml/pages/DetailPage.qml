import QtQuick
import LambdaSoftwareCenter

Rectangle {
    id: detailPage
    color: Theme.bgPrimary

    property var packageData

    Flickable {
        anchors.fill: parent
        contentHeight: contentCol.height + 40
        clip: true

        Column {
            id: contentCol
            width: parent.width - 40
            x: Theme.contentPadding
            y: Theme.contentPadding
            spacing: 16

            // ── Back button ──
            Text {
                text: "\u2190 Back"
                font.pixelSize: 13
                color: Theme.textTertiary
                MouseArea {
                    anchors.fill: parent
                    onClicked: StackView.view.pop()
                }
            }

            // ── App header row ──
            Row {
                width: parent.width
                spacing: 16

                Rectangle {
                    width: 52
                    height: 52
                    radius: 12
                    color: iconColorForSource(packageData.source)

                    Text {
                        anchors.centerIn: parent
                        text: initialForName(packageData.name)
                        font.pixelSize: 22
                        color: Theme.textSecondary
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Text {
                        text: packageData.name || "Package"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: Theme.textPrimary
                    }

                    Row {
                        spacing: 8

                        BadgePill {
                            variant: badgeForSource(packageData.source)
                            label: badgeForSource(packageData.source)
                            visible: packageData.source !== undefined
                        }

                        Text {
                            text: packageData.version || ""
                            font.pixelSize: 13
                            color: Theme.textTertiary
                        }
                    }
                }

                Item {
                    width: parent.width - iconHeader.width - buttonsRow.width - 24
                    height: 1
                }

                Row {
                    id: buttonsRow
                    spacing: 8
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        width: installText.implicitWidth + 20
                        height: 28
                        radius: Theme.radiusMd
                        color: "transparent"
                        border.color: Theme.borderSecondary
                        border.width: 1

                        Text {
                            id: installText
                            anchors.centerIn: parent
                            text: "Install"
                            font.pixelSize: 12
                            color: Theme.textSecondary
                        }
                    }

                    Rectangle {
                        width: removeText.implicitWidth + 20
                        height: 28
                        radius: Theme.radiusMd
                        color: "transparent"
                        border.color: Theme.borderSecondary
                        border.width: 1
                        visible: packageData.state === 1

                        Text {
                            id: removeText
                            anchors.centerIn: parent
                            text: "Remove"
                            font.pixelSize: 12
                            color: Theme.textSecondary
                        }
                    }
                }
            }

            // ── Divider ──
            Rectangle {
                width: parent.width
                height: 1
                color: Theme.borderTertiary
            }

            // ── Description ──
            Text {
                width: parent.width
                text: packageData.longDescription || packageData.description || "No description available."
                font.pixelSize: 13
                color: Theme.textSecondary
                lineHeightMode: Text.FixedHeight
                lineHeight: 13 * 1.5
                wrapMode: Text.WordWrap
            }

            // ── Metadata row ──
            Row {
                width: parent.width
                spacing: 24

                MetaItem {
                    label: "Installed size"
                    value: formatSize(packageData.installedSize)
                    visible: packageData.installedSize > 0
                }

                MetaItem {
                    label: "Download size"
                    value: packageData.downloadSize || "N/A"
                    visible: packageData.downloadSize != ""
                }

                MetaItem {
                    label: "Dependencies"
                    value: packageData.dependencies ? packageData.dependencies.length : 0
                    visible: packageData.dependencies && packageData.dependencies.length > 0
                }
            }

            // ── Dependencies list ──
            Text {
                width: parent.width
                text: packageData.dependencies ? packageData.dependencies.join(", ") : ""
                font.pixelSize: 12
                color: Theme.textTertiary
                wrapMode: Text.WordWrap
                visible: packageData.dependencies && packageData.dependencies.length > 0
            }

            // ── Flatpak rating ──
            Row {
                width: parent.width
                spacing: 6
                visible: packageData.source === 2 && packageData.rating > 0

                Text {
                    text: "Rating"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: Theme.textSecondary
                }

                Text {
                    text: (packageData.rating || 0).toFixed(1) + " / 5"
                    font.pixelSize: 12
                    color: Theme.flatpak
                }
            }
        }
    }

    component MetaItem: Column {
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

    function initialForName(name) {
        if (!name) return "?";
        return name.substring(0, 1).toUpperCase();
    }

    function iconColorForSource(source) {
        switch (source) {
        case 0: return Theme.pacmanSurface;
        case 1: return Theme.aurSurface;
        case 2: return Theme.flatpakSurface;
        default: return Theme.bgSecondary;
        }
    }

    function badgeForSource(source) {
        switch (source) {
        case 0: return "pacman";
        case 1: return "aur";
        case 2: return "flatpak";
        default: return "";
        }
    }

    function formatSize(bytes) {
        if (!bytes || bytes <= 0) return "N/A";
        if (bytes < 1024) return bytes + " B";
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB";
        if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + " MB";
        return (bytes / (1024 * 1024 * 1024)).toFixed(1) + " GB";
    }
}
