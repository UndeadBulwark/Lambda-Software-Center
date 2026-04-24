import QtQuick
import QtQuick.Layouts

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
        anchors.fill: parent
        anchors.leftMargin: Theme.contentPadding
        anchors.rightMargin: Theme.contentPadding
        spacing: 12

        SearchBar {
            id: searchBar
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            onSearchTextChanged: topbar.searchTriggered(text)
        }

        SourceTabs {
            id: sourceTabs
            Layout.alignment: Qt.AlignVCenter
            onFilterChanged: topbar.sourceFilterChanged(filter)
        }
    }
}
