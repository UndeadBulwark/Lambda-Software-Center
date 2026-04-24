import QtQuick
import QtQuick.Controls
import LambdaSoftwareCenter

Rectangle {
    id: installedPage
    color: Theme.bgPrimary

    Component.onCompleted: pacmanBackend.listInstalled()

    GridView {
        id: grid
        anchors.fill: parent
        anchors.margins: 20
        cellWidth: Math.floor(width / 3)
        cellHeight: 160
        clip: true
        model: installedModel

        delegate: PackageCard {
            width: grid.cellWidth - 6
            height: grid.cellHeight - 10
            anchors.margins: 3
        }

        Text {
            anchors.centerIn: parent
            text: "No installed packages"
            font.pixelSize: 13
            color: Theme.textTertiary
            visible: grid.count === 0
        }
    }
}
