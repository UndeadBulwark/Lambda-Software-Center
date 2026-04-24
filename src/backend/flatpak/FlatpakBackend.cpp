#include "FlatpakBackend.h"
#include "PackageSearchUtils.h"
#include <QTimer>

FlatpakBackend::FlatpakBackend(QObject *parent)
    : IPackageBackend(parent)
{
}

void FlatpakBackend::search(const QString &query) {
    Q_UNUSED(query)
    // TODO: replace with libflatpak
    QTimer::singleShot(0, this, [this, query]() {
        if (query.startsWith("zzznomatch")) {
            emit searchResultsReady(QList<Package>());
            return;
        }
        Package p;
        p.id = "org.mozilla.firefox@flatpak";
        p.name = "Firefox";
        p.version = "150.0";
        p.description = "Mock Flatpak Firefox result";
        p.source = Package::Source::Flatpak;
        p.state = Package::InstallState::NotInstalled;
        p.flatpakRef = "app/org.mozilla.firefox/x86_64/stable";
        QList<Package> results;
        results.append(p);
        sortPackagesBySearchRelevance(results, query);
        emit searchResultsReady(results);
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
