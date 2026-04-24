import QtQuick

Column {
    id: navGroup
    spacing: 0

    property string label

    Text {
        id: labelText
        text: navGroup.label
        font.pixelSize: 10
        font.weight: Font.Medium
        color: Theme.textTertiary
        letterSpacing: 0.08 * font.pixelSize
        height: navGroup.label === "" ? 0 : implicitHeight + 6
        verticalAlignment: Text.AlignBottom
        leftPadding: 16
        rightPadding: 16
    }

    Column {
        width: parent.width
        spacing: 0
        children: navGroup.children
    }
}
