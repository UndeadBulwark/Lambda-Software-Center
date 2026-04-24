#ifndef IPACKAGEBACKEND_H
#define IPACKAGEBACKEND_H

#include <QObject>
#include <QList>
#include "Package.h"

class IPackageBackend : public QObject {
    Q_OBJECT
protected:
    explicit IPackageBackend(QObject *parent = nullptr) : QObject(parent) {}
public:
    virtual ~IPackageBackend() = default;

    virtual void search(const QString &query) = 0;
    virtual void install(const QString &pkgId) = 0;
    virtual void remove(const QString &pkgId) = 0;
    virtual QList<Package> listInstalled() = 0;
    virtual QList<Package> checkUpdates() = 0;
    virtual Package::Source source() const = 0;

signals:
    void searchResultsReady(QList<Package> results);
    void installProgress(const QString &pkgId, int percent, const QString &step);
    void installFinished(const QString &pkgId, bool success, const QString &error);
    void removeFinished(const QString &pkgId, bool success, const QString &error);
    void installedListReady(QList<Package> packages);
    void updatesReady(QList<Package> updates);
    void errorOccurred(const QString &message);
};

#endif // IPACKAGEBACKEND_H
