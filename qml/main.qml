import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LambdaSoftwareCenter

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 640
    title: "Lambda Software Center"
    color: Theme.bgPrimary

    // Live theme binding: C++ signal updates systemDarkMode → Theme switches palette
    Binding {
        target: Theme
        property: "isDark"
        value: systemDarkMode ?? false
    }

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

    // Global modals and overlays
    ConfirmDialog {
        id: confirmDialog
        anchors.fill: parent
        visible: false

        onInstallConfirmed: {
            confirmDialog.visible = false
            progressDrawer.pkgName = confirmDialog.pkgName
            progressDrawer.statusText = "Downloading..."
            progressDrawer.percent = 0
            progressDrawer.visible = true
            mockTransaction.start()
        }

        onDialogCancelled: {
            confirmDialog.visible = false
        }

        Timer {
            id: mockTransaction
            interval: 500
            repeat: true
            onTriggered: {
                progressDrawer.percent += 20
                if (progressDrawer.percent >= 40)
                    progressDrawer.statusText = "Verifying..."
                if (progressDrawer.percent >= 60)
                    progressDrawer.statusText = "Installing..."
                if (progressDrawer.percent >= 100) {
                    mockTransaction.stop()
                    progressDrawer.statusText = "Complete"
                    progressDrawer.percent = 100
                    hideTimer.start()
                }
            }
        }

        Timer {
            id: hideTimer
            interval: 2000
            onTriggered: progressDrawer.visible = false
        }
    }

    ProgressDrawer {
        id: progressDrawer
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: false
    }

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
