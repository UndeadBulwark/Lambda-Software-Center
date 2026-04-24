#include <QtTest>
#include <QEventLoop>
#include <QTimer>
#include "AurClient.h"
#include "AurBackend.h"

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
};

QTEST_MAIN(TestAur)
#include "test_aur.moc"