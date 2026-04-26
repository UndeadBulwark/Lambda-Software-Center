#include "TransactionManager.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QLoggingCategory>
#include <QDateTime>

Q_LOGGING_CATEGORY(lscTransaction, "lsc.transaction")

static QFile *s_logFile = nullptr;

static void logToFile(const QString &msg) {
    if (!s_logFile) {
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(cacheDir);
        s_logFile = new QFile(cacheDir + QStringLiteral("/transaction.log"));
        if (!s_logFile->open(QIODevice::Append | QIODevice::Text)) {
            delete s_logFile;
            s_logFile = nullptr;
            return;
        }
    }
    s_logFile->write(msg.toUtf8());
    s_logFile->write("\n");
    s_logFile->flush();
}

TransactionManager::TransactionManager(QObject *parent)
    : QObject(parent)
{
}

bool TransactionManager::busy() const {
    return m_busy;
}

QString TransactionManager::stripPkgName(const QString &pkgId) const {
    int atPos = pkgId.lastIndexOf(QLatin1Char('@'));
    if (atPos > 0)
        return pkgId.left(atPos);
    return pkgId;
}

void TransactionManager::install(const QString &pkgId, int /*source*/) {
    if (m_busy)
        return;

    m_currentPkgId = pkgId;
    m_isSync = false;
    m_isUpgrade = false;
    m_isInstallDeps = false;
    QString pkgName = stripPkgName(pkgId);

    qCDebug(lscTransaction) << "install:" << pkgId << "→ stripped:" << pkgName;

    QStringList args;
    args << QStringLiteral("install") << pkgName;

    emit transactionStarted(pkgId);
    setBusy(true);
    runHelper(args);
}

void TransactionManager::remove(const QString &pkgId, int /*source*/, bool cascade) {
    if (m_busy)
        return;

    m_currentPkgId = pkgId;
    m_isSync = false;
    m_isUpgrade = false;
    m_isInstallDeps = false;
    QString pkgName = stripPkgName(pkgId);

    qCDebug(lscTransaction) << "remove:" << pkgId << "→ stripped:" << pkgName << "cascade:" << cascade;

    QStringList args;
    args << QStringLiteral("remove") << pkgName;
    if (cascade)
        args << QStringLiteral("--cascade");

    emit transactionStarted(pkgId);
    setBusy(true);
    runHelper(args);
}

void TransactionManager::syncDatabases() {
    if (m_busy)
        return;

    m_isSync = true;
    m_isUpgrade = false;
    m_isInstallDeps = false;
    m_currentPkgId.clear();

    qCDebug(lscTransaction) << "syncDatabases";

    emit syncStarted();
    setBusy(true);
    runHelper({QStringLiteral("sync")});
}

void TransactionManager::removeOrphans(const QStringList &orphans) {
    if (m_busy)
        return;

    if (orphans.isEmpty())
        return;

    m_isSync = false;
    m_isUpgrade = false;
    m_isInstallDeps = false;
    m_currentPkgId = QStringLiteral("orphan-cleanup");

    QStringList args;
    args << QStringLiteral("remove-orphans");
    for (const QString &name : orphans)
        args << name;

    qCDebug(lscTransaction) << "removeOrphans:" << orphans;

    emit transactionStarted(m_currentPkgId);
    setBusy(true);
    runHelper(args);
}

void TransactionManager::fixInstallReasons(const QStringList &packages) {
    if (m_busy)
        return;

    if (packages.isEmpty())
        return;

    m_isSync = true;
    m_isUpgrade = false;
    m_isInstallDeps = false;
    m_currentPkgId.clear();

    QStringList args;
    args << QStringLiteral("set-reason");
    for (const QString &name : packages)
        args << name;

    qCDebug(lscTransaction) << "fixInstallReasons:" << packages;

    emit syncStarted();
    setBusy(true);
    runHelper(args);
}

void TransactionManager::installLocal(const QString &filepath) {
    if (m_busy)
        return;

    m_isSync = false;
    m_isUpgrade = false;
    m_isInstallDeps = false;
    m_currentPkgId = filepath;

    QStringList args;
    args << QStringLiteral("install-local") << filepath;

    qCDebug(lscTransaction) << "installLocal:" << filepath;

    emit transactionStarted(m_currentPkgId);
    setBusy(true);
    runHelper(args);
}

void TransactionManager::installDeps(const QStringList &pkgNames) {
    if (m_busy)
        return;

    if (pkgNames.isEmpty())
        return;

    m_isSync = false;
    m_isUpgrade = false;
    m_isInstallDeps = true;
    m_currentPkgId = QStringLiteral("install-deps");

    QStringList args;
    args << QStringLiteral("install-deps");
    for (const QString &name : pkgNames)
        args << name;

    qCDebug(lscTransaction) << "installDeps:" << pkgNames;

    emit transactionStarted(m_currentPkgId);
    setBusy(true);
    runHelper(args);
}

void TransactionManager::systemUpgrade(const QStringList &packages) {
    if (m_busy)
        return;

    m_isSync = false;
    m_isUpgrade = true;
    m_isInstallDeps = false;
    m_currentPkgId = packages.isEmpty()
        ? QStringLiteral("system-upgrade")
        : QStringLiteral("selective-upgrade");

    QStringList args;
    args << QStringLiteral("upgrade");
    for (const QString &name : packages)
        args << name;

    qCDebug(lscTransaction) << "systemUpgrade:" << (packages.isEmpty() ? "full" : packages.join(", "));

    emit upgradeStarted();
    setBusy(true);
    runHelper(args);
}

void TransactionManager::runHelper(const QStringList &args) {
    if (m_process) {
        m_process->deleteLater();
    }

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    m_errorMessage.clear();
    m_stderrBuffer.clear();
    m_errorCode = 0;

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &TransactionManager::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &TransactionManager::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TransactionManager::onProcessFinished);

    QString helperPath;
    QFileInfo installed(QStringLiteral("/usr/bin/lsc-helper"));
    if (installed.isExecutable()) {
        helperPath = installed.absoluteFilePath();
    } else {
        helperPath = QStandardPaths::findExecutable(QStringLiteral("lsc-helper"),
            QStringList() << QStringLiteral("/usr/bin")
                          << QStringLiteral("/usr/local/bin")
                          << QFileInfo(QStringLiteral("/proc/self/exe")).canonicalFilePath().section(QLatin1Char('/'), 0, -2));
        if (helperPath.isEmpty()) {
#ifdef LSC_BUILD_DIR
            QFileInfo buildHelper(QStringLiteral(LSC_BUILD_DIR) + QStringLiteral("/lsc-helper"));
            if (buildHelper.isExecutable())
                helperPath = buildHelper.absoluteFilePath();
#endif
        }
    }

    if (helperPath.isEmpty()) {
        qCWarning(lscTransaction) << "lsc-helper not found";
        if (m_isSync)
            emit syncFinished(false, QStringLiteral("lsc-helper not found"));
        else
            emit transactionFinished(m_currentPkgId, false, QStringLiteral("lsc-helper not found"));
        setBusy(false);
        return;
    }

    QStringList pkexecArgs;
    pkexecArgs << helperPath << args;

    qCDebug(lscTransaction) << "running: pkexec" << pkexecArgs;
    logToFile(QStringLiteral("=== %1 %2 ===")
              .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
              .arg(args.join(QLatin1Char(' '))));

    m_process->start(QStringLiteral("pkexec"), pkexecArgs);
}

void TransactionManager::onReadyReadStandardOutput() {
    if (!m_process)
        return;

    while (m_process->canReadLine()) {
        QByteArray line = m_process->readLine().trimmed();
        if (!line.isEmpty())
            parseLine(line);
    }
}

void TransactionManager::onReadyReadStandardError() {
    if (!m_process)
        return;
    QByteArray data = m_process->readAllStandardError().trimmed();
    if (!data.isEmpty()) {
        m_stderrBuffer = QString::fromUtf8(data);
        qCDebug(lscTransaction) << "stderr:" << m_stderrBuffer;
        logToFile(QStringLiteral("STDERR: %1").arg(m_stderrBuffer));
    }
}

void TransactionManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    onReadyReadStandardOutput();

    qCDebug(lscTransaction) << "process finished: exitCode=" << exitCode
                            << "exitStatus=" << (exitStatus == QProcess::NormalExit ? "NormalExit" : "CrashExit")
                            << "errorMessage=" << m_errorMessage
                            << "stderrBuffer=" << m_stderrBuffer;

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString errMsg;
        if (!m_errorMessage.isEmpty())
            errMsg = m_errorMessage;
        else if (exitStatus == QProcess::CrashExit)
            errMsg = QStringLiteral("Process crashed");
        else if (exitCode == 126)
            errMsg = QStringLiteral("Authentication cancelled or helper not authorized");
        else if (exitCode == 127)
            errMsg = QStringLiteral("pkexec not found");
        else
            errMsg = QStringLiteral("Operation failed (exit code %1)").arg(exitCode);

        if (!m_stderrBuffer.isEmpty() && errMsg != m_stderrBuffer)
            errMsg += QStringLiteral(" — %1").arg(m_stderrBuffer);

        // Append actionable hints for known alpm error codes
        // ALPM_ERR_RETRIEVE=49, ALPM_ERR_RETRIEVE_PREPARE=48,
        // ALPM_ERR_LIBCURL=52, ALPM_ERR_EXTERNAL_DOWNLOAD=53,
        // ALPM_ERR_PKG_INVALID_CHECKSUM=36, ALPM_ERR_PKG_INVALID_SIG=37,
        // ALPM_ERR_SERVER_BAD_URL=21, ALPM_ERR_SERVER_NONE=22
        if (m_errorCode == 49 || m_errorCode == 48 || m_errorCode == 52 ||
            m_errorCode == 53 || m_errorCode == 21 || m_errorCode == 22) {
            errMsg += QStringLiteral(" — check your network connection or try refreshing databases");
        }

        qCWarning(lscTransaction) << "operation failed:" << errMsg;
        logToFile(QStringLiteral("FAILED: %1").arg(errMsg));

        if (m_isSync)
            emit syncFinished(false, errMsg);
        else if (m_isUpgrade)
            emit upgradeFinished(false, errMsg);
        else
            emit transactionFinished(m_currentPkgId, false, errMsg);
    } else {
        qCDebug(lscTransaction) << "operation succeeded";
        logToFile(QStringLiteral("SUCCESS"));

        if (m_isSync)
            emit syncFinished(true, QString());
        else if (m_isUpgrade)
            emit upgradeFinished(true, QString());
        else
            emit transactionFinished(m_currentPkgId, true, QString());
    }

    setBusy(false);
    m_process->deleteLater();
    m_process = nullptr;
}

void TransactionManager::parseLine(const QByteArray &line) {
    if (line.startsWith("PROGRESS ")) {
        QByteArray rest = line.mid(9);
        int spacePos = rest.indexOf(' ');
        if (spacePos <= 0)
            return;

        bool ok = false;
        int percent = rest.left(spacePos).toInt(&ok);
        if (!ok)
            return;

        QString step = QString::fromUtf8(rest.mid(spacePos + 1));
        if (m_isSync)
            emit syncProgress(percent, step);
        else if (m_isUpgrade)
            emit upgradeProgress(percent, step);
        else
            emit transactionProgress(m_currentPkgId, percent, step);
    } else if (line.startsWith("ERROR ")) {
        m_errorMessage = QString::fromUtf8(line.mid(6));
        qCWarning(lscTransaction) << "helper error:" << m_errorMessage;
        logToFile(QStringLiteral("ERROR: %1").arg(m_errorMessage));
    } else if (line.startsWith("ERRCODE ")) {
        m_errorCode = line.mid(8).toInt();
        qCDebug(lscTransaction) << "helper error code:" << m_errorCode;
        logToFile(QStringLiteral("ERRCODE: %1").arg(m_errorCode));
    } else if (line.startsWith("DEBUG ")) {
        QString msg = QString::fromUtf8(line.mid(6));
        qCDebug(lscTransaction) << "helper:" << msg;
        logToFile(QStringLiteral("DEBUG: %1").arg(msg));
    } else if (line.startsWith("SCRIPTLET ")) {
        logToFile(QString::fromUtf8(line));
    }
}

void TransactionManager::setBusy(bool busy) {
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged();
    }
}