import QtQuick
import QtQuick.Controls
import LambdaSoftwareCenter

Rectangle {
    id: searchBar
    height: 32
    radius: Theme.radiusMd
    color: Theme.bgSecondary

    signal searchTextChanged(string text)

    // Border workaround: Qt Quick border.width doesn't support 0.5px
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: Theme.borderSecondary
        border.width: 1 // Closest to 0.5px; actual visual border thinner due to antialiasing
    }

    // Search icon (circle + line)
    Canvas {
        id: searchIcon
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 14
        height: 14

        onPaint: {
            var ctx = getContext("2d");
            ctx.strokeStyle = Theme.textTertiary;
            ctx.lineWidth = 1.5;
            ctx.beginPath();
            ctx.arc(6, 6, 4, 0, 2 * Math.PI);
            ctx.stroke();
            ctx.beginPath();
            ctx.moveTo(9, 9);
            ctx.lineTo(12, 12);
            ctx.stroke();
        }
    }

    TextField {
        id: searchInput
        anchors.fill: parent
        leftPadding: 32
        rightPadding: 12
        verticalAlignment: TextInput.AlignVCenter
        placeholderText: "Search packages..."
        font.pixelSize: 13
        color: Theme.textPrimary
        background: Item {}

        // Debounced search at 250ms
        Timer {
            id: debounceTimer
            interval: 250
            onTriggered: searchBar.searchTextChanged(searchInput.text)
        }

        onTextChanged: debounceTimer.restart()
    }
}
