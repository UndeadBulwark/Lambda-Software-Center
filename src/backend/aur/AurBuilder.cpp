#include "AurBuilder.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lscAurBuilder, "lsc.aur.builder")

AurBuilder::AurBuilder(QObject *parent)
    : QObject(parent)
{
}

QString AurBuilder::getBuildDir(const QString &pkgName) const {
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return cacheDir + QStringLiteral("/aur/%1").arg(pkgName);
}

void AurBuilder::gitClone(const QString &pkgName, const QString &gitUrl) {
    m_currentPkgName = pkgName;

    QString buildDir = getBuildDir(pkgName);

    if (QDir(buildDir).exists()) {
        qCDebug(lscAurBuilder) << "removing existing build dir:" << buildDir;
        QDir(buildDir).removeRecursively();
    }

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/aur"));

    emit buildProgress(pkgName, 0, QStringLiteral("Cloning PKGBUILD..."));

    cleanupProcess();
    m_process = new QProcess(this);
    m_process->setProgram(QStringLiteral("git"));
    m_process->setArguments({QStringLiteral("clone"), gitUrl, buildDir});

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AurBuilder::onCloneFinished);

    qCDebug(lscAurBuilder) << "git clone" << gitUrl << "→" << buildDir;
    m_process->start();
}

void AurBuilder::onCloneFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitStatus)

    QString pkgName = m_currentPkgName;
    cleanupProcess();

    if (exitCode != 0) {
        qCWarning(lscAurBuilder) << "git clone failed for" << pkgName << "exitCode:" << exitCode;
        emit buildFinished(pkgName, false, QStringLiteral("Failed to clone AUR repository"));
        return;
    }

    emit buildProgress(pkgName, 30, QStringLiteral("Reading PKGBUILD..."));

    QString buildDir = getBuildDir(pkgName);
    QString pkgbuildPath = buildDir + QStringLiteral("/PKGBUILD");

    QFile f(pkgbuildPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(lscAurBuilder) << "failed to read PKGBUILD:" << pkgbuildPath;
        emit buildFinished(pkgName, false, QStringLiteral("Failed to read PKGBUILD"));
        return;
    }

    QString content = QString::fromUtf8(f.readAll());
    f.close();

    qCDebug(lscAurBuilder) << "PKGBUILD read for" << pkgName << content.length() << "chars";
    emit pkgbuildReady(pkgName, content);
}

void AurBuilder::makepkg(const QString &pkgName, const QString &buildDir) {
    m_currentPkgName = pkgName;

    emit buildProgress(pkgName, 35, QStringLiteral("Starting build..."));

    cleanupProcess();
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(buildDir);
    m_process->setProgram(QStringLiteral("makepkg"));
    m_process->setArguments({
        QStringLiteral("--syncdeps"),
        QStringLiteral("--noconfirm")
    });

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &AurBuilder::onMakepkgReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &AurBuilder::onMakepkgReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AurBuilder::onMakepkgFinished);

    qCDebug(lscAurBuilder) << "makepkg in" << buildDir;
    m_process->start();
}

void AurBuilder::onMakepkgReadyReadStandardOutput() {
    if (!m_process) return;
    QByteArray data = m_process->readAllStandardOutput();
    QString output = QString::fromUtf8(data);
    qCDebug(lscAurBuilder) << "makepkg stdout:" << output.trimmed();

    if (output.contains(QStringLiteral("Checking runtime dependencies")))
        emit buildProgress(m_currentPkgName, 38, QStringLiteral("Checking dependencies"));
    else if (output.contains(QStringLiteral("Checking build dependencies")))
        emit buildProgress(m_currentPkgName, 42, QStringLiteral("Checking build dependencies"));
    else if (output.contains(QStringLiteral("Fetching")) || output.contains(QStringLiteral("Downloading sources")))
        emit buildProgress(m_currentPkgName, 45, QStringLiteral("Downloading sources"));
    else if (output.contains(QStringLiteral("Starting build()")))
        emit buildProgress(m_currentPkgName, 55, QStringLiteral("Building"));
    else if (output.contains(QStringLiteral("Entering")))
        emit buildProgress(m_currentPkgName, 58, QStringLiteral("Building"));
    else if (output.contains(QStringLiteral("Starting check()")))
        emit buildProgress(m_currentPkgName, 70, QStringLiteral("Running checks"));
    else if (output.contains(QStringLiteral("Starting package()")))
        emit buildProgress(m_currentPkgName, 80, QStringLiteral("Packaging"));
    else if (output.contains(QStringLiteral("Finished making")))
        emit buildProgress(m_currentPkgName, 92, QStringLiteral("Packaging complete"));
}

void AurBuilder::onMakepkgReadyReadStandardError() {
    if (!m_process) return;
    QByteArray data = m_process->readAllStandardError();
    qCDebug(lscAurBuilder) << "makepkg stderr:" << QString::fromUtf8(data).trimmed();
}

void AurBuilder::onMakepkgFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitStatus)

    QString pkgName = m_currentPkgName;
    QString buildDir = getBuildDir(pkgName);

    if (exitCode != 0) {
        QString stderrOutput;
        if (m_process)
            stderrOutput = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
        cleanupProcess();
        qCWarning(lscAurBuilder) << "makepkg failed for" << pkgName << "exitCode:" << exitCode;
        QString error = stderrOutput.isEmpty()
            ? QStringLiteral("Build failed (exit code %1)").arg(exitCode)
            : stderrOutput;
        emit buildFinished(pkgName, false, error);
        return;
    }

    cleanupProcess();

    QStringList filters;
    filters << QStringLiteral("*.pkg.tar.zst") << QStringLiteral("*.pkg.tar.xz");
    QDir dir(buildDir);
    QStringList packages = dir.entryList(filters, QDir::Files, QDir::Time);

    if (packages.isEmpty()) {
        qCWarning(lscAurBuilder) << "no .pkg.tar.* found after makepkg for" << pkgName;
        emit buildFinished(pkgName, false, QStringLiteral("Build produced no package file"));
        return;
    }

    QString filepath = dir.absoluteFilePath(packages.first());
    qCDebug(lscAurBuilder) << "built package:" << filepath;
    emit buildProgress(pkgName, 95, QStringLiteral("Build complete"));
    emit buildFinished(pkgName, true, filepath);
}

void AurBuilder::cancelBuild() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
    QString pkgName = m_currentPkgName;
    cleanupProcess();
    emit buildFinished(pkgName, false, QStringLiteral("Cancelled by user"));
}

void AurBuilder::cleanupProcess() {
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}