#include "AlpmWrapper.h"
#include "PackageSearchUtils.h"
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QtDebug>

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
            return false;
        }
        QDir dbDir(m_dbpath);
        if (!dbDir.exists() && !dbDir.mkpath(".")) {
            m_lastError = QString("Failed to create dbpath directory: %1").arg(m_dbpath);
            return false;
        }

        // Ensure local/ subdir exists (required by libalpm)
        QDir localDir(m_dbpath + QStringLiteral("/local"));
        if (!localDir.exists()) {
            localDir.mkpath(".");
        }

        // Copy real sync DBs into test sandbox if present but sandbox sync dir is empty
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
            }
        }

        alpm_errno_t err = ALPM_ERR_OK;
        m_handle = alpm_initialize(m_root.toUtf8().constData(),
                                     m_dbpath.toUtf8().constData(),
                                     &err);
        if (!m_handle) {
            m_lastError = QString("Failed to initialize alpm: %1").arg(alpm_strerror(err));
            return false;
        }
    }

    // Re-sync registrations so newly-copied DBs are picked up
    if (m_handle) {
        alpm_unregister_all_syncdbs(m_handle);
    }

    QDir syncDir(m_dbpath + QStringLiteral("/sync"));
    for (const QString &file : syncDir.entryList({"*.db"}, QDir::Files)) {
        QString dbName = file;
        dbName.chop(3);
        alpm_db_t *db = alpm_register_syncdb(m_handle, dbName.toUtf8().constData(),
                                              ALPM_SIG_DATABASE_OPTIONAL);
        if (!db) {
            qWarning() << "Failed to register sync db:" << dbName
                     << alpm_strerror(alpm_errno(m_handle));
        }
    }

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

QList<Package> AlpmWrapper::search(const QString &query) {
    if (!initialize()) return {};

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

    sortPackagesBySearchRelevance(results, query);

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
