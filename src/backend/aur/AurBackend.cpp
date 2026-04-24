#include "AurBackend.h"
#include "AurClient.h"

AurBackend::AurBackend(QObject *parent)
    : IPackageBackend(parent)
    , m_client(std::make_unique<AurClient>())
{
    connect(m_client.get(), &AurClient::searchFinished,
            this, &AurBackend::searchResultsReady);
    connect(m_client.get(), &AurClient::errorOccurred,
            this, &AurBackend::errorOccurred);
}

AurBackend::~AurBackend() = default;

void AurBackend::search(const QString &query) {
    m_client->search(query);
}

void AurBackend::install(const QString &pkgId) {
    Q_UNUSED(pkgId)
    emit installFinished(pkgId, false, "Install not yet implemented for AUR");
}

void AurBackend::remove(const QString &pkgId) {
    Q_UNUSED(pkgId)
    emit removeFinished(pkgId, false, "Remove not yet implemented for AUR");
}

QList<Package> AurBackend::listInstalled() {
    QList<Package> results;
    emit installedListReady(results);
    return results;
}

QList<Package> AurBackend::checkUpdates() {
    QList<Package> results;
    emit updatesReady(results);
    return results;
}

Package::Source AurBackend::source() const {
    return Package::Source::AUR;
}
