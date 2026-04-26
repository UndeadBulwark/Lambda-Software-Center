#include <QtTest>
#include "FlatpakBackend.h"

class TestFlatpak : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
        m_backend = new FlatpakBackend(this);
    }

    // --- Source enum ---

    void test_flatpak_source_enum_value()
    {
        QCOMPARE(static_cast<int>(Package::Source::Flatpak), 2);
    }

    // --- Initialization ---

    void test_backend_initializes()
    {
        m_backend->listInstalled();
        QVERIFY(m_backend->isInitialized());
    }

    // --- Search ---

    void test_search_returns_results()
    {
        QSignalSpy spy(m_backend, &FlatpakBackend::searchResultsReady);
        m_backend->search("firefox");
        QVERIFY(spy.wait(5000));
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
    }

    void test_search_results_have_flatpak_source()
    {
        QSignalSpy spy(m_backend, &FlatpakBackend::searchResultsReady);
        m_backend->search("gimp");
        QVERIFY(spy.wait(5000));
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        for (const Package &p : results) {
            QCOMPARE(p.source, Package::Source::Flatpak);
        }
    }

    void test_search_result_has_ref_field()
    {
        QSignalSpy spy(m_backend, &FlatpakBackend::searchResultsReady);
        m_backend->search("firefox");
        QVERIFY(spy.wait(5000));
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
        // Flatpak packages must carry their ref for install/remove to work
        QVERIFY(!results.first().flatpakRef.isEmpty());
    }

    void test_search_no_match_returns_empty()
    {
        QSignalSpy spy(m_backend, &FlatpakBackend::searchResultsReady);
        m_backend->search("zzznomatchxyz");
        QVERIFY(spy.wait(5000));
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(results.isEmpty());
    }

    // --- listInstalled ---

    void test_list_installed_does_not_crash()
    {
        QList<Package> installed = m_backend->listInstalled();
        QVERIFY(installed.size() >= 0);
    }

    void test_installed_flatpaks_have_correct_source()
    {
        QList<Package> installed = m_backend->listInstalled();
        for (const Package &p : installed) {
            QCOMPARE(p.source, Package::Source::Flatpak);
        }
    }

    // --- checkUpdates ---

    void test_check_updates_does_not_crash()
    {
        QList<Package> updates = m_backend->checkUpdates();
        QVERIFY(updates.size() >= 0);
    }

    // --- Unified Package struct compatibility ---

    void test_flatpak_package_is_assignable_to_base_struct()
    {
        QSignalSpy spy(m_backend, &FlatpakBackend::searchResultsReady);
        m_backend->search("firefox");
        QVERIFY(spy.wait(5000));
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        if (!results.isEmpty()) {
            Package copy = results.first(); // must be copyable for model use
            QCOMPARE(copy.source, Package::Source::Flatpak);
        }
    }

private:
    FlatpakBackend *m_backend = nullptr;
};

QTEST_MAIN(TestFlatpak)
#include "test_flatpak.moc"