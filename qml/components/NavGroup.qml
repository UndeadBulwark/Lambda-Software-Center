import QtQuick
import LambdaSoftwareCenter

Column {
    id: navGroup
    width: parent.width
    spacing: 0
    topPadding: 8
    bottomPadding: 18

    property string label: ""

    Text {
        text: navGroup.label
        font.pixelSize: 11
        font.weight: Font.Medium
        font.letterSpacing: 1.2
        visible: navGroup.label !== ""
        height: visible ? implicitHeight + 6 : 0
        verticalAlignment: Text.AlignBottom
        leftPadding: 16
        rightPadding: 16
    }
}
