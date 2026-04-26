#include "InstalledModelAggregator.h"
#include "PackageListModel.h"
#include "backend/pacman/PacmanBackend.h"
#include "backend/aur/AurBackend.h"
#include "backend/flatpak/FlatpakBackend.h"

InstalledModelAggregator::InstalledModelAggregator(PackageListModel *target, QObject *parent)
    : QObject(parent)
    , m_target(target)
{
}

void InstalledModelAggregator::setBackends(PacmanBackend *pacman, AurBackend *aur, FlatpakBackend *flatpak)
{
    m_pacman = pacman;
    m_aur = aur;
    m_flatpak = flatpak;

    if (m_pacman)
        connect(m_pacman, &IPackageBackend::installedListReady,
                this, &InstalledModelAggregator::onPacmanInstalledReady);
    if (m_aur)
        connect(m_aur, &IPackageBackend::installedListReady,
                this, &InstalledModelAggregator::onAurInstalledReady);
    if (m_flatpak)
        connect(m_flatpak, &IPackageBackend::installedListReady,
                this, &InstalledModelAggregator::onFlatpakInstalledReady);
}

void InstalledModelAggregator::refresh()
{
    m_buffer.clear();
    m_pendingCount = EXPECTED_BACKENDS;

    if (m_pacman)
        m_pacman->listInstalled();
    else
        m_pendingCount--;

    if (m_aur)
        m_aur->listInstalled();
    else
        m_pendingCount--;

    if (m_flatpak)
        m_flatpak->listInstalled();
    else
        m_pendingCount--;

    if (m_pendingCount <= 0)
        checkAllReady();
}

void InstalledModelAggregator::onPacmanInstalledReady(const QList<Package> &packages)
{
    m_buffer.append(packages);
    m_pendingCount--;
    checkAllReady();
}

void InstalledModelAggregator::onAurInstalledReady(const QList<Package> &packages)
{
    m_buffer.append(packages);
    m_pendingCount--;
    checkAllReady();
}

void InstalledModelAggregator::onFlatpakInstalledReady(const QList<Package> &packages)
{
    m_buffer.append(packages);
    m_pendingCount--;
    checkAllReady();
}

void InstalledModelAggregator::checkAllReady()
{
    if (m_pendingCount <= 0) {
        m_target->setPackages(m_buffer);
        m_buffer.clear();
    }
}