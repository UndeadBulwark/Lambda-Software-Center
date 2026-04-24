#include "PacmanBackend.h"
#include "AlpmWrapper.h"
#include <QTimer>

PacmanBackend::PacmanBackend(QObject *parent)
    : IPackageBackend(parent)
    , m_alpm(std::make_unique<AlpmWrapper>())
{
}

PacmanBackend::~PacmanBackend() = default;

void PacmanBackend::search(const QString &query) {
    QTimer::singleShot(0, this, [this, query]() {
        if (m_testForceUninitialized || !m_alpm->initialize()) {
            emit errorOccurred(m_testForceUninitialized
                               ? QStringLiteral("Test: forced uninitialized state")
                               : m_alpm->lastError());
            return;
        }
        QList<Package> results = m_alpm->search(query);
        emit searchResultsReady(results);
    });
}

void PacmanBackend::install(const QString &pkgId) {
    Q_UNUSED(pkgId)
    emit installFinished(pkgId, false, "Install not yet implemented for pacman");
}

void PacmanBackend::remove(const QString &pkgId) {
    Q_UNUSED(pkgId)
    emit removeFinished(pkgId, false, "Remove not yet implemented for pacman");
}

QList<Package> PacmanBackend::listInstalled() {
    if (!m_alpm->initialize()) {
        emit errorOccurred(m_alpm->lastError());
        return {};
    }
    QList<Package> results = m_alpm->listInstalled();
    emit installedListReady(results);
    return results;
}

QList<Package> PacmanBackend::checkUpdates() {
    if (!m_alpm->initialize()) {
        emit errorOccurred(m_alpm->lastError());
        return {};
    }
    QList<Package> updates = m_alpm->checkUpdates();
    emit updatesReady(updates);
    return updates;
}

Package::Source PacmanBackend::source() const {
    return Package::Source::Pacman;
}

#ifdef QT_TESTLIB_LIB
void PacmanBackend::forceUninitializedState() {
    m_testForceUninitialized = true;
}
#endif
