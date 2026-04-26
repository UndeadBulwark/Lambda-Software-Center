# ConfirmDialog.qml — additions needed for Remove mode

## Add these two properties near the top of ConfirmDialog

    property string mode: "install"   // "install" | "remove"
    signal removeConfirmed()

## Change the primary button's onClicked handler

    // Before:
    onClicked: {
        visible = false
        installConfirmed()
    }

    // After:
    onClicked: {
        visible = false
        if (mode === "remove") removeConfirmed()
        else installConfirmed()
    }

## Change the primary button's label Text

    // Before:
    text: "Install"

    // After:
    text: mode === "remove" ? "Remove" : "Install"

## Optionally hide the dependency list and size rows when mode === "remove"
## Wrap those sections in: visible: mode === "install"
