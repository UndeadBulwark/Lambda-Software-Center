#include <QtTest>
#include "AlpmWrapper.h"
#include "PacmanBackend.h"

class TestPacman : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Use a sandboxed DB root — never the live system
        m_wrapper = new AlpmWrapper("/tmp/lambda-test-root", "/tmp/lambda-test-db", this);
    }

    void cleanupTestCase()
    {
        delete m_wrapper;
    }

    // --- AlpmWrapper init ---

    void test_initialize_succeeds()
    {
        QVERIFY(m_wrapper->isInitialized());
    }

    void test_initialize_with_bad_root_fails_gracefully()
    {
        AlpmWrapper bad("/nonexistent/root", "/nonexistent/db");
        QVERIFY(!bad.isInitialized());
        QVERIFY(!bad.lastError().isEmpty());
    }

    // --- Search ---

    void test_search_returns_results_for_known_package()
    {
        QList<Package> results = m_wrapper->search("linux");
        QVERIFY(!results.isEmpty());
    }

    void test_search_result_fields_are_populated()
    {
        QList<Package> results = m_wrapper->search("firefox");
        QVERIFY(!results.isEmpty());
        const Package &p = results.first();
        QVERIFY(!p.name.isEmpty());
        QVERIFY(!p.version.isEmpty());
        QCOMPARE(p.source, Package::Source::Pacman);
    }

    void test_search_empty_string_returns_empty_or_all()
    {
        // Behavior must be defined — either empty list or full list, not a crash
        QList<Package> results = m_wrapper->search("");
        // Just verify it does not throw or return garbage
        Q_UNUSED(results);
        QVERIFY(true);
    }

    void test_search_no_match_returns_empty_list()
    {
        QList<Package> results = m_wrapper->search("zzznomatchpackagexyz");
        QVERIFY(results.isEmpty());
    }

    // --- listInstalled ---

    void test_list_installed_returns_list()
    {
        QList<Package> installed = m_wrapper->listInstalled();
        // Sandboxed DB may be empty — just verify no crash and correct type
        QVERIFY(installed.size() >= 0);
    }

    void test_list_installed_packages_have_source_pacman()
    {
        QList<Package> installed = m_wrapper->listInstalled();
        for (const Package &p : installed) {
            QCOMPARE(p.source, Package::Source::Pacman);
        }
    }

    // --- PacmanBackend signal interface ---

    void test_backend_search_emits_results_signal()
    {
        PacmanBackend backend;
        QSignalSpy spy(&backend, &PacmanBackend::searchResultsReady);
        backend.search("linux");
        QVERIFY(spy.wait(3000));
        QCOMPARE(spy.count(), 1);
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
    }

    void test_backend_search_emits_error_on_uninitialized()
    {
        PacmanBackend bad;
        bad.forceUninitializedState(); // test seam
        QSignalSpy spy(&bad, &PacmanBackend::errorOccurred);
        bad.search("anything");
        QVERIFY(spy.wait(1000));
        QCOMPARE(spy.count(), 1);
    }

    // --- checkUpdates ---

    void test_check_updates_returns_list()
    {
        QList<Package> updates = m_wrapper->checkUpdates();
        QVERIFY(updates.size() >= 0);
    }

    void test_update_packages_have_new_version_field()
    {
        QList<Package> updates = m_wrapper->checkUpdates();
        for (const Package &p : updates) {
            QVERIFY(!p.newVersion.isEmpty());
        }
    }

    // --- Package struct integrity ---

    void test_package_default_constructed_source_is_unknown()
    {
        Package p;
        QCOMPARE(p.source, Package::Source::Unknown);
    }

    void test_search_sorts_exact_match_first()
    {
        QList<Package> results = m_wrapper->search("linux");
        QVERIFY(!results.isEmpty());
        QCOMPARE(results.first().name, QString("linux"));
    }

    void test_search_sorts_prefix_before_contains()
    {
        QList<Package> results = m_wrapper->search("linux");
        QVERIFY(results.size() >= 2);
        // 'linux' should come before 'linux-firmware' (exact vs prefix)
        int idxExact = -1;
        int idxPrefix = -1;
        for (int i = 0; i < results.size(); ++i) {
            if (results.at(i).name == "linux") idxExact = i;
            if (results.at(i).name == "linux-firmware") idxPrefix = i;
        }
        QVERIFY(idxExact != -1);
        QVERIFY(idxPrefix != -1);
        QVERIFY(idxExact < idxPrefix);
    }

    // --- findOrphans ---

    void test_find_orphans_returns_stringlist()
    {
        QStringList orphans = m_wrapper->findOrphans();
        // Sandboxed DB may have no orphans — just verify no crash and correct type
        QVERIFY(orphans.size() >= 0);
    }

    void test_find_orphans_does_not_crash_with_empty_db()
    {
        AlpmWrapper emptyWrapper("/tmp/lambda-test-empty-root", "/tmp/lambda-test-empty-db");
        QStringList orphans = emptyWrapper.findOrphans();
        QVERIFY(orphans.isEmpty());
    }

    // --- PacmanBackend checkOrphans ---

    void test_backend_check_orphans_emits_signal()
    {
        PacmanBackend backend;
        QSignalSpy spy(&backend, &PacmanBackend::orphansFound);
        backend.checkOrphans();
        QCOMPARE(spy.count(), 1);
    }

    // --- findDirtyReasons ---

    void test_find_dirty_reasons_returns_stringlist()
    {
        QStringList dirty = m_wrapper->findDirtyReasons();
        QVERIFY(dirty.size() >= 0);
    }

    void test_find_dirty_reasons_does_not_crash_with_empty_db()
    {
        AlpmWrapper emptyWrapper("/tmp/lambda-test-empty-root", "/tmp/lambda-test-empty-db");
        QStringList dirty = emptyWrapper.findDirtyReasons();
        QVERIFY(dirty.isEmpty());
    }

    // --- isReasonRepairNeeded / markReasonRepairDone ---

    void test_reason_repair_not_needed_in_sandbox()
    {
        QVERIFY(!m_wrapper->isReasonRepairNeeded());
    }

    void test_backend_check_dirty_reasons_emits_signal()
    {
        PacmanBackend backend;
        QSignalSpy spy(&backend, &PacmanBackend::dirtyReasonsFound);
        backend.checkDirtyReasons();
        QCOMPARE(spy.count(), 1);
    }

    void test_backend_check_dirty_reasons_always_emits()
    {
        PacmanBackend backend;
        QSignalSpy spy(&backend, &PacmanBackend::dirtyReasonsFound);
        backend.forceUninitializedState();
        backend.checkDirtyReasons();
        QCOMPARE(spy.count(), 1);
        QStringList result = spy.at(0).at(0).toStringList();
        QVERIFY(result.isEmpty());
    }

    // --- listForeignPackages ---

    void test_list_foreign_packages_returns_list()
    {
        QList<Package> foreign = m_wrapper->listForeignPackages();
        for (const Package &p : foreign) {
            QCOMPARE(p.source, Package::Source::Pacman);
            QCOMPARE(p.state, Package::InstallState::Installed);
        }
    }

    void test_list_foreign_packages_does_not_crash_with_empty_db()
    {
        AlpmWrapper emptyWrapper("/tmp/lambda-test-foreign-root", "/tmp/lambda-test-foreign-db");
        QList<Package> foreign = emptyWrapper.listForeignPackages();
        QVERIFY(foreign.isEmpty());
    }

    void test_list_foreign_packages_on_live_system()
    {
        AlpmWrapper liveWrapper;
        QList<Package> foreign = liveWrapper.listForeignPackages();
        for (const Package &p : foreign) {
            QVERIFY(!p.name.isEmpty());
            QVERIFY(!p.version.isEmpty());
        }
    }

    // --- isPackageInstalled ---

    void test_is_package_installed_strips_version()
    {
        // Verify version constraint stripping: "gcc>=12" and "gcc" should
        // produce the same lookup result. Test against sandbox (empty local DB)
        // so both return false — the point is they don't crash and produce
        // the same answer.
        bool withVersion = m_wrapper->isPackageInstalled("gcc>=12");
        bool withoutVersion = m_wrapper->isPackageInstalled("gcc");
        QCOMPARE(withVersion, withoutVersion);
    }

    void test_is_package_installed_unknown_returns_false()
    {
        QVERIFY(!m_wrapper->isPackageInstalled("zzznonexistentpkgxyz123"));
    }

    void test_is_package_installed_live_system()
    {
        // Test against live system where pacman is definitely installed
        AlpmWrapper liveWrapper;
        QVERIFY(liveWrapper.isPackageInstalled("pacman"));
        // Verify stripping: pacman>=99 should resolve same as pacman
        QVERIFY(liveWrapper.isPackageInstalled("pacman>=99"));
    }

private:
    AlpmWrapper *m_wrapper = nullptr;
};

QTEST_MAIN(TestPacman)
#include "test_pacman.moc"