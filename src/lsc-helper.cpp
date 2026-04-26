#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <map>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <alpm.h>

static bool g_debug = false;

enum CustomErrCode {
    LSC_ERR_NOT_FOUND = 100,
    LSC_ERR_LOCKED    = 101
};

static void emit_progress(int percent, const char *step) {
    std::fprintf(stdout, "PROGRESS %d %s\n", percent, step);
    std::fflush(stdout);
}

static void emit_error(const char *message) {
    std::fprintf(stdout, "ERROR %s\n", message);
    std::fflush(stdout);
}

static void emit_errcode(int code) {
    std::fprintf(stdout, "ERRCODE %d\n", code);
    std::fflush(stdout);
}

static void emit_debug(const char *message) {
    if (!g_debug) return;
    std::fprintf(stdout, "DEBUG %s\n", message);
    std::fflush(stdout);
}

static void progress_cb(void * /*ctx*/, alpm_progress_t progress, const char *pkg,
                        int percent, size_t /*howmany*/, size_t /*current*/) {
    (void)pkg;
    const char *step = "Installing";
    if (progress == ALPM_PROGRESS_REMOVE_START)
        step = "Removing";
    else if (progress == ALPM_PROGRESS_UPGRADE_START)
        step = "Upgrading";
    else if (progress == ALPM_PROGRESS_DOWNGRADE_START)
        step = "Downgrading";
    else if (progress == ALPM_PROGRESS_REINSTALL_START)
        step = "Reinstalling";
    else if (progress == ALPM_PROGRESS_CONFLICTS_START)
        step = "Resolving conflicts";
    else if (progress == ALPM_PROGRESS_DISKSPACE_START)
        step = "Checking disk space";
    else if (progress == ALPM_PROGRESS_INTEGRITY_START)
        step = "Verifying integrity";
    else if (progress == ALPM_PROGRESS_LOAD_START)
        step = "Loading packages";
    else if (progress == ALPM_PROGRESS_KEYRING_START)
        step = "Checking keyring";

    int mapped = 60 + (percent * 40 / 100);
    emit_progress(mapped, step);
}

static void event_cb(void * /*ctx*/, alpm_event_t *event) {
    switch (event->type) {
    case ALPM_EVENT_PKG_RETRIEVE_START:
        emit_progress(5, "Downloading");
        break;
    case ALPM_EVENT_PKG_RETRIEVE_DONE:
        emit_progress(35, "Download complete");
        break;
    case ALPM_EVENT_PKG_RETRIEVE_FAILED:
        emit_debug("package download failed (retrieve event)");
        break;
    case ALPM_EVENT_INTEGRITY_START:
        emit_progress(40, "Verifying");
        break;
    case ALPM_EVENT_INTEGRITY_DONE:
        emit_progress(50, "Verified");
        break;
    case ALPM_EVENT_CHECKDEPS_START:
        emit_progress(52, "Checking dependencies");
        break;
    case ALPM_EVENT_CHECKDEPS_DONE:
        emit_progress(55, "Dependencies resolved");
        break;
    case ALPM_EVENT_RESOLVEDEPS_START:
        emit_progress(56, "Resolving dependencies");
        break;
    case ALPM_EVENT_RESOLVEDEPS_DONE:
        emit_progress(58, "Dependencies resolved");
        break;
    case ALPM_EVENT_TRANSACTION_START:
        emit_progress(60, "Starting transaction");
        break;
    case ALPM_EVENT_TRANSACTION_DONE:
        break;
    case ALPM_EVENT_PACKAGE_OPERATION_START:
        emit_progress(65, "Processing package");
        break;
    case ALPM_EVENT_PACKAGE_OPERATION_DONE:
        break;
    case ALPM_EVENT_SCRIPTLET_INFO: {
        alpm_event_scriptlet_info_t *si = &event->scriptlet_info;
        if (si->line) {
            std::fprintf(stdout, "SCRIPTLET %s\n", si->line);
            std::fflush(stdout);
        }
        break;
    }
    default:
        break;
    }
}

static void dl_cb(void * /*ctx*/, const char *filename,
                  alpm_download_event_type_t event_type, void *data) {
    if (event_type == ALPM_DOWNLOAD_PROGRESS) {
        alpm_download_event_progress_t *prog = static_cast<alpm_download_event_progress_t*>(data);
        if (prog && prog->total != 0) {
            int pct = static_cast<int>(prog->downloaded * 100 / prog->total);
            int mapped = 5 + (pct * 30 / 100);
            emit_progress(mapped, "Downloading");
        }
    } else if (event_type == ALPM_DOWNLOAD_COMPLETED) {
        alpm_download_event_completed_t *comp = static_cast<alpm_download_event_completed_t*>(data);
        if (comp && comp->result == -1) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "download failed: %s", filename);
            emit_debug(buf);
        } else {
            emit_progress(35, "Download complete");
        }
    } else if (event_type == ALPM_DOWNLOAD_RETRY) {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "retrying download: %s", filename);
        emit_debug(buf);
    }
}

static bool clean_stale_lock() {
    const char *lockPath = "/var/lib/pacman/db.lck";
    struct stat st;
    if (stat(lockPath, &st) != 0) {
        emit_debug("no stale lock file found");
        return true;
    }

    emit_debug("lock file exists, checking /proc for running package managers");

    DIR *proc = opendir("/proc");
    if (!proc) {
        std::fprintf(stderr, "Warning: lock file exists and cannot check /proc\n");
        return false;
    }

    bool otherPkgManagerRunning = false;
    struct dirent *entry;
    while ((entry = readdir(proc)) != nullptr) {
        char *endp;
        long pid = strtol(entry->d_name, &endp, 10);
        if (*endp != '\0') continue;
        if (pid == getpid()) continue;

        char commPath[64];
        std::snprintf(commPath, sizeof(commPath), "/proc/%ld/comm", pid);
        FILE *f = std::fopen(commPath, "r");
        if (!f) continue;

        char comm[256];
        if (std::fgets(comm, sizeof(comm), f)) {
            comm[std::strcspn(comm, "\n")] = '\0';
            if (std::strcmp(comm, "pacman") == 0 ||
                std::strcmp(comm, "pamac-daemon") == 0 ||
                std::strcmp(comm, "lsc-helper") == 0 ||
                std::strcmp(comm, "yay") == 0 ||
                std::strcmp(comm, "paru") == 0) {
                otherPkgManagerRunning = true;
                char buf[256];
                std::snprintf(buf, sizeof(buf), "found running package manager: %s (pid %ld)", comm, pid);
                emit_debug(buf);
                std::fclose(f);
                break;
            }
        }
        std::fclose(f);
    }
    closedir(proc);

    if (otherPkgManagerRunning) {
        std::fprintf(stderr, "Another package manager is currently running\n");
        return false;
    }

    if (unlink(lockPath) != 0) {
        std::fprintf(stderr, "Warning: failed to remove stale lock: %s\n", std::strerror(errno));
        return false;
    }

    emit_debug("removed stale lock file");
    return true;
}

static void register_sync_dbs(alpm_handle_t *handle) {
    DIR *syncDir = opendir("/var/lib/pacman/sync");
    if (!syncDir) {
        emit_debug("cannot open /var/lib/pacman/sync");
        return;
    }

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(syncDir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.size() > 3 && name.substr(name.size() - 3) == ".db") {
            std::string dbName = name.substr(0, name.size() - 3);
            alpm_db_t *db = alpm_register_syncdb(handle, dbName.c_str(),
                                                  ALPM_SIG_DATABASE_OPTIONAL);
            if (!db) {
                std::fprintf(stderr, "Warning: failed to register sync db %s: %s\n",
                             dbName.c_str(), alpm_strerror(alpm_errno(handle)));
                char buf[256];
                std::snprintf(buf, sizeof(buf), "failed to register sync db: %s: %s",
                             dbName.c_str(), alpm_strerror(alpm_errno(handle)));
                emit_debug(buf);
            } else {
                char buf[256];
                std::snprintf(buf, sizeof(buf), "registered sync db: %s", dbName.c_str());
                emit_debug(buf);
                count++;
            }
        }
    }
    closedir(syncDir);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "registered %d sync databases", count);
    emit_debug(buf);
}

static bool detect_v3_support() {
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) return false;

    bool has_avx2 = false;
    bool has_fma = false;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        if (std::strncmp(line, "flags", 5) == 0) {
            if (std::strstr(line, " avx2 "))
                has_avx2 = true;
            if (std::strstr(line, " fma "))
                has_fma = true;
            break;
        }
    }
    fclose(f);
    return has_avx2 && has_fma;
}

struct RepoConfig {
    std::string name;
    std::vector<std::string> servers;
    std::vector<std::string> includeFiles;
};

struct PacmanConfig {
    std::string architecture;
    std::vector<RepoConfig> repos;
};

static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static PacmanConfig parse_pacman_conf() {
    PacmanConfig config;
    config.architecture = "auto";

    FILE *f = fopen("/etc/pacman.conf", "r");
    if (!f) {
        emit_debug("cannot open /etc/pacman.conf");
        return config;
    }

    RepoConfig *currentRepo = nullptr;
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        std::string l = trim(std::string(line));
        if (l.empty() || l[0] == '#') continue;

        if (l[0] == '[' && l.find(']') != std::string::npos) {
            std::string section = l.substr(1, l.find(']') - 1);
            if (section == "options") {
                currentRepo = nullptr;
            } else {
                config.repos.push_back(RepoConfig{section, {}, {}});
                currentRepo = &config.repos.back();
            }
            continue;
        }

        std::string::size_type eqPos = l.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = trim(l.substr(0, eqPos));
        std::string value = trim(l.substr(eqPos + 1));

        if (!currentRepo) {
            if (key == "Architecture")
                config.architecture = value;
        } else {
            if (key == "Server")
                currentRepo->servers.push_back(value);
            else if (key == "Include")
                currentRepo->includeFiles.push_back(value);
        }
    }
    fclose(f);

    char buf[128];
    std::snprintf(buf, sizeof(buf), "parsed pacman.conf: arch=%s, %zu repos",
                 config.architecture.c_str(), config.repos.size());
    emit_debug(buf);
    return config;
}

static void set_alpm_architectures(alpm_handle_t *handle, const std::string &archConfig) {
    alpm_list_t *archs = nullptr;

    bool v3 = false;
    if (archConfig == "auto") {
        archs = alpm_list_add(archs, const_cast<char*>("x86_64"));
        v3 = detect_v3_support();
        if (v3)
            archs = alpm_list_add(archs, const_cast<char*>("x86_64_v3"));
    } else if (archConfig == "x86_64_v3") {
        archs = alpm_list_add(archs, const_cast<char*>("x86_64_v3"));
        v3 = true;
    } else {
        archs = alpm_list_add(archs, const_cast<char*>(archConfig.c_str()));
    }

    alpm_option_set_architectures(handle, archs);
    alpm_list_free(archs);

    char buf[128];
    if (v3)
        std::snprintf(buf, sizeof(buf), "set architectures: x86_64, x86_64_v3 (v3 detected)");
    else
        std::snprintf(buf, sizeof(buf), "set architectures: %s", archConfig.c_str());
    emit_debug(buf);
}

static void add_servers_to_db(alpm_db_t *db, const std::vector<std::string> &servers, const std::string &arch) {
    std::string repoName = alpm_db_get_name(db);

    for (const auto &url : servers) {
        std::string serverUrl = url;

        std::string::size_type rp;
        while ((rp = serverUrl.find("$repo")) != std::string::npos)
            serverUrl.replace(rp, 5, repoName);

        // Replace $arch first, then $arch_v3 as fallback
        bool archReplaced = false;
        while ((rp = serverUrl.find("$arch_v3")) != std::string::npos) {
            serverUrl.replace(rp, 8, "x86_64_v3");
            archReplaced = true;
        }
        if (!archReplaced) {
            while ((rp = serverUrl.find("$arch")) != std::string::npos)
                serverUrl.replace(rp, 5, arch);
        }

        alpm_db_add_server(db, serverUrl.c_str());
    }
}

static std::vector<std::string> read_mirrorlist_servers(const std::string &path) {
    std::vector<std::string> servers;

    FILE *f = fopen(path.c_str(), "r");
    if (!f) return servers;

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        std::string l = trim(std::string(line));
        if (l.empty() || l[0] == '#') continue;

        std::string::size_type pos = l.find("Server");
        if (pos == std::string::npos) continue;
        pos = l.find('=', pos + 6);
        if (pos == std::string::npos) continue;
        std::string url = trim(l.substr(pos + 1));
        if (!url.empty())
            servers.push_back(url);
    }
    fclose(f);

    return servers;
}

static void set_mirror_servers(alpm_handle_t *handle) {
    PacmanConfig config = parse_pacman_conf();
    set_alpm_architectures(handle, config.architecture);

    alpm_list_t *dbs = alpm_get_syncdbs(handle);
    if (!dbs) return;

    // Build a map of db name → alpm_db_t*
    std::map<std::string, alpm_db_t*> dbMap;
    for (alpm_list_t *di = dbs; di; di = alpm_list_next(di)) {
        alpm_db_t *db = static_cast<alpm_db_t*>(di->data);
        dbMap[alpm_db_get_name(db)] = db;
    }

    // Determine base architecture for $arch replacement
    std::string arch = "x86_64";
    if (config.architecture == "auto" || config.architecture == "x86_64_v3") {
        // Use x86_64 for standard repos; v3 repos get x86_64_v3 via $arch_v3
        arch = "x86_64";
    } else {
        arch = config.architecture;
    }

    int serverCount = 0;

    // For each repo in pacman.conf, add its servers to the matching registered DB
    for (const auto &repo : config.repos) {
        auto it = dbMap.find(repo.name);
        if (it == dbMap.end()) continue;

        alpm_db_t *db = it->second;

        // Add inline Server directives
        if (!repo.servers.empty()) {
            add_servers_to_db(db, repo.servers, arch);
            serverCount += repo.servers.size();
        }

        // Add servers from Include files (mirrorlists)
        for (const auto &incFile : repo.includeFiles) {
            std::vector<std::string> mirrors = read_mirrorlist_servers(incFile);
            if (!mirrors.empty()) {
                add_servers_to_db(db, mirrors, arch);
                serverCount += mirrors.size();

                char buf[256];
                std::snprintf(buf, sizeof(buf), "loaded %zu servers from %s for repo %s",
                             mirrors.size(), incFile.c_str(), repo.name.c_str());
                emit_debug(buf);
            } else {
                char buf[256];
                std::snprintf(buf, sizeof(buf), "no servers found in %s for repo %s",
                             incFile.c_str(), repo.name.c_str());
                emit_debug(buf);
            }
        }
    }

    char buf[64];
    std::snprintf(buf, sizeof(buf), "set %d mirror server entries", serverCount);
    emit_debug(buf);
}

static int do_sync(alpm_handle_t *handle) {
    emit_progress(0, "Refreshing databases");

    alpm_list_t *sync_dbs = alpm_get_syncdbs(handle);
    if (!sync_dbs) {
        emit_error("No sync databases registered");
        emit_errcode(LSC_ERR_NOT_FOUND);
        return 1;
    }

    int total = 0;
    for (alpm_list_t *i = sync_dbs; i; i = alpm_list_next(i))
        total++;

    int current = 0;
    for (alpm_list_t *i = sync_dbs; i; i = alpm_list_next(i)) {
        alpm_db_t *db = static_cast<alpm_db_t*>(i->data);
        current++;
        int pct = (current * 100) / total;
        int mapped = (pct * 95) / 100;
        emit_progress(mapped, "Refreshing databases");

        int ret = alpm_db_update(handle, i, 0);
        if (ret < 0) {
            alpm_errno_t err = alpm_errno(handle);
            if (err != ALPM_ERR_OK) {
                std::fprintf(stderr, "Warning: failed to update db %s: %s\n",
                             alpm_db_get_name(db), alpm_strerror(err));
                char buf[256];
                std::snprintf(buf, sizeof(buf), "db update failed: %s: %s",
                             alpm_db_get_name(db), alpm_strerror(err));
                emit_debug(buf);
            }
        } else if (ret == 0) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "db up to date: %s", alpm_db_get_name(db));
            emit_debug(buf);
        } else {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "db updated: %s", alpm_db_get_name(db));
            emit_debug(buf);
        }
    }

    emit_progress(100, "Databases refreshed");
    return 0;
}

static int do_install(alpm_handle_t *handle, const char *pkg_name) {
    emit_progress(0, "Preparing transaction");

    char findbuf[256];
    std::snprintf(findbuf, sizeof(findbuf), "looking for package: %s", pkg_name);
    emit_debug(findbuf);

    alpm_pkg_t *pkg = nullptr;
    alpm_list_t *dbs = alpm_get_syncdbs(handle);
    for (alpm_list_t *i = dbs; i; i = alpm_list_next(i)) {
        alpm_db_t *db = static_cast<alpm_db_t*>(i->data);
        pkg = alpm_db_get_pkg(db, pkg_name);
        if (pkg) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "found package %s in db %s",
                         pkg_name, alpm_db_get_name(db));
            emit_debug(buf);
            break;
        }
    }

    if (!pkg) {
        emit_error("Package not found — try refreshing package databases first");
        emit_errcode(LSC_ERR_NOT_FOUND);
        return 1;
    }

    int flags = ALPM_TRANS_FLAG_ALLDEPS;
    emit_debug("initializing transaction (ALLDEPS)");
    if (alpm_trans_init(handle, flags) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_init failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    emit_debug("adding package to transaction");
    if (alpm_add_pkg(handle, pkg) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_add_pkg failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_trans_release(handle);
        return 1;
    }

    alpm_list_t *data = nullptr;
    emit_debug("preparing transaction");
    if (alpm_trans_prepare(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_prepare failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    data = nullptr;
    emit_debug("committing transaction");
    if (alpm_trans_commit(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_commit failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    alpm_trans_release(handle);

    alpm_pkg_t *installed = alpm_db_get_pkg(alpm_get_localdb(handle), pkg_name);
    if (installed) {
        if (alpm_pkg_set_reason(installed, ALPM_PKG_REASON_EXPLICIT) != 0) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "failed to set explicit reason for %s: %s",
                         pkg_name, alpm_strerror(alpm_errno(handle)));
            emit_debug(buf);
        } else {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "set install reason to EXPLICIT for %s", pkg_name);
            emit_debug(buf);
        }
    }

    emit_progress(100, "Complete");
    return 0;
}

static int do_install_local(alpm_handle_t *handle, const char *filepath) {
    emit_progress(0, "Preparing local install");

    alpm_pkg_t *pkg = nullptr;
    if (alpm_pkg_load(handle, filepath, 0, 0, &pkg) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_pkg_load failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    const char *loaded_name = alpm_pkg_get_name(pkg);
    char namebuf[256];
    std::snprintf(namebuf, sizeof(namebuf), "loaded local package: %s", loaded_name);
    emit_debug(namebuf);

    int flags = ALPM_TRANS_FLAG_ALLDEPS;
    if (alpm_trans_init(handle, flags) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_init failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    if (alpm_add_pkg(handle, pkg) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_add_pkg failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_trans_release(handle);
        return 1;
    }

    alpm_list_t *data = nullptr;
    if (alpm_trans_prepare(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    data = nullptr;
    if (alpm_trans_commit(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    alpm_trans_release(handle);

    alpm_pkg_t *installed = alpm_db_get_pkg(alpm_get_localdb(handle), loaded_name);
    if (installed) {
        if (alpm_pkg_set_reason(installed, ALPM_PKG_REASON_EXPLICIT) != 0) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "failed to set explicit reason for %s: %s",
                         loaded_name, alpm_strerror(alpm_errno(handle)));
            emit_debug(buf);
        } else {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "set install reason to EXPLICIT for %s", loaded_name);
            emit_debug(buf);
        }
    }

    emit_progress(100, "Complete");
    return 0;
}

static int do_remove(alpm_handle_t *handle, const char *pkg_name, bool cascade) {
    emit_progress(0, "Preparing removal");

    char findbuf[256];
    std::snprintf(findbuf, sizeof(findbuf), "looking for installed package: %s", pkg_name);
    emit_debug(findbuf);

    alpm_pkg_t *pkg = alpm_db_get_pkg(alpm_get_localdb(handle), pkg_name);
    if (!pkg) {
        emit_error("Package not found in local database");
        emit_errcode(LSC_ERR_NOT_FOUND);
        return 1;
    }

    int flags = 0;
    if (cascade)
        flags |= ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE;

    char flagbuf[64];
    std::snprintf(flagbuf, sizeof(flagbuf), "transaction flags: 0x%x (cascade=%d)", flags, cascade);
    emit_debug(flagbuf);

    if (alpm_trans_init(handle, flags) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_init failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    if (alpm_remove_pkg(handle, pkg) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_remove_pkg failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_trans_release(handle);
        return 1;
    }

    alpm_list_t *data = nullptr;
    if (alpm_trans_prepare(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    data = nullptr;
    if (alpm_trans_commit(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    alpm_trans_release(handle);
    emit_progress(100, "Complete");
    return 0;
}

static int do_remove_orphans(alpm_handle_t *handle, const std::vector<std::string> &pkg_names) {
    emit_progress(0, "Preparing orphan cleanup");

    int flags = ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE;

    char flagbuf[64];
    std::snprintf(flagbuf, sizeof(flagbuf), "transaction flags: 0x%x (cascade+recurse)", flags);
    emit_debug(flagbuf);

    if (alpm_trans_init(handle, flags) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_init failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    alpm_db_t *localdb = alpm_get_localdb(handle);

    for (const auto &name : pkg_names) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(localdb, name.c_str());
        if (!pkg) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "orphan package not found in local DB: %s", name.c_str());
            emit_debug(buf);
            continue;
        }
        if (alpm_remove_pkg(handle, pkg) != 0) {
            alpm_errno_t err = alpm_errno(handle);
            char buf[256];
            std::snprintf(buf, sizeof(buf), "alpm_remove_pkg failed for %s: %s", name.c_str(), alpm_strerror(err));
            emit_debug(buf);
            emit_error(alpm_strerror(err));
            emit_errcode(static_cast<int>(err));
            alpm_trans_release(handle);
            return 1;
        }
    }

    alpm_list_t *data = nullptr;
    if (alpm_trans_prepare(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    data = nullptr;
    if (alpm_trans_commit(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    alpm_trans_release(handle);
    emit_progress(100, "Complete");
    return 0;
}

static int do_set_reason(alpm_handle_t *handle, const std::vector<std::string> &pkg_names) {
    emit_progress(0, "Fixing install reasons");

    alpm_db_t *localdb = alpm_get_localdb(handle);
    int fixed = 0;

    for (const auto &name : pkg_names) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(localdb, name.c_str());
        if (!pkg) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "package not found in local DB: %s", name.c_str());
            emit_debug(buf);
            continue;
        }
        if (alpm_pkg_set_reason(pkg, ALPM_PKG_REASON_EXPLICIT) != 0) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "failed to set reason for %s: %s",
                         name.c_str(), alpm_strerror(alpm_errno(handle)));
            emit_debug(buf);
            continue;
        }
        char buf[256];
        std::snprintf(buf, sizeof(buf), "set install reason to EXPLICIT for %s", name.c_str());
        emit_debug(buf);
        fixed++;
    }

    char resultbuf[64];
    std::snprintf(resultbuf, sizeof(resultbuf), "Fixed %d packages", fixed);
    emit_progress(100, resultbuf);
    return 0;
}

static int do_upgrade(alpm_handle_t *handle) {
    emit_progress(0, "Preparing system upgrade");

    int flags = ALPM_TRANS_FLAG_ALLDEPS;
    emit_debug("initializing upgrade transaction (ALLDEPS)");

    if (alpm_trans_init(handle, flags) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_init failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    if (alpm_sync_sysupgrade(handle, 0) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_sync_sysupgrade failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_trans_release(handle);
        return 1;
    }

    alpm_list_t *data = nullptr;
    emit_debug("preparing upgrade transaction");
    if (alpm_trans_prepare(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_prepare failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    data = nullptr;
    emit_debug("committing upgrade transaction");
    if (alpm_trans_commit(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_commit failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    alpm_trans_release(handle);
    emit_progress(100, "System upgrade complete");
    return 0;
}

static int do_system_upgrade(alpm_handle_t *handle, const std::vector<std::string> &pkg_names) {
    if (pkg_names.empty()) {
        return do_upgrade(handle);
    }

    emit_progress(0, "Preparing selective upgrade");

    int flags = ALPM_TRANS_FLAG_ALLDEPS;
    if (alpm_trans_init(handle, flags) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_trans_init failed: %s", alpm_strerror(err));
        emit_debug(buf);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }

    alpm_db_t *localdb = alpm_get_localdb(handle);
    alpm_list_t *syncdbs = alpm_get_syncdbs(handle);

    for (const auto &name : pkg_names) {
        alpm_pkg_t *pkg = nullptr;
        for (alpm_list_t *i = syncdbs; i; i = alpm_list_next(i)) {
            alpm_db_t *db = static_cast<alpm_db_t*>(i->data);
            pkg = alpm_db_get_pkg(db, name.c_str());
            if (pkg) break;
        }

        if (!pkg) {
            alpm_pkg_t *local = alpm_db_get_pkg(localdb, name.c_str());
            if (local) {
                pkg = alpm_sync_get_new_version(local, syncdbs);
            }
        }

        if (!pkg) {
            char buf[256];
            std::snprintf(buf, sizeof(buf), "package not found for upgrade: %s", name.c_str());
            emit_debug(buf);
            continue;
        }

        if (alpm_add_pkg(handle, pkg) != 0) {
            alpm_errno_t err = alpm_errno(handle);
            char buf[256];
            std::snprintf(buf, sizeof(buf), "alpm_add_pkg failed for %s: %s", name.c_str(), alpm_strerror(err));
            emit_debug(buf);
            alpm_trans_release(handle);
            emit_error(alpm_strerror(err));
            emit_errcode(static_cast<int>(err));
            return 1;
        }
    }

    alpm_list_t *data = nullptr;
    if (alpm_trans_prepare(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    data = nullptr;
    if (alpm_trans_commit(handle, &data) != 0) {
        alpm_errno_t err = alpm_errno(handle);
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        alpm_list_free(data);
        alpm_trans_release(handle);
        return 1;
    }
    alpm_list_free(data);

    alpm_trans_release(handle);

    for (const auto &name : pkg_names) {
        alpm_pkg_t *installed = alpm_db_get_pkg(localdb, name.c_str());
        if (installed) {
            if (alpm_pkg_set_reason(installed, ALPM_PKG_REASON_EXPLICIT) != 0) {
                char buf[256];
                std::snprintf(buf, sizeof(buf), "failed to set explicit reason for %s: %s",
                             name.c_str(), alpm_strerror(alpm_errno(handle)));
                emit_debug(buf);
            }
        }
    }

    emit_progress(100, "Complete");
    return 0;
}

int main(int argc, char *argv[]) {
    g_debug = (std::getenv("LSC_DEBUG") != nullptr);

    if (argc < 2) {
        std::fprintf(stderr, "Usage: lsc-helper <sync|install|install-local|remove|remove-orphans|set-reason|upgrade> ...\\n");
        return 1;
    }

    std::string action = argv[1];

    if (action != "sync" && action != "install" && action != "install-local"
        && action != "remove" && action != "remove-orphans" && action != "set-reason"
        && action != "upgrade") {
        std::fprintf(stderr, "Unknown action: %s\n", action.c_str());
        return 1;
    }

    if ((action == "install" || action == "remove") && argc < 3) {
        std::fprintf(stderr, "Usage: lsc-helper %s <package-name> [--cascade]\n", action.c_str());
        return 1;
    }

    if (action == "install-local" && argc < 3) {
        std::fprintf(stderr, "Usage: lsc-helper install-local <filepath>\n");
        return 1;
    }

    if (action == "remove-orphans" && argc < 3) {
        std::fprintf(stderr, "Usage: lsc-helper remove-orphans <pkg1> [pkg2] ...\n");
        return 1;
    }

    if (action == "set-reason" && argc < 3) {
        std::fprintf(stderr, "Usage: lsc-helper set-reason <pkg1> [pkg2] ...\n");
        return 1;
    }

    std::string pkg_name;
    std::string local_filepath;
    bool cascade = false;
    std::vector<std::string> orphan_names;
    std::vector<std::string> reason_names;
    std::vector<std::string> upgrade_names;

    if (action == "install" || action == "remove") {
        pkg_name = argv[2];
        for (int i = 3; i < argc; ++i) {
            if (std::strcmp(argv[i], "--cascade") == 0)
                cascade = true;
        }
    } else if (action == "install-local") {
        local_filepath = argv[2];
    } else if (action == "remove-orphans") {
        for (int i = 2; i < argc; ++i)
            orphan_names.push_back(argv[i]);
    } else if (action == "set-reason") {
        for (int i = 2; i < argc; ++i)
            reason_names.push_back(argv[i]);
    } else if (action == "upgrade") {
        for (int i = 2; i < argc; ++i)
            upgrade_names.push_back(argv[i]);
    }

    if (!clean_stale_lock()) {
        emit_error("Package database is locked — another package manager may be running");
        emit_errcode(LSC_ERR_LOCKED);
        return 1;
    }

    alpm_errno_t err = ALPM_ERR_OK;
    alpm_handle_t *handle = alpm_initialize("/", "/var/lib/pacman", &err);
    if (!handle) {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "alpm_initialize failed: %s", alpm_strerror(err));
        emit_debug(buf);
        std::fprintf(stderr, "Failed to initialize alpm: %s\n", alpm_strerror(err));
        emit_error(alpm_strerror(err));
        emit_errcode(static_cast<int>(err));
        return 1;
    }
    emit_debug("alpm initialized successfully");

    register_sync_dbs(handle);
    set_mirror_servers(handle); // also parses pacman.conf, sets architectures, adds per-repo mirrors

    alpm_option_set_progresscb(handle, progress_cb, nullptr);
    alpm_option_set_eventcb(handle, event_cb, nullptr);
    alpm_option_set_dlcb(handle, dl_cb, nullptr);

    int ret;
    if (action == "sync")
        ret = do_sync(handle);
    else if (action == "install")
        ret = do_install(handle, pkg_name.c_str());
    else if (action == "install-local")
        ret = do_install_local(handle, local_filepath.c_str());
    else if (action == "remove")
        ret = do_remove(handle, pkg_name.c_str(), cascade);
    else if (action == "remove-orphans")
        ret = do_remove_orphans(handle, orphan_names);
    else if (action == "upgrade")
        ret = do_system_upgrade(handle, upgrade_names);
    else
        ret = do_set_reason(handle, reason_names);

    alpm_release(handle);
    return ret;
}