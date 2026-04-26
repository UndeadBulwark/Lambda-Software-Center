#ifndef PACMANBACKEND_H
#define PACMANBACKEND_H

#include "IPackageBackend.h"
#include <memory>

class AlpmWrapper;
class TransactionManager;

class PacmanBackend : public IPackageBackend {
    Q_OBJECT
public:
    explicit PacmanBackend(QObject *parent = nullptr);
    ~PacmanBackend();

    void setTransactionManager(TransactionManager *tm);

    void search(const QString &query) override;
    void install(const QString &pkgId) override;
    void remove(const QString &pkgId) override;
    QList<Package> listInstalled() override;
    QList<Package> checkUpdates() override;
    Package::Source source() const override;

    Q_INVOKABLE void checkOrphans();

    Q_INVOKABLE void checkDirtyReasons();
    Q_INVOKABLE void markReasonRepairDone();
    Q_INVOKABLE bool isReasonRepairNeeded() const;

#ifdef QT_TESTLIB_LIB
    void forceUninitializedState();
#endif

private:
    std::unique_ptr<AlpmWrapper> m_alpm;
    TransactionManager *m_tm = nullptr;
    bool m_testForceUninitialized = false;
};

#endif // PACMANBACKEND_H
