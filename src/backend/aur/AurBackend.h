#ifndef AURBACKEND_H
#define AURBACKEND_H

#include "IPackageBackend.h"
#include <memory>
#include <QHash>

class AlpmWrapper;
class AurClient;
class AurBuilder;
class TransactionManager;

class AurBackend : public IPackageBackend {
    Q_OBJECT
public:
    explicit AurBackend(QObject *parent = nullptr);
    ~AurBackend();

    void setTransactionManager(TransactionManager *tm);

    void search(const QString &query) override;
    void install(const QString &pkgId) override;
    void remove(const QString &pkgId) override;
    QList<Package> listInstalled() override;
    QList<Package> checkUpdates() override;
    Package::Source source() const override;

    Q_INVOKABLE void continueBuild(const QString &pkgName);
    Q_INVOKABLE void cancelBuild(const QString &pkgName);

private:
    std::unique_ptr<AurClient> m_client;
    std::unique_ptr<AurBuilder> m_builder;
    std::unique_ptr<AlpmWrapper> m_alpm;
    TransactionManager *m_tm = nullptr;

    QHash<QString, Package> m_searchCache;
    QString m_pendingInstallPkgId;
    QString m_pendingInstallPkgName;
    QString m_pendingRemovePkgId;
    bool m_isRemove = false;

    QHash<QString, QString> m_foreignPkgVersions;

    bool m_checkingUpdates = false;

    QString stripPkgName(const QString &pkgId) const;
    void onBuildProgress(const QString &pkgName, int percent, const QString &step);
    void onBuildFinished(const QString &pkgName, bool success, const QString &filepath);
    void onSearchResults(QList<Package> results);
    void onAurInfoResults(QList<Package> results);
};

#endif // AURBACKEND_H