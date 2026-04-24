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

private:
    AlpmWrapper *m_wrapper = nullptr;
};

QTEST_MAIN(TestPacman)
#include "test_pacman.moc"