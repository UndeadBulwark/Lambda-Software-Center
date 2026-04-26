#ifndef INSTALLEDMODELAGGREGATOR_H
#define INSTALLEDMODELAGGREGATOR_H

#include <QObject>
#include <QList>
#include "Package.h"

class PackageListModel;
class PacmanBackend;
class AurBackend;
class FlatpakBackend;

class InstalledModelAggregator : public QObject {
    Q_OBJECT
public:
    InstalledModelAggregator(PackageListModel *target, QObject *parent = nullptr);

    void setBackends(PacmanBackend *pacman, AurBackend *aur, FlatpakBackend *flatpak);
    Q_INVOKABLE void refresh();

private slots:
    void onPacmanInstalledReady(const QList<Package> &packages);
    void onFlatpakInstalledReady(const QList<Package> &packages);
    void onAurInstalledReady(const QList<Package> &packages);

private:
    void checkAllReady();

    PackageListModel *m_target;
    PacmanBackend *m_pacman = nullptr;
    AurBackend *m_aur = nullptr;
    FlatpakBackend *m_flatpak = nullptr;
    QList<Package> m_buffer;
    int m_pendingCount = 0;
    static const int EXPECTED_BACKENDS = 3;
};

#endif // INSTALLEDMODELAGGREGATOR_H