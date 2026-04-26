#include "FlatpakBackend.h"
#include "TransactionManager.h"
#include "PackageSearchUtils.h"
#include <QtConcurrent/QtConcurrent>
#include <QLoggingCategory>

// GLib headers use 'signals' as a struct field name which conflicts with
// Qt's #define signals Q_SIGNALS. Undefine before including GLib/Flatpak headers.
#undef signals
#include <flatpak/flatpak.h>
#define signals Q_SIGNALS

Q_LOGGING_CATEGORY(lscFlatpak, "lsc.flatpak")

static Package installedRefToPackage(FlatpakInstalledRef *ref) {
    Package pkg;
    pkg.source = Package::Source::Flatpak;
    pkg.id = QString::fromUtf8(flatpak_ref_format_ref_cached(FLATPAK_REF(ref)));
    pkg.name = QString::fromUtf8(flatpak_installed_ref_get_appdata_name(ref));
    if (pkg.name.isEmpty())
        pkg.name = QString::fromUtf8(flatpak_ref_get_name(FLATPAK_REF(ref)));
    pkg.version = QString::fromUtf8(flatpak_installed_ref_get_appdata_version(ref));
    if (pkg.version.isEmpty())
        pkg.version = QString::fromUtf8(flatpak_ref_get_branch(FLATPAK_REF(ref)));
    pkg.description = QString::fromUtf8(flatpak_installed_ref_get_appdata_summary(ref));
    pkg.state = Package::InstallState::Installed;
    pkg.installedSize = static_cast<qint64>(flatpak_installed_ref_get_installed_size(ref));
    pkg.flatpakRef = QString::fromUtf8(flatpak_ref_format_ref_cached(FLATPAK_REF(ref)));
    pkg.flatpakRemote = QString::fromUtf8(flatpak_installed_ref_get_origin(ref));
    return pkg;
}

static Package remoteRefToPackage(FlatpakRemoteRef *ref, const QString &remoteName) {
    Package pkg;
    pkg.source = Package::Source::Flatpak;
    pkg.id = QString::fromUtf8(flatpak_ref_format_ref_cached(FLATPAK_REF(ref)));
    pkg.name = QString::fromUtf8(flatpak_ref_get_name(FLATPAK_REF(ref)));
    pkg.version = QString::fromUtf8(flatpak_ref_get_branch(FLATPAK_REF(ref)));
    pkg.installedSize = static_cast<qint64>(flatpak_remote_ref_get_installed_size(ref));
    pkg.downloadSize = QString::number(flatpak_remote_ref_get_download_size(ref));
    pkg.flatpakRef = QString::fromUtf8(flatpak_ref_format_ref_cached(FLATPAK_REF(ref)));
    pkg.flatpakRemote = remoteName;
    pkg.state = Package::InstallState::NotInstalled;
    return pkg;
}

FlatpakBackend::FlatpakBackend(QObject *parent)
    : IPackageBackend(parent)
{
}

FlatpakBackend::~FlatpakBackend() {
    if (m_installation)
        g_object_unref(m_installation);
}

void FlatpakBackend::setTransactionManager(TransactionManager *tm) {
    m_tm = tm;
}

bool FlatpakBackend::ensureInstallation() {
    if (m_installation)
        return true;
    GError *error = nullptr;
    m_installation = flatpak_installation_new_system(nullptr, &error);
    if (!m_installation) {
        qCWarning(lscFlatpak) << "Failed to create Flatpak system installation:"
                              << (error ? error->message : "unknown error");
        if (error) g_error_free(error);
        return false;
    }
    flatpak_installation_set_no_interaction(m_installation, TRUE);
    return true;
}

QString FlatpakBackend::findRemoteForRef(const QString &ref) {
    QMutexLocker locker(&m_mutex);
    QString remote = m_refToRemote.value(ref);
    if (!remote.isEmpty())
        return remote;
    if (!ensureInstallation())
        return QString();

    GError *parseError = nullptr;
    FlatpakRef *parsedRef = flatpak_ref_parse(ref.toUtf8().constData(), &parseError);
    if (!parsedRef) {
        if (parseError) g_error_free(parseError);
        return QString();
    }
    const char *appName = flatpak_ref_get_name(parsedRef);
    const char *branch = flatpak_ref_get_branch(parsedRef);
    const char *arch = flatpak_ref_get_arch(parsedRef);

    GPtrArray *remotes = flatpak_installation_list_remotes(m_installation, nullptr, nullptr);
    if (!remotes) {
        g_object_unref(parsedRef);
        return QString();
    }
    for (guint r = 0; r < remotes->len; r++) {
        FlatpakRemote *remoteObj = FLATPAK_REMOTE(g_ptr_array_index(remotes, r));
        if (flatpak_remote_get_disabled(remoteObj))
            continue;
        const char *rname = flatpak_remote_get_name(remoteObj);
        GError *refError = nullptr;
        FlatpakRemoteRef *rref = flatpak_installation_fetch_remote_ref_sync(
            m_installation, rname, FLATPAK_REF_KIND_APP,
            appName, arch, branch, nullptr, &refError);
        if (rref) {
            remote = QString::fromUtf8(rname);
            g_object_unref(rref);
            m_refToRemote.insert(ref, remote);
            break;
        }
        if (refError) g_error_free(refError);
    }
    g_ptr_array_unref(remotes);
    g_object_unref(parsedRef);
    return remote;
}

QList<Package> FlatpakBackend::listInstalledRefs() {
    QList<Package> results;
    QMutexLocker locker(&m_mutex);
    if (!ensureInstallation())
        return results;
    GError *error = nullptr;
    GPtrArray *installed = flatpak_installation_list_installed_refs_by_kind(
        m_installation, FLATPAK_REF_KIND_APP, nullptr, &error);
    if (!installed) {
        qCWarning(lscFlatpak) << "list_installed_refs failed:" << (error ? error->message : "unknown");
        if (error) g_error_free(error);
        return results;
    }
    for (guint i = 0; i < installed->len; i++) {
        FlatpakInstalledRef *ref = FLATPAK_INSTALLED_REF(g_ptr_array_index(installed, i));
        Package pkg = installedRefToPackage(ref);
        m_refToRemote.insert(pkg.flatpakRef, pkg.flatpakRemote);
        results.append(pkg);
    }
    g_ptr_array_unref(installed);
    return results;
}

QList<Package> FlatpakBackend::listRemoteRefs() {
    QList<Package> allRefs;
    QMutexLocker locker(&m_mutex);
    if (!ensureInstallation())
        return allRefs;
    GError *error = nullptr;
    GPtrArray *remotes = flatpak_installation_list_remotes(m_installation, nullptr, &error);
    if (!remotes) {
        qCWarning(lscFlatpak) << "list_remotes failed:" << (error ? error->message : "unknown");
        if (error) g_error_free(error);
        return allRefs;
    }
    for (guint r = 0; r < remotes->len; r++) {
        FlatpakRemote *remote = FLATPAK_REMOTE(g_ptr_array_index(remotes, r));
        if (flatpak_remote_get_disabled(remote))
            continue;
        if (flatpak_remote_get_noenumerate(remote))
            continue;
        const char *remoteName = flatpak_remote_get_name(remote);
        GError *refError = nullptr;
        GPtrArray *refs = flatpak_installation_list_remote_refs_sync(
            m_installation, remoteName, nullptr, &refError);
        if (!refs) {
            qCDebug(lscFlatpak) << "list_remote_refs for" << remoteName
                                << "failed:" << (refError ? refError->message : "unknown");
            if (refError) g_error_free(refError);
            continue;
        }
        for (guint i = 0; i < refs->len; i++) {
            FlatpakRemoteRef *ref = FLATPAK_REMOTE_REF(g_ptr_array_index(refs, i));
            if (flatpak_ref_get_kind(FLATPAK_REF(ref)) != FLATPAK_REF_KIND_APP)
                continue;
            if (flatpak_ref_get_arch(FLATPAK_REF(ref)) != QString::fromUtf8(flatpak_get_default_arch()))
                continue;
            allRefs.append(remoteRefToPackage(ref, QString::fromUtf8(remoteName)));
            m_refToRemote.insert(allRefs.last().flatpakRef, allRefs.last().flatpakRemote);
        }
        g_ptr_array_unref(refs);
    }
    g_ptr_array_unref(remotes);
    return allRefs;
}

QList<Package> FlatpakBackend::listUpdateRefs() {
    QList<Package> results;
    QMutexLocker locker(&m_mutex);
    if (!ensureInstallation())
        return results;
    GError *error = nullptr;
    GPtrArray *updates = flatpak_installation_list_installed_refs_for_update(
        m_installation, nullptr, &error);
    if (!updates) {
        qCWarning(lscFlatpak) << "list_installed_refs_for_update failed:"
                              << (error ? error->message : "unknown");
        if (error) g_error_free(error);
        return results;
    }
    for (guint i = 0; i < updates->len; i++) {
        FlatpakInstalledRef *ref = FLATPAK_INSTALLED_REF(g_ptr_array_index(updates, i));
        Package pkg = installedRefToPackage(ref);
        pkg.state = Package::InstallState::UpdateAvailable;
        results.append(pkg);
    }
    g_ptr_array_unref(updates);
    return results;
}

void FlatpakBackend::doSearch(const QString &query) {
    QList<Package> results;
    if (query.isEmpty()) {
        emit searchResultsReady(results);
        return;
    }
    if (!m_remoteRefsCached) {
        m_cachedRemoteRefs = listRemoteRefs();
        m_remoteRefsCached = true;
        for (const Package &pkg : std::as_const(m_cachedRemoteRefs))
            m_refToRemote.insert(pkg.flatpakRef, pkg.flatpakRemote);
    }
    QString lowerQuery = query.toLower();
    for (const Package &pkg : std::as_const(m_cachedRemoteRefs)) {
        if (pkg.name.toLower().contains(lowerQuery) ||
            pkg.description.toLower().contains(lowerQuery)) {
            results.append(pkg);
        }
    }
    sortPackagesBySearchRelevance(results, query);
    emit searchResultsReady(results);
}

void FlatpakBackend::doListInstalled() {
    QList<Package> results = listInstalledRefs();
    emit installedListReady(results);
}

void FlatpakBackend::doCheckUpdates() {
    QList<Package> results = listUpdateRefs();
    emit updatesReady(results);
}

void FlatpakBackend::doInstall(const QString &ref) {
    QMutexLocker locker(&m_mutex);
    if (!ensureInstallation()) {
        emit installFinished(ref, false, "Failed to initialize Flatpak installation");
        return;
    }
    GError *error = nullptr;
    FlatpakTransaction *txn = flatpak_transaction_new_for_installation(m_installation, nullptr, &error);
    if (!txn) {
        QString msg = QString::fromUtf8(error ? error->message : "Failed to create transaction");
        if (error) g_error_free(error);
        emit installFinished(ref, false, msg);
        return;
    }
    flatpak_transaction_set_no_interaction(txn, TRUE);

    locker.unlock();
    QString remoteName = findRemoteForRef(ref);
    locker.relock();

    if (remoteName.isEmpty()) {
        g_object_unref(txn);
        m_pendingInstallRef.clear();
        emit installFinished(ref, false, "Could not determine remote for ref");
        return;
    }

    if (!flatpak_transaction_add_install(txn, remoteName.toUtf8().constData(), ref.toUtf8().constData(), nullptr, &error)) {
        QString msg = QString::fromUtf8(error ? error->message : "Failed to add install operation");
        if (error) g_error_free(error);
        g_object_unref(txn);
        m_pendingInstallRef.clear();
        emit installFinished(ref, false, msg);
        return;
    }

    locker.unlock();

    emit installProgress(ref, 0, "Starting Flatpak install");
    gboolean success = flatpak_transaction_run(txn, nullptr, &error);
    if (!success) {
        QString msg = QString::fromUtf8(error ? error->message : "Transaction failed");
        if (error) g_error_free(error);
        g_object_unref(txn);
        m_pendingInstallRef.clear();
        emit installFinished(ref, false, msg);
        return;
    }

    g_object_unref(txn);
    m_pendingInstallRef.clear();
    m_remoteRefsCached = false;

    GError *triggerError = nullptr;
    if (!flatpak_installation_run_triggers(m_installation, nullptr, &triggerError)) {
        qCDebug(lscFlatpak) << "run_triggers failed (non-fatal):"
                            << (triggerError ? triggerError->message : "unknown");
        if (triggerError) g_error_free(triggerError);
    }

    emit installProgress(ref, 100, "Complete");
    emit installFinished(ref, true, QString());
}

void FlatpakBackend::doRemove(const QString &ref) {
    QMutexLocker locker(&m_mutex);
    if (!ensureInstallation()) {
        emit removeFinished(ref, false, "Failed to initialize Flatpak installation");
        return;
    }
    GError *error = nullptr;
    FlatpakTransaction *txn = flatpak_transaction_new_for_installation(m_installation, nullptr, &error);
    if (!txn) {
        QString msg = QString::fromUtf8(error ? error->message : "Failed to create transaction");
        if (error) g_error_free(error);
        emit removeFinished(ref, false, msg);
        return;
    }
    flatpak_transaction_set_no_interaction(txn, TRUE);

    if (!flatpak_transaction_add_uninstall(txn, ref.toUtf8().constData(), &error)) {
        QString msg = QString::fromUtf8(error ? error->message : "Failed to add uninstall operation");
        if (error) g_error_free(error);
        g_object_unref(txn);
        emit removeFinished(ref, false, msg);
        return;
    }

    locker.unlock();

    emit removeProgress(ref, 0, "Starting Flatpak removal");
    gboolean success = flatpak_transaction_run(txn, nullptr, &error);
    if (!success) {
        QString msg = QString::fromUtf8(error ? error->message : "Transaction failed");
        if (error) g_error_free(error);
        g_object_unref(txn);
        m_pendingRemoveRef.clear();
        emit removeFinished(ref, false, msg);
        return;
    }

    g_object_unref(txn);
    m_pendingRemoveRef.clear();
    m_remoteRefsCached = false;

    GError *triggerError = nullptr;
    if (!flatpak_installation_run_triggers(m_installation, nullptr, &triggerError)) {
        qCDebug(lscFlatpak) << "run_triggers failed (non-fatal):"
                            << (triggerError ? triggerError->message : "unknown");
        if (triggerError) g_error_free(triggerError);
    }

    emit removeProgress(ref, 100, "Complete");
    emit removeFinished(ref, true, QString());
}

void FlatpakBackend::search(const QString &query) {
    QtConcurrent::run([this, query]() {
        doSearch(query);
    });
}

void FlatpakBackend::install(const QString &pkgId) {
    m_pendingInstallRef = pkgId;
    emit installProgress(pkgId, 0, "Preparing");
    QtConcurrent::run([this, pkgId]() {
        doInstall(pkgId);
    });
}

void FlatpakBackend::remove(const QString &pkgId) {
    m_pendingRemoveRef = pkgId;
    emit removeProgress(pkgId, 0, "Preparing");
    QtConcurrent::run([this, pkgId]() {
        doRemove(pkgId);
    });
}

QList<Package> FlatpakBackend::listInstalled() {
    QList<Package> results = listInstalledRefs();
    emit installedListReady(results);
    return results;
}

QList<Package> FlatpakBackend::checkUpdates() {
    QList<Package> results = listUpdateRefs();
    emit updatesReady(results);
    return results;
}

Package::Source FlatpakBackend::source() const {
    return Package::Source::Flatpak;
}

bool FlatpakBackend::isInitialized() const {
    return m_installation != nullptr;
}