#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>

class TransactionManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    explicit TransactionManager(QObject *parent = nullptr);

    Q_INVOKABLE void install(const QString &pkgId, int source);
    Q_INVOKABLE void remove(const QString &pkgId, int source, bool cascade = false);
    Q_INVOKABLE void removeOrphans(const QStringList &orphans);
    Q_INVOKABLE void fixInstallReasons(const QStringList &packages);
    Q_INVOKABLE void installLocal(const QString &filepath);
    Q_INVOKABLE void syncDatabases();
    Q_INVOKABLE void systemUpgrade(const QStringList &packages = QStringList());

    bool busy() const;

    Q_INVOKABLE QString stripPkgName(const QString &pkgId) const;

signals:
    void transactionStarted(const QString &pkgId);
    void transactionProgress(const QString &pkgId, int percent, const QString &step);
    void transactionFinished(const QString &pkgId, bool success, const QString &error);
    void syncStarted();
    void syncProgress(int percent, const QString &step);
    void syncFinished(bool success, const QString &error);
    void upgradeStarted();
    void upgradeProgress(int percent, const QString &step);
    void upgradeFinished(bool success, const QString &error);
    void busyChanged();

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void runHelper(const QStringList &args);
    void parseLine(const QByteArray &line);

    QProcess *m_process = nullptr;
    QString m_currentPkgId;
    QString m_errorMessage;
    QString m_stderrBuffer;
    int m_errorCode = 0;
    bool m_busy = false;
    bool m_isSync = false;
    bool m_isUpgrade = false;

    void setBusy(bool busy);

#ifdef QT_TESTLIB_LIB
    friend class TestTransaction;
#endif
};

#endif // TRANSACTIONMANAGER_H