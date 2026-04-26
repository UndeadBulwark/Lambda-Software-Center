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

    Binding {
        target: Theme
        property: "isDark"
        value: systemDarkMode ?? false
    }

    property string currentPage: "browse"
    property string searchQuery: ""
    property int activeSourceFilter: -1
    property bool lastTransactionWasRemove: false
    property int lastRemoveSource: -1
    property bool waitingForReasonFix: false
    property bool updateCheckPending: false

    function badgeForSource(src) {
        if (src === 0) return "pacman"
        if (src === 1) return "aur"
        if (src === 2) return "flatpak"
        return "pacman"
    }

    function formatBytes(bytes) {
        if (!bytes || bytes <= 0) return "\u2013"
        if (bytes < 1024)        return bytes + " B"
        if (bytes < 1048576)     return (bytes / 1024).toFixed(1) + " KB"
        if (bytes < 1073741824)  return (bytes / 1048576).toFixed(1) + " MB"
        return (bytes / 1073741824).toFixed(2) + " GB"
    }

    Row {
        anchors.fill: parent

        Sidebar {
            id: sidebar
            height: parent.height
            currentPage: root.currentPage
            updateCount: updateCount
            onRequestPage: function(page) { root.currentPage = page }
        }

        Column {
            width: parent.width - Theme.sidebarWidth
            height: parent.height

            Topbar {
                id: topbar
                width: parent.width
                onSearchTriggered: function(text) {
                    root.searchQuery = text
                    if (contentStack.depth > 1)
                        contentStack.pop()
                }
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
            onPackageClicked: function(pkg) {
                contentStack.push(detailPageItem, { packageData: pkg })
            }
            onUpdateAllRequested: transactionManager.systemUpgrade()
        }
    }

    Component { id: featuredPageItem;  FeaturedPage {} }
    Component { id: recentPageItem;    RecentPage {} }
    Component { id: installedPageItem;  InstalledPage {} }
    Component { id: updatesPageItem;   UpdatesPage {
        onUpdateAllRequested: transactionManager.systemUpgrade()
        onUpdatePackageRequested: function(pkg) {
            var pkgName = pkg.packageId.split("@")[0]
            if (pkg.source === 1) {
                aurBackend.install(pkg.packageId)
            } else {
                transactionManager.systemUpgrade([pkgName])
            }
        }
    } }

    Component {
        id: detailPageItem
        DetailPage {
            onGoBack: contentStack.pop()
            onInstallRequested: function(pkg) {
                confirmDialog.pkgId           = pkg.packageId || ""
                confirmDialog.pkgName         = pkg.name
                confirmDialog.pkgVersion      = pkg.version
                confirmDialog.pkgSource       = badgeForSource(pkg.source)
                confirmDialog.pkgSourceInt    = pkg.source
                confirmDialog.pkgDependencies = pkg.dependencies || []
                confirmDialog.pkgSize         = formatBytes(pkg.downloadSize)
                confirmDialog.mode            = "install"
                confirmDialog.visible         = true
            }
            onRemoveRequested: function(pkg) {
                confirmDialog.pkgId           = pkg.packageId || ""
                confirmDialog.pkgName         = pkg.name
                confirmDialog.pkgVersion      = pkg.version
                confirmDialog.pkgSource       = badgeForSource(pkg.source)
                confirmDialog.pkgSourceInt    = pkg.source
                confirmDialog.pkgDependencies = []
                confirmDialog.pkgSize         = ""
                confirmDialog.mode            = "remove"
                confirmDialog.visible         = true
            }
        }
    }

    ConfirmDialog {
        id: confirmDialog
        anchors.fill: parent
        visible: false

        onInstallConfirmed: {
            confirmDialog.visible = false
            if (confirmDialog.pkgSourceInt === 1)
                aurBackend.install(confirmDialog.pkgId)
            else
                transactionManager.install(confirmDialog.pkgId, confirmDialog.pkgSourceInt)
        }

        onRemoveConfirmed: {
            confirmDialog.visible = false
            root.lastTransactionWasRemove = true
            root.lastRemoveSource = confirmDialog.pkgSourceInt
            if (confirmDialog.pkgSourceInt === 1)
                aurBackend.remove(confirmDialog.pkgId)
            else
                transactionManager.remove(confirmDialog.pkgId, confirmDialog.pkgSourceInt)
        }

        onDialogCancelled: {
            confirmDialog.visible = false
        }
    }

    OrphanDialog {
        id: orphanDialog
        anchors.fill: parent
        visible: false

        onCleanupConfirmed: {
            transactionManager.removeOrphans(orphanDialog.selectedOrphanList)
        }

        onDialogSkipped: {
            orphanDialog.visible = false
        }
    }

    PkgbuildDialog {
        id: pkgbuildDialog
        anchors.fill: parent
        visible: false

        onReviewConfirmed: {
            aurBackend.continueBuild(pkgbuildDialog.pkgName)
        }

        onReviewCancelled: {
            aurBackend.cancelBuild(pkgbuildDialog.pkgName)
        }
    }

    Connections {
        target: aurBackend
        function onPkgbuildReady(pkgName, content) {
            pkgbuildDialog.pkgName = pkgName
            pkgbuildDialog.pkgbuildContent = content
            pkgbuildDialog.visible = true
        }
        function onInstallProgress(pkgId, percent, step) {
            progressDrawer.pkgName = pkgId
            progressDrawer.percent = percent
            progressDrawer.statusText = step
            if (!progressDrawer.visible)
                progressDrawer.visible = true
        }
    }

    Connections {
        target: pacmanBackend
        function onOrphansFound(orphans) {
            if (orphans.length > 0) {
                orphanDialog.orphanList = orphans
                orphanDialog.visible = true
            }
        }
        function onDirtyReasonsFound(packages) {
            if (packages.length > 0) {
                root.waitingForReasonFix = true
                transactionManager.fixInstallReasons(packages)
            } else {
                pacmanBackend.markReasonRepairDone()
            }
        }
    }

    Connections {
        target: transactionManager
        function onTransactionStarted(pkgId) {
            progressDrawer.pkgName = pkgId
            progressDrawer.isError = false
            progressDrawer.percent = 0
            progressDrawer.statusText = "Starting..."
            progressDrawer.visible = true
        }
        function onTransactionProgress(pkgId, percent, step) {
            progressDrawer.pkgName = pkgId
            progressDrawer.percent = percent
            progressDrawer.statusText = step
        }
        function onTransactionFinished(pkgId, success, error) {
            if (success) {
                progressDrawer.percent = 100
                progressDrawer.statusText = "Complete"
                hideTimer.start()
                if (root.lastTransactionWasRemove && root.lastRemoveSource === 0) {
                    root.lastTransactionWasRemove = false
                    pacmanBackend.checkOrphans()
                } else {
                    root.lastTransactionWasRemove = false
                }
            } else {
                progressDrawer.isError = true
                progressDrawer.statusText = error || "Failed"
            }
        }
        function onSyncStarted() {
            progressDrawer.pkgName = ""
            progressDrawer.isError = false
            progressDrawer.percent = 0
            progressDrawer.statusText = "Refreshing databases..."
            progressDrawer.visible = true
        }
        function onSyncProgress(percent, step) {
            progressDrawer.percent = percent
            progressDrawer.statusText = step
        }
        function onSyncFinished(success, error) {
            if (success) {
                progressDrawer.percent = 100
                if (root.waitingForReasonFix) {
                    progressDrawer.statusText = "Install reasons repaired"
                    root.waitingForReasonFix = false
                    pacmanBackend.markReasonRepairDone()
                } else {
                    progressDrawer.statusText = "Databases up to date"
                }
                hideTimer.start()

                pacmanBackend.checkUpdates()
                aurBackend.checkUpdates()
            } else {
                progressDrawer.isError = true
                progressDrawer.statusText = error || "Sync failed"
                if (root.waitingForReasonFix) {
                    root.waitingForReasonFix = false
                    pacmanBackend.markReasonRepairDone()
                }
            }
        }
        function onUpgradeStarted() {
            progressDrawer.pkgName = ""
            progressDrawer.isError = false
            progressDrawer.percent = 0
            progressDrawer.statusText = "Preparing upgrade..."
            progressDrawer.visible = true
        }
        function onUpgradeProgress(percent, step) {
            progressDrawer.percent = percent
            progressDrawer.statusText = step
        }
        function onUpgradeFinished(success, error) {
            if (success) {
                progressDrawer.percent = 100
                progressDrawer.statusText = "Upgrade complete"
                hideTimer.start()
                pacmanBackend.checkUpdates()
                aurBackend.checkUpdates()
            } else {
                progressDrawer.isError = true
                progressDrawer.statusText = error || "Upgrade failed"
            }
        }
    }

    ProgressDrawer {
        id: progressDrawer
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: false
    }

    Timer {
        id: hideTimer
        interval: 2000
        onTriggered: progressDrawer.visible = false
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

    Component.onCompleted: {
        transactionManager.syncDatabases()
        if (pacmanBackend.isReasonRepairNeeded())
            pacmanBackend.checkDirtyReasons()
    }
}