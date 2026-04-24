import QtQuick
import QtQuick.Controls
import LambdaSoftwareCenter

Rectangle {
    id: browsePage
    color: Theme.bgPrimary

    property string searchQuery: ""
    property int sourceFilter: -1

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

    GridView {
        id: grid
        anchors.fill: parent
        anchors.margins: 20
        cellWidth: Math.floor(width / 3)
        cellHeight: 160
        clip: true
        model: searchModel

        delegate: PackageCard {
            width: grid.cellWidth - 6
            height: grid.cellHeight - 10
            anchors.margins: 3
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
