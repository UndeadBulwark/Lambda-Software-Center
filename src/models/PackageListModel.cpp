#include "PackageListModel.h"

PackageListModel::PackageListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PackageListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_packages.size();
}

QVariant PackageListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_packages.size())
        return QVariant();

    const Package &pkg = m_packages.at(index.row());
    switch (role) {
    case IdRole: return pkg.id;
    case NameRole: return pkg.name;
    case VersionRole: return pkg.version;
    case NewVersionRole: return pkg.newVersion;
    case DescriptionRole: return pkg.description;
    case LongDescriptionRole: return pkg.longDescription;
    case CategoriesRole: return pkg.categories;
    case SourceRole: return QVariant::fromValue(pkg.source);
    case StateRole: return QVariant::fromValue(pkg.state);
    case InstalledSizeRole: return pkg.installedSize;
    case DownloadSizeRole: return pkg.downloadSize;
    case IconUrlRole: return pkg.iconUrl;
    case ScreenshotUrlsRole: return QVariant::fromValue(pkg.screenshotUrls);
    case DependenciesRole: return pkg.dependencies;
    case VotesRole: return pkg.votes;
    case PopularityRole: return pkg.popularity;
    case RatingRole: return pkg.rating;
    case FlatpakRefRole: return pkg.flatpakRef;
    default: return QVariant();
    }
}

QHash<int, QByteArray> PackageListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole]               = "packageId";
    roles[NameRole]             = "name";
    roles[VersionRole]          = "version";
    roles[NewVersionRole]       = "newVersion";
    roles[DescriptionRole]      = "description";
    roles[LongDescriptionRole]  = "longDescription";
    roles[CategoriesRole]       = "categories";
    roles[SourceRole]           = "source";
    roles[StateRole]            = "state";
    roles[InstalledSizeRole]    = "installedSize";
    roles[DownloadSizeRole]     = "downloadSize";
    roles[IconUrlRole]          = "iconUrl";
    roles[ScreenshotUrlsRole]   = "screenshotUrls";
    roles[DependenciesRole]     = "dependencies";
    roles[VotesRole]            = "votes";
    roles[PopularityRole]       = "popularity";
    roles[RatingRole]           = "rating";
    roles[FlatpakRefRole]       = "flatpakRef";
    return roles;
}

void PackageListModel::setPackages(const QList<Package> &packages) {
    beginResetModel();
    m_packages = packages;
    endResetModel();
}

QList<Package> PackageListModel::packages() const {
    return m_packages;
}
