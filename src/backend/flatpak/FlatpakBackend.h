#ifndef FLATPAKBACKEND_H
#define FLATPAKBACKEND_H

#include "IPackageBackend.h"
#include <QHash>
#include <QMutex>

typedef struct _FlatpakInstallation FlatpakInstallation;

class FlatpakBackend : public IPackageBackend {
    Q_OBJECT
public:
    explicit FlatpakBackend(QObject *parent = nullptr);
    ~FlatpakBackend();

    void search(const QString &query) override;
    void install(const QString &pkgId) override;
    void remove(const QString &pkgId) override;
    QList<Package> listInstalled() override;
    QList<Package> checkUpdates() override;
    Package::Source source() const override;

    bool isInitialized() const;
    void setTransactionManager(class TransactionManager *tm);

private:
    bool ensureInstallation();
    QList<Package> listRemoteRefs();
    QList<Package> listInstalledRefs();
    QList<Package> listUpdateRefs();
    QString findRemoteForRef(const QString &ref);

    FlatpakInstallation *m_installation = nullptr;
    class TransactionManager *m_tm = nullptr;
    QMutex m_mutex;

    QList<Package> m_cachedRemoteRefs;
    bool m_remoteRefsCached = false;
    QHash<QString, QString> m_refToRemote;
    QHash<QString, Package> m_installedCache;
    bool m_installedCacheDirty = true;

    QString m_pendingInstallRef;
    QString m_pendingRemoveRef;

    void doSearch(const QString &query);
    void doListInstalled();
    void doCheckUpdates();
    void doInstall(const QString &ref);
    void doRemove(const QString &ref);
};

#endif // FLATPAKBACKEND_H