#ifndef PACMANBACKEND_H
#define PACMANBACKEND_H

#include "IPackageBackend.h"
#include <memory>

class AlpmWrapper;

class PacmanBackend : public IPackageBackend {
    Q_OBJECT
public:
    explicit PacmanBackend(QObject *parent = nullptr);
    ~PacmanBackend();

    void search(const QString &query) override;
    void install(const QString &pkgId) override;
    void remove(const QString &pkgId) override;
    QList<Package> listInstalled() override;
    QList<Package> checkUpdates() override;
    Package::Source source() const override;

#ifdef QT_TESTLIB_LIB
    void forceUninitializedState();
#endif

private:
    std::unique_ptr<AlpmWrapper> m_alpm;
    bool m_testForceUninitialized = false;
};

#endif // PACMANBACKEND_H
