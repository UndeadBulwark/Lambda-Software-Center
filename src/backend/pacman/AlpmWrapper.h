#ifndef ALPMWRAPPER_H
#define ALPMWRAPPER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "Package.h"

extern "C" {
#include <alpm.h>
}

class AlpmWrapper : public QObject {
    Q_OBJECT
public:
    explicit AlpmWrapper(const QString &root = QStringLiteral("/"),
                         const QString &dbpath = QStringLiteral("/var/lib/pacman"),
                         QObject *parent = nullptr);
    ~AlpmWrapper();

    bool initialize();
    bool isInitialized() const;
    QString lastError() const;

    QList<Package> search(const QString &query);
    QList<Package> listInstalled();
    QList<Package> checkUpdates();
    QList<Package> listForeignPackages();
    QStringList findOrphans();
    QStringList findDirtyReasons();
    bool isReasonRepairNeeded() const;
    void markReasonRepairDone();

#ifdef QT_TESTLIB_LIB
    void forceUninitializedState();
#endif

private:
    alpm_handle_t *m_handle;
    QString m_root;
    QString m_dbpath;
    QString m_lastError;

    Package alpmPkgToPackage(alpm_pkg_t *pkg) const;
    QString formatSize(off_t size) const;
    QStringList readPacmanConfRepoOrder() const;
};

#endif // ALPMWRAPPER_H
