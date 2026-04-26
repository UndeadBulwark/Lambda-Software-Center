#ifndef IPACKAGEBACKEND_H
#define IPACKAGEBACKEND_H

#include <QObject>
#include <QList>
#include <QStringList>
#include "Package.h"

class IPackageBackend : public QObject {
    Q_OBJECT
protected:
    explicit IPackageBackend(QObject *parent = nullptr) : QObject(parent) {}
public:
    virtual ~IPackageBackend() = default;

    Q_INVOKABLE virtual void search(const QString &query) = 0;
    Q_INVOKABLE virtual void install(const QString &pkgId) = 0;
    Q_INVOKABLE virtual void remove(const QString &pkgId) = 0;
    Q_INVOKABLE virtual QList<Package> listInstalled() = 0;
    Q_INVOKABLE virtual QList<Package> checkUpdates() = 0;
    Q_INVOKABLE virtual Package::Source source() const = 0;

signals:
    void searchResultsReady(QList<Package> results);
    void installProgress(const QString &pkgId, int percent, const QString &step);
    void installFinished(const QString &pkgId, bool success, const QString &error);
    void removeFinished(const QString &pkgId, bool success, const QString &error);
    void installedListReady(QList<Package> packages);
    void updatesReady(QList<Package> updates);
    void errorOccurred(const QString &message);
    void orphansFound(QStringList orphans);
    void dirtyReasonsFound(QStringList packages);
    void pkgbuildReady(const QString &pkgName, const QString &content);
};

#endif // IPACKAGEBACKEND_H
