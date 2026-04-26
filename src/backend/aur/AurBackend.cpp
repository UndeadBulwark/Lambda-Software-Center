#include "AurBackend.h"
#include "AurClient.h"
#include "AurBuilder.h"
#include "AlpmWrapper.h"
#include "TransactionManager.h"
#include <QLoggingCategory>
#include <QDir>
#include <QStandardPaths>
#include <QSet>
#include <algorithm>

Q_LOGGING_CATEGORY(lscAur, "lsc.aur")

static bool versionLessThan(const QString &a, const QString &b) {
    auto isDotOrDash = [](QChar c) { return c == '.' || c == '-'; };
    auto split = [&](const QString &s) {
        QStringList parts;
        QString current;
        for (int i = 0; i < s.size(); ++i) {
            if (isDotOrDash(s[i])) {
                if (!current.isEmpty())
                    parts.append(current);
                current.clear();
            } else {
                current += s[i];
            }
        }
        if (!current.isEmpty())
            parts.append(current);
        return parts;
    };

    QStringList pa = split(a);
    QStringList pb = split(b);

    for (int i = 0; i < std::max(pa.size(), pb.size()); ++i) {
        bool okA = false, okB = false;
        int na = (i < pa.size()) ? pa[i].toInt(&okA) : 0;
        int nb = (i < pb.size()) ? pb[i].toInt(&okB) : 0;

        if (okA && okB) {
            if (na != nb) return na < nb;
        } else {
            QString sa = (i < pa.size()) ? pa[i] : QString();
            QString sb = (i < pb.size()) ? pb[i] : QString();
            if (sa != sb) return sa < sb;
        }
    }
    return false;
}

AurBackend::AurBackend(QObject *parent)
    : IPackageBackend(parent)
    , m_client(std::make_unique<AurClient>())
    , m_builder(std::make_unique<AurBuilder>())
    , m_alpm(std::make_unique<AlpmWrapper>())
{
    connect(m_client.get(), &AurClient::searchFinished,
            this, &AurBackend::onSearchResults);
    connect(m_client.get(), &AurClient::infoFinished,
            this, &AurBackend::onAurInfoResults);
    connect(m_client.get(), &AurClient::errorOccurred,
            this, &AurBackend::errorOccurred);

    connect(m_builder.get(), &AurBuilder::buildProgress,
            this, &AurBackend::onBuildProgress);
    connect(m_builder.get(), &AurBuilder::buildFinished,
            this, &AurBackend::onBuildFinished);
    connect(m_builder.get(), &AurBuilder::pkgbuildReady,
            this, &AurBackend::pkgbuildReady);
}

AurBackend::~AurBackend() = default;

void AurBackend::setTransactionManager(TransactionManager *tm) {
    if (m_tm) {
        disconnect(m_tm, nullptr, this, nullptr);
    }
    m_tm = tm;
    if (m_tm) {
        connect(m_tm, &TransactionManager::transactionProgress,
                this, [this](const QString &pkgId, int percent, const QString &step) {
                    Q_UNUSED(pkgId)
                    if (m_isRemove && !m_pendingRemovePkgId.isEmpty())
                        emit removeProgress(m_pendingRemovePkgId, percent, step);
                    else if (!m_pendingInstallPkgId.isEmpty())
                        emit installProgress(m_pendingInstallPkgId, percent, step);
                });
        connect(m_tm, &TransactionManager::transactionFinished,
                this, [this](const QString &pkgId, bool success, const QString &error) {
                    Q_UNUSED(pkgId)
                    if (m_isRemove) {
                        QString id = m_pendingRemovePkgId;
                        m_pendingRemovePkgId.clear();
                        m_isRemove = false;
                        if (success)
                            emit removeFinished(id, true, QString());
                        else
                            emit removeFinished(id, false, error);
                    } else if (!m_pendingInstallPkgId.isEmpty()) {
                        QString id = m_pendingInstallPkgId;
                        QString name = m_pendingInstallPkgName;
                        m_pendingInstallPkgId.clear();
                        m_pendingInstallPkgName.clear();

                        if (success) {
                            QString buildDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                + QStringLiteral("/aur/%1").arg(name);
                            QDir(buildDir).removeRecursively();
                        }

                        emit installFinished(id, success, error);
                    }
                });
    }
}

void AurBackend::search(const QString &query) {
    m_client->search(query);
}

void AurBackend::onSearchResults(QList<Package> results) {
    for (const Package &p : results) {
        m_searchCache.insert(p.name, p);
    }
    emit searchResultsReady(results);
}

void AurBackend::install(const QString &pkgId) {
    m_isRemove = false;
    QString pkgName = stripPkgName(pkgId);

    if (m_searchCache.contains(pkgName)) {
        const Package &pkg = m_searchCache.value(pkgName);
        if (pkg.gitUrl.isEmpty()) {
            emit installFinished(pkgId, false, QStringLiteral("No git URL available for %1").arg(pkgName));
            return;
        }
        m_pendingInstallPkgId = pkgId;
        m_pendingInstallPkgName = pkgName;
        m_builder->gitClone(pkgName, pkg.gitUrl);
    } else {
        emit installFinished(pkgId, false, QStringLiteral("Package %1 not found in search cache — search again first").arg(pkgName));
    }
}

void AurBackend::remove(const QString &pkgId) {
    m_isRemove = true;
    m_pendingRemovePkgId = pkgId;
    if (m_tm) {
        m_tm->remove(pkgId, static_cast<int>(Package::Source::AUR));
    } else {
        emit removeFinished(pkgId, false, QStringLiteral("No transaction manager"));
    }
}

QList<Package> AurBackend::listInstalled() {
    QList<Package> results;
    emit installedListReady(results);
    return results;
}

QList<Package> AurBackend::checkUpdates() {
    if (!m_alpm->initialize()) {
        emit updatesReady(QList<Package>());
        return {};
    }

    QList<Package> foreignPkgs = m_alpm->listForeignPackages();
    if (foreignPkgs.isEmpty()) {
        emit updatesReady(QList<Package>());
        return {};
    }

    m_foreignPkgVersions.clear();
    QStringList names;
    for (const Package &p : foreignPkgs) {
        m_foreignPkgVersions.insert(p.name, p.version);
        names.append(p.name);
        m_searchCache.insert(p.name, p);
    }

    m_checkingUpdates = true;
    m_client->info(names);

    return {};
}

void AurBackend::onAurInfoResults(QList<Package> results) {
    if (!m_checkingUpdates) return;

    m_checkingUpdates = false;

    QList<Package> updates;
    for (const Package &aurPkg : results) {
        QString localVersion = m_foreignPkgVersions.value(aurPkg.name);
        if (localVersion.isEmpty()) continue;

        if (versionLessThan(localVersion, aurPkg.version)) {
            Package p = aurPkg;
            p.state = Package::InstallState::UpdateAvailable;
            p.version = localVersion;
            p.newVersion = aurPkg.version;
            m_searchCache.insert(p.name, p);
            updates.append(p);
        }
    }

    qCDebug(lscAur) << "AUR updates available:" << updates.size();
    emit updatesReady(updates);
}

Package::Source AurBackend::source() const {
    return Package::Source::AUR;
}

void AurBackend::continueBuild(const QString &pkgName) {
    QString buildDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/aur/%1").arg(pkgName);
    m_builder->makepkg(pkgName, buildDir);
}

void AurBackend::cancelBuild(const QString &pkgName) {
    Q_UNUSED(pkgName)
    m_builder->cancelBuild();

    if (!m_pendingInstallPkgId.isEmpty()) {
        QString id = m_pendingInstallPkgId;
        m_pendingInstallPkgId.clear();
        m_pendingInstallPkgName.clear();
        emit installFinished(id, false, QStringLiteral("Cancelled by user"));
    }
}

QString AurBackend::stripPkgName(const QString &pkgId) const {
    int atPos = pkgId.lastIndexOf(QLatin1Char('@'));
    if (atPos > 0)
        return pkgId.left(atPos);
    return pkgId;
}

void AurBackend::onBuildProgress(const QString &pkgName, int percent, const QString &step) {
    Q_UNUSED(pkgName)
    if (!m_pendingInstallPkgId.isEmpty())
        emit installProgress(m_pendingInstallPkgId, percent, step);
}

void AurBackend::onBuildFinished(const QString &pkgName, bool success, const QString &filepath) {
    Q_UNUSED(pkgName)

    if (!success) {
        if (!m_pendingInstallPkgId.isEmpty()) {
            QString id = m_pendingInstallPkgId;
            m_pendingInstallPkgId.clear();
            m_pendingInstallPkgName.clear();
            emit installFinished(id, false, filepath);
        }
        return;
    }

    if (m_tm) {
        m_tm->installLocal(filepath);
    } else {
        if (!m_pendingInstallPkgId.isEmpty()) {
            QString id = m_pendingInstallPkgId;
            m_pendingInstallPkgId.clear();
            m_pendingInstallPkgName.clear();
            emit installFinished(id, false, QStringLiteral("No transaction manager"));
        }
    }
}