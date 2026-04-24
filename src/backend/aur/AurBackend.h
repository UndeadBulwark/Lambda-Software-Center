#ifndef AURBACKEND_H
#define AURBACKEND_H

#include "IPackageBackend.h"
#include <memory>

class AurClient;

class AurBackend : public IPackageBackend {
    Q_OBJECT
public:
    explicit AurBackend(QObject *parent = nullptr);
    ~AurBackend();

    void search(const QString &query) override;
    void install(const QString &pkgId) override;
    void remove(const QString &pkgId) override;
    QList<Package> listInstalled() override;
    QList<Package> checkUpdates() override;
    Package::Source source() const override;

private:
    std::unique_ptr<AurClient> m_client;
};

#endif // AURBACKEND_H
