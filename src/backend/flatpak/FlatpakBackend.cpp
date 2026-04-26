#include "FlatpakBackend.h"
#include "PackageSearchUtils.h"
#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lscFlatpak, "lsc.flatpak")

FlatpakBackend::FlatpakBackend(QObject *parent)
    : IPackageBackend(parent)
{
}

void FlatpakBackend::search(const QString &query) {
    Q_UNUSED(query)
    // TODO: replace with libflatpak
    QTimer::singleShot(0, this, [this]() {
        emit searchResultsReady(QList<Package>());
    });
}

void FlatpakBackend::install(const QString &pkgId) {
    Q_UNUSED(pkgId)
    emit installFinished(pkgId, false, "Install not yet implemented for Flatpak");
}

void FlatpakBackend::remove(const QString &pkgId) {
    Q_UNUSED(pkgId)
    emit removeFinished(pkgId, false, "Remove not yet implemented for Flatpak");
}

QList<Package> FlatpakBackend::listInstalled() {
    QList<Package> results;
    emit installedListReady(results);
    return results;
}

QList<Package> FlatpakBackend::checkUpdates() {
    QList<Package> results;
    emit updatesReady(results);
    return results;
}

Package::Source FlatpakBackend::source() const {
    return Package::Source::Flatpak;
}

bool FlatpakBackend::isInitialized() const {
    return true;
}
