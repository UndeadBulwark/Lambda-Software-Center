import QtQuick
import LambdaSoftwareCenter

Column {
    id: navGroup
    width: parent.width
    spacing: 0
    bottomPadding: 18

    property string label: ""

    Text {
        text: navGroup.label
        font.pixelSize: 10
        font.weight: Font.Medium
        color: Theme.textTertiary
        visible: navGroup.label !== ""
        height: visible ? implicitHeight + 6 : 0
        verticalAlignment: Text.AlignBottom
        leftPadding: 16
        rightPadding: 16
    }
}
