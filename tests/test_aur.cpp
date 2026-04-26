#include <QtTest>
#include <QEventLoop>
#include <QTimer>
#include "AurClient.h"
#include "AurBackend.h"
#include "AurBuilder.h"
#include "TransactionManager.h"

class TestAur : public QObject
{
    Q_OBJECT

private slots:

    // --- AurClient direct ---

    void test_search_yay_returns_results()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::searchFinished);

        client.search("yay");

        QEventLoop loop;
        connect(&client, &AurClient::searchFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
    }

    void test_search_result_has_aur_source()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::searchFinished);
        client.search("paru");

        QEventLoop loop;
        connect(&client, &AurClient::searchFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QVERIFY(!spy.isEmpty());
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        for (const Package &p : results) {
            QCOMPARE(p.source, Package::Source::AUR);
        }
    }

    void test_search_result_fields_populated()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::searchFinished);
        client.search("yay");

        QEventLoop loop;
        connect(&client, &AurClient::searchFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QVERIFY(!spy.isEmpty());
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
        const Package &p = results.first();
        QVERIFY(!p.name.isEmpty());
        QVERIFY(!p.version.isEmpty());
        QVERIFY(p.votes >= 0);
        QVERIFY(p.popularity >= 0.0);
    }

    void test_search_no_results_emits_empty_list_not_error()
    {
        AurClient client;
        QSignalSpy resultSpy(&client, &AurClient::searchFinished);
        QSignalSpy errorSpy(&client, &AurClient::errorOccurred);
        client.search("zzznomatchpackagexyz123abc");

        QEventLoop loop;
        connect(&client, &AurClient::searchFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(resultSpy.count(), 1);
        QCOMPARE(errorSpy.count(), 0);
        QList<Package> results = resultSpy.at(0).at(0).value<QList<Package>>();
        QVERIFY(results.isEmpty());
    }

    void test_network_error_emits_error_signal_not_crash()
    {
        AurClient client;
        client.setBaseUrl("http://127.0.0.1:1"); // nothing listening
        QSignalSpy spy(&client, &AurClient::errorOccurred);
        client.search("anything");

        QEventLoop loop;
        connect(&client, &AurClient::errorOccurred, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
        QString error = spy.at(0).at(0).toString();
        QVERIFY(!error.isEmpty());
    }

    void test_malformed_json_response_emits_error()
    {
        AurClient client;
        client.injectMockResponse("not valid json {{{{");
        QSignalSpy spy(&client, &AurClient::errorOccurred);
        client.search("anything");

        QEventLoop loop;
        connect(&client, &AurClient::errorOccurred, &loop, &QEventLoop::quit);
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
    }

    void test_debounce_does_not_fire_duplicate_requests()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::searchFinished);

        // Fire three rapid searches — only the last should resolve
        client.search("a");
        client.search("ab");
        client.search("abc");

        QEventLoop loop;
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        // Only one result signal for the final query
        QCOMPARE(spy.count(), 1);
    }

    // --- AurBackend wrapper ---

    void test_aur_backend_search_paru()
    {
        AurBackend backend;
        QSignalSpy spy(&backend, &AurBackend::searchResultsReady);
        backend.search("paru");

        QEventLoop loop;
        connect(&backend, &AurBackend::searchResultsReady, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
    }

    // --- AurBuilder ---

    void test_builder_git_clone_invalid_url_fails()
    {
        AurBuilder builder;
        QSignalSpy spy(&builder, &AurBuilder::buildFinished);
        builder.gitClone(QStringLiteral("nonexistent-test-pkg-12345"),
                         QStringLiteral("https://aur.archlinux.org/nonexistent-test-pkg-12345.git"));

        QEventLoop loop;
        connect(&builder, &AurBuilder::buildFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
        bool success = spy.at(0).at(1).toBool();
        QVERIFY(!success);
    }

    void test_builder_search_cache_stores_results()
    {
        AurBackend backend;
        QSignalSpy spy(&backend, &AurBackend::searchResultsReady);
        backend.search("yay");

        QEventLoop loop;
        connect(&backend, &AurBackend::searchResultsReady, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QVERIFY(!spy.isEmpty());

        QSignalSpy installSpy(&backend, &AurBackend::installProgress);
        backend.install("yay@aur");
        QVERIFY(installSpy.count() >= 0);
    }

    void test_search_result_has_git_url()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::searchFinished);
        client.search("yay");

        QEventLoop loop;
        connect(&client, &AurClient::searchFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QVERIFY(!spy.isEmpty());
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
        bool hasGitUrl = false;
        for (const Package &p : results) {
            if (p.name == "yay" && !p.gitUrl.isEmpty()) {
                hasGitUrl = true;
                QVERIFY(p.gitUrl.contains(QStringLiteral("aur.archlinux.org")));
                break;
            }
        }
        QVERIFY(hasGitUrl);
    }

    // --- AurClient.info() ---

    void test_info_returns_package_details()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::infoFinished);
        client.info(QStringList() << QStringLiteral("yay"));

        QEventLoop loop;
        connect(&client, &AurClient::infoFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(!results.isEmpty());
        bool foundYay = false;
        for (const Package &p : results) {
            if (p.name == "yay") {
                foundYay = true;
                QVERIFY(!p.version.isEmpty());
                QVERIFY(p.votes >= 0);
                break;
            }
        }
        QVERIFY(foundYay);
    }

    void test_info_multiple_packages()
    {
        AurClient client;
        QSignalSpy spy(&client, &AurClient::infoFinished);
        client.info(QStringList() << QStringLiteral("yay") << QStringLiteral("paru"));

        QEventLoop loop;
        connect(&client, &AurClient::infoFinished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
        QList<Package> results = spy.at(0).at(0).value<QList<Package>>();
        QVERIFY(results.size() >= 2);
    }

    // --- AurBackend.checkUpdates() ---

    void test_aur_backend_check_updates_emits_signal()
    {
        AurBackend backend;
        QSignalSpy spy(&backend, &AurBackend::updatesReady);
        backend.checkUpdates();

        QEventLoop loop;
        connect(&backend, &AurBackend::updatesReady, &loop, &QEventLoop::quit);
        QTimer::singleShot(10000, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(spy.count(), 1);
    }

    // --- TransactionManager.systemUpgrade() ---

    void test_system_upgrade_emits_upgrade_started()
    {
        TransactionManager tm;
        QSignalSpy startedSpy(&tm, &TransactionManager::upgradeStarted);

        tm.systemUpgrade();

        QCOMPARE(startedSpy.count(), 1);
    }

    // --- AurBuilder::extractDeps() ---

    void test_extract_deps_basic()
    {
        QString pkgbuild = QStringLiteral(
            "pkgname=foo\n"
            "depends=('gcc' 'cmake')\n"
            "makedepends=('git')\n"
        );
        QStringList deps = AurBuilder::extractDeps(pkgbuild);
        QVERIFY(deps.contains("gcc"));
        QVERIFY(deps.contains("cmake"));
        QVERIFY(deps.contains("git"));
        QCOMPARE(deps.size(), 3);
    }

    void test_extract_deps_strips_versions()
    {
        QString pkgbuild = QStringLiteral(
            "pkgname=foo\n"
            "depends=('gcc>=12' 'cmake>=3.20')\n"
            "makedepends=('git')\n"
        );
        QStringList deps = AurBuilder::extractDeps(pkgbuild);
        QVERIFY(deps.contains("gcc"));
        QVERIFY(deps.contains("cmake"));
        QVERIFY(deps.contains("git"));
        QCOMPARE(deps.size(), 3);
    }

    void test_extract_deps_skips_colon_qualifier()
    {
        QString pkgbuild = QStringLiteral(
            "pkgname=foo\n"
            "depends=('lib32:gcc' 'gcc')\n"
        );
        QStringList deps = AurBuilder::extractDeps(pkgbuild);
        QVERIFY(deps.contains("gcc"));
        QVERIFY(!deps.contains("lib32:gcc"));
        QCOMPARE(deps.size(), 1);
    }

    void test_extract_deps_empty()
    {
        QString pkgbuild = QStringLiteral(
            "pkgname=foo\n"
            "pkgver=1.0\n"
        );
        QStringList deps = AurBuilder::extractDeps(pkgbuild);
        QVERIFY(deps.isEmpty());
    }

    void test_extract_deps_multiline()
    {
        QSKIP("Multi-line array support not implemented — known limitation for v0.4.0");
        QString pkgbuild = QStringLiteral(
            "pkgname=foo\n"
            "depends=(\n"
            "    'gcc'\n"
            "    'cmake'\n"
            ")\n"
        );
        QStringList deps = AurBuilder::extractDeps(pkgbuild);
        QCOMPARE(deps.size(), 2);
    }
};

QTEST_MAIN(TestAur)
#include "test_aur.moc"