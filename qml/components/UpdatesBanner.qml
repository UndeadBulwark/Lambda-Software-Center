import QtQuick
import QtQuick.Layouts
import LambdaSoftwareCenter

Rectangle {
    id: root
    height: bannerRow.height + 20
    radius: Theme.radiusMd
    color: Theme.aurSurface
    border.width: 0.5
    border.color: Theme.aurBorder

    signal updateAllClicked()

    RowLayout {
        id: bannerRow
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
            text: updateCount + " update" + (updateCount === 1 ? "" : "s") + " available"
            font.pixelSize: 12
            color: Theme.aurText
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        Rectangle {
            width: btnText.width + 24
            height: 26
            radius: Theme.radiusMd
            color: Theme.aur
            Layout.alignment: Qt.AlignVCenter

            Text {
                id: btnText
                text: "Update all"
                font.pixelSize: 11
                color: Theme.aurSurface
                anchors.centerIn: parent
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.updateAllClicked()
            }
        }
    }
}