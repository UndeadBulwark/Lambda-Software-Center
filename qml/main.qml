import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 640
    title: "Lambda Software Center"
    color: Theme.bgPrimary

    property string currentPage: "browse"
    property string searchQuery: ""
    property int activeSourceFilter: -1 // -1: All, 0: Pacman, 1: AUR, 2: Flatpak

    Row {
        anchors.fill: parent

        Sidebar {
            id: sidebar
            height: parent.height
            currentPage: root.currentPage
            onRequestPage: function(page) { root.currentPage = page }
        }

        Column {
            width: parent.width - Theme.sidebarWidth
            height: parent.height

            Topbar {
                id: topbar
                width: parent.width
                onSearchTriggered: function(text) { root.searchQuery = text }
                onSourceFilterChanged: function(filter) { root.activeSourceFilter = filter }
            }

            StackView {
                id: contentStack
                width: parent.width
                height: parent.height - topbar.height - statusBar.height
                initialItem: browsePageItem
                clip: true
            }

            StatusBar {
                id: statusBar
                width: parent.width
            }
        }
    }

    Component {
        id: browsePageItem
        BrowsePage {
            searchQuery: root.searchQuery
            sourceFilter: root.activeSourceFilter
        }
    }

    Component { id: featuredPageItem;  FeaturedPage {} }
    Component { id: recentPageItem;    RecentPage {} }
    Component { id: installedPageItem;  InstalledPage {} }
    Component { id: updatesPageItem;   UpdatesPage {} }

    onCurrentPageChanged: {
        switch (currentPage) {
        case "browse":    contentStack.replace(browsePageItem);  break;
        case "featured":  contentStack.replace(featuredPageItem); break;
        case "recent":    contentStack.replace(recentPageItem);   break;
        case "installed": contentStack.replace(installedPageItem); break;
        case "updates":   contentStack.replace(updatesPageItem);  break;
        }
    }
}
