#include "AlpmWrapper.h"
#include "PackageSearchUtils.h"
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QSet>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(lscPacman, "lsc.pacman")

AlpmWrapper::AlpmWrapper(const QString &root, const QString &dbpath, QObject *parent)
    : QObject(parent)
    , m_handle(nullptr)
    , m_root(root)
    , m_dbpath(dbpath)
{
    initialize(); // Eager initialization per test contract
}

AlpmWrapper::~AlpmWrapper() {
    if (m_handle) {
        alpm_release(m_handle);
        m_handle = nullptr;
    }
}

bool AlpmWrapper::initialize() {
    if (!m_handle) {
        QDir rootDir(m_root);
        if (!rootDir.exists() && !rootDir.mkpath(".")) {
            m_lastError = QString("Failed to create root directory: %1").arg(m_root);
            qCWarning(lscPacman) << m_lastError;
            return false;
        }
        QDir dbDir(m_dbpath);
        if (!dbDir.exists() && !dbDir.mkpath(".")) {
            m_lastError = QString("Failed to create dbpath directory: %1").arg(m_dbpath);
            qCWarning(lscPacman) << m_lastError;
            return false;
        }

        QDir localDir(m_dbpath + QStringLiteral("/local"));
        if (!localDir.exists()) {
            localDir.mkpath(".");
        }

        QDir sandboxSyncDir(m_dbpath + QStringLiteral("/sync"));
        if (sandboxSyncDir.entryList({"*.db"}, QDir::Files).isEmpty()) {
            QDir realSyncDir("/var/lib/pacman/sync");
            QStringList dbs = realSyncDir.entryList({"*.db"}, QDir::Files);
            if (!dbs.isEmpty()) {
                sandboxSyncDir.mkpath(".");
                for (const QString &dbFile : dbs) {
                    QFile::copy(realSyncDir.absoluteFilePath(dbFile),
                                sandboxSyncDir.absoluteFilePath(dbFile));
                }
                qCDebug(lscPacman) << "copied" << dbs.size() << "sync DBs to sandbox";
            }
        }

        alpm_errno_t err = ALPM_ERR_OK;
        m_handle = alpm_initialize(m_root.toUtf8().constData(),
                                     m_dbpath.toUtf8().constData(),
                                     &err);
        if (!m_handle) {
            m_lastError = QString("Failed to initialize alpm: %1").arg(alpm_strerror(err));
            qCWarning(lscPacman) << m_lastError;
            return false;
        }
        qCDebug(lscPacman) << "alpm initialized: root=" << m_root << "dbpath=" << m_dbpath;
    }

    if (m_handle) {
        alpm_unregister_all_syncdbs(m_handle);
    }

    QDir syncDir(m_dbpath + QStringLiteral("/sync"));
    QStringList allDbFiles = syncDir.entryList({"*.db"}, QDir::Files);
    for (auto &name : allDbFiles)
        name.chop(3);

    QStringList repoOrder;
    bool useConfOrder = (m_root == QStringLiteral("/") && QFile::exists(QStringLiteral("/etc/pacman.conf")));
    if (useConfOrder) {
        repoOrder = readPacmanConfRepoOrder();
    }

    int dbCount = 0;
    QSet<QString> registered;

    // Register DBs in pacman.conf order first
    if (useConfOrder) {
        for (const QString &repoName : repoOrder) {
            if (!allDbFiles.contains(repoName))
                continue;
            if (registered.contains(repoName))
                continue;
            alpm_db_t *db = alpm_register_syncdb(m_handle, repoName.toUtf8().constData(),
                                                  ALPM_SIG_DATABASE_OPTIONAL);
            if (!db) {
                qCWarning(lscPacman) << "failed to register sync db:" << repoName
                         << alpm_strerror(alpm_errno(m_handle));
            } else {
                registered.insert(repoName);
                dbCount++;
            }
        }
    }

    // Register any remaining DBs not in pacman.conf (or fallback alphabetical order)
    for (const QString &dbName : allDbFiles) {
        if (registered.contains(dbName))
            continue;
        alpm_db_t *db = alpm_register_syncdb(m_handle, dbName.toUtf8().constData(),
                                              ALPM_SIG_DATABASE_OPTIONAL);
        if (!db) {
            qCWarning(lscPacman) << "failed to register sync db:" << dbName
                     << alpm_strerror(alpm_errno(m_handle));
        } else {
            dbCount++;
        }
    }

    // Log registration order
    {
        QStringList orderNames;
        alpm_list_t *dbs = alpm_get_syncdbs(m_handle);
        for (alpm_list_t *i = dbs; i; i = alpm_list_next(i)) {
            alpm_db_t *db = static_cast<alpm_db_t*>(i->data);
            orderNames.append(QString::fromUtf8(alpm_db_get_name(db)));
        }
        qCDebug(lscPacman) << "DB registration order:" << orderNames.join(", ");
    }

    qCDebug(lscPacman) << "registered" << dbCount << "sync databases";

    return true;
}

#ifdef QT_TESTLIB_LIB
void AlpmWrapper::forceUninitializedState() {
    if (m_handle) {
        alpm_release(m_handle);
        m_handle = nullptr;
    }
}
#endif

bool AlpmWrapper::isInitialized() const {
    return m_handle != nullptr;
}

QString AlpmWrapper::lastError() const {
    return m_lastError;
}

Package AlpmWrapper::alpmPkgToPackage(alpm_pkg_t *pkg) const {
    Package p;
    p.id = QString::fromUtf8(alpm_pkg_get_name(pkg)) + "@pacman";
    p.name = QString::fromUtf8(alpm_pkg_get_name(pkg));
    p.version = QString::fromUtf8(alpm_pkg_get_version(pkg));
    p.description = QString::fromUtf8(alpm_pkg_get_desc(pkg));
    p.source = Package::Source::Pacman;
    p.state = Package::InstallState::NotInstalled;
    p.installedSize = static_cast<qint64>(alpm_pkg_get_isize(pkg));
    p.downloadSize = formatSize(alpm_pkg_get_size(pkg));
    if (alpm_pkg_get_url(pkg)) {
        p.iconUrl = QUrl(QString::fromUtf8(alpm_pkg_get_url(pkg)));
    }

    alpm_list_t *deps = alpm_pkg_get_depends(pkg);
    for (alpm_list_t *i = deps; i; i = alpm_list_next(i)) {
        alpm_depend_t *dep = static_cast<alpm_depend_t*>(i->data);
        p.dependencies.append(QString::fromUtf8(dep->name));
    }

    return p;
}

QString AlpmWrapper::formatSize(off_t size) const {
    if (size < 1024) return QString("%1 B").arg(size);
    if (size < 1024 * 1024) return QString("%1 KB").arg(size / 1024.0, 0, 'f', 1);
    if (size < 1024 * 1024 * 1024) return QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

QStringList AlpmWrapper::readPacmanConfRepoOrder() const {
    // NOTE: This parser only reads the top-level /etc/pacman.conf.
    // It does not handle Include directives for additional config files
    // or conditional repo blocks (e.g. [repo] sections inside included files).
    // This is sufficient for determining repo priority order, since
    // pacman.conf itself defines the repo sections even if their Server
    // lines come from included mirrorlists.

    QStringList repos;
    QFile f(QStringLiteral("/etc/pacman.conf"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return repos;

    while (!f.atEnd()) {
        QByteArray line = f.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        if (line.startsWith('[') && line.endsWith(']')) {
            QByteArray section = line.mid(1, line.size() - 2);
            if (section != "options")
                repos.append(QString::fromUtf8(section));
        }
    }
    f.close();
    return repos;
}

QList<Package> AlpmWrapper::search(const QString &query) {
    if (!initialize()) return {};

    qCDebug(lscPacman) << "search:" << query;

    QList<Package> results;

    alpm_list_t *syncdbs = alpm_get_syncdbs(m_handle);
    if (!syncdbs) {
        return {};
    }

    for (alpm_list_t *dbi = syncdbs; dbi; dbi = alpm_list_next(dbi)) {
        alpm_db_t *db = static_cast<alpm_db_t*>(dbi->data);

        QByteArray queryBytes = query.toUtf8();
        alpm_list_t *needles = alpm_list_add(nullptr, queryBytes.data());

        alpm_list_t *found = nullptr;
        int ret = alpm_db_search(db, needles, &found);
        alpm_list_free(needles);

        if (ret != 0) continue;

        for (alpm_list_t *pi = found; pi; pi = alpm_list_next(pi)) {
            alpm_pkg_t *pkg = static_cast<alpm_pkg_t*>(pi->data);
            if (!pkg) continue;
            Package p = alpmPkgToPackage(pkg);
            alpm_pkg_t *local = alpm_db_get_pkg(alpm_get_localdb(m_handle), alpm_pkg_get_name(pkg));
            if (local) {
                p.state = Package::InstallState::Installed;
                p.version = QString::fromUtf8(alpm_pkg_get_version(local));
            }
            results.append(p);
        }
        alpm_list_free(found);
    }

    // Deduplicate: same package can appear in multiple repos (e.g. extra vs cachyos-extra-v3).
    // Keep first match, which corresponds to highest-priority repo
    // (DBs are registered in pacman.conf order and alpm_get_syncdbs returns registration order).
    QSet<QString> seen;
    QList<Package> deduped;
    deduped.reserve(results.size());
    for (const Package &p : results) {
        if (!seen.contains(p.name)) {
            seen.insert(p.name);
            deduped.append(p);
        }
    }
    results = deduped;

    sortPackagesBySearchRelevance(results, query);
    qCDebug(lscPacman) << "search results:" << results.size() << "packages";

    return results;
}

QList<Package> AlpmWrapper::listInstalled() {
    if (!initialize()) return {};

    QList<Package> results;
    alpm_db_t *localdb = alpm_get_localdb(m_handle);
    alpm_list_t *pkgcache = alpm_db_get_pkgcache(localdb);

    for (alpm_list_t *i = pkgcache; i; i = alpm_list_next(i)) {
        alpm_pkg_t *pkg = static_cast<alpm_pkg_t*>(i->data);
        if (!pkg) continue;
        Package p = alpmPkgToPackage(pkg);
        p.state = Package::InstallState::Installed;
        results.append(p);
    }

    return results;
}

QList<Package> AlpmWrapper::checkUpdates() {
    if (!initialize()) return {};

    QList<Package> updates;
    alpm_db_t *localdb = alpm_get_localdb(m_handle);
    alpm_list_t *localcache = alpm_db_get_pkgcache(localdb);

    for (alpm_list_t *i = localcache; i; i = alpm_list_next(i)) {
        alpm_pkg_t *local = static_cast<alpm_pkg_t*>(i->data);
        alpm_pkg_t *newpkg = alpm_sync_get_new_version(local, alpm_get_syncdbs(m_handle));
        if (newpkg) {
            Package p = alpmPkgToPackage(newpkg);
            p.state = Package::InstallState::UpdateAvailable;
            p.version = QString::fromUtf8(alpm_pkg_get_version(local));
            p.newVersion = QString::fromUtf8(alpm_pkg_get_version(newpkg));
            updates.append(p);
        }
    }

    return updates;
}

QList<Package> AlpmWrapper::listForeignPackages() {
    if (!initialize()) return {};

    QList<Package> foreignPkgs;
    alpm_db_t *localdb = alpm_get_localdb(m_handle);
    alpm_list_t *syncdbs = alpm_get_syncdbs(m_handle);

    QSet<QString> syncPkgNames;
    for (alpm_list_t *di = syncdbs; di; di = alpm_list_next(di)) {
        alpm_db_t *db = static_cast<alpm_db_t*>(di->data);
        alpm_list_t *pkgcache = alpm_db_get_pkgcache(db);
        for (alpm_list_t *pi = pkgcache; pi; pi = alpm_list_next(pi)) {
            alpm_pkg_t *pkg = static_cast<alpm_pkg_t*>(pi->data);
            if (pkg)
                syncPkgNames.insert(QString::fromUtf8(alpm_pkg_get_name(pkg)));
        }
    }

    alpm_list_t *localcache = alpm_db_get_pkgcache(localdb);
    for (alpm_list_t *i = localcache; i; i = alpm_list_next(i)) {
        alpm_pkg_t *pkg = static_cast<alpm_pkg_t*>(i->data);
        if (!pkg) continue;
        QString name = QString::fromUtf8(alpm_pkg_get_name(pkg));
        if (!syncPkgNames.contains(name)) {
            Package p = alpmPkgToPackage(pkg);
            p.state = Package::InstallState::Installed;
            foreignPkgs.append(p);
        }
    }

    qCDebug(lscPacman) << "foreign packages found:" << foreignPkgs.size();
    return foreignPkgs;
}

QStringList AlpmWrapper::findOrphans() {

    QStringList orphans;
    alpm_db_t *localdb = alpm_get_localdb(m_handle);
    alpm_list_t *pkgcache = alpm_db_get_pkgcache(localdb);

    for (alpm_list_t *i = pkgcache; i; i = alpm_list_next(i)) {
        alpm_pkg_t *pkg = static_cast<alpm_pkg_t*>(i->data);
        if (!pkg) continue;
        if (alpm_pkg_get_reason(pkg) != ALPM_PKG_REASON_DEPEND) continue;

        alpm_list_t *requiredby = alpm_pkg_compute_requiredby(pkg);
        if (!requiredby) {
            orphans.append(QString::fromUtf8(alpm_pkg_get_name(pkg)));
        }
        alpm_list_free(requiredby);
    }

    // NOTE: This algorithm uses alpm_pkg_get_requiredby() to detect orphans.
    // It will miss orphan cycles — two or more DEPEND-packages that only require
    // each other with no explicit parent. Each shows a non-empty requiredby list
    // and won't be flagged. This is the same limitation as `pacman -Qdt` and is
    // extremely rare in practice on Arch Linux. A full fix would require computing
    // the transitive reachable set from all EXPLICIT packages and flagging any
    // DEPEND package outside that set.

    qCDebug(lscPacman) << "orphan scan found" << orphans.size() << "packages";
    return orphans;
}

QStringList AlpmWrapper::findDirtyReasons() {
    if (!initialize()) return {};

    QStringList dirty;
    alpm_db_t *localdb = alpm_get_localdb(m_handle);
    alpm_list_t *pkgcache = alpm_db_get_pkgcache(localdb);

    for (alpm_list_t *i = pkgcache; i; i = alpm_list_next(i)) {
        alpm_pkg_t *pkg = static_cast<alpm_pkg_t*>(i->data);
        if (!pkg) continue;
        if (alpm_pkg_get_reason(pkg) != ALPM_PKG_REASON_DEPEND) continue;

        QString name = QString::fromUtf8(alpm_pkg_get_name(pkg));
        QString desktopPath = QStringLiteral("/usr/share/applications/%1.desktop").arg(name);
        if (QFile::exists(desktopPath)) {
            dirty.append(name);
        }
    }

    qCDebug(lscPacman) << "dirty reason scan found" << dirty.size() << "packages";
    return dirty;
}

bool AlpmWrapper::isPackageInstalled(const QString &name) {
    if (!initialize()) return false;

    // Strip version constraints: truncate at first >=, >, <, or =
    // Do NOT strip from colon — that's an architecture qualifier (e.g. lib32:gcc)
    QString strippedName = name;
    for (int i = 0; i < strippedName.size(); ++i) {
        QChar c = strippedName.at(i);
        if (c == QLatin1Char('>') || c == QLatin1Char('<') || c == QLatin1Char('=')) {
            strippedName = strippedName.left(i);
            break;
        }
    }
    strippedName = strippedName.trimmed();

    alpm_db_t *localdb = alpm_get_localdb(m_handle);

    // First try exact name lookup
    alpm_pkg_t *pkg = alpm_db_get_pkg(localdb, strippedName.toUtf8().constData());
    if (pkg)
        return true;

    // If exact match fails, check if any installed package provides this name
    // (e.g. "p7zip" is provided by "7zip" package)
    alpm_list_t *pkgcache = alpm_db_get_pkgcache(localdb);
    for (alpm_list_t *i = pkgcache; i; i = alpm_list_next(i)) {
        alpm_pkg_t *p = static_cast<alpm_pkg_t*>(i->data);
        alpm_list_t *provides = alpm_pkg_get_provides(p);
        for (alpm_list_t *j = provides; j; j = alpm_list_next(j)) {
            alpm_depend_t *dep = static_cast<alpm_depend_t*>(j->data);
            if (dep && dep->name && strippedName == QString::fromUtf8(dep->name))
                return true;
        }
    }

    return false;
}

bool AlpmWrapper::isReasonRepairNeeded() const {
    if (m_root != QStringLiteral("/"))
        return false;

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString sentinel = cacheDir + QStringLiteral("/reason-repair-done");
    return !QFile::exists(sentinel);
}

void AlpmWrapper::markReasonRepairDone() {
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);
    QString sentinel = cacheDir + QStringLiteral("/reason-repair-done");
    QFile f(sentinel);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("done");
        f.close();
        qCDebug(lscPacman) << "created reason-repair sentinel";
    }
}
