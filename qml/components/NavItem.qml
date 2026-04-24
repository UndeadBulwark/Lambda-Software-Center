import QtQuick
import QtQuick.Layouts
import LambdaSoftwareCenter

Rectangle {
    id: navItem
    width: parent ? parent.width : 200
    height: 38
    color: active ? Theme.bgPrimary : "transparent"

    property bool active: false
    property string text: ""
    property int count: 0
    property var sourceDot: null
    signal clicked()

    // Right border accent
    Rectangle {
        anchors.right: parent.right
        width: active ? 2 : 0
        height: parent.height
        color: Theme.accent
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 10

        // Source dot (for Sources group)
        Rectangle {
            id: dot
            width: navItem.sourceDot !== "" ? 8 : 0
            height: width
            Layout.alignment: Qt.AlignVCenter
            radius: width / 2
            color: navItem.sourceDot
            visible: navItem.sourceDot !== ""
        }

        Text {
            id: itemText
            text: navItem.text
            font.pixelSize: 13
            font.weight: active ? Font.Medium : Font.Normal
            color: active ? Theme.accent : Theme.textSecondary
            Layout.alignment: Qt.AlignVCenter
        }

        Item { Layout.fillWidth: true; height: 1 }

        // Updates count pill
        Rectangle {
            id: countPill
            visible: navItem.count > 0
            height: 18
            width: Math.max(20, countText.implicitWidth + 14)
            radius: 9
            color: Theme.aur
            Layout.alignment: Qt.AlignVCenter

            Text {
                id: countText
                anchors.centerIn: parent
                text: navItem.count
                font.pixelSize: 10
                color: Theme.aurSurface
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: navItem.clicked()

        onEntered: {
            if (!navItem.active)
                navItem.color = Theme.bgPrimary
        }
        onExited: {
            if (!navItem.active)
                navItem.color = "transparent"
        }
    }
}
