import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LambdaSoftwareCenter

Rectangle {
    id: topbar
    height: 56
    color: Theme.bgPrimary

    signal searchTriggered(string text)
    signal sourceFilterChanged(int filter)

    // Bottom border
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.borderTertiary
    }

    RowLayout {
        x: Theme.contentPadding
        width: parent.width - 2 * Theme.contentPadding
        height: parent.height
        spacing: 12

        SearchBar {
            id: searchBar
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            onSearchTextChanged: function(text) { topbar.searchTriggered(text) }
        }

        SourceTabs {
            id: sourceTabs
            Layout.alignment: Qt.AlignVCenter
            onFilterChanged: function(filter) { topbar.sourceFilterChanged(filter) }
        }
    }
}
