import QtQuick
import QtQuick.Controls
import LambdaSoftwareCenter

Rectangle {
    id: browsePage
    color: Theme.bgPrimary

    property string searchQuery: ""
    property int sourceFilter: -1
    signal packageClicked(var pkg)
    signal updateAllRequested()

    onSearchQueryChanged: {
        if (searchQuery.length < 2) {
            searchModel.clear();
            return;
        }
        searchModel.clear();
        if (sourceFilter === -1 || sourceFilter === 0)
            pacmanBackend.search(searchQuery);
        if (sourceFilter === -1 || sourceFilter === 1)
            aurBackend.search(searchQuery);
        if (sourceFilter === -1 || sourceFilter === 2)
            flatpakBackend.search(searchQuery);
    }

    onSourceFilterChanged: {
        if (searchQuery.length >= 2) {
            searchModel.clear();
            if (sourceFilter === -1 || sourceFilter === 0)
                pacmanBackend.search(searchQuery);
            if (sourceFilter === -1 || sourceFilter === 1)
                aurBackend.search(searchQuery);
            if (sourceFilter === -1 || sourceFilter === 2)
                flatpakBackend.search(searchQuery);
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.contentPadding
        spacing: 0

        UpdatesBanner {
            id: banner
            width: parent.width
            visible: updateCount > 0
            onUpdateAllClicked: browsePage.updateAllRequested()
        }

        GridView {
            id: grid
            width: parent.width
            height: parent.height - (banner.visible ? banner.height + 14 : 0)
            cellWidth: Math.floor(width / 3)
            cellHeight: 160
            clip: true
            model: searchModel

            delegate: PackageCard {
                width: grid.cellWidth - 6
                height: grid.cellHeight - 10
                anchors.margins: 3
                onPackageClicked: function(pkg) { browsePage.packageClicked(pkg) }
            }

            Text {
                anchors.centerIn: parent
                text: searchQuery.length < 2 ? "Type at least 2 characters to search" : "No packages found"
                font.pixelSize: 13
                color: Theme.textTertiary
                visible: grid.count === 0
            }

            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
