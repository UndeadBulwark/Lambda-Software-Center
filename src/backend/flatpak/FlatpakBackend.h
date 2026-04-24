#ifndef FLATPAKBACKEND_H
#define FLATPAKBACKEND_H

#include "IPackageBackend.h"

class FlatpakBackend : public IPackageBackend {
    Q_OBJECT
public:
    explicit FlatpakBackend(QObject *parent = nullptr);

    void search(const QString &query) override;
    void install(const QString &pkgId) override;
    void remove(const QString &pkgId) override;
    QList<Package> listInstalled() override;
    QList<Package> checkUpdates() override;
    Package::Source source() const override;

    bool isInitialized() const;
};

#endif // FLATPAKBACKEND_H
