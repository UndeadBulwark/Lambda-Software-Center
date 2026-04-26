#include "PacmanBackend.h"
#include "AlpmWrapper.h"
#include "TransactionManager.h"
#include <QTimer>

PacmanBackend::PacmanBackend(QObject *parent)
    : IPackageBackend(parent)
    , m_alpm(std::make_unique<AlpmWrapper>())
{
}

PacmanBackend::~PacmanBackend() = default;

void PacmanBackend::setTransactionManager(TransactionManager *tm) {
    if (m_tm) {
        disconnect(m_tm, nullptr, this, nullptr);
    }
    m_tm = tm;
    if (m_tm) {
        connect(m_tm, &TransactionManager::transactionProgress,
                this, [this](const QString &pkgId, int percent, const QString &step) {
                    if (m_isRemove)
                        emit removeProgress(pkgId, percent, step);
                    else
                        emit installProgress(pkgId, percent, step);
                });
        connect(m_tm, &TransactionManager::transactionFinished,
                this, [this](const QString &pkgId, bool success, const QString &error) {
                    QString id = m_pendingPkgId.isEmpty() ? pkgId : m_pendingPkgId;
                    m_pendingPkgId.clear();
                    if (m_isRemove) {
                        m_isRemove = false;
                        if (success)
                            emit removeFinished(id, true, QString());
                        else
                            emit removeFinished(id, false, error);
                    } else {
                        if (success)
                            emit installFinished(id, true, QString());
                        else
                            emit installFinished(id, false, error);
                    }
                });
    }
}

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
    m_isRemove = false;
    m_pendingPkgId = pkgId;
    if (m_tm) {
        m_tm->install(pkgId, static_cast<int>(Package::Source::Pacman));
    } else {
        emit installFinished(pkgId, false, QStringLiteral("No transaction manager"));
    }
}

void PacmanBackend::remove(const QString &pkgId) {
    m_isRemove = true;
    m_pendingPkgId = pkgId;
    if (m_tm) {
        m_tm->remove(pkgId, static_cast<int>(Package::Source::Pacman));
    } else {
        emit removeFinished(pkgId, false, QStringLiteral("No transaction manager"));
    }
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

void PacmanBackend::checkOrphans() {
    if (m_testForceUninitialized || !m_alpm->initialize()) {
        emit orphansFound(QStringList());
        return;
    }
    QStringList orphans = m_alpm->findOrphans();
    emit orphansFound(orphans);
}

void PacmanBackend::checkDirtyReasons() {
    if (m_testForceUninitialized || !m_alpm->initialize()) {
        emit dirtyReasonsFound(QStringList());
        return;
    }
    QStringList dirty = m_alpm->findDirtyReasons();
    emit dirtyReasonsFound(dirty);
}

void PacmanBackend::markReasonRepairDone() {
    m_alpm->markReasonRepairDone();
}

bool PacmanBackend::isReasonRepairNeeded() const {
    return m_alpm->isReasonRepairNeeded();
}

#ifdef QT_TESTLIB_LIB
void PacmanBackend::forceUninitializedState() {
    m_testForceUninitialized = true;
}
#endif