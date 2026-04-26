#include <QtTest>
#include "TransactionManager.h"

class TestTransaction : public QObject
{
    Q_OBJECT

private slots:
    void testStripPkgName_withSourceSuffix()
    {
        TransactionManager tm;
        QString result = tm.stripPkgName(QStringLiteral("firefox@pacman"));
        QCOMPARE(result, QStringLiteral("firefox"));
    }

    void testStripPkgName_withoutSourceSuffix()
    {
        TransactionManager tm;
        QString result = tm.stripPkgName(QStringLiteral("org.mozilla.firefox"));
        QCOMPARE(result, QStringLiteral("org.mozilla.firefox"));
    }

    void testStripPkgName_emptyString()
    {
        TransactionManager tm;
        QString result = tm.stripPkgName(QStringLiteral(""));
        QCOMPARE(result, QStringLiteral(""));
    }

    void testStripPkgName_atStartOnly()
    {
        TransactionManager tm;
        QString result = tm.stripPkgName(QStringLiteral("@pacman"));
        QCOMPARE(result, QStringLiteral("@pacman"));
    }

    void testStripPkgName_multipleAtSigns()
    {
        TransactionManager tm;
        QString result = tm.stripPkgName(QStringLiteral("some@thing@pacman"));
        QCOMPARE(result, QStringLiteral("some@thing"));
    }

    void testProgressLine_parsing()
    {
        TransactionManager tm;
        QSignalSpy progressSpy(&tm, &TransactionManager::transactionProgress);

        tm.m_currentPkgId = QStringLiteral("test@pacman");
        QByteArray line("PROGRESS 50 Downloading");
        tm.parseLine(line);

        QCOMPARE(progressSpy.count(), 1);
        QList<QVariant> args = progressSpy.takeFirst();
        QCOMPARE(args.at(1).toInt(), 50);
        QCOMPARE(args.at(2).toString(), QStringLiteral("Downloading"));
    }

    void testProgressLine_zeroPercent()
    {
        TransactionManager tm;
        QSignalSpy progressSpy(&tm, &TransactionManager::transactionProgress);

        tm.m_currentPkgId = QStringLiteral("test@pacman");
        QByteArray line("PROGRESS 0 Syncing databases");
        tm.parseLine(line);

        QCOMPARE(progressSpy.count(), 1);
        QList<QVariant> args = progressSpy.takeFirst();
        QCOMPARE(args.at(1).toInt(), 0);
        QCOMPARE(args.at(2).toString(), QStringLiteral("Syncing databases"));
    }

    void testProgressLine_100Percent()
    {
        TransactionManager tm;
        QSignalSpy progressSpy(&tm, &TransactionManager::transactionProgress);

        tm.m_currentPkgId = QStringLiteral("test@pacman");
        QByteArray line("PROGRESS 100 Complete");
        tm.parseLine(line);

        QCOMPARE(progressSpy.count(), 1);
        QList<QVariant> args = progressSpy.takeFirst();
        QCOMPARE(args.at(1).toInt(), 100);
        QCOMPARE(args.at(2).toString(), QStringLiteral("Complete"));
    }

    void testErrorLine_parsing()
    {
        TransactionManager tm;

        tm.m_currentPkgId = QStringLiteral("test@pacman");
        QByteArray line("ERROR conflict detected");
        tm.parseLine(line);

        QCOMPARE(tm.m_errorMessage, QStringLiteral("conflict detected"));
    }

    void testInvalidLine_ignored()
    {
        TransactionManager tm;
        QSignalSpy progressSpy(&tm, &TransactionManager::transactionProgress);
        QSignalSpy finishedSpy(&tm, &TransactionManager::transactionFinished);

        QByteArray line("GARBAGE data here");
        tm.parseLine(line);

        QCOMPARE(progressSpy.count(), 0);
        QCOMPARE(finishedSpy.count(), 0);
    }

    void testBusyProperty_initiallyFalse()
    {
        TransactionManager tm;
        QCOMPARE(tm.busy(), false);
    }

    void testInstall_whileBusy_rejected()
    {
        TransactionManager tm;
        tm.m_busy = true;
        QSignalSpy startedSpy(&tm, &TransactionManager::transactionStarted);

        tm.install(QStringLiteral("test@pacman"), 0);

        QCOMPARE(startedSpy.count(), 0);
    }

    void testRemove_whileBusy_rejected()
    {
        TransactionManager tm;
        tm.m_busy = true;
        QSignalSpy startedSpy(&tm, &TransactionManager::transactionStarted);

        tm.remove(QStringLiteral("test@pacman"), 0);

        QCOMPARE(startedSpy.count(), 0);
    }
};

QTEST_MAIN(TestTransaction)
#include "test_transaction.moc"