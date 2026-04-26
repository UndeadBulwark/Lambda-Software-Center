# main.qml — additions needed to wire Remove

## In the onInstallConfirmed handler block, add a parallel handler:

    Connections {
        target: confirmDialog
        function onRemoveConfirmed() {
            progressDrawer.packageName = confirmDialog.packageName
            progressDrawer.statusText  = "Removing..."
            progressDrawer.percent     = 0
            progressDrawer.isError     = false
            progressDrawer.visible     = true
            mockTransaction.restart()
        }
    }

## Or, if you wired installConfirmed inline rather than with Connections,
## add the equivalent onRemoveConfirmed signal handler on the ConfirmDialog
## declaration:

    ConfirmDialog {
        id: confirmDialog
        // ... existing properties ...
        onInstallConfirmed: { /* existing */ }
        onRemoveConfirmed: {
            progressDrawer.packageName = packageName
            progressDrawer.statusText  = "Removing..."
            progressDrawer.percent     = 0
            progressDrawer.isError     = false
            progressDrawer.visible     = true
            mockTransaction.restart()
        }
    }

## Note: the mock timer flow (Download → Verify → Install → Complete) will
## also fire for Remove. You can differentiate the status text labels later
## when TransactionManager is wired. For now it is functionally correct.
