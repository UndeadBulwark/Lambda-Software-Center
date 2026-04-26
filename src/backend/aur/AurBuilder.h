#ifndef AURBUILDER_H
#define AURBUILDER_H

#include <QObject>
#include <QString>
#include <QProcess>

class AurBuilder : public QObject {
    Q_OBJECT
public:
    explicit AurBuilder(QObject *parent = nullptr);

    void gitClone(const QString &pkgName, const QString &gitUrl);
    void makepkg(const QString &pkgName, const QString &buildDir);
    void cancelBuild();

signals:
    void buildProgress(const QString &pkgName, int percent, const QString &step);
    void buildFinished(const QString &pkgName, bool success, const QString &error);
    void pkgbuildReady(const QString &pkgName, const QString &content);

private:
    QProcess *m_process = nullptr;
    QString m_currentPkgName;

    QString getBuildDir(const QString &pkgName) const;
    void onCloneFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onMakepkgReadyReadStandardOutput();
    void onMakepkgReadyReadStandardError();
    void onMakepkgFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cleanupProcess();
};

#endif // AURBUILDER_H