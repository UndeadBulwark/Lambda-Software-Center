import QtQuick
import QtQuick.Controls
import LambdaSoftwareCenter

Rectangle {
    id: updatesPage
    color: Theme.bgPrimary

    Component.onCompleted: pacmanBackend.checkUpdates()

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        GridView {
            id: grid
            width: parent.width
            height: parent.height - 30
            cellWidth: Math.floor(width / 3)
            cellHeight: 160
            clip: true
            model: updatesModel

            delegate: PackageCard {
                width: grid.cellWidth - 6
                height: grid.cellHeight - 10
                anchors.margins: 3
            }

            Text {
                anchors.centerIn: parent
                text: "No updates available"
                font.pixelSize: 13
                color: Theme.textTertiary
                visible: grid.count === 0
            }
        }
    }
}
